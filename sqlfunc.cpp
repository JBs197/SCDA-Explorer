#include "sqlfunc.h"

using namespace std;

void SQLFUNC::bind(string& stmt, vector<string>& param)
{
    // Replaces SQL placeholders ('?') with parameter strings. Automatically adds single quotes.
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
        sqlerr("parameter count mismatch-bind");
    }

    pos1 = 0;
    for (int ii = 0; ii < (int)count; ii++)
    {
        sclean(param[ii], 1);
        pos1 = stmt.find('?', pos1 + 1);
        temp = "'" + param[ii] + "'";
        stmt.replace(pos1, 1, temp);
    }
}
void SQLFUNC::create_table(string tname, vector<string>& titles, vector<int>& types)
{
    string stmt = "CREATE TABLE IF NOT EXISTS [" + tname + "] (";
    for (int ii = 0; ii < titles.size(); ii++)
    {
        stmt += "[" + titles[ii] + "] ";
        if (types[ii] == 0)
        {
            stmt += "TEXT, ";
        }
        else if (types[ii] == 1)
        {
            stmt += "NUMERIC, ";
        }
    }
    stmt.pop_back();
    stmt.pop_back();
    stmt += ");";

    executor(stmt);
}
void SQLFUNC::err(string func)
{
    jfsf.err(func);
}
int SQLFUNC::get_num_col(string tname)
{
    vector<string> column_titles;
    get_col_titles(tname, column_titles);
    return column_titles.size();
}
void SQLFUNC::init(string db_path)
{
    int error = sqlite3_open_v2(db_path.c_str(), &db, (SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE), NULL);
    if (error) { sqlerr("open-init"); }
}
void SQLFUNC::insert_tg_existing(string tname)
{
    // Still needed???

    // Convenience function to facilitate the insertion of tables into TGenealogy. 
    // This function is only valid if the table in question already exists in the database.
    
    vector<string> row_data = jfsf.list_from_marker(tname, '$');
    safe_col("TGenealogy", row_data.size());
    insert("TGenealogy", row_data);
}
void SQLFUNC::insert_prepared(vector<string>& stmts)
{
    // Execute a list of prepared SQL statements as one transaction batch.

    int error = sqlite3_exec(db, "BEGIN EXCLUSIVE TRANSACTION", NULL, NULL, NULL);
    if (error) { sqlerr("begin transaction-insert_prepared"); }
    for (int ii = 0; ii < stmts.size(); ii++)
    {
        executor(stmts[ii]);
    }
    error = sqlite3_exec(db, "COMMIT TRANSACTION", NULL, NULL, NULL);
    if (error) 
    { 
        if (error != 5)
        {
            sqlerr("commit transaction-insert_prepared");
        }
        while (error == 5)
        {
            this_thread::sleep_for(5ms);
            error = sqlite3_exec(db, "COMMIT TRANSACTION", NULL, NULL, NULL);
        } 
    }
}
string SQLFUNC::insert_stmt(string tname, vector<string>& column_titles, vector<string>& row_data)
{
    string stmt = "INSERT INTO [" + tname + "] (";
    for (int ii = 0; ii < column_titles.size(); ii++)
    {
        stmt += "[" + column_titles[ii];
        if (ii < column_titles.size() - 1)
        {
            stmt += "], ";
        }
        else
        {
            stmt += "]) VALUES (";
        }
    }
    for (int ii = 0; ii < column_titles.size(); ii++)
    {
        stmt += "?, ";
    }
    stmt.pop_back();
    stmt.pop_back();
    stmt += ");";

    bind(stmt, row_data);
    return stmt;
}
void SQLFUNC::safe_col(string tname, int num_col)
{
    // For a given table name, if it has fewer columns than 'num_col', then append a sufficient
    // number of new columns to it. The new columns will be named 'paramX' where 'X' is its 
    // column index, starting from zero.

    vector<string> column_titles;
    string stmt, temp1;
    get_col_titles(tname, column_titles);
    int my_num = column_titles.size();
    while (my_num < num_col)
    {
        temp1 = "param" + to_string(my_num);
        stmt = "ALTER TABLE [" + tname + "] ADD COLUMN " + temp1 + " TEXT";
        executor(stmt);
        column_titles.clear();
        get_col_titles(tname, column_titles);
        my_num = column_titles.size();
    }
}
int SQLFUNC::sclean(string& bbq, int mode)
{
    int count = 0;
    int pos1, pos2;
    pos1 = bbq.find('[');
    if (pos1 > 0)
    {
        pos2 = bbq.find(']', pos1);
        bbq.erase(pos1, pos2 - pos1 + 1);
    }
    if (mode == 1)
    {
        pos1 = bbq.find('\'');
        while (pos1 > 0)
        {
            bbq.replace(pos1, 1, "''");
            pos1 = bbq.find('\'', pos1 + 2);
        }
    }
    while (1)
    {
        if (bbq.front() == ' ') { bbq.erase(0, 1); count++; }
        else { break; }
    }
    while (1)
    {
        if (bbq.back() == ' ') { bbq.erase(bbq.size() - 1, 1); }
        else { break; }
    }
    return count;
}
void SQLFUNC::select_tree2(string tname, vector<vector<int>>& tree_st, vector<wstring>& tree_pl)
{
    // Produce a tree structure and tree payload for the given table name as root. 
    // Certain table name parameters have special forks within the function.

    //vector<string> tname_params = jfsf.list_from_marker(tname, '$');
    vector<vector<wstring>> results;
    unordered_map<wstring, int> registry;
    vector<vector<int>> kids;
    vector<int> ivtemp;
    vector<string> vtemp;
    vector<string> search = { "GID", "[Region Name]" };
    string temp, sparent, stmt;
    int pl_index, iparent, pivot, inum;
    tree_pl.clear();
    tree_st.clear();

    stmt = "SELECT * FROM [" + tname + "]";
    executor(stmt, results);
    tree_pl.resize(results.size());
    tree_st.resize(results.size());
    kids.resize(results.size(), vector<int>());

    // Remove SQL's null entries, and register each node.
    for (int ii = 0; ii < results.size(); ii++)
    {
        for (int jj = 0; jj < results[ii].size(); jj++)
        {
            if (results[ii][jj] == L"")
            {
                results[ii].erase(results[ii].begin() + jj, results[ii].end());
                break;
            }
        }
        tree_pl[ii] = results[ii][1];
        registry.emplace(results[ii][0], ii);  // Input gid as string, output pl_index.
    }

    // Build the tree structure (parents->node).
    for (int ii = 0; ii < results.size(); ii++)
    {
        for (int jj = 2; jj < results[ii].size(); jj++)
        {
            try
            {
                iparent = registry.at(results[ii][jj]);
                tree_st[ii].push_back(iparent);
            }
            catch (out_of_range& oor)
            {
                err("iparent registry-sf.select_tree");
            }
        }
        if (results[ii].size() > 2)
        {
            kids[iparent].push_back(ii);
        }
        tree_st[ii].push_back(-1 * ii);
    }

    // Build the tree structure (node->children).
    for (int ii = 0; ii < results.size(); ii++)
    {
        for (int jj = 0; jj < kids[ii].size(); jj++)
        {
            tree_st[ii].push_back(kids[ii][jj]);
        }
    }

    return;
}
vector<string> SQLFUNC::select_years()
{
    // Returns the list of (ascending, unique) years represented in the database.
    vector<string> results;
    string stmt = "SELECT DISTINCT Year FROM TCatalogueIndex";
    executor(stmt, results);
    vector<int> iresults(results.size());
    for (int ii = 0; ii < results.size(); ii++)
    {
        try
        {
            iresults[ii] = stoi(results[ii]);
        }
        catch (invalid_argument& ia)
        {
            err("stoi-sf.select_years");
        }
    }
    int count, itemp;
    string temp;
    do
    {
        count = 0;
        for (int ii = 0; ii < iresults.size() - 1; ii++)
        {
            if (iresults[ii + 1] < iresults[ii])
            {
                itemp = iresults[ii + 1];
                temp = results[ii + 1];
                iresults[ii + 1] = iresults[ii];
                results[ii + 1] = results[ii];
                iresults[ii] = itemp;
                results[ii] = temp;
                count++;
            }
        }
    } while (count > 0);
    return results;
}
void SQLFUNC::set_error_path(string errpath)
{
    error_path = errpath;
}
void SQLFUNC::sqlerr(string func)
{
    // Threadsafe error log function specific to SQL errors.
    lock_guard<mutex> lock(m_err);
    int errcode = sqlite3_errcode(db);
    const char* errmsg = sqlite3_errmsg(db);
    string serrmsg(errmsg);
    string message = timestamper() + " SQL ERROR #" + to_string(errcode) + ", in ";
    message += func + serrmsg + "\r\n";
    ERR.open(error_path, ofstream::app);
    ERR << message << endl;
    ERR.close();
    exit(EXIT_FAILURE);
}
bool SQLFUNC::table_exist(string tname)
{
    // Returns TRUE or FALSE as to the existance of a given table within the database.

    string stmt = "SELECT name FROM sqlite_master WHERE type='table' AND name='" + tname + "';";
    vector<string> results1;
    executor(stmt, results1);
    if (results1.size() > 0)
    {
        return 1;
    }
    return 0;
}
string SQLFUNC::timestamper()
{
    // Return a timestamp from the system clock.
    char buffer[26];
    string timestampA;
    chrono::system_clock::time_point today = chrono::system_clock::now();
    time_t tt = chrono::system_clock::to_time_t(today);
    ctime_s(buffer, 26, &tt);
    for (int ii = 0; ii < 26; ii++)
    {
        if (buffer[ii] == '\0') { break; }
        else { timestampA.push_back(buffer[ii]); }
    }
    return timestampA;
}
vector<string> SQLFUNC::test_cata(string cata_name)
{
    // Returns PASS or FAIL as to whether the given catalogue has the minimum 
    // number of tables (with entries) in the database. Each subsequent line 
    // in the report describes the number of rows found for each table tested.

    vector<string> test_results(6); 
    vector<int> iresults(5);

    vector<string> results1;
    vector<string> search = { "Name" };
    string tname = "TCatalogueIndex";
    vector<string> conditions = { "Name = '" + cata_name + "'" };
    select(search, tname, results1, conditions);
    iresults[0] = results1.size();
    test_results[1] = "TCatalogueIndex had " + to_string(iresults[0]) + " entries.";

    results1.clear();
    search = { "GID" };
    tname = "TG_Region$" + cata_name;
    select(search, tname, results1);
    iresults[1] = results1.size();
    test_results[2] = tname + " had " + to_string(iresults[1]) + " entries.";

    results1.clear();
    search = { "[Row Index]" };
    tname = "TG_Row$" + cata_name;
    select(search, tname, results1);
    iresults[2] = results1.size();
    test_results[3] = tname + " had " + to_string(iresults[2]) + " entries.";

    results1.clear();
    search = { "GID" };
    tname = cata_name;
    select(search, tname, results1);
    iresults[3] = results1.size();
    test_results[4] = tname + " had " + to_string(iresults[3]) + " entries.";

    vector<string> vtemp;
    results1.clear();
    vector<string> tall;
    all_tables(tall);
    for (int ii = 0; ii < tall.size(); ii++)
    {
        vtemp = jfsf.list_from_marker(tall[ii], '$');
        if (vtemp[1] == cata_name && vtemp.size() > 2)
        {
            results1.push_back(vtemp[0]);
        }
    }
    iresults[4] = results1.size();
    test_results[5] = cata_name + "$GID had " + to_string(iresults[4]) + " tables.";

    for (int ii = 0; ii < iresults.size(); ii++)
    {
        if (iresults[ii] < 1)
        {
            test_results[0] = "FAIL";
            break;
        }
        else if (ii == iresults.size() - 1)
        {
            test_results[0] = "PASS";
        }
    }
    return test_results;
}
