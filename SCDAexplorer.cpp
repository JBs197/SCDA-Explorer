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
	tab->setCurrentIndex(indexCata);
	SCDAcatalogue* cata = (SCDAcatalogue*)tab->widget(indexCata);

	thread::id myid = this_thread::get_id();
	vector<vector<int>> comm(1, vector<int>());
	comm[0].assign(commLength, 0);
	sb.start_call(myid, 1, comm[0]);
	std::thread thr(&SCDAcatalogue::displayOnlineCata, cata, ref(sb), ref(cata), ref(sco));
	thr.detach();
	busyWheel(sb, comm);
	sb.end_call(myid);

	QGridLayout* gLayout = (QGridLayout*)cata->layout();
	qlItem = gLayout->itemAtPosition(1, cata->indexStatscan);
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
	sb.set_prompt(vsPrompt);
	sb.start_call(myid, 1, comm[0]);
	std::thread thr(&SConline::downloadCata, sco, ref(sb));
	thr.detach();
	busyWheel(sb, comm);
	sb.end_call(myid);

}
void SCDA::driveSelected(string drive)
{
	QWidget* central = this->centralWidget();
	QHBoxLayout* hLayout = (QHBoxLayout*)central->layout();
	QLayoutItem* qlItem = hLayout->itemAt(indexDisplay);
	QVBoxLayout* vLayout = (QVBoxLayout*)qlItem->layout();
	qlItem = vLayout->itemAt(0);
	QTabWidget* tab = (QTabWidget*)qlItem->widget();
	int indexPage = tab->currentIndex();
	switch (indexPage) {
	case 0:
	{
		SCDAcatalogue* page = (SCDAcatalogue*)tab->currentWidget();
		QJTREEMODEL* qjtm = page->getModel(page->indexLocal);
		qjtm->jt.reset();

		vector<vector<int>> comm(1, vector<int>());
		comm[0].assign(commLength, 0);
		thread::id myid = this_thread::get_id();
		vector<string> prompt = { drive + ":\\" };
		sb.set_prompt(prompt);
		sb.start_call(myid, 1, comm[0]);
		std::thread thr(&SCDA::scanCataLocal, this, ref(sb), ref(qjtm->jt));
		thr.detach();
		while (1)
		{
			jf.sleep(sleepTime);
			QCoreApplication::processEvents();
			comm = sb.update(myid, comm[0]);
			if (comm.size() > 1 && comm[1][0] == 1) { break; }
		}
		sb.end_call(myid);
		qjtm->populate();
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

	QLayoutItem* qlItem = vLayout->itemAt(control->indexDrive);
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
	connect(control, &SCDAcontrol::driveSelected, this, &SCDA::driveSelected);
	connect(control, &SCDAcontrol::sendDebug, this, &SCDA::debug);
	cb->setCurrentIndex(activeIndex);

	QWidget* central = this->centralWidget();
	hLayout = (QHBoxLayout*)central->layout();
	qlItem = hLayout->itemAt(indexDisplay);
	vLayout = (QVBoxLayout*)qlItem->layout();
	qlItem = vLayout->itemAt(0);
	QTabWidget* tab = (QTabWidget*)qlItem->widget();
	SCDAcatalogue* page = (SCDAcatalogue*)tab->currentWidget();
	connect(control, &SCDAcontrol::sendOnlineCata, this, &SCDA::displayOnlineCata);
}
void SCDA::initDatabase()
{
	vector<string> vsTag = { "path", "database" };
	vector<vector<string>> vvsTag = jf.getXML(configXML, vsTag);
	if (vvsTag[0][1].size() < 1) { err("Failed to extract database file path-initDatabase"); }
	sf.init(vvsTag[0][1]);
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
	indexCata = 0;
	SCDAcatalogue* cata = new SCDAcatalogue;
	connect(this, &SCDA::sendConfigXML, cata, &SCDAcatalogue::getConfigXML);
	connect(cata, &SCDAcatalogue::sendDownloadCata, this, &SCDA::downloadCata);
	tab->addTab(cata, "Catalogues");
	indexTable = 1;
	indexMap = 2;

	indexPBar = 1;
	QJPROGRESSBAR* qjPBar = new QJPROGRESSBAR;
	connect(this, &SCDA::barMessage, qjPBar, &QJPROGRESSBAR::barMessage);
	connect(this, &SCDA::initProgress, qjPBar, &QJPROGRESSBAR::initProgress);
	vLayout->insertWidget(indexPBar, qjPBar, 0);

	/*
	QProgressBar* pBar = new QProgressBar;
	vLayout->addWidget(pBar, 0);
	QLabel* labelBar = new QLabel("", central);
	labelBar->setObjectName("labelBar");
	labelBar->setAlignment(Qt::AlignCenter);
	labelBar->move(0, 0);
	*/

	QJBUSY* dialogBusy = new QJBUSY(this);
	initBusy(dialogBusy);
	connect(dialogBusy, &QJBUSY::reportProgress, qjPBar, &QJPROGRESSBAR::report);

	initControl(control);
	updateCataDB();
}
void SCDA::initStatscan()
{
	sco.configXML = configXML;
	vector<string> vsTag = { "url", "statscan" };
	vector<vector<string>> vvsTag = jf.getXML(configXML, vsTag);
	sco.urlRoot = vvsTag[0][1];
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

	emit sendConfigXML(configXML);
}
void SCDA::scanCataLocal(SWITCHBOARD& sbgui, JTREE& jtgui)
{
	thread::id myid = this_thread::get_id();
	vector<int> mycomm;
	sbgui.answer_call(myid, mycomm);
	vector<string> prompt = sbgui.get_prompt();
	string cataPath, metaPath, search = "*", yearPath;
	vector<string> folderList = wf.get_folder_list(prompt[0], search);
	vector<string> cataList, sYearList;
	vector<int> csvCount;
	JNODE jnRoot = jtgui.getRoot();
	int iYear, numCata, rootID = jnRoot.ID;
	size_t pos1;
	bool meta;

	for (int ii = 0; ii < folderList.size(); ii++) {
		pos1 = folderList[ii].find_first_not_of("1234567890");
		if (pos1 > folderList[ii].size()) {
			try { iYear = stoi(folderList[ii]); }
			catch (invalid_argument) { err("stoi-MainWindow-on_cB_drives"); }
			if (iYear >= 1981 && iYear <= 2017) {
				sYearList.push_back(folderList[ii]);
			}
		}
	}
	jf.sortInteger(sYearList, JFUNC::Increasing);
	int numYear = (int)sYearList.size();
	for (int ii = 0; ii < numYear; ii++) {
		JNODE jnYear;
		jnYear.vsData.push_back(sYearList[ii]);
		jtgui.addChild(rootID, jnYear);
	}

	vector<int> viYearID(numYear);
	vector<reference_wrapper<JNODE>> vJNYear = jtgui.getChildren(rootID);
	for (int ii = 0; ii < numYear; ii++) {
		viYearID[ii] = vJNYear[ii].get().ID;
	}

	for (int ii = 0; ii < numYear; ii++) {
		yearPath = prompt[0] + "\\" + sYearList[ii];
		folderList = wf.get_folder_list(yearPath, search);
		numCata = (int)folderList.size();
		csvCount.resize(numCata);
		for (int jj = 0; jj < numCata; jj++) {
			cataPath = yearPath + "\\" + folderList[jj];
			csvCount[jj] = wf.get_file_path_number(cataPath, "csv");
			metaPath = cataPath + "\\" + folderList[jj] + "_English_meta.txt";
			meta = wf.file_exist(metaPath);
			if (meta == 1 && csvCount[jj] > 0) {
				JNODE jn;
				jn.vsData.push_back(folderList[jj]);
				jtgui.addChild(viYearID[ii], jn);
			}
		}
	}
	mycomm[0] = 1;
	sbgui.update(myid, mycomm);
}
void SCDA::updateCataDB()
{
	QWidget* central = this->centralWidget();
	QHBoxLayout* hLayout = (QHBoxLayout*)central->layout();
	QLayoutItem* qlItem = hLayout->itemAt(indexDisplay);
	QVBoxLayout* vLayout = (QVBoxLayout*)qlItem->layout();
	qlItem = vLayout->itemAt(0);
	QTabWidget* tab = (QTabWidget*)qlItem->widget();
	SCDAcatalogue* page = (SCDAcatalogue*)tab->widget(indexCata);
	QJTREEMODEL* qjtm = page->getModel(page->indexDatabase);
	qjtm->jt.reset();

	JNODE jnRoot = qjtm->jt.getRoot();
	int rootID = jnRoot.ID;
	vector<string> search = { "Year" }, yearList, cataList;
	string tname = "Census";
	string orderby = "Year ASC";
	sf.selectOrderBy(search, tname, yearList, orderby);
	int numCata, numYear = (int)yearList.size();
	for (int ii = 0; ii < numYear; ii++) {
		JNODE jnYear;
		jnYear.vsData.push_back(yearList[ii]);
		qjtm->jt.addChild(rootID, jnYear);
	}

	vector<int> viYearID(numYear);
	vector<reference_wrapper<JNODE>> vJNYear = qjtm->jt.getChildren(rootID);
	for (int ii = 0; ii < numYear; ii++) {
		viYearID[ii] = vJNYear[ii].get().ID;
	}

	search = { "Catalogue" };
	for (int ii = 0; ii < numYear; ii++) {
		cataList.clear();
		tname = "Census$" + yearList[ii];
		numCata = sf.select(search, tname, cataList);
		for (int jj = 0; jj < numCata; jj++) {
			JNODE jn;
			jn.vsData.push_back(cataList[jj]);
			qjtm->jt.addChild(viYearID[ii], jn);
		}
	}

	qjtm->populate();
}
