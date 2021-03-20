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
    jfsql.err(func);
}
void SQLFUNC::init(string db_path)
{
    int error = sqlite3_open_v2(db_path.c_str(), &db, (SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE), NULL);
    if (error) { sqlerr("open-init"); }
}
void SQLFUNC::insert_tg_existing(string tname)
{
    // Convenience function to facilitate the insertion of tables into TGenealogy. 
    // This function is only valid if the table in question already exists in the database.
    
    vector<string> row_data = jfsql.list_from_marker(tname, '$');
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
    if (error) { sqlerr("commit transaction-insert_prepared"); }
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
void SQLFUNC::select_tree(string tname, vector<vector<int>>& tree_st, vector<string>& tree_pl)
{
    // Produce a tree structure and tree payload for the given table name as root. 

    string temp, sparent;
    vector<string> tname_params = jfsql.list_from_marker(tname, '$');
    int num_params = tname_params.size() - 1;
    string stmt = "SELECT * FROM TGenealogy WHERE (";
    for (int ii = 1; ii < tname_params.size(); ii++)
    {
        stmt += "param" + to_string(ii) + " = '" + tname_params[ii] + "'";
        if (ii < tname_params.size() - 1)
        {
            stmt += " AND ";
        }
        else
        {
            stmt += ");";
        }
    }
    vector<vector<string>> results;
    executor(stmt, results);

    for (int ii = 0; ii < results.size(); ii++)
    {
        for (int jj = 0; jj < results[ii].size(); jj++)
        {
            if (results[ii][jj] == "")
            {
                results[ii].erase(results[ii].begin() + jj);
                jj--;
            }
        }
    }

    unordered_map<string, int> registry;
    int pl_index, iparent, pivot;
    size_t pos1;
    tree_pl.clear();
    tree_st.clear();
    while (results.size() > 0)
    {      
        for (int ii = 0; ii < results.size(); ii++)
        {
            if (results[ii].size() == num_params + 1)
            {
                // Move this node from the results to the tree.
                temp = results[ii][0];
                results.erase(results.begin() + ii);
                ii--;

                pl_index = tree_pl.size();
                tree_st.push_back(vector<int>());
                tree_pl.push_back(temp);
                registry.emplace(temp, pl_index);

                pos1 = temp.rfind('$');
                pos1 = temp.find_last_not_of('$', pos1);
                sparent = temp.substr(0, pos1 + 1);
                try
                {
                    iparent = registry.at(sparent);
                }
                catch (out_of_range& oor)
                {
                    tree_st[pl_index].push_back(-1 * pl_index);
                    continue;
                }

                tree_st[pl_index] = tree_st[iparent];
                for (int jj = 0; jj < tree_st[pl_index].size(); jj++)
                {
                    if (tree_st[pl_index][jj] < 0)
                    {
                        pivot = jj;
                        break;
                    }
                    else if (jj == tree_st[pl_index].size() - 1)
                    {
                        pivot = 0;
                    }
                }
                tree_st[pl_index].resize(pivot + 2);
                tree_st[pl_index][pivot] *= -1;
                tree_st[pl_index][pivot + 1] = -1 * pl_index;

                tree_st[iparent].push_back(pl_index);
            }
        }
        num_params++;
    }
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
    message += func + "\r\n" + serrmsg + "\r\n";
    ERR.open(error_path, ofstream::app);
    ERR << message << endl;
    ERR.close();
    exit(EXIT_FAILURE);
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
int SQLFUNC::tg_max_param()
{
    vector<string> column_titles;
    if (TG_max_param < 0)
    {
        get_col_titles("TGenealogy", column_titles);
        TG_max_param = column_titles.size() - 1;
    }
    return TG_max_param;
}
