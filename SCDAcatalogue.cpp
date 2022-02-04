#include "SCDAcatalogue.h"

using namespace std;

void SCDAcatalogue::displayOnlineCata(SWITCHBOARD& sbgui, SCDAcatalogue*& cata, SConline& sco)
{
	thread::id myid = this_thread::get_id();
	vector<int> mycomm;
	sbgui.answerCall(myid, mycomm);

	QJTREEMODEL* qjtm = cata->modelStatscan.get();

	sco.getCataTree(qjtm->jt);
	qjtm->jt.compare(cata->modelDatabase->jt);
	vector<int> viNoTwin = qjtm->jt.hasTwinList(0);
	qjtm->jt.setNodeColour(viNoTwin, itemColourFail, itemColourSelected);
	qjtm->populate(qjtm->tree::jtree); 

	sbgui.endCall(myid);
}
void SCDAcatalogue::downloadCata()
{
	QVariant qVar = qaDownload->data();
	QString qsTemp = qVar.toString();
	wstring wsTemp = qsTemp.toStdWString();
	string prompt;
	jparse.utf16To8(prompt, wsTemp);
	emit sendDownloadCata(prompt);
}
void SCDAcatalogue::err(string message)
{
	string errorMessage = "SCDAcatalogue error:\n" + message;
	JLOG::getInstance()->err(errorMessage);
}
void SCDAcatalogue::getConfigXML(string configXML)
{
	initItemColour(configXML);
}
QJTREEMODEL* SCDAcatalogue::getModel(int indexTree)
{
	switch (indexTree) {
	case 0:
		return modelStatscan.get();
	case 1:
		return modelLocal.get();
	case 2:
		return modelDatabase.get();
	}
	return nullptr;
}
void SCDAcatalogue::init()
{
	vector<string> vsHeader = {""};

	QGridLayout* gLayout = new QGridLayout;
	this->setLayout(gLayout);

	modelStatscan = make_shared<QJTREEMODEL>(vsHeader, this);
	modelStatscan->jt.setNodeColour(-1, itemColourDefault, itemColourSelected);
	QLabel* label = new QLabel("Online Catalogues");
	gLayout->addWidget(label, 0, index::Statscan);
	QJTREEVIEW* treeStatscan = new QJTREEVIEW;
	treeStatscan->indexTree = index::Statscan;
	gLayout->addWidget(treeStatscan, 1, index::Statscan);
	connect(treeStatscan, &QJTREEVIEW::nodeRightClicked, this, &SCDAcatalogue::nodeRightClicked);

	modelLocal = make_shared<QJTREEMODEL>(vsHeader, this);
	modelLocal->jt.setNodeColour(-1, itemColourDefault, itemColourSelected);
	label = new QLabel("Local Catalogues");
	gLayout->addWidget(label, 0, index::Local);
	QJTREEVIEW* treeLocal = new QJTREEVIEW;
	treeLocal->indexTree = index::Local;
	gLayout->addWidget(treeLocal, 1, index::Local);
	connect(treeLocal, &QJTREEVIEW::nodeRightClicked, this, &SCDAcatalogue::nodeRightClicked);

	modelDatabase = make_shared<QJTREEMODEL>(vsHeader, this);
	modelDatabase->jt.setNodeColour(-1, itemColourDefault, itemColourSelected);
	label = new QLabel("Database Catalogues");
	gLayout->addWidget(label, 0, index::Database);
	QJTREEVIEW* treeDatabase = new QJTREEVIEW;
	treeDatabase->indexTree = index::Database;
	treeDatabase->setModel(modelDatabase.get());
	gLayout->addWidget(treeDatabase, 1, index::Database);
	connect(treeDatabase, &QJTREEVIEW::nodeRightClicked, this, &SCDAcatalogue::nodeRightClicked);

	initAction();
}
void SCDAcatalogue::initAction()
{
	qaDownload = new QAction("Download", this);
	connect(qaDownload, &QAction::triggered, this, &SCDAcatalogue::downloadCata);
	qaInsert = new QAction("Insert", this);
	connect(qaInsert, &QAction::triggered, this, &SCDAcatalogue::insertCata);
	qaSearch = new QAction("Search", this);
	connect(qaSearch, &QAction::triggered, this, &SCDAcatalogue::searchCata);
	qaTemplate = new QAction("Set GeoTree Templates", this);
	connect(qaTemplate, &QAction::triggered, this, &SCDAcatalogue::setGeoTreeTemplate);
}
void SCDAcatalogue::initItemColour(string& configXML)
{
	int indexBG = -1, indexFG = -1;
	vector<string> vsTag = { "colour", "solid", "item_default" };
	vector<vector<string>> vvsTag = jparse.getXML(configXML, vsTag);
	for (int ii = 0; ii < vvsTag.size(); ii++) {
		if (vvsTag[ii][0] == "background") { indexBG = ii; }
		else if (vvsTag[ii][0] == "foreground") { indexFG = ii;	}
	}
	itemColourDefault = make_pair(vvsTag[indexBG][1], vvsTag[indexFG][1]);
	
	vsTag = { "colour", "solid", "item_fail" };
	vvsTag = jparse.getXML(configXML, vsTag);
	for (int ii = 0; ii < vvsTag.size(); ii++) {
		if (vvsTag[ii][0] == "background") { indexBG = ii; }
		else if (vvsTag[ii][0] == "foreground") { indexFG = ii; }
	}
	itemColourFail = make_pair(vvsTag[indexBG][1], vvsTag[indexFG][1]);
	
	vsTag = { "colour", "solid", "item_selected" };
	vvsTag = jparse.getXML(configXML, vsTag);
	for (int ii = 0; ii < vvsTag.size(); ii++) {
		if (vvsTag[ii][0] == "background") { indexBG = ii; }
		else if (vvsTag[ii][0] == "foreground") { indexFG = ii; }
	}
	itemColourSelected = make_pair(vvsTag[indexBG][1], vvsTag[indexFG][1]);

	vsTag = { "colour", "solid", "item_warning" };
	vvsTag = jparse.getXML(configXML, vsTag);
	for (int ii = 0; ii < vvsTag.size(); ii++) {
		if (vvsTag[ii][0] == "background") { indexBG = ii; }
		else if (vvsTag[ii][0] == "foreground") { indexFG = ii; }
	}
	itemColourWarning = make_pair(vvsTag[indexBG][1], vvsTag[indexFG][1]);
}
void SCDAcatalogue::insertCata()
{
	QVariant qVar = qaInsert->data();
	QString qsTemp = qVar.toString();
	wstring wsTemp = qsTemp.toStdWString();
	string prompt;
	jparse.utf16To8(prompt, wsTemp);
	emit sendInsertCata(prompt);
}
void SCDAcatalogue::nodeClicked(const QModelIndex& qmIndex, int indexTree)
{
	//
}
void SCDAcatalogue::nodeRightClicked(const QPoint& globalPos, const QModelIndex& qmIndex, int indexTree)
{
	QMenu menu(this);
	QJTREEMODEL* qjtm = getModel(indexTree);
	vector<string> vsGenealogy = qjtm->getGenealogy(qmIndex);
	int numNode = (int)vsGenealogy.size();
	string sData = "";
	switch (indexTree) {
	case index::Statscan:
	{
		for (int ii = 0; ii < numNode; ii++) {
			sData += "$" + vsGenealogy[ii];
		}
		qaDownload->setData(sData.c_str());
		menu.addAction(qaDownload);
		break;
	}
	case index::Local:
	{
		// Permit catalogue insertion only if GeoTree templates have been set.
		QModelIndex qmiNode = qmIndex;
		if (numNode > 1) {			
			QModelIndex qmiParent;
			for (int ii = 0; ii < numNode - 1; ii++) {
				qmiParent = qjtm->parent(qmiNode);
				qmiNode = qmiParent;
			}			
		}
		QJTREEITEM* qjtiNode = qjtm->getNode(qmiNode);
		int numCol = qjtiNode->getNumCol();
		if (numCol >= 3) {
			for (int ii = 0; ii < numNode; ii++) {
				sData += "$" + vsGenealogy[ii];
			}
			qaInsert->setData(sData.c_str());
			menu.addAction(qaInsert);
		}
		else if (numCol == 2) {
			QVariant qvPath = qjtiNode->data(1);
			qaTemplate->setData(qvPath);
			menu.addAction(qaTemplate);
		}
		break;
	}
	case index::Database:
	{
		for (int ii = 0; ii < numNode; ii++) {
			sData += "$" + vsGenealogy[ii];
		}
		qaSearch->setData(sData.c_str());
		menu.addAction(qaSearch);
		break;
	}
	}
	menu.exec(globalPos);
}
void SCDAcatalogue::resetModel(int indexTree)
{
	QGridLayout* gLayout = (QGridLayout*)this->layout();
	QLayoutItem* qlItem = nullptr;
	QJTREEVIEW* qjTree = nullptr;
	vector<string> vsHeader = { "" };
	auto model = make_shared<QJTREEMODEL>(vsHeader, this);
	switch (indexTree) {
	case 0:
		modelStatscan.swap(model);
		modelStatscan->jt.setNodeColour(-1, itemColourDefault, itemColourSelected);
		break;
	case 1:
		modelLocal.swap(model);
		modelLocal->jt.setNodeColour(-1, itemColourDefault, itemColourSelected);
		break;
	case 2:
		modelDatabase.swap(model);
		modelDatabase->jt.setNodeColour(-1, itemColourDefault, itemColourSelected);
		break;
	}
}
void SCDAcatalogue::scanLocal(SWITCHBOARD& sbgui, SCDAcatalogue*& cata, string& configXML)
{
	// Search a local drive for census year folders, and then for catalogue
	// folders. Each census year has a list of files needed to represent
	// that year's dataset - if these can be found, then add that catalogue
	// to the tree indicating its presence within local storage. 
	thread::id myid = this_thread::get_id();
	vector<int> mycomm;
	sbgui.answerCall(myid, mycomm);
	vector<string> prompt;
	sbgui.getPrompt(prompt);
	string cataPath, filePath, search = "*", yearPath;
	vector<string> folderList = wf.getFolderList(prompt[0], search);
	vector<string> cataList, sYearList;
	size_t pos1;
	QJTREEMODEL* qjtm = cata->getModel(cata->index::Local);
	JNODE jnRoot = qjtm->jt.getRoot();
	int iYear, numCata, rootID = jnRoot.ID;

	// Create the first generation of tree nodes with census year folders.
	for (int ii = 0; ii < folderList.size(); ii++) {
		pos1 = folderList[ii].find_first_not_of("1234567890");
		if (pos1 > folderList[ii].size()) {
			try { iYear = stoi(folderList[ii]); }
			catch (invalid_argument) { err("stoi-scanLocal"); }
			if (iYear >= 1981 && iYear <= 2017) {
				sYearList.push_back(folderList[ii]);
			}
		}
	}
	jsort.integerList(sYearList, JSORT::Increasing);
	int numYear = (int)sYearList.size();
	if (numYear < 1) { 
		mycomm[0] = 1;
		sbgui.update(myid, mycomm);
		return; 
	}
	for (int ii = 0; ii < numYear; ii++) {
		JNODE jnYear;
		jnYear.vsData[0] = sYearList[ii];
		filePath = prompt[0] + sYearList[ii] + "/GeoTree_" + sYearList[ii] + ".txt";
		if (jfile.exist(filePath)) { jnYear.vsData.push_back(filePath); }
		filePath = prompt[0] + sYearList[ii] + "/GeoTreeTemplate_" + sYearList[ii] + ".txt";
		if (jfile.exist(filePath)) { jnYear.vsData.push_back(filePath); }
		qjtm->jt.addChild(rootID, jnYear);
	}

	vector<int> viYearID(numYear);
	vector<reference_wrapper<JNODE>> vJNYear = qjtm->jt.getChildren(rootID);
	for (int ii = 0; ii < numYear; ii++) {
		viYearID[ii] = vJNYear[ii].get().ID;
	}

	// Make a tree node for each folder containing a catalogue ZIP archive.
	string replace = "[cata]";
	vector<string> vsTag;
	vector<vector<string>> vvsTag;
	for (int ii = 0; ii < numYear; ii++) {
		yearPath = prompt[0] + "/" + sYearList[ii];
		folderList = wf.getFolderList(yearPath, search);
		numCata = (int)folderList.size();
		for (int jj = 0; jj < numCata; jj++) {
			filePath = yearPath + "/" + folderList[jj] + "/" + folderList[jj] + ".zip";
			if (jfile.exist(filePath)) {
				JNODE jn;
				jn.vsData[0] = folderList[jj];
				qjtm->jt.addChild(viYearID[ii], jn);
			}
		}
	}

	// If a catalogue is present within local storage but is not present
	// within the database, highlight it. 
	qjtm->jt.compare(cata->modelDatabase->jt);
	vector<int> viNoTwin = qjtm->jt.hasTwinList(0);
	qjtm->jt.setNodeColour(viNoTwin, itemColourWarning, itemColourSelected);

	sbgui.endCall(myid);
}
void SCDAcatalogue::searchCata()
{
	QVariant qVar = qaSearch->data();
	QString qsTemp = qVar.toString();
	wstring wsTemp = qsTemp.toStdWString();
	string prompt;
	jparse.utf16To8(prompt, wsTemp);
	prompt.insert(0, "*");
	prompt.push_back('*');
	vector<string> dirt = { "@" }, soap = { "$" };
	jparse.clean(prompt, dirt, soap);
	emit sendSearchCata(prompt);
}
void SCDAcatalogue::setGeoTreeTemplate()
{
	// Parse a census year's GeoTree file for GeoTrees lacking a catalogue template.
	// Create a dialog for the user to specify template choices.
	QVariant qVar = qaTemplate->data();
	QString qsTemp = qVar.toString();
	wstring wsTemp = qsTemp.toStdWString();
	string geoTreePath;
	jparse.utf16To8(geoTreePath, wsTemp);
	if (!jfile.exist(geoTreePath)) { 
		emit setTextIO(geoTreePath + " not found!\n");
		return;
	}
	string geoTreeFile, geoTreeTemplateFile;
	jfile.load(geoTreeFile, geoTreePath);

	// If some or all templates have already been saved, load them as initial values.
	size_t pos2, pos3;
	unordered_map<string, string> mapTemplate;
	string geoTreeTemplatePath = geoTreePath;
	size_t pos1 = geoTreeTemplatePath.rfind('_');
	geoTreeTemplatePath.insert(pos1, "Template");
	if (jfile.exist(geoTreeTemplatePath)) {
		jfile.load(geoTreeTemplateFile, geoTreeTemplatePath);
		pos1 = 0;
		pos3 = geoTreeTemplateFile.find("\n\n");
		while (pos3 < geoTreeTemplateFile.size()) {
			pos2 = geoTreeTemplateFile.rfind('\n', pos3 - 1);
			mapTemplate.emplace(geoTreeTemplateFile.substr(pos1, pos2 - pos1), geoTreeTemplateFile.substr(pos2 + 1, pos3 - pos2 - 1));
			pos1 = geoTreeTemplateFile.find_first_not_of('\n', pos3);
			pos3 = geoTreeTemplateFile.find("\n\n", pos3 + 2);
		}
	}

	// Assign each catalogue to a GeoTree.
	string cataList, sChosen;
	vector<string> vsGeoTree;
	vector<vector<string>> vvsCata;
	set<string> setCata;
	pos3 = geoTreeFile.find("\n\n");
	if (pos3 > geoTreeFile.size()) { err("Invalid GeoTree file-setGeoTreeTemplate"); }
	pos1 = 0;
	while (pos3 < geoTreeFile.size()) {
		pos2 = geoTreeFile.find('\n', pos1);
		vsGeoTree.emplace_back(geoTreeFile.substr(pos1, pos2 - pos1));
		cataList = geoTreeFile.substr(pos2 + 1, pos3 - pos2 - 1);
		vvsCata.push_back({});
		jparse.splitByMarker(vvsCata.back(), cataList, '\0');
		for (int ii = 0; ii < vvsCata.back().size(); ii++) {
			setCata.emplace(vvsCata.back()[ii]);
		}

		pos1 = geoTreeFile.find_first_not_of('\n', pos3);
		pos3 = geoTreeFile.find("\n\n", pos3 + 2);
	}

	QDialog* dialogGeoTree = new QDialog(this);
	pos2 = geoTreePath.rfind('.');
	pos1 = geoTreePath.rfind('_', pos2) + 1;
	string sTitle = "GeoTree: " + geoTreePath.substr(pos1, pos2 - pos1);
	dialogGeoTree->setWindowTitle(sTitle.c_str());
	dialogGeoTree->setWindowFlag(Qt::CustomizeWindowHint, 1);
	dialogGeoTree->setWindowFlag(Qt::WindowCloseButtonHint, 0);
	dialogGeoTree->setWindowFlag(Qt::WindowContextHelpButtonHint, 0);
	QGridLayout* gLayout = new QGridLayout;
	dialogGeoTree->setLayout(gLayout);
	QLabel* label = nullptr;
	QComboBox* qcb = nullptr;
	QLineEdit* qle = nullptr;
	QLayoutItem* qlItem = nullptr;
	unordered_map<string, string>::iterator it;
	int numGeoTree = (int)vsGeoTree.size();
	for (int ii = 0; ii < numGeoTree; ii++) {
		it = mapTemplate.find(vsGeoTree[ii]);
		if (it != mapTemplate.end()) { sChosen = it->second; }
		else { sChosen.clear(); }

		label = new QLabel(vsGeoTree[ii].c_str());
		gLayout->addWidget(label, ii, 0, Qt::AlignRight);
		qcb = new QComboBox;
		qcb->addItem("[None]");
		for (int jj = 0; jj < vvsCata[ii].size(); jj++) {
			qcb->addItem(vvsCata[ii][jj].c_str());
			if (vvsCata[ii][jj] == sChosen) {
				qcb->setCurrentIndex(jj);
			}
		}
		gLayout->addWidget(qcb, ii, 1, Qt::AlignLeft);
		qle = new QLineEdit;
		qle->setPlaceholderText("Override catalogue ...");
		if (qcb->currentIndex() == 0 && sChosen.size() > 0) {
			// Chosen catalogue template was manually entered (override).
			qle->setText(sChosen.c_str());
		}
		gLayout->addWidget(qle, ii, 2);
	}
	QHBoxLayout* hLayout = new QHBoxLayout;
	gLayout->addLayout(hLayout, numGeoTree, 2, Qt::AlignRight);
	hLayout->addStretch(1);
	QPushButton* pbSave = new QPushButton("Save");
	hLayout->addWidget(pbSave, 0);
	connect(pbSave, &QPushButton::clicked, dialogGeoTree, &QDialog::accept);
	QPushButton* pbCancel = new QPushButton("Cancel");
	hLayout->addWidget(pbCancel, 0);
	connect(pbCancel, &QPushButton::clicked, dialogGeoTree, &QDialog::reject);

	int result = dialogGeoTree->exec();
	if (result > 0) { 
		string sCata, temp;
		QString qsTemp;
		if (geoTreeTemplateFile.size() < 1) {
			for (int ii = 0; ii < numGeoTree; ii++) {
				geoTreeTemplateFile += vsGeoTree[ii] + "\n\n";
			}
		}

		for (int ii = 0; ii < numGeoTree; ii++) {
			sCata.clear();
			qlItem = gLayout->itemAtPosition(ii, 2);
			qle = (QLineEdit*)qlItem->widget();
			qsTemp = qle->text();
			if (qsTemp.size() > 0) {
				temp = qsTemp.toStdString();
				if (setCata.count(temp)) { sCata = temp; }
			}
			if (sCata.size() < 1) {
				qlItem = gLayout->itemAtPosition(ii, 1);
				qcb = (QComboBox*)qlItem->widget();
				qsTemp = qcb->currentText();
				sCata = qsTemp.toStdString();
			}

			pos1 = geoTreeTemplateFile.find(vsGeoTree[ii]);
			pos1 = geoTreeTemplateFile.find('\n', pos1);
			if (geoTreeTemplateFile[pos1 + 1] == '\n') {
				geoTreeTemplateFile.insert(pos1 + 1, sCata + "\n");
			}
			else {
				pos2 = geoTreeTemplateFile.find('\n', pos1 + 1);
				geoTreeTemplateFile.replace(pos1 + 1, pos2 - pos1 - 1, sCata);
			}
		}
		jfile.printer(geoTreePath, geoTreeFile);
	}
}
