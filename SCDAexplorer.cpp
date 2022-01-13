#include "SCDAexplorer.h"

SCDA::SCDA(string execFolder, QWidget* parent)
	: QMainWindow(parent), sExecFolder(execFolder)
{
	setWindowTitle("SCDA Explorer");
	QRect rectDesktop = getDesktop();

	initConfig();
	initDatabase();
	initGUI();

	this->setWindowState(Qt::WindowMaximized);
}

void SCDA::busyWheel(SWITCHBOARD& sb, vector<vector<int>> comm)
{
	QJBUSY* dialogBusy = this->findChild<QJBUSY*>("Busy", Qt::FindDirectChildrenOnly);
	dialogBusy->show();
	dialogBusy->busy(sb, comm);
	dialogBusy->hide();
}
void SCDA::debug()
{
	vector<pair<double, double>> vPerimeter = jf.rectangleRound(400.0, 300.0, 50.0);
	vector<pair<int, int>> vPixel = jf.pixelate(vPerimeter);
	set<pair<int, int>> setCoord = jf.solidShape(vPixel);
	QColor qcBG = QColor::fromRgbF(0.0, 0.0, 0.0, 1.0);
	QImage qiBG;
	string bgPath = sExecFolder + "\\BusyWheelBG.png";
	bool success = qiBG.load(bgPath.c_str());
	if (!success) { 
		err("QImage load-debug"); 
	}
	for (auto it = setCoord.begin(); it != setCoord.end(); ++it) {
		qiBG.setPixel(get<0>(*it), get<1>(*it), qcBG.rgba());
	}
	bgPath = sExecFolder + "\\BusyBG.png";
	success = qiBG.save(bgPath.c_str());
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
	vector<vector<int>> comm(1, vector<int>());
	comm[0].assign(commLength, 0);
	sb.startCall(myid, comm[0]);
	std::thread thr(&SCDAcatalogue::displayOnlineCata, cata, ref(sb), ref(cata), ref(sco));
	thr.detach();
	busyWheel(sb, comm);
	sb.endCall(myid);

	QGridLayout* gLayout = (QGridLayout*)cata->layout();
	qlItem = gLayout->itemAtPosition(1, cata->index::Statscan);
	QJTREEVIEW* treeStatscan = (QJTREEVIEW*)qlItem->widget();
	treeStatscan->setModel(cata->modelStatscan.get());
	treeStatscan->update();
}
void SCDA::downloadCata(string prompt)
{
	// Prompt should have form (@year@cata0@cata1...). If the prompt
	// contains only the year, then all catalogues for that year are 
	// downloaded. 
	vector<string> vsPrompt = jf.splitByMarker(prompt, prompt[0]);
	if (vsPrompt.size() < 1) { err("Invalid prompt-downloadCata"); }

	vector<string> vsProgress = {
		"Downloading files ...",
		"Unzipping files ...",
		"Splitting large files ...",
		"Finished downloading catalogues."
	};
	vector<double> vdProgress = { 0.0, 1.0/3.0, 2.0/3.0, 1.0 };
	emit initProgress(vdProgress, vsProgress);

	thread::id myid = this_thread::get_id();
	vector<vector<int>> comm(1, vector<int>());
	comm[0].assign(commLength, 0);
	sb.setPrompt(vsPrompt);
	sb.startCall(myid, comm[0]);
	std::thread thr(&SConline::downloadCata, sco, ref(sb));
	thr.detach();
	busyWheel(sb, comm);
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
			jf.sleep(sleepTime);
			QCoreApplication::processEvents();
			comm = sb.update(myid, comm[0]);
			if (comm.size() > 1 && comm[1][0] == 1) { break; }
		}
		sb.endCall(myid);

		qjtm->populate();
		QGridLayout* gLayout = (QGridLayout*)page->layout();
		qlItem = gLayout->itemAtPosition(1, page->index::Local);
		QJTREEVIEW* qjTree = (QJTREEVIEW*)qlItem->widget();
		qjTree->setModel(qjtm);

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
	vector<vector<string>> vvsTag = jf.getXML(configXML, vsTag);
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
	configXML = jf.load(configPath);
	initStatscan();

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
	vector<vector<string>> vvsTag = jf.getXML(configXML, vsTag);
	string defaultDrive = "";
	if (vvsTag.size() > 0) {
		defaultDrive = vvsTag[0][1];
	}
	vsTag = { "settings", "drive_type" };
	vvsTag = jf.getXML(configXML, vsTag);
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
	connect(control, &SCDAcontrol::sendOnlineCata, this, &SCDA::displayOnlineCata);
	connect(control, &SCDAcontrol::sendSearchDBTable, this, &SCDA::searchDBTable);
	connect(this, &SCDA::appendTextIO, control, &SCDAcontrol::textAppend);
	connect(this, &SCDA::setTextIO, control, &SCDAcontrol::textOutput);
	
	cb->setCurrentIndex(activeIndex);
}
void SCDA::initDatabase()
{
	vector<string> vsTag = { "path", "database" };
	vector<vector<string>> vvsTag = jf.getXML(configXML, vsTag);
	if (vvsTag[0][1].size() < 1) { err("Failed to extract database file path-initDatabase"); }
	scdb.initDatabase(vvsTag[0][1]);
}
void SCDA::initGUI()
{
	// Note: qjBusy must already exist as a child of the MainWindow.
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
	tab->addTab(cata, "Catalogues");
	SCDAtable* table = new SCDAtable;
	connect(table, &SCDAtable::fetchTable, this, &SCDA::fetchDBTable);
	tab->addTab(table, "Tables");

	indexPBar = 1;
	QJPROGRESSBAR* qjPBar = new QJPROGRESSBAR;
	connect(this, &SCDA::barMessage, qjPBar, &QJPROGRESSBAR::barMessage);
	connect(this, &SCDA::initProgress, qjPBar, &QJPROGRESSBAR::initProgress);
	vLayout->insertWidget(indexPBar, qjPBar, 0);

	QJBUSY* dialogBusy = new QJBUSY(this);
	initBusy(dialogBusy);
	connect(dialogBusy, &QJBUSY::reportProgress, qjPBar, &QJPROGRESSBAR::report);

	updateCataDB();
	initControl(control);
}
void SCDA::initStatscan()
{
	sco.configXML = configXML;
	vector<string> vsTag = { "url", "statscan" };
	vector<vector<string>> vvsTag = jf.getXML(configXML, vsTag);
	sco.urlRoot = vvsTag[0][1];

	scdb.configXML = configXML;
}
void SCDA::insertCata(string prompt)
{
	// Prompt should have form (@year@cata0@cata1...). 
	vector<string> vsPrompt = jf.splitByMarker(prompt, prompt[0]);
	int numCata = (int)vsPrompt.size() - 1;
	if (numCata < 0) { err("Invalid prompt-insertCata"); }

	// Determine the local root directory for this census year. 
	int iYear, numMissing, numThread, numZip;
	try { iYear = stoi(vsPrompt[0]); }
	catch (invalid_argument) { err("stoi-insertCata"); }
	if (iYear < 1981 || iYear > 2017) { err("Invalid year-insertCata"); }
	vector<string> vsTag = { "path", "local_storage" };
	string yearFolder = jf.getXML1(configXML, vsTag);
	yearFolder += "/" + vsPrompt[0];

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

	vector<string> vsProgress = {
		"Inserting local catalogue files ...",
		"Finished inserting catalogues into the database."
	};
	vector<double> vdProgress = { 0.0, 1.0 };
	emit initProgress(vdProgress, vsProgress);

	thread::id myid = this_thread::get_id();
	vector<vector<int>> comm(1, vector<int>());
	comm[0].assign(commLength, 0);
	comm[0][2] = numCata;
	sb.setPrompt(vsCataFolder);
	sb.startCall(myid, comm[0]);
	std::thread thr(&SCdatabase::insertCata, scdb, ref(sb));
	thr.detach();
	busyWheel(sb, comm);
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
	vector<vector<int>> comm(1, vector<int>());
	comm[0].assign(commLength, 0);
	sb.startCall(myid, comm[0]);
	std::thread thr(&SCdatabase::searchTable, scdb, ref(sb), ref(model->jt), ref(vsTable));
	thr.detach();
	busyWheel(sb, comm);
	sb.endCall(myid);
	model->jt.setExpandGeneration(20);
	model->populate();

	QGridLayout* gLayout = (QGridLayout*)table->layout();
	qlItem = gLayout->itemAtPosition(1, table->index::Search);
	QJTREEVIEW* treeSearch = (QJTREEVIEW*)qlItem->widget();
	treeSearch->setModel(model);
	treeSearch->expandRecursively(QModelIndex(), 1);
	treeSearch->update();

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
	vector<vector<int>> comm(1, vector<int>());
	comm[0].assign(commLength, 0);
	sb.startCall(myid, comm[0]);
	std::thread thr(&SCdatabase::makeTreeCata, scdb, ref(sb), ref(qjtm->jt));
	thr.detach();
	busyWheel(sb, comm);
	sb.endCall(myid);

	qjtm->populate();
}
