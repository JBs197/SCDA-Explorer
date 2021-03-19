#pragma once

#include <string>
#include <vector>
#include "jfunc.h"

using namespace std;

class STATSCAN
{
	JFUNC jf_sc;
	string cata_path;
	string cata_name;
	string cata_desc;
	bool multi_column;
	vector<string> gid_list;
	vector<string> csv_branches;
	vector<vector<string>> text_vars;
	vector<string> column_titles;
	vector<vector<string>> rows;  // Form [row_index][row title, row val1, row val2, ...].
	vector<string> linearized_titles;
	vector<vector<int>> csv_tree;  // Form [row_index][ancestor1's row_index, ... , (neg) row_index, child1's row_index, ...].
	vector<string> subtable_names_template;  // Has '!!!' for GID. Otherwise, is complete.
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
	int cata_init(string&);
	vector<string> extract_column_titles(string&);
	string extract_description(string&);
	void extract_csv_branches(vector<string>&);
	void extract_gid_list(vector<string>&);
	vector<vector<string>> extract_text_vars(string&);
	vector<vector<string>> extract_rows(string&, int&);
	string get_cata_desc();
	string get_cata_name();
	vector<string> get_column_titles();
	string get_create_csv_table_template();
	vector<vector<int>> get_csv_tree();
	vector<string> get_gid_list();
	string get_insert_csv_row_template();
	string get_insert_primary_template();
	int get_num_subtables();
	string get_subtable_name_template(int);
	vector<string> linearize_row_titles(vector<vector<string>>&, vector<string>&);
	string make_csv_path(int);
	string make_create_csv_table_statement(string&, string, string&);
	string make_create_csv_table_template(vector<string>&);
	void make_insert_csv_row_statement(string&, string, vector<string>&);
	string make_insert_csv_row_template(vector<string>&);
	vector<string> make_insert_csv_subtable_statements(string, vector<vector<int>>&, vector<vector<string>>&);
	string make_insert_damaged_csv(string, string, int);
	void make_insert_primary_statement(string&, string, vector<vector<string>>&, vector<vector<string>>&);
	string make_insert_primary_template(string, vector<vector<string>>&, vector<string>&);
	string make_subtable_name(int&, string, string, vector<int>&, vector<string>&);
	string make_tg_insert_statement(vector<string>&);


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

	template<typename ... Args> vector<string> make_subtable_names_template(string, int&, vector<vector<int>>&, Args& ... args)
	{
		// Produces a list of statements, each one to create a subtable. These statements are
		// complete except for the gid parameter, which is represented by '!!!'. The referenced
		// integer parameter is the highest number of tname parameters present within the list.
	}
	template<> vector<string> make_subtable_names_template<vector<string>>(string cata_name, int& tg_max, vector<vector<int>>& tree_st, vector<string>& tree_pl)
	{
		vector<string> subtable_list;
		tg_max = 0;
		int num_param, pos;
		string tname;
		for (int ii = 0; ii < tree_st.size(); ii++)
		{
			// Locate this node's position within its tree vector.
			for (int jj = 0; jj < tree_st[ii].size(); jj++)
			{
				if (tree_st[ii][jj] < 0)
				{
					pos = jj;
					break;
				}
				else if (jj == tree_st[ii].size() - 1)
				{
					pos = 0;
				}
			}
			
			// Generate a subtable name for this node if it is a parent.
			if (pos < tree_st[ii].size() - 1)
			{
				tname = make_subtable_name(num_param, cata_name, "!!!", tree_st[ii], tree_pl);
				if (num_param > tg_max) { tg_max = num_param; }
				subtable_list.push_back(tname);
			}			
		}
		return subtable_list;
	}
	template<> vector<string> make_subtable_names_template<vector<vector<string>>>(string cata_name, int& tg_max, vector<vector<int>>& tree_st, vector<vector<string>>& rows)
	{
		vector<string> tree_pl(rows.size());
		for (int ii = 0; ii < rows.size(); ii++)
		{
			tree_pl[ii] = rows[ii][0];
		}

		vector<string> subtable_list;
		tg_max = 0;
		int num_param, pos;
		string tname;
		for (int ii = 0; ii < tree_st.size(); ii++)
		{
			// Locate this node's position within its tree vector.
			for (int jj = 0; jj < tree_st[ii].size(); jj++)
			{
				if (tree_st[ii][jj] < 0)
				{
					pos = jj;
					break;
				}
				else if (jj == tree_st[ii].size() - 1)
				{
					pos = 0;
				}
			}

			// Generate a subtable name for this node if it is a parent.
			if (pos < tree_st[ii].size() - 1)
			{
				tname = make_subtable_name(num_param, cata_name, "!!!", tree_st[ii], tree_pl);
				if (num_param > tg_max) { tg_max = num_param; }
				subtable_list.push_back(tname);
			}
		}
		return subtable_list;
	}

};

