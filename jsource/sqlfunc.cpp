#include "sqlfunc.h"

using namespace std;

void SQLFUNC::addColumn(string tname, string colTitle, string colType)
{
    // Note that this function can only append a column on the rightmost side.
    if (!columnType.count(colType)) { err("Invalid column type-addColumn"); }
    string stmt = "ALTER TABLE \"" + tname + "\" ADD COLUMN \"";
    stmt += colTitle + "\" " + colType + ";";
    executor(stmt);
}
void SQLFUNC::allTables(vector<string>& vsTable)
{
    vsTable.clear();
    string stmt = "SELECT name FROM sqlite_master WHERE type='table';";
    executor(stmt, vsTable);
}
void SQLFUNC::allTables(set<string>& setTableList)
{
    vector<string> vsTable;
    string stmt = "SELECT name FROM sqlite_master WHERE type='table';";
    executor(stmt, vsTable);
    int numTable = (int)vsTable.size();

    setTableList.clear();
    for (int ii = 0; ii < numTable; ii++) {
        setTableList.emplace(vsTable[ii]);
    }
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
    catch (invalid_argument) { err("stoi-count"); }
    return iNum;
}
void SQLFUNC::createTable(string tname, vector<vector<string>>& vvsColTitle, vector<string> vsUnique)
{
    // vvsColTitle has form [column name, column type][column index].
    // vsUnique is a list of columns (or groups of columns) to be declared
    // unique within the table. 
    if (vvsColTitle.size() < 2) { err("Invalid vvsColTitle-createTable"); }
    int numCol = (int)vvsColTitle[0].size();
    string stmt = "CREATE TABLE IF NOT EXISTS \"" + tname + "\" (";
    for (int ii = 0; ii < numCol; ii++) {
        if (ii > 0) { stmt += ", "; }
        stmt += "'" + vvsColTitle[0][ii] + "' " + vvsColTitle[1][ii];
    }
    for (int ii = 0; ii < vsUnique.size(); ii++) {
        stmt += ", UNIQUE('" + vsUnique[ii] + "')";
    }
    stmt += ");";
    executor(stmt);
    setTable.emplace(tname);
}
void SQLFUNC::deleteRow(string tname, vector<string> conditions)
{
    string stmt = "DELETE FROM \"" + tname + "\" WHERE (";
    for (int ii = 0; ii < conditions.size(); ii++) {
        sclean(conditions[ii], 2);
        stmt += " " + conditions[ii];
    }
    stmt += ");";
    executor(stmt);
}
void SQLFUNC::clearTable(string tname)
{
    // Removes all rows from the table, but does not remove the table itself.
    string stmt = "DELETE FROM \"" + tname + "\";";
    executor(stmt);
}
void SQLFUNC::dropTable(string tname)
{
    // Completely removes the table from the database.
    string stmt = "DROP TABLE IF EXISTS \"" + tname + "\"";
    executor(stmt);
    setTable.erase(tname);
}
void SQLFUNC::err(string message)
{
    string errorMessage = "SQLFUNC error:\n" + message;
    JLOG::getInstance()->err(errorMessage);
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
    int type, size;  // Type: 1(int), 2(double), 3(string)
    sqlite3_int64 ivalue;
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
            ivalue = sqlite3_column_int64(statement, 0);
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
    int type, size;  // Type: 1(int), 2(double), 3(string)
    sqlite3_int64 ivalue;
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
            ivalue = sqlite3_column_int64(statement, 0);
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
    int type, size, inum;  // Type: 1(int), 2(double), 3(string)
    sqlite3_int64 ivalue;
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
            inum = (int)results.size();
            results.resize(inum + col_count);
            for (int ii = 0; ii < col_count; ii++)
            {
                type = sqlite3_column_type(statement, ii);
                switch (type)
                {
                case 1:
                    ivalue = sqlite3_column_int64(statement, ii);
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
            while (results.back() == "") { results.pop_back(); }
        }
        else  // Returned result will be a column.
        {
            type = sqlite3_column_type(statement, 0);
            switch (type)
            {
            case 1:
                ivalue = sqlite3_column_int64(statement, 0);
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
    int type, col_count, size;  // Type: 1(int), 2(double), 3(string)
    sqlite3_int64 ivalue;
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
                ivalue = sqlite3_column_int64(statement, ii);
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
        while (results.back().back() == "") { results.back().pop_back(); }
        error = sqlite3_step(statement);
    }
    if (error > 0 && error != 101) { sqlerr("step-executor2"); }
    error = sqlite3_finalize(statement);
    if (error > 0 && error != 100 && error != 101) { sqlerr("finalize-sf.executor2"); }
}
void SQLFUNC::executor(string stmt, vector<vector<wstring>>& results)
{
    int type, col_count, size;  // Type: 1(int), 2(double), 3(string)
    sqlite3_int64 ivalue;
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
                ivalue = sqlite3_column_int64(statement, ii);
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
        while (results.back().back() == L"") { results.back().pop_back(); }
        error = sqlite3_step(statement);
    }
    if (error > 0 && error != 101) { sqlerr("step-executor2"); }
    error = sqlite3_finalize(statement);
    if (error > 0 && error != 100 && error != 101) { sqlerr("finalize-sf.executor2"); }
}
vector<vector<string>> SQLFUNC::getColTitle(string tname)
{
    // Return form [column name, column type][column index].
    vector<vector<string>> vvsColTitle, vvsResult;
    string stmt = "PRAGMA table_info('" + tname + "');";
    executor(stmt, vvsResult);
    vvsColTitle.resize(2, vector<string>(vvsResult.size()));
    for (int ii = 0; ii < vvsResult.size(); ii++) {
        vvsColTitle[0][ii] = vvsResult[ii][1];
        vvsColTitle[1][ii] = vvsResult[ii][2];
        if (vvsResult[ii][5] == "1") {
            if (vvsColTitle[1][ii] == "INTEGER") {
                vvsColTitle[1][ii] += " PRIMARY KEY";
            }
        }
    }
    return vvsColTitle;
}
void SQLFUNC::getColTitle(map<string, string>& mapColTitle, string tname)
{
    // Note: order in returned set matches column index order.
    vector<vector<string>> vvsResult;
    string stmt = "PRAGMA table_info('" + tname + "');";
    executor(stmt, vvsResult);
    string temp;
    mapColTitle.clear();
    for (int ii = 0; ii < vvsResult.size(); ii++) {
        temp = vvsResult[ii][2];
        if (vvsResult[ii][5] == "1") {
            if (temp == "INTEGER") {
                temp += " PRIMARY KEY";
            }
        }
        mapColTitle.emplace(vvsResult[ii][1], temp);
    }
}
int SQLFUNC::getNumCol(string tname)
{
    vector<vector<string>> vvsResult;
    string stmt = "PRAGMA table_info('" + tname + "');";
    executor(stmt, vvsResult);
    return (int)vvsResult.size();
}
int SQLFUNC::getNumRow(string tname)
{
    if (!tableExist(tname)) { return -1; }
    string stmt = "SELECT COUNT(*) FROM [";
    stmt += tname + "];";
    string result;
    executor(stmt, result);
    int numRows;
    try { numRows = stoi(result); }
    catch (invalid_argument) { err("stoi-sf.getNumRows"); }
    return numRows;
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
                    if (tableExist("TG_Region$" + Tsuffix)) { sList.push_back("TG_Region$" + Tsuffix); }
                    if (tableExist("TG_Row$" + Tsuffix)) { sList.push_back("TG_Row$" + Tsuffix); }
                    if (tableExist("TMap$" + Tsuffix)) { sList.push_back("TMap$" + Tsuffix); }
                    
                    try { gid = stoi(Tsuffix); }
                    catch (invalid_argument) {}
                    if (gid >= 0)
                    {
                        for (int ii = 0; ii < cataList.size(); ii++)
                        {
                            if (tableExist(cataList[ii] + "$" + Tsuffix)) { sList.push_back(cataList[ii] + "$" + Tsuffix); }
                        }
                    }

                    if (Tsuffix == "Geo" || Tsuffix == "Geo_Layers")
                    {
                        for (int ii = 0; ii < cataList.size(); ii++)
                        {
                            if (tableExist(cataList[ii] + "$" + Tsuffix)) { sList.push_back(cataList[ii] + "$" + Tsuffix); }
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
                            if (tableExist(Tprefix + "$" + cataList[ii])) { sList.push_back(Tprefix + "$" + cataList[ii]); }
                        }
                    }
                    else if (tableExist(Tprefix))  // Catalogue name was given.
                    {
                        sList.push_back(Tprefix);
                        if (tableExist(Tprefix + "$Geo")) { sList.push_back(Tprefix + "$Geo"); }
                        if (tableExist(Tprefix + "$Geo_Layers")) { sList.push_back(Tprefix + "$Geo_Layers"); }
                        temp = "TG_Region$" + Tprefix;
                        if (tableExist(temp))
                        {
                            vsTemp = { "GID" };
                            select(vsTemp, temp, gidList);
                            for (int ii = 0; ii < gidList.size(); ii++)
                            {
                                if (tableExist(Tprefix + "$" + gidList[ii])) 
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
                if (tableExist("TG_Region$" + Tsuffix)) { sList.push_back("TG_Region$" + Tsuffix); }
                if (tableExist("TG_Row$" + Tsuffix)) { sList.push_back("TG_Row$" + Tsuffix); }
                if (tableExist("TMap$" + Tsuffix)) { sList.push_back("TMap$" + Tsuffix); }

                try { gid = stoi(Tsuffix); }
                catch (invalid_argument) {}
                if (gid >= 0)
                {
                    for (int ii = 0; ii < cataList.size(); ii++)
                    {
                        if (tableExist(cataList[ii] + "$" + Tsuffix)) { sList.push_back(cataList[ii] + "$" + Tsuffix); }
                    }
                }

                if (Tsuffix == "Geo" || Tsuffix == "Geo_Layers")
                {
                    for (int ii = 0; ii < cataList.size(); ii++)
                    {
                        if (tableExist(cataList[ii] + "$" + Tsuffix)) { sList.push_back(cataList[ii] + "$" + Tsuffix); }
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
                        if (tableExist(Tprefix + "$" + cataList[ii])) { sList.push_back(Tprefix + "$" + cataList[ii]); }
                    }
                }
                else if (tableExist(Tprefix))  // Catalogue name was given.
                {
                    sList.push_back(Tprefix);
                    if (tableExist(Tprefix + "$Geo")) { sList.push_back(Tprefix + "$Geo"); }
                    if (tableExist(Tprefix + "$Geo_Layers")) { sList.push_back(Tprefix + "$Geo_Layers"); }
                    temp = "TG_Region$" + Tprefix;
                    if (tableExist(temp))
                    {
                        vsTemp = { "GID" };
                        select(vsTemp, temp, gidList);
                        for (int ii = 0; ii < gidList.size(); ii++)
                        {
                            if (tableExist(Tprefix + "$" + gidList[ii]))
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
            if (tableExist(search)) { sList.push_back(search); }
            else { sList.push_back("No results found."); }
        }
    }
    return sList;
}
vector<string> SQLFUNC::getTableListFromRoot(string root)
{
    // Return a list of table names which begin with "root".
    vector<string> vsTable;
    string tname;
    size_t pos1;
    for (auto it = setTable.begin(); it != setTable.end(); ++it) {
        tname = *it;
        pos1 = tname.find(root);
        if (pos1 == 0) {
            vsTable.push_back(tname);
        }
    }
    return vsTable;
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
    allTables(setTable);

    columnType.emplace("TEXT");
    columnType.emplace("NUM");
    columnType.emplace("NUMERIC");
    columnType.emplace("INT");
    columnType.emplace("INTEGER");
    columnType.emplace("REAL");
    columnType.emplace("BLOB");

    mapColType.emplace("TEXT", 0);
    mapColType.emplace("NUM", 1);
    mapColType.emplace("NUMERIC", 1);
    mapColType.emplace("INT", 2);
    mapColType.emplace("INTEGER", 2);
    mapColType.emplace("INTEGER PRIMARY KEY", 2);
    mapColType.emplace("REAL", 3);
    mapColType.emplace("BLOB", 4);
}
void SQLFUNC::insert(string tname, vector<string>& rowData)
{
    int numCol = getNumCol(tname);
    if (numCol < rowData.size()) {
        err("row_data has more columns than the table-sqlfunc.insert");
    }
    while (numCol > rowData.size()) {
        rowData.push_back("");
    }

    string stmt = "INSERT INTO \"" + tname + "\" VALUES (";
    for (int ii = 0; ii < numCol; ii++) {
        if (ii > 0) { stmt += ", "; }
        stmt += rowData[ii];
    }

    executor(stmt);
}
void SQLFUNC::insert(string tname, vector<vector<string>>& vvsColTitle, vector<string>& rowData)
{
    int numCol = (int)vvsColTitle.size();
    vector<int> viColType(numCol);
    for (int ii = 0; ii < numCol; ii++) {
        if (mapColType.count(vvsColTitle[ii][1])) {
            viColType[ii] = mapColType.at(vvsColTitle[ii][1]);
        }
        else { err("Unknown column type-sf.insert"); }
    }

    string stmt0 = "INSERT INTO \"" + tname + "\" (";
    for (int ii = 0; ii < numCol; ii++) {
        if (ii > 0) { stmt0 += ", "; }
        stmt0 += "\"" + vvsColTitle[ii][0] + "\"";
    }
    stmt0 += ") VALUES (";

    while (rowData.size() < numCol) { rowData.push_back(""); }
    string stmt = stmt0;
    for (int jj = 0; jj < numCol; jj++) {
        if (jj > 0) { stmt += ", "; }
        if (rowData[jj] == "") { stmt += "NULL"; }
        else {
            if (viColType[jj] == 0) {
                sclean(rowData[jj], 1);  // Double the apostraphes.
                stmt += "'" + rowData[jj] + "'";  // TEXT
            }
            else { stmt += rowData[jj]; }
        }
    }
    stmt += ");";
    executor(stmt);
}
void SQLFUNC::insert(string tname, vector<vector<string>>& vvsColTitle, vector<vector<string>>& rowData)
{
    int numCol = (int)vvsColTitle.size();
    vector<int> viColType(numCol);
    for (int ii = 0; ii < numCol; ii++) {
        if (mapColType.count(vvsColTitle[ii][1])) {
            viColType[ii] = mapColType.at(vvsColTitle[ii][1]);
        }
        else { err("Unknown column type-sf.insert"); }
    }

    string stmt0 = "INSERT INTO \"" + tname + "\" (";
    for (int ii = 0; ii < numCol; ii++) {
        if (ii > 0) { stmt0 += ", "; }
        stmt0 += "\"" + vvsColTitle[ii][0] + "\"";
    }
    stmt0 += ") VALUES (";

    string stmt;
    int error = sqlite3_exec(db, "BEGIN EXCLUSIVE TRANSACTION", NULL, NULL, NULL);
    if (error) { sqlerr("begin transaction-insert_rows"); }
    for (int ii = 0; ii < rowData.size(); ii++)
    {
        while (rowData[ii].size() < numCol) { rowData[ii].push_back(""); }
        stmt = stmt0;
        for (int jj = 0; jj < numCol; jj++) {
            if (jj > 0) { stmt += ", "; }
            if (rowData[ii][jj] == "") { stmt += "NULL"; }
            else {
                if (viColType[jj] == 0) {
                    sclean(rowData[ii][jj], 1);  // Double the apostraphes.
                    stmt += "'" + rowData[ii][jj] + "'";  // TEXT
                }
                else { stmt += rowData[ii][jj]; }
            }
        }
        stmt += ");";
        executor(stmt);
    }
    error = sqlite3_exec(db, "COMMIT TRANSACTION", NULL, NULL, NULL);
    if (error) { sqlerr("commit transaction-insert_rows"); }
}
void SQLFUNC::insertPrepared(vector<string>& stmts)
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
    if (stmtAndParams.size() % numParam != 1) { err("Size mismatch-sf.insertPreparedBind"); }

    sqlite3_stmt* statement;
    int bufSize = (int)stmtAndParams[0].size();
    int error = sqlite3_prepare_v2(db, stmtAndParams[0].c_str(), bufSize, &statement, NULL);
    if (error) { sqlerr("prepare-sf.insertPreparedBind"); }

    error = sqlite3_exec(db, "BEGIN EXCLUSIVE TRANSACTION", NULL, NULL, NULL);
    if (error) { sqlerr("begin transaction-sf.insertPreparedBind"); }
    while (index < stmtAndParams.size() - 1)
    {
        for (int ii = 1; ii <= numParam; ii++)
        {
            error = sqlite3_bind_text(statement, ii, stmtAndParams[index + ii].c_str(), (int)stmtAndParams[index + ii].size(), SQLITE_TRANSIENT);
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
void SQLFUNC::insertRow(string tname, vector<vector<string>>& vvsRow)
{
    // vvsRow has form [column titles, row0, row1, ...][values].
    // Note: if vvsRow contains more than one row of data to insert, 
    // then the insertion will be done as a transaction.
    if (vvsRow.size() < 2 || vvsRow[0].size() < 1) { err("Missing vvsRow-insertRow"); }
    map<string, string> mapColTitle;
    getColTitle(mapColTitle, tname);
    int numCol = (int)vvsRow[0].size();
    for (int ii = 0; ii < numCol; ii++) {
        if (!mapColTitle.count(vvsRow[0][ii])) { err("vvsRow has column titles absent from table-insertRow"); }
    }

    string stmt0 = "INSERT OR IGNORE INTO \"" + tname + "\" (";
    string temp;
    if (vvsRow.size() == 2) {  // Single row insertion.
        string stmt = stmt0;
        for (int ii = 0; ii < numCol; ii++) {
            if (ii > 0) { stmt += ", "; }
            stmt += "\"" + vvsRow[0][ii] + "\"";
        }
        stmt += ") VALUES (";
        for (int ii = 0; ii < numCol; ii++) {
            if (ii > 0) { stmt += ", "; }
            if (mapColTitle.at(vvsRow[0][ii]) == "TEXT") {
                temp = vvsRow[1][ii];
                sclean(temp, 1);
                stmt += "'" + temp + "'";
            }
            else { stmt += vvsRow[1][ii]; }           
        }
        stmt += ");";
        executor(stmt);
        return;
    }
    
    // Multiple-row transaction insertion.
    vector<string> vStmt(vvsRow.size() - 1);
    for (int ii = 0; ii < vStmt.size(); ii++) {
        vStmt[ii] = stmt0;
        for (int jj = 0; jj < vvsRow[1 + ii].size(); jj++) {
            if (jj > 0) { vStmt[ii] += ", "; }
            vStmt[ii] += "\"" + vvsRow[0][jj] + "\"";
        }
        vStmt[ii] += ") VALUES (";
        for (int jj = 0; jj < vvsRow[1 + ii].size(); jj++) {
            if (jj > 0) { vStmt[ii] += ", "; }
            if (mapColTitle.at(vvsRow[0][jj]) == "TEXT") {
                temp = vvsRow[1 + ii][jj];
                sclean(temp, 1);
                vStmt[ii] += "'" + temp + "'";
            }
            else { vStmt[ii] += vvsRow[1 + ii][jj]; }
        }
        vStmt[ii] += ");";
    }
    insertPrepared(vStmt);
}
void SQLFUNC::log(string message)
{
    string logMessage = "SQLFUNC log entry:\n" + message;
    JLOG::getInstance()->log(logMessage);
}
void SQLFUNC::refreshTableList()
{
    allTables(setTable);
}
void SQLFUNC::removeCol(string tname, string colTitle)
{
    string stmt = "ALTER TABLE \"" + tname + "\" DROP COLUMN \"" + colTitle + "\";";
    if (tableExist(tname)) { executor(stmt); }
    else {
        log("Could not delete column from " + tname + " : could not find table.");
    }
}
void SQLFUNC::removeRow(string tname, vector<string> conditions)
{
    string stmt = "DELETE FROM \"" + tname + "\" WHERE (";
    for (int ii = 0; ii < conditions.size(); ii++)
    {
        stmt += conditions[ii] + " ";
    }
    stmt += ");";
    if (tableExist(tname))
    {
        executor(stmt);
    }
    else {
        log("Could not delete " + tname + " : could not find.");
    }
}
void SQLFUNC::safeCol(string tname, vector<vector<string>>& vvsColTitle)
{
    // For a given table name, if it has fewer columns than "vvsColTitle", 
    // then append a sufficient number of new columns to it. The new columns 
    // will match the appropriate index from "vvsColTitle".

    string stmt, temp;
    int numCol = (int)vvsColTitle.size();
    vector<vector<string>> vvsColTitleOld = getColTitle(tname);
    int numColOld = (int)vvsColTitleOld.size();
    if (numColOld < numCol) {
        for (int ii = numColOld; ii < numCol; ii++) {
            temp = vvsColTitle[ii][0];
            sclean(temp, 1);
            stmt = "ALTER TABLE \"" + tname + "\" ADD COLUMN \'";
            stmt += temp + "\' " + vvsColTitle[ii][1];
            executor(stmt);
        }
    }
}
int SQLFUNC::sclean(string& bbq, int mode)
{
    // mode 1 is to double all single quotation marks (general purpose SQL).
    // mode 2 is for conditional 'select' statements.
    // mode 3 is for 'update' revisions. 
    int count = 0;
    double dnum;
    size_t pos1, pos2, posLIKE, posEqual, len;
    string temp;

    pos1 = bbq.find('[');
    if (pos1 < bbq.size())
    {
        pos2 = bbq.find(']', pos1);
        if (pos2 > bbq.size()) { err("Square bracket asymmetry-sf.sclean"); }
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
    case 3:   // This variant is used for revision statements.
    {
        posEqual = bbq.find('=');
        if (posEqual > bbq.size()) { break; }

        temp = "\"";
        pos2 = bbq.find_last_not_of(' ', posEqual - 1);
        if (bbq[pos2] != '"') { bbq.insert(pos2 + 1, temp); }
        pos1 = bbq.find_first_not_of(' ');
        if (bbq[pos1] != '"') { bbq.insert(pos1, temp); }

        bool numeric = 1;
        posEqual = bbq.find('=');
        pos2 = bbq.find_first_not_of(' ', posEqual + 1);
        temp = bbq.substr(pos2);
        try { dnum = stod(temp); }
        catch (invalid_argument) { numeric = 0; }
        if (!numeric)
        {
            len = temp.size();
            pos1 = temp.find('\'');
            while (pos1 < temp.size())
            {
                if (pos1 == 0) 
                { 
                    pos1 = temp.find('\'', pos1 + 1);
                    continue;
                }
                else if (pos1 == temp.size() - 1) { break; }
                else if (temp[pos1 + 1] == '\'')
                {
                    pos1 = temp.find('\'', pos1 + 2);
                    continue;
                }
                temp.insert(temp.begin() + pos1, '\'');
                pos1 = temp.find('\'', pos1 + 2);
            }
            if (temp[0] != '\'') { temp.insert(temp.begin(), '\''); }
            if (temp[temp.size() - 1] != '\'') { temp.push_back('\''); }
            bbq.replace(pos2, len, temp);
        }
        break;
    }
    }

    return count;
}
int SQLFUNC::searchTableName(vector<string>& vsTable, string sQuery)
{
    // Any '*' wildcards are changed into SQL '%' wildcards, which can 
    // have any string length.
    vector<string> dirt = { "*" }, soap = { "%" };
    jstr.clean(sQuery, dirt, soap);
    sclean(sQuery, 1);

    vsTable.clear();
    string stmt = "SELECT name FROM sqlite_master WHERE (type='table' ";
    stmt += "AND name LIKE '" + sQuery + "');";
    executor(stmt, vsTable);
    return (int)vsTable.size();
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
    return (int)results.size();
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
    return (int)results.size();
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
    return (int)results.size();
}
int SQLFUNC::select(vector<string> search, string tname, string& result, vector<string> conditions)
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
int SQLFUNC::select(vector<string> search, string tname, wstring& result, vector<string> conditions)
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
int SQLFUNC::select(vector<string> search, string tname, vector<string>& results, vector<string> conditions)
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
    return (int)results.size();
}
int SQLFUNC::select(vector<string> search, string tname, vector<vector<string>>& results, vector<string> conditions)
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
    return (int)results.size();
}
int SQLFUNC::select(vector<string> search, string tname, vector<vector<wstring>>& results, vector<string> conditions)
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
    return (int)results.size();
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
    return (int)results.size();
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
    return (int)results.size();
}
void SQLFUNC::selectTree(string tname, vector<vector<int>>& tree_st, vector<string>& tree_pl)
{
    // Produce a tree structure and tree payload for the given table name as root. 
    // Certain table name parameters have special forks within the function.

    vector<vector<string>> results;
    unordered_map<string, int> registry;
    vector<vector<int>> kids;
    vector<int> ivtemp;
    vector<string> vtemp;
    string temp, sparent, stmt;
    int iparent;
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
            catch (out_of_range)
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
}
void SQLFUNC::sqlerr(string func)
{
    // Threadsafe error log function specific to SQL errors.
    lock_guard<mutex> lock(m_err);
    int errcode = sqlite3_errcode(db);
    const char* errmsg = sqlite3_errmsg(db);
    string serrmsg(errmsg);
    string message = " SQL ERROR #" + to_string(errcode) + ", in ";
    message += func + ": " + serrmsg + "\r\n";
    err(message);
}
string SQLFUNC::sqlErrMsg()
{
    // Return the most recent SQLITE error message.
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
void SQLFUNC::stmtInsertRow(vector<string>& vsStmt, string tname, vector<vector<string>>& vvsRow)
{
    // This function performs as "insertRow" does, except that it returns the SQL statements 
    // rather than inserting them immediately.
    // vvsRow has form [column titles, row0, row1, ...][values].
    if (vvsRow.size() < 2 || vvsRow[0].size() < 1) { err("Missing vvsRow-stmtInsertRow"); }
    if (tname.size() < 1) { err("Missing tname-stmtInsertRow"); }
    double test;
    int numCol = (int)vvsRow[0].size();
    int numRow = (int)vvsRow.size() - 1;
    int index = (int)vsStmt.size();
    vector<size_t> vSize(numCol);
    size_t rowSize, totalSize;
    string stmt0 = "INSERT OR IGNORE INTO \"" + tname + "\" (";
    string temp;
    vsStmt.resize(index + numRow);
    for (int ii = 0; ii < numRow; ii++) {
        rowSize = vvsRow[1 + ii].size();  // Number of values contained in this row.
        
        // Determine which columns to skip over (if any), or if the entire row should be skipped.
        totalSize = 0;
        for (int jj = 0; jj < rowSize; jj++) {
            vSize[jj] = vvsRow[1 + ii][jj].size();
            totalSize += vSize[jj];
        }
        if (totalSize == 0) { continue; }

        vsStmt[index + ii] = stmt0;
        for (int jj = 0; jj < rowSize; jj++) {
            if (vSize[jj] == 0) { continue; }
            if (jj > 0) { vsStmt[index + ii] += ", "; }
            vsStmt[index + ii] += "\"" + vvsRow[0][jj] + "\"";
        }
        vsStmt[index + ii] += ") VALUES (";
        for (int jj = 0; jj < rowSize; jj++) {
            if (vSize[jj] == 0) { continue; }
            if (jj > 0) { vsStmt[index + ii] += ", "; }
            try { 
                test = stod(vvsRow[1 + ii][jj]); 
                vsStmt[index + ii] += vvsRow[1 + ii][jj];
            }
            catch (invalid_argument) {
                temp = vvsRow[1 + ii][jj];
                sclean(temp, 1);
                vsStmt[index + ii] += "'" + temp + "'";
            }
        }
        vsStmt[index + ii] += ");";
    }
}
bool SQLFUNC::tableExist(string tname)
{
    // Returns TRUE or FALSE as to the existance of a given table within the database.
    return (bool)setTable.count(tname);
}
void SQLFUNC::tableExistUpdate()
{
    // Trigger an update to setTable (useful after tables were created within a transaction).
    allTables(setTable);
}
void SQLFUNC::update(string tname, vector<string> revisions, vector<string> conditions)
{
    // Change the table values for certain columns, for all rows
    // satisfying the given conditions. 
    string stmt = "UPDATE \"" + tname + "\" SET ";
    for (int ii = 0; ii < revisions.size(); ii++)
    {
        if (ii > 0) { stmt += ", "; }
        sclean(revisions[ii], 3);
        stmt += revisions[ii];
    }
    if (conditions.size() > 0) {
        stmt += " WHERE";
        for (int ii = 0; ii < conditions.size(); ii++)
        {
            sclean(conditions[ii], 2);
            stmt += " " + conditions[ii];
        }
    }
    stmt += ";";
    executor(stmt);
}
