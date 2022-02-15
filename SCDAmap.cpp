#include "SCDAmap.h"

using namespace std;

void SCDAmap::addRegion(SCregion& region, int backFront)
{
	switch (backFront) {
	case 0:
		dRegion.push_front(std::move(region));
		break;
	case 1:
		dRegion.push_back(std::move(region));
		break;
	}
}
void SCDAmap::displayRegion(vector<vector<unsigned char>> vRGBX)
{
	// Will paint all stored regions, in the order listed. Provided colours
	// will be cycled through.
	int numColour = (int)vRGBX.size();
	if (numColour == 0) { 
		vRGBX = { {0, 0, 0, 255} }; 
		numColour = 1;
	}
	int index{ -1 };
	int numRegion = (int)dRegion.size();
	for (int ii = 0; ii < numRegion; ii++) {
		index = (index + 1) % numColour;

	}

}
void SCDAmap::err(string message)
{
	string errorMessage = "SCDAmap error:\n" + message;
	JLOG::getInstance()->err(errorMessage);
}
void SCDAmap::init()
{
	QGridLayout* gLayout = new QGridLayout;
	this->setLayout(gLayout);

	vector<string> vsHeader = { "" };
	modelCataMap = make_shared<QJTREEMODEL>(vsHeader, this);
	modelCataMap->jtx.setNodeColour(-1, itemColourDefault, itemColourSelected);

	QLabel* qlCataTree = new QLabel("Map Tree Per Catalogue");
	gLayout->addWidget(qlCataTree, 0, index::Tree);
	
	QJTREEVIEW* qjTree = new QJTREEVIEW;
	qjTree->indexTree = index::Tree;
	gLayout->addWidget(qjTree, 1, index::Tree);
	qjTree->setMaximumWidth(400);
	connect(qjTree, &QJTREEVIEW::nodeDoubleClicked, this, &SCDAmap::nodeDoubleClicked);
	connect(qjTree, &QJTREEVIEW::nodeRightClicked, this, &SCDAmap::nodeRightClicked);

	QLabel* qlMap = new QLabel("Geographic Region Map");
	gLayout->addWidget(qlMap, 0, index::Map);

	QJPAINT* qjPaint = new QJPAINT;
	gLayout->addWidget(qjPaint, 1, index::Map);

	initAction();
}
void SCDAmap::initAction()
{
	qaDownloadMap = make_shared<QAction>("Download Maps", this);
	auto downloadMap = qaDownloadMap.get();
	connect(downloadMap, &QAction::triggered, this, [&]() {
		QVariant qVar = qaDownloadMap->data();
		QString qsTemp = qVar.toString();
		string sData = qsTemp.toStdString();
		vector<string> vsPrompt;
		jparse.splitByMarker(vsPrompt, sData);
		SCauto* scauto = new SCauto;
		connect(scauto, &SCauto::appendTextIO, this, &SCDAmap::appendTextIO);
		scauto->downloadMap(vsPrompt);
		});

	qaInsertMap = make_shared<QAction>("Insert Maps", this);
	auto insertMap = qaInsertMap.get();
	connect(insertMap, &QAction::triggered, this, [&]() {
		QVariant qVar = qaInsertMap->data();
		QString qsTemp = qVar.toString();
		string sData = qsTemp.toStdString();
		vector<string> vsPrompt;
		jparse.splitByMarker(vsPrompt, sData);
		emit sendInsertMap(vsPrompt[0], vsPrompt[1]);
		});

	qaLoadTree = make_shared<QAction>("Load Geo Tree", this);
	auto loadTree = qaLoadTree.get();
	connect(loadTree, &QAction::triggered, this, [&]() {
		QVariant qVar = qaLoadTree->data();
		QString qsTemp = qVar.toString();
		string sData = qsTemp.toStdString();
		vector<string> vsPrompt;
		jparse.splitByMarker(vsPrompt, sData);
		emit sendLoadGeoTree(vsPrompt[0], vsPrompt[1]);
		});
}
void SCDAmap::initItemColour(string& xml)
{
	int indexBG = -1, indexFG = -1;
	vector<string> vsTag = { "colour", "solid", "item_default" };
	vector<vector<string>> vvsTag = jparse.getXML(xml, vsTag);
	for (int ii = 0; ii < vvsTag.size(); ii++) {
		if (vvsTag[ii][0] == "background") { indexBG = ii; }
		else if (vvsTag[ii][0] == "foreground") { indexFG = ii; }
	}
	itemColourDefault = make_pair(vvsTag[indexBG][1], vvsTag[indexFG][1]);

	vsTag = { "colour", "solid", "item_fail" };
	vvsTag = jparse.getXML(xml, vsTag);
	for (int ii = 0; ii < vvsTag.size(); ii++) {
		if (vvsTag[ii][0] == "background") { indexBG = ii; }
		else if (vvsTag[ii][0] == "foreground") { indexFG = ii; }
	}
	itemColourFail = make_pair(vvsTag[indexBG][1], vvsTag[indexFG][1]);

	vsTag = { "colour", "solid", "item_selected" };
	vvsTag = jparse.getXML(xml, vsTag);
	for (int ii = 0; ii < vvsTag.size(); ii++) {
		if (vvsTag[ii][0] == "background") { indexBG = ii; }
		else if (vvsTag[ii][0] == "foreground") { indexFG = ii; }
	}
	itemColourSelected = make_pair(vvsTag[indexBG][1], vvsTag[indexFG][1]);

	vsTag = { "colour", "solid", "item_warning" };
	vvsTag = jparse.getXML(xml, vsTag);
	for (int ii = 0; ii < vvsTag.size(); ii++) {
		if (vvsTag[ii][0] == "background") { indexBG = ii; }
		else if (vvsTag[ii][0] == "foreground") { indexFG = ii; }
	}
	itemColourWarning = make_pair(vvsTag[indexBG][1], vvsTag[indexFG][1]);
}
int SCDAmap::insertMapWorker(JBUFFER<vector<vector<string>>, NUM_BUF_SLOT>& jbufSQL, 
	int parentID, string parentDir, string stmtMap, string stmtMapFrame, 
	vector<string>& vsColTitleMap, vector<string>& vsColTitleMapFrame)
{
	// Recursive function made to push CreateTable SQL statements for Map and MapFrame
	// tables, as well as vvsRow vectors to populate those tables with coordinate data. 
	// Operates on a Geo tree structure, starting at a given parent node. 
	string borderFile, geoLayer, pngPath, txtPath, tnameMap, tnameMapFrame;
	vector<string> vsPath;
	vector<vector<string>> vvsRow;
	size_t length, pos1, pos2, pos3, pos4;
	int ID, numPixel;
	vector<pair<double, double>> vCoord;
	vector<int> childrenID = modelCataMap->jt.getChildrenID(parentID);
	int numChildren = (int)childrenID.size();
	if (numChildren == 0) { return 1; }

	for (int ii = 0; ii < numChildren; ii++) {
		// Push CreateTable statements for the Map and MapFrame database tables.
		JNODE& jnChild = modelCataMap->jt.getNode(childrenID[ii]);
		tnameMap = "Map$canada$" + jnChild.vsData[1];  // GEO_CODE
		ID = jnChild.ID;
		geoLayer = jnChild.vsData[2];
		while (geoLayer != "canada") {  // Every Geo tree has this root.
			JNODE& jnParent = modelCataMap->jt.getParent(ID);
			tnameMap.insert(10, "$" + jnParent.vsData[1]);
			ID = jnParent.ID;
			geoLayer = jnParent.vsData[2];
		}
		vvsRow.resize(1, vector<string>(2));
		pos1 = stmtMap.find("\"\"");
		vvsRow[0][0] = stmtMap;
		vvsRow[0][0].insert(pos1 + 1, tnameMap);
		tnameMapFrame = tnameMap;
		tnameMapFrame.insert(3, "Frame");
		pos1 = stmtMapFrame.find("\"\"");
		vvsRow[0][1] = stmtMapFrame;
		vvsRow[0][1].insert(pos1 + 1, tnameMapFrame);
		jbufSQL.pushHard(vvsRow);

		// Load or create a list of coordinates representing the region border.
		vvsRow.clear();
		vvsRow.emplace_back(vsColTitleMap);
		jfile.makeDir(parentDir);
		txtPath = parentDir + "/" + jnChild.vsData[0] + "*.txt";
		vsPath.clear();
		jfile.search(vsPath, txtPath, 0, 0);
		borderFile.clear();
		if (vsPath.size() == 0) {
			// Make a coordinate map from a shaded-region PNG.
			pngPath = parentDir + "/" + jnChild.vsData[0] + "*.png";
			jfile.search(vsPath, pngPath, 0, 0);
			if (vsPath.size() == 0) {
				emit appendTextIO("Skipping " + jnChild.vsData[0] + " map: failed to locate.\n");
				continue;
			}
			for (int jj = 0; jj < vsPath.size(); jj++) {
				pos1 = vsPath[jj].rfind("[border]");
				if (pos1 > vsPath[jj].size()) {
					pngPath = vsPath[jj];
					makeBorderMap(vCoord, pngPath);

					txtPath = pngPath;
					pos1 = txtPath.rfind(".png");
					txtPath.replace(pos1, 4, ".txt");
					numPixel = (int)vCoord.size();
					for (int kk = 0; kk < numPixel; kk++) {
						vvsRow.emplace_back(vector<string>());
						vvsRow.back().emplace_back(to_string(get<0>(vCoord[kk])));
						vvsRow.back().emplace_back(to_string(get<1>(vCoord[kk])));
						borderFile += "(" + vvsRow.back()[0] + ",";
						borderFile += vvsRow.back()[1] + ")\n";
					}
					jfile.printer(txtPath, borderFile);
					break;
				}
			}
			if (vCoord.size() == 0) { err("Failed to make border map-insertMap"); }
		}
		else {
			// Load a previously-made coordinate map.
			txtPath = vsPath[0];
			jfile.load(borderFile, txtPath);
			length = borderFile.size();
			pos1 = borderFile.find('(');
			while (pos1 < length) {
				vvsRow.emplace_back(vector<string>());
				pos2 = borderFile.find(',', pos1 + 1);
				pos3 = borderFile.find_first_of("0123456789", pos2 + 1);
				pos4 = borderFile.find(')', pos3 + 1);
				vvsRow.back().emplace_back(borderFile.substr(pos1 + 1, pos2 - pos1 - 1));
				vvsRow.back().emplace_back(borderFile.substr(pos3, pos4 - pos3));
				pos1 = borderFile.find('(', pos4 + 1);
			}
		}
		vvsRow.emplace_back(vector<string>());
		vvsRow.back().emplace_back(tnameMap);
		jbufSQL.pushHard(vvsRow);

		// Push the frame (TLBR) coordinates of the map.
		vvsRow.clear();
		vvsRow.resize(4, vector<string>());
		vvsRow[0] = vsColTitleMapFrame;
		pos1 = txtPath.find('[');
		pos2 = txtPath.find(',', pos1 + 2);
		vvsRow[1].emplace_back(txtPath.substr(pos1 + 2, pos2 - pos1 - 2));
		pos1 = pos2 + 1;
		pos2 = txtPath.find(')', pos1);
		vvsRow[1].emplace_back(txtPath.substr(pos1, pos2 - pos1));
		pos1 = txtPath.find_first_of("0123456789", pos2 + 1);
		pos2 = txtPath.find(',', pos1 + 1);
		vvsRow[2].emplace_back(txtPath.substr(pos1, pos2 - pos1));
		pos1 = pos2 + 1;
		pos2 = txtPath.find(')', pos1);
		vvsRow[2].emplace_back(txtPath.substr(pos1, pos2 - pos1));
		vvsRow[3].emplace_back(tnameMapFrame);
		jbufSQL.pushHard(vvsRow);

		// Recursively carry on with grandchildren.
		insertMapWorker(jbufSQL, jnChild.ID, parentDir + "/" + jnChild.vsData[0], stmtMap, stmtMapFrame, vsColTitleMap, vsColTitleMapFrame);
	}
	return 1;
}
void SCDAmap::makeBorderMap(vector<pair<double, double>>& vCoord, string filePath)
{
	// Open a geographic region's shaded area map and make a new map which is hollow.
	// Return the list of border coordinates, converted to fauxGPS.
	if (!jfile.exist(filePath)) { return; }
	string outputPath = filePath;
	size_t pos1 = outputPath.rfind(".png"), pos2, pos3, pos4;
	if (pos1 > outputPath.size()) { err("Missing png extension-makeBorderMap"); }
	outputPath.insert(pos1, " [border]");

	double dx, dy;
	vector<unsigned char> data;
	vector<int> spec;
	vector<pair<int, int>> vOutline;
	set<pair<int, int>> setSolid;
	pair<int, int> TL = make_pair(0, 0), BR;
	pair<double, double> gpsTL, gpsBR;
	vector<unsigned char> rgbaColour{0, 0, 255, 51}, rgbaNull{ 0, 0, 0, 0 };

	// Load the shaded region map, and ensure it does not touch the outer frame.
	pngf.load(filePath, data, spec);
	BR = make_pair(spec[0] - 1, spec[1] - 1);
	jpixel.rectangle(vOutline, spec[0], spec[1]);
	pngf.paintPixel(vOutline, data, spec, rgbaNull);

	// Make a map of the border outline, for reference and/or debugging.
	vOutline.clear();
	pngf.scanRect(setSolid, data, spec, rgbaNull, TL, BR, -1);
	jpixel.solidToOutline(vOutline, setSolid);
	pngf.blankCanvas(data, spec, rgbaNull);
	pngf.paintPixel(vOutline, data, spec, rgbaColour);
	pngf.print(outputPath, data, spec);

	// Obtain the map's TLBR coordinates.
	pos1 = filePath.find('[');
	pos1 = filePath.find_first_of("0123456789", pos1 + 1);
	pos2 = filePath.find(',', pos1 + 1);
	pos3 = filePath.find_first_of("0123456789", pos2 + 1);
	pos4 = filePath.find(')', pos3 + 1);
	try {
		dx = stod(filePath.substr(pos1, pos2 - pos1));
		dy = stod(filePath.substr(pos3, pos4 - pos3));
		gpsTL = make_pair(dx, dy);
	}
	catch (invalid_argument) { err("gpsTL stod-makeBorderMap"); }
	pos1 = filePath.find_first_of("0123456789", pos4 + 1);
	pos2 = filePath.find(',', pos1 + 1);
	pos3 = filePath.find_first_of("0123456789", pos2 + 1);
	pos4 = filePath.find(')', pos3 + 1);
	try {
		dx = stod(filePath.substr(pos1, pos2 - pos1));
		dy = stod(filePath.substr(pos3, pos4 - pos3));
		gpsBR = make_pair(dx, dy);
	}
	catch (invalid_argument) { err("gpsBR stod-makeBorderMap"); }

	// Convert the pixel coordinates to fauxGPS.
	vCoord.clear();
	int numPixel = (int)vOutline.size();
	double xGPSPix = (get<0>(gpsBR) - get<0>(gpsTL)) / (double)spec[0];
	double yGPSPix = (get<1>(gpsTL) - get<1>(gpsBR)) / (double)spec[1];
	for (int ii = 0; ii < numPixel; ii++) {
		dx = (double)get<0>(vOutline[ii]) * xGPSPix;
		dy = (double)get<1>(vOutline[ii]) * yGPSPix;
		vCoord.emplace_back(make_pair(get<0>(gpsTL) + dx, get<1>(gpsTL) - dy));
	}
}
void SCDAmap::nodeDoubleClicked(const QModelIndex& qmIndex, int indexTree)
{
	QJTREEMODEL* qjtm = modelCataMap.get();
	vector<string> vsGenealogy = qjtm->getGenealogy(qmIndex);
	int numGenealogy = (int)vsGenealogy.size();
	if (numGenealogy < 3) { return; }

	QJTREEITEM* qjtiNode = qjtm->getNode(qmIndex);
	QVariant qVar = qjtiNode->data(1);  // GEO_CODE
	QString qsTemp = qVar.toString();

	emit sendDisplayMap(vsGenealogy[0], vsGenealogy[1], qsTemp.toStdString());
}
void SCDAmap::nodeRightClicked(const QPoint& globalPos, const QModelIndex& qmIndex, int indexTree)
{
	QMenu menu(this);
	QJTREEMODEL* qjtm = modelCataMap.get();
	vector<string> vsGenealogy = qjtm->getGenealogy(qmIndex);
	int numGenealogy = (int)vsGenealogy.size();
	if (numGenealogy < 2) { return; }
	string sData = "";
	for (int ii = 0; ii < vsGenealogy.size(); ii++) {
		sData += "$" + vsGenealogy[ii];
	}
	
	auto loadTree = qaLoadTree.get();
	loadTree->setData(sData.c_str());
	menu.addAction(loadTree);

	QJTREEITEM* qjtiNode = qjtm->getNode(qmIndex);
	int numChildren = qjtiNode->getNumChildren();
	if (numGenealogy > 2 || numChildren > 0) {
		// Offer the insert option only if a geo tree has already been loaded.
		auto insertMap = qaInsertMap.get();
		insertMap->setData(sData.c_str());
		menu.addAction(insertMap);
	}

	if (numGenealogy > 2) {
		// A Geo region node was right-clicked.
		QVariant qVar = qjtiNode->dataUserRole(Qt::UserRole + 0);  // Background
		QString qsTemp = qVar.toString();
		string sBG = qsTemp.toStdString();
		if (sBG == get<0>(itemColourFail) || sBG == get<0>(itemColourWarning)) {
			qVar = qjtiNode->data(1);  // GEO_CODE
			qsTemp = qVar.toString();
			if (qsTemp.size() == 0) { err("Geo tree node is missing its GEO_CODE-nodeRightClicked"); }
			sData += "$" + qsTemp.toStdString();

			qVar = qjtiNode->data(2);  // GeoLayer
			qsTemp = qVar.toString();
			if (qsTemp.size() == 0) { err("Geo tree node is missing its GeoLayer-nodeRightClicked"); }
			sData += "$" + qsTemp.toStdString();

			auto downloadMap = qaDownloadMap.get();
			downloadMap->setData(sData.c_str());
			menu.addAction(downloadMap);
		}
	}

	menu.exec(globalPos);
}
void SCDAmap::shadeToBorder(string filePath, vector<unsigned char> rgbxBG)
{
	// For a given shaded map region and background, save a copy of that map as a border only.
	size_t pos1 = filePath.rfind(".png");
	if (pos1 > filePath.size() || !jfile.exist(filePath)) { err("Invalid input file-shadeToBorder"); }
	string outputPath = filePath;
	outputPath.insert(pos1, "[border]");

	vector<unsigned char> data;
	vector<int> spec;
	pngf.load(filePath, data, spec);

	vector<unsigned char> colour;
	pair<int, int> TL = make_pair(0, 0), BR = make_pair(spec[0] - 1, spec[1] - 1);
	set<pair<int, int>> solid;
	pngf.scanRect(solid, data, spec, rgbxBG, TL, BR, -1);

	vector<pair<int, int>> outline, segment;
	jpixel.solidToOutline(outline, solid);
	pngf.getPixel(colour, outline[0], data, spec);
	pngf.blankCanvas(data, spec, rgbxBG);
	pngf.paintPixel(outline, data, spec, colour);
	pngf.print(outputPath, data, spec);
}
