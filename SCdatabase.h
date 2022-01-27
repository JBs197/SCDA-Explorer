#pragma once
#include "jfile.h"
#include "jparse.h"
#include "jtxml.h"
#include "sqlfunc.h"
#include "switchboard.h"

class SCdatabase
{
	JFILE jfile;
	JPARSE jparse;
	std::unordered_map<std::string, std::string> mapGeoLayers;
	const std::string marker;

	void err(std::string message);
	bool hasGeoGap(std::vector<std::string>& vsGeoLayer, std::vector<std::vector<std::string>>& vvsGeoLevel, std::vector<int>& viGeoLevel, std::vector<std::vector<std::string>>& vvsGeo);
	void insertCensusYear(std::string sYear, std::string sCata, std::string sTopic);
	void insertDIM(JTXML*& jtxml, std::unordered_map<std::string, std::string>& mapMeta, std::string sYear, std::string sCata);
	void insertForWhom(JTXML*& jtxml, std::unordered_map<std::string, std::string>& mapMeta, std::string sYear, std::string sCata);
	void insertGeo(std::vector<std::vector<std::string>>& vvsGeo, std::string sYear, std::string sCata);
	void insertGeo(std::vector<std::string>& vsGeoLayer, std::vector<std::vector<std::string>>& vvsGeo, std::string sYear, std::string sCata);
	std::vector<std::string> insertGeoLayer(std::string cataDir, std::string sYear, std::string sCata);
	void insertTopicYear(std::string sYear, std::string sTopic);
	void loadGeo(std::vector<std::vector<std::string>>& vvsGeo, std::string cataDir, std::string sCata);
	void loadMeta(JTXML*& jtxml, std::unordered_map<std::string, std::string>& mapMeta, std::string cataDir, std::string sYear, std::string sCata);
	void loadTopic(std::string& sTopic, std::string cataDir);
	void log(std::string message);
	void prepareLocal(std::string cataDir, std::string sCata, long long maxFileSize);
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
	void insertGeoTree(std::string yearDir);
	void insertGeoTreeTemplate(std::string yearDir);
	void loadTable(std::vector<std::vector<std::string>>& vvsData, std::vector<std::vector<std::string>>& vvsColTitle, std::string tname);
	void makeTreeCata(SWITCHBOARD& sbgui, JTREE& jt);
	void searchTable(SWITCHBOARD& sbgui, JTREE& jt, std::vector<std::string>& vsTable);
};