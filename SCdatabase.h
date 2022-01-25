#pragma once
#include "jfile.h"
#include "jparse.h"
#include "jtree.h"
#include "sqlfunc.h"
#include "switchboard.h"

class SCdatabase
{
	JFILE jfile;
	JPARSE jparse;
	std::unordered_map<std::string, std::string> mapGeoLayers;
	const std::string marker;

	void err(std::string message);
	void insertCensusYear(std::string sYear, std::string sCata, std::string sTopic);
	void insertGeo(std::string cataDir, std::string sYear, std::string sCata);
	void insertGeoLayer(std::string cataDir, std::string sYear, std::string sCata);
	void insertTopicYear(std::string sYear, std::string sTopic);
	//void loadMeta(std::string& metaFile, std::string dirCata);
	void loadTopic(std::string& sTopic, std::string cataDir);
	void log(std::string message);
	bool safeInsertRow(std::string tname, std::vector<std::vector<std::string>>& vvsRow);
	void xmlToColTitle(std::vector<std::vector<std::string>>& vvsColTitle, std::vector<std::string>& vsUnique, std::vector<std::vector<std::string>>& vvsTag);

public:
	SCdatabase() : marker("$") {}
	~SCdatabase() = default;

	std::string configXML;
	SQLFUNC sf;

	void deleteTable(std::string tname);
	void init(std::string& xml);
	void insertCata(SWITCHBOARD& sbgui);
	void insertCensus(std::string sYear);
	void insertGeoTreeTemplate(std::string yearDir);
	void loadTable(std::vector<std::vector<std::string>>& vvsData, std::vector<std::vector<std::string>>& vvsColTitle, std::string tname);
	void makeTreeCata(SWITCHBOARD& sbgui, JTREE& jt);
	void searchTable(SWITCHBOARD& sbgui, JTREE& jt, std::vector<std::string>& vsTable);
};