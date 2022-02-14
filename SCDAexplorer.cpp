#include "SCDAexplorer.h"

using namespace std;

SCDA::SCDA(string execFolder, QWidget* parent)
	: QMainWindow(parent), sExecFolder(execFolder)
{
	setWindowTitle("SCDA Explorer");

	initConfig();
	initGUI();

	this->setWindowState(Qt::WindowMaximized);
}

void SCDA::busyScreen(bool onOff)
{
	QJBUSY* dialogBusy = this->findChild<QJBUSY*>("Busy", Qt::FindDirectChildrenOnly);
	if (onOff) { dialogBusy->show(); }
	else { dialogBusy->hide(); }
}
void SCDA::busyWheel(SWITCHBOARD& sb)
{
	QJBUSY* dialogBusy = this->findChild<QJBUSY*>("Busy", Qt::FindDirectChildrenOnly);
	dialogBusy->show();
	dialogBusy->busy(sb);
	dialogBusy->hide();
}
void SCDA::debug()
{
	size_t pos1;
	string mapFolder, mapName, mapNameCore, mapSearch, skipped;
	string tname = "Geo$2013$99-010-X2011026";
	vector<string> search = { "*" }, vsFilePath;
	vector<vector<string>> vvsGeo;
	int numRegion = scdb.sf.select(search, tname, vvsGeo);
	string mapDir = "E:/maps";
	for (int ii = 0; ii < numRegion; ii++) {
		mapSearch = mapDir + "/Level" + vvsGeo[ii][2];
		if (vvsGeo[ii][2] == "0") { mapSearch += "/canada/"; }
		else if (vvsGeo[ii][2] == "1") { mapSearch += "/province/"; }
		else if (vvsGeo[ii][2] == "2") { mapSearch += "/cmaca/"; }
		mapSearch += vvsGeo[ii][1] + "*.png";
		vsFilePath.clear();
		jfile.search(vsFilePath, mapSearch);
		if (vsFilePath.size() < 1) {
			skipped += vvsGeo[ii][1] + "\n";
			continue;
		}
		vsFilePath.emplace_back(vsFilePath[0]);
		pos1 = vsFilePath[0].find_last_of("/\\");
		mapName = vsFilePath[0].substr(pos1 + 1);
		pos1 = mapName.find('[');
		mapNameCore = mapName.substr(0, pos1 - 1);

		if (vvsGeo[ii].size() < 4) {
			vsFilePath[1] = mapDir + "/" + mapName;
			mapFolder = mapDir + "/" + mapNameCore;
		}
		else {
			mapFolder = mapDir;
			for (int jj = 3; jj < vvsGeo[ii].size(); jj++) {
				for (int kk = 0; kk < numRegion; kk++) {
					if (vvsGeo[kk][0] == vvsGeo[ii][jj]) {
						mapFolder += "/" + vvsGeo[kk][1];
						break;
					}
					else if (kk == numRegion - 1) { err("Failed to find ancestor's row"); }
				}
			}

			vsFilePath[1] = mapFolder + "/" + mapName;
			mapFolder += "/" + mapNameCore;
		}
		wf.makeDir(mapFolder);
		jfile.rename(vsFilePath[0], vsFilePath[1]);
	}
	emit appendTextIO(skipped);

	emit appendTextIO("Done!");
}
void SCDA::deleteTable(string tname)
{
	thread::id myid = this_thread::get_id();
	sb.startCall(myid, 3);

	// Determine if tname is singular, or a list of table names.
	vector<string> vsTname;
	if (tname[0] == '@' || tname[0] == '|') {
		jparse.splitByMarker(vsTname, tname);
		sb.pushWork(vsTname);
	}
	else { sb.pushWork(tname); }
	std::jthread thr(&SCdatabase::deleteTable, scdb, ref(sb));
	busyWheel(sb);
	sb.endCall(myid);

	QWidget* central = this->centralWidget();
	QHBoxLayout* hLayout = (QHBoxLayout*)central->layout();
	QLayoutItem* qlItem = hLayout->itemAt(indexH::Control);
	SCDAcontrol* control = (SCDAcontrol*)qlItem->widget();
	if (control->sLastQuery.size() > 0) { searchDBTable(control->sLastQuery); }
}
void SCDA::deleteTableRow(string tnameRow)
{
	vector<string> vsCell, vsTnameRow;
	jparse.splitByMarker(vsTnameRow, tnameRow);
	jparse.splitByMarker(vsCell, vsTnameRow[1]);
	scdb.deleteTableRow(vsTnameRow[0], vsCell);
	fetchDBTable(vsTnameRow[0]);
}
void SCDA::dialogStructureStart()
{
	QString qsPath = QFileDialog::getOpenFileName(this, "Open File", sLocalStorage.c_str(), "XML files (*.xml)");
	if (qsPath.size() < 1) { return; }
	string sPath = qsPath.toStdString();

	QWidget* central = this->centralWidget();
	QHBoxLayout* hLayout = (QHBoxLayout*)central->layout();
	QLayoutItem* qlItem = hLayout->itemAt(indexH::Display);
	QVBoxLayout* vLayout = (QVBoxLayout*)qlItem->layout();
	qlItem = vLayout->itemAt(0);
	QTabWidget* tab = (QTabWidget*)qlItem->widget();
	tab->setCurrentIndex(indexTab::Structure);
	SCDAstructure* structure = (SCDAstructure*)tab->widget(indexTab::Structure);
	structure->loadXML(sPath);
}
void SCDA::displayOnlineCata()
{
	QWidget* central = this->centralWidget();
	QHBoxLayout* hLayout = (QHBoxLayout*)central->layout();
	QLayoutItem* qlItem = hLayout->itemAt(indexH::Display);
	QVBoxLayout* vLayout = (QVBoxLayout*)qlItem->layout();
	qlItem = vLayout->itemAt(0);
	QTabWidget* tab = (QTabWidget*)qlItem->widget();
	tab->setCurrentIndex(indexTab::Catalogue);
	SCDAcatalogue* cata = (SCDAcatalogue*)tab->widget(indexTab::Catalogue);
	cata->resetModel(cata->index::Statscan);

	thread::id myid = this_thread::get_id();
	sb.startCall(myid, commLength);
	std::thread thr(&SCDAcatalogue::displayOnlineCata, cata, ref(sb), ref(cata), ref(sco));
	thr.detach();
	busyWheel(sb);
	sb.endCall(myid);

	QJTREEMODEL* model = cata->modelStatscan.get();
	QGridLayout* gLayout = (QGridLayout*)cata->layout();
	qlItem = gLayout->itemAtPosition(1, cata->index::Statscan);
	QJTREEVIEW* treeStatscan = (QJTREEVIEW*)qlItem->widget();
	treeStatscan->setModel(model);
	int numGen = model->jt.getExpandGeneration(30);
	treeStatscan->expandRecursively(QModelIndex(), numGen);
}
void SCDA::downloadCata(string prompt)
{
	// Prompt should have form (@year@cata0@cata1...). If the prompt
	// contains only the year, then all catalogues for that year are 
	// downloaded. 
	vector<string> vsPrompt;
	jparse.splitByMarker(vsPrompt, prompt, prompt[0]);
	int numCata = (int)vsPrompt.size() - 1;
	if (numCata < 0) { err("Invalid prompt-downloadCata"); }
	else if (numCata == 0) {
		QWidget* central = this->centralWidget();
		QHBoxLayout* hLayout = (QHBoxLayout*)central->layout();
		QLayoutItem* qlItem = hLayout->itemAt(indexH::Display);
		QVBoxLayout* vLayout = (QVBoxLayout*)qlItem->layout();
		qlItem = vLayout->itemAt(0);
		QTabWidget* tab = (QTabWidget*)qlItem->widget();
		SCDAcatalogue* cata = (SCDAcatalogue*)tab->widget(indexTab::Catalogue);
		QJTREEMODEL* model = cata->getModel(cata->index::Statscan);
		vector<int> vID = model->jt.searchData(vsPrompt[0], 0);
		if (vID.size() != 1) { err("Failed to obtain year node-downloadCata"); }
		vector<reference_wrapper<JNODE>> vChildren = model->jt.getChildren(vID[0]);
		numCata = (int)vChildren.size();
		vsPrompt.resize(1 + numCata);
		int index = 0;
		for (JNODE& child : vChildren) {
			index++;
			vsPrompt[index] = child.vsData[0];
		}
	}

	// Initialize the progress bar.
	string sEnd = " of " + to_string(numCata) + ") ...";
	double dNumCata = (double)numCata;
	vector<string> vsProgress(numCata + 1);
	vector<double> vdProgress(numCata + 1);
	for (int ii = 0; ii < numCata; ii++) {
		vsProgress[ii] = "Downloading catalogues (" + to_string(ii + 1) + sEnd;
		vdProgress[ii] = (double)ii / dNumCata;
	}
	vsProgress.back() = "Finished downloading catalogues.";
	vdProgress.back() = 1.0;
	emit initProgress(vdProgress, vsProgress);

	// Launch a worker thread to download the catalogue(s) data files.
	thread::id myid = this_thread::get_id();
	sb.startCall(myid, commLength);
	sb.setPrompt(vsPrompt);
	std::thread thr(&SConline::downloadCata, sco, ref(sb));
	thr.detach();

	// Launch a worker thread to download the catalogue(s) geographic region maps, if necessary.
	SWITCHBOARD sbmap;


	busyWheel(sb);
	sb.endCall(myid);
	scanLocalCata();
}
void SCDA::err(string message)
{
	string errorMessage = "SCDA error:\n" + message;
	JLOG::getInstance()->err(errorMessage);
}
void SCDA::fetchDBTable(string tname)
{
	vector<vector<string>> vvsData, vvsColTitle;
	scdb.loadTable(vvsData, vvsColTitle, tname);
	if (vvsData.size() > 0 || vvsColTitle.size() > 0) {
		QWidget* central = this->centralWidget();
		QHBoxLayout* hLayout = (QHBoxLayout*)central->layout();
		QLayoutItem* qlItem = hLayout->itemAt(indexH::Display);
		QVBoxLayout* vLayout = (QVBoxLayout*)qlItem->layout();
		qlItem = vLayout->itemAt(0);
		QTabWidget* tab = (QTabWidget*)qlItem->widget();
		tab->setCurrentIndex(indexTab::Table);
		SCDAtable* table = (SCDAtable*)tab->widget(indexTab::Table);

		string title = "Table (" + tname + ")";
		table->displayTable(vvsData, vvsColTitle, title);
	}
}
QList<QRect> SCDA::getDesktop()
{
	QList<QScreen*> listScreen = qApp->screens();
	if (listScreen.size() < 1) { err("No screens found-getDesktop"); }
	QList<QRect> listGeometry;
	for (QScreen* screen : listScreen) {
		listGeometry.append(screen->geometry());
	}
	return listGeometry;
}
void SCDA::initBusy(QJBUSY*& dialogBusy)
{
	dialogBusy->setObjectName("Busy");

	vector<string> vsTag = { "path", "resource" };
	vector<vector<string>> vvsTag = jparse.getXML(configXML, vsTag);
	string busyFolder = vvsTag[0][1] + "/qjbusy";
	string busySearch = "BusyWheel*.png";
	vector<string> vsBusyWheel = wf.getFileList(busyFolder, busySearch);
	int numPNG = (int)vsBusyWheel.size();
	dialogBusy->init(busyFolder + "/" + busySearch, numPNG);
}
void SCDA::initConfig()
{
	string configPath = sExecFolder + "/SCDA_Explorer_Config.xml";
	if (!wf.file_exist(configPath)) {
		string backupPath = configPath;
		size_t pos2 = backupPath.rfind('/');
		size_t pos1 = backupPath.find_last_of("/\\", pos2 - 1);
		backupPath.erase(pos1, pos2 - pos1);
		if (!wf.file_exist(backupPath)) { err("XML config file not found-initConfig"); }
		wf.copyFile(backupPath, configPath);
	}
	jfile.load(configXML, configPath);

	vector<string> vsTag = { "path", "local_storage" };
	sLocalStorage = jparse.getXML1(configXML, vsTag);
	vsTag = { "settings", "cpu_cores" };
	string sCore = jparse.getXML1(configXML, vsTag);
	try { numCore = stoi(sCore); }
	catch (invalid_argument) { err("cpu_cores stoi-initConfig"); }

	vsTag = { "path", "css" };
	string cssPath = jparse.getXML1(configXML, vsTag);
	string cssFile;
	jfile.load(cssFile, cssPath);
	qApp->setStyleSheet(cssFile.c_str());

	scdb.init(configXML);
	sco.init(configXML);

	commLength = 4;
	sleepTime = 50;  // ms
}
void SCDA::initControl(SCDAcontrol*& control)
{
	QVBoxLayout* vLayout = (QVBoxLayout*)control->layout();

	QLayoutItem* qlItem = vLayout->itemAt(control->index::Drive);
	QHBoxLayout* hLayout = (QHBoxLayout*)qlItem->layout();
	int numItem = hLayout->count();
	qlItem = hLayout->itemAt(numItem - 1);
	QComboBox* cb = (QComboBox*)qlItem->widget();
	cb->addItem("");
	int activeIndex = 0;
	vector<string> vsTag = { "settings", "default_drive" };
	vector<vector<string>> vvsTag = jparse.getXML(configXML, vsTag);
	string defaultDrive = "";
	if (vvsTag.size() > 0) {
		defaultDrive = vvsTag[0][1];
	}
	vsTag = { "settings", "drive_type" };
	vvsTag = jparse.getXML(configXML, vsTag);
	set<string> setDriveType;
	for (int ii = 0; ii < vvsTag.size(); ii++) {
		if (vvsTag[ii][1] == "1" || vvsTag[ii][1] == "TRUE" || vvsTag[ii][1] == "true") {
			setDriveType.emplace(vvsTag[ii][0]);
		}
	}
	vector<vector<string>> vvsDrive = wf.getDrives();
	for (int ii = 0; ii < vvsDrive.size(); ii++) {
		if (setDriveType.count(vvsDrive[ii][1])) {
			cb->addItem(vvsDrive[ii][0].c_str());
			if (vvsDrive[ii][0] == defaultDrive) {
				activeIndex = cb->count() - 1;
			}
		}
	}

	qlItem = vLayout->itemAt(control->index::Text);
	QTextEdit* teIO = (QTextEdit*)qlItem->widget();
	teIO->setFocus();

	connect(control, &SCDAcontrol::driveSelected, this, &SCDA::scanLocalCata);
	connect(control, &SCDAcontrol::sendDebug, this, &SCDA::debug);
	connect(control, &SCDAcontrol::sendStructure, this, &SCDA::dialogStructureStart);
	connect(control, &SCDAcontrol::sendOnlineCata, this, &SCDA::displayOnlineCata);
	connect(control, &SCDAcontrol::sendSearchDBTable, this, &SCDA::searchDBTable);
	connect(this, &SCDA::appendTextIO, control, &SCDAcontrol::textAppend);
	connect(this, &SCDA::setTextIO, control, &SCDAcontrol::textOutput);

	cb->setCurrentIndex(activeIndex);
}
void SCDA::initGUI()
{
	QWidget* central = new QWidget;
	this->setCentralWidget(central);
	QHBoxLayout* hLayout = new QHBoxLayout;
	central->setLayout(hLayout);

	SCDAcontrol* control = new SCDAcontrol;
	hLayout->insertWidget(indexH::Control, control, 0);

	QVBoxLayout* vLayout = new QVBoxLayout;
	hLayout->insertLayout(indexH::Display, vLayout, 1);

	QTabWidget* tab = new QTabWidget;
	vLayout->insertWidget(indexV::Tab, tab, 1);
	SCDAcatalogue* cata = new SCDAcatalogue(configXML);
	connect(this, &SCDA::sendConfigXML, cata, &SCDAcatalogue::getConfigXML);
	connect(cata, &SCDAcatalogue::sendDownloadCata, this, &SCDA::downloadCata);
	connect(cata, &SCDAcatalogue::sendInsertCata, this, &SCDA::insertCata);
	connect(cata, &SCDAcatalogue::sendSearchCata, this, &SCDA::searchDBTable);
	connect(cata, &SCDAcatalogue::setTextIO, control, &SCDAcontrol::textOutput);
	tab->addTab(cata, "Catalogues");
	SCDAtable* table = new SCDAtable;
	connect(table, &SCDAtable::fetchTable, this, &SCDA::fetchDBTable);
	connect(table, &SCDAtable::sendBusyScreen, this, &SCDA::busyScreen);
	connect(table, &SCDAtable::sendDeleteRow, this, &SCDA::deleteTableRow);
	connect(table, &SCDAtable::sendDeleteTable, this, &SCDA::deleteTable);
	tab->addTab(table, "Tables");
	SCDAstructure* structure = new SCDAstructure;
	tab->addTab(structure, "Structures");
	SCDAcompare* compare = new SCDAcompare;
	tab->addTab(compare, "Compare");
	connect(compare, &SCDAcompare::sendDownloadMap, this, &SCDA::testMap);
	SCDAmap* map = new SCDAmap;
	map->initItemColour(configXML);
	tab->addTab(map, "Map");
	connect(map, &SCDAmap::sendLoadGeoTree, this, &SCDA::loadGeoTree);

	QJPROGRESSBAR* qjPBar1 = new QJPROGRESSBAR(1);
	vLayout->insertWidget(indexV::PBar1, qjPBar1, 0);
	connect(this, &SCDA::barMessage, qjPBar1, &QJPROGRESSBAR::barMessage);
	connect(this, &SCDA::initProgress, qjPBar1, &QJPROGRESSBAR::initProgress);
	connect(this, &SCDA::pbarHide, qjPBar1, &QJPROGRESSBAR::hide);
	connect(this, &SCDA::pbarShow, qjPBar1, &QJPROGRESSBAR::show);

	QJPROGRESSBAR* qjPBar0 = new QJPROGRESSBAR(0);
	vLayout->insertWidget(indexV::PBar0, qjPBar0, 0);
	connect(this, &SCDA::barMessage, qjPBar0, &QJPROGRESSBAR::barMessage);
	connect(this, &SCDA::initProgress, qjPBar0, &QJPROGRESSBAR::initProgress);
	connect(this, &SCDA::pbarHide, qjPBar0, &QJPROGRESSBAR::hide);
	connect(this, &SCDA::pbarShow, qjPBar0, &QJPROGRESSBAR::show);

	QJBUSY* dialogBusy = new QJBUSY(this, Qt::FramelessWindowHint | Qt::CustomizeWindowHint);
	initBusy(dialogBusy);
	connect(dialogBusy, &QJBUSY::reportProgress, qjPBar0, &QJPROGRESSBAR::report);
	connect(dialogBusy, &QJBUSY::reportProgress, qjPBar1, &QJPROGRESSBAR::report);
	connect(dialogBusy, &QJBUSY::setTextIO, control, &SCDAcontrol::textOutput);

	updateCataDB();
	initControl(control);
	connect(map, &SCDAmap::appendTextIO, control, &SCDAcontrol::textAppend);
}
void SCDA::insertCata(string prompt)
{
	// Prompt should have form ($year$cata0$cata1...). 
	vector<string> vsPrompt;
	jparse.splitByMarker(vsPrompt, prompt, prompt[0]);
	int numCata = (int)vsPrompt.size() - 1;
	if (numCata < 0) { err("Invalid prompt-insertCata"); }

	// Determine the local root directory for this census year. 
	int commLength, iYear;
	try { iYear = stoi(vsPrompt[0]); }
	catch (invalid_argument) { err("iYear stoi-insertCata"); }
	if (iYear < 1981 || iYear > 2017) { err("Invalid year-insertCata"); }
	vector<string> vsTag = { "path", "local_storage" };
	string yearFolder = jparse.getXML1(configXML, vsTag) + "/" + vsPrompt[0];

	// If the prompt contains only the year, then all available catalogues 
	// for that year are inserted (unless already present).
	vector<string> vsCataFolder;
	if (numCata == 0) {
		vsCataFolder = wf.getFolderList(yearFolder, "*", 1);
		numCata = (int)vsCataFolder.size();
	}
	else {
		vsCataFolder.resize(numCata);
		for (int ii = 0; ii < numCata; ii++) {
			vsCataFolder[ii] = yearFolder + "/" + vsPrompt[1 + ii];
		}
	}

	// Initialize the progress bar.
	string sEnd;
	double dNumCata = (double)numCata;
	vector<string> vsProgress(numCata + 1);
	vector<double> vdProgress;
	if (numCata == 1) {
		commLength = 3;
		vsProgress[0] = "Inserting catalogue " + vsPrompt[1] + " ...";
		vsProgress[1] = "Finished inserting catalogue " + vsPrompt[1];
		vdProgress = { 0.0, 1.0 };
	}
	else {
		commLength = 5;
		sEnd = " of " + to_string(numCata) + ") ...";
		vdProgress.resize(numCata + 1);
		for (int ii = 0; ii < numCata; ii++) {
			vsProgress[ii] = "Inserting catalogue (" + to_string(1 + ii) + sEnd;
			vdProgress[ii] = (double)ii / dNumCata;
		}
		vsProgress.back() = "Finished inserting " + to_string(numCata) + " catalogues.";
		vdProgress.back() = 1.0;
		emit initProgress({ 0.0 }, { "Inserting Data Tables..." }, 0, 1);
		emit pbarShow(1);
	}
	emit initProgress(vdProgress, vsProgress, 0);

	// Take care of necessary data insertion which is not bound to any specific catalogue.
	scdb.insertCensus(vsPrompt[0]);
	scdb.insertGeoTreeTemplate(yearFolder);

	// Launch a catalogue manager/writer thread. It will pull the top catalogue from the work 
	// queue until all catalogues have been inserted.
	thread::id myid = this_thread::get_id();
	sb.startCall(myid, commLength);  // Comm form [status, geo progress, geo max, cata progress, cata max]
	sb.pushWork(vsCataFolder);
	std::thread thr(&SCdatabase::insertCata, scdb, ref(sb), numCore);
	thr.detach();
	busyWheel(sb);
	sb.endCall(myid);
	emit pbarHide(1);
}
void SCDA::loadGeoTree(string sYear, string sCata)
{
	QWidget* central = this->centralWidget();
	QHBoxLayout* hLayout = (QHBoxLayout*)central->layout();
	QLayoutItem* qlItem = hLayout->itemAt(indexH::Display);
	QVBoxLayout* vLayout = (QVBoxLayout*)qlItem->layout();
	qlItem = vLayout->itemAt(0);
	QTabWidget* tab = (QTabWidget*)qlItem->widget();
	SCDAmap* map = (SCDAmap*)tab->widget(indexTab::Map);
	vector<int> vID = map->modelCataMap->jt.searchData(sCata, 0);

	thread::id myid = this_thread::get_id();
	vector<string> vsPrompt = { sYear, sCata };
	sb.startCall(myid, commLength);
	sb.setPrompt(vsPrompt);
	std::thread thr(&SCdatabase::makeTreeGeo, scdb, ref(sb), ref(map->modelCataMap->jt), vID[0]);
	thr.detach();
	busyWheel(sb);

	int numRegion;
	sb.getPrompt(vsPrompt);
	if (vsPrompt.size() > 0) {
		try { numRegion = stoi(vsPrompt[0]); }
		catch (invalid_argument) { err("makeTreeGeo stoi-loadGeoTree"); }
		string message;
		if (numRegion == 0) { message = "No Geo table found for catalogue " + sCata + " !\n"; }
		else { message = "Catalogue " + sCata + " contains " + to_string(numRegion) + " geographic regions."; }
		emit appendTextIO(message);
	}

	map->modelCataMap->populate(map->modelCataMap->tree::jtree);
	QGridLayout* gLayout = (QGridLayout*)map->layout();
	qlItem = gLayout->itemAtPosition(1, map->Tree);
	QJTREEVIEW* qjTree = (QJTREEVIEW*)qlItem->widget();
	qjTree->setModel(map->modelCataMap.get());

	sb.endCall(myid);
}
void SCDA::postRender()
{
	// Casserole function for tasks to be done after main window rendering.	
	QCoreApplication::processEvents();
	vector<string> vsTag = { "path", "resource" };
	string resDir = jparse.getXML1(configXML, vsTag);

	QWidget* central = this->centralWidget();
	QRect rectCentral = central->geometry();
	QHBoxLayout* mainLayout = (QHBoxLayout*)central->layout();
	QLayoutItem* qlItem = mainLayout->itemAt(indexH::Display);
	QVBoxLayout* displayLayout = (QVBoxLayout*)qlItem->layout();
	qlItem = displayLayout->itemAt(indexV::PBar0);
	QJPROGRESSBAR* qjPBar0 = (QJPROGRESSBAR*)qlItem->widget();
	qjPBar0->initChildren();
	qjPBar0->initSound(resDir + "/sound");
	qlItem = displayLayout->itemAt(indexV::PBar1);
	QJPROGRESSBAR* qjPBar1 = (QJPROGRESSBAR*)qlItem->widget();
	qjPBar1->initChildren();
	qjPBar1->initSound(resDir + "/sound");

	qlItem = displayLayout->itemAt(indexV::Tab);
	QTabWidget* tabW = (QTabWidget*)qlItem->widget();
	SCDAcompare* compare = (SCDAcompare*)tabW->widget(3);
	QSize parentSize = compare->frameSize();
	QVBoxLayout* compareLayout = (QVBoxLayout*)compare->layout();
	qlItem = compareLayout->itemAt(0);
	QHBoxLayout* hLayout = (QHBoxLayout*)qlItem->layout();
	QRect layoutRect = hLayout->geometry();
	qlItem = hLayout->itemAt(4);
	QRect itemRect = qlItem->geometry();
	QLineEdit* leText = (QLineEdit*)qlItem->widget();

}
void SCDA::scanLocalCata(string drive)
{
	QWidget* central = this->centralWidget();
	QHBoxLayout* hLayout = (QHBoxLayout*)central->layout();
	QLayoutItem* qlItem = nullptr;
	if (drive.size() == 0) {
		qlItem = hLayout->itemAt(indexH::Control);
		SCDAcontrol* control = (SCDAcontrol*)qlItem->widget();
		drive = control->getDrive();
	}

	qlItem = hLayout->itemAt(indexH::Display);
	QVBoxLayout* vLayout = (QVBoxLayout*)qlItem->layout();
	qlItem = vLayout->itemAt(0);
	QTabWidget* tab = (QTabWidget*)qlItem->widget();
	SCDAcatalogue* page = (SCDAcatalogue*)tab->currentWidget();
	QJTREEMODEL* qjtm = nullptr;
	int indexPage = tab->currentIndex();
	switch (indexPage) {
	case indexTab::Catalogue:
	{
		page->resetModel(page->index::Local);
		qjtm = page->getModel(page->index::Local);
		vector<vector<int>> comm(1, vector<int>());
		comm[0].assign(commLength, 0);
		thread::id myid = this_thread::get_id();
		vector<string> prompt = { drive + ":/" };
		sb.setPrompt(prompt);
		sb.startCall(myid, comm[0]);
		std::thread thr(&SCDAcatalogue::scanLocal, page, ref(sb), ref(page), ref(configXML));
		thr.detach();
		while (1) {
			jtime.sleep(sleepTime);
			QCoreApplication::processEvents();
			comm = sb.update(myid, comm[0]);
			if (comm.size() > 1 && comm[1][0] == 1) { break; }
		}
		sb.endCall(myid);

		qjtm->populate(qjtm->tree::jtree);
		int numGen = qjtm->jt.getExpandGeneration(30);

		QGridLayout* gLayout = (QGridLayout*)page->layout();
		qlItem = gLayout->itemAtPosition(1, page->index::Local);
		QJTREEVIEW* qjTree = (QJTREEVIEW*)qlItem->widget();
		qjTree->setModel(qjtm);
		qjTree->expandRecursively(QModelIndex(), numGen);

		emit barMessage("Displaying local catalogues from drive " + drive);
		break;
	}
	}
}
void SCDA::searchDBTable(string sQuery)
{
	// Display a tree structure of database tables which satisfy the 
	// given search criteria. '*' is treated as a wildcard char. 

	QWidget* central = this->centralWidget();
	QHBoxLayout* hLayout = (QHBoxLayout*)central->layout();
	QLayoutItem* qlItem = hLayout->itemAt(indexH::Display);
	QVBoxLayout* vLayout = (QVBoxLayout*)qlItem->layout();
	qlItem = vLayout->itemAt(0);
	QTabWidget* tab = (QTabWidget*)qlItem->widget();
	tab->setCurrentIndex(indexTab::Table);
	SCDAtable* table = (SCDAtable*)tab->widget(indexTab::Table);
	table->resetModel(table->index::Search);
	QJTREEMODEL* model = table->getModelTree(table->index::Search);

	vector<string> vsTable;
	sb.setPrompt(sQuery);
	thread::id myid = this_thread::get_id();
	sb.startCall(myid, commLength);
	std::thread thr(&SCdatabase::searchTable, scdb, ref(sb), ref(model->jt), ref(vsTable));
	thr.detach();
	busyWheel(sb);
	sb.endCall(myid);
	model->jt.setExpandGeneration(20);
	model->populate(model->tree::jtree);
	int numGen = model->jt.getExpandGeneration(30);

	QGridLayout* gLayout = (QGridLayout*)table->layout();
	qlItem = gLayout->itemAtPosition(1, table->index::Search);
	QJTREEVIEW* treeSearch = (QJTREEVIEW*)qlItem->widget();
	treeSearch->setModel(model);
	//treeSearch->expandToDepth(numGen - 1);
	treeSearch->expandRecursively(QModelIndex(), numGen);

	// Report the number of tables found.
	int numTable = (int)vsTable.size();
	string sReport = "\n" + to_string(numTable);
	if (numTable == 1) { sReport += " database table found."; }
	else { sReport += " database tables found."; }
	emit appendTextIO(sReport);

	// If only one table was found, display it.
	if (numTable == 1) { fetchDBTable(vsTable[0]); }
}
void SCDA::testMap(string url)
{
	string mapDir = sLocalStorage + "/mapsTest";
	QString qsPath = QFileDialog::getSaveFileName(this, "Save File", mapDir.c_str());
	if (qsPath.size() < 1) { return; }
	vector<string> vsPrompt(2);
	vsPrompt[0] = url;
	vsPrompt[1] = qsPath.toStdString();

	QWidget* central = this->centralWidget();
	QHBoxLayout* hLayout = (QHBoxLayout*)central->layout();
	QLayoutItem* qlItem = hLayout->itemAt(indexH::Display);
	QVBoxLayout* vLayout = (QVBoxLayout*)qlItem->layout();
	qlItem = vLayout->itemAt(0);
	QTabWidget* tab = (QTabWidget*)qlItem->widget();
	SCDAcompare* compare = (SCDAcompare*)tab->widget(indexTab::Compare);

	thread::id myid = this_thread::get_id();
	sb.startCall(myid, commLength);
	sb.setPrompt(vsPrompt);
	std::thread thr(&SCDAcompare::testScalePos, compare, ref(sb), ref(sco));
	thr.detach();
	busyWheel(sb);
	sb.endCall(myid);
}
void SCDA::updateCataDB()
{
	// Load a tree structure of all catalogues within the database, and populate the
	// relevant widgets.
	QWidget* central = this->centralWidget();
	QHBoxLayout* hLayout = (QHBoxLayout*)central->layout();
	QLayoutItem* qlItem = hLayout->itemAt(indexH::Display);
	QVBoxLayout* vLayout = (QVBoxLayout*)qlItem->layout();
	qlItem = vLayout->itemAt(0);
	QTabWidget* tab = (QTabWidget*)qlItem->widget();
	SCDAcatalogue* page = (SCDAcatalogue*)tab->widget(indexTab::Catalogue);
	QJTREEMODEL* qjtm = page->getModel(page->index::Database);

	thread::id myid = this_thread::get_id();
	sb.startCall(myid, commLength);
	std::thread thr(&SCdatabase::makeTreeCata, scdb, ref(sb), ref(qjtm->jt));
	thr.detach();
	busyWheel(sb);
	sb.endCall(myid);

	qjtm->populate(qjtm->tree::jtree);
	int numGen = qjtm->jt.getExpandGeneration(30);
	QGridLayout* gLayout = (QGridLayout*)page->layout();
	qlItem = gLayout->itemAtPosition(1, page->index::Database);
	QJTREEVIEW* qjTree = (QJTREEVIEW*)qlItem->widget();
	qjTree->setModel(qjtm);
	qjTree->expandRecursively(QModelIndex(), numGen);

	SCDAmap* map = (SCDAmap*)tab->widget(indexTab::Map);
	QJTREEMODEL* qjtmMap = map->modelCataMap.get();
	qjtmMap->jt = qjtm->jt;
	qjtmMap->populate(qjtmMap->tree::jtree);
	gLayout = (QGridLayout*)map->layout();
	qlItem = gLayout->itemAtPosition(1, map->index::Tree);
	qjTree = (QJTREEVIEW*)qlItem->widget();
	qjTree->setModel(qjtmMap);
	qjTree->expandRecursively(QModelIndex(), numGen);
}