#include "mainwindow.h"
#include "ui_mainwindow.h"

using namespace std;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->cB_drives->addItem("D:");
    ui->cB_drives->addItem("E:");
    ui->cB_drives->addItem("F:");
    ui->cB_drives->addItem("G:");
    ui->progressBar->setValue(0);
    ui->QL_bar->setText("");

    initialize();
}

MainWindow::~MainWindow()
{
    delete ui;
}


// MULTI-PURPOSE FUNCTIONS:

// Perform a variety of tasks from the MainWindow constructor:
// 1. Connect the database memory object with a pre-existing local database file, if it exists.
// 2. Display (in tree form) the database's current repository of catalogues on the GUI.
// 3. Begin a new process log for this runtime session.
void MainWindow::initialize()
{
    db_path = sroots[location] + "\\SCDA.db";
    int error = sqlite3_open_v2(db_path.c_str(), &db, (SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE), NULL);
    if (error) { sqlerror("open-initialize", db); }
    
    //q.exec(QStringLiteral("PRAGMA journal_mode=WAL"));

    create_cata_index_table();  // Even if starting a new database, this meta-table will always exist.
    update_cata_tree();  // The cata_tree matrix is kept in QString form, given its proximity to the GUI.
    build_ui_tree(cata_tree, 2);
    vector<mutex> dummy(cores);
    m_jobs.swap(dummy);

    reset_bar(100, " ");
    clear_log();
    log("MainWindow initialized.");
}

// Populate a 2D tree containing the names of all catalogues in the database. Form [year][year, catalogues...].
// Also, populate a map connecting qyear->year_index  and  qname->cata_index.
void MainWindow::all_cata_db(QVector<QVector<QString>>& ntree, QMap<QString, int>& map_tree)
{
    QString qyear, qname, temp;
    int year_index, cata_index;
    string stmt = "SELECT Year, Name FROM TCatalogueIndex;";
    int error = sqlite3_prepare_v2(db, stmt.c_str(), -1, &statement, NULL);
    if (error) { sqlerror("prepare-all_cata_db", db); }
    vector<vector<string>> results = step(db);    
    for (int ii = 0; ii < results.size(); ii++)
    {
        qyear = QString::fromStdString(results[ii][0]);
        qname = QString::fromStdString(results[ii][1]);
        qname.chop(1);
        qname.remove(0, 1);
        year_index = map_tree.value(qyear, -1);
        if (year_index < 0)
        {
            year_index = ntree.size();
            map_tree.insert(qyear, year_index);
            ntree.append(QVector<QString>());
            ntree[year_index].append(qyear);
        }
        cata_index = ntree[year_index].size();
        map_tree.insert(qname, cata_index);
        ntree[year_index].append(qname);
    }
}

// Threadsafe error log function specific to SQL errors.
void MainWindow::sqlerror(string func, sqlite3*& dbnoqt)
{
    int errcode = sqlite3_errcode(dbnoqt);
    const char* errmsg = sqlite3_errmsg(dbnoqt);
    string serrmsg(errmsg);
    string path = sroots[location] + "\\SCDAnoqt Error Log.txt";
    string message = timestamperA() + " SQL ERROR #" + to_string(errcode) + ", in ";
    message += func + "\r\n" + serrmsg + "\r\n\r\n";
    lock_guard<mutex> lock(m_err);
    HANDLE hprinter = CreateFileA(path.c_str(), (GENERIC_READ | GENERIC_WRITE), (FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE), NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hprinter == INVALID_HANDLE_VALUE)
    {
        cout << "sqlerror CreateFileA failure" << endl;
        system("pause");
    }
    SetFilePointer(hprinter, NULL, NULL, FILE_END);
    DWORD bytes;
    DWORD fsize = (DWORD)message.size();
    if (WriteFile(hprinter, message.c_str(), fsize, &bytes, NULL))
    {
        cout << "sqlerror WriteFile failure" << endl;
        system("pause");
    }
    CloseHandle(hprinter);
    exit(EXIT_FAILURE);
}

// Delete the previous runtime's log file.
void MainWindow::clear_log()
{
    string name = utf16to8(wroots[location]) + "\\SCDA Process Log.txt";
    HANDLE hprinter = CreateFileA(name.c_str(), (GENERIC_READ | GENERIC_WRITE), (FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE), NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hprinter == INVALID_HANDLE_VALUE) { warn(L"CreateFile-refresh_log"); }
    if (!DeleteFileA(name.c_str())) { warn(L"DeleteFile-refresh_log"); }
    if (!CloseHandle(hprinter)) { warn(L"CloseHandle-refresh_log"); }
}

// Progress bar related functions.
void MainWindow::update_bar()
{
    int jump, old;
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

// Functions for the meta-table 'TCatalogueIndex'.
void MainWindow::create_cata_index_table()
{
    string stmt = "CREATE TABLE IF NOT EXISTS TCatalogueIndex (Year TEXT, ";
    stmt += "Name TEXT, Description TEXT)";
    int error = sqlite3_prepare_v2(db, stmt.c_str(), -1, &statement, NULL);
    if (error) { sqlerror("prepare-create_cata_index_table", db); }
    step(db);
}

// For a prepared matrix of QString data, display it on the GUI as a 2D tree widget (year -> cata entry).
void MainWindow::build_ui_tree(QVector<QVector<QVector<QString>>>& qtree, int window)
{
    QTreeWidgetItem *item;
    QVector<QTreeWidgetItem*> qroots;
    switch (window)  // Window:  1 = Catalogues on Drive, 2 = Catalogues in Database
    {
    case 1:
        ui->TW_cataondrive->clear();
        for (int ii = 0; ii < qtree.size(); ii++)
        {
            item = new QTreeWidgetItem(ui->TW_cataondrive);  // For every year, make a root pointer.
            item->setText(0, qtree[ii][0][0]);
            item->setText(1, " ");
            auto item_flags = item->flags();
            item_flags.setFlag(Qt::ItemIsSelectable, false);
            item->setFlags(item_flags);
            qroots.append(item);
            add_children(item, qtree[ii]);
        }
        ui->TW_cataondrive->addTopLevelItems(qroots);
        break;
    case 2:
        ui->TW_cataindb->clear();
        for (int ii = 0; ii < qtree.size(); ii++)
        {
            item = new QTreeWidgetItem(ui->TW_cataindb);  // For every year, make a root pointer.
            item->setText(0, qtree[ii][0][0]);
            item->setText(1, " ");
            item->setText(2, " ");
            auto item_flags = item->flags();
            item_flags.setFlag(Qt::ItemIsSelectable, false);
            item->setFlags(item_flags);
            qroots.append(item);
            add_children(item, qtree[ii]);
        }
        ui->TW_cataindb->addTopLevelItems(qroots);
        break;
    }
}
void MainWindow::add_children(QTreeWidgetItem* parent, QVector<QVector<QString>>& cata_list)
{
    QList<QTreeWidgetItem*> branch;
    QTreeWidgetItem *item;
    for (int ii = 0; ii < cata_list.size(); ii++)  // For every catalogue in the list...
    {
        item = new QTreeWidgetItem();
        for (int jj = 0; jj < cata_list[ii].size(); jj++)  // For every column we want to display...
        {
            item->setText(jj, cata_list[ii][jj]);
        }
        branch.append(item);
    }
    parent->addChildren(branch);
}

// For a given CSV qfile, extract the Stats Canada 'text variables'.
void MainWindow::update_text_vars(QVector<QVector<QString>>& text_vars, QString& qf)
{
    int pos1, pos2;
    int var_index = -1;
    QChar math;
    QString temp1, temp2;
    pos1 = 0;
    pos2 = 0;
    while (1)
    {
        pos1 = qf.indexOf('=', pos1 + 1);
        if (pos1 < 0) { break; }  // Primary loop exit.
        math = qf[pos1 - 1];
        if (math == '<' || math == '>') { continue; }

        var_index++;
        pos2 = qf.lastIndexOf('"', pos1); pos2++;
        temp1 = qf.mid(pos2, pos1 - pos2);
        clean(temp1, 0);
        if (temp1 != text_vars[var_index][0])
        {
            err("text variable type check-update_text_vars");
        }

        pos2 = qf.indexOf('"', pos1);
        temp1 = qf.mid(pos1 + 1, pos2 - pos1 - 1);
        clean(temp1, 1);
        text_vars[var_index][1] = temp1;
    }
}

// Update the catalogue tree and the tree's maps by reading from the database.
void MainWindow::update_cata_tree()
{
    cata_tree.clear();
    map_tree_year.clear();
    map_tree_cata.clear();
    string stmt = "SELECT DISTINCT Year FROM TCatalogueIndex;";
    int error = sqlite3_prepare_v2(db, stmt.c_str(), -1, &statement, NULL);
    if (error) { sqlerror("prepare1-update_cata_tree", db); }
    vector<vector<string>> dyear = step(db);  // Column vector containing a list of distinct years in the db.

    // Create the map connecting qyear to tree's first index.
    int tree_index, name_index, year_index, desc_index;
    QVector<QString> temp;
    QString qname, qyear, qdesc;
    year_index = 0;
    for (int ii = 0; ii < dyear.size(); ii++)
    {
        qyear = QString::fromStdString(dyear[ii][0]);
        map_tree_year.insert(qyear, year_index);
        cata_tree.append(QVector<QVector<QString>>());
        year_index++;
    }

    // Populate the tree, mapping catalogue names as we go.
    stmt = "SELECT * FROM TCatalogueIndex;";
    int error = sqlite3_prepare_v2(db, stmt.c_str(), -1, &statement, NULL);
    if (error) { sqlerror("prepare2-update_cata_tree", db); }
    vector<vector<string>> TCI = step(db);    
    for (int ii = 0; ii < TCI.size(); ii++)                          // For each catalogue in the database...
    {
        qyear = QString::fromStdString(TCI[ii][0]);
        qname = QString::fromStdString(TCI[ii][1]);
        qdesc = QString::fromStdString(TCI[ii][2]);
        temp = { qyear, qname, qdesc };
        tree_index = map_tree_year.value(qyear);
        map_tree_cata.insert(qname, cata_tree[tree_index].size());
        cata_tree[tree_index].append(temp);
    }
    log("Updated the cata_tree");
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

// TASK FUNCTIONS, USED BY ONE OR MORE GUI BUTTONS:

// For a given incomplete catalogue, determine its existing CSV entries and return a list of missing GIDs.
vector<string> MainWindow::scan_incomplete_cata(string syear, string sname)
{
    // Determine which CSVs are already present within the database.
    string stmt = "SELECT GID FROM [" + sname + "];";
    int error = sqlite3_prepare_v2(db, stmt.c_str(), -1, &statement, NULL);
    if (error) { sqlerror("prepare-scan_incomplete_cata", db); }
    vector<vector<string>> gid_haves = step(db);

    // RESUME HERE. Just finished making extract_gids().

    QVector<QString> gid_want_list = cata.get_gid_list();
    QString gid;
    QSqlQuery q(db);
    bool fine = q.prepare(stmt);
    if (!fine) { sqlerr("prepare-resume_incomplete_cata", q.lastError()); }
    executor(q);
    //if (!fine) { sqlerr("exec-resume_incomplete_cata", q.lastError()); }
    while (q.next())
    {
        gid = q.value(0).toString();
        gid_have_list.append(gid);
    }
    for (int ii = 0; ii < gid_have_list.size(); ii++)
    {
        gid = gid_have_list[ii];
        for (int jj = 0; jj < gid_want_list.size(); jj++)
        {
            if (gid_want_list[jj] == gid)
            {
                gid_want_list.remove(jj, 1);
                break;
            }
        }
    }
    cata.set_gid_want_list(gid_want_list);
    logger("Catalogue " + cata.get_qname() + " was scanned for missing CSVs.");
}

// Execute a prepared statement, and return values if applicable.
vector<vector<string>> MainWindow::step(sqlite3*& dbnoqt)
{
    int type, col_count, size;  // Type: 1(int), 2(double), 3(string)
    int error = sqlite3_step(statement);
    int ivalue;
    double dvalue;
    string svalue;
    vector<vector<string>> results;

    while (error == 100)
    {
        col_count = sqlite3_column_count(statement);
        results.push_back(vector<string>(col_count));
        for (int ii = 0; ii < col_count; ii++)
        {
            type = sqlite3_column_type(statement, ii);
            switch (type)
            {
            case 1:
                ivalue = sqlite3_column_int(statement, ii);
                results[results.size() - 1][ii] = to_string(ivalue);
                break;
            case 2:
                dvalue = sqlite3_column_double(statement, ii);
                results[results.size() - 1][ii] = to_string(dvalue);
                break;
            case 3:
                size = sqlite3_column_bytes(statement, ii);
                char* buffer = (char*)sqlite3_column_text(statement, ii);
                svalue.assign(buffer, size);
                results[results.size() - 1][ii] = svalue;
                break;
            }
        }
        error = sqlite3_step(statement);
    }
    if (error > 0 && error != 101)
    {
        sqlerror("step: " + to_string(error), dbnoqt);
    }
    return results;
}

// Functions related to the insertion of data into the database.
void MainWindow::judicator(int& control, int& report, string syear, string sname)
{
    string temp = sroots[location] + "\\" + syear + "\\" + sname;
    wstring cata_wpath = utf8to16(temp);
    int num_gid = get_file_path_number(cata_wpath, L".csv");
    int workload = num_gid / cores;
    int bot = 0;
    int top = workload - 1;
    reset_bar(num_gid, "Adding spreadsheets to catalogue " + sname);
    vector<string> param(3);

    // Establish the thread's connection to the database.
    sqlite3* dbjudi;
    int error = sqlite3_open_v2(db_path.c_str(), &dbjudi, (SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE), NULL);
    if (error) { sqlerror("open-judicator", dbjudi); }
    sqlite3_stmt* statejudi;

    // Insert this catalogue into the catalogue index.
    string stmt = "INSERT INTO TCatalogueIndex ( Year, Name, Description ) VALUES (?, ?, ?);";
    param[0] = syear;
    param[1] = "[" + sname + "]";
    param[2].assign("Incomplete");
    bind(stmt, param);
    error = sqlite3_prepare_v2(dbjudi, stmt.c_str(), -1, &statejudi, NULL);
    if (error)
    {
        sqlerror("prepare1-judicator", dbjudi);
    }
    step(dbjudi);

    // Create a table for this catalogue's damaged CSVs.
    temp = "[" + sname + "$Damaged]";
    stmt = "CREATE TABLE IF NOT EXISTS " + temp + " (GID NUMERIC, [Number of Missing Data Entries] NUMERIC);";
    error = sqlite3_prepare_v2(dbjudi, stmt.c_str(), -1, &statejudi, NULL);
    if (error) { sqlerror("prepare2-judicator_noqt", dbjudi); }
    step(dbjudi);

    // Launch the worker threads, which will iterate through the CSVs.
    vector<std::thread> peons;
    vector<int> controls;
    controls.assign(cores, 0);
    vector<int> reports;
    reports.assign(cores, 0);
    vector<int> prompt(3);  // Form [id, bot, top]
    vector<vector<string>> all_queue(cores, vector<string>());  // Form [thread][statements]
    for (int ii = 0; ii < cores; ii++)
    {
        prompt[0] = ii;
        prompt[1] = bot;
        prompt[2] = top;
        std::thread thr(&MainWindow::insert_csvs, this, std::ref(all_queue[ii]), std::ref(controls[ii]), std::ref(reports[ii]), cata_wpath, prompt);
        peons.push_back(std::move(thr));
        bot += workload;
        if (ii < cores - 1) { top += workload; }
        else { top = num_gid - 1; }
    }

    // Create the catalogue's primary table.
    while (all_queue[0].size() < 2) { Sleep(50); }  // Wait for the first worker thread to finish its unique task.
    string cata_desc = all_queue[0][1];  // Used later.
    m_jobs[0].lock();
    stmt = all_queue[0][0];
    all_queue[0].erase(all_queue[0].begin(), all_queue[0].begin() + 2);
    m_jobs[0].unlock();
    error = sqlite3_prepare_v2(dbjudi, stmt.c_str(), -1, &statejudi, NULL);
    if (error) { sqlerror("prepare3-judicator", dbjudi); }
    step(dbjudi);

    // Loop through the worker threads, inserting their statements into the database.
    int active_thread = 0;
    int inert_threads = 0;
    int pile, progress, num1, num2, num3;
    vector<string> desk;
    while (control == 0 && jobs_done < jobs_max)  // Stop working if a 'cancel' signal is sent, or if the task is done.
    {
        progress = 0;
        for (int ii = 0; ii < cores; ii++)
        {
            progress += reports[ii];
        }
        report = progress;
        pile = all_queue[active_thread].size();
        if (pile > 0)
        {
            m_jobs[active_thread].lock();
            desk = all_queue[active_thread];
            all_queue[active_thread].clear();
            m_jobs[active_thread].unlock();
            error = sqlite3_exec(dbjudi, "BEGIN EXCLUSIVE TRANSACTION", NULL, NULL, NULL);
            if (error) { sqlerror("begin transaction1-judicator", dbjudi); }
            for (int ii = 0; ii < desk.size(); ii++)
            {
                error = sqlite3_prepare_v2(dbjudi, desk[ii].c_str(), -1, &statejudi, NULL);
                if (error)
                {
                    sqlerror("prepare4-judicator", dbjudi);
                }
                step(dbjudi);
            }
            error = sqlite3_exec(dbjudi, "COMMIT TRANSACTION", NULL, NULL, NULL);
            if (error) { sqlerror("commit transaction1-judicator", dbjudi); }
        }
        active_thread++;
        if (active_thread >= cores) { active_thread = 0; }
    }
    if (control == 1)  // Before shutting down, finish inserting the CSVs that were in queue.
    {
        for (size_t ii = 0; ii < controls.size(); ii++)  // Alert the worker threads to stop.
        {
            controls[ii] = 1;
        }
        active_thread = 0;
        while (inert_threads < cores)  // Keep working while the threads finish their final tasks.
        {
            inert_threads = 0;
            for (int ii = 0; ii < cores; ii++)  // Count how many worker threads have stopped.
            {
                if (controls[ii] < 0)
                {
                    inert_threads++;
                }
            }
            pile = all_queue[active_thread].size();
            if (pile > 0)
            {
                m_jobs[active_thread].lock();
                desk = all_queue[active_thread];
                all_queue[active_thread].clear();
                m_jobs[active_thread].unlock();
                error = sqlite3_exec(dbjudi, "BEGIN EXCLUSIVE TRANSACTION", NULL, NULL, NULL);
                if (error) { sqlerror("begin transaction2-judicator", dbjudi); }
                for (int ii = 0; ii < desk.size(); ii++)
                {
                    error = sqlite3_prepare_v2(dbjudi, desk[ii].c_str(), -1, &statejudi, NULL);
                    if (error) { sqlerror("prepare5-judicator", dbjudi); }
                    step(dbjudi);
                }
                error = sqlite3_exec(dbjudi, "COMMIT TRANSACTION", NULL, NULL, NULL);
                if (error) { sqlerror("commit transaction2-judicator", dbjudi); }
            }
            active_thread++;
            if (active_thread >= cores) { active_thread = 0; }
        }
        for (int ii = 0; ii < cores; ii++)  // When all worker threads are done, do one final sweep of the queue.
        {
            pile = all_queue[ii].size();
            if (pile > 0)
            {
                m_jobs[ii].lock();
                desk = all_queue[ii];
                all_queue[ii].clear();
                m_jobs[ii].unlock();
                error = sqlite3_exec(dbjudi, "BEGIN EXCLUSIVE TRANSACTION", NULL, NULL, NULL);
                if (error) { sqlerror("begin transaction3-judicator", dbjudi); }
                for (int jj = 0; jj < desk.size(); jj++)
                {
                    stmt = desk[jj];
                    error = sqlite3_prepare_v2(dbjudi, stmt.c_str(), -1, &statejudi, NULL);
                    if (error) { err(L"prepare7-judicator_noqt"); }
                    step(dbjudi);
                }
                error = sqlite3_exec(dbjudi, "COMMIT TRANSACTION", NULL, NULL, NULL);
                if (error) { sqlerror("commit transaction3-judicator", dbjudi); }
            }
        }
        progress = 0;
        for (int ii = 0; ii < cores; ii++)  // Do one final update of the progress bar.
        {
            progress += reports[ii];
        }
        report = progress;
    }
    for (auto& th : peons)
    {
        if (th.joinable())
        {
            th.join();
        }
    }
    if (control == 0)  // If we are done without being cancelled, we can mark the catalogue as 'complete'.
    {
        temp = "[" + sname + "]";
        stmt = "UPDATE TCatalogueIndex SET Description = ? WHERE Name = ?;";
        param.resize(2);
        param[0] = cata_desc;
        param[1] = temp;
        bind(stmt, param);
        error = sqlite3_prepare_v2(dbjudi, stmt.c_str(), -1, &statejudi, NULL);
        if (error) { sqlerror("prepare7-judicator", dbjudi); }
        step(dbjudi);
        log("Catalogue " + sname + " completed its CSV insertion.");
    }
    else
    {
        log("Catalogue " + sname + " had its CSV insertion cancelled.");
    }
    threads_working = 0;
}
void MainWindow::insert_csvs(vector<string>& my_queue, int& control, int& report, wstring cata_wpath, vector<int> prompt)
{
    int my_id = prompt[0];
    vector<vector<string>> text_vars, data_rows;
    string gid, sfile, stmt, temp;
    wstring csv_path;
    QString qtemp;
    int damaged_csv;

    // Prepare the catalogue helper object.
    CATALOGUE cata;
    cata.set_wpath(cata_wpath);
    cata.initialize_table();
    string primary_stmt = cata.create_primary_table();
    cata.insert_primary_columns_template();
    cata.create_csv_tables_template();
    cata.insert_csv_row_template();

    // Needed for catalogue-wide preliminary work, and best done by a thread containing a catalogue object.
    if (my_id == 0)
    {
        qtemp = cata.get_description();
        my_queue.push_back(primary_stmt);
        my_queue.push_back(qtemp.toStdString());
    }

    // Iterate through the assigned CSVs...
    for (int ii = prompt[1]; ii <= prompt[2]; ii++)
    {
        switch (control)
        {
        case 0:  // Standard work.
            damaged_csv = 0;
            gid = cata.get_gid(ii);
            csv_path = cata.get_csv_path(ii);
            sfile = s_memory(csv_path);
            text_vars = cata.extract_text_vars8(sfile);
            data_rows = cata.extract_data_rows8(sfile, damaged_csv);

            if (damaged_csv == 0)  // Undamaged CSV - to be inserted.
            {
                insert_primary_row(my_queue, my_id, cata, gid, text_vars, data_rows); 
                create_insert_csv_table(my_queue, my_id, cata, gid, data_rows);
                create_insert_csv_subtables(my_queue, my_id, cata, gid, data_rows);
            }
            else  // Damaged CSV - will not be inserted, but will be added to the catalogue's list of damaged CSVs.
            {     // This is a temporary measure - it would be better to incorporate these CSVs however possible.
                insert_damaged_row(my_queue, my_id, cata.get_sname(), gid, damaged_csv);
            }

            report++;
            break;

        case 1:  // Cancel.
            control = -1;  // Reporting that thread work has ended.
            return;
        }
    }
}
void MainWindow::insert_primary_row(vector<string>& my_queue, int my_id, CATALOGUE& cata, string& gid, vector<vector<string>>& text_vars, vector<vector<string>>& data_rows)
{
    string stmt = cata.get_primary_template();
    vector<string> param = { gid };
    for (int ii = 0; ii < text_vars.size(); ii++)
    {
        param.push_back(text_vars[ii][1]);
    }
    for (int ii = 0; ii < data_rows.size(); ii++)
    {
        for (int jj = 1; jj < data_rows[ii].size(); jj++)
        {
            param.push_back(data_rows[ii][jj]);
        }
    }
    bind(stmt, param);
    m_jobs[my_id].lock();
    my_queue.push_back(stmt);
    m_jobs[my_id].unlock();
}
void MainWindow::create_insert_csv_table(vector<string>& my_queue, int my_id, CATALOGUE& cata, string& gid, vector<vector<string>>& data_rows)
{
    // Create this CSV's full table, starting from the template.
    string stmt = cata.get_csv_template();
    string sname = cata.get_sname();
    size_t pos1 = stmt.find("!!!");
    string tname = "[" + sname + "$" + gid + "]";
    stmt.replace(pos1, 3, tname);
    m_jobs[my_id].lock();
    my_queue.push_back(stmt);
    m_jobs[my_id].unlock();

    // Append the full table's data rows, one by one, starting from the template each time.
    string stmt0 = cata.get_insert_csv_row_template();
    pos1 = stmt0.find("!!!");
    stmt0.replace(pos1, 3, tname);
    for (int ii = 0; ii < data_rows.size(); ii++)
    {
        stmt = stmt0;
        sclean(data_rows[ii][0], 1);
        bind(stmt, data_rows[ii]);
        m_jobs[my_id].lock();
        my_queue.push_back(stmt);
        m_jobs[my_id].unlock();
    }
}
void MainWindow::create_insert_csv_subtables(vector<string>& my_queue, int my_id, CATALOGUE& cata, string& gid, vector<vector<string>>& data_rows)
{
    string sname = cata.get_sname();
    QVector<QVector<QVector<int>>> tree = cata.get_tree();
    string create_subtable_template = cata.get_create_sub_template();
    string ins_csv_row_template = cata.get_insert_csv_row_template();
    string stmt, stmt0, tname, temp;
    size_t pos0, pos1, num_rows;
    int row_index;

    // Iterate by subtable.
    for (int ii = 0; ii < tree.size(); ii++)
    {
        tname = cata.sublabelmaker(gid, tree[ii]);
        num_rows = tree[ii][1].size() + 1;  // Top row is the parent, subsequent rows are direct children.

        // Create the subtable.
        stmt = create_subtable_template;
        pos1 = stmt.find("!!!");
        stmt.replace(pos1, 3, tname);
        m_jobs[my_id].lock();
        my_queue.push_back(stmt);
        m_jobs[my_id].unlock();

        // Insert the subtable's rows.
        stmt0 = ins_csv_row_template;
        pos1 = stmt0.find("!!!");
        stmt0.replace(pos1, 3, tname);
        for (int jj = 0; jj < num_rows; jj++)
        {
            stmt = stmt0;
            if (jj == 0)
            {
                row_index = tree[ii][0][tree[ii][0].size() - 1];
            }
            else
            {
                row_index = tree[ii][1][jj - 1];
            }
            sclean(data_rows[row_index][0], 1);
            bind(stmt, data_rows[row_index]);
            m_jobs[my_id].lock();
            my_queue.push_back(stmt);
            m_jobs[my_id].unlock();
        }
    }
}
void MainWindow::insert_damaged_row(vector<string>& my_queue, int my_id, string sname, string& gid, int damaged_csv)
{
    string temp = "[" + sname + "$Damaged]";
    string stmt = "INSERT INTO " + temp + " (GID, [Number of Missing Data Entries]) VALUES (?, ?);";
    vector<string> param = { gid, to_string(damaged_csv) };
    bind(stmt, param);
    m_jobs[my_id].lock();
    my_queue.push_back(stmt);
    m_jobs[my_id].unlock();
}


// GUI-SPECIFIC FUNCTIONS, LINKED TO A SINGLE GUI ELEMENT:

// Choose a local drive to examine for spreadsheets.
void MainWindow::on_cB_drives_currentTextChanged(const QString &arg1)
{
    wdrive = arg1.toStdWString();
    qdrive = arg1;
}

// For the given local drive, display (as a tree widget) the available catalogues, organized by year.
void MainWindow::on_pB_scan_clicked()
{
    std::vector<std::vector<std::wstring>> wtree = get_subfolders2(wdrive);
    QVector<QVector<QVector<QString>>> qtree;  // Form [year][catalogue][year, name]
    wstring wyear, wcata;
    QString qyear, qcata;
    size_t pos1, pos2;
    for (size_t ii = 0; ii < wtree.size(); ii++)  // For every year...
    {
        qtree.append(QVector<QVector<QString>>());
        pos1 = wtree[ii][0].find(L"\\");
        pos1++;
        pos2 = wtree[ii][0].find(L"\\", pos1);
        wyear = wtree[ii][0].substr(pos1, pos2 - pos1);
        qyear = QString::fromStdWString(wyear);
        for (size_t jj = 0; jj < wtree[ii].size(); jj++)  // For every catalogue in that year...
        {
            qtree[ii].append(QVector<QString>());
            pos1 = wtree[ii][jj].rfind(L"\\");
            wcata = wtree[ii][jj].substr(pos1 + 1);
            qcata = QString::fromStdWString(wcata);
            qtree[ii][jj].append(qyear);
            qtree[ii][jj].append(qcata);
        }
    }
    build_ui_tree(qtree, 1);  // Window code 1 will populate the 'Catalogues on Drive' section.
}

// Insert the selected catalogues into the database.
void MainWindow::on_pB_insert_clicked()
{
    QList<QTreeWidgetItem *> catas_to_do = ui->TW_cataondrive->selectedItems();
    QVector<QVector<QString>> catas_in_db;  // Form [year][year, catalogues...]
    QMap<QString, int> map_cata;
    QString qyear, qname, qdesc, stmt;
    string syear, sname;
    int control, report;
    int year_index, cata_index;
    bool fine;

    ui->pB_cancel->setEnabled(1);
    all_cata_db(catas_in_db, map_cata);  // Populate the tree and map. NOTE: THIS NEEDS WORK (partial catas).
    for (int ii = 0; ii < catas_to_do.size(); ii++)  // For each catalogue selected...
    {
        control = 0;  // 0 = Standard, 1 = Cancel.
        report = 0;
        qyear = catas_to_do[ii]->text(0);
        syear = qyear.toStdString();
        qname = catas_to_do[ii]->text(1);
        sname = qname.toStdString();
        log(L"Begin insertion of catalogue " + qname.toStdWString());
        threads_working = 1;
        std::thread judi(&MainWindow::judicator, this, std::ref(control), std::ref(report), syear, sname);
        while (threads_working)
        {
            std::this_thread::sleep_for (std::chrono::milliseconds(50));
            QCoreApplication::processEvents();
            jobs_done = report;
            update_bar();
            if (remote_controller)
            {
                control = 1;
            }
        }
        judi.join();
    }    
    update_cata_tree();
    build_ui_tree(cata_tree, 2);
    if (remote_controller)
    {
        log("Insert catalogue " + sname + " was cancelled.");
    }
    else
    {
        log("Insert catalogue " + sname + " was completed.");
    }
}

// (Debug function) Display some information.
void MainWindow::on_pB_test_clicked()
{
    QList<QTreeWidgetItem *> catas_to_do = ui->TW_cataondrive->selectedItems();
    QVector<QVector<QString>> catas_in_db;  // Form [year][year, catalogues...]
    QMap<QString, int> map_cata;
    QString qyear, qname, qdesc, stmt;
    int control, report;
    int year_index, cata_index;
    bool fine;

    ui->pB_cancel->setEnabled(1);
    all_cata_db(catas_in_db, map_cata);  // Populate the tree and map.
    for (int ii = 0; ii < catas_to_do.size(); ii++)  // For each catalogue selected...
    {
        control = 0;  // 0 = Standard, 1 = Cancel.
        report = 0;
        qyear = catas_to_do[ii]->text(0);
        qname = catas_to_do[ii]->text(1);
        log(L"Begin insertion of catalogue " + qname.toStdWString());
        threads_working = 1;
        std::thread judi(&MainWindow::judicator, this, std::ref(control), std::ref(report), qyear, qname);
        //std::thread judi(&MainWindow::judicator_noqt, this, std::ref(control), std::ref(report), qyear, qname);
        while (threads_working)
        {
            std::this_thread::sleep_for (std::chrono::milliseconds(50));
            QCoreApplication::processEvents();
            jobs_done = report;
            update_bar();
            if (remote_controller)
            {
                control = 1;
            }
        }
        judi.join();
    }
    update_cata_tree();
    build_ui_tree(cata_tree, 2);
}

// (Debug function) Perform a series of actions to test new functions.
void MainWindow::on_pB_benchmark_clicked()
{
    wdrive = L"F:";
    qdrive = "F:";
    //QString cata_path = "F:\\3067\\97-570-X1981005";
    reset_db(db_path);
    QString qyear = "3067";
    QString qname = "97-570-X1981005";



    int report = 0;
    int control = 0;  // 0 = Standard, 1 = Stop.
    threads_working = 1;
    std::thread judi(&MainWindow::judicator, this, std::ref(control), std::ref(report), qyear, qname);
    while (threads_working)
    {
        QCoreApplication::processEvents();
        jobs_done = report;
        update_bar();
        if (remote_controller)
        {
            control = 1;
        }
        std::this_thread::sleep_for (std::chrono::milliseconds(50));
    }
    judi.join();

}

// Display the 'tabbed data' for the selected catalogue.
void MainWindow::on_pB_viewdata_clicked()
{
    QSqlQuery q(db);
    QStringList geo_list, row_list;
    QString temp;
    QList<QTreeWidgetItem *> cata_to_do = ui->TW_cataindb->selectedItems();  // Only 1 catalogue can be selected.
    QString qyear = cata_to_do[0]->text(0);
    QString tname = cata_to_do[0]->text(1);

    // Populate the 'Geographic Region' tab.
    QString stmt = "SELECT Geography FROM " + tname;
    bool fine = q.prepare(stmt);
    //if (!fine) { sqlerr("prepare-on_pB_viewdata", q.lastError()); }
    executor(q);
    //if (!fine) { sqlerr("exec-on_pB_viewdata", q.lastError()); }
    while (q.next())
    {
        temp = q.value(0).toString();
        temp.chop(1);
        temp.remove(0, 1);
        geo_list.append(temp);
    }

    // Populate the 'Row Data' tab.
    q.clear();
    stmt = "SELECT * FROM " + tname;
    fine = q.prepare(stmt);
    //if (!fine) { sqlerr("prepare2-on_pB_viewdata", q.lastError()); }
    executor(q);
    //if (!fine) { sqlerr("exec2-on_pB_viewdata", q.lastError()); }
    QSqlRecord rec = q.record();
    int columns = rec.count();
    for (int ii = 0; ii < columns; ii++)
    {
        temp = rec.fieldName(ii);
        row_list.append(temp);
    }

    ui->GID_list->addItems(geo_list);
    ui->Rows_list->addItems(row_list);
}

// All threads inserting catalogues are told to stop after finishing their current CSV.
void MainWindow::on_pB_cancel_clicked()
{
    remote_controller = 1;
}
