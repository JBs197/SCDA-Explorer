#include "mainwindow.h"
#include "ui_mainwindow.h"

using namespace std;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->progressBar->setValue(0);
    ui->QL_bar->setText("");

    initialize();
}

MainWindow::~MainWindow()
{
    delete ui;
}

// MULTI-PURPOSE FUNCTIONS:

// Perform a variety of tasks from the MainWindow constructor.
void MainWindow::initialize()
{
    // Scan the local machine for drive letters, then populate the 'drives' drop-down menu.
    LPWSTR bufferW = new WCHAR[500];
    DWORD dsize = GetLogicalDriveStringsW(500, bufferW);
    wstring wdrives(bufferW, dsize);
    delete[] bufferW;
    wstring wtemp;
    QList<QString> qdrives;
    QString qtemp;
    size_t pos1 = wdrives.find(L"\\");
    if (pos1 >= wdrives.size()) { err(L"wdrives-initialize: " + wdrives); }
    do
    {
        wtemp = wdrives.substr(pos1 - 2, 2);
        qtemp = QString::fromStdWString(wtemp);
        qdrives.append(qtemp);
        pos1 = wdrives.find(L"\\", pos1 + 1);
    } while (pos1 < wdrives.size());
    int numDrives = qdrives.size();
    for (int ii = 0; ii < numDrives; ii++)
    {
        ui->cB_drives->addItem(qdrives[ii]);
    }
    ui->cB_drives->setCurrentIndex(numDrives - 1);
    viewcata_data.assign(2, "");

    // Open the database from an existing local db file, or (failing that) make a new one.
    string temp = wf.get_exec_dir();
    pos1 = temp.rfind('\\');
    pos1 = temp.rfind('\\', pos1 - 1);
    string dbPath = temp.substr(0, pos1);
    dbPath += "\\SCDA-Wt\\SCDA.db";
    sf.init(dbPath);
    //q.exec(QStringLiteral("PRAGMA journal_mode=WAL"));

    // Populate the navSearch matrix.
    navSearch = sc.navAsset();

    // Initialize the switchboard's error path.
    sb.setErrorPath(sroot + "\\SCDA SWITCHBOARD Error Log.txt");

    // Initialize JTREE objects.
    jtStatsCan.init("Statistics Canada Census Data", sroot);

    // Create (if necessary) these system-wide tables. 
    if (!sf.table_exist("TCatalogueIndex")) { create_cata_index_table(); }
    if (!sf.table_exist("TDamaged")) { create_damaged_table(); }
    if (!sf.table_exist("TMapIndex")) { createMapIndexTable(); }    
    
    // Load a font into the IMGFUNC object.
    initImgFont("Sylfaen");

    // Load hidden initial values into the GUI widgets.
    qf.set_display_root(ui->treeW_gid, 1);
    qf.set_display_root(ui->treeW_csvtree, 1);
    qf.set_display_root(ui->treeW_subtables, 0);
    update_treeW_cataindb();
    update_treeW_mapindb();
    
    // Initialize mode to 'local'. 
    qtemp = QString::fromStdString(modes[active_mode]);
    ui->label_mode->setText(qtemp);   
    ui->tabW_catalogues->setCurrentIndex(0);
    //ui->tabW_catalogues->setGeometry(10, 10, 351, 681);
    ui->pB_removecata->setGeometry(385, 690, 71, 41);
    ui->pte_localinput->setVisible(1);
    ui->pte_localinput->setGeometry(460, 690, 241, 41);
    ui->pB_search->setGeometry(710, 690, 71, 41);
    ui->pB_viewtable->setGeometry(790, 690, 71, 41);
    ui->pB_deletetable->setGeometry(870, 690, 71, 41);
    ui->pB_deletetable->setEnabled(0);
    ui->tabW_results2->setGeometry(760, 10, 496, 641);
    ui->tV_viewtable->setGeometry(0, 0, 486, 611);
    ui->treeW_subtables->setGeometry(0, 0, 486, 611);
    ui->tabW_online->setVisible(0);
    ui->tabW_online->setGeometry(10, 10, 1241, 641);
    ui->treeW_statscan->setGeometry(0, 0, 521, 611);
    ui->treeW_statscan->setVisible(0);
    ui->listW_statscan->setGeometry(526, 0, 386, 611);
    ui->listW_statscan->setVisible(0);
    ui->treeW_maps->setGeometry(0, 0, 911, 611);
    ui->treeW_maps->setVisible(0);
    ui->label_maps->setGeometry(0, 0, 910, 611);
    ui->label_maps->setVisible(0);
    ui->label_pos->setGeometry(0, 0, 910, 611);
    ui->label_pos->setVisible(0);
    ui->label_maps2->setGeometry(1035, 35, 100, 21);
    ui->label_maps2->setVisible(0);
    ui->listW_bindone->setGeometry(1035, 55, 190, 280);
    ui->listW_bindone->setVisible(0);
    ui->checkB_override->setGeometry(1055, 435, 80, 40);
    ui->checkB_override->setVisible(0);
    ui->checkB_eraser->setGeometry(1055, 485, 80, 40);
    ui->checkB_eraser->setVisible(0);
    ui->pB_resume->setGeometry(1140, 345, 81, 41);
    ui->pB_resume->setVisible(0);
    ui->pB_pause->setGeometry(1050, 345, 81, 41);
    ui->pB_pause->setVisible(0);
    ui->pB_advance->setGeometry(1050, 391, 81, 41);
    ui->pB_advance->setVisible(0);
    ui->pB_backspace->setGeometry(1140, 437, 80, 40);
    ui->pB_backspace->setVisible(0);
    ui->pB_backspace->setEnabled(0);
    ui->pte_advance->setGeometry(1140, 391, 78, 41);
    ui->pte_advance->setPlainText("1");
    ui->pte_advance->setVisible(0);
    ui->pB_undo->setGeometry(1140, 482, 80, 40);
    ui->pB_undo->setVisible(0);
    ui->pB_undo->setEnabled(0);
    ui->pB_savemap->setGeometry(1140, 527, 80, 40);
    ui->pB_savemap->setVisible(0);
    ui->pB_savemap->setEnabled(0);
    ui->pB_deletemap->setGeometry(1050, 527, 80, 40);
    ui->pB_deletemap->setVisible(0);
    ui->pB_deletemap->setEnabled(0);
    ui->pB_test->setGeometry(260, 690, 61, 41);
    ui->pB_test->setVisible(0);
    ui->pte_webinput->setVisible(0);
    ui->pte_webinput->setGeometry(10, 690, 241, 41);
    ui->pB_usc->setGeometry(330, 690, 61, 41);
    ui->pB_usc->setVisible(0);
    ui->pB_download->setGeometry(400, 690, 61, 41);
    ui->pB_download->setVisible(0);
    ui->pB_download->setEnabled(0);
    ui->pB_localmaps->setVisible(0);
    ui->pB_localmaps->setGeometry(470, 690, 81, 41);
    ui->pB_convert->setVisible(0);
    ui->pB_convert->setGeometry(560, 690, 61, 41);
    ui->pB_correct->setVisible(0);
    ui->pB_correct->setGeometry(630, 690, 61, 41);
    ui->pB_pos->setVisible(0);
    ui->pB_pos->setGeometry(700, 690, 81, 41);
    ui->pB_insertmap->setVisible(0);
    ui->pB_insertmap->setGeometry(790, 690, 61, 41);
    ui->pB_mode->setGeometry(1120, 690, 61, 41);
    ui->progressBar->setGeometry(10, 660, 1241, 23);
    ui->QL_bar->setGeometry(10, 660, 1241, 23);

    // Prepare mouse event offsets.
    QRect posLabelMaps = ui->tabW_online->geometry();
    QPoint TL = posLabelMaps.topLeft();
    labelMapsDx = (-1 * TL.x()) - 2;
    labelMapsDy = (-1 * TL.y()) - 25;

    // Initialize the progress bar with blanks.
    reset_bar(100, " ");

    // Reset the log file.
    wf.delete_file(sroot + "\\SCDA Process Log.txt");
    log("MainWindow initialized.");

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

// Progress bar related functions.
void MainWindow::update_bar()
{
    int jump, old;
    QString pb_text;
    if (jobs_max >= 0 && jobs_done >= 0)
    {
        jobs_percent = 100 * jobs_done / jobs_max;
        old = ui->progressBar->value();
        jump = jobs_percent - old;
        if (jump >= 15)
        {
            warn(L"Progress bar single increase exceeded 15%");
        }
        ui->progressBar->setValue(jobs_percent);
    }
    else
    {
        warn(L"Tried to update the progress bar before resetting it.");
    }
}
void MainWindow::reset_bar(int max, string status)
{
    QString qstatus = QString::fromStdString(status);
    lock_guard<mutex> lock(m_bar);
    jobs_done = 0;
    jobs_max = max;
    ui->progressBar->setValue(0);
    ui->QL_bar->setText(qstatus);
}
void MainWindow::barMessage(string message)
{
    QString qmessage = QString::fromStdString(message);
    lock_guard<mutex> lock(m_bar);
    ui->QL_bar->setText(qmessage);
}

// Given a catalogue folder path, return a list of GIDs present in that folder.
vector<string> MainWindow::extract_gids(string cata_path)
{
    vector<string> gids;
    string folder_search = cata_path + "\\*.csv";
    WIN32_FIND_DATAA info;
    HANDLE hfile = FindFirstFileA(folder_search.c_str(), &info);
    if (hfile == INVALID_HANDLE_VALUE) { winerr("FindFirstFile-extract_gids"); }
    string temp1, temp2;
    size_t pos1, pos2;

    do
    {
        temp1 = info.cFileName;
        pos1 = temp1.find('(');
        pos2 = temp1.find(')', pos1);
        temp2 = temp1.substr(pos1 + 1, pos2 - pos1 - 1);
        gids.push_back(temp2);
    } while (FindNextFileA(hfile, &info));

    FindClose(hfile);
    return gids;
}

// Replace '?' placeholders in a template statement with given parameter strings.
void MainWindow::bind(string& stmt, vector<string>& param)
{
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
        err(L"parameter count mismatch-bind");
    }

    pos1 = 0;
    for (int ii = 0; ii < (int)count; ii++)
    {
        pos1 = stmt.find('?', pos1 + 1);
        temp = "'" + param[ii] + "'";
        stmt.replace(pos1, 1, temp);
    }
}

// Functions for the meta-tables.
void MainWindow::create_cata_index_table()
{
    string stmt = "CREATE TABLE TCatalogueIndex (Year TEXT, ";
    stmt += "Name TEXT, Description TEXT);";
    sf.executor(stmt);
}
void MainWindow::create_damaged_table()
{
    int bbq = 0;
    string stmt = "CREATE TABLE TDamaged ([Catalogue Name] TEXT, ";
    stmt += "GID INT, [Number of Errors] INT);";
    sf.executor(stmt);
}
void MainWindow::createMapIndexTable()
{
    // Create the table, if necessary.
    string stmt = "CREATE TABLE TMapIndex ";
    stmt += "(coreDir TEXT, numParams INTEGER, param1 TEXT, ";
    stmt += "param2 TEXT, param3 TEXT, param4 TEXT, ";
    stmt += "UNIQUE(coreDir, param1, param2, param3, param4) );";
    sf.executor(stmt);

    // Initialize the table with the subfolders in "mapsBIN", if necessary.
    vector<string> dirt = { "\\" };
    vector<string> soap = { "$" };
    vector<string> treePL = { sroot + "\\mapsBIN" };
    vector<vector<int>> treeST = { {0} };
    wf.getTreeFolder(0, treeST, treePL);
    string coreDir, temp;
    size_t pos1;
    for (int ii = 1; ii < treePL.size(); ii++)
    {
        pos1 = treePL[ii].find("mapsBIN");
        pos1 = treePL[ii].find('\\', pos1 + 6) + 1;
        coreDir = treePL[ii].substr(pos1);
        jf.clean(coreDir, dirt, soap);
        sf.insertTMI(coreDir);
    }
}

// If the tree widget has 'max' or fewer elements, then expand those elements in the GUI.
void MainWindow::auto_expand(QTreeWidget*& qtree, int max)
{
    int count = qtree->topLevelItemCount();
    QTreeWidgetItem* qitem;
    if (count <= max)
    {
        for (int ii = 0; ii < count; ii++)
        {
            qitem = qtree->topLevelItem(ii);
            qtree->expandItem(qitem);
        }
    }
}

// Functions for missing files.
vector<string> MainWindow::notDownloaded(string syear, string sname)
{
    // Searches online for the list of CSVs included in the given catalogue.
    // Then, it reads the local drive and returns a list of missing CSV files.
    // Returned list entries are of the form   (gid) Region Name
    vector<string> listCSV;
    return listCSV;
}

// Mouse functions.
void MainWindow::mousePressEvent(QMouseEvent* event)
{
    QPoint pointClick;
    vector<int> clickCoord(2);
    vector<string> promptUpdate;
    double dist;
    int inum;
    if (event->button() == Qt::LeftButton)
    {
        pointClick = event->position().toPoint();
        clickCoord[0] = pointClick.x();
        clickCoord[1] = pointClick.y();
    }
    else if (event->button() == Qt::MiddleButton)
    {
        ui->checkB_override->setChecked(1);
        pointClick = event->position().toPoint();
        clickCoord[0] = pointClick.x();
        clickCoord[1] = pointClick.y();
    }
    if (active_mode == 1)  // Online mode.
    {
        if (ui->tabW_online->currentIndex() == 2)
        {
            if (remote_controller == 3 && debugMapCoord.size() > 0)
            {
                inum = qf.getLastMap();
                if (inum == 0)
                {
                    clickCoord[0] += labelMapsDx + debugMapCoord[0][0];
                    clickCoord[1] += labelMapsDy + debugMapCoord[0][1];
                    if (ui->checkB_override->isChecked())
                    {
                        qf.debugMapSelected(ui->label_maps, clickCoord, debugMapCoord);
                        promptUpdate = { "", "", jf.stringifyCoord(clickCoord) };
                        sb.set_prompt(promptUpdate);
                        advBuffer = 1;
                        remote_controller = 1;
                        ui->pB_pause->setEnabled(1);
                        ui->pB_resume->setEnabled(0);
                        ui->pB_backspace->setEnabled(0);
                        ui->checkB_override->setChecked(0);
                    }
                    else
                    {
                        for (int ii = 1; ii < debugMapCoord.size(); ii++)
                        {
                            dist = mf.coordDist(clickCoord, debugMapCoord[ii]);
                            if (dist < 5.0)
                            {
                                qf.debugMapSelected(ui->label_maps, ii, debugMapCoord);
                                promptUpdate = { "", jf.stringifyCoord(debugMapCoord[ii]) };
                                sb.set_prompt(promptUpdate);
                                remote_controller = 0;
                                ui->pB_pause->setEnabled(1);
                                ui->pB_resume->setEnabled(0);
                                ui->pB_backspace->setEnabled(0);
                                break;
                            }
                        }
                    }
                }
                else if (inum == 1)
                {
                    clickCoord[0] += labelMapsDx;
                    clickCoord[1] += labelMapsDy;
                    promptUpdate = { jf.stringifyCoord(clickCoord) };
                    qDebug() << "Click: " << QString::fromStdString(promptUpdate[0]);
                }
            }
            else if (ui->checkB_eraser->isChecked())
            {
                clickCoord[0] += labelMapsDx;
                clickCoord[1] += labelMapsDy;
                vector<vector<int>> TLBR(2, vector<int>(2));
                TLBR[0] = clickCoord;
                TLBR[1][0] = TLBR[0][0] + widthEraser;
                TLBR[1][1] = TLBR[0][1] + widthEraser;
                qf.eraser(ui->label_maps, TLBR);
                countEraser++;
                ui->pB_undo->setEnabled(1);
                ui->pB_savemap->setEnabled(1);
            }
        }
    }
}


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
void MainWindow::scan_drive(SWITCHBOARD& sb, WINFUNC& wf, QList<QTreeWidgetItem*>& qsroots)
{
    vector<int> mycomm;
    thread::id myid = this_thread::get_id();
    sb.answer_call(myid, mycomm);
    vector<string> prompt = sb.get_prompt();

    // Get a list of catalogue years and names.
    vector<string> folder_list = wf.get_folder_list(prompt[0], "*");
    int inum;
    for (int ii = 0; ii < folder_list.size(); ii++)
    {
        try
        {
            inum = stoi(folder_list[ii]);
        }
        catch (invalid_argument& ia)
        {
            folder_list.erase(folder_list.begin() + ii);
            ii--;
        }
    }
    vector<string> cata_name;
    string temp;
    vector<vector<string>> cata_list;  // Form [catalogue index][syear, sname].
    for (int ii = 0; ii < folder_list.size(); ii++)
    {
        temp = sdrive + "\\" + folder_list[ii];
        cata_name = wf.get_folder_list(temp, "*");
        for (int jj = 0; jj < cata_name.size(); jj++)
        {
            temp = sdrive + "\\" + folder_list[ii] + "\\" + cata_name[jj] + "\\";
            temp += cata_name[jj] + " geo list.bin";
            if (wf.file_exist(temp))
            {
                cata_list.push_back({ folder_list[ii], cata_name[jj] });
            }
        }
    }

    // Populate the tree widget with the catalogue list.
    qsroots.clear();
    string syear = "";
    QString qtemp;
    QTreeWidgetItem* qitem;
    for (int ii = 0; ii < cata_list.size(); ii++)
    {
        if (cata_list[ii][0] != syear)
        {
            inum = qsroots.size();
            syear = cata_list[ii][0];
            qtemp = QString::fromStdString(syear);
            qitem = new QTreeWidgetItem();
            qitem->setText(0, qtemp);
            auto item_flags = qitem->flags();
            item_flags.setFlag(Qt::ItemIsSelectable, false);
            qitem->setFlags(item_flags);
            qsroots.append(qitem);
        }
        qtemp = QString::fromStdString(cata_list[ii][1]);
        qitem = new QTreeWidgetItem(qsroots[inum]);
        qitem->setText(1, qtemp);
    }

    mycomm[0] = 1;
    sb.update(myid, mycomm);
}

// Insert the selected catalogues into the database.
void MainWindow::on_pB_insert_clicked()
{
    QList<QTreeWidgetItem*> catas_to_do = ui->treeW_cataondrive->selectedItems();
    QList<QTreeWidgetItem*> qSel;
    QTreeWidgetItem* qitem;
    vector<string> prompt(3);  // syear, sname, desc, mode.
    vector<string> results1, search, conditions, vtemp;
    vector<vector<string>> results;
    QString qyear, qname, qtemp;
    string syear, sname, stmt, sPath, search0 = "*.zip";
    int mode, error, sb_index;
    vector<vector<int>> comm;
    
    thread::id myid = this_thread::get_id();
    ui->tabW_catalogues->setCurrentIndex(0);
    ui->pB_cancel->setEnabled(1);

    // For each catalogue selected, launch worker threads and then process their statements.
    for (int ii = 0; ii < catas_to_do.size(); ii++)  
    {
        qitem = catas_to_do[ii]->parent();
        qyear = qitem->text(0);
        syear = qyear.toStdString();
        prompt[0] = syear;
        qname = catas_to_do[ii]->text(1);
        sname = qname.toStdString();
        prompt[1] = sname;
        sPath = sroot + "\\" + syear + "\\" + sname;
        results1 = wf.get_file_list(sPath, search0);
        if (results1.size() > 0) { continue; } // Cannot parse these yet - so ignore.
        comm.resize(1);
        comm[0].assign(comm_length, 0);  // Form [control, progress report, size report, max param].


        // Determine that catalogue's current status in the database.
        search = { "Description" };
        conditions = {
            "Year = '" + syear + "'",
            "AND Name = '" + sname + "'"
        };
        results1.clear();
        sf.select(search, "TCatalogueIndex", results1, conditions);
        if (results1.size() < 1)
        {
            mode = 0;  // Catalogue is not in database. Insert completely.
            vtemp.resize(3);
            vtemp[0] = prompt[0];
            vtemp[1] = prompt[1];
            vtemp[2] = "Incomplete";
            sf.insert("TCatalogueIndex", vtemp);
        }
        else if (results1[0] == "Incomplete")
        {                // Catalogue is partially present in the database.
            mode = 1;    // Determine which CSVs are missing, and insert them.
        }
        else
        {
            mode = 2;  // Catalogue is already present in the database. Inform the user.
        }

        // Launch the manager function to insert files (as needed).
        switch (mode)
        {
        case 0:
            log("Beginning insertion of catalogue " + sname);
            error = sb.start_call(myid, cores, comm[0]);
            if (error) { errnum("start_call1-pB_insert_clicked", error); }
            prompt[2] = to_string(mode);
            sb.set_prompt(prompt);
            std::thread judi(&MainWindow::judicator, this, ref(sf), ref(sb), ref(wf));
            judi.detach();
            break;
        }

        // While waiting for completion, keep the GUI active and communicate with the manager thread.
        while (1)
        {
            Sleep(gui_sleep);
            QCoreApplication::processEvents();
            if (remote_controller == 2)  // The 'cancel' button was pressed.
            {
                comm[0][0] = 2;  // Inform the manager thread it should abort its task.
                remote_controller = 0;
            }
            try
            {
                comm = sb.update(myid, comm[0]);
                if (comm[0][2] == 0)  // If the GUI thread does not yet know the size of the task...
                {
                    if (comm[1][2] > 0)  // ... and the manager thread does know, then ...
                    {
                        comm[0][2] = comm[1][2];
                        comm[0][1] = comm[1][1];
                        reset_bar(comm[0][2], "Inserting catalogue  " + sname);  // ... initialize the progress bar.
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
            }
            catch (out_of_range& oor)
            {
                err("sb.update-mainwindow");
            }
            
            if (comm[1][0] == 1)  // Task completed.
            {
                error = sb.end_call(myid);
                if (error) { errnum("sb.end_call-on_pB_insert", error); }
                jobs_done = comm[0][2];
                update_bar();
                break;           // Manager reports task finished/cancelled.
            }
            else if (comm[1][0] == -2)  // Error: too many columns.
            {
                error = sb.end_call(myid);
                if (error) { errnum("sb.end_call-on_pB_insert", error); }
                removeCataTemp.resize(3);
                removeCataTemp[0] = syear;
                removeCataTemp[1] = sname;
                removeCataTemp[2] = "Incomplete";
                on_pB_removecata_clicked();
                qtemp = qname + " was aborted: TOO MANY COLUMNS.";
                ui->pte_localinput->setPlainText(qtemp);
                break;
            }
        }
    }

    // Update the GUI and then retire to obscurity.
    ui->pB_cancel->setEnabled(0);
    Sleep(100);
    update_treeW_cataindb();
}
void MainWindow::judicator(SQLFUNC& sfgui, SWITCHBOARD& sb, WINFUNC& wf)
{
    vector<int> mycomm, gidList;
    vector<vector<int>> comm_gui;
    thread::id myid = this_thread::get_id();
    sb.answer_call(myid, mycomm);
    vector<string> prompt = sb.get_prompt(), regionList, layerList, geoLayers;
    string cata_path = sroot + "\\" + prompt[0] + "\\" + prompt[1];
    int num_gid = wf.get_file_path_number(cata_path, ".csv");
    mycomm[2] = num_gid;
    comm_gui = sb.update(myid, mycomm);
    int mode, error, inum;
    try { mode = stoi(prompt[2]); } 
    catch (invalid_argument& ia) { err("stoi1-judicator"); }
    JFUNC jfjudi;
    QElapsedTimer timer;

    // Create and initialize the Stats Can helper object, for the worker threads.
    vector<string> csv_list = wf.get_file_list(cata_path, "*.csv");
    string temp = cata_path + "\\" + csv_list[0];
    string sample_csv = jfjudi.load(temp);
    STATSCAN scjudi(cata_path);
    error = scjudi.cata_init(sample_csv);    
    if (error == -1)
    {
        mycomm[0] = -2;
        sb.update(myid, mycomm);
        return;
    }
    scjudi.extract_gid_list(csv_list);
    scjudi.extract_csv_branches(csv_list);
    
    // Create and insert this catalogue's indexing tables, if necessary.
    string geoPath = cata_path + "\\" + prompt[1] + " geo list.bin";
    size_t geoSize = sfgui.table_exist(prompt[1] + "$Geo");
    geoSize += sfgui.table_exist(prompt[1] + "$Geo_Layers");
    switch (geoSize)
    {
    case 0:
        scjudi.loadGeo(geoPath, gidList, regionList, layerList, geoLayers);
        sfgui.insertGeo(prompt[1], gidList, regionList, layerList, geoLayers);
        addTMap(sfgui, gidList, regionList, layerList, prompt[1]);
        break;
    case 1:
        err("Geo/Geo_Layers mismatch-MainWindow.judicator");
        break;
    case 2:
        scjudi.loadGeo(geoPath, gidList, regionList, layerList, geoLayers);
        addTMap(sfgui, gidList, regionList, layerList, prompt[1]);
        break;
    }
    addTGRow(sfgui, scjudi, prompt[1]);
    addTGRegion(sfgui, scjudi, prompt[0], prompt[1]);
    string stmt = scjudi.make_create_primary_table();
    sfgui.executor(stmt);

    // Launch the worker threads, which will iterate through the CSVs.
    SWITCHBOARD sbjudi;
    sbjudi.setErrorPath(sroot + "\\SCDA SBjudi Error Log.txt");
    int workload = num_gid / cores;
    int bot = 0, top = workload - 1;
    vector<vector<int>> comm_csv(1, vector<int>());  // Form [thread][control, progress report, size report, max params].
    comm_csv[0].assign(comm_gui[0].size(), 0);
    vector<string> prompt_csv(cores);  // Form [(bot,top)]
    vector<vector<vector<string>>> all_queue(cores, vector<vector<string>>());  // Form [thread][CSV][statements].  
    switch (mode)  // Mode 0 = full insertion, 1 = partial insertion.
    {
    case 0:
        for (int ii = 0; ii < cores; ii++)
        {
            prompt_csv[ii] = to_string(bot) + "," + to_string(top);
            bot += workload;
            if (ii < cores - 2) { top += workload; }
            else { top = num_gid - 1; }
        }
        error = sbjudi.start_call(myid, cores, mycomm);
        if (error) { errnum("start_call-judicator", error); }
        sbjudi.set_prompt(prompt_csv);
        for (int ii = 0; ii < cores; ii++)
        {
            std::thread incsv(&MainWindow::insert_csvs, this, ref(all_queue), ref(sbjudi), ref(scjudi));
            incsv.detach();
        }
        break;
    }

    // Loop through the worker threads, inserting their statements into the database.
    int active_thread = 0;
    int inert_threads = 0;
    int csv_pile, progress, num3, dayswork, num_batch, csv_average;
    int no_work = 0;              // How often the manager failed to find statements to process.
    while (mycomm[0] == 0)
    {
        active_thread = sbjudi.pull(myid, active_thread);
        timer.restart();
        num_batch = all_queue[active_thread].size();
        dayswork = min(handful, num_batch);
        if (dayswork > 0)
        {
            no_work = 0;
        }
        else
        {
            no_work++;
        }
        bot = 0;
        top = dayswork;
        do
        {
            for (int ii = bot; ii < top; ii++)   // For a specified number of CSVs in a batch...
            {
                sfgui.insert_prepared(all_queue[active_thread][ii]);
            }
            mycomm[1] += dayswork;
            sb.update(myid, mycomm);
            bot = top;
            dayswork = min(handful, (int)all_queue[active_thread].size() - bot);
            top += dayswork;
        } while (dayswork > 0);
        all_queue[active_thread].clear();
        sbjudi.done(myid);
        if (num_batch > 0)
        {
            csv_average = timer.restart() / num_batch;
            qDebug() << "Judicator average CSV time: " << csv_average;
        }

        // Manager stop check.
        comm_csv = sbjudi.update(myid, mycomm);
        if (comm_csv.size() == cores + 1)
        {
            inum = 0;
            for (int ii = 1; ii <= cores; ii++)
            {
                if (comm_csv[ii][0] == 2) { inum++; }
            }
            if (inum == cores)
            {
                mycomm[0] = -2;
                sb.update(myid, mycomm);
                return;
            }
            for (int ii = 1; ii <= cores; ii++)
            {
                if (comm_csv[ii][0] == 0)  // If the thread is still working, let it work. 
                {
                    break;
                }
                else if (ii == cores)  // If all threads report they have stopped...
                {
                    if (no_work > 3 * cores)  // ... and if we have made 3 empty sweeps for work...
                    {
                        mycomm[0] = 1;  // ... stop looking for work.
                        sb.update(myid, mycomm);
                    }
                }
            }
        }

        // Communicate updates to/from the GUI and/or worker threads.
        comm_gui = sb.update(myid, mycomm);
        if (comm_gui[0][0] == 2 && mycomm[0] == 0)
        {
            mycomm[0] = 2;
            comm_csv = sbjudi.update(myid, mycomm);  // Tell the workers to stop.
        }
        while (mycomm[0] == 2)
        {
            Sleep(10);
            comm_csv = sbjudi.update(myid, mycomm);
            num3 = 0;
            for (int ii = 1; ii <= cores; ii++)
            {
                if (comm_csv[ii][0] < 0)
                {
                    num3++;
                }
            }
            if (num3 >= cores)
            {
                mycomm[0] = -2;  // Final report to GUI: all threads have stopped.
                sb.update(myid, mycomm);
                log("Catalogue " + prompt[1] + " had its CSV insertion cancelled.");
                return;
            }
        }
    }

    // Update catalogue's description, if the insertion completed successfully.
    vector<string> test_results;
    if (mycomm[0] == 1)
    {
        test_results = sfgui.test_cata(prompt[1]);
        if (test_results[0] == "PASS")
        {
            temp = scjudi.get_cata_desc();
            jf.clean(temp, { "'" });
            stmt = "UPDATE TCatalogueIndex SET Description = '" + temp + "' WHERE Name = '";
            stmt += prompt[1] + "' ;";
            sfgui.executor(stmt);
            log("Catalogue " + prompt[1] + " completed its database insertion.");
            sbjudi.end_call(myid);
        }
        else
        {
            log("Catalogue " + prompt[1] + " failed its database insertion.");
            sbjudi.end_call(myid);
        }
    }
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
    ui->tV_viewtable->setModel(model);
    for (int ii = 0; ii < row_heights.size(); ii++)
    {
        ui->tV_viewtable->setRowHeight(row_heights[ii][0], row_heights[ii][1]);
    }
    ui->tabW_results2->setCurrentIndex(0);
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

// Download the selected file(s).
void MainWindow::on_pB_download_clicked()
{
    //jf.timerStart();
    int onlineTab = ui->tabW_online->currentIndex();
    QList<QTreeWidgetItem*> qlist, qChildren;
    vector<string> prompt, listCata, sLayer, csvLocal, csvOnline;
    vector<vector<int>> comm(1, vector<int>());
    comm[0].assign(comm_length, 0);  // Form [control, progress report, size report, max param].
    thread::id myid = this_thread::get_id();
    
    int kids, iStatusCata, error, iGen, inum, sizeComm, oldProgress, indexCata;
    int iYear, numCata;
    size_t pos1;
    vector<int> iLayer;
    QTreeWidgetItem* pNode = nullptr;
    QTreeWidgetItem* pParent = nullptr;
    QString qtemp;
    string temp, url, webpage, urlCata, urlGeoList;
    vector<vector<string>> csvDiff;
    switch (onlineTab)
    {
    case 0:
    {
        qlist = ui->treeW_statscan->selectedItems();
        if (qlist.size() < 1) { return; }
        iGen = qf.getBranchGen(qlist[0]);
        prompt.resize(2);  // Form [syear, sname].
        switch (iGen)
        {
        case 0:  // Root. Nothing to do...
            return;
        case 1:  // Year.
        {
            qtemp = qlist[0]->text(0);
            prompt[0] = qtemp.toStdString();
            url = sc.urlYear(prompt[0]);
            webpage = wf.browseS(url);
            listCata = jf.textParser(webpage, navSearch[1]);
            numCata = listCata.size();
            for (int ii = 0; ii < numCata; ii++)
            {
                prompt[1] = listCata[ii];
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
                while (comm[1][0] == 0)
                {
                    Sleep(gui_sleep);
                    QCoreApplication::processEvents();
                    comm = sb.update(myid, comm[0]);
                    if (comm[1][2] > comm[0][2])
                    {
                        comm[0][2] = comm[1][2];
                        temp = "Downloading catalogue " + prompt[1] + " (";
                        temp += to_string(ii + 1) + "/" + to_string(numCata) + ") ...";
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
                if (comm[0][2] == 0)
                {
                    temp = "Downloading catalogue " + prompt[1] + " (";
                    temp += to_string(ii + 1) + "/" + to_string(numCata) + ") ...";
                    barMessage(temp);
                }
                comm.resize(1);
                comm[0].assign(comm_length, 0);
                sb.end_call(myid);
            }
            return;
        }
        case 2:  // Catalogue.
        {
            qtemp = qlist[0]->parent()->text(0);
            prompt[0] = qtemp.toStdString();
            qtemp = qlist[0]->text(0);
            prompt[1] = qtemp.toStdString();
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
            return;
        }
        case 3:  // CSV or maps.
        {
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
            break;
        }
        }
        QCoreApplication::processEvents();

        if (listCata.size() < 1)
        {
            QList<QListWidgetItem*> qSelected = ui->listW_statscan->selectedItems();
            if (qSelected.size() > 0)
            {
                comm[0][2] = qSelected.size();
                prompt.resize(qSelected.size() + 3);
                for (int ii = 0; ii < qSelected.size(); ii++)
                {
                    qtemp = qSelected[ii]->text();
                    prompt[ii + 3] = qtemp.toStdString();
                }
            }
            else
            {
                iStatusCata = getCataStatus(prompt[0], prompt[1], csvLocal, csvOnline);
                if (iStatusCata == 1)
                {
                    csvDiff = jf.compareList(csvLocal, csvOnline);
                    prompt.resize(3 + csvDiff[1].size());
                    for (int jj = 0; jj < csvDiff[1].size(); jj++)
                    {
                        prompt[3 + jj] = csvDiff[1][jj];
                    }
                }
                else if (iStatusCata == 2)
                {
                    qf.displayText(ui->QL_bar, prompt[1] + " is already present in the database.");
                    break;
                }
                iStatusCata = sf.statusCata(prompt[1]);
                if (iStatusCata == 2)
                {
                    qf.displayText(ui->QL_bar, prompt[1] + " is already present in the database.");
                    return;
                }
                try { iYear = stoi(prompt[0]); }
                catch (invalid_argument& ia) { err("stoi-MainWindow.on_pB_download"); }
                urlCata = sc.urlCatalogue(iYear, prompt[1]);
                urlGeoList = sc.urlGeoList(iYear, urlCata);
                prompt.push_back(urlGeoList);
            }
            sb.set_prompt(prompt);
            sb.start_call(myid, 1, comm[0]);
            std::thread dl(&MainWindow::downloader, this, ref(sb));
            dl.detach();
            while (comm[0][1] < comm[0][2])
            {
                Sleep(gui_sleep);
                QCoreApplication::processEvents();
                comm = sb.update(myid, comm[0]);
                jobs_done = comm[1][1];
                update_bar();
                comm[0][1] = comm[1][1];
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

    while (1)
    {
        Sleep(gui_sleep);
        QCoreApplication::processEvents();
        if (remote_controller == 2)  // The 'cancel' button was pressed.
        {
            comm[0][0] = 2;  // Inform the manager thread it should abort its task.
            remote_controller = 0;
        }
        try
        {
            comm = sb.update(myid, comm[0]);
            if (comm[0][2] == 0)  // If the GUI thread does not yet know the size of the task...
            {
                if (comm[1][2] > 0)  // ... and the manager thread does know, then ...
                {
                    comm[0][2] = comm[1][2];
                    comm[0][1] = comm[1][1];
                    if (onlineTab == 0)
                    {
                        reset_bar(comm[1][2], "Downloading catalogue  " + prompt[1]);  // ... initialize the progress bar.
                    }
                    else if (onlineTab == 1)
                    {
                        reset_bar(comm[1][2], "Downloading maps...");  // ... initialize the progress bar.
                    }
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
        }
        catch (out_of_range& oor)
        {
            err("sb.update-on_pB_download_clicked");
        }

        if (comm[1][0] == 1 || comm[1][0] == -2)
        {
            error = sb.end_call(myid);
            if (error) { errnum("sb.end_call-on_pB_download_clicked", error); }
            jobs_done = comm[0][2];
            update_bar();
            break;           // Manager reports task finished/cancelled.
        }
    }
    
    //long long timer = jf.timerStop();
    //log("Downloaded catalogue " + prompt[1] + " in " + to_string(timer) + "ms");
}
void MainWindow::downloader(SWITCHBOARD& sb)
{
    vector<int> mycomm;
    vector<vector<int>> comm_gui;
    thread::id myid = this_thread::get_id();
    sb.answer_call(myid, mycomm);
    vector<string> prompt = sb.get_prompt();  // Form [syear, sname, gid0, ... ].
    int iyear, dlMode, yearMode;
    try { iyear = stoi(prompt[0]); }
    catch (out_of_range& oor) { err("stoi-MainWindow.downloader"); }
    string folderPath = sroot + "\\" + prompt[0] + "\\" + prompt[1];
    wf.makeDir(folderPath);
    string geoPath = folderPath + "\\" + prompt[1] + " geo list.bin";
    bool geo;
    if (prompt.size() > 3) { geo = 0; }
    else { geo = 1; sc.initGeo(); }

    size_t pos1;
    string sfile, urlCataDL, csvPath, csvFile;
    vector<string> gidList, geoLayers;
    vector<vector<string>> geoAll;
    string urlCata = sc.urlCatalogue(iyear, prompt[1]);
    string urlGeoList = sc.urlGeoList(iyear, urlCata);
    string geoPage = wf.browseS(urlGeoList);
    if (geo)
    {
        if (!wf.file_exist(geoPath))
        {
            fetchGeoList(iyear, prompt[1], geoLayers, geoPage, prompt[2]);
        }
        geoAll = sc.readGeo(geoPath);
        mycomm[2] = geoAll.size() - 1;
        sb.update(myid, mycomm);
    }
    else
    {
        mycomm[2] = prompt.size() - 2;
        sb.update(myid, mycomm);
        gidList.resize(mycomm[2]);
        for (int ii = 0; ii < mycomm[2]; ii++)
        {
            pos1 = prompt[ii + 2].find(')');
            gidList[ii] = prompt[ii + 2].substr(1, pos1 - 1);
        }
    }
    
    vector<string> dirt = { "/" };
    vector<string> soap = { "or" };
    for (int ii = 0; ii < mycomm[2]; ii++)
    {
        if (geo)
        {
            urlCataDL = sc.urlCataDownload(iyear, geoPage, geoAll[ii + 1][0]);
            csvPath = folderPath + "\\" + prompt[1] + " (" + geoAll[ii + 1][0];
            csvPath += ") " + geoAll[ii + 1][1] + ".csv";
        }
        else
        {
            urlCataDL = sc.urlCataDownload(iyear, geoPage, gidList[ii]);
            csvPath = folderPath + "\\" + prompt[1] + " " + prompt[ii + 2] + ".csv";
        }
        jf.clean(csvPath, dirt, soap);
        wf.download(urlCataDL, csvPath);
        mycomm[1]++;
        sb.update(myid, mycomm);
    }
    mycomm[0] = 1;
    sb.update(myid, mycomm);
    int bbq = 1;
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

// Search the database local drive for maps folders. Display the cumulative
// results as a tree, with only the best map (BIN > PNG > PDF) shown. 
void MainWindow::on_pB_localmaps_clicked()
{
    string pathFolder, search, nameLeaf, temp, pathSelected, nameSelected;
    string nameRoot = "Local Maps";
    string pathRoot = sroot;
    QString qtemp;
    vector<vector<int>> treeST;
    vector<int> treeiPL;
    vector<string> treePL, listName;
    QTreeWidgetItem* qnode = nullptr, *qParent = nullptr;
    QList<QTreeWidgetItem*> qSelected;
    size_t pos1;
    bool success;
    int numFolder, numFile, iValue;
    int numRoots = ui->treeW_maps->topLevelItemCount();
    if (numRoots == 0)  // Populate the QTree with all the map folders.
    {       
        jtMaps.init(nameRoot, pathRoot);
        treeST = { { 0 } };
        treePL = { pathRoot };
        search = "maps*";
        wf.getLayerFolder(0, treeST, treePL, search);
        numFolder = treePL.size();
        for (int ii = 1; ii < numFolder; ii++)
        {
            wf.getTreeFolder(ii, treeST, treePL);
        }
        treeiPL.assign(treeST.size(), -1);  // Folders always get an iValue of -1.
        jtMaps.inputTreeSTPL(treeST, treePL, treeiPL);
        qnode = new QTreeWidgetItem();
        qtemp = QString::fromStdString(nameRoot);
        qnode->setText(0, qtemp);
        qnode->setText(1, "");
        populateQTree(jtMaps, qnode, nameRoot);
        ui->treeW_maps->addTopLevelItem(qnode);
    }
    else
    {
        qSelected = ui->treeW_maps->selectedItems();
        if (qSelected.size() < 1) { return; }
        qtemp = qSelected[0]->text(1);
        if (qtemp == "")  // Is folder.
        {
            qtemp = qSelected[0]->text(0);
            qf.deleteLeaves(qSelected[0]);
        }
        else  // Is leaf.
        {
            qtemp = qSelected[0]->parent()->text(0);
            qnode = qSelected[0]->parent();
            qf.deleteLeaves(qnode);
        }
        nameSelected = qtemp.toStdString();
        jtMaps.deleteLeaves(nameSelected);

        pathSelected = nameSelected;
        qParent = qSelected[0]->parent();
        while (qParent != nullptr)
        {
            qtemp = qParent->text(0);
            pathSelected = qtemp.toStdString() + "\\" + pathSelected;
            qnode = qParent;
            qParent = qnode->parent();
        }
        pos1 = pathSelected.find('\\');
        temp = pathSelected.substr(pos1);
        pathSelected = pathRoot + temp;
        temp.push_back('\\');
        pos1 = temp.find('\\', 1);
        temp = temp.substr(1, pos1 - 1);
        if (temp == "mapsBIN") { search = ".bin"; }
        else if (temp == "mapsPNG") { search = ".png"; }
        else if (temp == "mapsPDF") { search = ".pdf"; }
        else { err("Failed to locate mapsX folder-MainWindow.on_pB_localmaps_clicked"); }
        search = "*" + search;
        listName = wf.get_file_list(pathSelected, search);
        selectedMapFolder = pathSelected;
        numFile = listName.size();
        nameLeaf = temp.substr(4);
        nameLeaf += " Maps";

        qf.displayBinList(ui->listW_bindone, listName);

        jtMaps.addChild(nameLeaf, numFile, pathSelected);
        qtemp = QString::fromStdString(nameLeaf);
        qnode = new QTreeWidgetItem(qSelected[0]);
        qnode->setText(0, qtemp);
        qtemp.setNum(numFile);
        qnode->setText(1, qtemp);
        qnode->setExpanded(1);
    }
    ui->tabW_online->setCurrentIndex(1);
    qParent = ui->treeW_maps->topLevelItem(0);
    qParent->setExpanded(1);
    qnode = qParent->child(0);
    qnode->setExpanded(1);
    qParent = qnode;
    qnode = qParent->child(1);
    qnode->setExpanded(1);
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

// Erase bad points on a BIN map.  
void MainWindow::on_pB_correct_clicked()
{
    QList<QTreeWidgetItem*> qSelected = ui->treeW_maps->selectedItems();
    if (qSelected.size() != 1) { return; }
    selectedMapFolder = qf.makePathTree(qSelected[0]);
    vector<string> dirt = { "mapsPNG" };
    vector<string> soap = { "mapsBIN" };
    jf.clean(selectedMapFolder, dirt, soap);
    string search = "*.bin";
    vector<string> listBin = wf.get_file_list(selectedMapFolder, search);
    qf.displayBinList(ui->listW_bindone, listBin);
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
    string binFile = jf.load(binPath), geoLayersPath, sNum, sParent;
    size_t pos1 = binFile.find("//position"), pos2, posGeo1, posGeo2;
    int inum, numLayers = 0;
    vector<double> binPos;
    vector<unsigned char> rgbTarget;
    string pngParentPath, pngGrandparentPath, myLayer, newBin, pngGPathASCII, pngPPathASCII;
    if (pos1 > binFile.size())  // Update the BIN map from the 'geo layers' file.
    {
        // Obtain the parent region's name.
        pos1 = binPath.rfind('\\');
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
        if (myLayer == "province") { rgbTarget = { 255, 255, 255 }; }
        else { rgbTarget = { 0, 112, 255 }; }
        binPos = im.getPosition(pngParentPath, rgbTarget);

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

// Modes: 0 = download given webpage
void MainWindow::on_pB_test_clicked()
{
    int mode = 0;

    switch (mode)
    {
    case 0:  // Download the webpage given by the text box's URL.
    {
        QString qtemp = ui->pte_webinput->toPlainText();
        string url = qtemp.toStdString();
        wstring webpage = wf.browseW(url);
        jf.printer(sroot + "\\Test webpage.txt", webpage);
        return;
    }
    case 1:  // Fix UTF-8 filenames into ASCII, for the given year's CSVs.
    {
        QString qtemp = ui->pte_webinput->toPlainText();
        string sYear = qtemp.toStdString();
        int iYear;
        try { iYear = stoi(sYear); }
        catch (invalid_argument& ia) { jf.err("stoi-MainWindow.test"); }
        string pathFolderYear = sroot + "\\" + sYear;
        string search = "*", pathFolderCata, csvPath, csvPathAscii, asciiName;
        vector<string> listFolderName = wf.get_folder_list(pathFolderYear, search);
        vector<string> listCSVName;
        search = "*.csv";
        for (int ii = 0; ii < listFolderName.size(); ii++)
        {
            pathFolderCata = pathFolderYear + "\\" + listFolderName[ii];
            listCSVName = wf.getFileList(pathFolderCata, search);
            for (int jj = 0; jj < listCSVName.size(); jj++)
            {
                for (int kk = 0; kk < listCSVName[jj].size(); kk++)
                {
                    if (listCSVName[jj][kk] == -61)
                    {
                        csvPath = pathFolderCata + "\\" + listCSVName[jj];
                        wf.delete_file(csvPath);
                        break;
                    }
                }
            }
        }
    }
    case 2:  // For every file in the given folder, force the file name into ASCII.
    {
        QList<QTreeWidgetItem*> qlist = ui->treeW_maps->selectedItems();
        if (qlist.size() != 1) { return; }
        string folderPath = qf.getBranchPath(qlist[0], sroot);
        makeTempASCII(folderPath);
        int bbq = 1;
        break;
    }
    case 3:  // For every bin map in the selected folder, convert frame coords to ints.
    {
        QList<QTreeWidgetItem*> qlist = ui->treeW_maps->selectedItems();
        if (qlist.size() != 1) { return; }
        string folderPath = qf.getBranchPath(qlist[0], sroot);
        string search = "*.bin", pathBin, sfile;
        vector<string> listBinName = wf.get_file_list(folderPath, search);
        size_t pos1, pos2;
        for (int ii = 0; ii < listBinName.size(); ii++)
        {
            pathBin = folderPath + "\\" + listBinName[ii];
            sfile = jf.load(pathBin);
            pos1 = 0;
            for (int jj = 0; jj < 4; jj++)
            {
                pos1 = sfile.find('.', pos1 + 1);
                if (pos1 > sfile.size()) { break; }
                pos2 = sfile.find(',', pos1);
                sfile.erase(pos1, pos2 - pos1);
                pos1 = sfile.find('.', pos1);
                pos2 = sfile.find_first_of("\r\n", pos1);
                sfile.erase(pos1, pos2 - pos1);
            }
            jf.printer(pathBin, sfile);
        }
        break;
    }
    case 4:  // For every bin map in the selected folder, update its frames to include scale and position.
    {
        QList<QTreeWidgetItem*> qlist = ui->treeW_maps->selectedItems();
        if (qlist.size() != 1) { return; }
        string folderPath = qf.getBranchPath(qlist[0], sroot);
        string search = "*.bin", pathBin, pathPng, sfileOld, sfileNew, temp;
        vector<string> dirt = { "mapsBIN", ".bin" };
        vector<string> soap = { "mapsPNG", ".png" };
        vector<string> otherDirt = { "\r", "\u00E9", "\u00CE", "\u00C9" };
        vector<string> listBinName = wf.get_file_list(folderPath, search);
        string folderPathPNG = folderPath;
        //makeTempASCII(folderPathPNG);
        size_t pos1, pos2;
        vector<vector<vector<int>>> threeFrames;
        for (int ii = 0; ii < listBinName.size(); ii++)
        {
            pathBin = folderPath + "\\" + listBinName[ii];
            sfileOld = jf.load(pathBin);
            pos1 = sfileOld.find("//frames");
            pos1 = sfileOld.find('\n', pos1);
            pos1 = sfileOld.find('@', pos1);
            if (pos1 < sfileOld.size()) { continue; }
            temp = jf.asciiOnly(listBinName[ii]);           
            pathPng = folderPathPNG + "\\" + temp;
            jf.clean(pathPng, dirt, soap);
            im.pngLoad(pathPng);
            threeFrames = im.pngThreeFrames();
            sfileNew = "//frames\n";
            for (int jj = 0; jj < 3; jj++)
            {
                sfileNew += to_string(threeFrames[jj][0][0]) + ",";
                sfileNew += to_string(threeFrames[jj][0][1]) + "@";
                sfileNew += to_string(threeFrames[jj][1][0]) + ",";
                sfileNew += to_string(threeFrames[jj][1][1]) + "\n";
            }
            sfileNew += "\n";
            pos1 = sfileOld.find("//border");
            pos2 = sfileOld.rfind(',');
            pos2 = sfileOld.find("\n", pos2) + 1;
            temp = sfileOld.substr(pos1, pos2 - pos1);
            sfileNew.append(temp);
            jf.printer(pathBin, sfileNew);
        }
        //makeTempASCII(folderPathPNG);
        break;
    }
    case 5:  // For every pdf map in the selected folder, extract its text. 
    {
        QList<QTreeWidgetItem*> qlist = ui->treeW_maps->selectedItems();
        if (qlist.size() != 1) { return; }
        string folderPath = qf.getBranchPath(qlist[0], sroot);
        string search = "*.pdf", pathPdf, pathTxt, sfileOld, sfileNew, temp;
        vector<string> dirt = { "mapsBIN", ".bin" };
        vector<string> soap = { "mapsPDF", ".pdf" };
        vector<string> otherDirt = { "\r", "\u00E9", "\u00CE", "\u00C9" };
        jf.clean(folderPath, dirt, soap);
        vector<string> listPdfName = wf.get_file_list(folderPath, search);
        dirt = { ".pdf" };
        soap = { ".txt" };
        for (int ii = 0; ii < listPdfName.size(); ii++)
        {
            pathPdf = folderPath + "\\" + listPdfName[ii];
            pathTxt = pathPdf;
            jf.clean(pathTxt, dirt, soap);
            gf.pdfToTxt(pathPdf, pathTxt);
        }
        break;
    }
    case 6:  // For every bin map in the selected folder, add a section for scale (pixels per kilometer).
    {
        QList<QTreeWidgetItem*> qlist = ui->treeW_maps->selectedItems();
        if (qlist.size() != 1) { return; }
        string folderPath = qf.getBranchPath(qlist[0], sroot);
        string search = "*.bin", pathBIN, pathPNG, pathTXT, sfileOld, sfileNew, temp, utf8;
        vector<string> listBinName = wf.get_file_list(folderPath, search), dirt, soap;
        size_t pos1, pos2;
        int scalePixels;
        double PPKM;
        vector<string> otherDirt = { "\u00E9", "\u00CE", "\u00C9", "\u00E8" };
        vector<vector<vector<int>>> threeFrames;
        string status = "Updating BIN map : ";
        reset_bar(listBinName.size(), status);
        for (int ii = 0; ii < listBinName.size(); ii++)
        {
            temp = status + listBinName[ii];
            barMessage(temp);
            pathBIN = folderPath + "\\" + listBinName[ii];
            sfileOld = jf.load(pathBIN);
            pos1 = sfileOld.find("//frames(");
            if (pos1 < sfileOld.size()) 
            { 
                jobs_done++;
                update_bar();
                continue; 
            }
            sfileNew = "//frames(topLeft@botRight, showing 'maps', 'scale', 'position')\n";
            pos1 = sfileOld.find("//frames") + 8;
            pos1 = sfileOld.find_first_of("1234567890", pos1);
            pos2 = sfileOld.find("//", pos1);
            pos2 = sfileOld.find_last_of("1234567890", pos2) + 1;
            temp = sfileOld.substr(pos1, pos2 - pos1);
            sfileNew += temp + "\n\n";
            sfileNew += "//scale(pixels per km)\n";
            dirt = { "mapsBIN", ".bin" };
            soap = { "mapsPNG", ".png" };
            pathPNG = pathBIN;
            jf.clean(pathPNG, dirt, soap);
            utf8 = jf.asciiToUTF8(pathPNG);
            jf.clean(utf8, otherDirt);
            pathPNG = jf.utf8ToAscii(utf8);
            im.pngLoad(pathPNG);
            threeFrames = im.pngThreeFrames();
            scalePixels = im.getScalePixels(threeFrames[1]);
            soap = { "mapsPDF", ".txt" };
            pathTXT = pathBIN;
            jf.clean(pathTXT, dirt, soap);
            PPKM = im.getPPKM(pathTXT, scalePixels);
            sfileNew += to_string(PPKM) + "\n\n";
            sfileNew += "//border\n";
            pos1 = sfileOld.find("//border");
            pos1 = sfileOld.find_first_of("1234567890", pos1);
            pos2 = sfileOld.find_last_of("1234567890") + 1;
            sfileNew += sfileOld.substr(pos1, pos2 - pos1);
            sfileNew += "\n";
            jf.printer(pathBIN, sfileNew);
            jobs_done++;
            update_bar();
            QCoreApplication::processEvents();
        }

        break;
    }
    case 7:  // For every CSV in all subfolders, rewrite it as UTF8 if it is UTF16.
    {
        QString qYear = ui->pte_webinput->toPlainText();
        string sYear = qYear.toStdString(), temp;
        vector<string> dirt = { "/" };
        vector<string> soap = { "or" };
        int iYear, count, index;
        size_t pos1, pos2, len, pos0;
        try { iYear = stoi(sYear); }
        catch (invalid_argument& ia)
        {
            qYear = "Not a valid year.";
            ui->pte_webinput->setPlainText(qYear);
            return;
        }
        string rootDir = sroot + "\\" + sYear;
        vector<vector<int>> treeST;
        vector<string> treePL, listCSVname;
        string search = "*", cataPath, csvPath, sfile, utf8, nameInside, nameOutside;
        wf.make_tree_local(treeST, treePL, 1, rootDir, 2, search);
        search = "*.csv";
        string status = "Converting UTF16 to UTF8 : ";
        reset_bar(treePL.size(), status);
        for (int ii = 1; ii < treePL.size(); ii++)
        {
            temp = status + treePL[ii];
            barMessage(temp);
            cataPath = rootDir + "\\" + treePL[ii];
            listCSVname = wf.get_file_list(cataPath, search);
            for (int jj = 0; jj < listCSVname.size(); jj++)
            {
                csvPath = cataPath + "\\" + listCSVname[jj];
                sfile = jf.load(csvPath);
                count = 0;
                for (int kk = 0; kk < 8; kk++)
                {
                    if (sfile[kk] == 0) { count++; }
                }
                if (count == 4)  // UTF16...
                {
                    utf8.clear();
                    utf8.resize(sfile.size() + 3);
                    index = 0;
                    for (int kk = 0; kk < sfile.size(); kk++)
                    {
                        if (sfile[kk] != 0)
                        {
                            utf8[index] = sfile[kk];
                            index++;
                        }
                    }
                    utf8.resize(index);
                    jf.printer(csvPath, utf8);
                }

                /*
                else   // FML...
                {
                    for (int ii = sfile.size() - 2; ii >= 0; ii--)
                    {
                        if (sfile[ii] != 0)
                        {
                            sfile.resize(ii + 1);
                            break;
                        }
                    }
                    pos1 = sfile.find("Geography = ");
                    if (pos1 > sfile.size()) { err("FML-mw.test7"); }
                    pos1 += 12;
                    pos0 = pos1;
                    pos2 = sfile.find_first_of("[\"", pos1);
                    while (sfile[pos2 - 1] == ' ') { pos2--; }
                    nameInside = sfile.substr(pos1, pos2 - pos1);
                    len = nameInside.size();
                    jf.clean(nameInside, dirt, soap);
                    pos1 = csvPath.find(nameInside);
                    if (pos1 > csvPath.size())
                    {
                        pos1 = csvPath.find(')') + 2;
                        pos2 = csvPath.rfind(".csv");
                        nameOutside = csvPath.substr(pos1, pos2 - pos1);
                        jf.clean(nameOutside, soap, dirt);
                        utf8 = jf.asciiToUTF8(nameOutside);
                        sfile.replace(pos0, len, utf8);
                    }
                    jf.printer(csvPath, sfile);
                }
                */

            }
            jobs_done++;
            update_bar();
            QCoreApplication::processEvents();
        }
        int bbq = 1;
        break;
    }
    case 8:  // For a given catalogue folder path, delete all CSVs with non-ASCII names.
    {
        QString qtemp = ui->pte_webinput->toPlainText();
        string folderPath = qtemp.toStdString();
        string search = "*.csv";
        vector<string> listCSVname = wf.get_file_list(folderPath, search);
        int numCSV = listCSVname.size();
        search = "Deleting files ...";
        reset_bar(numCSV, search);
        for (int ii = 0; ii < numCSV; ii++)
        {
            for (int jj = 0; jj < listCSVname[ii].size(); jj++)
            {
                if (listCSVname[ii][jj] < 0)
                {
                    search = folderPath + "\\" + listCSVname[ii];
                    wf.delete_file(search);
                    break;
                }
            }
            jobs_done++;
            update_bar();
        }
    }
    case 9:  // For a selected BIN map, slide it upward/leftward to reduce empty space.
    {
        QList<QListWidgetItem*> qSel = ui->listW_bindone->selectedItems();
        if (qSel.size() != 1) { return; }
        QString qtemp = qSel[0]->text();
        wstring wTemp = qtemp.toStdWString();
        string temp = jf.utf16to8(wTemp);
        string mapBinPath = selectedMapFolder + "\\" + jf.utf8ToAscii(temp) + ".bin";
        vector<vector<int>> frameCorners, border, TLBR;
        //qf.loadBinMap(mapBinPath, frameCorners, border);
        TLBR = im.makeBox(border);

        string sfile = jf.load(mapBinPath);
        size_t pos1 = sfile.find("//border");
        pos1 = sfile.find('\n', pos1) + 1;
        string newFile = sfile.substr(0, pos1);

        int Dx, Dy, cushion = 120;
        Dx = TLBR[0][0] - cushion;
        Dy = TLBR[0][1] - cushion;
        for (int ii = 0; ii < border.size(); ii++)
        {
            border[ii][0] -= Dx;
            border[ii][1] -= Dy;
            newFile += to_string(border[ii][0]) + "," + to_string(border[ii][1]) + "\n";
        }
        jf.printer(mapBinPath, newFile);
        int bbq = 1;
        break;
    }
    case 10: // Update a folder's worth of BIN maps to include position.
    {
        int numBin = ui->listW_bindone->count();
        if (numBin < 1) { return; }
        string temp = "test 10";
        reset_bar(numBin, temp);
        for (int ii = 0; ii < numBin; ii++)
        {
            ui->listW_bindone->setCurrentRow(ii, QItemSelectionModel::Select);
            on_pB_pos_clicked();
            ui->listW_bindone->setCurrentRow(ii, QItemSelectionModel::Deselect);
            jobs_done++;
            update_bar();
            QCoreApplication::processEvents();
        }
        break;
    }
    case 11: // For a given [pngPath \n binPath], make a single bin map.
    {
        QString qtemp = ui->pte_webinput->toPlainText(), qMessage;
        string temp = qtemp.toStdString(), temp8;
        string sfile = jf.load(temp);
        thread::id myid = this_thread::get_id();
        vector<vector<int>> comm(1, vector<int>());

        vector<string> prompt(4);  // Form ["width,height", "startX,startY", pngPath, binPath].
        int inum = ui->label_maps->width(), error;
        prompt[0] += to_string(inum) + ",";
        inum = ui->label_maps->height();
        prompt[0] += to_string(inum);

        size_t pos1 = sfile.find("//pathPNG");
        pos1 = sfile.find('\n', pos1) + 1;
        size_t pos2 = sfile.find('\r', pos1);
        prompt[2] = sfile.substr(pos1, pos2 - pos1);

        vector<string> listBIN, listCoord, debugPath, bagOfAir, vsTemp;
        size_t pos3 = sfile.find("//pathBIN");
        pos1 = sfile.find('\n', pos3) + 1;
        pos2 = sfile.find('\r', pos1);
        pos3 = sfile.find("\r\n\r\n", pos3);
        while (pos1 < pos3)
        {
            temp8 = sfile.substr(pos1, pos2 - pos1);
            temp = jf.utf8ToAscii(temp8);
            listBIN.push_back(temp);
            pos1 = pos2 + 2;
            pos2 = sfile.find('\r', pos1);
        }
        pos3 = sfile.find("//borderStart");
        pos1 = sfile.find('\n', pos3) + 1;
        pos2 = sfile.find('\r', pos1);
        pos3 = sfile.find("\r\n\r\n", pos3);
        while (pos1 < pos3)
        {
            temp8 = sfile.substr(pos1, pos2 - pos1);
            temp = jf.utf8ToAscii(temp8);
            listCoord.push_back(temp);
            pos1 = pos2 + 2;
            pos2 = sfile.find('\r', pos1);
        }

        ui->tabW_online->setCurrentIndex(2);
        qf.initPixmap(ui->label_maps);
        for (int ii = 0; ii < listBIN.size(); ii++)
        {
            if (wf.file_exist(listBIN[ii])) { continue; }
            prompt[1] = listCoord[ii];
            prompt[3] = listBIN[ii];
            comm.resize(1);
            comm[0].assign(comm_length, 0);
            error = sb.start_call(myid, 1, comm[0]);
            if (error) { errnum("start_call-MainWindow.test11", error); }
            sb.set_prompt(prompt);
            std::thread thr(&MainWindow::convertSingle, this, ref(sb));
            thr.detach();
            while (comm.size() < 2)
            {
                Sleep(gui_sleep);
                comm = sb.update(myid, comm[0]);
            }
            while (comm[0][0] == 0)
            {
                Sleep(gui_sleep);
                QCoreApplication::processEvents();
                comm = sb.update(myid, comm[0]);
                if (comm[1][0] == 1)
                {
                    error = sb.end_call(myid);
                    if (error) { errnum("sb.end_call-test11", error); }
                    comm[0][0] = 1;
                }
                else if (comm[1][0] == 3)
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
                        catch (invalid_argument) {}
                    }
                    ui->pB_resume->setEnabled(1);
                    ui->pB_pause->setEnabled(0);
                    ui->pB_advance->setEnabled(1);
                    ui->pB_backspace->setEnabled(1);
                    remote_controller = 3;
                    while (1)
                    {
                        comm[0][0] = 3;
                        QCoreApplication::processEvents();
                        comm = sb.update(myid, comm[0]);
                        if (remote_controller == 1 && comm[0][0] == 3)
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
                            if (remote_controller == 4) { remote_controller = 1; }
                            ui->checkB_override->setChecked(0);
                            break;
                        }
                        else if (comm[1][0] == 1)
                        {
                            remote_controller = 0;
                            comm[0][0] = 1;
                            break;
                        }
                        Sleep(50);
                    }
                }
            }
            sb.end_call(myid);
        }
        qtemp = "Done!";
        ui->pte_webinput->setPlainText(qtemp);
        break;
    }
    case 12:  // For all catalogues in the database, add their TMaps if needed.
    {
        vector<string> search = { "Year", "Name" };
        string temp = "TCatalogueIndex", geoPath;
        vector<vector<string>> vvsResult;
        sf.select(search, temp, vvsResult);
        QString qtemp;
        ui->listW_statscan->clear();
        for (int ii = 0; ii < vvsResult.size(); ii++)
        {
            qtemp = QString::fromStdString(vvsResult[ii][1]);
            ui->listW_statscan->addItem(qtemp);
        }
        QCoreApplication::processEvents();

        vector<int> gidList;
        vector<string> regionList, layerList, geoLayers;
        for (int ii = 0; ii < vvsResult.size(); ii++)
        {
            barMessage("Adding " + vvsResult[ii][1]);
            geoPath = sroot + "\\" + vvsResult[ii][0] + "\\" + vvsResult[ii][1];
            geoPath += "\\" + vvsResult[ii][1] + " geo list.bin";
            sc.loadGeo(geoPath, gidList, regionList, layerList, geoLayers);
            addTMap(sf, gidList, regionList, layerList, vvsResult[ii][1]);
        }
        temp = "Finished test12.";
        barMessage(temp);
        break;
    }
    case 13:  // Remove all 'incomplete' entries in TCatalogueIndex.
    {
        string stmt = "DELETE FROM TCatalogueIndex WHERE (Description ";
        stmt += "LIKE 'Incomplete');";
        sf.executor(stmt);
        break;
    }
    }

    int bbq = 1;
}

// Choose a local drive to examine for spreadsheets.
void MainWindow::on_cB_drives_currentTextChanged(const QString& arg1)
{
    qdrive = arg1;
    wdrive = arg1.toStdWString();
    sdrive = arg1.toStdString();
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
