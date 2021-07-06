#include "stdafx.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"

using namespace std;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    initialize();
    initGUI();
    initImgFont("Sylfaen");
    this->showMaximized();
}
MainWindow::~MainWindow()
{
    delete ui;
}

// Initialization functions.
void MainWindow::initGUI()
{
    QString qTemp;
    int inum, nls, navSearchTerms;
    size_t pos1, pos2;

    // navSearch.
    pos1 = savedSettings.find("//navSearch");
    pos2 = savedSettings.find("\r\n\r\n", pos1);
    if (pos2 > savedSettings.size())
    {
        pos2 = savedSettings.find("\n\n", pos1);
        if (pos2 > savedSettings.size()) { jf.err("Parsing navSearch-MainWindow.initGUI"); }
    }
    string navSearchBlob = savedSettings.substr(pos1, pos2 - pos1);
    navSearch = sc.parseNavSearch(navSearchBlob);

    // Progress bar.
    barReset(100, "");

    // Local drive combo box.
    LPWSTR bufferW = new WCHAR[500];
    DWORD dsize = GetLogicalDriveStringsW(500, bufferW);
    wstring wDrives(bufferW, dsize), wTemp;
    delete[] bufferW;
    pos1 = 0, pos2 = wDrives.find(L'\0');
    while (pos1 < wDrives.size())
    {
        wTemp = wDrives.substr(pos1, pos2 - pos1);
        qTemp = "  " + QString::fromStdWString(wTemp);
        ui->cB_drives->addItem(qTemp);
        pos1 = pos2 + 1;
        pos2 = wDrives.find(L'\0', pos1);
    }
    pos1 = savedSettings.find("//cB_drives");
    pos1 = savedSettings.find("recent_index=", pos1) + 13;
    pos2 = savedSettings.find_first_not_of("1234567890", pos1);
    string temp = savedSettings.substr(pos1, pos2 - pos1);
    try { inum = stoi(temp); }
    catch (invalid_argument) { jf.err("stoi-MainWindow.initGUI"); }
    ui->cB_drives->setCurrentIndex(inum);

    // Database catalogue tree.
    updateDBCata();

    // Plain Text Edit.
    pteDefault = ui->pte_search;

    // Local map paint widget.
    ui->qp_maplocal->initialize();

}
void MainWindow::initialize()
{
    string temp = sroot + "\\SCDA-Explorer Settings.txt";
    savedSettings = jf.load(temp);
    string db_path = sroot + "\\SCDA.db";

    // Adjust the resolution.
    on_pB_resolution_clicked();

    // Initialize the class objects.
    sb.setErrorPath(sroot + "\\SCDA SWITCHBOARD Error Log.txt");
    sf.init(db_path);
}
void MainWindow::initImgFont(string fontName)
{
    string fontDir = sroot + "\\font\\" + fontName;
    string filePath;
    for (int ii = 32; ii <= 256; ii++)
    {
        filePath = fontDir + "\\" + to_string(ii) + ".png";
        if (wf.file_exist(filePath))
        {
            im.initGlyph(filePath, ii);
        }
    }
}
void MainWindow::GetDesktopResolution(int& horizontal, int& vertical)
{
    RECT desktop;
    const HWND hDesktop = GetDesktopWindow();
    GetWindowRect(hDesktop, &desktop);
    horizontal = desktop.right;
    vertical = desktop.bottom;
}

// Progress bar functions.
void MainWindow::barUpdate(int iCurrent)
{
    int min, max;
    QString pb_text;
    lock_guard<mutex> lock(m_bar);
    min = ui->progressbar->minimum();
    max = ui->progressbar->maximum();
    if (iCurrent < min || iCurrent > max) { jf.err("Outside boundaries-MainWindow.barUpdate"); }
    ui->progressbar->setValue(iCurrent);
}
void MainWindow::barReset(int iMax, string message)
{
    if (iMax <= 0) { jf.err("Invalid maximum-MainWindow.barReset"); }
    QString qMessage = QString::fromStdString(message);
    lock_guard<mutex> lock(m_bar);
    ui->progressbar->setMinimum(0);
    ui->progressbar->setMaximum(iMax);
    ui->progressbar->setValue(0);
    ui->qlabel_progressbar->setText(qMessage);
}
void MainWindow::barMessage(string message)
{
    QString qmessage = QString::fromStdString(message);
    lock_guard<mutex> lock(m_bar);
    ui->qlabel_progressbar->setText(qmessage);
    QCoreApplication::processEvents();
}

// PlainTextEdit functions.
void MainWindow::qshow(string message)
{
    QString qMessage = QString::fromUtf8(message.c_str(), -1);
    pteDefault->setPlainText(qMessage);
}
void MainWindow::qshow(vector<string> message)
{
    QString qMessage;
    for (int ii = 0; ii < message.size(); ii++)
    {
        if (ii > 0) { qMessage += "\n"; }
        qMessage += QString::fromUtf8(message[ii].c_str(), -1);
    }  
    pteDefault->setPlainText(qMessage);
}
void MainWindow::reportTable(QTableWidget*& qTable)
{
    QString qTitle = qTable->verticalHeaderItem(0)->text();
    if (qTitle.size() < 1) { qTitle = "Unknown"; }
    int numRow = qTable->rowCount();
    int numCol = qTable->columnCount();
    QString qMessage = "Displaying table " + qTitle + "\nRows: ";
    qMessage += QString::number(numRow) + "\nColumns: " + QString::number(numCol);
    qMessage += "\nLoad time(ms): " + QString::number(time);
    pteDefault->clear();
    pteDefault->setPlainText(qMessage);
}

// Table shared functions.
void MainWindow::tablePopulate(QTableWidget*& qTable, vector<vector<string>>& sData)
{
    // Note: The zeroth row and column are used as row/column titles. 
    qTable->clear();
    QStringList hHeaderLabels, vHeaderLabels;
    QString qTemp;
    for (int ii = 1; ii < sData[0].size(); ii++)
    {
        qTemp = QString::fromStdString(sData[0][ii]);
        hHeaderLabels.append(qTemp);
    }
    for (int ii = 1; ii < sData.size(); ii++)
    {
        qTemp = QString::fromStdString(sData[ii][0]);
        vHeaderLabels.append(qTemp);
    }

    QTableWidgetItem* qCell;
    qTable->setColumnCount(sData[0].size() - 1);
    qTable->setRowCount(sData.size() - 1);
    for (int ii = 1; ii < sData.size(); ii++)
    {
        for (int jj = 1; jj < sData[ii].size(); jj++)
        {
            qTemp = QString::fromStdString(sData[ii][jj]);
            qCell = new QTableWidgetItem(qTemp);
            qTable->setItem(ii - 1, jj - 1, qCell);
        }
    }
    qTable->setHorizontalHeaderLabels(hHeaderLabels);
    qTable->setVerticalHeaderLabels(vHeaderLabels);
}
void MainWindow::on_tableW_maplocal_currentCellChanged(int RowNow, int ColNow, int RowThen, int ColThen)
{
    if (RowNow < 0 || RowNow == RowThen) { return; }
    QTableWidgetItem* qCell = ui->tableW_maplocal->verticalHeaderItem(RowNow);
    if (qCell == nullptr) { qDebug() << "Header item is NULL !"; }
    QString qTemp = qCell->text();
    string regionName = qTemp.toStdString();
    ui->qp_maplocal->drawSelectedDot(regionName);
}

// Multi-purpose functions.
void MainWindow::autoExpand(QTreeWidget*& qTree, int maxNum)
{
    QTreeWidgetItem* qRoot = qTree->topLevelItem(0);
    int numKids = qRoot->childCount();
    if (numKids > maxNum) { return; }
    qRoot->setExpanded(1);
    QList<QTreeWidgetItem*> gen1;
    for (int ii = 0; ii < numKids; ii++)
    {
        gen1.append(qRoot->child(ii));
    }
    for (int ii = 0; ii < gen1.size(); ii++)
    {
        numKids += gen1[ii]->childCount();
    }
    if (numKids > maxNum) { return; }
    for (int ii = 0; ii < gen1.size(); ii++)
    {
        gen1[ii]->setExpanded(1);
    }
}
void MainWindow::bind(string& stmt, vector<string>& param)
{
    // Replace '?' placeholders in a template statement with given parameter strings.

    string temp;
    size_t pos1 = 0;
    size_t count = 0;
    while (pos1 < stmt.size())
    {
        pos1 = stmt.find('?', pos1 + 1);
        if (pos1 < stmt.size())
        {
            count++;
        }
    }
    if (count != param.size())
    {
        jf.err("parameter count mismatch-MainWindow.bind");
    }

    pos1 = 0;
    for (int ii = 0; ii < (int)count; ii++)
    {
        pos1 = stmt.find('?', pos1 + 1);
        temp = "'" + param[ii] + "'";
        stmt.replace(pos1, 1, temp);
    }
}
void MainWindow::thrDownload(SWITCHBOARD& sbgui)
{
    // Downloads a file using a new thread.
    thread::id myid = this_thread::get_id();
    vector<int> mycomm;
    sbgui.answer_call(myid, mycomm);
    vector<string> prompt = sbgui.get_prompt();  // Form [url, filePath, type].
    size_t pos1 = prompt[1].rfind('\\');
    string folderPath = prompt[1].substr(0, pos1);
    wf.makeDir(folderPath);
    int type;
    try { type = stoi(prompt[2]); }
    catch (invalid_argument) { jf.err("stoi-MainWindow.thrDownload"); }
    wf.download(prompt[0], prompt[1], type);
    mycomm[0] = 1;
    sbgui.update(myid, mycomm);
}
void MainWindow::thrFileSplitter(SWITCHBOARD& sbgui, int& progress, mutex& m_progress)
{
    thread::id myid = this_thread::get_id();
    vector<int> mycomm;
    sbgui.answer_call(myid, mycomm);
    vector<string> prompt = sbgui.get_prompt();  // Form [filePath, maxSize(bytes)].
    long long maxSize;
    try { maxSize = stoll(prompt[1]); }
    catch (invalid_argument) { jf.err("stoll-MainWindow.thrFileSplitter"); }
    long long fileSize = wf.getFileSize(prompt[0]);
    mycomm[2] = (fileSize / maxSize) + 1;
    sbgui.update(myid, mycomm);
    mycomm[3] = wf.fileSplitter(prompt[0], maxSize, progress, m_progress);
    mycomm[0] = 1;
    sbgui.update(myid, mycomm);
}
void MainWindow::thrUnzip(SWITCHBOARD& sbgui)
{
    thread::id myid = this_thread::get_id();
    vector<int> mycomm;
    sbgui.answer_call(myid, mycomm);
    vector<string> filePath = sbgui.get_prompt();
    zf.unzip(filePath[0]);
    mycomm[0] = 1;
    sbgui.update(myid, mycomm);
}

// Mouse functions.
void MainWindow::mousePressEvent(QMouseEvent* event)
{
    QPoint pointClick;
    QString qTemp, qsX, qsY;
    vector<int> clickCoord(2);
    vector<string> promptUpdate;
    if (event->button() == Qt::LeftButton)
    {
        clickCoord[0] = event->x();
        clickCoord[1] = event->y();
    }
    else if (event->button() == Qt::MiddleButton)
    {
        ui->checkB_override->setChecked(1);
        clickCoord[0] = event->x();
        clickCoord[1] = event->y();
    }
    
}

// Database searching.
void MainWindow::on_pB_search_clicked()
{
    QString qTemp = ui->pte_search->toPlainText();
    string query = qTemp.toStdString(), sMessage;
    vector<string> tableList;
    size_t pos1;
    bool wildcard = 0;
    if (query.size() > 0)
    {
        while (query.back() == ' ') { query.pop_back(); }
        pos1 = query.find('*');
        if (pos1 < query.size()) { wildcard = 1; }
    }
    if (query == "")
    {
        ui->listW_searchresult->clear();
        sf.all_tables(tableList);
        for (int ii = 0; ii < tableList.size(); ii++)
        {
            qTemp = QString::fromStdString(tableList[ii]);
            ui->listW_searchresult->addItem(qTemp);
        }
        ui->tabW_main->setCurrentIndex(2);
        sMessage = "Displaying " + to_string(tableList.size());
        sMessage += " tables from the search criteria 'all'.";
        qTemp = QString::fromStdString(sMessage);
        ui->pte_search->setPlainText(qTemp);
    }
    else if (!wildcard)
    {
        ui->listW_searchresult->clear();
        sf.all_tables(tableList);
        for (int ii = 0; ii < tableList.size(); ii++)
        {
            if (tableList[ii] == query)
            {
                qTemp = QString::fromStdString(tableList[ii]);
                ui->listW_searchresult->addItem(qTemp);
            }
        }
        int numTables = ui->listW_searchresult->count();
        if (numTables == 1)
        {
            ui->listW_searchresult->item(0)->setSelected(1);
            on_pB_viewtable_clicked();
        }
    }
}
void MainWindow::on_listW_searchresult_itemSelectionChanged()
{
    QList<QListWidgetItem*> qSel = ui->listW_searchresult->selectedItems();
    if (qSel.size() == 0)
    {
        ui->pB_viewtable->setEnabled(0);
        ui->pB_deletetable->setEnabled(0);
    }
    else
    {
        ui->pB_viewtable->setEnabled(1);
        ui->pB_deletetable->setEnabled(1);
        ui->pB_deletetable->setText("Delete\nTable");
    }
}
void MainWindow::on_listW_searchresult_itemDoubleClicked(QListWidgetItem* qItem)
{
    on_pB_viewtable_clicked();
}

// Local catalogue display.
void MainWindow::on_cB_drives_currentTextChanged(const QString& arg1)
{
    string sDrive = arg1.toStdString();
    sDrive.pop_back();
    while (sDrive[0] == ' ') { sDrive.erase(sDrive.begin()); }
    jtCataLocal.init("Local", sDrive);
    vector<vector<int>> comm(1, vector<int>());
    comm[0].assign(comm_length, 0);
    thread::id myid = this_thread::get_id();
    vector<string> prompt = { sDrive };
    sb.set_prompt(prompt);
    sb.start_call(myid, 1, comm[0]);
    std::thread thr(&MainWindow::scanLocalCata, this, ref(sb), ref(jtCataLocal));
    thr.detach();
    ui->tabW_main->setCurrentIndex(0);
    while (1)
    {
        Sleep(gui_sleep);
        QCoreApplication::processEvents();
        comm = sb.update(myid, comm[0]);
        if (comm.size() > 1 && comm[1][0] == 1) { break; }
    }
    sb.end_call(myid);
    barMessage("Displaying local catalogues from drive " + sDrive);
    qf.populateTree(ui->treeW_catalocal, jtCataLocal, 1);
    autoExpand(ui->treeW_catalocal, treeLength);
}
void MainWindow::scanLocalCata(SWITCHBOARD& sbgui, JTREE& jtgui)
{
    thread::id myid = this_thread::get_id();
    vector<int> mycomm;
    sbgui.answer_call(myid, mycomm);
    vector<string> prompt = sbgui.get_prompt();
    string search = "*", yearPath, cataPath, metaPath;
    vector<string> folderList = wf.get_folder_list(prompt[0], search);
    vector<string> sYearList, cataList;
    vector<int> iYearList, csvCount;
    size_t pos1;
    bool meta;
    int iYear;
    for (int ii = 0; ii < folderList.size(); ii++)
    {
        pos1 = folderList[ii].find_first_not_of("1234567890");
        if (pos1 > folderList[ii].size())
        {
            try { iYear = stoi(folderList[ii]); }
            catch (invalid_argument) { jf.err("stoi-MainWindow-on_cB_drives"); }
            if (iYear >= 1981 && iYear <= 2017)
            {
                iYearList.push_back(iYear);
            }
        }
    }
    jf.isort_ilist(iYearList, JFUNC::Increasing);
    sYearList.resize(iYearList.size());
    for (int ii = 0; ii < sYearList.size(); ii++)
    {
        sYearList[ii] = to_string(iYearList[ii]);
    }
    jtCataLocal.addChildren(sYearList, iYearList, -1);
    for (int ii = 0; ii < iYearList.size(); ii++)
    {
        yearPath = prompt[0] + "\\" + sYearList[ii];
        folderList = wf.get_folder_list(yearPath, search);
        csvCount.resize(folderList.size());
        for (int jj = 0; jj < folderList.size(); jj++)
        {
            cataPath = yearPath + "\\" + folderList[jj];
            csvCount[jj] = wf.get_file_path_number(cataPath, "csv");
            metaPath = cataPath + "\\" + folderList[jj] + "_English_meta.txt";
            meta = wf.file_exist(metaPath);
            if (meta == 1 && csvCount[jj] > 0)
            {
                jtCataLocal.addChild(folderList[jj], csvCount[jj], iYearList[ii]);
            }
        }
    }   
    mycomm[0] = 1;
    sbgui.update(myid, mycomm);
}
void MainWindow::on_treeW_catalocal_itemSelectionChanged()
{
    QList<QTreeWidgetItem*> qList = ui->treeW_catalocal->selectedItems();
    if (qList.size() == 0)
    {
        ui->pB_insert->setEnabled(0);
        return;
    }
    int type;
    for (int ii = 0; ii < qList.size(); ii++)
    {
        type = qList[ii]->type();
        if (type == 2)
        {
            ui->pB_insert->setEnabled(1);
            break;
        }
        else if (ii == qList.size() - 1)
        {
            ui->pB_insert->setEnabled(0);
        }
    }
}

// Database catalogue display.
void MainWindow::updateDBCata()
{
    ui->treeW_catadb->clear();
    QTreeWidgetItem* qRoot = new QTreeWidgetItem(0), *qNode, *qParent;
    QString qTemp = "Database";
    qRoot->setText(0, qTemp);
    int index;
    unordered_map<string, int> mapYear, mapCata;  // Output is child index in tree.
    vector<string> search = { "Year" }, yearList, cataList;
    string tname = "Census";
    sf.select(search, tname, yearList);
    jf.isort_ilist(yearList, JFUNC::Increasing);
    for (int ii = 0; ii < yearList.size(); ii++)
    {
        mapYear.emplace(yearList[ii], ii);
        qNode = new QTreeWidgetItem(qRoot, 1);
        qTemp = QString::fromStdString(yearList[ii]);
        qNode->setText(0, qTemp);
    }
    search = { "Catalogue" };
    for (int ii = 0; ii < yearList.size(); ii++)
    {
        tname = "Census$" + yearList[ii];
        sf.select(search, tname, cataList);
        index = mapYear.at(yearList[ii]);
        qParent = qRoot->child(index);
        for (int jj = 0; jj < cataList.size(); jj++)
        {
            mapCata.emplace(cataList[jj], jj);
            qNode = new QTreeWidgetItem(qParent, 2);
            qTemp = QString::fromStdString(cataList[jj]);
            qNode->setText(0, qTemp);
        }
    }
    ui->treeW_catadb->addTopLevelItem(qRoot);
    autoExpand(ui->treeW_catadb, treeLength);
}

// Local catalogue insertion.
void MainWindow::on_pB_insert_clicked()
{
    QList<QTreeWidgetItem*> qList = ui->treeW_catalocal->selectedItems();
    QTreeWidgetItem* qParent;
    int type;
    for (int ii = 0; ii < qList.size(); ii++)
    {
        type = qList[ii]->type();
        if (type != 2)
        {
            qList.erase(qList.begin() + ii);
            ii--;
        }
    }
    thread::id myid = this_thread::get_id();
    vector<vector<int>> comm;    
    vector<string> prompt(2);  // Form [sYear, sName].
    QString qTemp;
    for (int ii = 0; ii < qList.size(); ii++)
    {
        comm.resize(1);
        comm[0].assign(comm_length, 0);
        qTemp = qList[ii]->text(0);
        prompt[1] = qTemp.toStdString();
        qParent = qList[ii]->parent();
        qTemp = qParent->text(0);
        prompt[0] = qTemp.toStdString();
        sb.set_prompt(prompt);
        sb.start_call(myid, 1, comm[0]);
        std::thread thr(&MainWindow::judicator, this, ref(sb), ref(sf));
        thr.detach();
        while (1)
        {
            Sleep(gui_sleep);
            QCoreApplication::processEvents();
            comm = sb.update(myid, comm[0]);
            if (comm.size() > 1) 
            { 
                barMessage("Indexing catalogue " + prompt[1] + " ...");
                break; 
            }
        }
        while (comm[0][0] == 0)
        {
            Sleep(gui_sleep);
            QCoreApplication::processEvents();
            comm = sb.update(myid, comm[0]);
            if (comm[0][2] == 0 && comm[1][2] > 0)
            {
                comm[0][2] = comm[1][2];
                barReset(comm[0][2], "Inserting catalogue " + prompt[1] + " ...");
            }
            if (comm[1][1] > comm[0][1])
            {
                comm[0][1] = comm[1][1];
                barUpdate(comm[0][1]);
            }
            if (comm[1][0] == 1)
            {
                barUpdate(comm[0][2]);
                barMessage("Finished inserting catalogue " + prompt[1]);
                comm[0][0] = 1;
            }
        }
        sb.end_call(myid);
        updateDBCata();
    }
}
void MainWindow::judicator(SWITCHBOARD& sbgui, SQLFUNC& sfgui)
{
    thread::id myid = this_thread::get_id();
    vector<int> mycomm;
    sbgui.answer_call(myid, mycomm);
    vector<string> prompt = sbgui.get_prompt();  // Form [sYear, sName].
    string cataPath = sroot + "\\" + prompt[0] + "\\" + prompt[1];
    sc.init(cataPath);
    vector<string> DIMList, dimList, colTitles;
    string result, stmt, temp;

    // Convert the Stats Can geo file to one suited for split CSV files.
    string geoPath = cataPath + "\\Geo.txt";
    if (!wf.file_exist(geoPath))
    {
        string geoPathOld = cataPath + "\\Geo_starting_row_CSV.csv";
        sc.convertSCgeo(geoPathOld);
    }

    // Determine the number of regions, and update the GUI thread progress bar.
    vector<vector<string>> geoList = sc.loadGeoList(geoPath);
    unordered_map<string, string> mapGeoPart;
    for (int ii = 0; ii < geoList.size(); ii++)
    {
        mapGeoPart.emplace(geoList[ii][0], geoList[ii][2]);
    }
    mycomm[2] = geoList.size();
    string tname = "Census$" + prompt[0] + "$" + prompt[1] + "$Geo";
    vector<string> vsResult, search = { "GEO_CODE" };
    if (sfgui.table_exist(tname))
    {
        sfgui.select(search, tname, vsResult);
    }
    vector<string> geoToDo = sc.compareGeoListDB(geoList, vsResult);  // Form [# already in DB, geoIndex of first absence].
    if (geoToDo.size() < 1) { goto FTL1; }
    mycomm[1] += (geoList.size() - geoToDo.size());
    sbgui.update(myid, mycomm);
    try { temp = mapGeoPart.at(geoToDo[0]); }
    catch (out_of_range) { jf.err("mapGeoPart-MainWindow.judicator"); }
    sc.initCSV(geoToDo[0], temp);

    // Add this year to the root census table. 
    tname = "Census";
    if (!sfgui.table_exist(tname))
    {
        result = sc.makeCreateCensus();
        sfgui.executor(result);
    }
    result = sc.makeInsertCensus();
    sfgui.executor(result);

    // Add this catalogue (and its topic) to the 'Year' table.
    tname = "Census$" + prompt[0];
    if (!sfgui.table_exist(tname))
    {
        result = sc.makeCreateYear();
        sfgui.executor(result);
    }
    result = sc.makeInsertYear();
    sfgui.executor(result);

    // Add this catalogue to the appropriate 'Topic' table.
    tname = "Census$" + prompt[0] + "$Topic";
    if (sfgui.table_exist(tname)) { sfgui.get_col_titles(tname, colTitles); }
    vsResult = sc.makeCreateInsertTopic(colTitles);
    sfgui.executor(vsResult);

    // Create the catalogue's geo table. Rows are added later, with the CSV data.
    result = sc.makeCreateGeo();
    sfgui.executor(result);

    // Insert DIM/Dim tables.
    vsResult = sc.makeCreateInsertDIMIndex(DIMList);
    sfgui.executor(vsResult);
    vsResult = sc.makeCreateInsertDIM();
    sfgui.executor(vsResult);
    vsResult = sc.makeCreateInsertDim(dimList);
    sfgui.executor(vsResult);

    // Create a data table for each GEO_CODE region.
    vsResult = sc.makeCreateData(DIMList, dimList);
    sfgui.insert_prepared(vsResult);
    for (int ii = 0; ii < geoToDo.size(); ii++)
    {
        vsResult = sc.makeInsertData(geoToDo[ii], result);
        sfgui.executor(result);
        sfgui.insertPreparedBind(vsResult);
        mycomm[1]++;
        sbgui.update(myid, mycomm);
    }

    // Add ancestry columns to the geo table, if necessary.
    vsResult.clear();
    search = { "GEO_LEVEL" };
    tname = "Census$" + prompt[0] + "$" + prompt[1] + "$Geo";
    sfgui.select(search, tname, vsResult);
    int numCol = sfgui.get_num_col(tname), maxLevel = -1, inum, iGeoLevel;
    for (int ii = 0; ii < vsResult.size(); ii++)
    {
        try { inum = stoi(vsResult[ii]); }
        catch (invalid_argument) { jf.err("stoi-sc.makeInsertData"); }
        if (inum > maxLevel) { maxLevel = inum; }
    }
    int needCol = maxLevel + 3 - numCol;
    if (needCol > 0)
    {
        inum = numCol - 3;
        for (int ii = 0; ii < needCol; ii++)
        {
            result = "ALTER TABLE \"" + tname + "\" ADD COLUMN Ancestor";
            result += to_string(inum + ii) + " INT;";
            sfgui.executor(result);
        }
    }

FTL1:
    // Insert ancestry values into the geo table.
    vector<string> ancestry, conditions, revisions;
    for (int ii = 0; ii < geoList.size(); ii++)
    {
        result.clear();
        revisions.clear();
        conditions = { "GEO_CODE = " + geoList[ii][0] };
        sfgui.select(search, tname, result, conditions);
        try { iGeoLevel = stoi(result); }
        catch (invalid_argument) { jf.err("stoi-MainWindow.judicator"); }
        if (iGeoLevel >= ancestry.size())
        {
            ancestry.push_back(geoList[ii][0]);
        }
        else
        {
            ancestry[iGeoLevel] = geoList[ii][0];
        }
        for (int jj = 0; jj < iGeoLevel; jj++)
        {
            temp = "Ancestor" + to_string(jj) + " = " + ancestry[jj];
            revisions.push_back(temp);
        }
        if (revisions.size() > 0)
        {
            sfgui.update(tname, revisions, conditions);
        }
    }

    // Report completion to the GUI thread.
    mycomm[0] = 1;
    sbgui.update(myid, mycomm);
}

// Local map listings.
void MainWindow::on_pB_maplocal_clicked()
{
    QString qTemp;
    vector<vector<int>> comm(1, vector<int>());
    comm[0].assign(comm_length, 0);
    thread::id myid = this_thread::get_id();
    vector<string> prompt, binNameList;
    int tabIndex = ui->tabW_main->currentIndex();
    switch (tabIndex)
    {
    case 0:  // Search the local drive for map folders (mapsBIN, mapsPNG, mapsPDF, mapsSHD).
    {        
        qTemp = ui->cB_drives->currentText();
        string sDrive = qTemp.toStdString(), sCata, sYear;
        sDrive.pop_back();
        while (sDrive[0] == ' ') { sDrive.erase(sDrive.begin()); }
        jtMapLocal.init("Local Maps", sDrive);
        prompt = { sDrive };
        sb.set_prompt(prompt);
        sb.start_call(myid, 1, comm[0]);
        std::thread thr(&MainWindow::scanLocalMap, this, ref(sb), ref(jtMapLocal));
        thr.detach();
        while (1)
        {
            Sleep(gui_sleep);
            QCoreApplication::processEvents();
            comm = sb.update(myid, comm[0]);
            if (comm.size() > 1 && comm[1][0] == 1) { break; }
        }
        sb.end_call(myid);
        barMessage("Displaying local maps from drive " + sDrive);
        qf.populateTree(ui->treeW_maplocal, jtMapLocal, 2);
        QTreeWidgetItem* qNode, * qRoot = ui->treeW_maplocal->topLevelItem(0);
        qRoot->setExpanded(1);
        int numKids = qRoot->childCount();
        for (int ii = 0; ii < numKids; ii++)
        {
            qNode = qRoot->child(ii);
            qNode->setExpanded(1);
        }
        ui->tabW_main->setCurrentIndex(1);
        break;
    }
    case 1:  // Make a list of bin maps within the selected folder.
    {
         // Note that the folder path is the list's invisible zeroth item.
        QList<QTreeWidgetItem*> qSel = ui->treeW_maplocal->selectedItems();
        if (qSel.size() != 1) { return; }
        ui->listW_maplocal->clear();
        qTemp = ui->cB_drives->currentText();
        string sDrive = qTemp.toStdString(), temp, temp8;
        sDrive.pop_back();
        while (sDrive[0] == ' ') { sDrive.erase(sDrive.begin()); }
        string folderPath = qf.getBranchPath(qSel[0], sDrive);
        qTemp = QString::fromStdString(folderPath);
        ui->listW_maplocal->addItem(qTemp);
        ui->listW_maplocal->item(0)->setHidden(1);
        binNameList = wf.get_file_list(folderPath, "*.bin");
        size_t pos1;
        for (int ii = 0; ii < binNameList.size(); ii++)
        {
            if (ignorePartie)
            {
                pos1 = binNameList[ii].find("partie");
                if (pos1 < binNameList[ii].size()) { continue; }
            }
            pos1 = binNameList[ii].rfind(".bin");
            temp = binNameList[ii].substr(0, pos1);
            temp8 = jf.asciiToUTF8(temp);
            qTemp = QString::fromUtf8(temp8.c_str());
            ui->listW_maplocal->addItem(qTemp);
        }
        break;
    }
    }

}
void MainWindow::scanLocalMap(SWITCHBOARD& sbgui, JTREE& jtgui)
{
    thread::id myid = this_thread::get_id();
    vector<int> mycomm, treePLi;
    sbgui.answer_call(myid, mycomm);
    vector<string> prompt = sbgui.get_prompt();
    vector<string> mapFolders = { "mapsBIN", "mapsPDF", "mapsPNG", "mapsSHD" };
    vector<string> mapExt = { ".bin", ".pdf", ".png", ".shd" };
    vector<vector<int>> treeST;
    vector<string> treePL;
    string temp, folderRoot, treeRoot;
    int finalFolderIndex;
    for (int ii = 0; ii < mapFolders.size(); ii++)
    {
        folderRoot = prompt[0] + "\\" + mapFolders[ii];
        treeST = { {0} };
        treePL = { folderRoot };                    // Firstly, make a tree from
        wf.getTreeFolder(0, treeST, treePL);        // the relevant subfolders.        
        treePLi.clear();
        treePLi.resize(treeST.size());
        finalFolderIndex = treeST.size() - 1;
        for (int jj = 0; jj <= finalFolderIndex; jj++)
        {
            treePLi[jj] = wf.get_file_path_number(treePL[jj], mapExt[ii]);
        }
        treeRoot = jtgui.getRootName();
        jtgui.addBranchSTPL(treeST, treePL, treePLi, treeRoot);       
    }
    jtgui.setRemovePath(1);
    mycomm[0] = 1;
    sbgui.update(myid, mycomm);
}
void MainWindow::on_treeW_maplocal_itemSelectionChanged()
{
    recentClick = ui->treeW_maplocal;
    QList<QTreeWidgetItem*> qList = ui->treeW_maplocal->selectedItems();
    if (qList.size() == 0)
    {
        ui->pB_convert->setEnabled(0);
        ui->pB_reviewmap->setEnabled(0);
        return;
    }
    QTreeWidgetItem* qParent = qList[0]->parent();
    if (qParent == nullptr)
    {
        ui->pB_convert->setEnabled(0);
        ui->pB_reviewmap->setEnabled(0);
        return;
    }
    ui->pB_convert->setEnabled(1);
    ui->pB_reviewmap->setEnabled(1);
}

// Local map conversion.
void MainWindow::on_pB_convert_clicked()
{
    vector<vector<int>> comm(1, vector<int>());
    comm[0].assign(comm_length, 0);
    thread::id myid = this_thread::get_id();
    int mode = 0;
    if (recentClick == ui->treeW_maplocal) { mode = 1; }
    else if (recentClick = ui->listW_maplocal) { mode = 2; }
    switch (mode)
    {
    case 0:  // Nothing found.
    {
        jf.err("No recentClick-MainWindow.on_pB_convert_clicked");
        break;
    }
    case 1:  // Tree widget.
    {
        QList<QTreeWidgetItem*> qSel = ui->treeW_maplocal->selectedItems();
        if (qSel.size() != 1) { return; }
        string folderPath = qf.getBranchPath(qSel[0], jtMapLocal.pathRoot);
        size_t pos1 = folderPath.find("mapsPDF");
        if (pos1 < folderPath.size()) { qshow("PDF->PNG needs work."); return; }
        pos1 = folderPath.find("mapsPNG");
        if (pos1 < folderPath.size()) { qshow("PNG->BIN needs work."); return; }
        pos1 = folderPath.find("mapsBIN");
        if (pos1 < folderPath.size())
        {
            qshow("Launching BIN map upgrades...");
            vector<string> prompt = { folderPath };
            sb.set_prompt(prompt);
            sb.start_call(myid, 1, comm[0]);
            std::thread thr(&MainWindow::upgradeBinMap, this, ref(sb));
            thr.detach();
            ui->tabW_main->setCurrentIndex(1);
            while (1)
            {
                Sleep(gui_sleep);
                QCoreApplication::processEvents();
                comm = sb.update(myid, comm[0]);
                if (comm.size() > 1) { break; }
            }
            while (comm[0][0] == 0)
            {
                Sleep(gui_sleep);
                QCoreApplication::processEvents();
                comm = sb.update(myid, comm[0]);
                if (comm[0][2] == 0 && comm[1][2] > 0)
                {
                    comm[0][2] = comm[1][2];
                    barReset(comm[0][2], "Upgrading BIN maps in " + prompt[0] + " ...");
                }
                if (comm[1][1] > comm[0][1])
                {
                    comm[0][1] = comm[1][1];
                    barUpdate(comm[0][1]);
                }
                if (comm[1][0] == 1)
                {
                    barUpdate(comm[0][2]);
                    comm[0][0] = 1;
                }
            }
            sb.end_call(myid);
            barMessage("Finished upgrading BIN maps from " + prompt[0]);
        }
        break;
    }
    case 2:  // List widget.
    {
        QList<QListWidgetItem*> qSel = ui->listW_maplocal->selectedItems();
        if (qSel.size() != 1) { return; }
        QString qTemp = ui->listW_maplocal->item(0)->text();
        qTemp.append("\\" + qSel[0]->text() + ".bin");
        string binPath8 = qTemp.toStdString();
        vector<string> prompt = { jf.utf8ToAscii(binPath8) };
        sb.set_prompt(prompt);
        sb.start_call(myid, 1, comm[0]);
        std::thread thr(&MainWindow::upgradeBinMap, this, ref(sb));
        thr.detach();
        while (1)
        {
            Sleep(gui_sleep);
            QCoreApplication::processEvents();
            comm = sb.update(myid, comm[0]);
            if (comm.size() > 1) { break; }
        }
        while (comm[0][0] == 0)
        {
            Sleep(gui_sleep);
            QCoreApplication::processEvents();
            comm = sb.update(myid, comm[0]);
            if (comm[1][0] == 1)
            {
                comm[0][0] = 1;
            }
        }
        sb.end_call(myid);
        barMessage("Finished upgrading BIN map " + binPath8);
        break;
    }
    }

}
void MainWindow::upgradeBinMap(SWITCHBOARD& sbgui)
{
    thread::id myid = this_thread::get_id();
    vector<int> mycomm;
    sbgui.answer_call(myid, mycomm);
    vector<string> prompt = sbgui.get_prompt(), binNameList, dirt, soap;
    vector<vector<vector<int>>> frames;
    vector<vector<int>> border;
    vector<double> positionGPS, positionPNG;
    double scale;
    string temp, binPath, binFile, binFileNew, parent, myName, pngPath;
    size_t pos1 = prompt[0].find(".bin");
    if (pos1 < prompt[0].size())
    {
        pos1 = prompt[0].rfind('\\') + 1;
        binNameList = { prompt[0].substr(pos1) };
    }
    else { binNameList = wf.get_file_list(prompt[0], "*.bin"); }
    mycomm[2] = binNameList.size();
    sbgui.update(myid, mycomm);
    for (int ii = 0; ii < binNameList.size(); ii++)
    {
        binFileNew.clear();
        pos1 = binNameList[ii].find("(Canada)");
        if (pos1 < binNameList[ii].size()) { continue; }
        if (ignorePartie)
        {
            pos1 = binNameList[ii].find("partie");
            if (pos1 < binNameList[ii].size()) { continue; }
        }

        pos1 = prompt[0].find(".bin");
        if (pos1 < prompt[0].size()) { binPath = prompt[0]; }
        else { binPath = prompt[0] + "\\" + binNameList[ii]; }
        binFile = wf.load(binPath);

        frames = sc.readBinFrames(binFile);
        if (frames.size() != 3) { qDebug() << binNameList[ii].c_str() << " needs FRAMES"; }
        else { binFileNew += sc.makeBinFrames(frames) + "\n\n"; }
        
        scale = sc.readBinScale(binFile);
        if (scale < 0.0) { qDebug() << binNameList[ii].c_str() << " needs SCALE"; }
        else { binFileNew += sc.makeBinScale(scale) + "\n\n"; }
        
        parent = sc.readBinParent(binFile);
        if (parent.size() < 1) { binFileNew += sc.makeBinParentNew(binPath, parent) + "\n\n"; }
        else { binFileNew += sc.makeBinParent(parent) + "\n\n"; }
        
        positionPNG = sc.readBinPositionPNG(binFile);
        if (positionPNG[0] < 0.0)
        {
            dirt = { "mapsBIN", ".bin" };
            soap = { "pos", ".png" };
            pngPath = binPath;
            jf.clean(pngPath, dirt, soap);
            positionPNG = qf.makeBlueDot(pngPath);
            binFileNew += sc.makeBinPositionPNG(positionPNG) + "\n\n";
        }
        else { binFileNew += sc.makeBinPositionPNG(positionPNG) + "\n\n"; }

        positionGPS = sc.readBinPositionGPS(binFile);
        if (positionGPS[0] < -180.0)
        {
            pos1 = binNameList[ii].rfind(".bin");
            myName = binNameList[ii].substr(0, pos1);
            temp = jf.utf8ToAscii(parent);
            positionGPS = io.getBinPosition(myName, temp);
            binFileNew += sc.makeBinPositionGPS(positionGPS) + "\n\n";
        }
        else { binFileNew += sc.makeBinPositionGPS(positionGPS) + "\n\n"; }

        border = sc.readBinBorder(binFile);
        if (border.size() < 1) { qDebug() << binNameList[ii].c_str() << " needs BORDER"; }
        else { binFileNew += sc.makeBinBorder(border) + "\n\n"; }

        wf.printer(binPath, binFileNew);
        mycomm[1]++;
        sbgui.update(myid, mycomm);
    }
    mycomm[0] = 1;
    sbgui.update(myid, mycomm);
}

// Local map review.
void MainWindow::on_pB_reviewmap_clicked()
{
    /*
    QList<QListWidgetItem*> qSel = ui->listW_maplocal->selectedItems();
    if (qSel.size() != 1) { return; }
    QString qTemp = ui->listW_maplocal->item(0)->text();
    qTemp.append("\\" + qSel[0]->text() + ".bin");
    string temp8 = qTemp.toStdString();
    vector<string> prompt = { jf.utf8ToAscii(temp8) };
    vector<BINMAP> binMaps;
    vector<vector<int>> comm(1, vector<int>());
    comm[0].assign(comm_length, 0);
    thread::id myid = this_thread::get_id();
    sb.set_prompt(prompt);
    sb.start_call(myid, 1, comm[0]);
    std::thread thr(&MainWindow::populateBinFamily, this, ref(sb), ref(binMaps));
    thr.detach();
    while (1)
    {
        Sleep(gui_sleep);
        QCoreApplication::processEvents();
        comm = sb.update(myid, comm[0]);
        if (comm.size() > 1 && comm[1][0] == 1) { break; }
    }
    sb.end_call(myid);
    ui->qp_maplocal->drawFamilyBlue(binMaps);
    vector<vector<string>> tableData = getBinGpsTable(binMaps);
    tablePopulate(ui->tableW_maplocal, tableData);
    string temp = to_string(binMaps.size() - 1);
    string sMessage = "Displaying parent region " + binMaps[0].myName;
    sMessage += " and " + temp + " child regions.";
    barMessage(sMessage);
    */
}
void MainWindow::populateBinFamily(SWITCHBOARD& sbgui, vector<BINMAP>& vBM)
{
    /*
    thread::id myid = this_thread::get_id();
    vector<int> mycomm;
    sbgui.answer_call(myid, mycomm);
    vector<string> prompt = sbgui.get_prompt();
    BINMAP binChild;
    binChild.loadFromPath(prompt[0]);
    string pathParent = binChild.getPathParent();
    string pathGeo = binChild.getPathGeo();
    vBM.clear();
    int index = 0;
    BINMAP binBlank;
    vBM.emplace_back(binBlank);
    vBM[index].loadFromPath(pathParent);
    vBM[index].childrenFromGeo(pathGeo);
    vector<string> pathList = vBM[index].getPathChildren(1);
    for (int ii = 0; ii < pathList.size(); ii++)
    {
        index = vBM.size();
        BINMAP binBlank;
        vBM.emplace_back(binBlank);
        vBM[index].loadFromPath(pathList[ii]);
        vBM[index].makeBlueDot();
        if (pathList[ii] == prompt[0]) { vBM[index].isSelected = 1; mycomm[1]++; }
    }
    mycomm[0] = 1;
    sbgui.update(myid, mycomm);
    */
}
void MainWindow::on_listW_maplocal_itemSelectionChanged()
{
    recentClick = ui->listW_maplocal;
    QList<QListWidgetItem*> qSel = ui->listW_maplocal->selectedItems();
    if (qSel.size() == 0)
    {
        ui->pB_reviewmap->setEnabled(0);
    }
    else
    {
        ui->pB_reviewmap->setEnabled(1);
    }
}

// Database table review.
void MainWindow::on_pB_viewtable_clicked()
{
    QList<QListWidgetItem*> qSel = ui->listW_searchresult->selectedItems();
    if (qSel.size() != 1) { return; }
    jf.timerStart();
    QString qTemp = qSel[0]->text();
    string tname = qTemp.toStdString();
    vector<vector<string>> vvsResult;
    vector<string> colTitles, prompt = { tname };
    sb.set_prompt(prompt);
    vector<vector<int>> comm(1, vector<int>());
    comm[0].assign(comm_length, 0);
    thread::id myid = this_thread::get_id();
    sb.start_call(myid, 1, comm[0]);
    std::thread thr(&MainWindow::tableTopper, this, ref(sb), ref(vvsResult));
    thr.detach();
    while (1)
    {
        Sleep(gui_sleep);
        QCoreApplication::processEvents();
        comm = sb.update(myid, comm[0]);
        if (comm.size() > 1)
        {
            barMessage("Indexing catalogue " + prompt[0] + " ...");
            break;
        }
    }
    while (comm[0][0] == 0)
    {
        Sleep(gui_sleep);
        QCoreApplication::processEvents();
        comm = sb.update(myid, comm[0]);
        if (comm[0][2] == 0 && comm[1][2] > 0)
        {
            comm[0][2] = comm[1][2];
            barReset(comm[0][2], "Loading table " + prompt[0] + " ...");
        }
        if (comm[1][0] == 1)
        {
            comm[0][0] = 1;
        }
    }
    sb.end_call(myid);
    sf.get_col_titles(tname, colTitles);
    ui->tableW_db->clear();
    ui->tableW_db->setColumnCount(colTitles.size());
    ui->tableW_db->setRowCount(vvsResult.size());
    QTableWidgetItem* qCell = nullptr;
    QSize qSize(30, 30);
    for (int ii = 0; ii < vvsResult.size(); ii++)
    {
        for (int jj = 0; jj < vvsResult[ii].size(); jj++)
        {
            qTemp = QString::fromStdString(vvsResult[ii][jj]);
            qCell = new QTableWidgetItem(qTemp);
            qSize.setWidth(qTemp.size() * 10);
            qCell->setSizeHint(qSize);
            ui->tableW_db->setItem(ii, jj, qCell);
        }
    }
    QStringList hHeaderLabels;
    for (int ii = 0; ii < colTitles.size(); ii++)
    {
        qTemp = QString::fromStdString(colTitles[ii]);
        hHeaderLabels.append(qTemp);
    }
    qTemp = QString::fromStdString(tname);
    qCell = new QTableWidgetItem(qTemp);
    ui->tableW_db->setVerticalHeaderItem(0, qCell);
    ui->tableW_db->verticalHeader()->setVisible(0);
    ui->tableW_db->setHorizontalHeaderLabels(hHeaderLabels);
    time = jf.timerStop();
    reportTable(ui->tableW_db);
}
void MainWindow::tableTopper(SWITCHBOARD& sbgui, vector<vector<string>>& vvsResult)
{
    thread::id myid = this_thread::get_id();
    vector<int> mycomm;
    sbgui.answer_call(myid, mycomm);
    vector<string> prompt = sbgui.get_prompt();  // Form [tname].
    vector<string> search = { "*" };
    sf.select(search, prompt[0], vvsResult);
    mycomm[0] = 1;
    sbgui.update(myid, mycomm);
}
void MainWindow::on_pB_deletetable_clicked()
{
    int mode, iRow, inum = -1;
    double dnum = -1.0;
    string tname;
    QString qTemp = ui->pB_deletetable->text();
    if (qTemp == "Delete\nTable") { mode = 0; }
    else if (qTemp == "Delete\nRow") { mode = 1; }
    else { jf.err("Failed to read button-MainWindow.on_pB_deletetable_clicked"); }
    switch (mode)
    {
    case 0:
    {
        QList<QListWidgetItem*> qSel = ui->listW_searchresult->selectedItems();
        if (qSel.size() != 1) { return; }
        qTemp = qSel[0]->text();
        tname = qTemp.toStdString();
        sf.remove(tname);
        qSel[0]->setHidden(1);
        break;
    }
    case 1:
    {
        QList<QTableWidgetItem*> qSel = ui->tableW_db->selectedItems();
        if (qSel.size() != 1) { return; }
        QTableWidgetItem* qCell = ui->tableW_db->verticalHeaderItem(0);
        qTemp = qCell->text();
        tname = qTemp.toStdString();
        qCell = ui->tableW_db->horizontalHeaderItem(0);
        qTemp = qCell->text();
        string colTitle = qTemp.toStdString();
        iRow = ui->tableW_db->row(qSel[0]);
        qCell = ui->tableW_db->item(iRow, 0);
        qTemp = qCell->text();
        string sValue = qTemp.toStdString();
        try { inum = stoi(sValue); }
        catch (invalid_argument) 
        { 
            inum = -1; 
            try { dnum = stod(sValue); }
            catch (invalid_argument) { dnum = -1.0; }
        }
        vector<string> conditions = { "\"" + colTitle };
        if (inum == -1 && dnum == -1.0)
        {
            conditions[0] += "\" LIKE '" + sValue + "'";
        }
        else { conditions[0] += "\" = " + sValue; }
        sf.remove(tname, conditions);
        ui->tableW_db->removeRow(iRow);
        break;
    }
    }
}
void MainWindow::on_tableW_db_currentCellChanged(int RowNow, int ColNow, int RowThen, int ColThen)
{
    if (RowNow < 0 || RowNow == RowThen) { return; }
    ui->pB_deletetable->setEnabled(1);
    ui->pB_deletetable->setText("Delete\nRow");
}

// Online Statistics Canada functionalities.
void MainWindow::on_pB_usc_clicked()
{
    int uscMode;
    QString qTemp;
    QTreeWidgetItem* qNode;
    QList<QTreeWidgetItem*> qSel = ui->treeW_cataonline->selectedItems();
    if (qSel.size() < 1) { uscMode = 0; }
    else
    {
        qNode = qSel[0]->parent();
        uscMode = 0;
        while (qNode != nullptr)
        {
            qNode = qNode->parent();
            uscMode++;
        }
    }
    switch (uscMode)
    {
    case 0:  // Set root, display years.
    {
        string nameRoot = "Statistics Canada Census Data";
        jtCataOnline.init(nameRoot, sroot);
        string webpage = wf.browseS(scroot);
        vector<string> yearList = jf.textParser(webpage, navSearch[0]);
        jf.isort_ilist(yearList, JFUNC::Increasing);
        vector<int> iYearList;
        iYearList.assign(yearList.size(), -2);  // Unique placeholders.
        jtCataOnline.addChildren(yearList, iYearList, -1);
        qf.populateTree(ui->treeW_cataonline, jtCataOnline, 1);
        ui->treeW_cataonline->topLevelItem(0)->setExpanded(1);
        break;
    }
    case 1:  // Set year, display catalogues.
    {
        QString qYear = qSel[0]->text(0);
        string sYear = qYear.toStdString();
        string url = sc.urlYear(sYear);
        string webpage = wf.browseS(url);
        vector<string> cataList = jf.textParser(webpage, navSearch[1]);
        vector<int> iCataList(cataList.size(), -2);
        jtCataOnline.addChildren(cataList, iCataList, sYear);
        qf.populateTree(ui->treeW_cataonline, jtCataOnline, 1);
        qNode = ui->treeW_cataonline->topLevelItem(0);
        qNode->setExpanded(1);
        int numKids = qNode->childCount();
        for (int ii = 0; ii < numKids; ii++)
        {
            qTemp = qNode->child(ii)->text(0);
            if (qTemp == qYear)
            {
                qNode->child(ii)->sortChildren(0, Qt::SortOrder::AscendingOrder);
                qNode->child(ii)->setExpanded(1);
                break;
            }
        }
        break;
    }
    }
}
void MainWindow::on_pB_download_clicked()
{
    QString qTemp;
    QList<QTreeWidgetItem*> qlist, qChildren;
    string sYear, sCata, temp;
    vector<string> prompt, listChildren;
    vector<vector<int>> comm(1, vector<int>());
    comm[0].assign(comm_length, 0);  // Form [control, progress report, size report, max param].
    thread::id myid = this_thread::get_id();
    int activeTab = ui->tabW_main->currentIndex(), iGen;
    switch (activeTab)
    {
    case 0:
    {
        qlist = ui->treeW_cataonline->selectedItems();
        if (qlist.size() < 1) { return; }
        iGen = qf.getBranchGen(qlist[0]);
        prompt.resize(2);  // Form [syear, sname].
        switch (iGen)
        {
        case 0:  // Root. Nothing to do...
            return;
        case 1:  // Year.
        {
            QString qYear = qlist[0]->text(0);
            sYear = qYear.toStdString();
            int numKids = qlist[0]->childCount(), numGKids;
            QTreeWidgetItem* qRoot, *qBranch;
            if (numKids == 0)
            {
                on_pB_usc_clicked();
                qRoot = ui->treeW_cataonline->topLevelItem(0);
                numKids = qRoot->childCount();
                for (int ii = 0; ii < numKids; ii++)
                {
                    qBranch = qRoot->child(ii);
                    qTemp = qBranch->text(0);
                    if (qTemp == qYear)
                    {
                        numGKids = qBranch->childCount();
                        listChildren.resize(numGKids);
                        for (int jj = 0; jj < numGKids; jj++)
                        {
                            qTemp = qBranch->child(jj)->text(0);
                            listChildren[jj] = qTemp.toStdString();
                        }
                        break;
                    }
                }
            }
            else
            {
                listChildren.resize(numKids);
                for (int ii = 0; ii < numKids; ii++)
                {
                    qTemp = qlist[0]->child(ii)->text(0);
                    listChildren[ii] = qTemp.toStdString();
                }
            }

            for (int ii = 0; ii < listChildren.size(); ii++)
            {
                downloadCatalogue(sYear, listChildren[ii]);
            }
            break;
        }
        case 2:  // Catalogue.
        {
            qTemp = qlist[0]->parent()->text(0);
            sYear = qTemp.toStdString();
            qTemp = qlist[0]->text(0);
            sCata = qTemp.toStdString();
            downloadCatalogue(sYear, sCata);
            break;
        }
        case 3:  // CSV or maps.
        {
            /*
            qtemp = qlist[0]->parent()->parent()->text(0);
            prompt[0] = qtemp.toStdString();
            qtemp = qlist[0]->parent()->text(0);
            prompt[1] = qtemp.toStdString();
            qtemp = qlist[0]->text(0);
            temp = qtemp.toStdString();
            pos1 = temp.find("CSV");
            if (pos1 < temp.size())  // Download catalogue's CSVs.
            {
                sb.set_prompt(prompt);
                sb.start_call(myid, 1, comm[0]);
                std::thread dlCata(&STATSCAN::downloadCatalogue, ref(sc), ref(sb));
                dlCata.detach();
                while (comm.size() < 2)
                {
                    Sleep(gui_sleep);
                    QCoreApplication::processEvents();
                    comm = sb.update(myid, comm[0]);
                }
                while (comm[0][0] == 0)
                {
                    Sleep(gui_sleep);
                    QCoreApplication::processEvents();
                    comm = sb.update(myid, comm[0]);
                    if (comm[1][2] > comm[0][2])
                    {
                        comm[0][2] = comm[1][2];
                        temp = "Downloading catalogue " + prompt[1] + " ...";
                        reset_bar(comm[0][2], temp);
                    }
                    jobs_done = comm[1][1];
                    update_bar();
                    if (comm[1][1] == comm[0][2] && comm[1][1] > 0)
                    {
                        comm[0][0] = 1;
                    }
                    else { comm[0][1] = comm[1][1]; }
                }
                sb.end_call(myid);
                temp += " done!";
                barMessage(temp);
            }
            else  // Download catalogue's maps.
            {
                sb.set_prompt(prompt);
                sb.start_call(myid, 1, comm[0]);
                std::thread dlMaps(&STATSCAN::downloadMaps, ref(sc), ref(sb));
                dlMaps.detach();
                while (comm.size() < 2)
                {
                    Sleep(gui_sleep);
                    QCoreApplication::processEvents();
                    comm = sb.update(myid, comm[0]);
                }
                while (comm[0][0] == 0)
                {
                    Sleep(gui_sleep);
                    QCoreApplication::processEvents();
                    comm = sb.update(myid, comm[0]);
                    if (comm[1][2] > comm[0][2])
                    {
                        comm[0][2] = comm[1][2];
                        temp = "Downloading maps for catalogue " + prompt[1] + " ...";
                        reset_bar(comm[0][2], temp);
                    }
                    jobs_done = comm[1][1];
                    update_bar();
                    if (comm[1][1] == comm[0][2] && comm[1][1] > 0)
                    {
                        comm[0][0] = 1;
                    }
                    else { comm[0][1] = comm[1][1]; }
                }
                sb.end_call(myid);
                temp += " done!";
                barMessage(temp);
            }
            */
            
            break;
        }
        }
        break;
    }
    case 1:
    {
        // NOTE: Display tree of existing maps.

        break;
    }
    }

}
void MainWindow::downloadCatalogue(string sYear, string sCata)
{
    QList<QTreeWidgetItem*> qlist, qChildren;
    string temp;
    vector<string> prompt;
    vector<vector<int>> comm(1, vector<int>());
    comm[0].assign(comm_length, 0);  // Form [control, progress report, size report, max param].
    thread::id myid = this_thread::get_id();
    int activeTab = ui->tabW_main->currentIndex(), iGen;

    // Download the zip file.
    prompt.resize(3);
    prompt[0] = sc.urlCSVDownload(sYear, sCata);
    prompt[1] = sroot + "\\" + sYear + "\\" + sCata;
    prompt[1] += "\\" + sCata + "_ENG_CSV.zip";
    prompt[2] = "1";  // Binary file.
    if (!wf.file_exist(prompt[1]))
    {
        sb.set_prompt(prompt);
        sb.start_call(myid, 1, comm[0]);
        temp = "Downloading " + sCata + ", please wait ...";
        barMessage(temp);
        std::thread thr(&MainWindow::thrDownload, this, ref(sb));
        thr.detach();
        QCoreApplication::processEvents();
        while (comm.size() < 2)
        {
            Sleep(20);
            comm = sb.update(myid, comm[0]);
        }
        while (comm[0][0] == 0)
        {
            Sleep(gui_sleep);
            QCoreApplication::processEvents();
            comm = sb.update(myid, comm[0]);
            if (comm[1][0] == 1)
            {
                sb.end_call(myid);
                comm[0][0] = 1;
            }
        }
    }
    temp = "Download complete! Unzipping file:    " + prompt[1];
    barMessage(temp);

    // Extract the contents of the zip file to its folder.
    if (!wf.file_exist(prompt[1])) { jf.err("Downloaded file not found-MainWindow.on_pB_download_clicked"); }
    size_t pos1 = prompt[1].rfind('\\');
    string folderPath = prompt[1].substr(0, pos1);
    string csvPath = folderPath + "\\" + sCata + "_English_CSV_data.csv";
    if (!wf.file_exist(csvPath))
    {
        temp = prompt[1];
        prompt.resize(1);
        prompt[0] = temp;
        sb.set_prompt(prompt);
        comm.resize(1);
        comm[0].assign(comm_length, 0);
        sb.start_call(myid, 1, comm[0]);
        std::thread thr2(&MainWindow::thrUnzip, this, ref(sb));
        thr2.detach();
        while (comm.size() < 2)
        {
            Sleep(20);
            comm = sb.update(myid, comm[0]);
        }
        while (comm[0][0] == 0)
        {
            Sleep(gui_sleep);
            QCoreApplication::processEvents();
            comm = sb.update(myid, comm[0]);
            if (comm[1][0] == 1)
            {
                sb.end_call(myid);
                comm[0][0] = 1;
            }
        }
    }
    temp = "Unzip complete! Splitting the CSV mega-file:    " + csvPath;
    barMessage(temp);

    // Split the CSV mega-file into manageable pieces. 
    if (!wf.file_exist(csvPath)) { jf.err("Unzipped file not found-MainWindow.on_pB_download_clicked"); }
    int progress = 0, myProgress;
    mutex m_progress;
    vector<string> csvList = wf.get_file_list(folderPath, "*PART*.csv");
    if (csvList.size() < 1)
    {
        prompt.resize(2);
        prompt[0] = csvPath;
        prompt[1] = to_string(csvMaxSize);
        sb.set_prompt(prompt);
        comm.resize(1);
        comm[0].assign(comm_length, 0);
        sb.start_call(myid, 1, comm[0]);
        std::thread thr3(&MainWindow::thrFileSplitter, this, ref(sb), ref(progress), ref(m_progress));
        thr3.detach();
        while (comm.size() < 2)
        {
            Sleep(20);
            comm = sb.update(myid, comm[0]);
        }
        while (comm[0][0] == 0)
        {
            Sleep(gui_sleep);
            QCoreApplication::processEvents();
            comm = sb.update(myid, comm[0]);
            if (comm[1][2] > comm[0][2])
            {
                comm[0][2] = comm[1][2];
                barReset(comm[0][2], temp);
            }
            if (comm[1][0] == 1)
            {
                sb.end_call(myid);
                comm[0][3] = comm[1][3];
                comm[0][0] = 1;
            }
            m_progress.lock();
            myProgress = progress;
            m_progress.unlock();
            barUpdate(myProgress);
        }
    }
    temp = "File splitting complete! " + to_string(comm[0][3]) + " CSV parts were made.";
    barMessage(temp);
}
void MainWindow::on_treeW_cataonline_itemSelectionChanged()
{
    QList<QTreeWidgetItem*> qSel = ui->treeW_cataonline->selectedItems();
    if (qSel.size() != 1) { return; }
    int iGen = qf.getBranchGen(qSel[0]);
    if (iGen >= 1 && iGen <= 2)
    {
        ui->pB_download->setEnabled(1);
    }
    else
    {
        ui->pB_download->setEnabled(0);
    }
}

// Modes: 0 = download given webpage
void MainWindow::on_pB_test_clicked()
{
    int mode = 13;

    switch (mode)
    {
    case 0:  // Download the webpage given by the text box's URL.
    {
        QString qtemp = ui->pte_search->toPlainText();
        string url = qtemp.toStdString();
        wstring webpage = wf.browseW(url);
        jf.printer(sroot + "\\Test webpage.txt", webpage);
        qtemp = "Done!";
        ui->pte_search->setPlainText(qtemp);
        return;
    }
    case 1:  // Inspect a table cell's width.
    {
        QList<QTableWidgetItem*> qSel = ui->tableW_db->selectedItems();
        if (qSel.size() != 1) { return; }
        QSize qSize = qSel[0]->sizeHint();
        string sMessage = "Size Hint\nWidth: " + to_string(qSize.width());
        sMessage += "\nHeight: " + to_string(qSize.height());
        QString qMessage = QString::fromStdString(sMessage);
        ui->pte_search->setPlainText(qMessage);
        break;
    }
    case 2:
    {
        QString qTemp = ui->pte_search->toPlainText();
        if (qTemp != "provinceonly") { return; }
        ui->pte_search->clear();
        QList<QTreeWidgetItem*> qSel = ui->treeW_maplocal->selectedItems();
        if (qSel.size() != 1) { return; }
        int mode;
        size_t pos1, pos2;
        string binPath, binFile, binFileNew, temp;
        string folderPath = qf.getBranchPath(qSel[0], jtMapLocal.pathRoot);
        vector<string> binNameList = wf.get_file_list(folderPath, "*.bin");
        for (int ii = 0; ii < binNameList.size(); ii++)
        {
            pos1 = binNameList[ii].find("(Canada)");
            if (pos1 < binNameList[ii].size()) { mode = 1; }
            else { mode = 0; }
            binPath = folderPath + "\\" + binNameList[ii];
            binFile = wf.load(binPath);
            switch (mode)
            {
            case 0:
            {
                pos1 = binFile.find("//position");
                binFileNew = binFile.substr(0, pos1);
                binFileNew += "//parent\nCanada\n\n//position(PNG)\n";
                pos1 = binFile.find('\n', pos1) + 1;
                pos2 = binFile.find_first_of("\r\n", pos1);
                temp = binFile.substr(pos1, pos2 - pos1);               
                binFileNew += temp + "\n\n//position(GPS)\n-181.0,-181.0\n\n";
                pos1 = binFile.find("//border");
                binFileNew += binFile.substr(pos1);
                break;
            }
            case 1:
            {
                pos1 = binFile.find("//border");
                binFileNew = binFile.substr(0, pos1);
                binFileNew += "//parent\nCanada\n\n//position(PNG)\n";
                binFileNew += "-1.0,-1.0\n\n//position(GPS)\n-181.0,-181.0\n\n";
                pos1 = binFile.find("//border");
                binFileNew += binFile.substr(pos1);
                break;
            }
            }
            wf.printer(binPath, binFileNew);
        }
    }
    case 3:  // Upgrade a given SC geo file.
    {
        QString qTemp = ui->pte_search->toPlainText();
        string geoPathOld = qTemp.toStdString(), folderPath;
        size_t pos1;
        if (wf.file_exist(geoPathOld))
        {
            pos1 = geoPathOld.rfind('\\');
            folderPath = geoPathOld.substr(0, pos1);
            sc.init(folderPath);
            sc.convertSCgeo(geoPathOld);
        }
        else
        {
            qTemp = "Specified file does not exist.";
            ui->pte_search->setPlainText(qTemp);
        }
        break;
    }
    case 4:  // Insert a complete geo table using the given tnameGeo.
    {
        QString qTemp = ui->pte_search->toPlainText();
        string tnameGeo = qTemp.toStdString();
        size_t pos1 = tnameGeo.find('$') + 1;
        size_t pos2 = tnameGeo.find('$', pos1);
        string sYear = tnameGeo.substr(pos1, pos2 - pos1);
        pos1 = pos2 + 1;
        pos2 = tnameGeo.find('$', pos1);
        string sCata = tnameGeo.substr(pos1, pos2 - pos1);
        string cataPath = sroot + "\\" + sYear + "\\" + sCata;
        vector<string> nameList = wf.get_file_list(cataPath, "*PART*.csv");
        int maxPART = nameList.size(), index, maxLevel = -1, inum;
        string csvPath, csvFile, csvBuffer, line, temp, currentGeoCode;
        vector<string> colTitles, colValues, ancestry, stmts;
        vector<string> dirt = { "'" }, soap = { "''" };
        vector<vector<string>> geoData;
        vector<int> scoopIndex;
        for (int ii = 1; ii <= maxPART; ii++)
        {
            csvPath = cataPath + "\\" + sCata + "_English_CSV_data (PART ";
            csvPath += to_string(ii) + ").csv";
            csvFile = wf.load(csvPath);
            if (ii == 1)
            {
                pos1 = csvFile.find('"');
                pos2 = csvFile.find("\r\n");
                line = csvFile.substr(pos1, pos2 - pos1);
                pos1 = 0;
                while (pos1 < line.size())
                {
                    pos1 = line.find_first_not_of(",\" ", pos1);
                    pos2 = line.find('"', pos1);
                    temp = line.substr(pos1, pos2 - pos1);
                    colTitles.push_back(temp);
                    pos1 = line.find(',', pos2);
                }
                for (int jj = 0; jj < colTitles.size(); jj++)
                {
                    pos1 = colTitles[jj].find("ALT_GEO_CODE");
                    if (pos1 < colTitles[jj].size()) { scoopIndex.push_back(jj); }
                    pos1 = colTitles[jj].find("GEO_LEVEL");
                    if (pos1 < colTitles[jj].size()) { scoopIndex.push_back(jj); }
                    pos1 = colTitles[jj].find("GEO_NAME");
                    if (pos1 < colTitles[jj].size()) { scoopIndex.push_back(jj); }
                    if (scoopIndex.size() >= 3) { break; }
                }
                pos1 = csvFile.find("\r\n");
                pos2 = csvFile.find("\r\n", pos1 + 1);
                line = csvFile.substr(pos1, pos2 - pos1);
            }
            else
            {
                pos1 = 0;
                pos2 = csvFile.find("\r\n");
                line = csvBuffer + csvFile.substr(pos1, pos2 - pos1);
            }

            while (pos2 < csvFile.size())
            {
                colValues = sc.extractCSVLineValue(line, scoopIndex);
                if (colValues[2] != currentGeoCode)
                {
                    currentGeoCode = colValues[2];
                    index = geoData.size();
                    geoData.push_back(vector<string>(3));
                    geoData[index][0] = colValues[2];
                    geoData[index][1] = colValues[1];
                    geoData[index][2] = colValues[0];
                }
                pos1 = pos2;
                pos2 = csvFile.find("\r\n", pos1 + 1);
                if (pos2 < csvFile.size()) { line = csvFile.substr(pos1, pos2 - pos1); }
            }
            csvBuffer = csvFile.substr(pos1);
            pos1--;
        }
        for (int ii = 0; ii < geoData.size(); ii++)
        {
            try { inum = stoi(geoData[ii][2]); }
            catch (invalid_argument) { jf.err("stoi-MainWindow.test4"); }
            if (inum > maxLevel) { maxLevel = inum; }
        }
        string stmt = "CREATE TABLE IF NOT EXISTS \"" + tnameGeo + "\" (GEO_CODE ";
        stmt += "INTEGER PRIMARY KEY, \"Region Name\" TEXT, GEO_LEVEL INT";
        for (int ii = 0; ii < maxLevel; ii++)
        {
            stmt += ", Ancestor" + to_string(ii) + " INT";
        }
        stmt += ");";
        sf.executor(stmt);
        for (int ii = 0; ii < geoData.size(); ii++)
        {
            try { inum = stoi(geoData[ii][2]); }
            catch (invalid_argument) { jf.err("stoi-MainWindow.test4"); }
            if (inum >= ancestry.size())
            {
                ancestry.push_back(geoData[ii][0]);
            }
            else
            {
                ancestry[inum] = geoData[ii][0];
            }
            for (int jj = 0; jj < inum; jj++)
            {
                geoData[ii].push_back(ancestry[jj]);
            }
            stmt = "INSERT OR REPLACE INTO \"" + tnameGeo + "\" (GEO_CODE, ";
            stmt += "\"Region Name\", GEO_LEVEL";
            for (int jj = 0; jj < inum; jj++)
            {
                stmt += ", Ancestor" + to_string(jj);
            }
            jf.clean(geoData[ii][1], dirt, soap);
            stmt += ") VALUES (" + geoData[ii][0] + ", '" + geoData[ii][1];
            stmt += "', " + geoData[ii][2];
            for (int jj = 3; jj < geoData[ii].size(); jj++)
            {
                stmt += ", " + geoData[ii][jj];
            }
            stmt += ");";
            stmts.push_back(stmt);
        }
        sf.insert_prepared(stmts);
        break;
    }
    case 5:  // Search for a small image within a larger one.
    {
        string targetPath = sroot + "\\1.png";
        string bgPath = sroot + "\\2.png";
        im.markTargetTL(targetPath, bgPath);
        QString qTemp = "Finished markTargetTL.";
        ui->pte_search->setPlainText(qTemp);
        break;
    }
    case 6:  // Define a TLBR, and save it as a cropped image. 
    {
        QString qTemp = "NUM1: Hover your cursor over the Top-Left corner of the rectangle.";
        qDebug() << qTemp;
        qTemp = "NUM2: Hover your cursor over the Bottom-Right corner of the rectangle.";
        qDebug() << qTemp;
        vector<POINT> TLBR = io.getUserTLBR(VK_NUMPAD1, VK_NUMPAD2);
        Sleep(1000);
        vector<string> filePath(2);
        filePath[0] = sroot + "\\background.png";
        filePath[1] = sroot + "\\overview.png";
        gdi.screenshot(filePath[0]);
        im.cropSave(filePath, TLBR);
        break;
    }
    case 7:  // Count the pixel colours, and display them as a list sorted by frequency measured.
    {
        vector<vector<unsigned char>> rgbList;
        vector<unsigned char> img, legend;
        vector<int> imgSpec, legendSpec, freqList;
        vector<string> sFreqList;
        string inputPath = sroot + "\\overview (Flattened).png";
        im.countPixelColour(inputPath, rgbList, freqList);
        sFreqList.resize(freqList.size());
        for (int ii = 0; ii < sFreqList.size(); ii++)
        {
            sFreqList[ii] = to_string(freqList[ii]);
        }
        im.makeLegendV(legend, legendSpec, sFreqList, rgbList);
        if (legendSpec.size() < 3) { legendSpec.push_back(3); }
        string outputPath = sroot + "\\Overview Colour Count (Flattened).png";
        im.pngPrint(legend, legendSpec, outputPath);
        break;
    }
    case 8:  // Scan a PNG for a pattern of colours, along horizontal or vertial lines.
    {
        string inputPath = sroot + "\\overview.png";
        vector<vector<unsigned char>> colourListV = {
            { 179, 217, 247, 255 },
            { 240, 240, 240, 255 },
            { 179, 217, 247, 255 },
            { 215, 215, 215, 255 },
            { 179, 217, 247, 255 },
            { 215, 215, 215, 255 }
        };
        vector<vector<unsigned char>> colourListH = {
            { 215, 215, 215, 255 },
            { 179, 217, 247, 255 },
            { 215, 215, 215, 255 },
            { 179, 217, 247, 255 },
            { 215, 215, 215, 255 },
            { 179, 217, 247, 255 }
        };
        vector<unsigned char> img, red = { 255, 0, 0 };
        vector<int> imgSpec, viResultV, viResultH, frontrunner = { -1, -1 };
        im.pngLoadHere(inputPath, img, imgSpec);
        im.scanPatternLineV(img, imgSpec, colourListV, viResultV);
        im.scanPatternLineH(img, imgSpec, colourListH, viResultH);
        int count = 0;
        for (int ii = 1; ii < viResultV.size(); ii++)
        {
            if (viResultV[ii - 1] + 1 == viResultV[ii]) { count++; }
            else { count = 0; }
            if (count > frontrunner[0])
            {
                frontrunner[0] = count;
                frontrunner[1] = viResultV[ii];
            }
        }
        frontrunner[1] -= frontrunner[0] / 2;
        POINT origin;
        origin.x = frontrunner[1];
        frontrunner = { -1, -1 };
        count = 0;
        for (int ii = 1; ii < viResultH.size(); ii++)
        {
            if (viResultH[ii - 1] + 1 == viResultH[ii]) { count++; }
            else { count = 0; }
            if (count > frontrunner[0])
            {
                frontrunner[0] = count;
                frontrunner[1] = viResultH[ii];
            }
        }
        frontrunner[1] -= frontrunner[0] / 2;
        origin.y = frontrunner[1];
        im.drawSquare(img, imgSpec, origin);
        string outputPath = sroot + "\\Overview Pattern.png";
        im.pngPrint(img, imgSpec, outputPath);
        break;
    }
    case 9:  // Succinctly determine the Overview origin point.
    {
        vector<POINT> bHome;
        bm.init(ui->pte_search);
        bm.recordButton(bHome, "home");
        string inputPath = sroot + "\\desktop.png", outputPath = sroot + "\\desktopTest.png";
        vector<unsigned char> img;
        vector<int> imgSpec;
        im.pngLoadHere(inputPath, img, imgSpec);
        bm.findFrames(img, imgSpec, bHome[2]);
        POINT origin = bm.getOrigin(img, imgSpec);
        im.drawSquare(img, imgSpec, origin);
        im.pngPrint(img, imgSpec, outputPath);
        break;
    }
    case 10:  // Perform a search and determine the delta origin.
    {
        vector<POINT> bHome, bPanel;
        bm.init(ui->pte_search);
        bm.recordButton(bHome, "home");
        Sleep(500);
        bm.recordButton(bPanel, "panel");
        Sleep(500);
        POINT searchBar, bMinus = bHome[2];
        bm.recordPoint(searchBar, "search bar");
        bMinus.y += 45;
        string inputPath = sroot + "\\desktop.png";
        vector<unsigned char> img;
        vector<int> imgSpec;
        im.pngLoadHere(inputPath, img, imgSpec);
        bm.findFrames(img, imgSpec, bHome[2]);
        POINT originCanada = bm.getOrigin(img, imgSpec);
        Sleep(500);
        io.mouseClick(searchBar);
        Sleep(500);
        string temp = "alberta";
        io.kbInput(temp);
        Sleep(500);
        io.kbInput(VK_DOWN);
        Sleep(500);
        io.kbInput(VK_RETURN);
        Sleep(3000);
        io.mouseClick(bPanel[2]);
        Sleep(2000);
        io.mouseClick(bMinus);
        Sleep(2000);
        string albertaPath = sroot + "\\albertaTest.png";
        gdi.screenshot(albertaPath);
        im.pngLoadHere(albertaPath, img, imgSpec);
        POINT originAlberta = bm.getOrigin(img, imgSpec);
        int dx = originAlberta.x - originCanada.x;
        int dy = originAlberta.y - originCanada.y;
        QString qMessage = "deltaOrigin x: " + QString::number(dx);
        qMessage += "\ndeltaOrigin y: " + QString::number(dy);
        ui->pte_search->setPlainText(qMessage);
        break;
    }
    case 11:  // Load a screenshot, make a list of highlighted pixels, make a bounding rectangle, display a color frequency list. 
    {
        vector<unsigned char> img, highlighted = { 192, 192, 243, 255 };
        vector<unsigned char> red = { 255, 0, 0, 255 }, legend;
        vector<int> imgSpec, freqList, legendSpec;
        string inputPath = sroot + "\\albertaTest.png";
        im.pngLoadHere(inputPath, img, imgSpec);
        vector<POINT> mainTLBR(2);
        mainTLBR[0].x = 2207;
        mainTLBR[0].y = 157;
        mainTLBR[1].x = 3836;
        mainTLBR[1].y = 971;
        vector<POINT> vpAlberta = im.findColour(img, imgSpec, highlighted, mainTLBR);
        vector<POINT> albertaTLBR = im.makeTLBR(vpAlberta);
        vector<vector<unsigned char>> rgbList;
        im.countPixelColour(img, imgSpec, rgbList, freqList, albertaTLBR);
        jf.sortLinkedList(freqList, rgbList, JFUNC::Decreasing);
        ui->tabW_main->setCurrentIndex(3);
        ui->tableW_debug->clear();
        ui->tableW_debug->setColumnCount(6);
        ui->tableW_debug->setRowCount(freqList.size());
        QString qTemp;
        QTableWidgetItem* qCell;
        QColor qColour;
        QBrush qBrush(Qt::SolidPattern);
        for (int ii = 0; ii < 6; ii++)
        {
            for (int jj = 0; jj < freqList.size(); jj++)
            {
                switch (ii)
                {
                case 0:
                {
                    qTemp = QString::number(freqList[jj]);
                    qCell = new QTableWidgetItem(qTemp);
                    ui->tableW_debug->setItem(jj, ii, qCell);
                    break;
                }
                case 1:
                {
                    qColour.setRgb((int)rgbList[jj][0], (int)rgbList[jj][1], (int)rgbList[jj][2], (int)rgbList[jj][3]);
                    qBrush.setColor(qColour);
                    qCell = new QTableWidgetItem(qTemp);
                    qCell->setBackground(qBrush);
                    ui->tableW_debug->setItem(jj, ii, qCell);
                    break;
                }
                case 2:
                {
                    qTemp = QString::number(rgbList[jj][0]);
                    qCell = new QTableWidgetItem(qTemp);
                    ui->tableW_debug->setItem(jj, ii, qCell);
                    break;
                }
                case 3:
                {
                    qTemp = QString::number(rgbList[jj][1]);
                    qCell = new QTableWidgetItem(qTemp);
                    ui->tableW_debug->setItem(jj, ii, qCell);
                    break;
                }
                case 4:
                {
                    qTemp = QString::number(rgbList[jj][2]);
                    qCell = new QTableWidgetItem(qTemp);
                    ui->tableW_debug->setItem(jj, ii, qCell);
                    break;
                }
                case 5:
                {
                    qTemp = QString::number(rgbList[jj][3]);
                    qCell = new QTableWidgetItem(qTemp);
                    ui->tableW_debug->setItem(jj, ii, qCell);
                    break;
                }
                }
            }
        }
        
        break;
    }
    case 12:  // Load a screenshot, make a list of highlighted pixels, make a bounding rectangle, cast shadows from NESW, connect the dots to make a border.
    {
        vector<unsigned char> img, highlighted = { 192, 192, 243, 255 }, rgba;
        vector<unsigned char> red = { 255, 0, 0, 255 };
        vector<int> imgSpec, freqList;
        string inputPath = sroot + "\\albertaTest.png";
        im.pngLoadHere(inputPath, img, imgSpec);
        vector<POINT> mainTLBR(2);
        mainTLBR[0].x = 2207;
        mainTLBR[0].y = 157;
        mainTLBR[1].x = 3836;
        mainTLBR[1].y = 971;
        vector<POINT> vpAlberta = im.findColour(img, imgSpec, highlighted, mainTLBR);
        vector<POINT> albertaTLBR = im.makeTLBR(vpAlberta);
        albertaTLBR[0].x -= 10;
        albertaTLBR[0].y -= 10;
        albertaTLBR[1].x += 10;
        albertaTLBR[1].y += 10;
        // Go top->bot, right->left, bot->top, left->right.
        vector<vector<POINT>> border(4, vector<POINT>()); // NESW
        vector<vector<unsigned char>> outIn(2, vector<unsigned char>(4));
        outIn[0] = { 240, 240, 240, 255 };
        outIn[1] = highlighted;
        POINT p1, p2, pLastOut;
        for (int ii = albertaTLBR[0].x; ii <= albertaTLBR[1].x; ii++)
        {
            p1.x = ii;
            pLastOut.x = -1;
            for (int jj = albertaTLBR[0].y; jj <= albertaTLBR[1].y; jj++)
            {
                p1.y = jj;
                rgba = im.pixelRGB(img, imgSpec, p1);
                if (rgba == outIn[0]) { pLastOut = p1; }
                else if (rgba == outIn[1] && pLastOut.x >= 0)
                {
                    p2 = p1;
                    p2.y = (p1.y + pLastOut.y) / 2;
                    border[0].push_back(p2);
                    break;
                }
            }
        }
        for (int ii = albertaTLBR[0].y; ii <= albertaTLBR[1].y; ii++)
        {
            p1.y = ii;
            pLastOut.x = -1;
            for (int jj = albertaTLBR[1].x; jj >= albertaTLBR[0].x; jj--)
            {
                p1.x = jj;
                rgba = im.pixelRGB(img, imgSpec, p1);
                if (rgba == outIn[0]) { pLastOut = p1; }
                else if (rgba == outIn[1] && pLastOut.x >= 0)
                {
                    p2 = p1;
                    p2.x = (p1.x + pLastOut.x) / 2;
                    border[1].push_back(p2);
                    break;
                }
            }
        }
        for (int ii = albertaTLBR[1].x; ii >= albertaTLBR[0].x; ii--)
        {
            p1.x = ii;
            pLastOut.x = -1;
            for (int jj = albertaTLBR[1].y; jj >= albertaTLBR[0].y; jj--)
            {
                p1.y = jj;
                rgba = im.pixelRGB(img, imgSpec, p1);
                if (rgba == outIn[0]) { pLastOut = p1; }
                else if (rgba == outIn[1] && pLastOut.x >= 0)
                {
                    p2 = p1;
                    p2.y = (p1.y + pLastOut.y) / 2;
                    border[2].push_back(p2);
                    break;
                }
            }
        }
        for (int ii = albertaTLBR[1].y; ii >= albertaTLBR[0].y; ii--)
        {
            p1.y = ii;
            pLastOut.x = -1;
            for (int jj = albertaTLBR[0].x; jj <= albertaTLBR[1].x; jj++)
            {
                p1.x = jj;
                rgba = im.pixelRGB(img, imgSpec, p1);
                if (rgba == outIn[0]) { pLastOut = p1; }
                else if (rgba == outIn[1] && pLastOut.x >= 0)
                {
                    p2 = p1;
                    p2.x = (p1.x + pLastOut.x) / 2;
                    border[3].push_back(p2);
                    break;
                }
            }
        }
        
        for (int ii = 0; ii < 4; ii++)
        {
            for (int jj = 0; jj < border[ii].size(); jj++)
            {
                im.pixelPaint(img, imgSpec, red, border[ii][jj]);
            }
        }
        string outputPath = sroot + "\\Alberta Highlighted with Shadow Borders.png";
        im.pngPrint(img, imgSpec, outputPath);
        break;
    }
    case 13:  // Make the CANADA parent map.
    {
        bm.init(ui->pte_search);
        string homePath = sroot + "\\Home.png";
        string canadaPath = sroot + "\\mapParent\\Canada.png";
        bm.makeCanada(canadaPath);
        break;
    }
    }

}
void MainWindow::on_tabW_main_currentChanged(int index)
{
    if (index == 2)
    {
        int width, height;
        GetDesktopResolution(width, height);
        qDebug() << width << ", " << height;
    }
}
void MainWindow::on_pB_resolution_clicked()
{
    GetDesktopResolution(resDesktop[0], resDesktop[1]);
    resScaling[0] = (double)resDesktop[0] / 1920.0;
    resScaling[1] = (double)resDesktop[1] / 1080.0;

    QRect TLWH = this->geometry();
    qDebug() << "TL(" << TLWH.x() << ", " << TLWH.y() << ")";
    qDebug() << "WH(" << TLWH.width() << ", " << TLWH.height() << ")";
    QList<QWidget*> widgets = this->findChildren<QWidget*>();    
    int x, y, w, h;
    for (int ii = 0; ii < widgets.size(); ii++)
    {
        TLWH = widgets[ii]->geometry();
        x = TLWH.x();
        x *= resScaling[0];
        y = TLWH.y();
        y *= resScaling[1];
        w = TLWH.width();
        w *= resScaling[0];
        h = TLWH.height();
        h *= resScaling[1];
        TLWH.setRect(x, y, w, h);
        widgets[ii]->setGeometry(TLWH);
    }



}

/*

// GUI UPDATE FUNCTIONS
void MainWindow::update_treeW_cataindb()
{
    QList<QTreeWidgetItem*> qlist;
    QTreeWidgetItem* qitem;
    string stmt = "SELECT DISTINCT Year FROM TCatalogueIndex;";
    vector<string> results1;
    sf.executor(stmt, results1);
    jf.isort_slist(results1);
    QString qtemp;
    unordered_map<string, int> map_year;
    for (int ii = 0; ii < results1.size(); ii++)
    {
        map_year.emplace(results1[ii], ii);
        qtemp = QString::fromStdString(results1[ii]);
        qitem = new QTreeWidgetItem();
        qitem->setText(0, qtemp);
        auto item_flags = qitem->flags();
        item_flags.setFlag(Qt::ItemIsSelectable, false);
        qitem->setFlags(item_flags);
        qlist.append(qitem);
    }
    stmt = "SELECT * FROM TCatalogueIndex;";
    vector<vector<string>> results;  // Form [catalogue index][syear, sname, desc].
    sf.executor(stmt, results);
    int index;
    for (int ii = 0; ii < results.size(); ii++)
    {
        index = map_year.at(results[ii][0]);
        qitem = new QTreeWidgetItem(qlist[index]);
        qtemp = QString::fromStdString(results[ii][0]);
        qitem->setText(0, qtemp);
        qtemp = QString::fromStdString(results[ii][1]);
        qitem->setText(1, qtemp);
        qtemp = QString::fromStdString(results[ii][2]);
        qitem->setText(2, qtemp);
    }
    ui->treeW_cataindb->clear();
    ui->treeW_cataindb->addTopLevelItems(qlist);
}
void MainWindow::update_treeW_mapindb()
{
    jf.timerStart();

    // Make the root.
    string nameRoot = "TMap", coreDir, temp;
    QString qtemp = "TMap";
    QTreeWidgetItem* qRoot = new QTreeWidgetItem(), *qBranch, *qParent;
    qRoot->setText(0, qtemp);
    auto item_flags = qRoot->flags();
    item_flags.setFlag(Qt::ItemIsSelectable, false);
    qRoot->setFlags(item_flags);

    // Make branches for folders.
    int params, index, indexParent;
    size_t pos1, pos2;
    vector<vector<string>> TMapIndex = sf.getTMapIndex();
    QList<QTreeWidgetItem*> qBranches;
    unordered_map<string, int> mapTMap;
    for (int ii = 0; ii < TMapIndex.size(); ii++)
    {
        try { params = stoi(TMapIndex[ii][1]); }
        catch (invalid_argument& ia) { err("stoi-MainWindow.update_treeW_mapindb"); }
        if (params == 1)
        {
            index = qBranches.size();
            mapTMap.emplace(TMapIndex[ii][0], index);
            qtemp = QString::fromStdString(TMapIndex[ii][params + 1]);
            qBranch = new QTreeWidgetItem(qRoot);
            qBranch->setText(0, qtemp);
            qBranches.append(qBranch);
        }
        else
        {
            pos1 = TMapIndex[ii][0].rfind('$');
            temp = TMapIndex[ii][0].substr(0, pos1);
            try { indexParent = mapTMap.at(temp); }
            catch (out_of_range& oor) { err("mapTMap-MainWindow.update_treeW_mapindb"); }
            index = qBranches.size();
            mapTMap.emplace(TMapIndex[ii][0], index);
            qtemp = QString::fromStdString(TMapIndex[ii][params + 1]);
            qBranch = new QTreeWidgetItem(qBranches[indexParent]);
            qBranch->setText(0, qtemp);
            qBranches.append(qBranch);
        }
    }

    // Update the GUI.
    ui->treeW_mapindb->clear();
    ui->treeW_mapindb->addTopLevelItem(qRoot);
    ui->treeW_mapindb->expandItem(qRoot);

    long long timer = jf.timerStop();
    qDebug() << "Time to update mapindb: " << timer;
}
void MainWindow::update_mode()
{
    switch (active_mode)
    {
    case 0:
        ui->cB_drives->setVisible(1);
        ui->pB_scan->setVisible(1);
        ui->pB_insert->setVisible(1);
        ui->pB_cancel->setVisible(1);
        ui->tabW_catalogues->setVisible(1);
        ui->tabW_results->setVisible(1);
        ui->tabW_results2->setVisible(1);
        ui->pB_viewcata->setVisible(1);
        ui->pB_removecata->setVisible(1);
        ui->tabW_online->setVisible(0);
        ui->treeW_statscan->setVisible(0);
        ui->listW_statscan->setVisible(0);
        ui->treeW_maps->setVisible(0);
        ui->label_maps->setVisible(0);
        ui->label_pos->setVisible(0);
        ui->label_maps2->setVisible(0);
        ui->listW_bindone->setVisible(0);
        ui->pB_resume->setVisible(0);
        ui->pB_pause->setVisible(0);
        ui->pB_advance->setVisible(0);
        ui->pB_resume->setVisible(0);
        ui->pB_pause->setVisible(0);
        ui->pB_advance->setVisible(0);
        ui->pB_backspace->setVisible(0);
        ui->pte_advance->setVisible(0);
        ui->pB_undo->setVisible(0);
        ui->pB_savemap->setVisible(0);
        ui->pB_deletemap->setVisible(0);
        ui->pB_usc->setVisible(0);
        ui->pB_test->setVisible(0);
        ui->pte_webinput->setVisible(0);
        ui->pte_localinput->setVisible(1);
        ui->pB_search->setVisible(1);
        ui->pB_download->setVisible(0);
        ui->pB_viewtable->setVisible(1);
        ui->pB_deletetable->setVisible(1);
        ui->pB_localmaps->setVisible(0);
        ui->pB_convert->setVisible(0);
        ui->pB_correct->setVisible(0);
        ui->pB_pos->setVisible(0);
        ui->pB_insertmap->setVisible(0);
        ui->checkB_override->setVisible(0);
        ui->checkB_eraser->setVisible(0);
        ui->pB_backspace->setVisible(0);
        break;
    case 1:
        ui->cB_drives->setVisible(0);
        ui->pB_scan->setVisible(0);
        ui->pB_insert->setVisible(0);
        ui->pB_cancel->setVisible(0);
        ui->tabW_catalogues->setVisible(0);
        ui->tabW_results->setVisible(0);
        ui->tabW_results2->setVisible(0);
        ui->pB_viewcata->setVisible(0);
        ui->pB_removecata->setVisible(0);
        ui->tabW_online->setVisible(1);
        ui->treeW_statscan->setVisible(1);
        ui->listW_statscan->setVisible(1);
        ui->treeW_maps->setVisible(1);
        ui->label_maps->setVisible(1);
        ui->label_pos->setVisible(1);
        ui->label_maps2->setVisible(1);
        ui->listW_bindone->setVisible(1);
        ui->pte_advance->setVisible(1);
        ui->pB_advance->setEnabled(0);
        ui->pB_resume->setVisible(1);
        ui->pB_pause->setVisible(1);
        ui->pB_advance->setVisible(1);
        ui->pB_backspace->setVisible(1);
        ui->pB_undo->setVisible(1);
        ui->pB_savemap->setVisible(1);
        ui->pB_deletemap->setVisible(1);
        ui->pB_usc->setVisible(1);
        ui->pB_test->setVisible(1);
        ui->pte_webinput->setVisible(1);
        ui->pte_localinput->setVisible(0);
        ui->pB_search->setVisible(0);
        ui->pB_download->setVisible(1);
        ui->pB_viewtable->setVisible(0);
        ui->pB_deletetable->setVisible(0);
        ui->pB_localmaps->setVisible(1);
        ui->pB_convert->setVisible(1);
        ui->pB_correct->setVisible(1);
        ui->pB_pos->setVisible(1);
        ui->pB_insertmap->setVisible(1);
        ui->checkB_override->setVisible(1);
        ui->checkB_eraser->setVisible(1);
        int indexTab = ui->tabW_online->currentIndex();
        if (indexTab == 2)
        {
            ui->pB_resume->setVisible(1);
            ui->pB_resume->setEnabled(0);
            ui->pB_pause->setVisible(1);
            ui->pB_pause->setEnabled(0);
            ui->pB_advance->setVisible(1);
            ui->pB_backspace->setVisible(1);
        }
        break;
    }
}


// GUI-SPECIFIC FUNCTIONS, WITH ASSOCIATED MANAGER AND WORKER FUNCTIONS:

// For the given local drive, display (as a tree widget) the available catalogues, organized by year.
void MainWindow::on_pB_scan_clicked()
{
    if (sdrive.size() < 1)
    {
        ui->QL_bar->setText("Choose a local drive before scanning.");
        return;
    }
    vector<vector<int>> comm(1, vector<int>());
    comm[0].assign(comm_length, 0);  // Form [control, progress, task size, max columns].
    thread::id myid = this_thread::get_id();
    QList<QTreeWidgetItem*> qsroots;
    vector<string> prompt = { sdrive };
    int error = sb.start_call(myid, 1, comm[0]);
    if (error) { errnum("start_call-pB_scan_clicked", error); }
    sb.set_prompt(prompt);
    std::thread scan(&MainWindow::scan_drive, this, ref(sb), ref(wf), ref(qsroots));
    while (1)
    {
        Sleep(gui_sleep);
        QCoreApplication::processEvents();
        comm = sb.update(myid, comm[0]);       
        if (comm[1][0] > 0)
        {
            break;  
        }
    }
    scan.join();
    error = sb.end_call(myid);
    if (error) { errnum("end_call-pB_scan_clicked", error); }
    ui->treeW_cataondrive->addTopLevelItems(qsroots);
    ui->tabW_catalogues->setCurrentIndex(1);
    log("Scanned drive " + sdrive + " for catalogues.");
}




void MainWindow::insert_csvs(vector<vector<vector<string>>>& all_queue, SWITCHBOARD& sbjudi, STATSCAN& scjudi)
{
    vector<int> mycomm;
    vector<vector<int>> comm_judi;
    thread::id myid = this_thread::get_id();
    int my_id = sbjudi.answer_call(myid, mycomm) - 1;
    if (my_id < 0) { err("sbjudi.answer_call-insert_csvs"); }
    vector<string> prompt = sbjudi.get_prompt();
    size_t pos1 = prompt[my_id].find(',');
    string temp1 = prompt[my_id].substr(0, pos1);
    string temp2 = prompt[my_id].substr(pos1 + 1);
    int bot, top, damaged_csv, num_subtables, ipaper;
    try
    {
        bot = stoi(temp1);
        top = stoi(temp2);
    }
    catch (invalid_argument& ia) { err("stoi-insert_csvs"); }
    JFUNC jfcsv;
    vector<vector<string>> text_vars, data_rows, paperwork;
    vector<string> vtemp, linearized_titles;
    string gid, csv_path, sfile, stmt, stmt0, tname;
    string cata_name = scjudi.get_cata_name();
    vector<string> column_titles = scjudi.get_column_titles();
    mycomm[2] = top - bot + 1;
    size_t finalTextVar;
    int tally = 0;
    bool success;

    // Iterate through the assigned CSVs...
    for (int ii = bot; ii <= top; ii++)
    {
        comm_judi = sbjudi.update(myid, mycomm);
        if (comm_judi[0][0] == 2)  // Check for a 'cancel' signal from manager.
        {
            mycomm[0] = -2;  // Report compliance with message.
            sbjudi.update(myid, mycomm);
            return;
        }

        // Load values for this CSV.
        damaged_csv = 0;
        ipaper = paperwork.size();
        paperwork.push_back(vector<string>());
        gid = scjudi.get_gid(ii);
        csv_path = scjudi.make_csv_path(ii);
        temp1 = jfcsv.load(csv_path);
        sfile = jfcsv.utf8ToAscii(temp1);
        text_vars = scjudi.extract_text_vars(sfile, finalTextVar);
        data_rows = scjudi.extract_rows(sfile, damaged_csv, finalTextVar);
        linearized_titles = scjudi.linearize_row_titles(data_rows, column_titles);
        if (linearized_titles.size() > 2000)  // SQLITE column limit.
        {
            mycomm[0] = 2;
            sbjudi.update(myid, mycomm);
            return;
        }
        if (damaged_csv == 0)
        {
            // Insert this CSV's row in the primary table.
            stmt = scjudi.get_insert_primary_template();
            scjudi.make_insert_primary_statement(stmt, gid, text_vars, data_rows);
            paperwork[ipaper].push_back(stmt);

            // Create this CSV's main table.
            stmt = scjudi.get_create_csv_table_template();
            temp1 = cata_name + "$!!!";
            tname = scjudi.make_create_csv_table_statement(stmt, gid, temp1);
            paperwork[ipaper].push_back(stmt);

            // Insert this CSV's main table rows.
            stmt0 = scjudi.get_insert_csv_row_template();
            for (int ii = 0; ii < data_rows.size(); ii++)
            {
                stmt = stmt0;
                tname = cata_name + "$" + gid;
                scjudi.make_insert_csv_row_statement(stmt, tname, data_rows[ii]);
                paperwork[ipaper].push_back(stmt);
            }

        }
        else  // Damaged CSV - will not be inserted, but will be added to the catalogue's list of damaged CSVs.
        {     // This is a temporary measure - it would be better to incorporate these CSVs however possible.
            stmt = scjudi.make_insert_damaged_csv(cata_name, gid, damaged_csv);
            paperwork[ipaper].push_back(stmt);
            log("GID " + gid + " not inserted: damaged.");
        }
        
        // Report to manager thread.
        mycomm[1]++;
        tally++;
        if (tally % worker_batch == 0)
        {
            success = sbjudi.push(myid);
            if (success)
            {
                all_queue[my_id].insert(all_queue[my_id].end(), paperwork.begin(), paperwork.end());
                success = sbjudi.done(myid);
                if (!success) { err("sbjudi.done-insert_csvs"); }
                paperwork.clear();
            }
        }
        while (ii == top && paperwork.size() > 0)
        {
            Sleep(10);
            success = sbjudi.push(myid);
            if (success)
            {
                all_queue[my_id].insert(all_queue[my_id].end(), paperwork.begin(), paperwork.end());
                success = sbjudi.done(myid);
                if (!success) { err("sbjudi.done-insert_csvs"); }
                paperwork.clear();
            }
        }

    }
    
    mycomm[0] = 1;
    sbjudi.update(myid, mycomm);
}
void MainWindow::addTGRow(SQLFUNC& sfjudi, STATSCAN& scjudi, string cataName)
{
    string tname = "TG_Row$" + cataName;
    int tg_row_col;
    vector<string> row_queue;
    if (!sfjudi.table_exist(tname))
    {
        log("Begin row indexing (TGR) for catalogue " + cataName);
        tg_row_col = scjudi.make_tgrow_statements(row_queue);
        sfjudi.executor(row_queue[0]);
        row_queue.erase(row_queue.begin());
        sfjudi.safe_col(tname, tg_row_col);
        sfjudi.insert_prepared(row_queue);
        log("Completed row indexing (TGR) for catalogue " + cataName);
    }
}
void MainWindow::addTGRegion(SQLFUNC& sfjudi, STATSCAN& scjudi, string cataYear, string cataName)
{
    string tname = "TG_Region$" + cataName;
    int tg_region_col;
    vector<string> geo_queue;
    if (!sfjudi.table_exist(tname))
    {
        log("Begin region indexing (TGR) for catalogue " + cataName);
        tg_region_col = scjudi.make_tgr_statements(geo_queue, cataYear, cataName);
        sfjudi.executor(geo_queue[0]);
        geo_queue.erase(geo_queue.begin());
        sfjudi.safe_col(tname, tg_region_col);
        sfjudi.insert_prepared(geo_queue);
        log("Completed region indexing (TGR) for catalogue " + cataName);
    }
}
void MainWindow::addTMap(SQLFUNC& sfjudi, vector<int>& gidList, vector<string>& regionList, vector<string>& layerList, string cataName)
{
    string tname = "TMap$" + cataName, stmt, stmt0, mapPath, temp;
    vector<string> params(2), coreDir;
    int index, inum;
    if (!sfjudi.table_exist(tname))
    {
        log("Begin map indexing for catalogue " + cataName);
        stmt = "CREATE TABLE \"" + tname;
        stmt += "\" (GID INTEGER PRIMARY KEY, [Map Path] TEXT);";
        sfjudi.executor(stmt);

        stmt0 = "INSERT OR IGNORE INTO \"" + tname;
        stmt0 += "\" (GID, [Map Path]) VALUES (?, ?);";
        for (int ii = 0; ii < gidList.size(); ii++)
        {
            params[0] = to_string(gidList[ii]);
            mapPath = "TMap$";
            if (layerList[ii] != "")
            {
                index = -1;
                for (int jj = 0; jj < coreDir.size(); jj++)
                {
                    if (layerList[ii] == coreDir[jj])
                    {
                        index = jj;
                        break;
                    }
                }
                if (index < 0)
                {
                    coreDir.push_back(layerList[ii]);
                }
                else if (index < coreDir.size() - 1) // Return to a parent layer.
                {
                    inum = coreDir.size() - 1 - index;
                    for (int jj = 0; jj < inum; jj++)
                    {
                        coreDir.pop_back();
                    }
                }
                for (int jj = 0; jj < coreDir.size(); jj++)
                {
                    mapPath += coreDir[jj] + "$";
                }
            }
            temp = regionList[ii];
            sfjudi.sclean(temp, 1);
            params[1] = mapPath + temp;
            stmt = stmt0;
            bind(stmt, params);
            sfjudi.executor(stmt);
        }
        log("Completed map indexing for catalogue " + cataName);
    }
}

// Display the 'tabbed data' for the selected catalogue.
void MainWindow::on_pB_viewcata_clicked()
{
    QList<QTreeWidgetItem *> cata_to_do = ui->treeW_cataindb->selectedItems();  // Only 1 catalogue can be selected.
    QString qyear = cata_to_do[0]->text(0);
    string syear = qyear.toStdString();
    QString qname = cata_to_do[0]->text(1);
    string sname = qname.toStdString();
    QList<QStringList> qlistviews;
    vector<vector<int>> comm(1, vector<int>());
    comm[0].assign(comm_length, 0);  // Form [control, progress report, size report]
    thread::id myid = this_thread::get_id();
    viewcata_data.resize(2);
    viewcata_data[0] = syear;
    viewcata_data[1] = sname;
    vector<vector<vector<int>>> tree_st2;  // Form [tree index][pl_index][ancestors, node, children].
    vector<vector<string>> tree_pl2;  // Form [tree index][pl_index].
    vector<string> prompt = { syear, sname };
    int error = sb.start_call(myid, 1, comm[0]);
    if (error) { errnum("start_call-pB_viewcata_clicked", error); }
    sb.set_prompt(prompt);
    std::thread dispcata(&MainWindow::display_catalogue, this, ref(sf), ref(sb), ref(qlistviews), ref(tree_st2), ref(tree_pl2));
    while (1)
    {
        Sleep(gui_sleep);
        QCoreApplication::processEvents();
        comm = sb.update(myid, comm[0]);
        if (comm[0][2] == 0)  // If the GUI thread does not yet know the size of the task...
        {
            if (comm[1][2] > 0)  // ... and the manager thread does know, then ...
            {
                comm[0][2] = comm[1][2];
                comm[0][1] = comm[1][1];
                reset_bar(comm[0][2], "Loading catalogue  " + sname);  // ... initialize the progress bar.
                jobs_done = comm[0][1];
                update_bar();
            }
        }
        else
        {
            comm[0][1] = comm[1][1];
            jobs_done = comm[0][1];
            update_bar();
        }

        if (comm[1][0] > 0)
        {

            break;
        }
    }
    dispcata.join();
    error = sb.end_call(myid);
    if (error) { errnum("end_call-pB_viewcata_clicked", error); }
    
    qf.display(ui->treeW_gid, tree_st2[0], tree_pl2[0]);
    ui->listW_csvrows->clear();
    ui->listW_csvrows->addItems(qlistviews[0]);
    qf.display(ui->treeW_csvtree, tree_st2[1], tree_pl2[1]);

    ui->tabW_results->setCurrentIndex(0);
    ui->pB_viewtable->setEnabled(1);

    log("Displayed catalogue " + sname + " on the GUI.");
}
void MainWindow::display_catalogue(SQLFUNC& sf, SWITCHBOARD& sb, QList<QStringList>& qlistviews, vector<vector<vector<int>>>& tree_st2, vector<vector<string>>& tree_pl2)
{
    JFUNC jf;
    sqlite3_stmt* state;
    vector<int> mycomm;
    thread::id myid = this_thread::get_id();
    sb.answer_call(myid, mycomm);
    vector<string> prompt = sb.get_prompt();  // syear, sname.
    mycomm[2] = 3;  // This function has 3 widgets to process.
    sb.update(myid, mycomm);

    // Populate the 'Geographic Region' tree tab.
    tree_st2.resize(2);
    tree_pl2.resize(2);
    string tname = "TG_Region$" + prompt[1];
    sf.select_tree(tname, tree_st2[0], tree_pl2[0]);
    mycomm[1]++;
    sb.update(myid, mycomm);

    // Populate the 'Row Data' tab.
    vector<vector<string>> results;
    vector<string> results1;
    QString qtemp;
    string region_name = tree_pl2[0][0];
    vector<string> search = { "GID" };
    vector<string> conditions = { "Geography = '" + region_name + "'" };
    sf.select(search, prompt[1], results1, conditions);
    string gid = results1[0];
    tname = prompt[1] + "$" + gid;
    search = { "*" };
    sf.select(search, tname, results);
    QStringList row_list;
    for (int ii = 0; ii < results.size(); ii++)
    {
        qtemp = QString::fromStdString(results[ii][0]);
        row_list.append(qtemp);
    }
    qlistviews.append(row_list);
    mycomm[1]++;
    sb.update(myid, mycomm);

    // Populate the 'CSV Structure' tab.
    tname = "TG_Row$" + prompt[1];
    sf.select_tree(tname, tree_st2[1], tree_pl2[1]);
    mycomm[1]++;
    sb.update(myid, mycomm);

    // Report completion to the GUI thread.
    mycomm[0] = 1;
    sb.update(myid, mycomm);
}

// Remove the selected catalogue from the database.
void MainWindow::on_pB_removecata_clicked()
{
    QList<QTreeWidgetItem*> cata_to_do = ui->treeW_cataindb->selectedItems();  // Only 1 catalogue can be selected.
    vector<string> prompt(3);  // syear, sname, desc.
    QString qtemp;
    if (removeCataTemp.size() > 0)
    {
        if (removeCataTemp.size() == 3)
        {
            prompt = removeCataTemp;
            removeCataTemp.clear();
        }
        else { err("removeCataTemp-MainWindow.on_pB_removecata_clicked"); }
    }
    else
    {
        qtemp = cata_to_do[0]->text(0);
        prompt[0] = qtemp.toStdString();
        qtemp = cata_to_do[0]->text(1);
        prompt[1] = qtemp.toStdString();
        qtemp = cata_to_do[0]->text(2);
        prompt[2] = qtemp.toStdString();
    }
    vector<vector<int>> comm(1, vector<int>());
    comm[0].assign(comm_length, 0);  // Form [control, progress report, size report, max columns].
    thread::id myid = this_thread::get_id();
    log("Beginning removal of catalogue " + prompt[1] + " from the database.");
    int error = sb.start_call(myid, 1, comm[0]);
    if (error) { errnum("start_call-pB_removecata_clicked", error); }
    sb.set_prompt(prompt);
    std::thread remcata(&MainWindow::delete_cata, this, ref(sb), ref(sf));
    remcata.detach();
    while (1)
    {
        Sleep(gui_sleep);
        QCoreApplication::processEvents();
        comm = sb.update(myid, comm[0]);
        if (comm[0][2] == 0)  // If the GUI thread does not yet know the size of the task...
        {
            if (comm[1][2] > 0)  // ... and the manager thread does know, then ...
            {
                comm[0][2] = comm[1][2];
                comm[0][1] = comm[1][1];
                reset_bar(comm[1][2], "Removing catalogue  " + prompt[1]);  // ... initialize the progress bar.
                jobs_done = comm[0][1];
                update_bar();
            }
        }
        else
        {
            comm[0][1] = comm[1][1];
            jobs_done = comm[0][1];
            update_bar();
        }

        if (comm[1][0] > 0)
        {
            break;
        }
    }
    error = sb.end_call(myid);
    if (error) { errnum("end_call-pB_removecata_clicked", error); }
    if (prompt[1] == viewcata_data[1])
    {
        ui->treeW_gid->clear();
        ui->listW_csvrows->clear();
        ui->treeW_csvtree->clear();
        ui->tV_viewtable->clearSpans();
        viewcata_data[0].clear();
        viewcata_data[1].clear();
    }
    update_treeW_cataindb();
    log("Removed catalogue " + prompt[1] + " from the database.");
}
void MainWindow::delete_cata(SWITCHBOARD& sb, SQLFUNC& sf)
{
    vector<int> mycomm;
    thread::id myid = this_thread::get_id();
    sb.answer_call(myid, mycomm);
    vector<string> prompt = sb.get_prompt();  // syear, sname, desc.
    mycomm[2] = 9;
    sb.update(myid, mycomm);
    QElapsedTimer timer;

    // Remove all of this catalogue's CSV tables.
    timer.start();
    vector<string> table_list;
    sf.all_tables(table_list);
    vector<string> table_split, csv_list;
    int mode = 1;
    for (int ii = 0; ii < table_list.size(); ii++)
    {
        table_split = jf.list_from_marker(table_list[ii], '$');
        if (table_split[1] == prompt[1])
        {
            csv_list.push_back(table_list[ii]);
        }
    }
    for (int ii = 0; ii < csv_list.size(); ii++)
    {
        sf.remove(csv_list[ii]);
    }
    mycomm[1]++;
    sb.update(myid, mycomm);
    qDebug() << "Time to remove CSV tables: " << timer.restart();

    // Remove the catalogue's primary table.
    sf.remove(prompt[1]);
    mycomm[1]++;
    sb.update(myid, mycomm);
    qDebug() << "Time to remove the primary table: " << timer.restart();

    // Remove the catalogue's TG_Region.
    string tname = "TG_Region$" + prompt[1];
    sf.remove(tname);
    mycomm[1]++;
    sb.update(myid, mycomm);
    qDebug() << "Time to remove TG_Region: " << timer.restart();

    // Remove the catalogue's TG_Row.
    tname = "TG_Row$" + prompt[1];
    sf.remove(tname);
    mycomm[1]++;
    sb.update(myid, mycomm);
    qDebug() << "Time to remove TG_Row: " << timer.restart();

    // Remove the catalogue's TMap.
    tname = "TMap$" + prompt[1];
    sf.remove(tname);
    mycomm[1]++;
    sb.update(myid, mycomm);
    qDebug() << "Time to remove TMap: " << timer.restart();

    // Remove the catalogue's Geo.
    tname = prompt[1] + "$Geo";
    sf.remove(tname);
    mycomm[1]++;
    sb.update(myid, mycomm);
    qDebug() << "Time to remove Geo: " << timer.restart();

    // Remove the catalogue's Geo_Layers.
    tname = prompt[1] + "$Geo_Layers";
    sf.remove(tname);
    mycomm[1]++;
    sb.update(myid, mycomm);
    qDebug() << "Time to remove Geo_Layers: " << timer.restart();

    // Remove entries from TDamaged.
    vector<string> conditions = {"[Catalogue Name] = '" + prompt[1] + "'"};
    tname = "TDamaged";
    sf.remove(tname, conditions);
    mycomm[1]++;
    sb.update(myid, mycomm);
    qDebug() << "Time to remove entries from TDamaged: " << timer.restart();

    // Remove entry from TCatalogueIndex.
    tname = "TCatalogueIndex";
    conditions = { "[Name] = '" + prompt[1] + "'" };
    sf.remove(tname, conditions);
    mycomm[1]++;
    qDebug() << "Time to remove entry from TCatalogueIndex: " << timer.restart();

    // Report completion to the GUI.
    mycomm[0] = 1;
    sb.update(myid, mycomm);
}

// All threads inserting catalogues are told to stop after finishing their current CSV.
void MainWindow::on_pB_cancel_clicked()
{
    remote_controller = 2;
}

// Display the raw table data for the selected region/table.
void MainWindow::on_pB_viewtable_clicked()
{
    int tab_index = ui->tabW_results->currentIndex();
    int row, kids;
    wstring wTemp;
    string temp, gid, tname, row_title, result;
    vector<string> row_split, search, conditions;
    QList<QTreeWidgetItem*> qcurrent;
    QTreeWidgetItem* qitem;
    QString qtemp;

    switch (tab_index)
    {
    case 0:
    {
        // Obtain the CSV table name.
        qcurrent = ui->treeW_gid->selectedItems();
        if (qcurrent.size() < 1) { return; }
        qtemp = qcurrent[0]->text(0);
        wTemp = qtemp.toStdWString();
        temp = qtemp.toStdString();
        string temp2 = "'";
        jf.clean(temp, { "" }, temp2);
        search = { "GID" };
        tname = "TG_Region$" + viewcata_data[1];
        conditions = { "[Region Name] = '" + temp + "'" };
        sf.select(search, tname, gid, conditions);
        tname = viewcata_data[1] + "$" + gid;
        break;
    }
    case 3:
    {
        QList<QListWidgetItem*> qSelected = ui->listW_search->selectedItems();
        if (qSelected.size() != 1) { return; }
        qtemp = qSelected[0]->text();
        tname = qtemp.toStdString();
        break;
    }
    }

    display_table(tname);
}
void MainWindow::display_table(string tname)
{
    // Note that this helper function is meant to be run in the GUI thread. 

    // Pull the CSV table data and plug it in.
    QStandardItemModel* model = new QStandardItemModel;
    QStandardItem* cell;
    vector<string> column_titles;
    sf.get_col_titles(tname, column_titles);
    QStringList qcolumn_titles, qrow_titles;
    QString qtemp;
    for (int ii = 1; ii < column_titles.size(); ii++)
    {
        qtemp = QString::fromStdString(column_titles[ii]);
        qcolumn_titles.append(qtemp);
    }
    vector<vector<string>> results;
    int max_col = sf.select({ "*" }, tname, results);
    int title_size, line_breaks, height, pos1;
    vector<vector<int>> row_heights;
    for (int ii = 0; ii < results.size(); ii++)
    {
        jf.tclean(results[ii][0], '+', "   ");
        qtemp = QString::fromStdString(results[ii][0]);
        title_size = qtemp.size();
        if (title_size > qrow_title_width)
        {
            line_breaks = title_size / qrow_title_width;
            for (int jj = 1; jj < line_breaks + 1; jj++)
            {
                pos1 = qtemp.lastIndexOf(' ', jj * qrow_title_width);
                qtemp.insert(pos1, " \n");
            }
            height = (line_breaks + 1) * qrow_title_width;
            row_heights.push_back(vector<int>(2));
            row_heights[row_heights.size() - 1][0] = ii;
            row_heights[row_heights.size() - 1][1] = height;
        }
        qrow_titles.append(qtemp);
    }
    model->setRowCount(results.size());
    model->setColumnCount(max_col - 1);
    model->setHorizontalHeaderLabels(qcolumn_titles);
    model->setVerticalHeaderLabels(qrow_titles);
    for (int ii = 0; ii < results.size(); ii++)
    {
        for (int jj = 1; jj < results[ii].size(); jj++)
        {
            qtemp = QString::fromStdString(results[ii][jj]);
            cell = new QStandardItem;
            cell->setText(qtemp);
            model->setItem(ii, jj - 1, cell);
        }
    }

    toggleTableGeometry();
    ui->tV_viewtable->setModel(model);
    for (int ii = 0; ii < row_heights.size(); ii++)
    {
        ui->tV_viewtable->setRowHeight(row_heights[ii][0], row_heights[ii][1]);
    }
    ui->tabW_results2->setCurrentIndex(0);
}
void MainWindow::toggleTableGeometry()
{
    int xCoord = ui->tabW_results2->x();
    if (xCoord > 500)
    {
        ui->tabW_results2->setGeometry(10, 10, 1241, 641);
        ui->tV_viewtable->setGeometry(0, 0, 1226, 616);
    }
    else
    {
        ui->tabW_results2->setGeometry(760, 10, 496, 641);
        ui->tV_viewtable->setGeometry(0, 0, 486, 611);
    }
}

// Remove the selected table from the database. 
void MainWindow::on_pB_deletetable_clicked()
{
    int tab_index = ui->tabW_results->currentIndex();
    int row, kids;
    wstring wTemp;
    string temp, gid, tname, row_title, result;
    vector<string> row_split, search, conditions;
    QList<QTreeWidgetItem*> qcurrent;
    QTreeWidgetItem* qitem;
    QString qtemp;

    // Get the table name. 
    switch (tab_index)
    {
    case 0:
    {
        // Obtain the CSV table name.
        qcurrent = ui->treeW_gid->selectedItems();
        if (qcurrent.size() < 1) { return; }
        qtemp = qcurrent[0]->text(0);
        wTemp = qtemp.toStdWString();
        temp = qtemp.toStdString();
        string temp2 = "'";
        jf.clean(temp, { "" }, temp2);
        search = { "GID" };
        tname = "TG_Region$" + viewcata_data[1];
        conditions = { "[Region Name] = '" + temp + "'" };
        sf.select(search, tname, gid, conditions);
        tname = viewcata_data[1] + "$" + gid;
        break;
    }
    case 3:
    {
        QList<QListWidgetItem*> qSelected = ui->listW_search->selectedItems();
        if (qSelected.size() != 1) { return; }
        qtemp = qSelected[0]->text();
        tname = qtemp.toStdString();
        break;
    }
    }

    // Delete.
    sf.remove(tname);

}

// Query Statistics Canada for information.
void MainWindow::on_pB_usc_clicked()
{
    int error, inum, uscMode, iyear, numMap;
    int iroot = -1;
    string temp, syear, filePath, sname, webpage, url, fileType;
    vector<string> slayer;
    vector<int> ilayer;
    QTreeWidgetItem* qnode;
    QString qtemp;
    size_t pos1;
    QList<QTreeWidgetItem*> qSelected = ui->treeW_statscan->selectedItems();
    if (qSelected.size() < 1) { uscMode = 0; }
    else
    {
        qnode = qSelected[0]->parent();
        uscMode = 0;
        while (qnode != nullptr)
        {
            qnode = qnode->parent();
            uscMode++;
        }
    }

    switch (uscMode)
    {
    case 0:  // Set root, display years.
    {
        webpage = wf.browseS(scroot);
        slayer = jf.textParser(webpage, navSearch[0]);
        jf.isort_slist(slayer);
        sname = "Statistics Canada Census Data";
        temp = sroot;
        jtStatsCan.init(sname, temp);
        ilayer = jf.svectorToIvector(slayer);
        jtStatsCan.addChildren(slayer, ilayer, iroot);
        ui->treeW_statscan->clear();
        qnode = new QTreeWidgetItem();
        qtemp = QString::fromStdString(sname);
        qnode->setText(0, qtemp);
        ui->treeW_statscan->addTopLevelItem(qnode);
        populateQtree(jtStatsCan, qnode, sname);
        ui->treeW_statscan->expandAll();
        break;
    }
    case 1:  // Set year, display catalogues.
    {
        qtemp = qSelected[0]->text(0);
        syear = qtemp.toStdString();
        url = sc.urlYear(syear);
        webpage = wf.browseS(url);
        slayer = jf.textParser(webpage, navSearch[1]);
        ilayer.assign(slayer.size(), -1);
        jtStatsCan.addChildren(slayer, ilayer, syear);
        populateQtree(jtStatsCan, qSelected[0], syear);
        qSelected[0]->sortChildren(0, Qt::SortOrder::AscendingOrder);
        break;
    }
    case 2:  // Set catalogue, display data types.
    {
        qtemp = qSelected[0]->text(0);
        sname = qtemp.toStdString();
        qtemp = qSelected[0]->parent()->text(0);
        syear = qtemp.toStdString();
        QList<QTreeWidgetItem*> qChildren = qSelected[0]->takeChildren();
        if (qChildren.size() > 0)
        {
            foreach(QTreeWidgetItem * child, qChildren)
            {
                delete child;
            }
        }
        try { iyear = stoi(syear); }
        catch (out_of_range& oor) { err("stoi-MainWindow.on_pB_usc_clicked"); }
        slayer = { "Local CSVs", "Online CSVs" };
        ilayer.resize(2);  
        temp = sroot + "\\" + syear + "\\" + sname;
        ilayer[0] = wf.get_file_path_number(temp, ".csv");

        vector<string> geoLayers;
        sc.initGeo();
        temp += "\\" + sname + " geo list.bin";
        if (!wf.file_exist(temp))
        {
            ilayer[1] = fetchGeoList(iyear, sname, geoLayers);
        }
        else if (!sc.testGeoList(temp))
        {
            ilayer[1] = fetchGeoList(iyear, sname, geoLayers);
        }
        else
        {
            ilayer[1] = sc.skimGeoList(temp, geoLayers);
        }

        if (iyear >= 2011)
        {
            if (geoLayers.size() < 1) { err("No geoLayers-MainWindow.on_pB_usc_clicked"); }
            slayer.push_back("Local PDF Maps");
            temp = sroot + "\\mapsPDF";
            numMap = 0;
            for (int ii = 0; ii < geoLayers.size(); ii++)
            {
                if (geoLayers[ii] != "")
                {
                    temp += "\\" + geoLayers[ii];
                }
                inum = wf.get_file_path_number(temp, ".pdf");
                numMap += inum;
            }
            ilayer.push_back(numMap);

            slayer.push_back("Local BIN Maps");
            temp = sroot + "\\mapsBIN";
            numMap = 0;
            for (int ii = 0; ii < geoLayers.size(); ii++)
            {
                if (geoLayers[ii] != "")
                {
                    temp += "\\" + geoLayers[ii];
                }
                inum = wf.get_file_path_number(temp, ".bin");
                numMap += inum;
            }
            ilayer.push_back(numMap);
        }

        jtStatsCan.deleteChildren(sname);
        jtStatsCan.addChildren(slayer, ilayer, sname);
        populateQtree(jtStatsCan, qSelected[0], sname);
        qSelected[0]->setExpanded(1);
        break;
    }
    case 3:  // Set data type, display discrepancies.
    {
        qtemp = qSelected[0]->text(0);
        fileType = qtemp.toStdString();
        qtemp = qSelected[0]->parent()->text(0);
        sname = qtemp.toStdString();
        qtemp = qSelected[0]->parent()->parent()->text(0);
        syear = qtemp.toStdString();
        pos1 = fileType.find("CSVs");
        if (pos1 < fileType.size())
        {
            temp = sroot + "\\" + syear + "\\" + sname;
            displayDiscrepancies(temp, ui->listW_statscan);
        }
        break;
    }
    }
    ui->tabW_online->setCurrentIndex(0);
    int bbq = 1;
}
void MainWindow::populateQtree(JTREE& jtx, QTreeWidgetItem*& qparent, string sparent)
{
    bool twig = 0;
    bool maps = 0;
    vector<int> ikids;
    vector<string> skids; 
    jtx.listChildren(sparent, ikids, skids);
    if (skids.size() < 1) { return; }  // Leaf node.
    string temp = "Local CSVs";
    if (skids[0] == temp) { twig = 1; }
    //string rootName = jtx.getRootName();
    string rootName;
    temp = "Local Maps";
    if (rootName == temp) { maps = 1; }

    QString qtemp;
    QTreeWidgetItem* qkid;
    QList<QTreeWidgetItem*> qkids;
    for (int ii = 0; ii < skids.size(); ii++)
    {
        qtemp = QString::fromUtf8(skids[ii]);
        qkid = new QTreeWidgetItem(qparent);
        qkid->setText(0, qtemp);
        if (twig)
        {
            qtemp.setNum(ikids[ii]);
            qkid->setText(1, qtemp);
        }
        else if (maps)
        {
            qtemp = QString::number(0);
            qkid->setText(1, qtemp);
        }
        qkids.append(qkid);
    }
    for (int ii = 0; ii < skids.size(); ii++)
    {
        populateQtree(jtx, qkids[ii], skids[ii]);
    }
}
int MainWindow::getCataStatus(string sYear, string sName, vector<string>& csvLocal, vector<string>& csvOnline)
{
    // Returns 0 = cata absent, 1 = cata partially downloaded, 2 = cata completely downloaded.
    string folderPath = sroot + "\\" + sYear + "\\" + sName;
    wf.makeDir(folderPath);
    csvLocal = wf.get_file_list(folderPath, "*.csv");

    sc.initGeo();
    string geoPath = folderPath + "\\" + sName + " geo list.bin";
    vector<string> geoLayers;
    int iYear;
    try { iYear = stoi(sYear); }
    catch (invalid_argument& ia) { err("stoi-MainWindow.getCataStatus"); }
    if (!wf.file_exist(geoPath)) { fetchGeoList(iYear, sName, geoLayers); }
    else if (!sc.testGeoList(geoPath)) { fetchGeoList(iYear, sName, geoLayers); }
    vector<vector<string>> geoAll = sc.readGeo(geoPath);
    csvOnline.resize(geoAll.size() - 1);
    for (int ii = 0; ii < csvOnline.size(); ii++)
    {
        csvOnline[ii] = sName + " (" + geoAll[ii + 1][0] + ") " + geoAll[ii + 1][1] + ".csv";
    }

    if (csvLocal.size() == 0) { return 0; }
    else if (csvOnline.size() > csvLocal.size()) { return 1; }
    else if (csvOnline.size() == csvLocal.size()) { return 2; }
    return -1;
}
int MainWindow::fetchGeoList(int iYear, string sCata, vector<string>& geoLayers)
{
    // Will download a geo list file (including geo layers) in the catalogue's folder.
    string urlCata = sc.urlCatalogue(iYear, sCata);
    string geoURL = sc.urlGeoList(iYear, urlCata);
    string geoPage = wf.browseS(geoURL);
    int numRegions = fetchGeoList(iYear, sCata, geoLayers, geoPage, geoURL);
    return numRegions;
}
int MainWindow::fetchGeoList(int iYear, string sCata, vector<string>& geoLayers, string& geoPage, string geoURL)
{
    // Will download a geo list file (including geo layers) in the catalogue's folder.
    vector<string> geoTemp = jf.textParser(geoPage, navSearch[6]);
    geoLayers = sc.makeGeoLayers(geoTemp[0]);
    vector<string> geoLinkNames = jf.textParser(geoPage, navSearch[2]);
    string geoList = sc.makeGeoList(geoLinkNames, geoLayers, geoURL);
    string temp = sroot + "\\" + to_string(iYear) + "\\" + sCata;
    temp += "\\" + sCata + " geo list.bin";
    jf.printer(temp, geoList);
    int numRegions = geoLinkNames.size();
    return numRegions;
}
void MainWindow::displayDiscrepancies(string& folderPath, QListWidget*& qlist)
{
    vector<int> gidLocal, gidOnline;
    vector<string> csvLocal, csvOnline, geoLayers;
    string sfile, temp, syear, sCata;
    QString qtemp;
    int iYear;
    size_t pos2;
    size_t pos1 = folderPath.find("maps");
    if (pos1 < folderPath.size())
    {
        // Add later.
    }
    else
    {
        wf.makeDir(folderPath);
        csvLocal = wf.get_file_list(folderPath, "*.csv");
        gidLocal.resize(csvLocal.size());
        for (int ii = 0; ii < csvLocal.size(); ii++)
        {
            pos1 = csvLocal[ii].find('(') + 1;
            pos2 = csvLocal[ii].find(')', pos1);
            temp = csvLocal[ii].substr(pos1, pos2 - pos1);
            try { gidLocal[ii] = stoi(temp); }
            catch (invalid_argument& ia) { err("stoi-MainWindow.displayDiscrepancies"); }
        }

        pos1 = folderPath.rfind('\\') + 1;
        sCata = folderPath.substr(pos1);
        temp = folderPath + "\\" + sCata + " geo list.bin";
        if (!wf.file_exist(temp))
        {
            pos1 = temp.find('\\') + 1;
            pos2 = temp.find('\\', pos1);
            syear = temp.substr(pos1, pos2 - pos1);
            try { iYear = stoi(syear); }
            catch (invalid_argument& ia) { err("stoi-MainWindow.displayDiscrepancies"); }
            fetchGeoList(iYear, sCata, geoLayers);
        }
        sfile = jf.load(temp);
        pos1 = sfile.find_first_of("1234567890");
        pos2 = sfile.find('$');
        while (pos2 < sfile.size())
        {
            temp = sfile.substr(pos1, pos2 - pos1);
            try { gidOnline.push_back(stoi(temp)); }
            catch (invalid_argument& ia) { err("stoi-MainWindow.displayDiscrepancies"); }
            pos1 = pos2 + 1;
            pos2 = sfile.find('$', pos1);
            temp = sfile.substr(pos1, pos2 - pos1);
            csvOnline.push_back(temp);
            pos1 = sfile.find('\n', pos2) + 1;
            pos2 = sfile.find('$', pos1);
        }

        for (int ii = gidLocal.size() - 1; ii >= 0; ii--)
        {
            for (int jj = gidOnline.size() - 1; jj >= 0; jj--)
            {
                if (gidLocal[ii] == gidOnline[jj])
                {
                    csvOnline.erase(csvOnline.begin() + jj);
                    gidOnline.erase(gidOnline.begin() + jj);
                    break;
                }
            }
        }
        qlist->clear();
        if (gidOnline.size() < 1)
        {
            temp = "No discrepancies!";
            qtemp = QString::fromUtf8(temp);
            qlist->addItem(qtemp);
        }
        for (int ii = 0; ii < gidOnline.size(); ii++)
        {
            temp = "(" + to_string(gidOnline[ii]) + ") " + csvOnline[ii];
            qtemp = QString::fromUtf8(temp);
            qlist->addItem(qtemp);
        }
    }
}

// Toggle between local database mode, and online Stats Canada navigation.
void MainWindow::on_pB_mode_clicked()
{
    QString qtemp;
    if (active_mode == 0)
    {
        active_mode = 1;
        qtemp = QString::fromStdString(modes[active_mode]);
        ui->label_mode->setText(qtemp);
    }
    else if (active_mode == 1)
    {
        active_mode = 0;
        qtemp = QString::fromStdString(modes[active_mode]);
        ui->label_mode->setText(qtemp);
    }
    update_mode();
}

// Search the database for a table name. If an exact match is found, display 
// that table's contents. If a partial match is found, display a list of 
// tables which contain the search parameter. 
void MainWindow::on_pB_search_clicked()
{
    QString qtemp = ui->pte_localinput->toPlainText();
    string tname = qtemp.toStdString(), temp, sYear, sName, filePath;
    size_t pos1 = tname.find("delete");
    vector<string> results, regionList, layerList, geoLayers;
    vector<int> gidList;
    QStringList qlist;
    long long timer;
    jf.timerStart();
    if (pos1 == 0)
    {
        temp = tname.substr(6);
        sf.all_tables(results);
        for (int ii = 0; ii < results.size(); ii++)
        {
            pos1 = results[ii].find(temp);
            if (pos1 < results[ii].size())
            {
                sf.remove(results[ii]);
            }
        }
        tname = "Deleted " + temp + " !";
        qtemp = QString::fromStdString(tname);
        ui->pte_localinput->setPlainText(qtemp);
    }
    else if (tname == "all")
    {
        ui->listW_search->clear();
        sf.all_tables(results);
        for (int ii = 0; ii < results.size(); ii++)
        {
            qtemp = QString::fromStdString(results[ii]);
            ui->listW_search->addItem(qtemp);
        }
        ui->tabW_results->setCurrentIndex(3);
        temp = "Search returned " + to_string(results.size()) + " results.";
        qtemp = QString::fromStdString(temp);
        ui->pte_localinput->setPlainText(qtemp);
    }
    else if (tname == "insertGeo")
    {
        QList<QTreeWidgetItem*> qSel = ui->treeW_cataindb->selectedItems();
        if (qSel.size() != 1) { return; }
        qtemp = qSel[0]->text(0);
        sYear = qtemp.toStdString();
        qtemp = qSel[0]->text(1);
        sName = qtemp.toStdString();
        filePath = sroot + "\\" + sYear + "\\" + sName + "\\" + sName + " geo list.bin";
        sc.loadGeo(filePath, gidList, regionList, layerList, geoLayers);
        sf.insertGeo(sName, gidList, regionList, layerList, geoLayers);
        barMessage(sName + " done!");
    }
    else if (tname == "TG_Row")
    {
        QList<QTreeWidgetItem*> qSel = ui->treeW_cataindb->selectedItems();
        if (qSel.size() != 1) { return; }
        qtemp = qSel[0]->text(0);
        sYear = qtemp.toStdString();
        qtemp = qSel[0]->text(1);
        sName = qtemp.toStdString();
        string folderPath = sroot + "\\" + sYear + "\\" + sName;
        string search = "*.csv";
        vector<string> nameList = wf.get_file_list(folderPath, search);
        string csvPath = folderPath + "\\" + nameList[0];
        sc.set_path(csvPath);
        string sfile = jf.load(csvPath);
        sc.cata_init(sfile);
        temp = "TG_Row$" + sName;
        sf.remove(temp);
        vector<string> tgRowStmts;
        sc.make_tgrow_statements(tgRowStmts);
        for (int ii = 0; ii < tgRowStmts.size(); ii++)
        {
            sf.executor(tgRowStmts[ii]);
        }
        int bbq = 1;
    }
    else if (sf.table_exist(tname))
    {
        timer = jf.timerRestart();
        qDebug() << "pB_search to table_exist: " << timer;
        display_table(tname);
        timer = jf.timerStop();
        qDebug() << "display_table: " << timer;
    }
    else
    {
        sf.get_table_list(results, tname);
        if (results.size() > 0)
        {
            for (int ii = 0; ii < results.size(); ii++)
            {
                qtemp = QString::fromStdString(results[ii]);
                qlist.append(qtemp);
            }
        }
        else
        {
            qtemp = "No results found.";
            qlist.append(qtemp);
        }
        ui->listW_search->clear();
        ui->listW_search->addItems(qlist);
        ui->tabW_results->setCurrentIndex(3);
    }
}


void MainWindow::populateQTree(JTREE& jtx, QTreeWidgetItem*& qMe, string myName)
{
    // Given a JTREE structure and a qRoot, make a qTree from the JTREE.
    string temp;
    wstring wtemp;
    vector<int> ikids;
    vector<string> skids;
    jtx.listChildren(myName, ikids, skids);
    if (skids.size() < 1) { return; }  // Leaf node.

    size_t pos1;
    QString qtemp;
    QTreeWidgetItem* qkid;
    QList<QTreeWidgetItem*> qkids;
    for (int ii = 0; ii < skids.size(); ii++)
    {
        pos1 = skids[ii].rfind('\\');
        if (pos1 > skids[ii].size())
        {
            temp = skids[ii];
        }
        else
        {
            temp = skids[ii].substr(pos1 + 1);
        }
        wtemp = jf.utf8to16(temp);
        qtemp = QString::fromStdWString(wtemp);
        qkid = new QTreeWidgetItem(qMe);
        qkid->setText(0, qtemp);
        if (ikids[ii] == -1)  // Folder
        {
            qkid->setText(1, "");
        }
        else
        {
            qtemp.setNum(ikids[ii]);
            qkid->setText(1, qtemp);
        }
        qkids.append(qkid);
    }
    for (int ii = 0; ii < qkids.size(); ii++)
    {
        populateQTree(jtx, qkids[ii], skids[ii]);
    }
}

// Convert a downloaded PDF map into a BIN map.
void MainWindow::on_pB_convert_clicked()
{
    // Determine the selected folder path containing files to convert.
    QList<QTreeWidgetItem*> qlist = ui->treeW_maps->selectedItems();
    QTreeWidgetItem* qNode = nullptr, *qParent = nullptr;
    if (qlist.size() < 1) { return; }
    selectedMapFolder = qf.makePathTree(qlist[0]);
    QString qtemp = qlist[0]->text(1), qMessage;
    string temp, folderPath;
    if (qtemp == "")  // Folder selected.
    {
        qtemp = qlist[0]->text(0);
        qParent = qlist[0]->parent();
    }
    else  // Leaf selected.
    {
        qtemp = qlist[0]->parent()->text(0);
        qParent = qlist[0]->parent()->parent();
    }
    folderPath = qtemp.toStdString();
    while (qParent != nullptr)
    {
        qtemp = qParent->text(0);
        folderPath = qtemp.toStdString() + "\\" + folderPath;
        qNode = qParent;
        qParent = qNode->parent();
    }
    size_t pos1 = folderPath.find('\\');
    temp = folderPath.substr(pos1);
    folderPath = sroot + temp;
    pos1 = temp.find('\\', 1);
    temp = temp.substr(5, pos1 - 5);

    // Prepare the worker thread's prompt.
    int inum;
    vector<string> prompt(2), dirt;
    if (temp == "BIN")
    {
        qtemp = "BIN files already converted!";
        ui->pte_webinput->setPlainText(qtemp);
        return;
    }
    else if (temp == "PNG")
    {
        qtemp = ui->pte_webinput->toPlainText();
        temp = qtemp.toStdString();
        if (temp != "")
        {
            dirt = { " " };
            jf.clean(temp, dirt);
            try { inum = stoi(temp); }
            catch (invalid_argument& ia) { inum = -1; }
        }
        else { inum = -1; }
        prompt[0] = to_string(inum) + ",";
        inum = ui->label_maps->width();
        prompt[0] += to_string(inum) + ",";
        inum = ui->label_maps->height();
        prompt[0] += to_string(inum);
        prompt[1] = folderPath;
    }

    // Prepare the GUI thread to act as a bridge between the user and the worker thread.
    qf.initPixmap(ui->label_maps);
    thread::id myid = this_thread::get_id();
    vector<vector<int>> comm(1, vector<int>());
    comm[0].assign(comm_length, 0);  // Form [control, progress report, size report, max param].
    vector<vector<double>> pathBorder, pathBorderBuffer;
    vector<double> mapShift;
    QPainterPath painterPathBorder, myPPB;
    vector<int> colourDots = { 3, 2, 1 };  // Green, Yellow, Red.
    vector<vector<int>> dots;

    // Launch the worker thread.
    int error = sb.start_call(myid, 1, comm[0]);
    if (error) { errnum("start_call-MainWindow.on_pB_convert", error); }
    sb.set_prompt(prompt);
    vector<string> pathBin, pathBinShared, vsTemp;
    std::thread thr(&MainWindow::convertGuide, this, ref(sb), ref(painterPathBorder), ref(pathBinShared));
    thr.detach();
    ui->tabW_online->setCurrentIndex(2);
    ui->pB_pause->setEnabled(1);

    // Receive and display path data. 
    double lenPPB;
    string sController, sBar;
    vector<string> debugPath, bagOfAir;
    bool letMeOut = 0;
    int local_controller = remote_controller, iController;
    while (!letMeOut)
    {
        Sleep(gui_sleep);
        comm = sb.update(myid, comm[0]);
        if (local_controller != remote_controller)
        {
            // NOTE: Controller codes must be limited to [0,9].
            sController = to_string(local_controller) + to_string(remote_controller);
            try { iController = stoi(sController); }
            catch (invalid_argument& ia) { err("stoi-MainWindow.on_pB_convert_clicked"); }
            switch (iController)
            {
            case 1:   // Run->Advance
            {
                local_controller = 1;
                break;
            }
            case 2:   // Run->Cancel
            {
                comm[0][0] = 2;
                sb.update(myid, comm[0]);
                while (1)
                {
                    Sleep(gui_sleep);
                    comm = sb.update(myid, comm[0]);
                    if (comm[1][0] == 2)
                    {
                        barMessage("Map conversions cancelled.");
                        remote_controller = 0;
                        return;
                    }
                }
                break;
            }
            case 3:   // Run->Pause
            {
                comm[0][0] = 3;
                sb.update(myid, comm[0]);
                local_controller = 3;
                barMessage("PAUSED");
                while (comm[1][0] != 3)
                {
                    Sleep(gui_sleep);
                    QCoreApplication::processEvents();
                    comm = sb.update(myid, comm[0]);
                }
                break;
            }
            case 30:  // Pause->Run
            {
                comm[0][0] = 0;
                sb.update(myid, comm[0]);
                local_controller = 0;
                sBar = "Converting maps (" + to_string(comm[0][1]);
                sBar += "/" + to_string(comm[0][2]) + ") ...";
                barMessage(sBar);
                break;
            }
            }
        }
        switch (local_controller)
        {
        case 0:
        {
            if (comm[1][2] > comm[0][2])
            {
                comm[0][2] = comm[1][2];
                sBar = "Converting maps (1/" + to_string(comm[0][2]) + ") ...";
                reset_bar(comm[0][2], sBar);
            }
            if (comm[1][0] == 3)
            {
                debugPath = sb.get_prompt();
                sb.set_prompt(bagOfAir);
                qf.displayDebug(ui->label_maps, debugPath, debugMapCoord);
                if (debugPath.size() > 1)
                {
                    try
                    {
                        inum = stoi(debugPath[1]);
                        qMessage = "Center point index: ";
                        qMessage.append(debugPath[1].c_str());
                        if (debugPath.size() > 2)
                        {
                            pos1 = debugPath[2].rfind('\\') + 1;
                            temp = debugPath[2].substr(pos1);
                            qtemp = QString::fromStdString(temp);
                            qMessage.append("\n");
                            qMessage.append(qtemp);
                        }
                        ui->pte_webinput->setPlainText(qMessage);
                    }
                    catch (invalid_argument& ia) {}
                }
                ui->pB_resume->setEnabled(1);
                ui->pB_pause->setEnabled(0);
                ui->pB_advance->setEnabled(1);
                ui->pB_backspace->setEnabled(1);
                remote_controller = 3;
                comm[0][0] = 3;
                while (1)
                {
                    QCoreApplication::processEvents();
                    comm = sb.update(myid, comm[0]);
                    if (remote_controller == 0 && comm[0][0] == 3)
                    {
                        //vsTemp = { "" };
                        //sb.set_prompt(vsTemp);
                        comm[0][0] = 0;
                        sb.update(myid, comm[0]);
                    }
                    else if (remote_controller == 1 && comm[0][0] == 3)
                    {
                        vsTemp = sb.get_prompt();
                        if (vsTemp.size() == 3)  // Keep coords in vsTemp[2].
                        {
                            vsTemp[0] = to_string(advBuffer);
                        }
                        else
                        {
                            vsTemp = { to_string(advBuffer) };
                        }
                        advBuffer = -1;
                        sb.set_prompt(vsTemp);
                        comm[0][0] = 0;
                        sb.update(myid, comm[0]);
                    }
                    else if (remote_controller == 4 && comm[0][0] == 3)
                    {
                        vsTemp = { to_string(-1 * backBuffer) };
                        backBuffer = -1;
                        sb.set_prompt(vsTemp);
                        comm[0][0] = 0;
                        sb.update(myid, comm[0]);
                    }
                    if (comm[1][0] == 0)
                    {
                        local_controller = 0;
                        if (remote_controller == 4) { remote_controller = 1; }
                        ui->checkB_override->setChecked(0);
                        break;
                    }
                    Sleep(50);
                }
            }
            if (comm[1][1] > comm[0][1])  // New BIN map available to display.
            {
                error = sb.pull(myid, 0);
                if (error < 0) { err("sb.pull-MainWindow.on_pB_convert"); }
                myPPB = painterPathBorder;
                pathBin.insert(pathBin.end(), pathBinShared.begin(), pathBinShared.end());
                pathBinShared.clear();
                painterPathBorder.clear();
                sb.done(myid);
                comm[0][1] = comm[1][1];
                jobs_done = comm[0][1];
                update_bar();
                sBar = "Converting maps (" + to_string(comm[0][1]);
                sBar += "/" + to_string(comm[0][2]) + ") ...";
                barMessage(sBar);
                lenPPB = myPPB.length();
                if (lenPPB > 0.1) { qf.displayBin(ui->label_maps, myPPB); }
                else if (pathBin.size() > 0) { qf.displayBin(ui->label_maps, pathBin[pathBin.size() - 1]); }
                qf.displayBinList(ui->listW_bindone, pathBin);
            }
            if (comm[1][0] == 1)
            {
                error = sb.end_call(myid);
                if (error) { errnum("sb.end_call-on_pB_insert", error); }
                jobs_done = comm[0][2];
                update_bar();
                barMessage("Map conversions completed.");
                letMeOut = 1;
                break;
            }
            QCoreApplication::processEvents();
            break;
        }
        case 1:
        {
            sb.update(myid, comm[0]);
            if (comm[1][0] == 3)
            {
                debugPath = sb.get_prompt();
                sb.set_prompt(bagOfAir);
                qf.displayDebug(ui->label_maps, debugPath, debugMapCoord);
                if (debugPath.size() > 1)
                {
                    try
                    {
                        inum = stoi(debugPath[1]);
                        qtemp = "Center point index: ";
                        qtemp.append(debugPath[1].c_str());
                        ui->pte_webinput->setPlainText(qtemp);
                    }
                    catch (invalid_argument& ia) {}
                }
                ui->pB_resume->setEnabled(1);
                ui->pB_pause->setEnabled(0);
                ui->pB_advance->setEnabled(1);
                ui->pB_backspace->setEnabled(1);
                remote_controller = 3;
                comm[0][0] = 3;
                while (1)
                {
                    QCoreApplication::processEvents();
                    comm = sb.update(myid, comm[0]);
                    if (remote_controller == 0 && comm[0][0] == 3)
                    {
                        //vsTemp = { "" };
                        //sb.set_prompt(vsTemp);
                        comm[0][0] = 0;
                        sb.update(myid, comm[0]);
                    }
                    else if (remote_controller == 1 && comm[0][0] == 3)
                    {
                        vsTemp = sb.get_prompt();
                        if (vsTemp.size() == 3)  // Keep coords in vsTemp[2].
                        {
                            vsTemp[0] = to_string(advBuffer);
                        }
                        else
                        {
                            vsTemp = { to_string(advBuffer) };
                        }
                        advBuffer = -1;
                        sb.set_prompt(vsTemp);
                        comm[0][0] = 0;
                        sb.update(myid, comm[0]);
                    }
                    else if (remote_controller == 4 && comm[0][0] == 3)
                    {
                        vsTemp = { to_string(-1 * backBuffer) };
                        backBuffer = -1;
                        sb.set_prompt(vsTemp);
                        comm[0][0] = 0;
                        sb.update(myid, comm[0]);
                    }
                    if (comm[1][0] == 0)
                    {
                        local_controller = 0;
                        if (remote_controller == 4) { remote_controller = 1; }
                        break;
                    }
                    Sleep(50);
                }
            }
            QCoreApplication::processEvents();
            break;
        }
        case 3:
        {
            if (comm[1][0] == 0) 
            {
                comm[0][0] = 0; 
                local_controller = 0;
            }
            if (comm[1][0] == 3)
            {
                debugPath = sb.get_prompt();
                qf.displayDebug(ui->label_maps, debugPath, debugMapCoord);
                if (debugPath.size() > 1)
                {
                    try 
                    { 
                        inum = stoi(debugPath[1]); 
                        qtemp = "Center point index: ";
                        qtemp.append(debugPath[1].c_str());
                        ui->pte_webinput->setPlainText(qtemp);
                    }
                    catch (invalid_argument& ia) {}
                }
                ui->pB_resume->setEnabled(1);
                ui->pB_pause->setEnabled(0);
                ui->pB_advance->setEnabled(1);
                ui->checkB_override->setChecked(0);
                ui->pB_backspace->setEnabled(1);
                remote_controller = 3;
                while (1)
                {
                    QCoreApplication::processEvents();
                    comm = sb.update(myid, comm[0]);
                    if (remote_controller == 0 && comm[0][0] == 3)
                    {
                        vsTemp = { "" };
                        sb.set_prompt(vsTemp);
                        comm[0][0] = 0;
                        sb.update(myid, comm[0]);
                    }
                    else if (remote_controller == 1 && comm[0][0] == 3)
                    {
                        vsTemp = { to_string(advBuffer) };
                        advBuffer = -1;
                        sb.set_prompt(vsTemp);
                        comm[0][0] = 0;
                        sb.update(myid, comm[0]);
                    }
                    else if (remote_controller == 4 && comm[0][0] == 3)
                    {
                        vsTemp = { to_string(-1 * backBuffer) };
                        backBuffer = -1;
                        sb.set_prompt(vsTemp);
                        comm[0][0] = 0;
                        sb.update(myid, comm[0]);
                    }
                    if (comm[1][0] == 0)
                    {
                        local_controller = 0;
                        if (remote_controller == 4) { remote_controller = 1; }
                        break;
                    }
                    Sleep(50);
                }
            }
            QCoreApplication::processEvents();
            break;
        }
        }
    }
    QCoreApplication::processEvents();
}
void MainWindow::convertGuide(SWITCHBOARD& sbgui, QPainterPath& painterPathBorder, vector<string>& pathBIN)
{
    vector<int> mycomm;
    vector<vector<int>> comm_gui;
    thread::id myid = this_thread::get_id();
    sbgui.answer_call(myid, mycomm);
    vector<string> prompt = sbgui.get_prompt();  // Form ["pauseVBP,width,height", pathFolder].
    vector<double> DxDyGa;
    QPainterPath pPB;
    vector<vector<double>> borderPathBIN, borderFrameBIN;
    bool success, letMeOut;
    int rank, inum;

    // Setup a planned stop point, and set the extracted image dimensions.
    vector<int> windowDim(2);
    size_t pos1 = prompt[0].find(',');
    string temp = prompt[0].substr(0, pos1);
    try { inum = stoi(temp); }
    catch (invalid_argument& ia) { jf.err("stoi-MainWindow.convertGuide"); }
    im.setPauseVBP(inum);
    pos1++;
    size_t pos2 = prompt[0].find(',', pos1);
    temp = prompt[0].substr(pos1, pos2 - pos1);
    try { windowDim[0] = stoi(temp); }
    catch (invalid_argument& ia) { jf.err("stoi-MainWindow.convertGuide"); }
    temp = prompt[0].substr(pos2 + 1);
    try { windowDim[1] = stoi(temp); }
    catch (invalid_argument& ia) { jf.err("stoi-MainWindow.convertGuide"); }
    im.setExtractDim(windowDim);

    // Make a list of PNG file paths to convert.
    makeTempASCII(prompt[1]); 
    string folderPathBIN = prompt[1];
    vector<string> dirt = { "mapsPNG" };
    vector<string> soap = { "mapsBIN" };
    jf.clean(folderPathBIN, dirt, soap);
    wf.makeDir(folderPathBIN);
    temp = "*.png";
    vector<string> pngNameList = wf.get_file_list(prompt[1], temp);
    mycomm[2] = pngNameList.size();
    sbgui.update(myid, mycomm);

    // Work through the task list, sending problematic spots to the user.
    string filepathPNG, filepathBIN;
    dirt = { ".png" };
    soap = { ".bin" };
    for (int ii = 0; ii < pngNameList.size(); ii++)
    {
        filepathPNG = prompt[1] + "\\" + pngNameList[ii];
        filepathBIN = folderPathBIN + "\\" + pngNameList[ii];
        jf.clean(filepathBIN, dirt, soap);
        if (!wf.file_exist(filepathBIN))
        {
            im.pngToBin(sbgui, filepathPNG, filepathBIN);
        }
        im.mapBinLoad(filepathBIN, borderFrameBIN, borderPathBIN);
        im.makeMapshift(windowDim, borderFrameBIN, DxDyGa);
        pPB = qf.pathMake(borderPathBIN, DxDyGa);
        success = 0;
        while (!success)
        {
            success = sbgui.push(myid);
            Sleep(7);
        }
        painterPathBorder = pPB;
        pathBIN.push_back(filepathBIN);
        success = sbgui.done(myid);
        mycomm[1]++;
        sbgui.update(myid, mycomm);
    }
    makeTempASCII(prompt[1]);  // Undo the damage...
    mycomm[0] = 1;
    sbgui.update(myid, mycomm);
}
void MainWindow::convertSingle(SWITCHBOARD& sbgui)
{
    vector<int> mycomm, windowDim(2);
    vector<vector<int>> comm_gui;
    thread::id myid = this_thread::get_id();
    sbgui.answer_call(myid, mycomm);
    vector<string> prompt = sbgui.get_prompt();  // Form ["width,height", "startX,startY", pngPath, binPath].
    size_t pos1 = prompt[0].find(',');
    string sWidth = prompt[0].substr(0, pos1);
    string sHeight = prompt[0].substr(pos1 + 1);
    try
    {
        windowDim[0] = stoi(sWidth);
        windowDim[1] = stoi(sHeight);
    }
    catch (invalid_argument) { jf.err("stoi-MainWindow.convertSingle"); }
    im.setExtractDim(windowDim);
    im.pngToBin(sbgui, prompt[2], prompt[3]);
    mycomm[0] = 1;
    sbgui.update(myid, mycomm);
}
void MainWindow::on_pB_resume_clicked()
{
    remote_controller = 0;
    ui->pB_pause->setEnabled(1);
    ui->pB_resume->setEnabled(0);
    ui->pB_backspace->setEnabled(0);
    ui->checkB_override->setChecked(0);
}
void MainWindow::on_pB_pause_clicked()
{
    remote_controller = 3;
    ui->pB_pause->setEnabled(0);
    ui->checkB_override->setChecked(0);
}
void MainWindow::on_pB_advance_clicked()
{
    QString qtemp = ui->pte_advance->toPlainText();
    string temp = qtemp.toStdString();
    vector<string> dirt = { " " };
    jf.clean(temp, dirt);
    int inum;
    if (temp == "")
    {
        advBuffer = 1;
        ui->pte_advance->setPlainText("1");
    }
    else
    {
        try 
        {
            inum = stoi(temp); 
            advBuffer = abs(inum);
        }
        catch (invalid_argument& ia)
        {
            ui->pte_advance->setPlainText("Error");
            return;
        }
    }
    remote_controller = 1;
    ui->pB_resume->setEnabled(0);
    ui->pB_advance->setEnabled(0);
    ui->pB_backspace->setEnabled(0);
    ui->checkB_override->setChecked(0);
}
void MainWindow::on_pB_backspace_clicked()
{
    QString qtemp = ui->pte_advance->toPlainText();
    string temp = qtemp.toStdString();
    vector<string> dirt = { " " };
    jf.clean(temp, dirt);
    int inum;
    if (temp == "")
    {
        backBuffer = 1;
        ui->pte_advance->setPlainText("1");
    }
    else
    {
        try
        {
            inum = stoi(temp);
            backBuffer = abs(inum) + 1;
        }
        catch (invalid_argument& ia)
        {
            ui->pte_advance->setPlainText("Error");
            return;
        }
    }
    ui->checkB_override->setChecked(0);
    remote_controller = 4;
}
void MainWindow::makeTempASCII(string folderPath)
{
    string hearthstonePath = folderPath + "\\tempASCII.txt";
    string hearthstoneFile, oldPath, newPath, temp;
    vector<string> folderNameList;
    vector<vector<string>> hearthstoneBits;
    size_t index, pos1, pos2;
    HANDLE hFile = INVALID_HANDLE_VALUE;
    if (wf.file_exist(hearthstonePath))
    {
        temp = jf.load(hearthstonePath.c_str());
        hearthstoneFile = jf.utf8ToAscii(temp);
        pos1 = hearthstoneFile.find_first_of("QWERTYUIOPASDFGHJKLZXCVBNM");
        pos2 = hearthstoneFile.find('$', pos1);
        while (pos2 < hearthstoneFile.size())
        {
            index = hearthstoneBits.size();
            hearthstoneBits.push_back(vector<string>(2));
            hearthstoneBits[index][0] = hearthstoneFile.substr(pos1, pos2 - pos1);
            pos1 = pos2 + 1;
            pos2 = hearthstoneFile.find('\n', pos1);
            hearthstoneBits[index][1] = hearthstoneFile.substr(pos1, pos2 - pos1);
            pos1 = pos2 + 1;
            pos2 = hearthstoneFile.find('$', pos1);
        }
        for (int ii = 0; ii < hearthstoneBits.size(); ii++)
        {
            oldPath = folderPath + "\\" + hearthstoneBits[ii][1];
            newPath = folderPath + "\\" + hearthstoneBits[ii][0];
            wf.renameFile(oldPath, newPath);
        }
        wf.delete_file(hearthstonePath);
    }
    else  // Before the nonsense... 
    {
        string search = "*.png";
        folderNameList = wf.get_file_list(folderPath, search);
        for (int ii = 0; ii < folderNameList.size(); ii++)
        {
            for (int jj = 0; jj < folderNameList[ii].size(); jj++)
            {
                if (folderNameList[ii][jj] < 0)
                {
                    index = hearthstoneBits.size();
                    hearthstoneBits.push_back(vector<string>(2));
                    hearthstoneBits[index][0] = folderNameList[ii];
                    for (int kk = folderNameList[ii].size() - 1; kk >= 0; kk--)
                    {
                        if (folderNameList[ii][kk] < 0)
                        {
                            folderNameList[ii].erase(folderNameList[ii].begin() + kk);
                        }
                    }
                    hearthstoneBits[index][1] = folderNameList[ii];
                    break;
                }
            }
        }
        for (int ii = 0; ii < hearthstoneBits.size(); ii++)
        {
            hearthstoneFile += hearthstoneBits[ii][0] + "$" + hearthstoneBits[ii][1] + "\n";
        }
        wstring wTemp = jf.asciiToUTF16(hearthstoneFile);
        jf.printer(hearthstonePath, wTemp);  // ... save the real names before enforced ASCII.
        for (int ii = 0; ii < hearthstoneBits.size(); ii++)
        {
            oldPath = folderPath + "\\" + hearthstoneBits[ii][0];
            newPath = folderPath + "\\" + hearthstoneBits[ii][1];
            wf.renameFile(oldPath, newPath);
        }
    }
    int bbq = 1;
}


void MainWindow::on_pB_undo_clicked()
{
    qf.undoEraser(ui->label_maps);
    countEraser--;
    if (countEraser < 1)
    {
        ui->pB_undo->setEnabled(0);
        ui->pB_savemap->setEnabled(0);
    }
    ui->tabW_online->setCurrentIndex(2);
}
void MainWindow::on_pB_savemap_clicked()
{
    QString qtemp;
    string mapBinPath, temp;
    vector<string> dirt = { "mapsPNG" };
    vector<string> soap = { "mapsBIN" };
    QList<QListWidgetItem*> qlist = ui->listW_bindone->selectedItems();
    if (qlist.size() == 1)
    {
        qtemp = qlist[0]->text();
        temp = qtemp.toUtf8();
        mapBinPath = selectedMapFolder + "\\" + jf.utf8ToAscii(temp) + ".bin";
        jf.clean(mapBinPath, dirt, soap);
        qf.printEditedMap(mapBinPath);
    }
}
void MainWindow::on_pB_deletemap_clicked()
{
    QString qtemp;
    string mapBinPath, temp;
    vector<string> dirt = { "mapsPNG" };
    vector<string> soap = { "mapsBIN" };
    QList<QListWidgetItem*> qlist = ui->listW_bindone->selectedItems();
    if (qlist.size() == 1)
    {
        qtemp = qlist[0]->text();
        temp = qtemp.toUtf8();
        mapBinPath = selectedMapFolder + "\\" + jf.utf8ToAscii(temp) + ".bin";
        jf.clean(mapBinPath, dirt, soap);
        wf.delete_file(mapBinPath);
        on_pB_correct_clicked();
    }
}

// Draw a BIN map over its parent region, and adjust position/rotation.
void MainWindow::on_pB_pos_clicked()
{
    QList<QListWidgetItem*> qlist = ui->listW_bindone->selectedItems();
    if (qlist.size() != 1) { return; }
    QString qtemp = qlist[0]->text();
    string binName8 = qtemp.toStdString();  // UTF8
    string binName = jf.utf8ToAscii(binName8), geoLayersFile, temp;
    if (selectedMapFolder.size() < 1) { err("selectedMapFolder-MainWindow.on_pB_pos_clicked"); }
    string binPath = selectedMapFolder + "\\" + binName + ".bin";
    vector<string> dirt = { "mapsPNG", "mapsPDF" };
    vector<string> soap = { "mapsBIN", "mapsBIN" };
    jf.clean(binPath, dirt, soap);
    string binFile = wf.load(binPath), geoLayersPath, sNum, sParent;
    int inum, numLayers = 0;
    vector<double> binPos;
    vector<vector<unsigned char>> rgbTarget(2, vector<unsigned char>());
    string pngParentPath, pngGrandparentPath, myLayer, newBin, pngGPathASCII, pngPPathASCII;
    
    // Obtain the parent region's name.
    size_t pos1 = binPath.rfind('\\'), pos2, posGeo1, posGeo2;
    geoLayersPath = binPath.substr(0, pos1) + "\\geo layers.txt";
    if (wf.file_exist(geoLayersPath))
    {
        geoLayersFile = jf.load(geoLayersPath);
        pos2 = geoLayersFile.rfind("@@Geo URL");
        pos2 = geoLayersFile.find_last_not_of('\n', pos2 - 1) + 1;
        pos1 = geoLayersFile.rfind("@@Geo Layers");
        pos1 = geoLayersFile.find('\n', pos1) + 1;
        while (pos1 < pos2)
        {
            numLayers++;
            pos1 = geoLayersFile.find('\n', pos1) + 1;
        }

        temp = "$" + binName8 + "$";
        pos1 = geoLayersFile.find(temp) + 1;
        if (pos1 > geoLayersFile.size()) { err("Cannot locate region name-MainWindow.on_pB_pos_clicked"); }
        pos1 = geoLayersFile.find('$', pos1) + 1;
        pos2 = geoLayersFile.find('\n', pos1);
        temp = geoLayersFile.substr(pos1, pos2 - pos1);
        try { inum = stoi(temp); }
        catch (invalid_argument& ia) { err("stoi-MainWindow.on_pB_pos_clicked"); }
        if (inum < 0 || inum >= numLayers) { err("No geo parent-MainWindow.on_pB_pos_clicked"); }

        posGeo1 = geoLayersFile.rfind("@@Geo Layers");
        posGeo1 = geoLayersFile.find('\n', posGeo1) + 1;
        for (int ii = 0; ii < inum; ii++)
        {
            posGeo1 = geoLayersFile.find('\n', posGeo1) + 1;
        }
        posGeo2 = geoLayersFile.find('\n', posGeo1);
        if (posGeo1 == posGeo2)
        {
            myLayer = "canada";
        }
        else
        {
            myLayer = geoLayersFile.substr(posGeo1, posGeo2 - posGeo1);
        }

        inum--;
        sNum = to_string(inum);
        while (pos2 < geoLayersFile.size())
        {
            pos2 = geoLayersFile.rfind('\n', pos2 - 1);
            pos1 = geoLayersFile.rfind('$', pos2) + 1;
            temp = geoLayersFile.substr(pos1, pos2 - pos1);
            if (temp == sNum)
            {
                pos2 = pos1 - 1;
                pos1 = geoLayersFile.rfind('$', pos2 - 1) + 1;
                sParent = geoLayersFile.substr(pos1, pos2 - pos1);
                break;
            }
        }
    }
    else { err("Missing geo layers.txt-MainWindow.on_pB_pos_clicked"); }

    // Determine this region's position within the parent region.
    dirt = { "mapsBIN", ".bin" };
    soap = { "pos", ".png" };
    pngParentPath = binPath;                   // pngParent refers to the small 
    jf.clean(pngParentPath, dirt, soap);       // "minimap" png from which we 
    if (!wf.file_exist(pngParentPath))         // derive position for the bin map.
    {
        pos1 = pngParentPath.rfind('\\');
        temp = pngParentPath.substr(0, pos1);
        wf.makeDir(temp);
        dirt = { "\\pos\\" };
        soap = { "\\mapsPNG\\" };
        pngGrandparentPath = pngParentPath;
        jf.clean(pngGrandparentPath, dirt, soap);
        pngGPathASCII = jf.asciiOnly(pngGrandparentPath);
        if (!wf.file_exist(pngGPathASCII)) { err("Missing PNG map-MainWindow.on_pB_pos_clicked"); }
        pngPPathASCII = jf.asciiOnly(pngParentPath);
        im.makePositionPNG(pngGPathASCII, pngPPathASCII);
    }
    rgbTarget[0] = { 0, 112, 255 };  // Blue dot.
    rgbTarget[1] = { 255, 255, 255 };  // White parent region.
    binPos = im.getPosition(pngParentPath, rgbTarget);
    
    // Insert the position values into the BIN map.
    pos1 = binFile.find("//position");
    if (pos1 > binFile.size())  // BIN map has no position data to override.
    {
        // Print the updated BIN map.
        pos1 = binFile.find("//scale");
        if (pos1 > binFile.size()) { err("//scale not found-MainWindow.on_pB_pos_clicked"); }
        pos1 = binFile.find('\n', pos1 + 1);
        pos1 = binFile.find('\n', pos1 + 1);
        newBin = binFile.substr(0, pos1);
        newBin += "\n\n//position(" + sParent + ")\n";
        newBin += to_string(binPos[0]) + "," + to_string(binPos[1]) + "\n\n";
        pos1 = binFile.find("//border");
        newBin += binFile.substr(pos1);
        jf.printer(binPath, newBin);
    }
    else  // BIN map has previous position data, which will be overridden.
    {
        newBin = binFile.substr(0, pos1);
        newBin += "//position(" + sParent + ")\n";
        newBin += to_string(binPos[0]) + "," + to_string(binPos[1]) + "\n\n";
        pos1 = binFile.find("//border");
        newBin += binFile.substr(pos1);
        dirt = { "\r\n" };
        soap = { "\n" };
        jf.clean(newBin, dirt, soap);
        jf.printer(binPath, newBin);
    }
}

// Load BIN maps from local storage and insert them into the SQL database.
void MainWindow::on_pB_insertmap_clicked()
{
    QList<QTreeWidgetItem*> qSelected = ui->treeW_maps->selectedItems();
    if (qSelected.size() != 1) { return; }
    selectedMapFolder = qf.makePathTree(qSelected[0]);
    QString qtemp;
    string search = "*.bin";
    vector<string> listBin = wf.get_file_list(selectedMapFolder, search);
    if (listBin.size() < 1)
    {
        qtemp = "There are 0 BIN maps in that folder.";
        ui->pte_webinput->setPlainText(qtemp);
        return;
    }
    listBin.push_back(selectedMapFolder);

    thread::id myid = this_thread::get_id();
    vector<vector<int>> comm(1, vector<int>());
    comm[0].assign(comm_length, 0);
    comm[0][2] = listBin.size() - 1;
    sb.start_call(myid, 1, comm[0]);
    sb.set_prompt(listBin);
    std::thread thr(&MainWindow::insertMapWorker, this, ref(sb), ref(sf));
    thr.detach();
    string status = "Inserting BIN maps for " + selectedMapFolder + " ...";
    reset_bar(comm[0][2], status);
    while (comm[0][0] == 0)
    {
        Sleep(gui_sleep);
        comm = sb.update(myid, comm[0]);
        if (comm.size() < 2) { continue; }
        if (comm[1][1] > comm[0][1])
        {
            comm[0][1] = comm[1][1];
            jobs_done = comm[0][1];
            update_bar();
        }
        if (comm[1][0] == 1) { comm[0][0] = 1; }
        QCoreApplication::processEvents();
    }
    update_treeW_mapindb();
    status += " done!";
    barMessage(status);
}
void MainWindow::insertMapWorker(SWITCHBOARD& sbgui, SQLFUNC& sf)
{
    vector<int> mycomm;
    vector<vector<int>> comm_gui;
    thread::id myid = this_thread::get_id();
    sbgui.answer_call(myid, mycomm);
    vector<string> nameList = sbgui.get_prompt();
    vector<vector<vector<int>>> frames;
    vector<vector<int>> border;
    vector<double> position;
    double scale;
    string sParent8, binPath;

    string folderPath = nameList[nameList.size() - 1];
    nameList.pop_back();
    for (int ii = 0; ii < nameList.size(); ii++)
    {
        binPath = folderPath + "\\" + nameList[ii]; 
        qf.loadBinMap(binPath, frames, scale, position, sParent8, border);
        sf.insertBinMap(binPath, frames, scale, position, sParent8, border);
        mycomm[1]++;
        sbgui.update(myid, mycomm);
    }
    mycomm[0] = 1;
    sbgui.update(myid, mycomm);
}

// Update button status (enabled/disabled).
void MainWindow::on_treeW_cataindb_itemSelectionChanged()
{
    QList<QTreeWidgetItem*> db_selected = ui->treeW_cataindb->selectedItems();
    if (db_selected.size() > 0)
    {
        QString qname = db_selected[0]->text(1);
        if (qname == " ")
        {
            ui->pB_viewcata->setEnabled(0);
            ui->pB_removecata->setEnabled(0);
        }
        else
        {
            ui->pB_viewcata->setEnabled(1);
            ui->pB_removecata->setEnabled(1);
        }
    }
    else
    {
        ui->pB_viewcata->setEnabled(0);
        ui->pB_removecata->setEnabled(0);
    }

}
void MainWindow::on_treeW_cataondrive_itemSelectionChanged()
{
    QList<QTreeWidgetItem*> local_selected = ui->treeW_cataondrive->selectedItems();
    if (local_selected.size() > 0)
    {
        ui->pB_insert->setEnabled(1);
    }
    else
    {
        ui->pB_insert->setEnabled(0);
    }
}
void MainWindow::on_treeW_gid_itemSelectionChanged()
{
    QList<QTreeWidgetItem*> region_selected = ui->treeW_gid->selectedItems();
    if (region_selected.size() > 0)
    {
        ui->pB_viewtable->setEnabled(1);
        ui->pB_deletetable->setEnabled(1);
    }
    else
    {
        ui->pB_viewtable->setEnabled(0);
        ui->pB_deletetable->setEnabled(0);
    }
}
void MainWindow::on_listW_search_itemSelectionChanged()
{
    QList<QListWidgetItem*> qSelected = ui->listW_search->selectedItems();
    if (qSelected.size() != 1) { return; }
    ui->pB_viewtable->setEnabled(1);
    ui->pB_deletetable->setEnabled(1);
}
void MainWindow::on_tabW_catalogues_currentChanged(int index)
{
    QList<QTreeWidgetItem*> qtree;
    switch (index)
    {
    case 0:
        ui->pB_insert->setEnabled(0);
        qtree = ui->treeW_cataondrive->selectedItems();
        for (int ii = 0; ii < qtree.size(); ii++)
        {
            qtree[ii]->setSelected(0);
        }
        qtree = ui->treeW_cataindb->selectedItems();
        if (qtree.size() > 0)
        {
            ui->pB_viewcata->setEnabled(1);
            ui->pB_removecata->setEnabled(1);
        }
        break;

    case 1:
        ui->pB_viewcata->setEnabled(0);
        ui->pB_removecata->setEnabled(0);
        break;
    }
}
void MainWindow::on_tabW_online_currentChanged(int index)
{
    switch (index)
    {
    case 0:
        ui->pB_download->setEnabled(0);
        //ui->pte_advance->setVisible(0);
        //ui->checkB_override->setVisible(0);
        break;
    case 1:
        ui->pB_download->setEnabled(1);
        //ui->pte_advance->setVisible(0);
        //ui->checkB_override->setVisible(0);
        break;
    case 2:
        ui->pB_download->setEnabled(0);
        //ui->pte_advance->setVisible(1);
        //ui->checkB_override->setVisible(1);
        break;
    }
}
void MainWindow::on_tabW_results_currentChanged(int index)
{
    //QList<QTreeWidgetItem*> qtree;
    switch (index)
    {
    case 0:
        ui->pB_viewtable->setEnabled(1);
        break;

    case 1:
        ui->pB_viewtable->setEnabled(0);
        break;

    case 2:
        ui->pB_viewtable->setEnabled(0);
        break;
    }
}
void MainWindow::on_treeW_statscan_itemSelectionChanged()
{
    QList<QTreeWidgetItem*> cataSelected = ui->treeW_statscan->selectedItems();
    QTreeWidgetItem *pParent, *pNode;
    int inum = -1;
    if (cataSelected.size() < 1) { return; }
    pParent = cataSelected[0];
    do
    {
        pNode = pParent;
        pParent = pNode->parent();
        inum++;
    } while (pParent != nullptr);
    if (inum > 0)
    {
        ui->pB_download->setEnabled(1);
    }
    else
    {
        ui->pB_download->setEnabled(0);
    }
    downloadWindow = 0;
}
void MainWindow::on_treeW_maps_itemSelectionChanged() 
{

}
void MainWindow::on_listW_bindone_itemSelectionChanged()
{
    int index;
    QString qtemp;
    string mapBinPath, temp;
    vector<string> dirt = { "mapsPNG" };
    vector<string> soap = { "mapsBIN" };
    QList<QListWidgetItem*> qlist = ui->listW_bindone->selectedItems();
    if (qlist.size() == 1)
    {
        index = ui->tabW_online->currentIndex();
        switch (index)
        {
        case 2:
        {
            qtemp = qlist[0]->text();
            temp = qtemp.toUtf8();
            mapBinPath = selectedMapFolder + "\\" + jf.utf8ToAscii(temp) + ".bin";
            jf.clean(mapBinPath, dirt, soap);
            qf.displayBin(ui->label_maps, mapBinPath);
            break;
        }
        case 3:
        {

            break;
        }
        }

    }
    countEraser = 0;
    ui->tabW_online->setCurrentIndex(2);
    ui->pB_undo->setEnabled(0);
    ui->pB_savemap->setEnabled(0);
    ui->pB_deletemap->setEnabled(1);
}
void MainWindow::on_listW_statscan_itemSelectionChanged()
{
    downloadWindow = 1;
}
void MainWindow::on_checkB_eraser_stateChanged(int iState)
{
    if (iState == 0)
    {
        QCursor qCursor = QCursor(Qt::ArrowCursor);
        ui->label_maps->setCursor(qCursor);
    }
    else if (iState == 2)
    {
        QPixmap qPM(widthEraser, widthEraser);
        qPM.fill(Qt::black);
        QBitmap qBM = QBitmap::fromPixmap(qPM);
        QBitmap qMask = QBitmap(widthEraser, widthEraser);
        qMask.clear();
        QCursor qCursor = QCursor(qBM, qMask, 0, 0);
        ui->label_maps->setCursor(qCursor);
    }
}

// DEBUG FUNCTIONS:
int MainWindow::sql_callback(void* NotUsed, int argc, char** argv, char** column_name)
{
    for (int ii = 0; ii < argc; ii++)
    {
        qDebug() << "Column name: " << column_name[ii];
        qDebug() << "argv: " << argv[ii];
    }
    return 0;
}

*/
