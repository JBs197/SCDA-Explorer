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
    //q.exec(QStringLiteral("PRAGMA journal_mode=WAL"));

    // Create (if necessary) these system-wide tables. 
    create_cata_index_table();  
    create_prov_index_table();
    
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
        gid_indices.insert(gid_indices.begin(), input.begin(), input.begin() + 1);
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
        sqlerror("step: " + to_string(error), db);
    }
    return results;
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
void MainWindow::create_prov_index_table()
{
    string stmt = "CREATE TABLE IF NOT EXISTS TProvinceIndex (";
    for (int ii = 0; ii < provinces.size(); ii++)
    {
        stmt += provinces[ii] + " INT, ";
    }
    stmt.pop_back();
    stmt.pop_back();
    stmt += ");";
    int error = sqlite3_prepare_v2(db, stmt.c_str(), -1, &statement, NULL);
    if (error) { sqlerror("prepare-create_prov_index_table", db); }
    step(db, statement);
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
void MainWindow::output_tables()
{
    // Use SQLite's internal listing.
    string stmt = "SELECT name FROM sqlite_master WHERE type='table';";
    int error = sqlite3_prepare_v2(db, stmt.c_str(), -1, &statement, NULL);
    if (error) { sqlerror("prepare-output_tables", db); }
    vector<vector<string>> results = step(db, statement);
    string temp = "Tables in Database (SQLite): \r\n";
    OutputDebugStringA(temp.c_str());
    for (int ii = 0; ii < results.size(); ii++)
    {
        temp = results[ii][0] + "\r\n";
        OutputDebugStringA(temp.c_str());
    }

    // Use the catalogue index.
    stmt = "SELECT * FROM TCatalogueIndex;";
    error = sqlite3_prepare_v2(db, stmt.c_str(), -1, &statement, NULL);
    if (error) { sqlerror("prepare-output_tables", db); }
    results = step(db, statement);
    temp = "Tables in TCatalogueIndex: \r\n";
    OutputDebugStringA(temp.c_str());
    for (int ii = 0; ii < results.size(); ii++)
    {
        temp = results[ii][0] + "\r\n";
        OutputDebugStringA(temp.c_str());
    }
}


// TASK FUNCTIONS, USED BY ONE OR MORE GUI BUTTONS:

// Populate the 'Catalogues on Drive' tab with a tree-list of catalogues present on the selected local drive.
void MainWindow::scan_drive(vector<int>& comm)
{
    std::vector<std::vector<std::wstring>> wtree = get_subfolders2(wdrive);
    QVector<QVector<QVector<QString>>> qtree;  // Form [year][catalogue][year, name]
    wstring wyear, wcata;
    QString qyear, qcata;
    size_t pos1, pos2;
    int iyear;

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
        }
    }
    
    build_ui_tree(qtree, 1);  // Window code 1 will populate the 'Catalogues on Drive' section.
    comm[0] = -1;
}

// Populate the information tabs for a selected catalogue. 
void MainWindow::display_catalogue(sqlite3*& db_gui, vector<int>& comm_cata, string syear, string tname)
{
    sqlite3_stmt* state;

    // Populate the 'Geographic Region' tab.
    string stmt = "SELECT Geography FROM [" + tname + "];";
    int error = sqlite3_prepare_v2(db_gui, stmt.c_str(), -1, &state, NULL);
    if (error) { sqlerror("prepare1-display_catalogue", db_gui); }
    vector<vector<string>> geography = step(db_gui, state);
    QStringList geo_list;
    QString qtemp;
    for (int ii = 0; ii < geography.size(); ii++)
    {
        qtemp = QString::fromStdString(geography[ii][0]);
        geo_list.append(qtemp);
    }
    ui->GID_list->clear();
    ui->GID_list->addItems(geo_list);

    // Populate the 'Row Data' tab.
    stmt = "SELECT Column FROM [" + tname + "$Columns];";
    error = sqlite3_prepare_v2(db_gui, stmt.c_str(), -1, &state, NULL);
    if (error) { sqlerror("prepare2-display_catalogue", db_gui); }
    vector<vector<string>> rows = step(db_gui, state);
    QStringList row_list;
    for (int ii = 0; ii < rows.size(); ii++)
    {
        qtemp = QString::fromStdString(rows[ii][0]);
        row_list.append(qtemp);
    }
    ui->Rows_list->clear();
    ui->Rows_list->addItems(row_list);
    
    // Report completion to the GUI thread.
    comm_cata[0] = -1;
}

// Insert catalogue(s) from local storage into the database.
void MainWindow::judicator(sqlite3*& db_gui, vector<int>& comm_cata, vector<string> prompt)
{
    // prompt has form [syear, sname, gid1, gid2, ...], gid list is only included for partial catalogue insertions.
    string temp = sroots[location] + "\\" + prompt[0] + "\\" + prompt[1];
    wstring cata_wpath = utf8to16(temp);
    int num_gid = get_file_path_number(cata_wpath, L".csv");
    int workload = num_gid / cores;
    int bot = 0;
    int top = workload - 1;
    bool partial_entry = 0;
    vector<string> param(3);
    sqlite3_stmt* statejudi;
    string stmt;
    int error;

    if (prompt.size() == 2) { partial_entry = 0; }
    else if (prompt.size() > 2) { partial_entry = 1; }

    // Insert this catalogue into the catalogue index.
    if (!partial_entry)
    {
        stmt = "INSERT INTO TCatalogueIndex ( Year, Name, Description ) VALUES (?, ?, ?);";
        param[0] = prompt[0];
        param[1] = prompt[1];
        param[2].assign("Incomplete");
        bind(stmt, param);
        error = sqlite3_prepare_v2(db_gui, stmt.c_str(), -1, &statejudi, NULL);
        if (error)
        {
            sqlerror("prepare1-judicator", db_gui);
        }
        step(db_gui, statejudi);
    }

    // Insert this catalogue into the province index (with dummy values).
    stmt = "INSERT INTO TProvinceIndex (";
    for (int ii = 0; ii < provinces.size(); ii++)
    {
        stmt += provinces[ii];
        if (ii < provinces.size() - 1)
        {
            stmt += ", ";
        }
        else
        {
            stmt += ") VALUES (";
        }
    }
    for (int ii = 0; ii < provinces.size(); ii++)
    {
        stmt += "0";
        if (ii < provinces.size() - 1)
        {
            stmt += ", ";
        }
        else
        {
            stmt += ");";
        }
    }
    error = sqlite3_prepare_v2(db_gui, stmt.c_str(), -1, &statejudi, NULL);
    if (error)
    {
        sqlerror("prepare1.1-judicator", db_gui);
    }
    step(db_gui, statejudi);

    // Create a table for this catalogue's damaged CSVs.
    temp = "[" + prompt[1] + "$Damaged]";
    stmt = "CREATE TABLE IF NOT EXISTS " + temp + " (GID NUMERIC, [Number of Missing Data Entries] NUMERIC);";
    error = sqlite3_prepare_v2(db_gui, stmt.c_str(), -1, &statejudi, NULL);
    if (error) { sqlerror("prepare2-judicator_noqt", db_gui); }
    step(db_gui, statejudi);

    // Launch the worker threads, which will iterate through the CSVs.
    vector<std::thread> peons;
    vector<vector<int>> comm_csv(cores, vector<int>());  // Form [thread][control, progress report, size report].
    for (int ii = 0; ii < cores; ii++)
    {
        comm_csv[ii].assign(3, 0);
    }
    vector<int> prompt_csv(3);  // Form [id, bot, top]
    vector<vector<int>> prompt_csv_partial;  // Form [thread][id, gid1, gid2, ...]
    vector<vector<string>> all_queue(cores, vector<string>());  // Form [thread][statements]   
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
    for (int ii = 0; ii < cores; ii++)
    {
        if (!partial_entry)
        {
            prompt_csv[0] = ii;
            prompt_csv[1] = bot;
            prompt_csv[2] = top;
            std::thread thr(&MainWindow::insert_csvs, this, std::ref(all_queue[ii]), std::ref(comm_csv[ii]), cata_wpath, prompt_csv);
            peons.push_back(std::move(thr));
            bot += workload;
            if (ii < cores - 1) { top += workload; }
            else { top = num_gid - 1; }
        }
        else
        {
            comm_csv[ii][0] = 3;
            std::thread thr(&MainWindow::insert_csvs, this, std::ref(all_queue[ii]), std::ref(comm_csv[ii]), cata_wpath, prompt_csv_partial[ii]);
            peons.push_back(std::move(thr));
        }
    }

    // Create the catalogue's primary table.
    while (all_queue[0].size() < 3) { Sleep(50); }  // Wait for the first worker thread to finish its unique task.
    m_jobs[0].lock();
    string stmt_primary = all_queue[0][0];
    string cata_desc = all_queue[0][1];  // Used later.
    string stmt_column = all_queue[0][2];
    all_queue[0].erase(all_queue[0].begin(), all_queue[0].begin() + 3);
    m_jobs[0].unlock();
    error = sqlite3_prepare_v2(db_gui, stmt_primary.c_str(), -1, &statejudi, NULL);
    if (error) { sqlerror("prepare3-judicator", db_gui); }
    step(db_gui, statejudi);

    // Create and populate the catalogue's column table.  
    vector<string> col_titles;
    size_t pos1, pos2;
    pos1 = 0;
    pos2 = stmt_column.find("!!!", pos1 + 3);
    while (pos2 < stmt_column.size())
    {
        temp = stmt_column.substr(pos1 + 3, pos2 - pos1 - 3);
        col_titles.push_back(temp);
        pos1 = pos2;
        pos2 = stmt_column.find("!!!", pos1 + 3);
    }
    error = sqlite3_exec(db_gui, "BEGIN EXCLUSIVE TRANSACTION", NULL, NULL, NULL);
    if (error) { sqlerror("begin transaction0.1-judicator", db_gui); }
    for (int ii = 0; ii < col_titles.size(); ii++)
    {
        error = sqlite3_prepare_v2(db_gui, col_titles[ii].c_str(), -1, &statejudi, NULL);
        if (error) { sqlerror("prepare3.1-judicator", db_gui); }
        step(db_gui, statejudi);
    }
    error = sqlite3_exec(db_gui, "COMMIT TRANSACTION", NULL, NULL, NULL);
    if (error) { sqlerror("commit transaction1.1-judicator", db_gui); }

    // Report the total task size to the GUI thread.
    bool size_reported = 0;
    int num1, num2;
    vector<int> dummy = { 0, 0 };
    vector<vector<int>> reports;
    while (!size_reported)  // Until we have initialized the progress bar... 
    {
        for (int ii = 0; ii < cores; ii++)  // Look at all worker thread reports...
        {
            if (comm_csv[ii][2] == 0)  // If the thread has not yet reported on its task size...
            {
                Sleep(10);  // Pause and try again. 
                break;
            }
            else if (ii == cores - 1)  // If all threads have reported on their size... 
            {
                size_reported = 1;  // Ready to report to GUI.
            }
        }
    }
    reports.push_back(dummy);  // First report is automatically inserted. 
    reports[0][0] = comm_csv[0][2];
    reports[0][1] = 1;
    for (int ii = 1; ii < cores; ii++)
    {
        for (int jj = 0; jj < reports.size(); jj++)
        {
            if (comm_csv[ii][2] == reports[jj][0])
            {
                reports[jj][1]++;
                break;
            }
            else if (jj == reports.size() - 1)
            {
                reports.push_back(dummy);
                reports[reports.size() - 1][0] = comm_csv[ii][2];
                reports[reports.size() - 1][1] = 1;
            }
        }
    }
    num1 = 0;
    for (int ii = 0; ii < reports.size(); ii++)
    {
        if (reports[ii][1] > num1)
        {
            num1 = reports[ii][1];
            num2 = reports[ii][0];
        }
    }
    comm_cata[2] = num2 * num_gid;

    // Loop through the worker threads, inserting their statements into the database.
    int active_thread = 0;
    int inert_threads = 0;
    int pile, progress, num3;
    vector<string> desk;

    // Stop working if a 'jobs done' or 'cancel' signal is sent.
    int no_work = 0;
    while (comm_cata[0] == 0)  
    {
        progress = 0;  // RESUME HERE. Progress bar should be linked to judicator, not workers. 
        for (int ii = 0; ii < cores; ii++)
        {
            progress += comm_csv[ii][1];
        }
        comm_cata[1] = progress;  // Report work progress to GUI thread. 
        pile = all_queue[active_thread].size();
        if (pile > 0)
        {
            no_work = 0;
            m_jobs[active_thread].lock();
            desk = all_queue[active_thread];
            all_queue[active_thread].clear();
            m_jobs[active_thread].unlock();
            error = sqlite3_exec(db_gui, "BEGIN EXCLUSIVE TRANSACTION", NULL, NULL, NULL);
            if (error) { sqlerror("begin transaction1-judicator", db_gui); }
            for (int ii = 0; ii < desk.size(); ii++)
            {
                error = sqlite3_prepare_v2(db_gui, desk[ii].c_str(), -1, &statejudi, NULL);
                if (error)
                {
                    sqlerror("prepare4-judicator", db_gui);
                }
                step(db_gui, statejudi);
            }
            error = sqlite3_exec(db_gui, "COMMIT TRANSACTION", NULL, NULL, NULL);
            if (error) { sqlerror("commit transaction1-judicator", db_gui); }
        }
        else
        {
            no_work++;
            Sleep(10);
        }

        for (int ii = 0; ii < cores; ii++)
        {
            if (comm_csv[ii][0] >= 0)  // If the thread is still working, let it work. 
            {
                break;
            }
            else if (ii == cores - 1)  // If all threads report they have stopped...
            {
                if (no_work >= 3 * cores)  // ... and if we have made 3 empty sweeps for work...
                {
                    comm_cata[0] = 1;  // ... stop looking for work.
                }
            }
        }

        active_thread++;
        if (active_thread >= cores) { active_thread = 0; }
    }
    
    // If task is cancelled, finish inserting the CSVs that were in queue.
    if (comm_cata[0] == 2)  
    {
        for (int ii = 0; ii < cores; ii++)  // Alert the worker threads to stop.
        {
            comm_csv[ii][0] = 1;
        }
        active_thread = 0;
        while (inert_threads < cores)  // Keep working while the threads finish their final tasks.
        {
            pile = all_queue[active_thread].size();
            if (pile > 0)
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

    // Tie up loose threads. 
    for (auto& th : peons)
    {
        if (th.joinable())
        {
            th.join();
        }
    }

    // If we are done without being cancelled, we can mark the catalogue as 'complete'.
    if (comm_cata[0] < 2)  
    {
        stmt = "UPDATE TCatalogueIndex SET Description = ? WHERE Name = ?;";
        param.resize(2);
        param[0] = cata_desc;
        param[1] = prompt[1];
        bind(stmt, param);
        error = sqlite3_prepare_v2(db_gui, stmt.c_str(), -1, &statejudi, NULL);
        if (error) { sqlerror("prepare7-judicator", db_gui); }
        step(db_gui, statejudi);
        log("Catalogue " + prompt[1] + " completed its CSV insertion.");
    }
    else
    {
        log("Catalogue " + prompt[1] + " had its CSV insertion cancelled.");
    }
    threads_working = 0;
}
void MainWindow::insert_csvs(vector<string>& my_queue, vector<int>& comm, wstring cata_wpath, vector<int> prompt)
{
    int my_id = prompt[0];
    vector<vector<string>> text_vars, data_rows;
    string gid, sfile, stmt, temp;
    wstring csv_path;
    QString qtemp;
    int damaged_csv;
    int stmt_count;
    bool reported_size = 0;

    // Prepare the catalogue helper object.
    CATALOGUE cata;
    cata.set_wpath(cata_wpath);
    cata.initialize_table();
    string primary_stmt = cata.create_primary_table();
    string column_stmt = cata.create_column_table();
    cata.insert_primary_columns_template();
    cata.create_csv_tables_template();
    cata.insert_csv_row_template();
    string syear = cata.get_syear();

    // Needed for catalogue-wide preliminary work, and best done by a thread containing a catalogue object.
    if (my_id == 0)
    {
        qtemp = cata.get_description();
        my_queue.push_back(primary_stmt);
        my_queue.push_back(qtemp.toStdString());
        my_queue.push_back(column_stmt);
    }

    // Iterate through the assigned CSVs...
    switch (comm[0])
    {
    case 0:  // Standard work.
        for (int ii = prompt[1]; ii <= prompt[2]; ii++)
        {
            if (comm[0] == 2)  // If a new 'cancel' signal has been received...
            {
                comm[0] = -2;  // Reporting that thread work has ended.
                return;
            }

            stmt_count = 0;
            damaged_csv = 0;
            gid = cata.get_gid(ii);
            csv_path = cata.get_csv_path(ii);
            sfile = s_memory(csv_path);
            text_vars = cata.extract_text_vars8(sfile);
            data_rows = cata.extract_data_rows8(sfile, damaged_csv);

            if (damaged_csv == 0)  // Undamaged CSV - to be inserted.
            {
                stmt_count += insert_primary_row(my_queue, my_id, cata, gid, text_vars, data_rows);
                stmt_count += insert_
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

    case 2:   // Cancel.
        comm[0] = -2;  // Reporting that thread work has ended.
        return;

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
    }
    
    // Reporting 'jobs done'.
    comm[0] = -1;
}
int MainWindow::insert_primary_row(vector<string>& my_queue, int my_id, CATALOGUE& cata, string& gid, vector<vector<string>>& text_vars, vector<vector<string>>& data_rows)
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
    return 1;
}
int MainWindow::update_prov_index(vector<string>& my_queue, int my_id, string& syear, string& gid, vector<vector<string>>& text_vars)
{
    // Determine the region name and the catalogue year.
    string region_name, temp;
    int iyear;
    for (int ii = 0; ii < text_vars.size(); ii++)
    {
        if (text_vars[ii][0] == "Geography")
        {
            region_name = text_vars[ii][1];
            break;
        }
        else if (ii == text_vars.size() - 1)
        {
            for (int jj = 0; jj < text_vars.size(); jj++)
            {
                qDebug() << QString::fromStdString(text_vars[jj][0]) << " = " << QString::fromStdString(text_vars[jj][1]);
            }
            err("find geography-update_prov_index");
        }
    }
    try
    {
        iyear = stoi(syear);
    }
    catch (invalid_argument& ia)
    {
        err("stoi-update_prov_index");
    }

    // Pidgeonhole the region name by year, due to Stats Canada's shifting writing style.
    switch (iyear)
    {
    case 1981:

    }

}
int MainWindow::create_insert_csv_table(vector<string>& my_queue, int my_id, CATALOGUE& cata, string& gid, vector<vector<string>>& data_rows)
{
    int count = 0;

    // Create this CSV's full table, starting from the template.
    string stmt = cata.get_csv_template();
    string sname = cata.get_sname();
    size_t pos1 = stmt.find("!!!");
    string tname = "[" + sname + "$" + gid + "]";
    stmt.replace(pos1, 3, tname);
    m_jobs[my_id].lock();
    my_queue.push_back(stmt);
    m_jobs[my_id].unlock();
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
        m_jobs[my_id].lock();
        my_queue.push_back(stmt);
        m_jobs[my_id].unlock();
        count++;
    }

    return count;
}
int MainWindow::create_insert_csv_subtables(vector<string>& my_queue, int my_id, CATALOGUE& cata, string& gid, vector<vector<string>>& data_rows)
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
        m_jobs[my_id].lock();
        my_queue.push_back(stmt);
        m_jobs[my_id].unlock();
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
            m_jobs[my_id].lock();
            my_queue.push_back(stmt);
            m_jobs[my_id].unlock();
            count++;
        }
    }

    return count;
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
    qdrive = arg1;
    wdrive = arg1.toStdWString();
    sdrive = arg1.toStdString();
}

// For the given local drive, display (as a tree widget) the available catalogues, organized by year.
void MainWindow::on_pB_scan_clicked()
{
    vector<int> comm = { 0, 0, 0 };  // Form [control, progress, task size].
    std::thread scan(&MainWindow::scan_drive, this, std::ref(comm));
    while (comm[0] >= 0)
    {
        Sleep(50);
        QCoreApplication::processEvents();
        if (comm[2] > 0)
        {
            reset_bar(comm[2], "Scanning drive  " + sdrive);
        }
    }
    scan.join();
    ui->tabW_catalogues->setCurrentIndex(1);
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
    vector<int> comm_cata = { 0, 0, 0 };  // Form [control, progress report, size report].
    int year_index, cata_index;
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
            log("Begin insertion of catalogue " + sname);
            threads_working = 1;
            std::thread judi(&MainWindow::judicator, this, std::ref(db), std::ref(comm_cata), prompt);
            while (!size_received)  // Start the progress bar when the judicator has reported on the total number of
            {                       // insertions to be made.
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
                QCoreApplication::processEvents();
                if (comm_cata[2] > 0)
                {
                    reset_bar(comm_cata[2], "Inserting spreadsheets into  " + sname);
                    size_received = 1;
                }
            }
            while (threads_working)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
                QCoreApplication::processEvents();
                jobs_done = comm_cata[1];
                update_bar();

                if (remote_controller == 2)
                {
                    comm_cata[0] = 2;
                }
            }
            judi.join();
        }
        else
        {
            year_index = map_cata.value(qyear);
            qdesc = catas_in_db[year_index][cata_index][2];
            if (qdesc == "Incomplete")  // Catalogue is partially present within the database. Finish that.
            {
                prompt = missing_gids(db, 3, syear, sname);
                log("Resume insertion of partial catalogue " + sname);
                threads_working = 1;
                std::thread judi(&MainWindow::judicator, this, std::ref(db), std::ref(comm_cata), prompt);
                while (!size_received)  // Start the progress bar when the judicator has reported on the total number of
                {                       // insertions to be made.
                    std::this_thread::sleep_for(std::chrono::milliseconds(50));
                    QCoreApplication::processEvents();
                    if (comm_cata[2] > 0)
                    {
                        reset_bar(comm_cata[2], "Inserting spreadsheets into  " + sname);
                        size_received = 1;
                    }
                }
                while (threads_working)
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds(50));
                    QCoreApplication::processEvents();
                    jobs_done = comm_cata[1];
                    update_bar();

                    if (remote_controller == 2)
                    {
                        comm_cata[0] = 2;
                    }
                }
                judi.join();
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
}

// (Debug function) Display some information.
void MainWindow::on_pB_test_clicked()
{

}

// (Debug function) Perform a series of actions to test new functions.
void MainWindow::on_pB_benchmark_clicked()
{

}

// Display the 'tabbed data' for the selected catalogue.
void MainWindow::on_pB_viewdata_clicked()
{
    QList<QTreeWidgetItem *> cata_to_do = ui->TW_cataindb->selectedItems();  // Only 1 catalogue can be selected.
    QString qyear = cata_to_do[0]->text(0);
    QString tname = cata_to_do[0]->text(1);
    vector<int> comm_cata = { 0, 0, 0 };  // Form [control, progress report, size report]

    std::thread dispcata(&MainWindow::display_catalogue, this, std::ref(db), std::ref(comm_cata), qyear.toStdString(), tname.toStdString());
    while (comm_cata[0] >= 0)
    {
        Sleep(50);
        QCoreApplication::processEvents();
        jobs_done = comm_cata[1];
        update_bar();
        if (remote_controller)
        {
            comm_cata[0] = 1;
        }
    }
    dispcata.join();
}

// All threads inserting catalogues are told to stop after finishing their current CSV.
void MainWindow::on_pB_cancel_clicked()
{
    remote_controller = 2;
}

// Display the raw table data for the selected region.
void MainWindow::on_pB_region_clicked()
{

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
            ui->pB_viewdata->setEnabled(0);
        }
        else
        {
            ui->pB_viewdata->setEnabled(1);
        }
    }
    else
    {
        ui->pB_viewdata->setEnabled(0);
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
        ui->pB_region->setEnabled(1);
    }
    else
    {
        ui->pB_region->setEnabled(0);
    }
}


/*
    vector<int> comm = { 0, 0, 0 };  // Form [control, progress, task size].
    std::thread func(&MainWindow::function, this, std::ref(db), std::ref(comm));
    while (comm[0] >= 0)
    {
        Sleep(50);
        QCoreApplication::processEvents();
        if (comm[2] > 0)
        {
            reset_bar(comm[2], "I am doing...  " + stuff);
        }
    }
    func.join();
*/

/*
    int error = sqlite3_prepare_v2(db_gui, stmt.c_str(), -1, &state, NULL);
    if (error) { sqlerror("The error message.", db_gui); }
    step(db_gui, state);
*/