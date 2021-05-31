#pragma once
#include <string>
#include <vector>
#include "switchboard.h"
#include "winfunc.h"

using namespace std;

class STATSCAN
{
	JFUNC jf;
	WINFUNC wf;

	string cata_path;
	string cata_name;
	string cata_desc;
	vector<string> column_titles;
	string create_csv_table_template;
	vector<string> csv_branches;
	vector<vector<int>> csv_tree;  // Form [row_index][ancestor1's row_index, ... , (neg) row_index, child1's row_index, ...].
	//size_t final_text_var = 0;
	vector<string> gid_list;
	string insert_csv_row_template;
	string insert_primary_template;
	vector<string> linearized_titles;
	unordered_map<string, string> mapGeoLayers;
	bool multi_column;
	vector<vector<string>> navSearch;
	vector<vector<string>> rows;  // Form [row_index][indented row title, row val1, row val2, ...].
	vector<vector<string>> text_vars;
	vector<string> subtable_names_template;  // Has '!!!' for GID. Otherwise, is complete.

	int sclean(string&, int);

public:
	explicit STATSCAN(string path) : cata_path(path) {}
	STATSCAN() {}
	~STATSCAN() {}
	void set_path(string);
	int cata_init(string&);
	void cleanURL(string& url);
	vector<string> disassembleNameCSV(string& fileName);
	void downloadCatalogue(SWITCHBOARD& sbgui);
	void downloadGeoList(string sYear, string sName, string& geoPage);
	void downloadMaps(SWITCHBOARD& sbgui);
	void err(string);
	vector<string> extract_column_titles(string&);
	string extract_description(string&);
	void extract_csv_branches(vector<string>&);
	void extract_gid_list(vector<string>&);
	vector<vector<string>> extract_text_vars(string& sfile, size_t& finalTextVar);
	vector<vector<string>> extract_rows(string& sfile, int& damaged, size_t& finalTextVar);
	string geoLinkToRegionUrl(string& urlGeoList, string& geoLink);
	vector<string> getLayerSelected(string& sfile);
	string get_cata_desc();
	string get_cata_name();
	vector<string> get_column_titles();
	string get_create_csv_table_template();
	vector<vector<int>> get_csv_tree();
	string get_gid(int);
	vector<string> get_gid_list();
	string get_insert_csv_row_template();
	string get_insert_primary_template();
	int get_num_subtables();
	string get_subtable_name_template(int);
	void initGeo();
	vector<string> linearize_row_titles(vector<vector<string>>&, vector<string>&);
	vector<string> makeGeoLayers(string& lineGeo);
	string makeGeoList(vector<string>& geoLinkNames, vector<string>& geoLayers, string geoURL);
	string make_csv_path(int);
	wstring make_csv_wpath(int);
	string make_create_csv_table_statement(string&, string, string&);
	string make_create_csv_table_template(vector<string>&);
	void make_insert_csv_row_statement(string&, string, vector<string>&);
	string make_insert_csv_row_template(vector<string>&);
	string make_insert_damaged_csv(string, string, int);
	void make_insert_primary_statement(string&, string, vector<vector<string>>&, vector<vector<string>>&);
	string make_insert_primary_template(string, vector<vector<string>>&, vector<string>&);
	string make_subtable_name(int&, string, string, vector<int>&, vector<string>&);
	int make_tgr_statements(vector<string>&, string, string);
	int make_tgrow_statements(vector<string>&);
	string mapLinkToPDFUrl(string& urlMap, string& mapLink);
	vector<vector<string>> navAsset();
	vector<vector<string>> readGeo(string& geoPath);
	vector<vector<string>> readGeo(string& geoPath, unordered_map<string, string>& mapGeo);
	string regionLinkToMapUrl(string& urlRegion, string& regionLink);
	int skimGeoList(string& filePath, vector<string>& geoLayers);
	vector<vector<string>> splitLinkNames(vector<string>& linkNames);
	bool testCanadaOnly(string& geoLayer);
	bool testFileNotFound(string& webpage);
	bool testGeoList(string& filePath);
	string urlCatalogue(int iYear, string sCata);
	string urlCataDownload(int iyear, string& geoPage, string gid);
	string urlCataList(int iyear, string scata);
	string urlEntireTableDownload(int iYear, string& urlCata);
	string urlGeoList(int iyear, string urlCata);
	string urlYear(string);

	// TEMPLATES
	template<typename ... Args> void getGidFromPath(Args& ... args)
	{
		err("getGidFromPath template-sc");
	}
	template<> void getGidFromPath<string, string>(string& csvPath, string& csvGid)
	{
		size_t pos1 = csvPath.find('(') + 1;
		size_t pos2 = csvPath.find(')', pos1);
		csvGid = csvPath.substr(pos1, pos2 - pos1);
	}
	template<> void getGidFromPath<vector<string>, vector<string>>(vector<string>& csvPaths, vector<string>& csvGids)
	{
		size_t pos1, pos2;
		csvGids.resize(csvPaths.size());
		for (int ii = 0; ii < csvPaths.size(); ii++)
		{
			pos1 = csvPaths[ii].find('(') + 1;
			pos2 = csvPaths[ii].find(')', pos1);
			csvGids[ii] = csvPaths[ii].substr(pos1, pos2 - pos1);
		}
	}

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
	template<> vector<string> make_subtable_names_template<vector<vector<string>>>(string cata_name, int& tg_row_col, vector<vector<int>>& tree_st, vector<vector<string>>& rows)
	{
		vector<string> tree_pl(rows.size());
		for (int ii = 0; ii < rows.size(); ii++)
		{
			tree_pl[ii] = rows[ii][0];
		}

		vector<string> subtable_list;
		tg_row_col = 0;
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
				if (tg_row_col <= num_param) { tg_row_col = num_param + 1; }
				subtable_list.push_back(tname);
			}
		}
		return subtable_list;
	}

};

