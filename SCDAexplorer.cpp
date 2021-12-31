#include "SCDAexplorer.h"

SCDA::SCDA(string execFolder, QWidget* parent)
	: QMainWindow(parent), sExecFolder(execFolder)
{
	setWindowTitle("SCDA Explorer");

	initConfig();
	initDatabase();
	initGUI();

	this->setWindowState(Qt::WindowMaximized);
}

void SCDA::barMessage(string message)
{
	int numChar = message.size();
	QString qsMessage = QString::fromUtf8(message.c_str());
	QCoreApplication::processEvents();

	lock_guard<mutex> lock(m_bar);
	QWidget* central = this->centralWidget();
	QLabel* labelBar = central->findChild<QLabel*>("labelBar", Qt::FindDirectChildrenOnly);
	QHBoxLayout* hLayout = (QHBoxLayout*)central->layout();
	QLayoutItem* qlItem = hLayout->itemAt(indexDisplay);
	QVBoxLayout* vLayout = (QVBoxLayout*)qlItem->layout();
	qlItem = vLayout->itemAt(1);
	QProgressBar* pBar = (QProgressBar*)qlItem->widget();
	QRect rect = pBar->geometry();
	labelBar->setGeometry(rect);
	labelBar->setText(qsMessage);
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
		barMessage("Displaying local catalogues from drive " + drive);

		break;
	}
	}
}
void SCDA::err(string message)
{
	string errorMessage = "SCDA error:\n" + message;
	JLOG::getInstance()->err(errorMessage);
}
void SCDA::initConfig()
{
	string configPath = sExecFolder + "\\SCDA_Explorer_Config.xml";
	configXML = jf.load(configPath);

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
	cb->setCurrentIndex(activeIndex);

	QWidget* central = this->centralWidget();
	hLayout = (QHBoxLayout*)central->layout();
	qlItem = hLayout->itemAt(indexDisplay);
	vLayout = (QVBoxLayout*)qlItem->layout();
	qlItem = vLayout->itemAt(0);
	QTabWidget* tab = (QTabWidget*)qlItem->widget();
	SCDAcatalogue* page = (SCDAcatalogue*)tab->currentWidget();
	connect(control, &SCDAcontrol::sendOnlineCata, page, &SCDAcatalogue::displayOnlineCata);
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

	QTabWidget* tab = new QTabWidget;
	vLayout->addWidget(tab, 1);
	indexCata = 0;
	SCDAcatalogue* cata = new SCDAcatalogue;
	connect(this, &SCDA::sendConfigXML, cata, &SCDAcatalogue::getConfigXML);
	tab->addTab(cata, "Catalogues");
	indexTable = 1;
	indexMap = 2;

	QProgressBar* pBar = new QProgressBar;
	vLayout->addWidget(pBar, 0);
	QLabel* labelBar = new QLabel("", central);
	labelBar->setObjectName("labelBar");
	labelBar->setAlignment(Qt::AlignCenter);
	labelBar->move(0, 0);

	initControl(control);
	updateCataDB();
}
void SCDA::postRender()
{
	// Casserole function for tasks to be done after main window rendering.
	QCoreApplication::processEvents();
	QWidget* central = this->centralWidget();
	QLabel* labelBar = central->findChild<QLabel*>("labelBar", Qt::FindDirectChildrenOnly);
	QHBoxLayout* hLayout = (QHBoxLayout*)central->layout();
	QLayoutItem* qlItem = hLayout->itemAt(indexDisplay);
	QVBoxLayout* vLayout = (QVBoxLayout*)qlItem->layout();
	qlItem = vLayout->itemAt(1);
	QProgressBar* pBar = (QProgressBar*)qlItem->widget();
	QRect rect = pBar->geometry();
	labelBar->setGeometry(rect);

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
	int iYear, numCata, parentID, rootID = jnRoot.ID;
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
	int numYear = sYearList.size();
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
		numCata = folderList.size();
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
	int index, rootID = jnRoot.ID;
	vector<string> search = { "Year" }, yearList, cataList;
	string tname = "Census";
	string orderby = "Year ASC";
	sf.selectOrderBy(search, tname, yearList, orderby);
	int numCata, numYear = yearList.size();
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
