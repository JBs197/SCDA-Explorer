#pragma once
#include "jtree.h"
#include "sqlfunc.h"
#include "switchboard.h"

using namespace std;

class SCdatabase
{
	JFUNC jf;
	unordered_map<string, string> mapGeoLayers;
	const char marker;
	string sYear;

	void err(string message);
	void insertRow(string tname, vector<vector<string>>& vvsRow);
	void loadGeo(vector<vector<string>>& vvsGeo, vector<vector<string>>& vvsGeoLayers, string dirCata, string& sYear, string& sCata);
	void log(string message);
	vector<vector<string>> makeGeoLayers(string sCata, vector<vector<string>>& vvsGeo);
	bool safeInsertRow(string tname, vector<vector<string>>& vvsRow);

public:
	SCdatabase() :marker('$') {}
	~SCdatabase() {}

	string configXML;
	SQLFUNC sf;

	void initDatabase(string dbPath) { sf.init(dbPath); }
	void insertCata(SWITCHBOARD& sbgui);
	void loadTable(vector<vector<string>>& vvsData, vector<vector<string>>& vvsColTitle, string tname);
	void makeTreeCata(SWITCHBOARD& sbgui, JTREE& jt);
	void searchTable(SWITCHBOARD& sbgui, JTREE& jt, vector<string>& vsTable);
};