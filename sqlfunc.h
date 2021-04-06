#pragma once

#include <string>
#include <vector>
#include <chrono>
#include <fstream>
#include <sqlite3.h>
#include "jfunc.h"

using namespace std;

class SQLFUNC 
{
    JFUNC jfsf;
	sqlite3* db;
    ofstream ERR;
    string error_path = sroot + "\\SCDA Error Log.txt";
    
    void bind(string&, vector<string>&);
    void err(string);
    int sclean(string&, int);
	void sqlerr(string);
	string timestamper();

public:
    explicit SQLFUNC() {}
	~SQLFUNC() {}
    void create_table(string, vector<string>&, vector<int>&);
    void init(string);
    void insert_tg_existing(string);
    void insert_prepared(vector<string>&);
    string insert_stmt(string, vector<string>&, vector<string>&);
    int get_num_col(string);
    void safe_col(string, int);
    void select_tree2(string tname, vector<vector<int>>& tree_st, vector<wstring>& tree_pl);
    vector<string> select_years();
    void set_error_path(string);
    bool table_exist(string);
    vector<string> test_cata(string);

	// TEMPLATES
    template<typename ... Args> void all_tables(Args& ... args)
    {
        // Returns data obtained from the table master list, such as the number 
        // of tables or a list of their names.
    }
    template<> void all_tables<int>(int& num_tables)
    {
        string stmt = "SELECT name FROM sqlite_master WHERE type='table';";
        vector<string> results;
        executor(stmt, results);
        num_tables = results.size();
    }
    template<> void all_tables<vector<string>>(vector<string>& table_list)
    {
        table_list.clear();
        string stmt = "SELECT name FROM sqlite_master WHERE type='table';";
        executor(stmt, table_list);
    }

	template<typename ... Args> void executor(string, Args& ... args) {}
	template<> void executor(string stmt)
	{
        sqlite3_stmt* statement;
		int error = sqlite3_prepare_v2(db, stmt.c_str(), -1, &statement, NULL);
		if (error) { sqlerr("prepare-executor0"); }
		error = sqlite3_step(statement);
		if (error > 0 && error != 100 && error != 101) 
		{
			sqlerr("step-executor0"); 
		}
	}
    template<> void executor<string>(string stmt, string& result)
    {
        // Note that this variant of the executor function will only return the first result.
        int type, size;  // Type: 1(int), 2(double), 3(string)
        sqlite3_stmt* statement;
        int error = sqlite3_prepare_v2(db, stmt.c_str(), -1, &statement, NULL);
        if (error) { sqlerr("prepare-executor0.5"); }
        error = sqlite3_step(statement);
        int ivalue;
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
            return;
        }
        else if (error > 0 && error != 101)
        {
            sqlerr("step-executor0.5");
        }
    }
    template<> void executor<wstring>(string stmt, wstring& result)
    {
        // Note that this variant of the executor function can accomodate either a column or a row as the result.
        int type, size;  // Type: 1(int), 2(double), 3(string)
        sqlite3_stmt* statement;
        int error = sqlite3_prepare_v2(db, stmt.c_str(), -1, &statement, NULL);
        if (error) { sqlerr("prepare-executor1"); }
        error = sqlite3_step(statement);
        int ivalue, inum;
        double dvalue;
        wstring wvalue;
        int col_count = -1;
        char* buffer;

        if (error == 100)
        {
            type = sqlite3_column_type(statement, 0);
            switch (type)
            {
            case 1:
                ivalue = sqlite3_column_int(statement, 0);
                result = to_wstring(ivalue);
                break;
            case 2:
                dvalue = sqlite3_column_double(statement, 0);
                result = to_wstring(dvalue);
                break;
            case 3:
            {
                size = sqlite3_column_bytes(statement, 0);
                const unsigned char* buffer = sqlite3_column_text(statement, 0);
                wvalue.resize(size);
                for (int jj = 0; jj < size; jj++)
                {
                    wvalue[jj] = (wchar_t)buffer[jj];
                }
                result = wvalue;
                break;
            }
            case 5:
                result = L"";
                break;
            }
        }
        else if (error > 0 && error != 101)
        {
            sqlerr("step-executor1");
        }
    }
    template<> void executor<vector<string>>(string stmt, vector<string>& results)
	{
        // Note that this variant of the executor function can accomodate either a column or a row as the result.
        int type, size;  // Type: 1(int), 2(double), 3(string)
        sqlite3_stmt* statement;
        int error = sqlite3_prepare_v2(db, stmt.c_str(), -1, &statement, NULL);
        if (error) { sqlerr("prepare-executor1"); }
        error = sqlite3_step(statement);
        int ivalue, inum;
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
                        results[inum + ii] = svalue;
                        iextra = 0;
                        break;
                    }
                    case 5:
                        results[inum + ii] = "";
                        break;
                    }
                }
                return;
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
        if (error > 0 && error != 101)
        {
            sqlerr("step-executor1");
        }
	}
    template<> void executor<vector<wstring>>(string stmt, vector<wstring>& results)
    {
        // Note that this variant of the executor function can accomodate either a column or a row as the result.
        int type, size;  // Type: 1(int), 2(double), 3(string)
        sqlite3_stmt* statement;
        int error = sqlite3_prepare_v2(db, stmt.c_str(), -1, &statement, NULL);
        if (error) { sqlerr("prepare-executor1"); }
        error = sqlite3_step(statement);
        int ivalue, inum;
        double dvalue;
        wstring wvalue;
        int col_count = -1;
        char* buffer;

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
                        results[inum + ii] = to_wstring(ivalue);
                        break;
                    case 2:
                        dvalue = sqlite3_column_double(statement, ii);
                        results[inum + ii] = to_wstring(dvalue);
                        break;
                    case 3:
                    {
                        size = sqlite3_column_bytes(statement, ii);
                        const unsigned char* buffer = sqlite3_column_text(statement, ii);
                        wvalue.resize(size);
                        for (int jj = 0; jj < size; jj++)
                        {
                            wvalue[jj] = (wchar_t)buffer[jj];
                        }
                        results[inum + ii] = wvalue;
                        break;
                    }
                    case 5:
                        results[inum + ii] = L"";
                        break;
                    }
                }
                return;
            }
            else  // Returned result will be a column.
            {
                type = sqlite3_column_type(statement, 0);
                switch (type)
                {
                case 1:
                    ivalue = sqlite3_column_int(statement, 0);
                    results.push_back(to_wstring(ivalue));
                    break;
                case 2:
                    dvalue = sqlite3_column_double(statement, 0);
                    results.push_back(to_wstring(dvalue));
                    break;
                case 3:
                {
                    size = sqlite3_column_bytes(statement, 0);
                    const unsigned char* buffer = sqlite3_column_text(statement, 0);
                    wvalue.resize(size);
                    for (int jj = 0; jj < size; jj++)
                    {
                        wvalue[jj] = (wchar_t)buffer[jj];
                    }
                    results.push_back(wvalue);
                    break;
                }
                case 5:
                    results.push_back(L"");
                    break;
                }
            }
            error = sqlite3_step(statement);
        }
        if (error > 0 && error != 101)
        {
            sqlerr("step-executor1");
        }
    }
    template<> void executor<vector<vector<string>>>(string stmt, vector<vector<string>>& results)
    {
        int type, col_count, size;  // Type: 1(int), 2(double), 3(string)
        sqlite3_stmt* statement;
        int error = sqlite3_prepare_v2(db, stmt.c_str(), -1, &statement, NULL);
        if (error) { sqlerr("prepare-executor2"); }
        error = sqlite3_step(statement);
        int ivalue;
        double dvalue;
        string svalue;
        int iextra = 0;

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
                    results[results.size() - 1][ii] = svalue;
                    iextra = 0;
                    break;
                }
                case 5:
                    results[results.size() - 1].push_back("");
                    break;
                }
            }
            error = sqlite3_step(statement);
        }
        if (error > 0 && error != 101)
        {
            sqlerr("step-executor2");
        }
    }
    template<> void executor<vector<vector<wstring>>>(string stmt, vector<vector<wstring>>& results)
    {
        int type, col_count, size;  // Type: 1(int), 2(double), 3(string)
        sqlite3_stmt* statement;
        int error = sqlite3_prepare_v2(db, stmt.c_str(), -1, &statement, NULL);
        if (error) { sqlerr("prepare-executor2"); }
        error = sqlite3_step(statement);
        int ivalue;
        double dvalue;
        wstring wvalue;

        while (error == 100)
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
                        wvalue[ii] = (wchar_t)buffer[ii];
                    }
                    results[results.size() - 1][ii] = wvalue;
                    break;
                }
                case 5:
                    results[results.size() - 1].push_back(L"");
                    break;
                }
            }
            error = sqlite3_step(statement);
        }
        if (error > 0 && error != 101)
        {
            sqlerr("step-executor2");
        }
    }
   
    template<typename ... Args> void get_col_titles(string, Args& ... args) 
    {
        // Referencing a 1D vector will output [name 1, name 2, ...].
        // Referencing a 2D vector will output [column index][name, type].
    }
    template<> void get_col_titles<vector<string>>(string tname, vector<string>& titles)
    {
        string temp1;
        vector<vector<string>> results;
        string stmt = "pragma table_info ('" + tname + "')";
        executor(stmt, results);
        for (int ii = 0; ii < results.size(); ii++)
        {
            temp1 = results[ii][1];
            titles.push_back(temp1);
        }
    }
    template<> void get_col_titles<vector<wstring>>(string tname, vector<wstring>& titles)
    {
        wstring temp1;
        vector<vector<wstring>> results;
        string stmt = "pragma table_info ('" + tname + "')";
        executor(stmt, results);
        for (int ii = 0; ii < results.size(); ii++)
        {
            temp1 = results[ii][1];
            titles.push_back(temp1);
        }
    }
    template<> void get_col_titles<vector<vector<string>>>(string tname, vector<vector<string>>& titles)
    {
        vector<string> vtemp(2);
        vector<vector<string>> results;
        string stmt = "pragma table_info ('" + tname + "')";
        executor(stmt, results);
        for (int ii = 0; ii < results.size(); ii++)
        {
            vtemp[0] = results[ii][1];
            vtemp[1] = results[ii][2];
            titles.push_back(vtemp);
        }
    }

    template<typename ... Args> void get_table_list(vector<string>&, Args& ... args)
    {
        // Return a list of all table names present within a limiting division.
        // If only the result vector is provided, the function will return a list of all tables in the database.
        // If a second string vector is specified, then each element in that vector will be taken as a limiting
        // search parameter, in the same order as given. 
        // Example: [cata_name, GID] will return a list of all subtables for that GID.
    }
    template<> void get_table_list(vector<string>& results)
    {
        string stmt = "SELECT name FROM sqlite_master WHERE type='table';";
        executor(stmt, results);
    }
    template<> void get_table_list<string>(vector<string>& results, string& search)
    {
        results.clear();
        vector<string> search_split = jfsf.list_from_marker(search, '$');
        vector<string> chaff_split;
        vector<string> chaff;
        string stmt = "SELECT name FROM sqlite_master WHERE type='table';";
        executor(stmt, chaff);
        string cheddar;
        size_t pos1, pos2;
        for (int ii = 0; ii < chaff.size(); ii++)
        {
            chaff_split = jfsf.list_from_marker(chaff[ii], '$');
            if (chaff_split.size() < search_split.size()) { continue; }
            for (int jj = 1; jj < search_split.size(); jj++)
            {
                if (search_split[jj] != chaff_split[jj]) { break; }
                else if (jj == search_split.size() - 1)
                {
                    results.push_back(chaff[ii]);
                }
            }
        }
    }
    
    template<typename ... Args> void insert(string, Args& ... args)
    {
        // Insert one or more rows into the given table.
    }
    template<> void insert<vector<string>>(string tname, vector<string>& row_data)
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
    template<> void insert<vector<vector<string>>>(string tname, vector<vector<string>>& row_data)
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

    template<typename ... Args> void remove(Args& ... args)
    {
        // If only a table name is given, then delete that entire table name from the database.
        // If an integer reference is given, then (mode = 1) will bypass the existance check.
        // If a list is also given, then that list serves as boolean conditions from which 
        // TRUE rows within the table will be deleted.
    }
    template<> void remove<int, vector<string>>(int& mode, vector<string>& death_row)
    {
        string stmt;
        int error;
        if (mode == 1)
        {
            error = sqlite3_exec(db, "BEGIN EXCLUSIVE TRANSACTION", NULL, NULL, NULL);
            if (error) { sqlerr("begin transaction-remove(list)"); }
            for (int ii = 0; ii < death_row.size(); ii++)
            {
                stmt = "DELETE FROM [" + death_row[ii] + "];";
                executor(stmt);
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
    }
    template<> void remove<string>(string& tname)
    {
        string stmt = "DELETE FROM [" + tname + "];";
        if (table_exist(tname))
        {
            executor(stmt);
        }
        else
        {
            jfsf.log("Could not delete " + tname + " : could not find.");
        }
        
    }
    template<> void remove<string, vector<string>>(string& tname, vector<string>& conditions)
    {
        string stmt = "DELETE FROM [" + tname + "] WHERE (";
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
            jfsf.log("Could not delete " + tname + " : could not find.");
        }
    }

    template<typename ... Args> int select(vector<string> search, string tname, Args& ... args)
    {
        // Return (by reference) the database's values for a given the search query. 
        // 
        // Form (search queries, table name, results vector, conditions).
        // 
        // First optional parameter (results size):
        // If a 2D string vector is referenced, the full SQL results table is returned.
        // If a 1D string vector is referenced, and the SQL results contain only one 
        // column, then that column is returned. However, if the SQL results contain more
        // than one column, then it is ONLY the first row of the SQL results which are returned.
        // 
        // Second optional parameter (search conditions):
        // If the vector of conditions is given, then each string's boolean expression (after the first 
        // one) must include a logical operator "AND", "OR", "NOT". Complex logical expressions 
        // can be made, but they must include parentheses if the logical operators are not uniform.
        // 
        // The formal return integer is the maximum number of columns present in the results.

        return 0;
    }
    template<> int select<string>(vector<string> search, string tname, string& result)
    {
        string stmt = "SELECT " + search[0] + " FROM [" + tname + "];";
        executor(stmt, result);
        return 1;
    }
    template<> int select<vector<string>>(vector<string> search, string tname, vector<string>& results)
    {
        string stmt = "SELECT ";
        for (int ii = 0; ii < search.size(); ii++)
        {
            stmt += "" + search[ii] + ", ";
        }
        stmt.erase(stmt.size() - 2, 2);
        stmt += " FROM [" + tname + "];";
        executor(stmt, results);
        return results.size();
    }
    template<> int select<vector<wstring>>(vector<string> search, string tname, vector<wstring>& results)
    {
        string stmt = "SELECT ";
        for (int ii = 0; ii < search.size(); ii++)
        {
            stmt += "" + search[ii] + ", ";
        }
        stmt.erase(stmt.size() - 2, 2);
        stmt += " FROM [" + tname + "];";
        executor(stmt, results);
        return results.size();
    }
    template<> int select<vector<vector<string>>>(vector<string> search, string tname, vector<vector<string>>& results)
    {
        string stmt = "SELECT ";
        for (int ii = 0; ii < search.size(); ii++)
        {
            stmt += "" + search[ii] + ", ";
        }
        stmt.erase(stmt.size() - 2, 2);
        stmt += " FROM [" + tname + "];";
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
    template<> int select<vector<vector<wstring>>>(vector<string> search, string tname, vector<vector<wstring>>& results)
    {
        string stmt = "SELECT ";
        for (int ii = 0; ii < search.size(); ii++)
        {
            stmt += "" + search[ii] + ", ";
        }
        stmt.erase(stmt.size() - 2, 2);
        stmt += " FROM [" + tname + "];";
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
    template<> int select<string, vector<string>>(vector<string> search, string tname, string& result, vector<string>& conditions)
    {
        string stmt = "SELECT " + search[0] + " FROM [" + tname + "] WHERE (";
        for (int ii = 0; ii < conditions.size(); ii++)
        {
            stmt += conditions[ii] + " ";
        }
        stmt += ");";
        executor(stmt, result);
        return 1;
    }
    template<> int select<wstring, vector<string>>(vector<string> search, string tname, wstring& result, vector<string>& conditions)
    {
        string stmt = "SELECT " + search[0] + " FROM [" + tname + "] WHERE (";
        for (int ii = 0; ii < conditions.size(); ii++)
        {
            stmt += conditions[ii] + " ";
        }
        stmt += ");";
        executor(stmt, result);
        return 1;
    }
    template<> int select<vector<string>, vector<string>>(vector<string> search, string tname, vector<string>& results, vector<string>& conditions)
    {
        string stmt = "SELECT ";
        for (int ii = 0; ii < search.size(); ii++)
        {
            stmt += "" + search[ii] + ", ";
        }
        stmt.erase(stmt.size() - 2, 2);
        stmt += " FROM [" + tname + "] WHERE (";
        for (int ii = 0; ii < conditions.size(); ii++)
        {
            stmt += conditions[ii] + " ";
        }
        stmt += ");";
        executor(stmt, results);
        return results.size();
    }
    template<> int select<vector<vector<string>>, vector<string>>(vector<string> search, string tname, vector<vector<string>>& results, vector<string>& conditions)
    {
        string stmt = "SELECT ";
        for (int ii = 0; ii < search.size(); ii++)
        {
            stmt += "" + search[ii] + ", ";
        }
        stmt.erase(stmt.size() - 2, 2);
        stmt += " FROM [" + tname + "] WHERE (";
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
    template<> int select<vector<vector<wstring>>, vector<string>>(vector<string> search, string tname, vector<vector<wstring>>& results, vector<string>& conditions)
    {
        string stmt = "SELECT ";
        for (int ii = 0; ii < search.size(); ii++)
        {
            stmt += "" + search[ii] + ", ";
        }
        stmt.erase(stmt.size() - 2, 2);
        stmt += " FROM [" + tname + "] WHERE (";
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

    template<typename S> void select_tree(string tname, vector<vector<int>>& tree_st, vector<S>& tree_pl) {}
    template<> void select_tree<string>(string tname, vector<vector<int>>& tree_st, vector<string>& tree_pl)
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
    template<> void select_tree<wstring>(string tname, vector<vector<int>>& tree_st, vector<wstring>& tree_pl)
    {
        // Produce a tree structure and tree payload for the given table name as root. 
        // Certain table name parameters have special forks within the function.

        vector<vector<wstring>> results;
        unordered_map<wstring, int> registry;
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
};

