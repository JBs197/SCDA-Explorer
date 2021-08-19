#include "sqlfunc.h"

using namespace std;

void SQLFUNC::all_tables(vector<string>& table_list)
{
    table_list.clear();
    string stmt = "SELECT name FROM sqlite_master WHERE type='table';";
    executor(stmt, table_list);
}
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
int SQLFUNC::count(string tname)
{
    // Return the number of rows in the given table. 
    string stmt = "SELECT COUNT(*) FROM \"" + tname + "\";";
    string sNum;
    int iNum;
    executor(stmt, sNum);
    try { iNum = stoi(sNum); }
    catch (invalid_argument) { jf.err("stoi-sf.count"); }
    return iNum;
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
void SQLFUNC::executor(string stmt)
{
    sqlite3_stmt* statement;
    int error = sqlite3_prepare_v2(db, stmt.c_str(), -1, &statement, NULL);
    if (error) { sqlerr("prepare-sf.executor"); }
    error = sqlite3_step(statement);
    if (error > 0 && error != 100 && error != 101) { sqlerr("step-sf.executor"); }
    error = sqlite3_finalize(statement);
    if (error > 0 && error != 100 && error != 101) { sqlerr("finalize-sf.executor"); }
}
void SQLFUNC::executor(vector<string> stmts)
{
    // Note this function will not execute the statements as a transaction on its own. 
    sqlite3_stmt* statement;
    int error;
    for (int ii = 0; ii < stmts.size(); ii++)
    {
        error = sqlite3_prepare_v2(db, stmts[ii].c_str(), -1, &statement, NULL);
        if (error) { sqlerr("prepare-sf.executor"); }
        error = sqlite3_step(statement);
        if (error > 0 && error != 100 && error != 101) { sqlerr("step-sf.executor"); }
        error = sqlite3_finalize(statement);
        if (error > 0 && error != 100 && error != 101) { sqlerr("finalize-sf.executor"); }
    }
}
void SQLFUNC::executor(string stmt, string& result)
{
    // Note that this variant of the executor function will only return the first result.
    int type, size, ivalue;  // Type: 1(int), 2(double), 3(string)
    sqlite3_stmt* statement;
    int error = sqlite3_prepare_v2(db, stmt.c_str(), -1, &statement, NULL);
    if (error) { sqlerr("prepare-sf.executor0"); }
    error = sqlite3_step(statement);
    double dvalue;
    string svalue;
    int iextra = 0;
    if (error == 100)
    {
        type = sqlite3_column_type(statement, 0);
        switch (type)
        {
        case 1:
            ivalue = sqlite3_column_int(statement, 0);
            result += to_string(ivalue);
            break;
        case 2:
            dvalue = sqlite3_column_double(statement, 0);
            result += to_string(dvalue);
            break;
        case 3:
        {
            size = sqlite3_column_bytes(statement, 0);
            const unsigned char* buffer = sqlite3_column_text(statement, 0);
            svalue.resize(size);
            for (int ii = 0; ii < size; ii++)
            {
                if (buffer[ii] > 127)
                {
                    svalue[ii + iextra] = -61;
                    iextra++;
                    svalue.insert(ii + iextra, 1, buffer[ii] - 64);
                }
                else
                {
                    svalue[ii + iextra] = buffer[ii];
                }
            }
            result += svalue;
            break;
        }
        case 5:
            result += "";
            break;
        }
    }
    else if (error > 0 && error != 101) { sqlerr("step-sf.executor0"); }
    error = sqlite3_finalize(statement);
    if (error > 0 && error != 100 && error != 101) { sqlerr("finalize-sf.executor0"); }
}
void SQLFUNC::executor(string stmt, wstring& result)
{
    // Note that this variant of the executor function will only return the first result.
    int type, size, ivalue;  // Type: 1(int), 2(double), 3(string)
    sqlite3_stmt* statement;
    int error = sqlite3_prepare_v2(db, stmt.c_str(), -1, &statement, NULL);
    if (error) { sqlerr("prepare-sf.executor0"); }
    error = sqlite3_step(statement);
    double dvalue;
    wstring wvalue;
    int iextra = 0;
    if (error == 100)
    {
        type = sqlite3_column_type(statement, 0);
        switch (type)
        {
        case 1:
            ivalue = sqlite3_column_int(statement, 0);
            result += to_wstring(ivalue);
            break;
        case 2:
            dvalue = sqlite3_column_double(statement, 0);
            result += to_wstring(dvalue);
            break;
        case 3:
        {
            size = sqlite3_column_bytes(statement, 0);
            const unsigned char* buffer = sqlite3_column_text(statement, 0);
            wvalue.resize(size);
            for (int ii = 0; ii < size; ii++)
            {
                if (buffer[ii] == 195)
                {
                    wvalue[ii + iextra] = (wchar_t)(buffer[ii + 1] + 64);
                    iextra--;
                    ii++;
                    wvalue.pop_back();
                }
                else
                {
                    wvalue[ii + iextra] = (wchar_t)buffer[ii];
                }
            }
            result += wvalue;
            break;
        }
        case 5:
            result += L"";
            break;
        }
    }
    else if (error > 0 && error != 101) { sqlerr("step-sf.executor0"); }
    error = sqlite3_finalize(statement);
    if (error > 0 && error != 100 && error != 101) { sqlerr("finalize-sf.executor0"); }
}
void SQLFUNC::executor(string stmt, vector<string>& results)
{
    // Note that this variant of the executor function can accomodate either a column or a row as the result.
    int type, size, ivalue, inum;  // Type: 1(int), 2(double), 3(string)
    sqlite3_stmt* statement;
    int error = sqlite3_prepare_v2(db, stmt.c_str(), -1, &statement, NULL);
    if (error) { sqlerr("prepare-executor1"); }
    error = sqlite3_step(statement);
    double dvalue;
    string svalue;
    int col_count = -1;
    int iextra = 0;
    while (error == 100)
    {
        if (col_count < 0)
        {
            col_count = sqlite3_column_count(statement);
        }
        if (col_count > 1)  // Returned vector will be a row.
        {
            inum = results.size();
            results.resize(inum + col_count);
            for (int ii = 0; ii < col_count; ii++)
            {
                type = sqlite3_column_type(statement, ii);
                switch (type)
                {
                case 1:
                    ivalue = sqlite3_column_int(statement, ii);
                    results[inum + ii] = to_string(ivalue);
                    break;
                case 2:
                    dvalue = sqlite3_column_double(statement, ii);
                    results[inum + ii] = to_string(dvalue);
                    break;
                case 3:
                {
                    size = sqlite3_column_bytes(statement, ii);
                    const unsigned char* buffer = sqlite3_column_text(statement, ii);
                    svalue.resize(size);
                    for (int ii = 0; ii < size; ii++)
                    {
                        if (buffer[ii] > 127 && buffer[ii] != 195)
                        {
                            if (ii == 0)
                            {
                                svalue[ii + iextra] = -61;
                                iextra++;
                                svalue.insert(ii + iextra, 1, buffer[ii] - 64);
                            }
                            else if (buffer[ii - 1] != 195)
                            {
                                svalue[ii + iextra] = -61;
                                iextra++;
                                svalue.insert(ii + iextra, 1, buffer[ii] - 64);
                            }
                            else
                            {
                                svalue[ii + iextra] = buffer[ii];
                            }
                        }
                        else
                        {
                            svalue[ii + iextra] = buffer[ii];
                        }
                    }
                    results[inum + ii] = svalue;
                    iextra = 0;
                    break;
                }
                case 5:
                    results[inum + ii] = "";
                    break;
                }
            }
        }
        else  // Returned result will be a column.
        {
            type = sqlite3_column_type(statement, 0);
            switch (type)
            {
            case 1:
                ivalue = sqlite3_column_int(statement, 0);
                results.push_back(to_string(ivalue));
                break;
            case 2:
                dvalue = sqlite3_column_double(statement, 0);
                results.push_back(to_string(dvalue));
                break;
            case 3:
            {
                size = sqlite3_column_bytes(statement, 0);
                const unsigned char* buffer = sqlite3_column_text(statement, 0);
                svalue.resize(size);
                for (int ii = 0; ii < size; ii++)
                {
                    if (buffer[ii] > 127 && buffer[ii] != 195)
                    {
                        if (ii == 0)
                        {
                            svalue[ii + iextra] = -61;
                            iextra++;
                            svalue[ii + iextra] = buffer[ii] - 64;
                            svalue.push_back(0);
                        }
                        else if (buffer[ii - 1] == 195)
                        {
                            svalue[ii + iextra] = buffer[ii];
                        }
                        else
                        {
                            svalue[ii + iextra] = -61;
                            iextra++;
                            svalue[ii + iextra] = buffer[ii] - 64;
                            svalue.push_back(0);
                        }
                    }
                    else
                    {
                        svalue[ii + iextra] = buffer[ii];
                    }
                }
                results.push_back(svalue);
                iextra = 0;
                break;
            }
            case 5:
                results.push_back("");
                break;
            }
        }
        error = sqlite3_step(statement);
    }
    if (error > 0 && error != 101) { sqlerr("step-executor1"); }
    error = sqlite3_finalize(statement);
    if (error > 0 && error != 100 && error != 101) { sqlerr("finalize-sf.executor1"); }
}
void SQLFUNC::executor(string stmt, vector<vector<string>>& results)
{
    int type, col_count, size, ivalue;  // Type: 1(int), 2(double), 3(string)
    sqlite3_stmt* statement;
    int error = sqlite3_prepare_v2(db, stmt.c_str(), -1, &statement, NULL);
    if (error) { sqlerr("prepare-executor2"); }
    error = sqlite3_step(statement);
    double dvalue;
    string svalue;
    int iextra = 0;
    while (error == 100)  // Output text should be UTF8.
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
            {
                size = sqlite3_column_bytes(statement, ii);
                const unsigned char* buffer = sqlite3_column_text(statement, ii);
                svalue.resize(size);
                for (int ii = 0; ii < size; ii++)
                {
                    if (buffer[ii] > 127 && buffer[ii] != 195)
                    {
                        if (ii == 0)
                        {
                            svalue[ii + iextra] = -61;
                            iextra++;
                            svalue.insert(svalue.begin() + ii + iextra, buffer[ii] - 64);
                            svalue.push_back(0);
                        }
                        else if (buffer[ii - 1] == 195)
                        {
                            svalue[ii + iextra] = buffer[ii];
                        }
                        else
                        {
                            svalue[ii + iextra] = -61;
                            iextra++;
                            svalue.insert(svalue.begin() + ii + iextra, buffer[ii] - 64);
                            svalue.push_back(0);
                        }
                    }
                    else
                    {
                        svalue[ii + iextra] = buffer[ii];
                    }
                }
                results[results.size() - 1][ii] = svalue;
                iextra = 0;
                break;
            }
            case 5:
                results[results.size() - 1][ii] = "";
                break;
            }
        }
        error = sqlite3_step(statement);
    }
    if (error > 0 && error != 101) { sqlerr("step-executor2"); }
    error = sqlite3_finalize(statement);
    if (error > 0 && error != 100 && error != 101) { sqlerr("finalize-sf.executor2"); }
}
void SQLFUNC::executor(string stmt, vector<vector<wstring>>& results)
{
    int type, col_count, size, ivalue;  // Type: 1(int), 2(double), 3(string)
    sqlite3_stmt* statement;
    int error = sqlite3_prepare_v2(db, stmt.c_str(), -1, &statement, NULL);
    if (error) { sqlerr("prepare-executor2"); }
    error = sqlite3_step(statement);
    double dvalue;
    wstring wvalue;
    int iextra = 0;
    while (error == 100)  // Output text should be UTF16.
    {
        col_count = sqlite3_column_count(statement);
        results.push_back(vector<wstring>(col_count));
        for (int ii = 0; ii < col_count; ii++)
        {
            type = sqlite3_column_type(statement, ii);
            switch (type)
            {
            case 1:
                ivalue = sqlite3_column_int(statement, ii);
                results[results.size() - 1][ii] = to_wstring(ivalue);
                break;
            case 2:
                dvalue = sqlite3_column_double(statement, ii);
                results[results.size() - 1][ii] = to_wstring(dvalue);
                break;
            case 3:
            {
                size = sqlite3_column_bytes(statement, ii);
                const unsigned char* buffer = sqlite3_column_text(statement, ii);
                wvalue.resize(size);
                for (int ii = 0; ii < size; ii++)
                {
                    if (buffer[ii] == 195)
                    {
                        wvalue[ii + iextra] = (wchar_t)(buffer[ii + 1] + 64);
                        iextra--;
                        ii++;
                        wvalue.pop_back();
                    }
                    else
                    {
                        wvalue[ii + iextra] = (wchar_t)buffer[ii];
                    }
                }
                results[results.size() - 1][ii] = wvalue;
                iextra = 0;
                break;
            }
            case 5:
                results[results.size() - 1][ii] = L"";
                break;
            }
        }
        error = sqlite3_step(statement);
    }
    if (error > 0 && error != 101) { sqlerr("step-executor2"); }
    error = sqlite3_finalize(statement);
    if (error > 0 && error != 100 && error != 101) { sqlerr("finalize-sf.executor2"); }
}
void SQLFUNC::get_col_titles(string tname, vector<string>& titles)
{
    vector<vector<string>> results;
    string stmt = "PRAGMA table_info('" + tname + "');";
    executor(stmt, results);
    titles.resize(results.size());
    for (int ii = 0; ii < results.size(); ii++)
    {
        titles[ii] = results[ii][1];
    }
}
string SQLFUNC::getLinearizedTitle(string& cataName, string& rowTitle, string& colTitle)
{
    vector<string> columnTitles;
    get_col_titles(cataName, columnTitles);
    int count = 0;
    while (rowTitle[count] == '+') { count++; }
    return columnTitles[0];
}
int SQLFUNC::getNumRows(string tname)
{
    string stmt = "SELECT COUNT(*) FROM [";
    stmt += tname + "];";
    string result;
    executor(stmt, result);
    int numRows;
    try { numRows = stoi(result); }
    catch (invalid_argument& ia) { jf.err("stoi-sf.getNumRows"); }
    return numRows;
}
vector<string> SQLFUNC::getTableListFromRoot(string& root)
{
    vector<string> tList;
    all_tables(tList);
    size_t pos1;
    for (int ii = tList.size() - 1; ii >= 0; ii--)
    {
        pos1 = tList[ii].rfind(root, 1);
        if (pos1 == 0) { continue; }
        tList.erase(tList.begin() + ii);
    }
    return tList;
}
int SQLFUNC::get_num_col(string tname)
{
    vector<string> column_titles;
    get_col_titles(tname, column_titles);
    return column_titles.size();
}
void SQLFUNC::get_table_list(vector<string>& results, string& search)
{
    // NOTE: Obsolete function kept for the sustenance of near-obsolete functions.
    results.clear();
    vector<string> search_split = jf.list_from_marker(search, '$');
    vector<string> chaff_split;
    for (int ii = 0; ii < tableList.size(); ii++)
    {
        chaff_split = jf.list_from_marker(tableList[ii], '$');
        if (chaff_split.size() < search_split.size()) { continue; }
        for (int jj = 1; jj < search_split.size(); jj++)
        {
            if (search_split[jj] != chaff_split[jj]) { break; }
            else if (jj == search_split.size() - 1)
            {
                results.push_back(tableList[ii]);
            }
        }
    }
}
vector<string> SQLFUNC::getTableList(string search)
{
    // Uses '*' as a wildcard symbol. If no '$' parameter char is given, 
    // then the wildcard must be the first or last char in the search string.
    vector<string> sList, cataList, vsTemp, gidList;
    string temp, Tprefix, Tsuffix;
    size_t pos1, pos2;
    int gid = -1;
    if (search.size() > 0)
    {
        pos1 = search.find('*');
        if (pos1 < search.size())
        {
            vsTemp = { "Name" };
            temp = "TCatalogueIndex";
            select(vsTemp, temp, cataList);

            pos2 = search.find('$');
            if (pos2 < search.size())
            {
                if (pos1 < pos2)
                {
                    Tsuffix = search.substr(pos2 + 1);
                    if (table_exist("TG_Region$" + Tsuffix)) { sList.push_back("TG_Region$" + Tsuffix); }
                    if (table_exist("TG_Row$" + Tsuffix)) { sList.push_back("TG_Row$" + Tsuffix); }
                    if (table_exist("TMap$" + Tsuffix)) { sList.push_back("TMap$" + Tsuffix); }
                    
                    try { gid = stoi(Tsuffix); }
                    catch (invalid_argument) {}
                    if (gid >= 0)
                    {
                        for (int ii = 0; ii < cataList.size(); ii++)
                        {
                            if (table_exist(cataList[ii] + "$" + Tsuffix)) { sList.push_back(cataList[ii] + "$" + Tsuffix); }
                        }
                    }

                    if (Tsuffix == "Geo" || Tsuffix == "Geo_Layers")
                    {
                        for (int ii = 0; ii < cataList.size(); ii++)
                        {
                            if (table_exist(cataList[ii] + "$" + Tsuffix)) { sList.push_back(cataList[ii] + "$" + Tsuffix); }
                        }
                    }
                }
                else
                {
                    Tprefix = search.substr(0, pos2);
                    if (Tprefix == "TG_Region" || Tprefix == "TG_Row" || Tprefix == "TMap")
                    {
                        for (int ii = 0; ii < cataList.size(); ii++)
                        {
                            if (table_exist(Tprefix + "$" + cataList[ii])) { sList.push_back(Tprefix + "$" + cataList[ii]); }
                        }
                    }
                    else if (table_exist(Tprefix))  // Catalogue name was given.
                    {
                        sList.push_back(Tprefix);
                        if (table_exist(Tprefix + "$Geo")) { sList.push_back(Tprefix + "$Geo"); }
                        if (table_exist(Tprefix + "$Geo_Layers")) { sList.push_back(Tprefix + "$Geo_Layers"); }
                        temp = "TG_Region$" + Tprefix;
                        if (table_exist(temp))
                        {
                            vsTemp = { "GID" };
                            select(vsTemp, temp, gidList);
                            for (int ii = 0; ii < gidList.size(); ii++)
                            {
                                if (table_exist(Tprefix + "$" + gidList[ii])) 
                                { 
                                    sList.push_back(Tprefix + "$" + gidList[ii]); 
                                }
                            }
                        }
                    }
                }
            }
            else if (pos1 == 0)
            {
                Tsuffix = search.substr(1);
                if (table_exist("TG_Region$" + Tsuffix)) { sList.push_back("TG_Region$" + Tsuffix); }
                if (table_exist("TG_Row$" + Tsuffix)) { sList.push_back("TG_Row$" + Tsuffix); }
                if (table_exist("TMap$" + Tsuffix)) { sList.push_back("TMap$" + Tsuffix); }

                try { gid = stoi(Tsuffix); }
                catch (invalid_argument) {}
                if (gid >= 0)
                {
                    for (int ii = 0; ii < cataList.size(); ii++)
                    {
                        if (table_exist(cataList[ii] + "$" + Tsuffix)) { sList.push_back(cataList[ii] + "$" + Tsuffix); }
                    }
                }

                if (Tsuffix == "Geo" || Tsuffix == "Geo_Layers")
                {
                    for (int ii = 0; ii < cataList.size(); ii++)
                    {
                        if (table_exist(cataList[ii] + "$" + Tsuffix)) { sList.push_back(cataList[ii] + "$" + Tsuffix); }
                    }
                }
            }
            else if (pos1 == search.size() - 1)
            {
                Tprefix = search.substr(0, search.size() - 1);
                if (Tprefix == "TG_Region" || Tprefix == "TG_Row" || Tprefix == "TMap")
                {
                    for (int ii = 0; ii < cataList.size(); ii++)
                    {
                        if (table_exist(Tprefix + "$" + cataList[ii])) { sList.push_back(Tprefix + "$" + cataList[ii]); }
                    }
                }
                else if (table_exist(Tprefix))  // Catalogue name was given.
                {
                    sList.push_back(Tprefix);
                    if (table_exist(Tprefix + "$Geo")) { sList.push_back(Tprefix + "$Geo"); }
                    if (table_exist(Tprefix + "$Geo_Layers")) { sList.push_back(Tprefix + "$Geo_Layers"); }
                    temp = "TG_Region$" + Tprefix;
                    if (table_exist(temp))
                    {
                        vsTemp = { "GID" };
                        select(vsTemp, temp, gidList);
                        for (int ii = 0; ii < gidList.size(); ii++)
                        {
                            if (table_exist(Tprefix + "$" + gidList[ii]))
                            {
                                sList.push_back(Tprefix + "$" + gidList[ii]);
                            }
                        }
                    }
                }
            }
            else 
            { 
                sList.push_back("Invalid search criterion.");
            }
        }
        else
        {
            if (table_exist(search)) { sList.push_back(search); }
            else { sList.push_back("No results found."); }
        }
    }
    else { return tableList; }
    return sList;
}
vector<vector<string>> SQLFUNC::getTMapIndex()
{
    vector<vector<string>> TMI;
    string stmt = "SELECT * FROM TMapIndex ORDER BY numParams ASC NULLS LAST;";
    executor(stmt, TMI);
    return TMI;
}
void SQLFUNC::init(string db_path)
{
    dbPath = db_path;
    int error = sqlite3_open_v2(db_path.c_str(), &db, (SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE), NULL);
    if (error) { sqlerr("open-init"); }
    if (analyze)
    {
        string stmt = "PRAGMA optimize;";
        executor(stmt);
    }
    all_tables(tableList);
    for (int ii = 0; ii < tableList.size(); ii++)
    {
        tableSet.emplace(tableList[ii]);
    }
}
void SQLFUNC::insert(string tname, vector<string>& row_data)
{
    vector<string> column_titles;
    get_col_titles(tname, column_titles);
    if (column_titles.size() > row_data.size())
    {
        column_titles.resize(row_data.size());
    }
    else if (column_titles.size() < row_data.size())
    {
        safe_col(tname, row_data.size());
    }
    string stmt0 = "INSERT INTO [" + tname + "] (";
    for (int ii = 0; ii < column_titles.size(); ii++)
    {
        stmt0 += "[" + column_titles[ii];
        if (ii < column_titles.size() - 1)
        {
            stmt0 += "], ";
        }
        else
        {
            stmt0 += "]) VALUES (";
        }
    }
    for (int ii = 0; ii < column_titles.size(); ii++)
    {
        stmt0 += "?, ";
    }
    stmt0.pop_back();
    stmt0.pop_back();
    stmt0 += ");";

    bind(stmt0, row_data);
    executor(stmt0);
}
void SQLFUNC::insert(string tname, vector<vector<string>>& row_data)
{
    vector<string> column_titles;
    get_col_titles(tname, column_titles);
    if (column_titles.size() > row_data.size())
    {
        column_titles.resize(row_data.size());
    }
    else if (column_titles.size() < row_data.size())
    {
        safe_col(tname, row_data.size());
    }
    string stmt0 = "INSERT INTO [" + tname + "] (";
    for (int ii = 0; ii < column_titles.size(); ii++)
    {
        stmt0 += "[" + column_titles[ii];
        if (ii < column_titles.size() - 1)
        {
            stmt0 += "], ";
        }
        else
        {
            stmt0 += "]) VALUES (";
        }
    }
    for (int ii = 0; ii < column_titles.size(); ii++)
    {
        stmt0 += "?, ";
    }
    stmt0.pop_back();
    stmt0.pop_back();
    stmt0 += ");";

    string stmt;
    int error = sqlite3_exec(db, "BEGIN EXCLUSIVE TRANSACTION", NULL, NULL, NULL);
    if (error) { sqlerr("begin transaction-insert_rows"); }
    for (int ii = 0; ii < row_data.size(); ii++)
    {
        stmt = stmt0;
        bind(stmt, row_data[ii]);
        executor(stmt);
    }
    error = sqlite3_exec(db, "COMMIT TRANSACTION", NULL, NULL, NULL);
    if (error) { sqlerr("commit transaction-insert_rows"); }
}
void SQLFUNC::insertBinMap(string& binPath, vector<vector<vector<int>>>& frames, double& scale, vector<double>& position, string& sParent8, vector<vector<int>>& border)
{
    // Make the table name root.
    vector<string> directory;
    size_t pos1 = binPath.rfind(".bin");
    string temp = binPath.substr(0, pos1);
    string binPath8 = jf.asciiToUTF8(temp);
    pos1 = binPath8.find("mapsBIN");
    pos1 = binPath8.find('\\', pos1) + 1;
    size_t pos2 = binPath8.find('\\', pos1);
    while (pos2 < binPath8.size())
    {
        directory.push_back(binPath8.substr(pos1, pos2 - pos1));
        pos1 = pos2 + 1;
        pos2 = binPath8.find('\\', pos1);
    }
    directory.push_back(binPath8.substr(pos1));
    string tname0 = "TMap$", tname, stmt;
    for (int ii = 0; ii < directory.size(); ii++)
    {
        tname0 += directory[ii] + "$";
    }

    // Make and insert the table for frames.
    tname = tname0 + "frames";
    vector<string> columnTitles = { "map", "scale", "position" }, rowData(3);
    vector<int> columnTypes = { 1, 1, 1 };
    int indexTLBR, indexXY, numRows, error;
    create_table(tname, columnTitles, columnTypes);
    numRows = getNumRows(tname);
    if (!numRows)
    {
        for (int ii = 0; ii < 4; ii++)
        {
            if (ii < 2) { indexTLBR = 0; }
            else { indexTLBR = 1; }
            indexXY = ii % 2;
            for (int jj = 0; jj < 3; jj++)
            {
                rowData[jj] = to_string(frames[jj][indexTLBR][indexXY]);
            }
            stmt = insert_stmt(tname, columnTitles, rowData);
            executor(stmt);
        }
    }

    // Make and insert the table for scale.
    tname = tname0 + "scale";
    columnTitles = { "Pixels Per km" };
    columnTypes = { 1 };
    create_table(tname, columnTitles, columnTypes);
    numRows = getNumRows(tname);
    if (!numRows)
    {
        rowData = { to_string(scale) };
        stmt = insert_stmt(tname, columnTitles, rowData);
        executor(stmt);
    }

    // Make and insert the table for position (minimap). Maybe.
    size_t sizePos = position.size();
    if (sizePos == 2)
    {
        tname = tname0 + "position";
        columnTitles = { "WH Fraction" };
        columnTypes = { 1 };
        create_table(tname, columnTitles, columnTypes);
        numRows = getNumRows(tname);
        if (!numRows)
        {
            for (int ii = 0; ii < 2; ii++)
            {
                rowData = { to_string(position[ii]) };
                stmt = insert_stmt(tname, columnTitles, rowData);
                executor(stmt);
            }
        }
    }
    else
    {
        tname = tname0 + "position";
        columnTitles = { "WH Fraction" };
        columnTypes = { 1 };
        create_table(tname, columnTitles, columnTypes);
        numRows = getNumRows(tname);
        if (!numRows)
        {
            rowData = { "-1.0" };
            stmt = insert_stmt(tname, columnTitles, rowData);
            executor(stmt);
        }
    }

    // Make and insert the table for parent.
    pos1 = tname0.rfind('$');
    pos1 = tname0.rfind('$', pos1 - 1);
    pos2 = tname0.find("(Canada)", pos1);
    if (sParent8.size() > 0)
    {
        tname = tname0 + "parent";
        columnTitles = { "Region Name" };
        columnTypes = { 0 };
        create_table(tname, columnTitles, columnTypes);
        numRows = getNumRows(tname);
        if (!numRows)
        {
            rowData = { sParent8 };
            stmt = insert_stmt(tname, columnTitles, rowData);
            executor(stmt);
        }
    }
    else
    {
        tname = tname0 + "parent";
        columnTitles = { "Region Name" };
        columnTypes = { 0 };
        create_table(tname, columnTitles, columnTypes);
        numRows = getNumRows(tname);
        if (!numRows)
        {
            if (pos2 > tname0.size()) { rowData = { "None" }; }
            else { rowData = { "Canada" }; }
            stmt = insert_stmt(tname, columnTitles, rowData);
            executor(stmt);
        }
    }


    // Make and insert the table for border. Inserted via transaction.
    tname = tname0 + "border";
    columnTitles = { "xCoord", "yCoord" };
    columnTypes = { 1, 1 };
    create_table(tname, columnTitles, columnTypes);
    numRows = getNumRows(tname);
    if (!numRows)
    {
        error = sqlite3_exec(db, "BEGIN EXCLUSIVE TRANSACTION", NULL, NULL, NULL);
        if (error) { sqlerr("begin transaction-sf.insertBinMaps"); }
        for (int ii = 0; ii < border.size(); ii++)
        {
            rowData = { to_string(border[ii][0]), to_string(border[ii][1]) };
            stmt = insert_stmt(tname, columnTitles, rowData);
            executor(stmt);
        }
        error = sqlite3_exec(db, "COMMIT TRANSACTION", NULL, NULL, NULL);
        if (error) { sqlerr("commit transaction-sf.insertBinMaps"); }
    }
    
    // Insert a new row in TMapIndex, if necessary.
    pos1 = tname0.find('$') + 1;
    string myCoreDir = tname0.substr(pos1);
    myCoreDir.pop_back();  // Remove trailing '$'.
    if (myCoreDir == "Canada") { return; }
    vector<string> vSearch = { "coreDir" };
    tname = "TMapIndex";
    vector<string> conditions = { "coreDir LIKE '" + myCoreDir + "'" };
    temp.clear();
    select(vSearch, tname, temp, conditions);
    if (temp.size() < 1)  // This row does not exist yet.
    {
        insertTMI(myCoreDir);
    }

}
void SQLFUNC::insertGeo(string cataName, vector<int>& gidList, vector<string>& regionList, vector<string>& layerList, vector<string>& geoLayers)
{
    if (geoLayers[0] == "") { geoLayers[0] = "canada"; }
    if (layerList[0] == "") { layerList[0] = "canada"; }

    // Make and fill the Geo_Layers table.
    string tname = cataName + "$Geo_Layers";
    vector<string> colTitles = { "Layer" }, stmts, row(3), vsTemp;
    string stmt = "CREATE TABLE [" + tname + "] (Layer TEXT);";
    executor(stmt);
    for (int ii = 0; ii < geoLayers.size(); ii++)
    {
        vsTemp = { geoLayers[ii] };
        stmt = insert_stmt(tname, colTitles, vsTemp);
        executor(stmt);
    }

    // Make and fill the Geo table.
    tname = cataName + "$Geo";
    stmt = "CREATE TABLE [" + tname + "] (";
    stmt += "GID INTEGER PRIMARY KEY, [Region Name] TEXT, ";
    stmt += "Layer TEXT);";
    executor(stmt);
    string stmt0 = "INSERT INTO [" + tname + "] (GID, [Region Name], Layer)";
    stmt0 += " VALUES (?, ?, ?);";
    stmts.resize(gidList.size());
    for (int ii = 0; ii < gidList.size(); ii++)
    {
        row = { to_string(gidList[ii]), regionList[ii], layerList[ii] };
        stmt = stmt0;
        bind(stmt, row);
        executor(stmt);
    }

}
void SQLFUNC::insert_tg_existing(string tname)
{
    // Still needed???

    // Convenience function to facilitate the insertion of tables into TGenealogy. 
    // This function is only valid if the table in question already exists in the database.
    
    vector<string> row_data = jf.list_from_marker(tname, '$');
    safe_col("TGenealogy", row_data.size());
    insert("TGenealogy", row_data);
}
void SQLFUNC::insert_prepared(vector<string>& stmts)
{
    // Execute a list of prepared SQL statements as one transaction batch.
    int error = sqlite3_exec(db, "BEGIN EXCLUSIVE TRANSACTION", NULL, NULL, NULL);
    if (error) { sqlerr("begin transaction-insert_prepared"); }
    executor(stmts);
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
    error = sqlite3_db_release_memory(db);
    if (error) { sqlerr("release memory-insert_prepared"); }
}
void SQLFUNC::insertPreparedBind(vector<string>& stmtAndParams)
{
    int numParam = 0, index = 0;
    size_t pos1 = stmtAndParams[0].find('@');
    while (pos1 < stmtAndParams[0].size())
    {
        numParam++;
        pos1 = stmtAndParams[0].find('@', pos1 + 1);
    }
    if (stmtAndParams.size() % numParam != 1) { jf.err("Size mismatch-sf.insertPreparedBind"); }

    sqlite3_stmt* statement;
    int bufSize = stmtAndParams[0].size();
    int error = sqlite3_prepare_v2(db, stmtAndParams[0].c_str(), bufSize, &statement, NULL);
    if (error) { sqlerr("prepare-sf.insertPreparedBind"); }

    error = sqlite3_exec(db, "BEGIN EXCLUSIVE TRANSACTION", NULL, NULL, NULL);
    if (error) { sqlerr("begin transaction-sf.insertPreparedBind"); }
    while (index < stmtAndParams.size() - 1)
    {
        for (int ii = 1; ii <= numParam; ii++)
        {
            error = sqlite3_bind_text(statement, ii, stmtAndParams[index + ii].c_str(), stmtAndParams[index + ii].size(), SQLITE_TRANSIENT);
            if (error) { sqlerr("bind_text-sf.insertPreparedBind"); }
        }
        error = sqlite3_step(statement);
        if (error > 0 && error != 101) { sqlerr("step-sf.insertPreparedBind"); }
        error = sqlite3_clear_bindings(statement);
        if (error > 0 && error != 101) { sqlerr("clear_bindings-sf.insertPreparedBind"); }
        error = sqlite3_reset(statement);
        if (error > 0 && error != 101) { sqlerr("reset-sf.insertPreparedBind"); }
        index += numParam;
    }
    error = sqlite3_exec(db, "COMMIT TRANSACTION", NULL, NULL, NULL);
    if (error) { sqlerr("commit transaction-sf.insertPreparedBind"); }
}
void SQLFUNC::insertPreparedStartStop(vector<string>& stmts, int start, int stop)
{
    // Execute an interval within a list of prepared SQL statements, as one transaction batch.
    int error = sqlite3_exec(db, "BEGIN EXCLUSIVE TRANSACTION", NULL, NULL, NULL);
    if (error) { sqlerr("begin transaction-insertPreparedStartStop"); }
    for (int ii = start; ii <= stop; ii++)
    {
        sqlite3_stmt* statement;
        int error = sqlite3_prepare_v2(db, stmts[ii].c_str(), -1, &statement, NULL);
        if (error) { sqlerr("prepare-insertPreparedStartStop"); }
        error = sqlite3_step(statement);
        if (error > 0 && error != 100 && error != 101) { sqlerr("step-insertPreparedStartStop"); }
    }
    error = sqlite3_exec(db, "COMMIT TRANSACTION", NULL, NULL, NULL);
    if (error)
    {
        if (error != 5)
        {
            sqlerr("commit transaction-insertPreparedStartStop");
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
    if (column_titles.size() != row_data.size()) { jf.err("Parameter size mismatch-sf.insert_stmt"); }
    string stmt = "INSERT OR IGNORE INTO [" + tname + "] (";
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
void SQLFUNC::insertTMI(string myCoreDir)
{
    sclean(myCoreDir, 1);
    vector<string> params = jf.list_from_marker(myCoreDir, '$');
    int numParams = params.size() - 1;
    string tname = "TMapIndex";
    string stmt = "INSERT OR IGNORE INTO TMapIndex (coreDir, numParams, ";
    stmt += "param1, param2, param3, param4) VALUES ('";
    stmt += myCoreDir + "', '" + to_string(numParams);
    for (int ii = 1; ii <= 4; ii++)
    {
        if (ii <= numParams)
        {
            stmt += "', '" + params[ii];
        }
        else
        {
            stmt += "', '";
        }
    }
    stmt += "');";
    executor(stmt);
}
void SQLFUNC::makeANSI(string& task)
{
    for (int ii = 0; ii < task.size(); ii++)
    {
        if (task[ii] == -61)
        {
            task.erase(task.begin() + ii);
            task[ii] += 64;
        }
    }
}
unordered_map<string, string> SQLFUNC::makeMapDataIndex(string tname)
{
    // Returns a map connecting col1$col2$...$colN -> col0
    vector<string> search = { "*" };
    vector<vector<string>> vvsResult;
    select(search, tname, vvsResult);
    unordered_map<string, string> mapDataIndex;
    if (vvsResult.size() < 1) { jf.err("Table is empty-sf.makeMapDataIndex"); }
    else if (vvsResult.size() == 1) { return mapDataIndex; }
    string params;
    for (int ii = 0; ii < vvsResult.size(); ii++)
    {
        params = vvsResult[ii][1];
        for (int jj = 2; jj < vvsResult[ii].size(); jj++)
        {
            params += "$" + vvsResult[ii][jj];
        }
        mapDataIndex.emplace(params, vvsResult[ii][0]);
    }
    return mapDataIndex;
}
void SQLFUNC::remove(string& tname)
{
    string stmt = "DROP TABLE IF EXISTS [" + tname + "];";
    if (table_exist(tname))
    {
        executor(stmt);
        tableSet.erase(tname);
    }
    else
    {
        jf.log("Could not delete " + tname + " : could not find.");
    }
}
void SQLFUNC::removeCol(string& tname, string colTitle)
{
    string stmt = "ALTER TABLE \"" + tname + "\" DROP COLUMN \"" + colTitle + "\";";
    if (table_exist(tname))
    {
        executor(stmt);
    }
    else
    {
        jf.log("Could not delete column from " + tname + " : could not find table.");
    }
}
void SQLFUNC::removeRow(string& tname, vector<string>& conditions)
{
    string stmt = "DELETE FROM \"" + tname + "\" WHERE (";
    for (int ii = 0; ii < conditions.size(); ii++)
    {
        stmt += conditions[ii] + " ";
    }
    stmt += ");";
    if (table_exist(tname))
    {
        executor(stmt);
    }
    else
    {
        jf.log("Could not delete " + tname + " : could not find.");
    }
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
    // mode 2 is for conditional 'select' statements.
    bool skip = 0;
    int count = 0;
    size_t pos1, pos2, posLIKE;
    string temp;

    pos1 = bbq.find('[');
    if (pos1 < bbq.size())
    {
        pos2 = bbq.find(']', pos1);
        if (pos2 > bbq.size()) { jf.err("Square bracket asymmetry-sf.sclean"); }
        bbq.erase(pos1, pos2 - pos1 + 1);
    }
    while (1)
    {
        if (bbq.front() == ' ') { bbq.erase(0, 1); count++; }
        else { break; }
    }
    while (1)
    {
        if (bbq.back() == ' ') { bbq.pop_back(); }
        else { break; }
    }

    switch (mode)
    {
    case 0:
        break;
    case 1:  // Double ALL single quotation marks.
    {
        pos1 = bbq.find('\'');
        while (pos1 < bbq.size())
        {
            if (pos1 == bbq.size() - 1)
            {
                bbq.push_back('\'');
                break;
            }
            else if (bbq[pos1 + 1] == '\'')  // Already doubled.
            {
                if (pos1 == bbq.size() - 2) { break; }
                pos1 = bbq.find('\'', pos1 + 2);
            }
            else  // Needs doubling.
            {
                bbq.insert(bbq.begin() + pos1, '\'');
                if (pos1 < bbq.size() - 2)
                {
                    pos1 = bbq.find('\'', pos1 + 2);
                }
                else { break; }
            }
        }
        break;
    }
    case 2:  // This variant is used for conditional statements.
    {
        posLIKE = bbq.find("LIKE");
        if (posLIKE > bbq.size()) { break; }

        pos1 = bbq.find('\'', posLIKE + 6);
        while (pos1 < bbq.size())
        {
            if (pos1 == bbq.size() - 1) { break; }
            bbq.insert(bbq.begin() + pos1, '\'');
            pos1 = bbq.find('\'', pos1 + 2);
        }

        if (bbq[posLIKE + 5] != '\'')
        {
            bbq.insert(bbq.begin() + posLIKE + 5, '\'');
        }
        if (bbq.back() != '\'')
        {
            bbq.push_back('\'');
        }
        break;
    }
    }

    return count;
}
int SQLFUNC::select(vector<string> search, string tname, string& result)
{
    string stmt = "SELECT ";
    if (search[0] == "*" && search.size() == 1)
    {
        stmt += "* FROM \"" + tname + "\"";
    }
    else
    {
        stmt += "\"" + search[0] + "\" FROM \"" + tname + "\"";
    }
    executor(stmt, result);
    return 1;
}
int SQLFUNC::select(vector<string> search, string tname, vector<string>& results)
{
    string stmt = "SELECT ";
    if (search[0] == "*" && search.size() == 1)
    {
        stmt += "* FROM \"" + tname + "\"";
    }
    else
    {
        for (int ii = 0; ii < search.size(); ii++)
        {
            stmt += "\"" + search[ii] + "\", ";
        }
        stmt.erase(stmt.size() - 2, 2);
        stmt += " FROM \"" + tname + "\"";
    }
    executor(stmt, results);
    return results.size();
}
int SQLFUNC::select(vector<string> search, string tname, vector<vector<string>>& results)
{
    string stmt = "SELECT ";
    if (search[0] == "*" && search.size() == 1)
    {
        stmt += "* FROM \"" + tname + "\"";
    }
    else
    {
        for (int ii = 0; ii < search.size(); ii++)
        {
            stmt += "\"" + search[ii] + "\", ";
        }
        stmt.erase(stmt.size() - 2, 2);
        stmt += " FROM \"" + tname + "\"";
    }
    executor(stmt, results);
    int max_col = 0;
    for (int ii = 0; ii < results.size(); ii++)
    {
        if (results[ii].size() > max_col)
        {
            max_col = results[ii].size();
        }
    }
    return max_col;
}
int SQLFUNC::select(vector<string> search, string tname, vector<vector<wstring>>& results)
{
    string stmt = "SELECT ";
    if (search[0] == "*" && search.size() == 1)
    {
        stmt += "* FROM \"" + tname + "\"";
    }
    else
    {
        for (int ii = 0; ii < search.size(); ii++)
        {
            stmt += "\"" + search[ii] + "\", ";
        }
        stmt.erase(stmt.size() - 2, 2);
        stmt += " FROM \"" + tname + "\"";
    }
    executor(stmt, results);
    int max_col = 0;
    for (int ii = 0; ii < results.size(); ii++)
    {
        if (results[ii].size() > max_col)
        {
            max_col = results[ii].size();
        }
    }
    return max_col;
}
int SQLFUNC::select(vector<string> search, string tname, string& result, vector<string>& conditions)
{
    for (int ii = 0; ii < conditions.size(); ii++)
    {
        sclean(conditions[ii], 2);
    }
    string stmt = "SELECT ";
    if (search[0] == "*" && search.size() == 1)
    {
        stmt += "* FROM \"" + tname + "\"";
    }
    else
    {
        stmt += "\"" + search[0] + "\" FROM \"" + tname + "\"";
    }
    stmt += " WHERE (";
    for (int ii = 0; ii < conditions.size(); ii++)
    {
        stmt += conditions[ii] + " ";
    }
    stmt.pop_back();
    stmt += ");";
    executor(stmt, result);
    return 1;
}
int SQLFUNC::select(vector<string> search, string tname, wstring& result, vector<string>& conditions)
{
    for (int ii = 0; ii < conditions.size(); ii++)
    {
        sclean(conditions[ii], 2);
    }
    string stmt = "SELECT ";
    if (search[0] == "*" && search.size() == 1)
    {
        stmt += "* FROM \"" + tname + "\"";
    }
    else
    {
        stmt += "\"" + search[0] + "\" FROM \"" + tname + "\"";
    }
    stmt += " WHERE (";
    for (int ii = 0; ii < conditions.size(); ii++)
    {
        stmt += conditions[ii] + " ";
    }
    stmt += ");";
    executor(stmt, result);
    return 1;
}
int SQLFUNC::select(vector<string> search, string tname, vector<string>& results, vector<string>& conditions)
{
    for (int ii = 0; ii < conditions.size(); ii++)
    {
        sclean(conditions[ii], 2);
    }
    string stmt = "SELECT ";
    if (search[0] == "*" && search.size() == 1)
    {
        stmt += "* FROM \"" + tname + "\" WHERE (";
    }
    else
    {
        for (int ii = 0; ii < search.size(); ii++)
        {
            stmt += "\"" + search[ii] + "\", ";
        }
        stmt.erase(stmt.size() - 2, 2);
        stmt += " FROM \"" + tname + "\" WHERE (";
    }
    for (int ii = 0; ii < conditions.size(); ii++)
    {
        stmt += conditions[ii] + " ";
    }
    stmt += ");";
    executor(stmt, results);
    return results.size();
}
int SQLFUNC::select(vector<string> search, string tname, vector<vector<string>>& results, vector<string>& conditions)
{
    for (int ii = 0; ii < conditions.size(); ii++)
    {
        sclean(conditions[ii], 2);
    }
    string stmt = "SELECT ";
    if (search[0] == "*" && search.size() == 1)
    {
        stmt += "* FROM \"" + tname + "\" WHERE (";
    }
    else
    {
        for (int ii = 0; ii < search.size(); ii++)
        {
            stmt += "\"" + search[ii] + "\", ";
        }
        stmt.erase(stmt.size() - 2, 2);
        stmt += " FROM \"" + tname + "\" WHERE (";
    }
    for (int ii = 0; ii < conditions.size(); ii++)
    {
        stmt += conditions[ii] + " ";
    }
    stmt += ");";
    executor(stmt, results);
    int max_col = 0;
    for (int ii = 0; ii < results.size(); ii++)
    {
        if (results[ii].size() > max_col)
        {
            max_col = results[ii].size();
        }
    }
    return max_col;
}
int SQLFUNC::select(vector<string> search, string tname, vector<vector<wstring>>& results, vector<string>& conditions)
{
    for (int ii = 0; ii < conditions.size(); ii++)
    {
        sclean(conditions[ii], 2);
    }
    string stmt = "SELECT ";
    if (search[0] == "*" && search.size() == 1)
    {
        stmt += "* FROM \"" + tname + "\" WHERE (";
    }
    else
    {
        for (int ii = 0; ii < search.size(); ii++)
        {
            stmt += "\"" + search[ii] + "\", ";
        }
        stmt.erase(stmt.size() - 2, 2);
        stmt += " FROM \"" + tname + "\" WHERE (";
    }
    for (int ii = 0; ii < conditions.size(); ii++)
    {
        stmt += conditions[ii] + " ";
    }
    stmt += ");";
    executor(stmt, results);
    int max_col = 0;
    for (int ii = 0; ii < results.size(); ii++)
    {
        if (results[ii].size() > max_col)
        {
            max_col = results[ii].size();
        }
    }
    return max_col;
}
int SQLFUNC::selectOrderBy(vector<string> search, string tname, vector<string>& results, string orderby)
{
    string stmt = "SELECT ";
    if (search[0] == "*" && search.size() == 1)
    {
        stmt += "* FROM \"" + tname + "\"";
    }
    else
    {
        for (int ii = 0; ii < search.size(); ii++)
        {
            stmt += "\"" + search[ii] + "\", ";
        }
        stmt.erase(stmt.size() - 2, 2);
        stmt += " FROM \"" + tname + "\"";
    }
    stmt += " ORDER BY " + orderby;
    executor(stmt, results);
    return results.size();
}
int SQLFUNC::selectOrderBy(vector<string> search, string tname, vector<vector<string>>& results, string orderby)
{
    string stmt = "SELECT ";
    if (search[0] == "*" && search.size() == 1)
    {
        stmt += "* FROM \"" + tname + "\"";
    }
    else
    {
        for (int ii = 0; ii < search.size(); ii++)
        {
            stmt += "\"" + search[ii] + "\", ";
        }
        stmt.erase(stmt.size() - 2, 2);
        stmt += " FROM \"" + tname + "\"";
    }
    stmt += " ORDER BY " + orderby;
    executor(stmt, results);
    return results.size();
}
void SQLFUNC::select_tree(string tname, vector<vector<int>>& tree_st, vector<string>& tree_pl)
{
    // Produce a tree structure and tree payload for the given table name as root. 
    // Certain table name parameters have special forks within the function.

    vector<vector<string>> results;
    unordered_map<string, int> registry;
    vector<vector<int>> kids;
    vector<int> ivtemp;
    vector<string> vtemp;
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
            if (results[ii][jj] == "")
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
                jf.err("iparent registry-sf.select_tree");
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
vector<string> SQLFUNC::selectYears()
{
    // Returns the list of (ascending, unique) years represented in the database.
    vector<string> results, sYears;
    string stmt = "SELECT Year FROM TCatalogueIndex";
    executor(stmt, results);
    unordered_map<string, int> mapYear;
    int index = 0, inum;
    for (int ii = 0; ii < results.size(); ii++)
    {
        try { inum = mapYear.at(results[ii]); }
        catch (out_of_range)
        {
            sYears.push_back(results[ii]);
            mapYear.emplace(results[ii], index);
            index++;
        }
    }
    jf.isort_ilist(sYears, JFUNC::Increasing);
    return sYears;
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
    string message = jf.timestamper() + " SQL ERROR #" + to_string(errcode) + ", in ";
    message += func + ": " + serrmsg + "\r\n";
    ERR.open(error_path, ofstream::app);
    ERR << message << endl;
    ERR.close();
    exit(EXIT_FAILURE);
}
string SQLFUNC::sqlErrMsg()
{
    const char* errmsg = sqlite3_errmsg(db);
    string output;
    int index = 0;
    while (errmsg[index] != 0)
    {
        output.push_back(errmsg[index]);
        index++;
    }
    return output;
}
int SQLFUNC::statusCata(string sname)
{
    vector<string> search = { "Description" };
    string tname = "TCatalogueIndex";
    string result;
    vector<string> conditions = { "Name = '" + sname + "'" };
    select(search, tname, result, conditions);
    if (result.size() < 1) { return 0; }  // Catalogue absent.
    else if (result == "Incomplete") { return 1; }  // Catalogue incomplete.
    return 2;  // Catalogue ready.
}
size_t SQLFUNC::table_exist(string tname)
{
    // Returns TRUE or FALSE as to the existance of a given table within the database.
    return tableSet.count(tname);
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
        vtemp = jf.list_from_marker(tall[ii], '$');
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
void SQLFUNC::update(string tname, vector<string> revisions, vector<string> conditions)
{
    string stmt = "UPDATE \"" + tname + "\" SET ";
    for (int ii = 0; ii < revisions.size(); ii++)
    {
        if (ii > 0) { stmt += ", "; }
        stmt += revisions[ii];
    }
    stmt += " WHERE";
    for (int ii = 0; ii < conditions.size(); ii++)
    {
        stmt += " " + conditions[ii];
    }
    stmt += ";";
    executor(stmt);
}
