#include "SCDAcatalogue.h"

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
	qjtm->populate(); 

	sbgui.endCall(myid);
}
void SCDAcatalogue::downloadCata()
{
	QVariant qVar = qaDownload->data();
	QString qsTemp = qVar.toString();
	string prompt = qsTemp.toUtf8();
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

	indexStatscan = 0;
	modelStatscan = make_shared<QJTREEMODEL>(vsHeader, this);
	modelStatscan->jt.setNodeColour(-1, itemColourDefault, itemColourSelected);
	QLabel* label = new QLabel("Online Catalogues");
	gLayout->addWidget(label, 0, indexStatscan);
	QJTREEVIEW* treeStatscan = new QJTREEVIEW;
	treeStatscan->indexTree = indexStatscan;
	gLayout->addWidget(treeStatscan, 1, indexStatscan);
	connect(treeStatscan, &QJTREEVIEW::nodeRightClicked, this, &SCDAcatalogue::nodeRightClicked);

	indexLocal = 1;
	modelLocal = make_shared<QJTREEMODEL>(vsHeader, this);
	modelLocal->jt.setNodeColour(-1, itemColourDefault, itemColourSelected);
	label = new QLabel("Local Catalogues");
	gLayout->addWidget(label, 0, indexLocal);
	QJTREEVIEW* treeLocal = new QJTREEVIEW;
	treeLocal->indexTree = indexLocal;
	gLayout->addWidget(treeLocal, 1, indexLocal);
	connect(treeLocal, &QJTREEVIEW::nodeRightClicked, this, &SCDAcatalogue::nodeRightClicked);

	indexDatabase = 2;
	modelDatabase = make_shared<QJTREEMODEL>(vsHeader, this);
	modelDatabase->jt.setNodeColour(-1, itemColourDefault, itemColourSelected);
	label = new QLabel("Database Catalogues");
	gLayout->addWidget(label, 0, indexDatabase);
	QJTREEVIEW* treeDatabase = new QJTREEVIEW;
	treeDatabase->setModel(modelDatabase.get());
	gLayout->addWidget(treeDatabase, 1, indexDatabase);
	connect(treeStatscan, &QJTREEVIEW::nodeRightClicked, this, &SCDAcatalogue::nodeRightClicked);

	initAction();
}
void SCDAcatalogue::initAction()
{
	qaDownload = new QAction("Download", this);
	connect(qaDownload, &QAction::triggered, this, &SCDAcatalogue::downloadCata);
	qaInsert = new QAction("Insert", this);
	connect(qaInsert, &QAction::triggered, this, &SCDAcatalogue::insertCata);
}
void SCDAcatalogue::initItemColour(string& configXML)
{
	int indexBG = -1, indexFG = -1;
	vector<string> vsTag = { "colour", "solid", "item_default" };
	vector<vector<string>> vvsTag = jf.getXML(configXML, vsTag);
	for (int ii = 0; ii < vvsTag.size(); ii++) {
		if (vvsTag[ii][0] == "background") { indexBG = ii; }
		else if (vvsTag[ii][0] == "foreground") { indexFG = ii;	}
	}
	itemColourDefault = make_pair(vvsTag[indexBG][1], vvsTag[indexFG][1]);
	
	vsTag = { "colour", "solid", "item_fail" };
	vvsTag = jf.getXML(configXML, vsTag);
	for (int ii = 0; ii < vvsTag.size(); ii++) {
		if (vvsTag[ii][0] == "background") { indexBG = ii; }
		else if (vvsTag[ii][0] == "foreground") { indexFG = ii; }
	}
	itemColourFail = make_pair(vvsTag[indexBG][1], vvsTag[indexFG][1]);
	
	vsTag = { "colour", "solid", "item_selected" };
	vvsTag = jf.getXML(configXML, vsTag);
	for (int ii = 0; ii < vvsTag.size(); ii++) {
		if (vvsTag[ii][0] == "background") { indexBG = ii; }
		else if (vvsTag[ii][0] == "foreground") { indexFG = ii; }
	}
	itemColourSelected = make_pair(vvsTag[indexBG][1], vvsTag[indexFG][1]);

	vsTag = { "colour", "solid", "item_warning" };
	vvsTag = jf.getXML(configXML, vsTag);
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
	string prompt = qsTemp.toUtf8();
	emit sendInsertCata(prompt);
}
void SCDAcatalogue::nodeClicked(const QModelIndex& qmIndex, int indexTree)
{
	int bbq = 1;
}
void SCDAcatalogue::nodeRightClicked(const QPoint& globalPos, const QModelIndex& qmIndex, int indexTree)
{
	QMenu menu(this);
	QJTREEMODEL* qjtm = getModel(indexTree);
	vector<string> vsGenealogy = qjtm->getGenealogy(qmIndex);
	int numNode = (int)vsGenealogy.size();
	switch (indexTree) {
	case 0:
	{   // Database
		string sData = "";
		for (int ii = 0; ii < numNode; ii++) {
			sData += "@" + vsGenealogy[ii];
		}
		qaDownload->setData(sData.c_str());
		menu.addAction(qaDownload);
		break;
	}
	case 1:
	{   // Local
		string sData = "";
		for (int ii = 0; ii < numNode; ii++) {
			sData += "@" + vsGenealogy[ii];
		}
		qaInsert->setData(sData.c_str());
		menu.addAction(qaInsert);
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
		break;
	case 1:
		modelLocal.swap(model);
		break;
	case 2:
		modelDatabase.swap(model);
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
	QJTREEMODEL* qjtm = cata->getModel(cata->indexLocal);
	JNODE jnRoot = qjtm->jt.getRoot();
	int iYear, numCata, numMissing, rootID = jnRoot.ID;

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
	jf.sortInteger(sYearList, JFUNC::Increasing);
	int numYear = (int)sYearList.size();
	if (numYear < 1) { 
		mycomm[0] = 1;
		sbgui.update(myid, mycomm);
		return; 
	}
	for (int ii = 0; ii < numYear; ii++) {
		JNODE jnYear;
		jnYear.vsData.push_back(sYearList[ii]);
		qjtm->jt.addChild(rootID, jnYear);
	}

	vector<int> viYearID(numYear);
	vector<reference_wrapper<JNODE>> vJNYear = qjtm->jt.getChildren(rootID);
	for (int ii = 0; ii < numYear; ii++) {
		viYearID[ii] = vJNYear[ii].get().ID;
	}

	// Load the list of files necessary to represent a catalogue within a 
	// census year, then determine if those files are present. 
	string replace = "[cata]";
	vector<string> vsTag;
	vector<vector<string>> vvsTag;
	for (int ii = 0; ii < numYear; ii++) {
		vsTag = { "file_name", sYearList[ii] };
		vvsTag = jf.getXML(configXML, vsTag);

		yearPath = prompt[0] + "/" + sYearList[ii];
		folderList = wf.getFolderList(yearPath, search);
		numCata = (int)folderList.size();
		for (int jj = 0; jj < numCata; jj++) {
			numMissing = 0;
			cataPath = yearPath + "/" + folderList[jj];
			for (int kk = 0; kk < vvsTag.size(); kk++) {
				filePath = cataPath + "/" + vvsTag[kk][1];
				pos1 = filePath.rfind(replace);
				if (pos1 < filePath.size()) {
					filePath.replace(pos1, replace.size(), folderList[jj]);
				}
				if (!wf.file_exist(filePath)) {
					numMissing++;
					break;
				}
			}
			if (numMissing == 0) {
				JNODE jn;
				jn.vsData.push_back(folderList[jj]);
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
