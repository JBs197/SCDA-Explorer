#pragma once
#include <map>
#include <mutex>
#include <set>
#include <thread>
#include "sqlite3.h"
#include "jlog.h"
#include "jstring.h"

extern std::mutex m_err;

class SQLFUNC 
{
    bool analyze = 0;
	sqlite3* db = nullptr;
    std::string dbPath;
    JSTRING jstr;
    std::unordered_map<std::string, int> mapColType;
    std::set<std::string> setTable;
    
    void bind(std::string& stmt, std::vector<std::string>& param);
    void err(std::string message);
    void log(std::string message);
	void sqlerr(std::string func);

public:
    SQLFUNC() {}
    ~SQLFUNC() = default;

    std::set<std::string> columnType;

    void addColumn(std::string tname, std::string colTitle, std::string colType);
    void allTables(std::vector<std::string>& vsTable);
    void allTables(std::set<std::string>& setTableList);
    int count(std::string tname);
    void createTable(std::string tname, std::vector<std::vector<std::string>>& vvsColTitle, std::vector<std::string> vsUnique = {});
    void createTable(std::string& stmt, std::string tname, std::vector<std::vector<std::string>>& vvsColTitle, std::vector<std::string> vsUnique = {});
    void deleteRow(std::string tname, std::vector<std::string> conditions);
    void clearTable(std::string tname);
    void dropTable(std::string tname);
    void executor(std::string stmt);
    void executor(std::vector<std::string> stmts);
    void executor(std::string stmt, std::string& result);
    void executor(std::string stmt, std::wstring& result);
    void executor(std::string stmt, std::vector<std::string>& results);
    void executor(std::string stmt, std::vector<std::vector<std::string>>& results);
    void executor(std::string stmt, std::vector<std::vector<std::wstring>>& results);
    std::vector<std::vector<std::string>> getColTitle(std::string tname);
    void getColTitle(std::map<std::string, std::string>& mapColTitle, std::string tname);
    int getNumCol(std::string tname);
    int getNumRow(std::string tname);
    std::vector<std::string> getTableListFromRoot(std::string root);
    std::vector<std::string> getTableList(std::string search);
    void init(std::string db_path);
    void insert(std::string tname, std::vector<std::string>& rowData);
    void insert(std::string tname, std::vector<std::vector<std::string>>& vvsColTitle, std::vector<std::string>& rowData);
    void insert(std::string tname, std::vector<std::vector<std::string>>& vvsColTitle, std::vector<std::vector<std::string>>& rowData);
    void insertPrepared(std::vector<std::string>& stmts);
    void insertPreparedBind(std::vector<std::string>& stmtAndParams);
    void insertPreparedStartStop(std::vector<std::string>& stmts, int start, int stop);
    void insertRow(std::string tname, std::vector<std::vector<std::string>>& vvsRow);
    void refreshTableList();
    void removeCol(std::string tname, std::string colTitle);
    void removeRow(std::string tname, std::vector<std::string> conditions);
    void safeCol(std::string tname, std::vector<std::vector<std::string>>& vvsColTitle);
    int sclean(std::string&, int);
    int searchTableName(std::vector<std::string>& vsTable, std::string sQuery);
    int select(std::vector<std::string> search, std::string tname, std::string& result);
    int select(std::vector<std::string> search, std::string tname, std::vector<std::string>& results);
    int select(std::vector<std::string> search, std::string tname, std::vector<std::vector<std::string>>& results);
    int select(std::vector<std::string> search, std::string tname, std::vector<std::vector<std::wstring>>& results);
    int select(std::vector<std::string> search, std::string tname, std::string& result, std::vector<std::string> conditions);
    int select(std::vector<std::string> search, std::string tname, std::wstring& result, std::vector<std::string> conditions);
    int select(std::vector<std::string> search, std::string tname, std::vector<std::string>& results, std::vector<std::string> conditions);
    int select(std::vector<std::string> search, std::string tname, std::vector<std::vector<std::string>>& results, std::vector<std::string> conditions);
    int select(std::vector<std::string> search, std::string tname, std::vector<std::vector<std::wstring>>& results, std::vector<std::string> conditions);
    int selectOrderBy(std::vector<std::string> search, std::string tname, std::vector<std::string>& results, std::string orderby);
    int selectOrderBy(std::vector<std::string> search, std::string tname, std::vector<std::vector<std::string>>& results, std::string orderby);
    void selectTree(std::string tname, std::vector<std::vector<int>>& tree_st, std::vector<std::string>& tree_pl);
    std::string sqlErrMsg();
    void stmtInsertRow(std::vector<std::string>& vsStmt, std::string tname, std::vector<std::vector<std::string>>& vvsRow);
    bool tableExist(std::string tname);
    void tableExistUpdate();
    void update(std::string tname, std::vector<std::string> revisions, std::vector<std::string> conditions);
};

