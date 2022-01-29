#include "SCDAexplorer.h"

using namespace std;

SCDA::SCDA(string execFolder, QWidget* parent) 
	: QMainWindow(parent), sExecFolder(execFolder)
{
	setWindowTitle("SCDA Explorer");
	//QRect rectDesktop = getDesktop();

	initConfig();
	initGUI();

	this->setWindowState(Qt::WindowMaximized);
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

}
void SCDA::deleteTable(string tname)
{
	scdb.deleteTable(tname);
	
	QWidget* central = this->centralWidget();
	QHBoxLayout* hLayout = (QHBoxLayout*)central->layout();
	QLayoutItem* qlItem = hLayout->itemAt(indexControl);
	SCDAcontrol* control = (SCDAcontrol*)qlItem->widget();
	if (control->sLastQuery.size() > 0) {
		searchDBTable(control->sLastQuery);
	}
}
void SCDA::dialogStructureStart()
{
	QString qsPath = QFileDialog::getOpenFileName(this, "Open File", sLocalStorage.c_str(), "XML files (*.xml)");
	if (qsPath.size() < 1) { return; }
	wstring wsPath = qsPath.toStdWString();
	string sPath;
	jparse.utf16To8(sPath, wsPath);

	QWidget* central = this->centralWidget();
	QHBoxLayout* hLayout = (QHBoxLayout*)central->layout();
	QLayoutItem* qlItem = hLayout->itemAt(indexDisplay);
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
	QLayoutItem* qlItem = hLayout->itemAt(indexDisplay);
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
		QLayoutItem* qlItem = hLayout->itemAt(indexDisplay);
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

	thread::id myid = this_thread::get_id();
	sb.startCall(myid, commLength);
	sb.setPrompt(vsPrompt);
	std::thread thr(&SConline::downloadCata, sco, ref(sb));
	thr.detach();
	busyWheel(sb);
	sb.endCall(myid);
}
void SCDA::driveSelected(string drive)
{
	QWidget* central = this->centralWidget();
	QHBoxLayout* hLayout = (QHBoxLayout*)central->layout();
	QLayoutItem* qlItem = hLayout->itemAt(indexDisplay);
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
void SCDA::err(string message)
{
	string errorMessage = "SCDA error:\n" + message;
	JLOG::getInstance()->err(errorMessage);
}
void SCDA::fetchDBTable(string tname)
{
	vector<vector<string>> vvsData, vvsColTitle;
	scdb.loadTable(vvsData, vvsColTitle, tname);
	if (vvsData.size() > 0) {
		QWidget* central = this->centralWidget();
		QHBoxLayout* hLayout = (QHBoxLayout*)central->layout();
		QLayoutItem* qlItem = hLayout->itemAt(indexDisplay);
		QVBoxLayout* vLayout = (QVBoxLayout*)qlItem->layout();
		qlItem = vLayout->itemAt(0);
		QTabWidget* tab = (QTabWidget*)qlItem->widget();
		tab->setCurrentIndex(indexTab::Table);
		SCDAtable* table = (SCDAtable*)tab->widget(indexTab::Table);

		string title = "Table (" + tname + ")";
		table->displayTable(vvsData, vvsColTitle, title);
	}
}
QRect SCDA::getDesktop()
{
	QList<QScreen*> listScreen = qApp->screens();
	if (listScreen.size() < 1) { err("No screens found-getDesktop"); }
	return listScreen[0]->geometry();
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

	connect(control, &SCDAcontrol::driveSelected, this, &SCDA::driveSelected);
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

	indexControl = 0;
	SCDAcontrol* control = new SCDAcontrol;
	hLayout->insertWidget(indexControl, control, 0);

	indexDisplay = 1;
	QVBoxLayout* vLayout = new QVBoxLayout;
	hLayout->insertLayout(indexDisplay, vLayout, 1);

	indexTab = 0;
	QTabWidget* tab = new QTabWidget;
	vLayout->insertWidget(indexTab, tab, 1);
	SCDAcatalogue* cata = new SCDAcatalogue(configXML);
	connect(this, &SCDA::sendConfigXML, cata, &SCDAcatalogue::getConfigXML);
	connect(cata, &SCDAcatalogue::sendDownloadCata, this, &SCDA::downloadCata);
	connect(cata, &SCDAcatalogue::sendInsertCata, this, &SCDA::insertCata);
	connect(cata, &SCDAcatalogue::sendSearchCata, this, &SCDA::searchDBTable);
	connect(cata, &SCDAcatalogue::setTextIO, control, &SCDAcontrol::textOutput);
	tab->addTab(cata, "Catalogues");
	SCDAtable* table = new SCDAtable;
	connect(table, &SCDAtable::fetchTable, this, &SCDA::fetchDBTable);
	connect(table, &SCDAtable::sendDeleteTable, this, &SCDA::deleteTable);
	tab->addTab(table, "Tables");
	SCDAstructure* structure = new SCDAstructure;
	tab->addTab(structure, "Structures");

	indexPBar = 1;
	QJPROGRESSBAR* qjPBar = new QJPROGRESSBAR;
	connect(this, &SCDA::barMessage, qjPBar, &QJPROGRESSBAR::barMessage);
	connect(this, &SCDA::initProgress, qjPBar, &QJPROGRESSBAR::initProgress);
	vLayout->insertWidget(indexPBar, qjPBar, 0);

	QJBUSY* dialogBusy = new QJBUSY(this);
	initBusy(dialogBusy);
	connect(dialogBusy, &QJBUSY::reportProgress, qjPBar, &QJPROGRESSBAR::report);
	connect(dialogBusy, &QJBUSY::sendText, control, &SCDAcontrol::textOutput);

	updateCataDB();
	initControl(control);
}
void SCDA::insertCata(string prompt)
{
	// Prompt should have form (@year@cata0@cata1...). 
	vector<string> vsPrompt;
	jparse.splitByMarker(vsPrompt, prompt, prompt[0]);
	int numCata = (int)vsPrompt.size() - 1;
	if (numCata < 0) { err("Invalid prompt-insertCata"); }

	// Determine the local root directory for this census year. 
	int iYear;
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
		vsProgress[0] = "Inserting catalogue " + vsPrompt[1] + " ...";
		vdProgress = { 0.0, 1.0 };
	}
	else {
		sEnd = " of " + to_string(numCata) + ") ...";
		vdProgress.resize(numCata + 1);
		for (int ii = 0; ii < numCata; ii++) {
			vsProgress[ii] = "Inserting catalogue (" + to_string(1 + ii) + sEnd;
			vdProgress[ii] = (double)ii / dNumCata;
		}
		vdProgress.back() = 1.0;
	}
	vsProgress.back() = "Finished inserting catalogue " + vsPrompt[1];
	emit initProgress(vdProgress, vsProgress);

	// Take care of necessary data insertion which is not bound to any specific catalogue.
	scdb.insertCensus(vsPrompt[0]);
	scdb.insertGeoTreeTemplate(yearFolder);

	// Launch worker threads. Each one will pull the top catalogue from the work queue 
	// until all catalogues have been inserted.
	thread::id myid = this_thread::get_id();
	sb.startCall(myid, commLength);
	sb.pushWork(vsCataFolder);
	std::thread thr(&SCdatabase::insertCata, scdb, ref(sb), numCore);
	thr.detach();
	busyWheel(sb);
	sb.endCall(myid);
}
void SCDA::postRender()
{
	// Casserole function for tasks to be done after main window rendering.	
	QCoreApplication::processEvents();
	QWidget* central = this->centralWidget();
	QRect rectCentral = central->geometry();
	QHBoxLayout* mainLayout = (QHBoxLayout*)central->layout();
	QLayoutItem* qlItem = mainLayout->itemAt(indexDisplay);
	QVBoxLayout* displayLayout = (QVBoxLayout*)qlItem->layout();
	qlItem = displayLayout->itemAt(indexPBar);
	QJPROGRESSBAR* qjPBar = (QJPROGRESSBAR*)qlItem->widget();

	/*
	QLabel* labelBar = central->findChild<QLabel*>("labelBar", Qt::FindDirectChildrenOnly);
	QHBoxLayout* hLayout = (QHBoxLayout*)central->layout();
	QLayoutItem* qlItem = hLayout->itemAt(indexDisplay);
	QVBoxLayout* vLayout = (QVBoxLayout*)qlItem->layout();
	qlItem = vLayout->itemAt(1);
	QProgressBar* pBar = (QProgressBar*)qlItem->widget();
	QRect rect = pBar->geometry();
	labelBar->setGeometry(rect);
	*/

	qjPBar->initChildren();
}
void SCDA::searchDBTable(string sQuery)
{
	// Display a tree structure of database tables which satisfy the 
	// given search criteria. '*' is treated as a wildcard char. 

	QWidget* central = this->centralWidget();
	QHBoxLayout* hLayout = (QHBoxLayout*)central->layout();
	QLayoutItem* qlItem = hLayout->itemAt(indexDisplay);
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
void SCDA::updateCataDB()
{
	QWidget* central = this->centralWidget();
	QHBoxLayout* hLayout = (QHBoxLayout*)central->layout();
	QLayoutItem* qlItem = hLayout->itemAt(indexDisplay);
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
}
