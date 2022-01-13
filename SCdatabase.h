#pragma once
#include "jtree.h"
#include "sqlfunc.h"
#include "switchboard.h"

using namespace std;

class SCdatabase
{
	JFUNC jf;
	const char marker;
	SQLFUNC sf;

	void err(string message);
	void log(string message);

public:
	SCdatabase() :marker('$') {}
	~SCdatabase() {}

	string configXML;

	void initDatabase(string dbPath) { sf.init(dbPath); }
	void insertCata(SWITCHBOARD& sbgui);
	void loadTable(vector<vector<string>>& vvsData, vector<vector<string>>& vvsColTitle, string tname);
	void makeTreeCata(SWITCHBOARD& sbgui, JTREE& jt);
	void searchTable(SWITCHBOARD& sbgui, JTREE& jt, vector<string>& vsTable);
};