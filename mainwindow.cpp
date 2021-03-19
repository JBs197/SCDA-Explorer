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

// CONTROL INTEGER REFERENCE LIST:
// 0 = Normal
// 1 = Finished
// 2 = Cancelled
// 3 = Partial

// MULTI-PURPOSE FUNCTIONS:

// Perform a variety of tasks from the MainWindow constructor:
// 1. Connect the database memory object with a pre-existing local database file, if it exists.
// 2. Display (in tree form) the database's current repository of catalogues on the GUI.
// 3. Begin a new process log for this runtime session.
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

    // Open the database from an existing local db file, or (failing that) make a new one.
    db_path = sroots[location] + "\\SCDA.db";
    int error = sqlite3_open_v2(db_path.c_str(), &db, (SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE), NULL);
    if (error) { sqlerror("open-initialize", db); }  
    string stmt = "PRAGMA secure_delete;";
    error = sqlite3_prepare_v2(db, stmt.c_str(), -1, &statement, NULL);
    if (error) { sqlerror("prepare1-initialize", db); }
    step(db, statement);

    sf.init(sroots[location] + "\\SCDA.db");

    //q.exec(QStringLiteral("PRAGMA journal_mode=WAL"));

    // Create (if necessary) these system-wide tables. 
    create_cata_index_table();  
    vector<string> tg_titles = { "Name", "param1" };
    vector<int> tg_types = { 0, 0 };
    sf.create_table("TGenealogy", tg_titles, tg_types);
    sf.tg_max_param();  // To update the internal value.
    
    update_cata_tree();  // The cata_tree matrix is kept in QString form, given its proximity to the GUI.
    build_ui_tree(cata_tree, 2);
    vector<mutex> dummy(cores);
    m_jobs.swap(dummy);

    // Initialize the progress bar with blanks.
    reset_bar(100, " ");

    // Reset the runtime log file.
    clear_log();
    log("MainWindow initialized.");
}

// Populate a 3D tree containing the names of all catalogues in the database. Form [year][name][year, name, description].
// Also, populate a map connecting qyear->year_index  and  qname->cata_index.
void MainWindow::all_cata_db(QVector<QVector<QVector<QString>>>& ntree, QMap<QString, int>& map_tree)
{
    QString qyear, qname, qdesc, temp;
    int year_index, cata_index;
    string stmt = "SELECT Year, Name, Description FROM TCatalogueIndex;";
    int error = sqlite3_prepare_v2(db, stmt.c_str(), -1, &statement, NULL);
    if (error) { sqlerror("prepare-all_cata_db", db); }
    vector<vector<string>> results = step(db, statement);    
    for (int ii = 0; ii < results.size(); ii++)
    {
        qyear = QString::fromStdString(results[ii][0]);
        qname = QString::fromStdString(results[ii][1]);
        qdesc = QString::fromStdString(results[ii][2]);
        year_index = map_tree.value(qyear, -1);
        if (year_index < 0)
        {
            year_index = ntree.size();
            map_tree.insert(qyear, year_index);
            ntree.append(QVector<QVector<QString>>());
        }
        cata_index = ntree[year_index].size();
        map_tree.insert(qname, cata_index);
        ntree[year_index].append(QVector<QString>());
        ntree[year_index][cata_index].append(qyear);
        ntree[year_index][cata_index].append(qname);
        ntree[year_index][cata_index].append(qdesc);
    }
}

// Threadsafe error log function specific to SQL errors.
void MainWindow::sqlerror(string func, sqlite3*& dbnoqt)
{
    int errcode = sqlite3_errcode(dbnoqt);
    const char* errmsg = sqlite3_errmsg(dbnoqt);
    string serrmsg(errmsg);
    string path = sroots[location] + "\\SCDA Error Log.txt";
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
    if (!WriteFile(hprinter, message.c_str(), fsize, &bytes, NULL))
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

// Returns TRUE or FALSE as to the existance of a given table within the database.
bool MainWindow::table_exist(string& tname)
{
    string stmt = "SELECT name FROM sqlite_master WHERE type='table';";
    int error = sqlite3_prepare_v2(db, stmt.c_str(), -1, &statement, NULL);
    if (error) { sqlerror("prepare-table_exist", db); }
    vector<vector<string>> results = step(db, statement);
    for (int ii = 0; ii < results.size(); ii++)
    {
        if (tname == results[ii][0])
        {
            return 1;
        }
    }
    return 0;
}

// For a given incomplete catalogue, determine its existing CSV entries and return a list of missing GIDs.
vector<string> MainWindow::scan_incomplete_cata(string syear, string sname)
{
    // Determine which CSVs are already present within the database.
    string stmt = "SELECT GID FROM [" + sname + "];";
    int error = sqlite3_prepare_v2(db, stmt.c_str(), -1, &statement, NULL);
    if (error) { sqlerror("prepare-scan_incomplete_cata", db); }
    vector<vector<string>> gid_haves = step(db, statement);

    // Subtract the CSVs in the database from the complete list.
    string cata_path = sroots[location] + "\\" + syear + "\\" + sname;
    vector<string> gid_list = extract_gids(cata_path);
    string temp1, temp2;
    for (int ii = gid_haves.size() - 1; ii >= 0; ii--)
    {
        for (int jj = gid_list.size() - 1; jj >= 0; jj--)
        {
            if (gid_haves[ii][0] == gid_list[jj])
            {
                gid_list.erase(gid_list.begin() + jj);
                break;
            }
        }
    }
    log("Catalogue " + sname + " was scanned. " + to_string(gid_list.size()) + " CSVs were missing.");
    return gid_list;
}

// Given a catalogue year and name, return a list of GIDs not already present in the database.
// Mode 0 = return gid list | 1 = return syear, sname, gid list. 
// Mode 2 = return gid index | 3 = return syear, sname, gid index.
vector<string> MainWindow::missing_gids(sqlite3*& db_gui, int mode, string syear, string sname)
{
    string cata_path = sroots[location] + "\\" + syear + "\\" + sname;
    vector<string> gid_list = extract_gids(cata_path);
    sqlite3_stmt* state;
    string stmt = "SELECT GID FROM [" + sname + "];";
    int error = sqlite3_prepare_v2(db_gui, stmt.c_str(), -1, &state, NULL);
    if (error) { sqlerror("prepare1-missing_gids", db_gui); }
    vector<vector<string>> db_gids = step(db_gui, state);
    vector<string> input(2);
    vector<string> gid_indices;

    switch (mode)
    {
    case 0:
        for (int ii = 0; ii < db_gids.size(); ii++)
        {
            for (int jj = 0; jj < gid_list.size(); jj++)
            {
                if (db_gids[ii][0] == gid_list[jj])
                {
                    gid_list.erase(gid_list.begin() + jj);
                    break;
                }
            }
        }
        break;

    case 1:
        for (int ii = 0; ii < db_gids.size(); ii++)
        {
            for (int jj = 0; jj < gid_list.size(); jj++)
            {
                if (db_gids[ii][0] == gid_list[jj])
                {
                    gid_list.erase(gid_list.begin() + jj);
                    break;
                }
            }
        }
        input[0] = syear;
        input[1] = sname;
        gid_list.insert(gid_list.begin(), input.begin(), input.begin() + 1);
        break;

    case 2:
        for (int ii = 0; ii < gid_list.size(); ii++)
        {
            for (int jj = 0; jj < db_gids.size(); jj++)
            {
                if (gid_list[ii] == db_gids[jj][0])
                {
                    break;
                }
                else if (jj == db_gids.size() - 1)
                {
                    gid_indices.push_back(to_string(ii));
                }
            }
        }
        return gid_indices;

    case 3:
        for (int ii = 0; ii < gid_list.size(); ii++)
        {
            for (int jj = 0; jj < db_gids.size(); jj++)
            {
                if (gid_list[ii] == db_gids[jj][0])
                {
                    break;
                }
                else if (jj == db_gids.size() - 1)
                {
                    gid_indices.push_back(to_string(ii));
                }
            }
        }
        input[0] = syear;
        input[1] = sname;
        gid_indices.insert(gid_indices.begin(), input.begin(), input.begin() + 2);
        return gid_indices;
    }
    return gid_list;
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

// Execute a prepared statement, and return values if applicable.
vector<vector<string>> MainWindow::step(sqlite3*& db, sqlite3_stmt* state)
{
    int type, col_count, size;  // Type: 1(int), 2(double), 3(string)
    int error = sqlite3_step(state);
    int ivalue;
    double dvalue;
    string svalue;
    vector<vector<string>> results;

    while (error == 100)
    {
        col_count = sqlite3_column_count(state);
        results.push_back(vector<string>(col_count));
        for (int ii = 0; ii < col_count; ii++)
        {
            type = sqlite3_column_type(state, ii);
            switch (type)
            {
            case 1:
                ivalue = sqlite3_column_int(state, ii);
                results[results.size() - 1][ii] = to_string(ivalue);
                break;
            case 2:
                dvalue = sqlite3_column_double(state, ii);
                results[results.size() - 1][ii] = to_string(dvalue);
                break;
            case 3:
                size = sqlite3_column_bytes(state, ii);
                char* buffer = (char*)sqlite3_column_text(state, ii);
                svalue.assign(buffer, size);
                results[results.size() - 1][ii] = svalue;
                break;
            }
        }
        error = sqlite3_step(state);
    }
    if (error > 0 && error != 101)
    {
        sqlerror("step: " + to_string(error), db);
    }
    return results;
}
vector<string> MainWindow::step_1(sqlite3*& db, sqlite3_stmt* state)
{
    // This function variant returns only the first row of the SELECT command.
    int type, col_count, size;  // Type: 1(int), 2(double), 3(string)
    int error = sqlite3_step(state);
    int ivalue;
    double dvalue;
    string svalue;
    vector<string> results;

    if (error == 100)
    {
        col_count = sqlite3_column_count(state);
        results.resize(col_count);
        for (int ii = 0; ii < col_count; ii++)
        {
            type = sqlite3_column_type(state, ii);
            switch (type)
            {
            case 1:
                ivalue = sqlite3_column_int(state, ii);
                results[ii] = to_string(ivalue);
                break;
            case 2:
                dvalue = sqlite3_column_double(state, ii);
                results[ii] = to_string(dvalue);
                break;
            case 3:
                size = sqlite3_column_bytes(state, ii);
                char* buffer = (char*)sqlite3_column_text(state, ii);
                svalue.assign(buffer, size);
                results[ii] = svalue;
                break;
            }
        }
        return results;
    }
    else if (error > 0 && error != 101)
    {
        sqlerror("step1: " + to_string(error), db);
    }
    return results;
}

// Return a list of all tables in the database.
vector<string> MainWindow::all_tables()
{
    // Use SQLite's internal listing.
    string stmt = "SELECT name FROM sqlite_master WHERE type='table';";
    int error = sqlite3_prepare_v2(db, stmt.c_str(), -1, &statement, NULL);
    if (error) { sqlerror("prepare-output_tables", db); }
    vector<vector<string>> results = step(db, statement);
    vector<string> output(results.size());
    for (int ii = 0; ii < results.size(); ii++)
    {
        output[ii] = results[ii][0];
    }
    return output;
}

// Delete all tables and subtables related to the given catalogue.
void MainWindow::delete_cata(sqlite3*& db_gui, SWITCHBOARD& sb, int sb_index, string sname)
{
    vector<int> comm = { 0, 0, 1 };
    thread::id myid = this_thread::get_id();
    sb.answer_call(myid, comm, sb_index);

    sqlite3_stmt* state;
    vector<string> table_list = all_tables();
    vector<int> marked;
    size_t pos1;
    string stmt;
    int error;
    
    // Determine which tables contain the catalogue(s) to be deleted.
    for (int jj = 0; jj < table_list.size(); jj++)
    {
        pos1 = table_list[jj].find(sname);
        if (pos1 < table_list[jj].size())
        {
            marked.push_back(jj);
        }
    }

    // Firstly, delete each catalogue's row in the index table.
    stmt = "DELETE FROM TCatalogueIndex WHERE Name = '" + sname + "' ;";
    error = sqlite3_prepare_v2(db_gui, stmt.c_str(), -1, &state, NULL);
    if (error) { sqlerror("prepare1-delete_cata", db_gui); }
    step(db_gui, state);

    // Then, delete each marked table.
    string stmt0 = "DELETE FROM [!!!] ;";
    error = sqlite3_exec(db_gui, "BEGIN EXCLUSIVE TRANSACTION", NULL, NULL, NULL);
    if (error) { sqlerror("begin transaction1-delete_cata", db_gui); }
    for (int ii = 0; ii < marked.size(); ii++)
    {
        stmt = stmt0;
        pos1 = stmt.find("!!!");
        stmt.replace(pos1, 3, table_list[marked[ii]]);
        error = sqlite3_prepare_v2(db_gui, stmt.c_str(), -1, &state, NULL);
        if (error) { sqlerror("prepare2-delete_cata", db_gui); }
        step(db_gui, state);
    }
    error = sqlite3_exec(db_gui, "COMMIT TRANSACTION", NULL, NULL, NULL);
    if (error) { sqlerror("commit transaction1-delete_cata", db_gui); }

    comm[0] = 1;
    comm[1] = 1;
    sb.update(myid, comm);
}

// Functions for the meta-tables.
void MainWindow::create_cata_index_table()
{
    string stmt = "CREATE TABLE IF NOT EXISTS TCatalogueIndex (Year TEXT, ";
    stmt += "Name TEXT, Description TEXT);";
    int error = sqlite3_prepare_v2(db, stmt.c_str(), -1, &statement, NULL);
    if (error) { sqlerror("prepare-create_cata_index_table", db); }
    step(db, statement);
}

// For a prepared matrix of QString data, display it on the GUI as a 2D tree widget (year -> cata entry).
void MainWindow::build_ui_tree(QVector<QVector<QVector<QString>>>& qtree, int window)
{
    QTreeWidgetItem* item;
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
    QTreeWidgetItem* item;
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
QVector<QTreeWidgetItem*> MainWindow::build_ui_tree_temp(QVector<QVector<QVector<QString>>>& qtree, int window)
{
    QTreeWidgetItem* item;
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
        break;
    }
    return qroots;
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
    vector<vector<string>> dyear = step(db, statement);  // Column vector containing a list of distinct years in the db.

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
    error = sqlite3_prepare_v2(db, stmt.c_str(), -1, &statement, NULL);
    if (error) { sqlerror("prepare2-update_cata_tree", db); }
    vector<vector<string>> TCI = step(db, statement);    
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


// GUI-SPECIFIC FUNCTIONS, WITH ASSOCIATED MANAGER AND WORKER FUNCTIONS:

// For the given local drive, display (as a tree widget) the available catalogues, organized by year.
void MainWindow::on_pB_scan_clicked()
{
    vector<vector<int>> comm(1, vector<int>());
    comm[0] = { 0, 0, 0 };  // Form [control, progress, task size].
    thread::id myid = this_thread::get_id();
    int sb_index;
    QVector<QTreeWidgetItem*> qroots;
    int error = sb.start_call(myid, comm[0], sb_index, "Scan Drive");
    if (error) { errnum("start_call-pB_scan_clicked", error); }
    std::thread scan(&MainWindow::scan_drive, this, ref(sb), sb_index, ref(qroots));
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
                reset_bar(comm[1][2], "Scanning drive  " + sdrive);  // ... initialize the progress bar.
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
    scan.join();
    error = sb.end_call(myid);
    if (error) { errnum("end_call-pB_scan_clicked", error); }
    ui->TW_cataondrive->addTopLevelItems(qroots);
    //auto_expand(ui->TW_cataondrive, 20);
    ui->tabW_catalogues->setCurrentIndex(1);
    log("Scanned drive " + sdrive + " for catalogues.");
}
void MainWindow::scan_drive(SWITCHBOARD& sb, int sb_index, QVector<QTreeWidgetItem*>& qroots)
{
    vector<int> comm = { 0, 0, 1 };
    thread::id myid = this_thread::get_id();
    sb.answer_call(myid, comm, sb_index);

    std::vector<std::vector<std::wstring>> wtree = get_subfolders2(wdrive);
    QVector<QVector<QVector<QString>>> qtree;  // Form [year][catalogue][year, name]
    wstring wyear, wcata;
    QString qyear, qcata;
    size_t pos1, pos2;
    int iyear;
    int cata_count = 0;

    // Filter out root folders that are not years.
    for (int ii = 0; ii < wtree.size(); ii++)
    {
        pos1 = wtree[ii][0].find(L"\\");
        pos2 = wtree[ii][0].find(L"\\", pos1 + 1);
        if (pos2 > wtree[ii][0].size())
        {
            pos2 = wtree[ii][0].size();
        }
        wyear = wtree[ii][0].substr(pos1 + 1, pos2 - pos1 - 1);
        if (wyear.size() == 4)
        {
            try
            {
                iyear = stoi(wyear);  // Only folders named as 4-digit numbers are kept.
                continue;
            }
            catch (invalid_argument& ia)
            {
                warn(L"stoi-scan_drive");
            }
        }
        wtree.erase(wtree.begin() + ii);
        ii--;
    }

    // Convert the folder/file paths into a GUI-friendly tree. 
    for (size_t ii = 0; ii < wtree.size(); ii++)  // For every year...
    {
        qtree.append(QVector<QVector<QString>>());
        pos1 = wtree[ii][0].find(L"\\");
        pos1++;
        pos2 = wtree[ii][0].find(L"\\", pos1);
        wyear = wtree[ii][0].substr(pos1, pos2 - pos1);
        qyear = QString::fromStdWString(wyear);
        cata_count++;  // One space for a year.
        for (size_t jj = 0; jj < wtree[ii].size(); jj++)  // For every catalogue in that year...
        {
            pos1 = wtree[ii][jj].find('.');
            if (pos1 < wtree[ii][jj].size()) { continue; }
            qtree[ii].append(QVector<QString>());
            pos1 = wtree[ii][jj].rfind(L"\\");
            wcata = wtree[ii][jj].substr(pos1 + 1);
            qcata = QString::fromStdWString(wcata);
            qtree[ii][qtree[ii].size() - 1].append(qyear);
            qtree[ii][qtree[ii].size() - 1].append(qcata);
            cata_count++;
        }
    }

    qroots = build_ui_tree_temp(qtree, 1);  // Window code 1 will populate the 'Catalogues on Drive' section.
    comm[0] = 1;
    comm[1] = 1;
    sb.update(myid, comm);
    Sleep(10);
}

// Insert the selected catalogues into the database.
void MainWindow::on_pB_insert_clicked()
{
    QList<QTreeWidgetItem *> catas_to_do = ui->TW_cataondrive->selectedItems();
    QVector<QVector<QVector<QString>>> catas_in_db;  // Form [year][name][year, name, description]
    QMap<QString, int> map_cata;  // qyear -> year_index, qname -> cata_index
    QString qyear, qname, qdesc, stmt, qtemp;
    string syear, sname;
    vector<string> prompt(2);
    vector<vector<int>> comm(1, vector<int>());
    comm[0].assign(comm_length, 0);  // Form [control, progress report, size report, max param].
    thread::id myid = this_thread::get_id();
    int error, sb_index, year_index, cata_index;
    bool size_received = 0;
    bool fine;

    ui->tabW_catalogues->setCurrentIndex(0);
    ui->pB_cancel->setEnabled(1);
    all_cata_db(catas_in_db, map_cata);  // Populate the tree and map. 
    for (int ii = 0; ii < catas_to_do.size(); ii++)  // For each catalogue selected...
    {
        qyear = catas_to_do[ii]->text(0);
        syear = qyear.toStdString();
        prompt[0] = syear;
        qname = catas_to_do[ii]->text(1);
        sname = qname.toStdString();
        prompt[1] = sname;

        // Check if the catalogue is already in the database.
        cata_index = map_cata.value(qname, -1);
        if (cata_index < 0)  // Catalogue is not in the database. Remedy that.
        {
            log("Beginning insertion of catalogue " + sname);
            error = sb.start_call(myid, comm[0], sb_index, "Insert Catalogues");
            if (error) { errnum("start_call1-pB_insert_clicked", error); }
            sb.set_prompt(myid, prompt);
            std::thread judi(&MainWindow::judicator, this, std::ref(sf), std::ref(sb), sb_index);
            while (1)
            {
                Sleep(gui_sleep);
                QCoreApplication::processEvents();
                if (remote_controller == 2)  // The 'cancel' button was pressed.
                {
                    comm[0][0] = 2;  // Inform the manager thread it should abort its task.
                    remote_controller = 0;
                }
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

                if (comm[1][0] > 0)
                {
                    break;
                }
            }
            judi.join();
            error = sb.end_call(myid);
            if (error) { errnum("end_call1-pB_insert_clicked", error); }
            log("Finished insertion of catalogue " + sname);
        }
        else
        {
            year_index = map_cata.value(qyear);
            qdesc = catas_in_db[year_index][cata_index][2];
            if (qdesc == "Incomplete")  // Catalogue is partially present within the database. Finish that.
            {
                prompt = missing_gids(db, 3, syear, sname);
                log("Resuming insertion of partial catalogue " + sname);
                error = sb.start_call(myid, comm[0], sb_index, "Insert Catalogues");
                if (error) { errnum("start_call2-pB_insert_clicked", error); }
                std::thread judi(&MainWindow::judicator, this, std::ref(sf), std::ref(sb), sb_index);
                while (1)
                {
                    Sleep(gui_sleep);
                    QCoreApplication::processEvents();
                    if (remote_controller == 2)  // The 'cancel' button was pressed.
                    {
                        comm[0][0] = 2;  // Inform the manager thread it should abort its task.
                        remote_controller = 0;
                    }
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

                    if (comm[1][0] > 0)
                    {
                        break;
                    }
                }
                judi.join();
                error = sb.end_call(myid);
                if (error) { errnum("end_call2-pB_insert_clicked", error); }
                log("Finished insertion of catalogue " + sname);
            }
            else  // Catalogue is already present within the database. Inform the user. 
            {
                qtemp = "Catalogue " + qname + " is already present within the database.";
                ui->progressBar->setValue(100);
                ui->QL_bar->setText(qtemp);
            }
        }
    }
    ui->pB_cancel->setEnabled(0);
    update_cata_tree();
    build_ui_tree(cata_tree, 2);
    auto_expand(ui->TW_cataindb, 20);
}
void MainWindow::judicator(SQLFUNC& sf, SWITCHBOARD& sb, int sb_index)
{
    vector<int> comm;
    vector<vector<int>> gui_comm;
    thread::id myid = this_thread::get_id();
    sb.answer_call(myid, comm, sb_index);
    vector<string> prompt = sb.get_prompt(myid);

    WINFUNC wf_judi;
    JFUNC jf_judi;

    // prompt has form [syear, sname, gid1, gid2, ...], gid list is only included for partial catalogue insertions.
    string cata_path = sroots[location] + "\\" + prompt[0] + "\\" + prompt[1];
    wstring cata_wpath = utf8to16(cata_path);
    int num_gid = get_file_path_number(cata_wpath, L".csv");
    comm[2] = num_gid;
    sb.update(myid, comm);
    bool partial_entry = 0;
    string stmt, stmt0;
    int error, geo_index, csv_tindex;

    int tg_max_param = sf.tg_max_param();
    bool need_geo = !table_exist(prompt[1]);
    if (prompt.size() > 2) { partial_entry = 1; }

    // Insert this catalogue into the catalogue index.
    vector<string> param(3);
    if (!partial_entry)
    {
        stmt = "INSERT INTO TCatalogueIndex ( Year, Name, Description ) VALUES (?, ?, ?);";
        param[0] = prompt[0];  // year
        param[1] = prompt[1];  // name
        param[2].assign("Incomplete");
        bind(stmt, param);
        sf.executor(stmt);
    }

    // Perform region indexing for this catalogue, if necessary.
    SWITCHBOARD sb_geo;
    vector<string> geo_queue;
    vector<vector<int>> geo_comm(1, vector<int>());
    geo_comm[0].assign(comm.size(), 0);
    if (need_geo)
    {
        log("Begin region indexing for catalogue " + prompt[1]);
        error = sb_geo.start_call(myid, geo_comm[0], geo_index, "Region Indexing");
        if (error) { errnum("start_call(geo)-judicator", error); }
        sb_geo.set_prompt(myid, prompt);
        std::thread geo(&MainWindow::create_insert_region_tables, this, ref(geo_queue), ref(sb_geo), geo_index);
        geo.detach();
    }
    else { geo_comm[0][0] = 1; }

    // Create a table for this catalogue's damaged CSVs.
    string temp = "[" + prompt[1] + "$Damaged]";
    stmt = "CREATE TABLE IF NOT EXISTS " + temp + " (GID NUMERIC, [Number of Missing Data Entries] NUMERIC);";
    sf.executor(stmt);
    sf.insert_tg_existing(prompt[1] + "$Damaged");

    // Create the Stats Can helper object, and create the catalogue's primary table.
    vector<string> csv_list = wf_judi.get_file_list(cata_path, "*.csv");
    temp = cata_path + "\\" + csv_list[0];
    string sample_csv = jf_judi.load(temp);
    STATSCAN sc(cata_path);
    int tg_needed_param = sc.cata_init(sample_csv);
    if (tg_needed_param > tg_max_param)
    {
        sf.safe_col("TGenealogy", tg_needed_param + 1);  // TG must have one column more than max param.
    }
    stmt = sc.make_create_primary_table();
    sf.executor(stmt);
    sf.insert_tg_existing(prompt[1]);
    sc.extract_gid_list(csv_list);
    sc.extract_csv_branches(csv_list);

    // Launch the worker threads, which will iterate through the CSVs.
    SWITCHBOARD sb_judi;
    int workload = num_gid / cores;
    int bot = 0;
    int top = workload - 1;
    vector<std::thread> peons;
    vector<vector<int>> comm_csv(1, vector<int>());  // Form [thread][control, progress report, size report, max params].
    comm_csv[0].assign(comm.size(), 0);
    vector<int> prompt_csv(3);  // Form [id, bot, top]
    vector<vector<int>> prompt_csv_partial;  // Form [thread][id, gid1, gid2, ...]
    vector<vector<vector<string>>> all_queue(cores, vector<vector<string>>());  // Form [thread][CSV][statements].   
    if (partial_entry)  // If this catalogue insertion is partial, specify individual GIDs tasked to each thread.
    {
        int itemp;
        bot = 2;  // Passing over syear, sname...
        workload = (prompt.size() - 2) / cores;
        prompt_csv_partial.resize(cores);
        for (int ii = 0; ii < cores; ii++)
        {
            prompt_csv_partial[ii].resize(workload + 1);
            prompt_csv_partial[ii][0] = ii;
            for (int jj = 1; jj <= workload; jj++)
            {
                try
                {
                    itemp = stoi(prompt[bot + jj - 1]);
                }
                catch (invalid_argument& ia)
                {
                    err("Failed to extract integer from partial insertion prompt: " + prompt[bot + jj - 1]);
                }
                prompt_csv_partial[ii][jj] = itemp;
            }
            bot += workload;

            if (ii == cores - 1)
            {
                top = (prompt.size() - 2) % cores;
                for (int jj = 0; jj < top; jj++)
                {
                    try
                    {
                        itemp = stoi(prompt[bot + jj - 1]);
                    }
                    catch (invalid_argument& ia)
                    {
                        err("Failed to extract integer from partial insertion prompt: " + prompt[bot + jj - 1]);
                    }
                    prompt_csv_partial[ii].push_back(itemp);
                }
            }
        }
    }
    sb_judi.start_call(myid, comm_csv[0], csv_tindex, "CSV Insertion");
    for (int ii = 0; ii < cores; ii++)
    {
        if (!partial_entry)
        {
            prompt_csv[0] = ii;
            prompt_csv[1] = bot;
            prompt_csv[2] = top;
            std::thread thr(&MainWindow::insert_csvs, this, std::ref(all_queue[ii]), std::ref(sb_judi), csv_tindex, ref(sc), prompt_csv);
            peons.push_back(std::move(thr));
            bot += workload;
            if (ii < cores - 1) { top += workload; }
            else { top = num_gid - 1; }
        }
        else
        {
            comm_csv[ii][0] = 3;
            std::thread thr(&MainWindow::insert_csvs, this, std::ref(all_queue[ii]), std::ref(sb_judi), csv_tindex, ref(sc), prompt_csv_partial[ii]);
            peons.push_back(std::move(thr));
        }
    }

    // Try to process the geo thread's finished statements, if available.
    if (geo_comm[0][0] == 0)
    {
        geo_comm = sb_geo.update(myid, geo_comm[0]);
        if (geo_comm[1][0] == 1)
        {
            sf.insert_prepared(geo_queue);
            geo_comm[0][0] = 1;
            log("Completed the region indexing for catalogue " + prompt[1]);
        }
    }

    // Loop through the worker threads, inserting their statements into the database.
    //vector<int> batch_record;  // List of millisec/CSV recorded.
    int active_thread = 0;
    int inert_threads = 0;
    int csv_pile, progress, num3, dayswork, batch_time, batch_average;
    vector<vector<string>> desk;  // Form [CSV][statements]
    int no_work = 0;      // Stop working if a 'jobs done' or 'cancel' signal is sent.
    QElapsedTimer batch_timer;
    batch_timer.start();
    while (comm[0] == 0)
    {
        m_jobs[active_thread].lock();
        csv_pile = all_queue[active_thread].size();
        m_jobs[active_thread].unlock();
        if (csv_pile > 0)
        {
            qDebug() << "csv_pile size: " << csv_pile;
            no_work = 0;
            m_jobs[active_thread].lock();
            desk = all_queue[active_thread];
            all_queue[active_thread].clear();
            m_jobs[active_thread].unlock();
            dayswork = min(handful, desk.size());
            bot = 0;
            top = dayswork;
            do
            {
                batch_timer.restart();
                for (int ii = bot; ii < top; ii++)   // For a specified number of CSVs in a batch...
                {
                    sf.insert_prepared(desk[ii]);
                }
                batch_time = (int)batch_timer.restart();
                batch_average = batch_time / dayswork;
                qDebug() << batch_average << " msec/CSV";
                comm[1] += dayswork;
                sb.update(myid, comm);
                bot = top;
                dayswork = min(handful, desk.size() - bot);
                top += dayswork;

            } while (dayswork > 0);

        }
        else
        {
            no_work++;
            Sleep(10);
        }

        comm_csv = sb_judi.update(myid, comm_csv[0]);
        for (int ii = 1; ii <= cores; ii++)
        {
            if (comm_csv[ii][0] == 0)  // If the thread is still working, let it work. 
            {
                break;
            }
            else if (ii == cores)  // If all threads report they have stopped...
            {
                if (no_work >= 3 * cores)  // ... and if we have made 3 empty sweeps for work...
                {
                    comm[1] = comm[2];
                    comm[0] = 1;  // ... stop looking for work.
                    sb.update(myid, comm);
                }
            }
        }

        active_thread++;
        if (active_thread >= cores) { active_thread = 0; }

        // Update the GUI thread on manager's progress, and receive status updates from GUI.
        gui_comm = sb.update(myid, comm);
        for (int ii = 0; ii < gui_comm[1].size(); ii++)
        {
            comm[ii] = gui_comm[1][ii];
        }
    }

    // If task is cancelled, finish inserting the CSVs that were in queue.
    /*
    if (comm_cata[0] == 2)
    {
        for (int ii = 0; ii < cores; ii++)  // Alert the worker threads to stop.
        {
            comm_csv[ii][0] = 1;
        }
        active_thread = 0;
        while (inert_threads < cores)  // Keep working while the threads finish their final tasks.
        {
            csv_pile = all_queue[active_thread].size();
            if (csv_pile > 0)
            {
                m_jobs[active_thread].lock();
                desk = all_queue[active_thread];
                all_queue[active_thread].clear();
                m_jobs[active_thread].unlock();
                error = sqlite3_exec(db_gui, "BEGIN EXCLUSIVE TRANSACTION", NULL, NULL, NULL);
                if (error) { sqlerror("begin transaction2-judicator", db_gui); }
                for (int ii = 0; ii < desk.size(); ii++)
                {
                    error = sqlite3_prepare_v2(db_gui, desk[ii].c_str(), -1, &statejudi, NULL);
                    if (error) { sqlerror("prepare5-judicator", db_gui); }
                    step(db_gui, statejudi);
                }
                error = sqlite3_exec(db_gui, "COMMIT TRANSACTION", NULL, NULL, NULL);
                if (error) { sqlerror("commit transaction2-judicator", db_gui); }
            }
            active_thread++;
            if (active_thread >= cores) { active_thread = 0; }

            inert_threads = 0;
            for (int ii = 0; ii < cores; ii++)  // Count how many worker threads have stopped.
            {
                if (comm_csv[ii][0] < 0)
                {
                    inert_threads++;
                }
            }
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
                error = sqlite3_exec(db_gui, "BEGIN EXCLUSIVE TRANSACTION", NULL, NULL, NULL);
                if (error) { sqlerror("begin transaction3-judicator", db_gui); }
                for (int jj = 0; jj < desk.size(); jj++)
                {
                    stmt = desk[jj];
                    error = sqlite3_prepare_v2(db_gui, stmt.c_str(), -1, &statejudi, NULL);
                    if (error) { err(L"prepare7-judicator_noqt"); }
                    step(db_gui, statejudi);
                }
                error = sqlite3_exec(db_gui, "COMMIT TRANSACTION", NULL, NULL, NULL);
                if (error) { sqlerror("commit transaction3-judicator", db_gui); }
            }
        }
        progress = 0;
        for (int ii = 0; ii < cores; ii++)  // Do one final update of the progress bar.
        {
            progress += comm_csv[ii][1];
        }
        comm_cata[1] = progress;
    }
    */

    // Tie up loose threads. 
    for (auto& th : peons)
    {
        if (th.joinable())
        {
            th.join();
        }
    }

    // If the geo thread had not yet finished, wait for it now.
    while (geo_comm[0][0] == 0)
    {
        geo_comm = sb_geo.update(myid, geo_comm[0]);
        if (geo_comm[1][0] == 1)
        {
            sf.insert_prepared(geo_queue);
            geo_comm[0][0] = 1;
            log("Completed the region indexing for catalogue " + prompt[1]);
        }
        else
        {
            Sleep(50);
        }
    }

    // If we are done without being cancelled, we can mark the catalogue as 'complete'.
    param.resize(2);
    if (comm[0] < 2)
    {
        stmt = "UPDATE TCatalogueIndex SET Description = ? WHERE Name = ?;";
        param[0] = sc.get_cata_desc();
        param[1] = prompt[1];
        bind(stmt, param);
        sf.executor(stmt);
        log("Catalogue " + prompt[1] + " completed its CSV insertion.");
    }
    else
    {
        log("Catalogue " + prompt[1] + " had its CSV insertion cancelled.");
    }
}
void MainWindow::insert_csvs(vector<vector<string>>& my_queue, SWITCHBOARD& sb_judi, int csv_tindex, STATSCAN& sc, vector<int> prompt)
{
    vector<int> comm;
    vector<vector<int>> judi_comm;
    thread::id myid = this_thread::get_id();
    sb_judi.answer_call(myid, comm, csv_tindex);

    JFUNC jf_csv;
    WINFUNC wf_csv;
    int my_id = prompt[0];
    vector<string> paperwork;
    vector<string> gid_list = sc.get_gid_list();
    vector<vector<string>> text_vars;
    vector<vector<string>> data_rows;
    vector<string> linearized_titles, vtemp;
    string gid, sfile, stmt, stmt0, csv_path, tname, temp;
    QString qtemp;
    int damaged_csv, tg_params, tg_params_max, num_subtables;

    string cata_name = sc.get_cata_name();
    vector<vector<int>> tree_st = sc.get_csv_tree();
    vector<string> column_titles = sc.get_column_titles();

    // Iterate through the assigned CSVs...
    switch (comm[0])
    {
    case 0:  // Standard work.
        for (int ii = prompt[1]; ii <= prompt[2]; ii++)
        {
            // Send and receive status updates to and from the manager thread.
            judi_comm = sb_judi.update(myid, comm);
            comm[0] = judi_comm[0][0];
            if (comm[0] == 2)  // If a new 'cancel' signal has been received...
            {
                comm[0] = -2;  // Reporting that thread work has ended.
                sb_judi.update(myid, comm);
                return;
            }

            // Load values for this CSV.
            damaged_csv = 0;
            gid = gid_list[ii];  
            csv_path = sc.make_csv_path(ii);
            sfile = jf_csv.load(csv_path);
            text_vars = sc.extract_text_vars(sfile);
            data_rows = sc.extract_rows(sfile, damaged_csv);
            linearized_titles = sc.linearize_row_titles(data_rows, column_titles);

            paperwork.clear();
            tg_params_max = 0;
            if (damaged_csv == 0)  // Undamaged CSV - to be inserted.
            {
                // Insert this CSV's row in the primary table.
                stmt = sc.get_insert_primary_template();
                sc.make_insert_primary_statement(stmt, gid, text_vars, data_rows);
                paperwork.push_back(stmt);

                // Create this CSV's main table.
                stmt = sc.get_create_csv_table_template();
                temp = cata_name + "$!!!";
                tname = sc.make_create_csv_table_statement(stmt, gid, temp);
                paperwork.push_back(stmt);
                vtemp = jf_csv.list_from_marker(tname, '$');
                stmt = sc.make_tg_insert_statement(vtemp);
                paperwork.push_back(stmt);

                // Insert this CSV's main table rows.
                stmt0 = sc.get_insert_csv_row_template();
                for (int ii = 0; ii < data_rows.size(); ii++)
                {
                    stmt = stmt0;
                    vtemp.assign(data_rows[ii].begin() + 1, data_rows[ii].end());  // We insert values only - no row titles.
                    tname = cata_name + "$" + gid;
                    sc.make_insert_csv_row_statement(stmt, tname, vtemp);
                    paperwork.push_back(stmt);
                }

                // Create this CSV's subtables.
                num_subtables = sc.get_num_subtables();
                stmt0 = sc.get_create_csv_table_template();  // Subtables use the same template as the full table.
                for (int ii = 0; ii < num_subtables; ii++)
                {
                    stmt = stmt0;
                    temp = sc.get_subtable_name_template(ii);
                    tname = sc.make_create_csv_table_statement(stmt, gid, temp);
                    paperwork.push_back(stmt);
                    vtemp = jf_csv.list_from_marker(tname, '$');
                    stmt = sc.make_tg_insert_statement(vtemp);
                    paperwork.push_back(stmt);
                }

                // Insert this CSV's subtable rows.
                stmt0 = sc.get_insert_csv_row_template();  // Subtables use the same template as the full table.
                vtemp = sc.make_insert_csv_subtable_statements(gid, tree_st, data_rows);
                for (int ii = 0; ii < vtemp.size(); ii++)
                {
                    paperwork.push_back(vtemp[ii]);
                }
            }
            else  // Damaged CSV - will not be inserted, but will be added to the catalogue's list of damaged CSVs.
            {     // This is a temporary measure - it would be better to incorporate these CSVs however possible.
                stmt = sc.make_insert_damaged_csv(cata_name, gid, damaged_csv);
                log("GID " + gid + " not inserted: damaged.");
            }
            m_jobs[my_id].lock();
            my_queue.push_back(paperwork);
            m_jobs[my_id].unlock();

        }
        break;

    case 2:   // Cancel.
        comm[0] = -2;  // Reporting that thread work has ended.
        sb_judi.update(myid, comm);
        return;
        /*
        case 3:
            for (int ii = 1; ii < prompt.size(); ii++)
            {
                if (comm[0] == 2)  // If a new 'cancel' signal has been received...
                {
                    comm[0] = -2;  // Reporting that thread work has ended.
                    return;
                }

                stmt_count = 0;
                damaged_csv = 0;
                gid = cata.get_gid(prompt[ii]);
                csv_path = cata.get_csv_path(prompt[ii]);
                sfile = s_memory(csv_path);
                text_vars = cata.extract_text_vars8(sfile);
                data_rows = cata.extract_data_rows8(sfile, damaged_csv);

                if (damaged_csv == 0)  // Undamaged CSV - to be inserted.
                {
                    stmt_count += insert_primary_row(my_queue, my_id, cata, gid, text_vars, data_rows);
                    stmt_count += create_insert_csv_table(my_queue, my_id, cata, gid, data_rows);
                    stmt_count += create_insert_csv_subtables(my_queue, my_id, cata, gid, data_rows);
                }
                else  // Damaged CSV - will not be inserted, but will be added to the catalogue's list of damaged CSVs.
                {     // This is a temporary measure - it would be better to incorporate these CSVs however possible.
                    insert_damaged_row(my_queue, my_id, cata.get_sname(), gid, damaged_csv);
                    log("GID " + gid + " not inserted: damaged.");
                    stmt_count += comm[2];
                }

                comm[1] += stmt_count;  // Add this loop's work to the progress report.

                // If we have yet to do so, report the number of insertions needed for one CSV.
                if (!reported_size)
                {
                    comm[2] = stmt_count;
                    reported_size = 1;
                }
            }
            break;
            */
    }

    // Reporting 'jobs done'.
    comm[0] = 1;
    sb_judi.update(myid, comm);
}
int MainWindow::insert_primary_row(vector<string>& paperwork, CATALOGUE& cata, string& gid, vector<vector<string>>& text_vars, vector<vector<string>>& data_rows)
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
    paperwork.push_back(stmt);
    return 1;
}
int MainWindow::create_insert_csv_table(vector<string>& paperwork, CATALOGUE& cata, string& gid, vector<vector<string>>& data_rows)
{
    int count = 0;

    // Create this CSV's full table, starting from the template.
    string stmt = cata.get_csv_template();
    string sname = cata.get_sname();
    size_t pos1 = stmt.find("!!!");
    string tname = "[" + sname + "$" + gid + "]";
    stmt.replace(pos1, 3, tname);
    paperwork.push_back(stmt);
    count++;

    // Append the full table's data rows, one by one, starting from the template each time.
    string stmt0 = cata.get_insert_csv_row_template();
    pos1 = stmt0.find("!!!");
    stmt0.replace(pos1, 3, tname);
    for (int ii = 0; ii < data_rows.size(); ii++)
    {
        stmt = stmt0;
        sclean(data_rows[ii][0], 1);
        bind(stmt, data_rows[ii]);
        paperwork.push_back(stmt);
        count++;
    }
    return count;
}
int MainWindow::create_insert_csv_subtables(vector<string>& paperwork, CATALOGUE& cata, string& gid, vector<vector<string>>& data_rows)
{
    int count = 0;
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
        paperwork.push_back(stmt);
        count++;

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
            paperwork.push_back(stmt);
            count++;
        }
    }

    return count;
}
void MainWindow::insert_damaged_row(vector<string>& paperwork, string sname, string& gid, int damaged_csv)
{
    string temp = "[" + sname + "$Damaged]";
    string stmt = "INSERT INTO " + temp + " (GID, [Number of Missing Data Entries]) VALUES (?, ?);";
    vector<string> param = { gid, to_string(damaged_csv) };
    bind(stmt, param);
    paperwork.push_back(stmt);
}
void MainWindow::create_insert_region_tables(vector<string>& paperwork, SWITCHBOARD& geo, int geo_index)
{
    SQLFUNC sf;
    JFUNC jf;

    vector<int> comm;
    thread::id myid = this_thread::get_id();
    geo.answer_call(myid, comm, geo_index);
    vector<string> prompt = geo.get_prompt(myid);

    // Create the region index table.
    string tname = prompt[1] + "$RegionIndex";
    string stmt = "CREATE TABLE IF NOT EXISTS [" + prompt[1] + "$RegionIndex] ";
    stmt += "(GID INTEGER PRIMARY KEY, [Region Name] TEXT);";
    paperwork.push_back(stmt);
    vector<string> col_titles = { "Name", "param1", "param2" };
    vector<string> row_data = { tname, prompt[1], "RegionIndex" };
    stmt = sf.insert_stmt("TGenealogy", col_titles, row_data);
    paperwork.push_back(stmt);

    // Populate the region index table.
    vector<vector<string>> geo_values;
    int geosize = load_geo(geo_values, prompt[0], prompt[1]);  // NOTE: region names have been cleaned already.
    string stmt0 = "INSERT INTO [" + prompt[1] + "$RegionIndex] (GID, [Region Name]) VALUES (?, ?);";
    for (int ii = 0; ii < geosize; ii++)
    {
        stmt = stmt0;
        bind(stmt, geo_values[ii]);
        paperwork.push_back(stmt);
    }

    // Create the subtable tree and namelist (matching first indices).
    vector<vector<vector<int>>> geo_tree;
    vector<string> slist(geosize);
    for (int ii = 0; ii < geosize; ii++)
    {
        slist[ii] = geo_values[ii][1];
    }
    geo_tree = tree_maker(slist, "+");  // Throughout this program, '+' is used to mark indentation.
    vector<string> tnames;
    int cheddar, max_cheddar, ancestry, itemp;
    max_cheddar = 2;
    for (int ii = 0; ii < geo_tree.size(); ii++)
    {
        cheddar = 2;
        ancestry = geo_tree[ii][0].size();
        tname = "[" + prompt[1] + "$Region";
        for (int jj = 0; jj < ancestry; jj++)
        {
            for (int kk = 0; kk < cheddar; kk++)
            {
                tname += "$";
            }
            if (cheddar > max_cheddar) { max_cheddar = cheddar; }
            cheddar++;
            itemp = geo_tree[ii][0][jj];
            tname += slist[itemp];
        }
        tname += "]";
        tnames.push_back(tname);
    }
    comm[3] = max_cheddar + 1;

    // Create the subtables.
    for (int ii = 0; ii < tnames.size(); ii++)
    {
        stmt = "CREATE TABLE IF NOT EXISTS " + tnames[ii] + "(GID INTEGER PRIMARY KEY, [Region Name] TEXT);";
        paperwork.push_back(stmt);
    }

    // Populate the subtables.
    int num_rows, row_index;
    for (int ii = 0; ii < tnames.size(); ii++)  // For every subtable...
    {
        stmt0 = "INSERT INTO " + tnames[ii] + " (GID, [Region Name]) VALUES (?, ?);";
        num_rows = geo_tree[ii][1].size() + 1;  // Top row is the parent, subsequent rows are direct children.
        for (int jj = 0; jj < num_rows; jj++)  // For every row in this subtable...
        {
            stmt = stmt0;
            if (jj == 0)
            {
                row_index = geo_tree[ii][0][geo_tree[ii][0].size() - 1];
            }
            else
            {
                row_index = geo_tree[ii][1][jj - 1];
            }
            bind(stmt, geo_values[row_index]);
            paperwork.push_back(stmt);
        }
    }

    // Populate TG.
    string temp1;
    vector<string> params, col_temp;
    while (col_titles.size() <= comm[3])
    {
        temp1 = "param" + to_string(col_titles.size());
        col_titles.push_back(temp1);
    }
    for (int ii = 0; ii < tnames.size(); ii++)
    {
        row_data = jf.list_from_marker(tnames[ii], '$');
        col_temp = col_titles;
        col_temp.resize(row_data.size());
        stmt = sf.insert_stmt("TGenealogy", col_temp, row_data);
        paperwork.push_back(stmt);
    }

    // Signal the judicator that the work is done. 
    comm[0] = 1;
    geo.update(myid, comm);
}
int MainWindow::load_geo(vector<vector<string>>& geo_values, string& syear, string& sname)
{
    string geo_list_path = sroots[location] + "\\" + syear + "\\" + sname + "\\" + sname + " geo list.bin";
    wstring wtemp = utf8to16(geo_list_path);
    string geo_list = s_memory(wtemp);
    size_t pos1, pos2;
    string temp, region;
    int region_index, indent;
    pos1 = 0;
    pos2 = geo_list.find('$');
    do
    {
        region_index = geo_values.size();
        geo_values.push_back(vector<string>(2));  // geo_values has form [region_index][gid, region name].
        temp = geo_list.substr(pos1, pos2 - pos1);
        geo_values[region_index][0] = temp;

        pos2 = geo_list.find('\r', pos2);
        pos1 = geo_list.rfind('$', pos2);
        temp = geo_list.substr(pos1 + 1, pos2 - pos1 - 1);
        try
        {
            indent = stoi(temp);
        }
        catch (invalid_argument& ia)
        {
            err("stoi-load_geo");
        }
        region.clear();
        for (int ii = 0; ii < indent; ii++)
        {
            region += '+';
        }

        pos2 = geo_list.rfind('$', pos1 - 1);
        temp = geo_list.substr(pos2 + 1, pos1 - pos2 - 1);
        region += temp;
        sclean(region, 1);
        geo_values[region_index][1] = region;

        pos2 = geo_list.find('\n', pos1);
        pos1 = pos2 + 1;
        pos2 = geo_list.find('$', pos1);
    } while (pos2 < geo_list.size());
    int count = (int)geo_values.size();
    return count;
}

// Display the 'tabbed data' for the selected catalogue.
void MainWindow::on_pB_viewcata_clicked()
{
    QList<QTreeWidgetItem *> cata_to_do = ui->TW_cataindb->selectedItems();  // Only 1 catalogue can be selected.
    QString qyear = cata_to_do[0]->text(0);
    string syear = qyear.toStdString();
    QString qname = cata_to_do[0]->text(1);
    string sname = qname.toStdString();
    vector<string> gid_list;
    QList<QStringList> qlistviews;
    vector<vector<int>> comm(1, vector<int>());
    comm[0] = { 0, 0, 0 };  // Form [control, progress report, size report]
    thread::id myid = this_thread::get_id();
    int sb_index;
    viewcata_data.resize(2);
    viewcata_data[0] = syear;
    viewcata_data[1] = sname;
    vector<vector<int>> tree_st;
    vector<string> tree_pl;
    vector<string> prompt = { syear, sname };
    int error = sb.start_call(myid, comm[0], sb_index, "Display Catalogue");
    if (error) { errnum("start_call-pB_viewcata_clicked", error); }
    sb.set_prompt(myid, prompt);
    std::thread dispcata(&MainWindow::display_catalogue, this, ref(sf), ref(sb), sb_index, ref(gid_list), ref(qlistviews), ref(tree_st), ref(tree_pl));
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
                reset_bar(comm[1][2], "Loading catalogue  " + sname);  // ... initialize the progress bar.
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
    
    ui->GID_list->clear();
    ui->GID_list->addItems(qlistviews[0]);
    ui->Rows_list->clear();
    ui->Rows_list->addItems(qlistviews[1]);
    ui->Tables_list->clear();
    ui->Tables_list->addItems(qlistviews[2]);

    qf.display(ui->TW_tablesindb, tree_st, tree_pl);

    viewcata_gid_list = gid_list;
    log("Displayed catalogue " + sname + " on the GUI.");
}
void MainWindow::display_catalogue(SQLFUNC& sf, SWITCHBOARD& sb, int sb_index, vector<string>& gid_list, QList<QStringList>& qlistviews, vector<vector<int>>& tree_st, vector<string>& tree_pl)
{
    JFUNC jf;
    sqlite3_stmt* state;
    vector<int> comm = { 0, 0, 4 };
    thread::id myid = this_thread::get_id();
    sb.answer_call(myid, comm, sb_index);
    vector<string> prompt = sb.get_prompt(myid);  // syear, sname.

    // Populate the 'Geographic Region' tab.
    vector<string> vtemp = { "GID", "Geography" };
    vector<vector<string>> results;
    sf.select(vtemp, prompt[1], results);
    QStringList geo_list;
    QString qtemp;
    for (int ii = 0; ii < results.size(); ii++)
    {
        gid_list.push_back(results[ii][0]);
        qtemp = QString::fromStdString(results[ii][1]);
        geo_list.append(qtemp);
    }
    qlistviews.append(geo_list);
    comm[1]++;
    sb.update(myid, comm);

    // Populate the 'Row Data' tab.
    string gid = gid_list[0];
    string tname = prompt[1] + "$" + gid;
    vtemp = { "*" };
    sf.select(vtemp, tname, results);
    QStringList row_list;
    for (int ii = 0; ii < results.size(); ii++)
    {
        qtemp = QString::fromStdString(results[ii][0]);
        row_list.append(qtemp);
    }
    qlistviews.append(row_list);
    comm[1]++;
    sb.update(myid, comm);

    // Populate the 'Tables' tab. Display only the tables with this catalogue as param1.
    tname = "TGenealogy";
    vtemp = { "Name", "param1" };
    results.clear();
    sf.select(vtemp, tname, results);
    QStringList qtable_list;
    for (int ii = 0; ii < results.size(); ii++)
    {
        if (results[ii][1] == prompt[1])
        {
            qtemp = QString::fromStdString(results[ii][0]);
            qtable_list.append(qtemp);
        }
    }
    qlistviews.append(qtable_list);
    comm[1]++;
    sb.update(myid, comm);

    // Populate the 'Tables as a Tree' tab.  //RESUME HERE. MAKE A SQL->TREE FUNCTION
    /*
    vector<int> table_indent_list = get_indent_list(table_list, '$');
    jf.tree_from_indent(table_indent_list, tree_st);
    tree_pl = table_list;  
    comm[1]++;
    sb.update(myid, comm);
    */

    // Report completion to the GUI thread.
    comm[0] = 1;
    sb.update(myid, comm);
}
vector<int> MainWindow::get_indent_list(vector<string>& param_list, char param)
{
    vector<int> indent_list(param_list.size());
    size_t pos1, pos2;
    int count;
    for (int ii = 0; ii < param_list.size(); ii++)
    {
        pos1 = param_list[ii].rfind(param);
        pos2 = param_list[ii].find_last_not_of(param, pos1);
        count = pos1 - pos2;
        if (count < 0) { count = 0; }
        indent_list[ii] = count;
    }
    return indent_list;
}

// Remove the selected catalogue from the database.
void MainWindow::on_pB_removecata_clicked()
{
    QList<QTreeWidgetItem*> cata_to_do = ui->TW_cataindb->selectedItems();  // Only 1 catalogue can be selected.
    QString qname = cata_to_do[0]->text(1);
    string sname = qname.toStdString();
    vector<vector<int>> comm(1, vector<int>());
    comm[0] = { 0, 0, 0 };  // Form [control, progress report, size report]
    thread::id myid = this_thread::get_id();
    int sb_index;
    log("Beginning removal of catalogue " + sname + " from the database.");
    int error = sb.start_call(myid, comm[0], sb_index, "Remove Catalogue");
    if (error) { errnum("start_call-pB_removecata_clicked", error); }
    std::thread remcata(&MainWindow::delete_cata, this, ref(db), ref(sb), sb_index, sname);
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
                reset_bar(comm[1][2], "Removing catalogue  " + sname);  // ... initialize the progress bar.
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
    remcata.join();
    error = sb.end_call(myid);
    if (error) { errnum("end_call-pB_removecata_clicked", error); }
    update_cata_tree();
    build_ui_tree(cata_tree, 2);
    auto_expand(ui->TW_cataindb, 20);
    if (sname == viewcata_data[1])
    {
        ui->GID_list->clear();
        ui->Rows_list->clear();
        ui->Tables_list->clear();
        viewcata_data[0].clear();
        viewcata_data[1].clear();
        ui->TW_tablesindb->clear();
    }
    log("Removed catalogue " + sname + " from the database.");
}

// All threads inserting catalogues are told to stop after finishing their current CSV.
void MainWindow::on_pB_cancel_clicked()
{
    remote_controller = 2;
}

// Display the raw table data for the selected region/table.
void MainWindow::on_pB_viewtable_clicked()
{
    QString qtemp;
    string gid, tname, temp1;
    vector<string> column_titles;
    QStringList col_titles;
    vector<vector<string>> row_data;
    vector<vector<int>> row_heights;
    int tab_index = ui->tabW_results->currentIndex();
    int row, title_size, line_breaks, pos1, height;
    QStandardItemModel* model = new QStandardItemModel;
    QStandardItem* cell;

    switch (tab_index)
    {
    case 0:
        row = ui->GID_list->currentRow();
        gid = viewcata_gid_list[row];
        tname = viewcata_data[1] + "$" + gid;
        sf.get_col_titles(tname, column_titles);
        sf.select({ "*" }, tname, row_data);
        if (column_titles.size() <= 2) { pos1 = 1; }
        else { pos1 = 0; }
        for (int ii = pos1; ii < column_titles.size(); ii++)
        {
            qtemp = QString::fromStdString(column_titles[ii]);
            col_titles.append(qtemp);
        }
        model->setRowCount(row_data.size());
        model->setColumnCount(row_data[0].size() - 1);
        model->setHorizontalHeaderLabels(col_titles);
        for (int ii = 0; ii < row_data.size(); ii++)
        {
            qtemp = QString::fromStdString(row_data[ii][0]);
            title_size = qtemp.size();
            if (title_size > 20)
            {
                line_breaks = title_size / 20;
                for (int jj = 1; jj < line_breaks + 1; jj++)
                {
                    pos1 = qtemp.lastIndexOf(' ', jj * 20);
                    qtemp.insert(pos1, " \n");
                }
                height = (line_breaks + 1) * 20;
                row_heights.push_back(vector<int>(2));
                row_heights[row_heights.size() - 1][0] = ii;
                row_heights[row_heights.size() - 1][1] = height;
            }
            model->setHeaderData(ii, Qt::Vertical, qtemp);
            for (int jj = 1; jj < row_data[0].size(); jj++)
            {
                qtemp = QString::fromStdString(row_data[ii][jj]);
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
        break;

    case 2:
        break;
    }
}

// (Debug function) Display some information.
void MainWindow::on_pB_test_clicked()
{
    QList<QString> qrow;
    QString qtemp;
    vector<vector<string>> results;
    sf.select({ "*" }, "TGenealogy", results);
    QStandardItemModel* model = new QStandardItemModel;
    QStandardItem* cell;
    model->setRowCount(results.size());
    model->setColumnCount(6);

    for (int ii = 0; ii < results.size(); ii++)
    {
        for (int jj = 0; jj < results[ii].size(); jj++)
        {
            qtemp = QString::fromStdString(results[ii][jj]);
            cell = new QStandardItem;
            cell->setText(qtemp);
            model->setItem(ii, jj, cell);
        }
    }
    ui->tV_viewtable->setModel(model);


}

// Choose a local drive to examine for spreadsheets.
void MainWindow::on_cB_drives_currentTextChanged(const QString& arg1)
{
    qdrive = arg1;
    wdrive = arg1.toStdWString();
    sdrive = arg1.toStdString();
}

// Update button status (enabled/disabled).
void MainWindow::on_TW_cataindb_itemSelectionChanged()
{
    QList<QTreeWidgetItem*> db_selected = ui->TW_cataindb->selectedItems();
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
void MainWindow::on_TW_cataondrive_itemSelectionChanged()
{
    QList<QTreeWidgetItem*> local_selected = ui->TW_cataondrive->selectedItems();
    if (local_selected.size() > 0)
    {
        ui->pB_insert->setEnabled(1);
    }
    else
    {
        ui->pB_insert->setEnabled(0);
    }
}
void MainWindow::on_GID_list_itemSelectionChanged()
{
    QList<QListWidgetItem*> region_selected = ui->GID_list->selectedItems();
    if (region_selected.size() > 0)
    {
        ui->pB_viewtable->setEnabled(1);
    }
    else
    {
        ui->pB_viewtable->setEnabled(0);
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

/*
    int error = sqlite3_prepare_v2(db_gui, stmt.c_str(), -1, &state, NULL);
    if (error) { sqlerror("The error message.", db_gui); }
    step(db_gui, state);
*/