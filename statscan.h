#pragma once

#include <string>
#include <vector>
#include "jfunc.h"

using namespace std;

class STATSCAN
{
	JFUNC jf;
	string cata_path;
	string cata_name;
	string cata_desc;
	bool multi_column;
	vector<string> gid_list;
	vector<string> csv_branches;
	vector<vector<string>> text_vars;
	vector<string> column_titles;
	vector<vector<string>> rows;
	vector<string> linearized_titles;
	size_t final_text_var = 0;
	string insert_csv_row_template;
	string insert_primary_template;
	string create_csv_table_template;

	int sclean(string&, int);

public:
	explicit STATSCAN(string path) : cata_path(path) {}
	STATSCAN() {}
	~STATSCAN() {}
	void set_path(string);
	void cata_init(string&);
	vector<string> extract_column_titles(string&);
	string extract_description(string&);
	void extract_csv_branches(vector<string>&);
	void extract_gid_list(vector<string>&);
	vector<vector<string>> extract_text_vars(string&);
	vector<vector<string>> extract_rows(string&);
	string get_cata_desc();
	string get_create_csv_table_template();
	vector<string> get_gid_list();
	string get_insert_csv_row_template();
	string get_insert_primary_template();
	vector<string> linearize_row_titles(vector<vector<string>>&, vector<string>&);
	string make_csv_path(int);
	string make_create_csv_table_template(vector<string>&);
	string make_insert_csv_row_template(vector<string>&);
	string make_insert_primary_template(string, vector<vector<string>>&, vector<string>&);


	// TEMPLATES
	template<typename ... Args> string make_create_primary_table(Args& ... args)
	{
		// Returns a complete statement to create a catalogue's primary table. 
		// If no parameters are given, 'cata_init' must already have been completed so as to
		// populate the class internal variables. Otherwise, the catalogue name, text variables, 
		// and row titles must be given via parameters.
	}
	template<> string make_create_primary_table()
	{
		// This function returns the statement to create the primary table for this catalogue. 

		string stmt = "CREATE TABLE IF NOT EXISTS [" + cata_name + "] (";
		stmt += "GID INTEGER PRIMARY KEY, ";
		for (int ii = 0; ii < text_vars.size(); ii++)
		{
			stmt += "[" + text_vars[ii][0] + "] TEXT, ";
		}
		for (int ii = 0; ii < linearized_titles.size(); ii++)
		{
			stmt += "[" + linearized_titles[ii] + "] NUMERIC, ";
		}
		stmt.erase(stmt.size() - 2, 2);
		stmt += ");";
		return stmt;
	}
	template<> string make_create_primary_table<string&, vector<vector<string>>&, vector<string>&>(string& cata_name, vector<vector<string>>& text_vars, vector<string>& linearized_titles)
	{
		// This function returns the statement to create the primary table for this catalogue. 

		string stmt = "CREATE TABLE IF NOT EXISTS [" + cata_name + "] (";
		stmt += "GID INTEGER PRIMARY KEY, ";
		for (int ii = 0; ii < text_vars.size(); ii++)
		{
			stmt += "[" + text_vars[ii][0] + "] TEXT, ";
		}
		for (int ii = 0; ii < linearized_titles.size(); ii++)
		{
			stmt += "[" + linearized_titles[ii] + "] NUMERIC, ";
		}
		stmt.erase(stmt.size() - 2, 2);
		stmt += ");";
		return stmt;
	}

};

