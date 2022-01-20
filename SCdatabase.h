#pragma once
#include "jtree.h"
#include "sqlfunc.h"
#include "switchboard.h"

class SCdatabase
{
	JFUNC jf;
	std::unordered_map<std::string, std::string> mapGeoLayers;
	const char marker;
	std::string sYear;

	void err(std::string message);
	std::vector<std::string> extractUnique(std::vector<std::vector<std::string>>& vvsTag);
	void insertCensus(std::string sYear);
	void insertCensusYear(std::string& metaFile, std::string sYear, std::string sCata, std::string sTopic);
	void insertTopicYear(std::string sYear, std::string sTopic);
	void loadGeo(std::vector<std::vector<std::string>>& vvsGeo, std::vector<std::vector<std::string>>& vvsGeoLayers, std::string dirCata, std::string& sCata);
	void loadMeta(std::string& metaFile, std::string dirCata);
	void loadTopic(std::string& sTopic, std::string dirCata);
	void log(std::string message);
	bool safeInsertRow(std::string tname, std::vector<std::vector<std::string>>& vvsRow);

public:
	SCdatabase() : marker('$') {}
	~SCdatabase() = default;

	std::string configXML;
	SQLFUNC sf;

	void deleteTable(std::string tname);
	void init(std::string& xml);
	void insertCata(SWITCHBOARD& sbgui);
	void loadTable(std::vector<std::vector<std::string>>& vvsData, std::vector<std::vector<std::string>>& vvsColTitle, std::string tname);
	void makeTreeCata(SWITCHBOARD& sbgui, JTREE& jt);
	void searchTable(SWITCHBOARD& sbgui, JTREE& jt, std::vector<std::string>& vsTable);
};