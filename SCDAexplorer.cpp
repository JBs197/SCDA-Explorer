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

void SCDA::busyWheel(SWITCHBOARD& sb)
{
	QJBUSY* dialogBusy = this->findChild<QJBUSY*>("Busy", Qt::FindDirectChildrenOnly);
	dialogBusy->show();
	dialogBusy->busy(sb);
	dialogBusy->hide();
}
void SCDA::debug()
{
	string sSearch = "E:/2013/*";
	vector<string> vsCataDir, vsPartPath;
	jfile.search(vsCataDir, sSearch, JFILE::Directory);
	for (string cataDir : vsCataDir) {
		vsPartPath.clear();
		sSearch = cataDir + "/*PART*";
		jfile.search(vsPartPath, sSearch, JFILE::File);
		if (vsPartPath.size() > 0) {
			jfile.fileMerger(vsPartPath, 0);
			int bbq = 1;
		}

		
	}

	/*
	size_t pos1, pos2;
	int countDown, countUp, index, indexPixel, sum;
	string bot, left, right, top, mapPath, savedFile, sInform, sName, sx, sy, url;
	string tname{ "GeoTreeTemplate$2013$canada$province$cmaca" };
	vector<string> search{ "Region Name" };
	vector<string> conditions{ "GEO_LEVEL = 2" };
	vector<string> vsRegion;
	int numRegion = scdb.sf.select(search, tname, vsRegion, conditions);
	if (numRegion == 0) { err("Failed to load region list"); }
	string mapDir = "E:/maps/canada/province/cmaca";
	
	QList<QRect> listResolution = getDesktop();
	int width = listResolution[0].x() + listResolution[0].width();
	int height = listResolution[0].y() + listResolution[0].height();
	for (int ii = 1; ii < listResolution.size(); ii++) {
		if (listResolution[ii].x() + listResolution[ii].width() > width) {
			width = listResolution[ii].x() + listResolution[ii].width();
		}
		if (listResolution[ii].y() + listResolution[ii].height() > height) {
			height = listResolution[ii].y() + listResolution[ii].height();
		}
	}
	IOFUNC io(width, height);
	vector<POINT> vPosition;  // Form [search bar, candidate1 url, candidate1 copy address, candidate2 url, candidate2 copy address, ...].
	POINT p1;
	bool needPosition = 1;
	string savedPath = sExecFolder + "/savedIO.txt";
	if (jfile.fileExist(savedPath)) {
		jfile.load(savedFile, savedPath);
		sInform = "Previous position settings detected! Saved from:\n";
		pos2 = savedFile.find_first_of("\r\n");
		sInform += savedFile.substr(0, pos2);

		pos1 = savedFile.find_first_not_of(" \r\n", pos2);
		while (pos1 < savedFile.size()) {
			pos2 = savedFile.find(',', pos1);
			sx = savedFile.substr(pos1, pos2 - pos1);
			pos1 = savedFile.find_first_not_of(" ,", pos2);
			pos2 = savedFile.find_first_of("\r\n", pos1);
			sy = savedFile.substr(pos1, pos2 - pos1);
			try { 
				p1.x = stoi(sx);
				p1.y = stoi(sy);
			}
			catch (invalid_argument) { err("vPosition stoi"); }
			vPosition.emplace_back(p1);
			pos1 = savedFile.find_first_not_of(" \r\n", pos2);
		}
		if (vPosition.size() >= 6 && vPosition.size() % 2 == 0) { 
			QMessageBox qmb;
			qmb.setText(sInform.c_str());
			qmb.setInformativeText("Do you want to reuse these saved positions?");
			qmb.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
			qmb.setDefaultButton(QMessageBox::Yes);
			int yesNo = qmb.exec();
			if (yesNo == QMessageBox::Yes) { needPosition = 0; }
		}
	}
	if (needPosition) {
		string message = "Position cursor over SEARCH BAR, hit NUMPAD1 when ready...\n";
		emit appendTextIO(message);
		qApp->processEvents();
		vPosition.emplace_back(io.getCoord(VK_NUMPAD1));
		message = "Position cursor over FIRST CANDIDATE URL, hit NUMPAD2 when ready...\n";
		emit appendTextIO(message);
		qApp->processEvents();
		vPosition.emplace_back(io.getCoord(VK_NUMPAD2));
		message = "Position cursor over \"Copy link address\", hit NUMPAD3 when ready...\n";
		emit appendTextIO(message);
		qApp->processEvents();
		vPosition.emplace_back(io.getCoord(VK_NUMPAD3));
		message = "Position cursor over SECOND CANDIDATE URL, hit NUMPAD4 when ready...\n";
		emit appendTextIO(message);
		qApp->processEvents();
		vPosition.emplace_back(io.getCoord(VK_NUMPAD4));
		vPosition[3].x = vPosition[1].x - 3;  // Use minimum drift leftward, to avoid right-clicking the previous context menu.
		vPosition.emplace_back(vPosition[3]);
		vPosition.back().x += vPosition[2].x - vPosition[1].x;
		vPosition.back().y += vPosition[2].y - vPosition[1].y;

		savedFile.clear();
		savedFile = JLOG::getInstance()->timestamper();
		for (int ii = 0; ii < vPosition.size(); ii++) {
			savedFile += "\n" + to_string(vPosition[ii].x) + "," + to_string(vPosition[ii].y);
		}
		savedFile += "\n";
		jfile.printer(savedPath, savedFile);

		emit appendTextIO("Starting automation ...\n");
		qApp->processEvents();
	}

	GDIFUNC gdi;
	PNGFUNC pngf;
	vector<POINT> boxTLBR{ vPosition[0], vPosition[0] };
	boxTLBR[0].x -= 25;
	boxTLBR[0].y -= 50;
	boxTLBR[1].y += 50;
	vector<unsigned char> boxImg, mapFile, scanRowCol;
	vector<unsigned char> rgbTarget{ 124, 167, 243 };
	vector<int> boxSpec;
	double avg, dBot, dLeft, dRight, dTop;
	for (int ii = 0; ii < numRegion; ii++) {
		mapPath = mapDir + "/" + vsRegion[ii] + "*.png";
		if (jfile.fileExist(mapPath)) {
			emit appendTextIO("Skipping " + vsRegion[ii] + "\n");
			continue;
		}

		io.mouseClick(vPosition[0]);
		Sleep(1000);
		io.mouseClick(vPosition[0]);
		Sleep(1000);
		sName = vsRegion[ii];
		jparse.asciiNearestFit(sName);
		io.kbInput(sName, 0.5);  // Type at half speed.
		Sleep(1000);
		for (int jj = 0; jj < 4; jj++) {
			boxImg.clear();
			boxSpec.clear();
			gdi.capture(boxImg, boxSpec, boxTLBR);
			pngf.print("E:/" + to_string(jj) + ".png", boxImg, boxSpec);
			scanRowCol.clear();
			pngf.extractRow(scanRowCol, boxSpec[1] / 2, boxImg, boxSpec);
			index = 0;
			while (scanRowCol[index] == 255) { index++; }
			index -= boxSpec[2];
			indexPixel = index / boxSpec[2]; 
			scanRowCol.clear();
			pngf.extractCol(scanRowCol, indexPixel, boxImg, boxSpec);
			countUp = -1;
			indexPixel = scanRowCol.size() / (2 * boxSpec[2]);
			while (indexPixel >= 0) {
				index = indexPixel * boxSpec[2];
				if (abs(scanRowCol[index] - rgbTarget[0]) < 3) {
					if (abs(scanRowCol[index + 1] - rgbTarget[1]) < 3) {
						if (abs(scanRowCol[index + 2] - rgbTarget[2]) < 3) {
							countUp = indexPixel;
							break;
						}
					}
				}
				indexPixel--;
			}
			if (countUp < 0) { err("Failed to locate rgbTarget upward"); }
			countDown = -1;
			indexPixel = scanRowCol.size() / (2 * boxSpec[2]);
			while (indexPixel < boxSpec[1] - countUp + 10) {
				index = indexPixel * boxSpec[2];
				if (abs(scanRowCol[index] - rgbTarget[0]) < 3) {
					if (abs(scanRowCol[index + 1] - rgbTarget[1]) < 3) {
						if (abs(scanRowCol[index + 2] - rgbTarget[2]) < 3) {
							countDown = indexPixel;
							break;
						}
					}
				}
				indexPixel++;
			}
			if (countDown < 0) { break; }
			io.kbInput(VK_BACK);
			Sleep(2000);
		}
		if (countDown >= 0) { err("Failed to identify dropdown box"); }
		io.kbInput(VK_DOWN);
		Sleep(1000);
		io.kbInput(VK_RETURN);		
		Sleep(5000);

		for (int jj = 0; jj < 2; jj++) {
			io.mouseClick(vPosition[1 + (2 * jj)], io.click::Right);
			Sleep(2000);
			io.mouseClick(vPosition[2 + (2 * jj)]);
			Sleep(500);

			BOOL success = OpenClipboard(NULL);
			if (!success) { io.winerr("OpenClipboard-io.copyText"); }
			HANDLE hClip = GetClipboardData(CF_TEXT);
			if (hClip == NULL) { io.winerr("GetClipboardData-io.copyText"); }
			else { url = (string)static_cast<char*>(hClip); }
			if (url.size() < 1) { err("Copying from clipboard to string-io.copyText"); }
			success = EmptyClipboard();
			if (!success) { io.winerr("EmptyClipboard-io.copyText"); }
			success = CloseClipboard();
			if (!success) { io.winerr("CloseClipboard-io.copyText"); }

			pos1 = url.find("png32");
			if (pos1 < url.size()) { break; }
			pos2 = url.find("png8");
			if (pos2 > url.size()) { err("Lost while determining region png url"); }
		}
		if (pos1 > url.size()) { err("Failed to locate png32 in url"); }
		
		pos1 = url.rfind("&bbox=");
		if (pos1 > url.size()) { err("Failed to locate bbox within url"); }
		pos1 += 6;
		pos2 = url.find("%2C", pos1);
		left = url.substr(pos1, pos2 - pos1);
		pos1 = pos2 + 3;
		pos2 = url.find("%2C", pos1);
		bot = url.substr(pos1, pos2 - pos1);
		pos1 = pos2 + 3;
		pos2 = url.find("%2C", pos1);
		right = url.substr(pos1, pos2 - pos1);
		pos1 = pos2 + 3;
		pos2 = url.find('&', pos1);
		top = url.substr(pos1, pos2 - pos1);
		try {
			dLeft = stod(left);
			dTop = stod(top);
			dRight = stod(right);
			dBot = stod(bot);
		}
		catch (invalid_argument) { err("url TLBR stod"); }

		mapPath.resize(mapPath.size() - 5);
		mapPath += " [(" + left + "," + top + "),(" + right + "," + bot + ")].png";
		mapFile.clear();
		wf.browse(mapFile, url);
		jfile.printer(mapPath, mapFile);

		emit appendTextIO("Downloaded PNG map for " + vsRegion[ii] + "\n");
	}
	*/
}
void SCDA::deleteTable(string tname)
{
	scdb.deleteTable(tname);
	
	QWidget* central = this->centralWidget();
	QHBoxLayout* hLayout = (QHBoxLayout*)central->layout();
	QLayoutItem* qlItem = hLayout->itemAt(indexH::Control);
	SCDAcontrol* control = (SCDAcontrol*)qlItem->widget();
	if (control->sLastQuery.size() > 0) {
		searchDBTable(control->sLastQuery);
	}
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
	if (vvsData.size() > 0) {
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
	connect(table, &SCDAtable::sendDeleteRow, this, &SCDA::deleteTableRow);
	connect(table, &SCDAtable::sendDeleteTable, this, &SCDA::deleteTable);
	tab->addTab(table, "Tables");
	SCDAstructure* structure = new SCDAstructure;
	tab->addTab(structure, "Structures");
	SCDAcompare* compare = new SCDAcompare;
	tab->addTab(compare, "Compare");
	connect(compare, &SCDAcompare::sendDownloadMap, this, &SCDA::testMap);

	QJPROGRESSBAR* qjPBar = new QJPROGRESSBAR;
	connect(this, &SCDA::barMessage, qjPBar, &QJPROGRESSBAR::barMessage);
	connect(this, &SCDA::initProgress, qjPBar, &QJPROGRESSBAR::initProgress);
	vLayout->insertWidget(indexV::PBar, qjPBar, 0);

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

	// Launch a catalogue manager/writer thread. It will pull the top catalogue from the work 
	// queue until all catalogues have been inserted.
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
	QLayoutItem* qlItem = mainLayout->itemAt(indexH::Display);
	QVBoxLayout* displayLayout = (QVBoxLayout*)qlItem->layout();
	qlItem = displayLayout->itemAt(indexV::PBar);
	QJPROGRESSBAR* qjPBar = (QJPROGRESSBAR*)qlItem->widget();
	qjPBar->initChildren();

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
}
