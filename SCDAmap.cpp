#include "SCDAmap.h"

using namespace std;

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
void SCDAmap::makeBorderMap(vector<pair<int, int>>& vOutline, string filePath)
{
	// Open a geographic region's shaded area map and make a new map which is hollow.
	// Return the list of border coordinates.
	if (!jfile.exist(filePath)) { return; }
	string outputPath = filePath;
	size_t pos1 = outputPath.rfind(".png");
	if (pos1 > outputPath.size()) { err("Missing png extension-makeBorderMap"); }
	outputPath.insert(pos1, " [border]");

	vector<unsigned char> data;
	vector<int> spec;
	set<pair<int, int>> setSolid;
	pair<int, int> TL = make_pair(0, 0), BR;
	vector<unsigned char> rgbaColour{0, 0, 255, 51}, rgbaNull{ 0, 0, 0, 0 };

	pngf.load(filePath, data, spec);
	BR = make_pair(spec[0] - 1, spec[1] - 1);
	pngf.scanRect(setSolid, data, spec, rgbaNull, TL, BR, -1);
	jpixel.solidToOutline(vOutline, setSolid);
	pngf.blankCanvas(data, spec, rgbaNull);
	pngf.paintPixel(vOutline, data, spec, rgbaColour);
	pngf.print(outputPath, data, spec);
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
