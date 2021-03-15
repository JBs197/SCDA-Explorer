#pragma once

#include <string>
#include <vector>
#include <chrono>
#include <fstream>
#include <sqlite3.h>

using namespace std;

class SQLFUNC
{
	sqlite3* db;
	sqlite3_stmt* state;
	string error_path;

    void bind(string&, vector<string>&);
    int sclean(string&, int);
	void sqlerr(string);
	string timestamper();

public:
	explicit SQLFUNC() {}
	~SQLFUNC() {}
	void init(string);
    void create_table(string, vector<string>&, vector<int>&);
    void insert_rows(string, vector<vector<string>>&);

	// TEMPLATES
	template<typename ... Args> void executor(string, Args& ... args) {}
	template<> void executor(string stmt)
	{
		int error = sqlite3_prepare_v2(db, stmt.c_str(), -1, &state, NULL);
		if (error) { sqlerr("prepare-executor0"); }
		error = sqlite3_step(state);
		if (error > 0 && error != 100 && error != 101) 
		{
			sqlerr("step-executor0"); 
		}
	}
	template<> void executor<vector<string>>(string stmt, vector<string>& results)
	{
        // Note that this variant of the executor function can accomodate either a column or a row as the result.
        int type, size;  // Type: 1(int), 2(double), 3(string)
        int error = sqlite3_prepare_v2(db, stmt.c_str(), -1, &state, NULL);
        if (error) { sqlerr("prepare-executor1"); }
        error = sqlite3_step(state);
        int ivalue;
        double dvalue;
        string svalue;
        int col_count = -1;

        while (error == 100)
        {
            if (col_count < 0)
            {
                col_count = sqlite3_column_count(state);
            }
            if (col_count > 1)  // Returned vector will be a row.
            {
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
                break;
            }
            else  // Returned result will be a column.
            {
                type = sqlite3_column_type(state, 0);
                switch (type)
                {
                case 1:
                    ivalue = sqlite3_column_int(state, 0);
                    results.push_back(to_string(ivalue));
                    break;
                case 2:
                    dvalue = sqlite3_column_double(state, 0);
                    results.push_back(to_string(dvalue));
                    break;
                case 3:
                    size = sqlite3_column_bytes(state, 0);
                    char* buffer = (char*)sqlite3_column_text(state, 0);
                    svalue.assign(buffer, size);
                    results.push_back(svalue);
                    break;
                }
            }
        }
        if (error > 0 && error != 101)
        {
            sqlerr("step-executor1");
        }
	}
    template<> void executor<vector<vector<string>>>(string stmt, vector<vector<string>>& results)
    {
        int type, col_count, size;  // Type: 1(int), 2(double), 3(string)
        int error = sqlite3_prepare_v2(db, stmt.c_str(), -1, &state, NULL);
        if (error) { sqlerr("prepare-executor2"); }
        error = sqlite3_step(state);
        int ivalue;
        double dvalue;
        string svalue;

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

    template<typename ... Args> void select(string, string, Args& ... args)
    {
        // Work in progress. In time, the select functions will grow considerably in complexity.
    }
    template<> void select<vector<string>>(string search, string tname, vector<string>& results)
    {
        string stmt = "SELECT " + search + " FROM [" + tname + "];";
        executor(stmt, results);
    }
    template<> void select<vector<vector<string>>>(string search, string tname, vector<vector<string>>& results)
    {
        string stmt = "SELECT " + search + " FROM [" + tname + "];";
        executor(stmt, results);
    }

};

