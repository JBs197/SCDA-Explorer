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
	vector<string> extractUnique(vector<vector<string>>& vvsTag);
	void insertCensus(string sYear);
	void insertCensusYear(string& metaFile, string sYear, string sCata, string sTopic);
	void insertTopicYear(string sYear, string sTopic);
	void loadGeo(vector<vector<string>>& vvsGeo, vector<vector<string>>& vvsGeoLayers, string dirCata, string& sCata);
	void loadMeta(string& metaFile, string dirCata);
	void loadTopic(string& sTopic, string dirCata);
	void log(string message);
	bool safeInsertRow(string tname, vector<vector<string>>& vvsRow);

public:
	SCdatabase() :marker('$') {}
	~SCdatabase() {}

	string configXML;
	SQLFUNC sf;

	void deleteTable(string tname);
	void initDatabase(string dbPath) { sf.init(dbPath); }
	void insertCata(SWITCHBOARD& sbgui);
	void loadTable(vector<vector<string>>& vvsData, vector<vector<string>>& vvsColTitle, string tname);
	void makeTreeCata(SWITCHBOARD& sbgui, JTREE& jt);
	void searchTable(SWITCHBOARD& sbgui, JTREE& jt, vector<string>& vsTable);
};