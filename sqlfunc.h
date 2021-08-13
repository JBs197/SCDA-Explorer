#pragma once

#include <set>
#include <thread>
#include <sqlite3.h>
#include "jfunc.h"

using namespace std;

class SQLFUNC 
{
    bool analyze = 0;
	sqlite3* db;
    string dbPath;
    ofstream ERR;
    string error_path = sroot + "\\SCDA Error Log.txt";
    JFUNC jf;
    set<string> tableSet;
    vector<string> TPrefix, tableList;
    
    void bind(string&, vector<string>&);
	void sqlerr(string);

public:
    explicit SQLFUNC() {}
	~SQLFUNC() {}
    void all_tables(vector<string>& table_list);
    void create_table(string, vector<string>&, vector<int>&);
    void executor(string stmt);
    void executor(vector<string> stmts);
    void executor(string stmt, string& result);
    void executor(string stmt, wstring& result);
    void executor(string stmt, vector<string>& results);
    void executor(string stmt, vector<vector<string>>& results);
    void executor(string stmt, vector<vector<wstring>>& results);
    void get_col_titles(string tname, vector<string>& titles);
    string getLinearizedTitle(string& cataName, string& rowTitle, string& colTitle);
    int get_num_col(string tname);
    int getNumRows(string tname);
    vector<string> getTableListFromRoot(string& root);
    vector<vector<string>> getTMapIndex();
    void get_table_list(vector<string>& results, string& search);
    vector<string> getTableList(string search);
    void init(string db_path);
    void insert(string tname, vector<string>& row_data);
    void insert(string tname, vector<vector<string>>& row_data);
    void insertBinMap(string& binPath, vector<vector<vector<int>>>& frames, double& scale, vector<double>& position, string& sParent8, vector<vector<int>>& border);
    void insertGeo(string cataName, vector<int>& gidList, vector<string>& regionList, vector<string>& layerList, vector<string>& geoLayers);
    void insertTMI(string myCoreDir);
    void insert_tg_existing(string);
    void insert_prepared(vector<string>& stmts);
    void insertPreparedBind(vector<string>& stmtAndParams);
    void insertPreparedStartStop(vector<string>& stmts, int start, int stop);
    string insert_stmt(string, vector<string>&, vector<string>&);
    void makeANSI(string&);
    unordered_map<string, string> makeMapDataIndex(string tname);
    void remove(string& tname);
    void removeCol(string& tname, string colTitle);
    void removeRow(string& tname, vector<string>& conditions);
    void safe_col(string, int);
    int sclean(string&, int);
    int select(vector<string> search, string tname, string& result);
    int select(vector<string> search, string tname, vector<string>& results);
    int select(vector<string> search, string tname, vector<vector<string>>& results);
    int select(vector<string> search, string tname, vector<vector<wstring>>& results);
    int select(vector<string> search, string tname, string& result, vector<string>& conditions);
    int select(vector<string> search, string tname, wstring& result, vector<string>& conditions);
    int select(vector<string> search, string tname, vector<string>& results, vector<string>& conditions);
    int select(vector<string> search, string tname, vector<vector<string>>& results, vector<string>& conditions);
    int select(vector<string> search, string tname, vector<vector<wstring>>& results, vector<string>& conditions);
    int selectOrderBy(vector<string> search, string tname, vector<string>& results, string orderby);
    int selectOrderBy(vector<string> search, string tname, vector<vector<string>>& results, string orderby);
    void select_tree(string tname, vector<vector<int>>& tree_st, vector<string>& tree_pl);
    vector<string> selectYears();
    void set_error_path(string);
    string sqlErrMsg();
    int statusCata(string sname);
    size_t table_exist(string);
    vector<string> test_cata(string);
    void update(string tname, vector<string> revisions, vector<string> conditions);

	// TEMPLATES

};

