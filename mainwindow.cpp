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
    for (int ii = 0; ii < qdrives.size(); ii++)
    {
        ui->cB_drives->addItem(qdrives[ii]);
    }
    viewcata_data.assign(2, "");

    // Open the database from an existing local db file, or (failing that) make a new one.
    sf.init(sroot + "\\SCDA.db");
    //q.exec(QStringLiteral("PRAGMA journal_mode=WAL"));

    // Populate the navSearch matrix.
    navSearch = sc.navAsset();

    // Create (if necessary) these system-wide tables. 
    create_cata_index_table();  
    create_damaged_table();
    
    // Load hidden initial values into the GUI widgets.
    qf.set_display_root(ui->treeW_gid, 1);
    qf.set_display_root(ui->treeW_csvtree, 1);
    qf.set_display_root(ui->treeW_subtables, 0);
    update_treeW_cataindb();
    ui->tabW_catalogues->setCurrentIndex(0);

    // Initialize mode to 'local'. 
    qtemp = QString::fromStdString(modes[active_mode]);
    ui->label_mode->setText(qtemp);
    ui->tabW_results2->setGeometry(760, 10, 531, 481);
    ui->tV_viewtable->setGeometry(0, 0, 531, 461);
    ui->treeW_subtables->setGeometry(0, 0, 531, 461);
    ui->tabW_online->setVisible(0);
    ui->tabW_online->setGeometry(370, 10, 921, 481);
    ui->treeW_statscan->setGeometry(0, 0, 921, 461);
    ui->treeW_statscan->setVisible(0);
    ui->treeW_maps->setGeometry(0, 0, 921, 461);
    ui->treeW_maps->setVisible(0);
    ui->label_maps->setGeometry(0, 0, 710, 450);
    ui->label_maps->setVisible(0);
    ui->label_maps2->setGeometry(720, 0, 200, 450);
    ui->label_maps2->setVisible(0);
    ui->pB_usc->setVisible(0);
    ui->pB_download->setVisible(0);
    ui->pB_download->setEnabled(0);
    ui->pte_webinput->setVisible(0);
    ui->pte_webinput->setGeometry(370, 530, 241, 41);
    ui->pte_localinput->setVisible(1);
    ui->pte_localinput->setGeometry(810, 530, 81, 41);
    ui->pB_localmaps->setVisible(0);
    ui->pB_localmaps->setGeometry(720, 530, 81, 41);

    // Initialize the progress bar with blanks.
    reset_bar(100, " ");

    // Reset the log file.
    wf.delete_file(sroot + "\\SCDA Process Log.txt");
    log("MainWindow initialized.");
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
        if (jobs_percent == 100 && jump > 0)
        {
            pb_text = ui->QL_bar->text();
            pb_text.append(" ... done!");
            ui->QL_bar->setText(pb_text);
        }
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
    string stmt = "CREATE TABLE IF NOT EXISTS TCatalogueIndex (Year TEXT, ";
    stmt += "Name TEXT, Description TEXT);";
    sf.executor(stmt);
}
void MainWindow::create_damaged_table()
{
    string stmt = "CREATE TABLE IF NOT EXISTS TDamaged ([Catalogue Name] TEXT, ";
    stmt += "GID INT, [Number of Errors] INT);";
    sf.executor(stmt);
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
void MainWindow::update_mode()
{
    switch (active_mode)
    {
    case 0:
        ui->tabW_results->setVisible(1);
        ui->tabW_results2->setVisible(1);
        ui->pB_viewcata->setVisible(1);
        ui->pB_viewcata->setVisible(1);
        ui->tabW_online->setVisible(0);
        ui->treeW_statscan->setVisible(0);
        ui->treeW_maps->setVisible(0);
        ui->label_maps->setVisible(0);
        ui->label_maps2->setVisible(0);
        ui->pB_usc->setVisible(0);
        ui->pte_webinput->setVisible(0);
        ui->pte_localinput->setVisible(1);
        ui->pB_search->setVisible(1);
        ui->pB_download->setVisible(0);
        ui->pB_viewtable->setVisible(1);
        ui->pB_localmaps->setVisible(0);
        break;
    case 1:
        ui->tabW_results->setVisible(0);
        ui->tabW_results2->setVisible(0);
        ui->pB_viewcata->setVisible(0);
        ui->pB_viewcata->setVisible(0);
        ui->tabW_online->setVisible(1);
        ui->treeW_statscan->setVisible(1);
        ui->treeW_maps->setVisible(1);
        ui->label_maps->setVisible(1);
        ui->label_maps2->setVisible(1);
        ui->pB_usc->setVisible(1);
        ui->pte_webinput->setVisible(1);
        ui->pte_localinput->setVisible(0);
        ui->pB_search->setVisible(0);
        ui->pB_download->setVisible(1);
        ui->pB_viewtable->setVisible(0);
        ui->pB_localmaps->setVisible(1);
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
    sb.set_prompt(myid, prompt);
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
    QTreeWidgetItem* qitem;
    vector<string> prompt(3);  // syear, sname, desc, mode.
    vector<string> results1, search, conditions, vtemp;
    vector<vector<string>> results;
    QString qyear, qname;
    string syear, sname, stmt;
    int mode, error, sb_index;
    vector<vector<int>> comm(1, vector<int>());
    comm[0].assign(comm_length, 0);  // Form [control, progress report, size report, max param].
    
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

        // Determine that catalogue's current status in the database.
        search = { "Description" };
        conditions = {
            "Year = '" + syear + "'",
            "AND Name = '" + sname + "'"
        };
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
            sb.set_prompt(myid, prompt);
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
                        reset_bar(comm[1][2], "Inserting catalogue  " + sname);  // ... initialize the progress bar.
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
            
            if (comm[1][0] == 1 || comm[1][0] == -2)
            {
                error = sb.end_call(myid);
                if (error) { errnum("sb.end_call-on_pB_insert", error); }
                jobs_done = comm[0][2];
                update_bar();
                break;           // Manager reports task finished/cancelled.
            }
        }
    }

    // Update the GUI and then retire to obscurity.
    ui->pB_cancel->setEnabled(0);
    Sleep(100);
    update_treeW_cataindb();
}
void MainWindow::judicator(SQLFUNC& sf, SWITCHBOARD& sb, WINFUNC& wf)
{
    vector<int> mycomm;
    vector<vector<int>> comm_gui;
    thread::id myid = this_thread::get_id();
    sb.answer_call(myid, mycomm);
    vector<string> prompt = sb.get_prompt();
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
    timer.start();
    vector<string> csv_list = wf.get_file_list(cata_path, "*.csv");
    string temp = cata_path + "\\" + csv_list[0];
    string sample_csv = jfjudi.load(temp);
    STATSCAN scjudi(cata_path);
    scjudi.cata_init(sample_csv);    
    scjudi.extract_gid_list(csv_list);
    scjudi.extract_csv_branches(csv_list);
    qDebug() << "Judicator timer, STATSCAN object: " << timer.restart();
    
    // Create and insert this catalogue's row indexing table, if necessary.
    string tname = "TG_Row$" + prompt[1];
    int tg_row_col;
    vector<string> row_queue;
    if (!sf.table_exist(tname))
    {
        log("Begin row indexing (TGR) for catalogue " + prompt[1]);
        tg_row_col = scjudi.make_tgrow_statements(row_queue);
        sf.executor(row_queue[0]);
        row_queue.erase(row_queue.begin());
        sf.safe_col(tname, tg_row_col);
        sf.insert_prepared(row_queue);
        log("Completed row indexing (TGR) for catalogue " + prompt[1]);
    }
    qDebug() << "Judicator timer, TG_Row: " << timer.restart();

    // Create and insert this catalogue's region indexing table, if necessary.
    tname = "TG_Region$" + prompt[1];
    int tg_region_col;
    vector<string> geo_queue;
    if (!sf.table_exist(tname))
    {
        log("Begin region indexing (TGR) for catalogue " + prompt[1]);
        tg_region_col = scjudi.make_tgr_statements(geo_queue, prompt[0], prompt[1]);
        sf.executor(geo_queue[0]);
        geo_queue.erase(geo_queue.begin());
        sf.safe_col(tname, tg_region_col);
        sf.insert_prepared(geo_queue);
        log("Completed region indexing (TGR) for catalogue " + prompt[1]);
    }
    qDebug() << "Judicator timer, TG_Region: " << timer.restart();

    // Create and insert this catalogue's primary table, if necessary.
    string stmt = scjudi.make_create_primary_table();
    sf.executor(stmt);
    qDebug() << "Judicator timer, primary table: " << timer.restart();

    // Launch the worker threads, which will iterate through the CSVs.
    SWITCHBOARD sbjudi;
    int workload = num_gid / cores;
    int bot = 0;
    int top = workload - 1;
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
        sbjudi.set_prompt(myid, prompt_csv);
        for (int ii = 0; ii < cores; ii++)
        {
            std::thread incsv(&MainWindow::insert_csvs, this, ref(all_queue), ref(sbjudi), ref(scjudi));
            incsv.detach();
        }
        break;
    }
    qDebug() << "Judicator timer, launch workers: " << timer.restart();

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
                sf.insert_prepared(all_queue[active_thread][ii]);
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
        test_results = sf.test_cata(prompt[1]);
        if (test_results[0] == "PASS")
        {
            temp = scjudi.get_cata_desc();
            jf.clean(temp, { "'" });
            stmt = "UPDATE TCatalogueIndex SET Description = '" + temp + "' WHERE Name = '";
            stmt += prompt[1] + "' ;";
            sf.executor(stmt);
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
        sfile = jfcsv.load(csv_path);
        text_vars = scjudi.extract_text_vars(sfile);
        data_rows = scjudi.extract_rows(sfile, damaged_csv);
        linearized_titles = scjudi.linearize_row_titles(data_rows, column_titles);
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
    sb.set_prompt(myid, prompt);
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
    QString qtemp = cata_to_do[0]->text(0);
    prompt[0] = qtemp.toStdString();
    qtemp = cata_to_do[0]->text(1);
    prompt[1] = qtemp.toStdString();
    qtemp = cata_to_do[0]->text(2);
    prompt[2] = qtemp.toStdString();
    vector<vector<int>> comm(1, vector<int>());
    comm[0].assign(comm_length, 0);  // Form [control, progress report, size report, max columns].
    thread::id myid = this_thread::get_id();
    log("Beginning removal of catalogue " + prompt[1] + " from the database.");
    int error = sb.start_call(myid, 1, comm[0]);
    if (error) { errnum("start_call-pB_removecata_clicked", error); }
    sb.set_prompt(myid, prompt);
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
    mycomm[2] = 6;
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
        sf.remove(csv_list[ii], mode);
    }
    mycomm[1]++;
    sb.update(myid, mycomm);
    qDebug() << "Time to remove CSV tables: " << timer.restart();

    // Remove the catalogue's primary table.
    sf.remove(prompt[1]);
    mycomm[1]++;
    sb.update(myid, mycomm);
    qDebug() << "Time to remove the primary table: " << timer.restart();

    // Remove TG_Region.
    string tname = "TG_Region$" + prompt[1];
    sf.remove(tname);
    mycomm[1]++;
    sb.update(myid, mycomm);
    qDebug() << "Time to remove TG_Region: " << timer.restart();

    // Remove TG_Row.
    tname = "TG_Row$" + prompt[1];
    sf.remove(tname);
    mycomm[1]++;
    sb.update(myid, mycomm);
    qDebug() << "Time to remove TG_Row: " << timer.restart();

    // Remove entries from TDamaged.
    vector<string> conditions = {"[Catalogue Name] = '" + prompt[1] + "'"};
    sf.remove("TDamaged", conditions);
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
    string temp, gid, tname, row_title, result;
    vector<string> row_split, search, conditions;
    QList<QTreeWidgetItem*> qcurrent;
    QTreeWidgetItem* qitem;
    QString qtemp;

    // Obtain the CSV table name.
    qcurrent = ui->treeW_gid->selectedItems();
    if (qcurrent.size() < 1) { return; }
    qtemp = qcurrent[0]->text(0);
    temp = qtemp.toStdString();
    string temp2 = "'";
    jf.clean(temp, { "" }, temp2);
    search = { "GID" };
    tname = "TG_Region$" + viewcata_data[1];
    conditions = { "[Region Name] = '" + temp + "'" };
    sf.select(search, tname, gid, conditions);
    tname = viewcata_data[1] + "$" + gid;

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

// Query Statistics Canada for information.
void MainWindow::on_pB_usc_clicked()
{
    int error, inum, uscMode, iyear, ipdf, ijpg;
    int iroot = -1;
    string temp, syear, filePath, sname, webpage, url;
    vector<string> slayer;
    vector<int> ilayer;
    QTreeWidgetItem* qnode;
    QString qtemp;

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
        webpage = wf.browse(scroot);
        slayer = jf.textParser(webpage, navSearch[0]);
        jf.isort_slist(slayer);
        sname = "Statistics Canada Census Data";
        jt.init(sname);
        ilayer = jf.svectorToIvector(slayer);
        jt.addChildren(slayer, ilayer, iroot);
        ui->treeW_statscan->clear();
        qnode = new QTreeWidgetItem();
        qtemp = QString::fromStdString(sname);
        qnode->setText(0, qtemp);
        ui->treeW_statscan->addTopLevelItem(qnode);
        populateQtree(jt, qnode, sname);
        ui->treeW_statscan->expandAll();
        break;
    }
    case 1:  // Set year, display catalogues.
    {
        qtemp = qSelected[0]->text(0);
        syear = qtemp.toStdString();
        url = sc.urlYear(syear);
        webpage = wf.browse(url);
        slayer = jf.textParser(webpage, navSearch[1]);
        ilayer.assign(slayer.size(), -1);
        jt.addChildren(slayer, ilayer, syear);
        populateQtree(jt, qSelected[0], syear);
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
        slayer = { "CSV Data", "Geo Data" };
        ilayer.resize(2);
        temp = sroot + "\\" + syear + "\\" + sname;
        ilayer[0] = wf.get_file_path_number(temp, ".csv");
        temp += "\\" + sname + " geo list.bin";
        ilayer[1] = (int)wf.file_exist(temp);
        if (iyear >= 2011)
        {
            slayer.push_back("Map Data");
            temp = sroot + "\\" + syear + "\\" + sname + "\\maps";
            if (wf.file_exist(temp))
            {
                ipdf = wf.get_file_path_number(temp, ".pdf");
                ijpg = wf.get_file_path_number(temp, ".jpg");
                ilayer.push_back(max(ipdf, ijpg));
            }
            else
            {
                ilayer.push_back(0);
            }
        }
        jt.deleteChildren(sname);
        jt.addChildren(slayer, ilayer, sname);
        populateQtree(jt, qSelected[0], sname);
        qSelected[0]->setExpanded(1);
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
    jtx.listChildren(sparent, skids);
    if (skids.size() < 1) { return; }  // Leaf node.
    string temp = "CSV Data";
    if (skids[0] == temp)
    {
        jtx.listChildren(sparent, ikids);
        twig = 1;
    }
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

// Download the selected file(s).
void MainWindow::on_pB_download_clicked()
{
    jf.timerStart();
    int onlineTab = ui->tabW_online->currentIndex();
    QList<QTreeWidgetItem*> qlist, qChildren;
    vector<string> prompt, listCata;
    vector<vector<int>> comm(1, vector<int>());
    comm[0].assign(comm_length, 0);  // Form [control, progress report, size report, max param].
    thread::id myid = this_thread::get_id();
    
    qlist = ui->treeW_statscan->selectedItems();
    if (qlist.size() < 1) { return; }
    int kids, iStatusCata, error;
    int igen = 0;
    QTreeWidgetItem* pNode = nullptr;
    QTreeWidgetItem* pParent = qlist[0];
    do
    {
        pNode = pParent;
        pParent = pNode->parent();
        igen++;
    } while (pParent != nullptr);
    QString qtemp;
    switch (onlineTab)
    {
    case 0:
    {
        prompt.resize(3);  // Form [syear, sname, dlType].
        if (igen <= 2) 
        {
            qtemp = qlist[0]->text(0);
            prompt[0] = qtemp.toStdString();
            kids = qlist[0]->childCount();
            if (kids > 0)
            {
                listCata.resize(kids);
                for (int ii = 0; ii < kids; ii++)
                {
                    pNode = qlist[0]->child(ii);
                    qtemp = pNode->text(0);
                    listCata[ii] = qtemp.toStdString();
                }
            }
            else
            {
                string url = sc.urlYear(prompt[0]);
                string webpage = wf.browse(url);
                listCata = jf.textParser(webpage, navSearch[1]);
            }
        }
        else if (igen == 3) 
        { 
            qtemp = qlist[0]->parent()->text(0);
            prompt[0] = qtemp.toStdString();
            qtemp = qlist[0]->text(0);
            prompt[1] = qtemp.toStdString();
        }
        else 
        { 
            qtemp = qlist[0]->parent()->parent()->text(0);
            prompt[0] = qtemp.toStdString();
            qtemp = qlist[0]->parent()->text(0);
            prompt[1] = qtemp.toStdString();
        }
        
        if (listCata.size() < 1)
        {
            iStatusCata = sf.statusCata(prompt[1]);
            if (iStatusCata == 2)
            {
                qf.displayText(ui->QL_bar, prompt[1] + " is already present in the database.");
                return;
            }
            else { prompt[2] = to_string(iStatusCata); }
            sb.set_prompt(myid, prompt);
            std::thread dl(&MainWindow::downloader, this, ref(sb));
            dl.detach();
        }
        else
        {
            for (int ii = 0; ii < listCata.size(); ii++)
            {
                prompt[1] = listCata[ii];
                iStatusCata = sf.statusCata(prompt[1]);
                if (iStatusCata == 2)
                {
                    qf.displayText(ui->QL_bar, prompt[1] + " is already present in the database.");
                    continue;
                }
                else { prompt[2] = to_string(iStatusCata); }
                error = sb.start_call(myid, 1, comm[0]);
                if (error) { errnum("start_call-pB_download_clicked", error); }
                sb.set_prompt(myid, prompt);
                std::thread dl(&MainWindow::downloader, this, ref(sb));
                dl.detach();
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
                int bbq = 1;
            }
            return;
        }

        break;
    }
    case 1:
    {
        // NOTE: Display tree of existing maps.
        error = sb.start_call(myid, 1, comm[0]);
        if (error) { errnum("start_call-pB_download_clicked", error); }
        prompt.resize(1);  // Form [syear].
        prompt[0] = { "2016" };
        sb.set_prompt(myid, prompt);
        std::thread dl(&MainWindow::downloadMaps, this, ref(sb));
        dl.detach();
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
    
    long long timer = jf.timerStop();
    log("Downloaded catalogue " + prompt[1] + " in " + to_string(timer) + "ms");
}
void MainWindow::downloadMaps(SWITCHBOARD& sb)
{
    vector<int> mycomm;
    vector<vector<int>> comm_gui;
    thread::id myid = this_thread::get_id();
    sb.answer_call(myid, mycomm);
    vector<string> prompt = sb.get_prompt();  // Form [syear].
    sc.initGeo();
    string urlYear, pageYear, temp, urlCata, urlGeoList, geoPage; 
    string urlRegion, pageRegion, urlMap, pageMap, urlPDF, sPDF;
    string nameRegion, namePDF, namePNG, pathDirPDF, pathDirPNG, typeMap;
    vector<string> geoLayers, geoTemp, geoLinkNames, regionLink;
    vector<string> mapLink;
    vector<vector<string>> splitLinkNames;
    vector<string> badChar = { "/" };
    vector<string> goodChar = { "or" };
    size_t pos1, pos2;
    int iyear, spaces, indent;
    bool fileExist;
    if (prompt.size() == 1)
    {
        try { iyear = stoi(prompt[0]); }
        catch (out_of_range& oor) { err("stoi-MainWindow.downloadMaps"); }
        urlYear = sc.urlYear(prompt[0]);
        pageYear = wf.browse(urlYear);
        vector<string> listCata = jf.textParser(pageYear, navSearch[1]);
        mycomm[2] = listCata.size();
        sb.update(myid, mycomm);
        sort(listCata.begin(), listCata.end());
        for (int ii = 0; ii < listCata.size(); ii++)
        {
            temp = sc.urlCata(listCata[ii]);
            urlCata = wf.urlRedirect(temp);
            urlGeoList = sc.urlGeoList(iyear, urlCata);
            geoPage = wf.browse(urlGeoList);
            geoTemp = jf.textParser(geoPage, navSearch[6]);
            geoLayers = sc.makeGeoLayers(geoTemp[0]);
            geoLinkNames = jf.textParser(geoPage, navSearch[2]);
            vector<int> vIndents = { -1 };
            for (int jj = 0; jj < geoLinkNames.size(); jj++)
            {
                pos1 = geoLinkNames[jj].find('"');
                temp = geoLinkNames[jj].substr(0, pos1);
                try { spaces = stoi(temp); }
                catch (out_of_range& oor) { err("stoi-MainWindow.downloadMaps"); }
                for (int kk = 0; kk < vIndents.size(); kk++)
                {
                    if (vIndents[kk] == spaces)
                    {
                        indent = kk;
                        break;
                    }
                    else if (kk == vIndents.size() - 1)
                    {
                        indent = vIndents.size();
                        vIndents.push_back(spaces);
                    }
                }

                if (jj == 0 && indent > 0)
                {
                    geoLayers.insert(geoLayers.begin(), "Canada");
                }

                if (indent < geoLayers.size())
                {
                    typeMap = geoLayers[indent];
                }
                else
                {
                    typeMap = geoLayers[geoLayers.size() - 1];
                }

                if (typeMap == "ct")
                {
                    pos1 = geoLinkNames[jj].rfind('>') + 1;
                    pos2 = geoLinkNames[jj].find_first_of("0123456789", pos1);
                    if (pos2 == pos1)
                    {
                        pos2 = geoLinkNames[jj].rfind(')');
                        pos1 = geoLinkNames[jj].rfind('(', pos2) + 1;
                        nameRegion = geoLinkNames[jj].substr(pos1, pos2 - pos1);
                    }
                    else
                    {
                        pos2 = geoLinkNames[jj].rfind('"');
                        pos1 = geoLinkNames[jj].rfind('"', pos2 - 1) + 1;
                        nameRegion = geoLinkNames[jj].substr(pos1, pos2 - pos1);
                    }

                }
                else
                {
                    pos1 = geoLinkNames[jj].rfind('>') + 1;
                    nameRegion = geoLinkNames[jj].substr(pos1);
                }
                if (nameRegion == "Canada") { indent = 0; }
                jf.clean(nameRegion, badChar, goodChar);

                namePDF = sroot + "\\mapsPDF\\";
                namePNG = sroot + "\\mapsPNG\\";
                for (int kk = 0; kk < indent; kk++)
                {
                    if (kk < geoLayers.size() - 1)
                    {
                        namePDF += geoLayers[kk + 1] + "\\";
                        namePNG += geoLayers[kk + 1] + "\\";
                    }
                }
                pathDirPDF = namePDF;
                pathDirPDF.pop_back();
                namePDF += nameRegion + ".pdf";
                pathDirPNG = namePNG;
                pathDirPNG.pop_back();
                namePNG += nameRegion + ".png";

                fileExist = wf.file_exist(namePNG);
                if (!fileExist)
                {
                    // Download the PDF map.
                    wf.makeDir(pathDirPDF);
                    urlRegion = sc.geoLinkToRegionUrl(urlGeoList, geoLinkNames[jj]);
                    pageRegion = wf.browse(urlRegion);
                    regionLink = jf.textParser(pageRegion, navSearch[7]);
                    urlMap = sc.regionLinkToMapUrl(urlRegion, regionLink[0]);
                    pageMap = wf.browse(urlMap);
                    mapLink = jf.textParser(pageMap, navSearch[8]);
                    urlPDF = sc.mapLinkToPDFUrl(urlMap, mapLink[0]);
                    wf.download(urlPDF, namePDF);

                    // Convert the PDF map to a PNG map.
                    wf.makeDir(pathDirPNG);
                    gf.pdfToPng(namePDF, namePNG);
                }
            }
            mycomm[1]++;
            sb.update(myid, mycomm);
        }
        mycomm[0] = 1;
        sb.update(myid, mycomm);
    }
}
void MainWindow::downloader(SWITCHBOARD& sb)
{
    vector<int> mycomm;
    vector<vector<int>> comm_gui;
    thread::id myid = this_thread::get_id();
    sb.answer_call(myid, mycomm);
    vector<string> prompt = sb.get_prompt();  // Form [syear, sname, dlType].
    int iyear, dlMode, yearMode;
    try { iyear = stoi(prompt[0]); }
    catch (out_of_range& oor) { err("stoi-MainWindow.downloader"); }
    if (iyear <= 2006) { yearMode = 0; }
    else if (iyear <= 2013) { yearMode = 1; }
    else if (iyear <= 2017) { yearMode = 2; }
    if (prompt[2] == "Catalogue") { dlMode = 0; }

    string temp, urlCata, urlGeoList, geoPage;
    temp = sc.urlCata(prompt[1]);
    urlCata = wf.urlRedirect(temp);
    urlGeoList = sc.urlGeoList(iyear, urlCata);
    prompt.push_back(urlGeoList);
    geoPage = wf.browse(urlGeoList);
    vector<string> geoLinkNames = jf.textParser(geoPage, navSearch[2]);
    vector<vector<string>> splitLinkNames = sc.splitLinkNames(geoLinkNames);  // Form [region index][indentation, url object, region name, gid].
    unordered_map<string, int> mapGeoIndent;
    vector<string> geoLayerCodes;
    temp = sroot + "\\" + prompt[0] + "\\" + prompt[1];
    wf.makeDir(temp);
    switch (yearMode)  // Define a task as 10 CSV downloads, or 1 geo list download.
    {  
    case 0:
    {
        mycomm[2] = (splitLinkNames.size() / 10) + 1;
        break;
    }
    case 1:
    {
        sc.initGeo();
        geoLayerCodes = sc.getLayerSelected(geoPage);
        mycomm[2] = 2 * ((splitLinkNames.size() / 10) + 1) + 1;
        break;
    }
    case 2:
    {
        sc.initGeo();
        geoLayerCodes = sc.getLayerSelected(geoPage);
        mycomm[2] = 2 * ((splitLinkNames.size() / 10) + 1) + 1;
        break;
    }
    }
    sb.update(myid, mycomm);

    for (int ii = 0; ii < splitLinkNames.size(); ii++)
    {
        dlCSV(splitLinkNames, prompt, ii);
        if (ii % 10 == 9 || ii == splitLinkNames.size() - 1)
        {
            mycomm[1]++;
            comm_gui = sb.update(myid, mycomm);
            if (comm_gui[0][0] == 2)
            {
                mycomm[0] = -2;
                sb.update(myid, mycomm);
                return;
            }
        }
    }
    dlGeo(splitLinkNames, prompt, mapGeoIndent);
    mycomm[1]++;
    mycomm[0] = 1;
    sb.update(myid, mycomm);

    int bbq = 1;
}
void MainWindow::dlCSV(vector<vector<string>>& sLN, vector<string>& prompt, int indexCSV)
{
    size_t pos1 = prompt[3].rfind('/') + 1;
    string urlServer = prompt[3].substr(0, pos1);
    string urlCSV, pageCSV, fileCSV, urlDL, urlDL0, pathCSV, pageDL;
    vector<string> vstemp;
    int iyear = stoi(prompt[0]);
    urlCSV = urlServer + sLN[indexCSV][1];
    pathCSV = sroot + "\\" + prompt[0] + "\\" + prompt[1] + "\\" + prompt[1];
    pathCSV += " (" + sLN[indexCSV][3] + ") " + sLN[indexCSV][2] + ".csv";
    if (wf.file_exist(pathCSV)) { return; }
    pageCSV = wf.browse(urlCSV);
    if (iyear <= 2006)
    {
        vstemp = jf.textParser(pageCSV, navSearch[3]);
        sc.cleanURL(vstemp[0]);
        urlDL = urlServer + vstemp[0];
        fileCSV = wf.browse(urlDL);
        jf.printer(pathCSV, fileCSV);
    }
    else if (iyear >= 2011)
    {
        vstemp = jf.textParser(pageCSV, navSearch[4]);
        sc.cleanURL(vstemp[0]);
        urlDL0 = urlServer + vstemp[0];
        pageDL = wf.browse(urlDL0);
        vstemp = jf.textParser(pageDL, navSearch[5]);
        sc.cleanURL(vstemp[0]);
        urlDL = urlServer + vstemp[0];
        fileCSV = wf.browse(urlDL);
        jf.printer(pathCSV, fileCSV);
    }
}
string MainWindow::dlGeo(vector<vector<string>>& sLN, vector<string>& prompt, unordered_map<string, int>& mapGeoIndent)
{
    string temp, sgeo;
    size_t pos1, pos2;
    int inum, indent, spaces;
    vector<int> vIndents = { -1 };
    for (int ii = 0; ii < sLN.size(); ii++)
    {
        for (int jj = 0; jj < vIndents.size(); jj++)
        {
            if (vIndents[jj] == spaces)
            {
                indent = jj;
                break;
            }
            else if (jj == vIndents.size() - 1)
            {
                indent = vIndents.size();
                vIndents.push_back(spaces);
            }
        }
        if (sLN[ii][2] == "Canada") { indent = 0; }
        mapGeoIndent.emplace(sLN[ii][2], indent);
        temp = sLN[ii][3] + "$" + sLN[ii][2] + "$" + to_string(indent) + "\n";
        sgeo.append(temp);
    }
    temp = sroot + "\\" + prompt[0] + "\\" + prompt[1] + "\\" + prompt[1] + " geo list.bin";
    jf.printer(temp, sgeo);
    return sgeo;
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
    string tname = qtemp.toStdString();
    vector<string> results;
    QStringList qlist;
    if (sf.table_exist(tname))
    {
        display_table(tname);
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
    string pathFolder, search;
    string nameRoot = "Local Maps";
    QString qtemp;
    vector<vector<int>> tree_st;
    vector<int> tree_ipl;
    vector<string> tree_pl;
    QTreeWidgetItem* qnode = nullptr;
    QList<QTreeWidgetItem*> qfolders;
    unordered_map<QString, int> mapFolders;
    int numRoots = ui->treeW_maps->topLevelItemCount();
    if (numRoots == 0)
    {
        // Populate the QTree with the map folders/subfolders.
        jtMaps.init(nameRoot);
        pathFolder = sroot + "\\mapsPDF";
        search = "*";
        wf.make_tree_local(tree_st, tree_pl, 1, pathFolder, 5, search);
        tree_ipl.assign(tree_st.size(), 0);  // All folders, so iname = 0.
        //jtMaps.inputTreeSTPL(tree_st, tree_pl, tree_ipl);
        qnode = new QTreeWidgetItem();
        qtemp = QString::fromUtf8(nameRoot);
        qnode->setText(0, qtemp);
        ui->treeW_maps->addTopLevelItem(qnode);
        qfolders.append(qnode);
        populateQtreeList(jtMaps, qnode, nameRoot, qfolders);
        for (int ii = 0; ii < qfolders.size(); ii++)
        {
            mapFolders.emplace(qfolders[ii]->text(0), ii);
        }

        // Add the BIN map files to the QTree.
        pathFolder = sroot + "\\mapsBIN\\*.bin";
        int bbq = 1;

    }
}
void MainWindow::populateQtreeList(JTREE& jtx, QTreeWidgetItem*& qparent, string sparent, QList<QTreeWidgetItem*>& qlist)
{
    bool twig = 0;
    bool maps = 0;
    vector<int> ikids;
    vector<string> skids;
    jtx.listChildren(sparent, skids);
    if (skids.size() < 1) { return; }  // Leaf node.
    string temp = "CSV Data";
    if (skids[0] == temp)
    {
        jtx.listChildren(sparent, ikids);
        twig = 1;
    }
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
        qlist.append(qkid);
    }
    for (int ii = 0; ii < skids.size(); ii++)
    {
        populateQtreeList(jtx, qkids[ii], skids[ii], qlist);
    }
}

// (Debug function) Perform a test function.
// 0 = download given webpage
// 1 = single file PNG->BIN,  2 = contents of folder PNG->BIN.
// 3 = display BIN map.
void MainWindow::on_pB_test_clicked()
{
    string temp, pathPNG, pathBIN, dirPNG, dirBIN;
    vector<string> prompt, dirt, soap, slist;
    vector<vector<double>> pathBorder, pathBorderShared, pathBorderSegment;
    vector<double> mapShift;
    int error, inum, coordX, coordY;
    bool frameDone = 0;
    QPainterPath painterPathBorder;
    vector<std::thread> tapestry;
    thread::id myid = this_thread::get_id();
    vector<vector<int>> comm(1, vector<int>());
    comm[0].assign(comm_length, 0);  // Form [control, progress report, size report, max param].

    int mode = 1;
    //dirPNG = sroot + "\\mapsPNG\\cmaca";
    //dirBIN = sroot + "\\mapsBIN\\cmaca";
    pathPNG = sroot + "\\mapsPNG\\province\\British Columbia or Colombie-Britannique.png";
    pathBIN = sroot + "\\mapsBIN\\province\\British Columbia or Colombie-Britannique.bin";
    qf.initPixmap(ui->label_maps);
    switch (mode)
    {
    case 0:
    {
        QString qtemp = ui->pte_webinput->toPlainText();
        string url = qtemp.toStdString();
        string webpage = wf.browse(url);
        jf.printer(sroot + "\\Test webpage.txt", webpage);
        return;
    }
    case 1:
    {
        prompt.resize(3);  // Form [pathPNG, pathBIN, "w,h"].
        prompt[0] = pathPNG;
        prompt[1] = pathBIN;
        if (pathPNG.size() < 1)
        {
            reset_bar(100, "No PNG path specified.");
            return;
        }
        else if (pathBIN.size() < 1)
        {
            prompt[1] = pathPNG;
            dirt = { "PNG", ".png" };
            soap = { "BIN", ".bin" };
            jf.clean(prompt[1], dirt, soap);       
        }
        inum = ui->label_maps->width();
        prompt[2] = to_string(inum) + ",";
        inum = ui->label_maps->height();
        prompt[2] += to_string(inum);
        error = sb.start_call(myid, cores, comm[0]);
        if (error) { errnum("start_call-MainWindow.on_pB_test_clicked", error); }
        sb.set_prompt(myid, prompt);
        std::thread dl(&IMGFUNC::pngToBinLive, ref(im), ref(sb), ref(pathBorderShared));
        dl.detach();
        reset_bar(100, "Converting " + pathPNG);
        break;
    }
    case 2:
    {
        prompt.resize(3);
        if (dirPNG.size() < 1)
        {
            reset_bar(100, "No PNG folder directory specified.");
            return;
        }
        else if (dirBIN.size() < 1)
        {
            dirBIN = dirPNG;
            dirt = { "PNG", ".png" };
            soap = { "BIN", ".bin" };
            jf.clean(dirBIN, dirt, soap);
        }
        if (!wf.file_exist(dirPNG)) { wf.makeDir(dirPNG); }
        if (!wf.file_exist(dirBIN)) { wf.makeDir(dirBIN); }
        inum = ui->label_maps->width();
        prompt[2] = to_string(inum) + ",";
        inum = ui->label_maps->height();
        prompt[2] += to_string(inum);
        slist = wf.get_file_list(dirPNG, "*.png");
        for (int ii = 0; ii < slist.size(); ii++)
        {
            prompt[0] = dirPNG + "\\" + slist[ii];
            prompt[1] = dirBIN + "\\" + slist[ii];
            dirt = { "PNG", ".png" };
            soap = { "BIN", ".bin" };
            jf.clean(prompt[1], dirt, soap);
            error = sb.start_call(myid, cores, comm[0]);
            if (error) { errnum("start_call-MainWindow.on_pB_test_clicked", error); }
            sb.set_prompt(myid, prompt);
            std::thread dl(&IMGFUNC::pngToBinLive, ref(im), ref(sb), ref(pathBorderShared));
            dl.detach();
            reset_bar(100, "Converting " + prompt[0]);
        }
        reset_bar(100, prompt[1] + " done!");
        
        break;
    }
    case 3:
    {
        qf.displayBin(ui->label_maps, pathBIN);
        return;
    }
    case 4:
    {
        prompt.resize(3);  // Form [pathPNG, pathBIN, "w,h"].
        prompt[0] = pathPNG;
        prompt[1] = pathBIN;
        if (pathPNG.size() < 1)
        {
            reset_bar(100, "No PNG path specified.");
            return;
        }
        else if (pathBIN.size() < 1)
        {
            prompt[1] = pathPNG;
            dirt = { "PNG", ".png" };
            soap = { "BIN", ".bin" };
            jf.clean(prompt[1], dirt, soap);
        }
        inum = ui->label_maps->width();
        prompt[2] = to_string(inum) + ",";
        inum = ui->label_maps->height();
        prompt[2] += to_string(inum);
        error = sb.start_call(myid, cores, comm[0]);
        if (error) { errnum("start_call-MainWindow.on_pB_test_clicked", error); }
        sb.set_prompt(myid, prompt);
        std::thread dl(&IMGFUNC::pngToBinLiveDebug, ref(im), ref(sb), ref(pathBorderShared));
        dl.detach();
        reset_bar(100, "Converting " + pathPNG);
        break;
    }
    }

    string sMessage;
    QString qMessage;
    vector<int> dotColours = { 1, 3, 4, 5 };
    vector<vector<int>> dots;
    vector<vector<vector<double>>> importDouble;
    QPixmap pmDebug;
    QImage imgDebug;
    string pathImg = sroot + "\\debug\\tempDebug.png";
    if (mode == 4)
    {
        while (1)  // Manager thread has binary data for GUI thread.
        {
            Sleep(50);
            comm = sb.update(myid, comm[0]);
            if (comm[1][0] == 3)
            {
                qf.displayDebug(ui->label_maps, pathImg);
                QCoreApplication::processEvents();
                int bbq = 1;
            }

        }
    }
    // RESUME HERE. Make a permanent launch/receive function for PNG->BIN.
    while (1)
    {
        Sleep(gui_sleep);
        comm = sb.update(myid, comm[0]);
        if (comm[1][0] == 3)
        {
            qf.displayDebug(ui->label_maps, pathImg);
            while (1)
            {
                QCoreApplication::processEvents();
                Sleep(50);
                int bbq = 1;
            }
        }
        error = sb.pull(myid, 0);
        if (error < 0) { err("sb.pull-MainWindow.on_pB_test"); }
        if (!frameDone)
        {
            comm = sb.update(myid, comm[0]);
            while (comm[1][1] == 0 && comm[1][0] == 0)
            {
                Sleep(10);
                comm = sb.update(myid, comm[0]);
            }
            mapShift.resize(3);            // Form [Dx, Dy, stretchFactor].
            mapShift[0] = -1.0 * pathBorderShared[0][0];
            mapShift[1] = -1.0 * pathBorderShared[0][1];
            mapShift[2] = pathBorderShared[4][0];
            pathBorderShared.clear();
            frameDone = 1;
            continue;
        }
        pathBorder = pathBorderShared;
        sb.done(myid);
        if (pathBorder.size() < 1) { continue; }
        im.coordShift(pathBorder, mapShift);
        painterPathBorder = qf.pathMake(pathBorder);
        if (pathBorder.size() >= 4)
        {
            pathBorderSegment.assign(pathBorder.begin() + pathBorder.size() - 4, pathBorder.begin() + pathBorder.size());
        }
        else
        {
            pathBorderSegment.assign(pathBorder.begin(), pathBorder.end());
        }
        dots = qf.dotsMake(pathBorderSegment, dotColours);
        qf.displayPainterPathDots(ui->label_maps, painterPathBorder, dots);
        sMessage = to_string(pathBorder.size()) + " border points shown.\n";
        for (int ii = 0; ii < pathBorderSegment.size(); ii++)
        {
            coordX = int(pathBorder[pathBorder.size() - 1 - ii][0]);
            coordY = int(pathBorder[pathBorder.size() - 1 - ii][1]);
            sMessage += "Last-" + to_string(ii) + ": (" + to_string(coordX);
            sMessage += "," + to_string(coordY) + ")\n";
        }
        qMessage = QString::fromStdString(sMessage);
        ui->label_maps2->setText(qMessage);
        QCoreApplication::processEvents();
        if (comm[1][0] == 1 || comm[1][0] == -2)
        {
            QCoreApplication::processEvents();
            error = sb.end_call(myid);
            if (error) { errnum("sb.end_call-on_pB_insert", error); }
            jobs_done = comm[0][2];
            update_bar();
            break;           // Manager reports task finished/cancelled.
        }
    }



    //string dirPDF = sroot + "\\mapsPDF\\province\\cmaca";
    //gf.folderConvert(dirPDF);

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
    }
    else
    {
        ui->pB_viewtable->setEnabled(0);
    }
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
        break;
    case 1:
        ui->pB_download->setEnabled(1);
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
}
void MainWindow::on_treeW_maps_itemSelectionChanged() 
{

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
