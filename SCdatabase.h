#pragma once
#include "jfile.h"
#include "jparse.h"
#include "jtxml.h"
#include "sqlfunc.h"
#include "switchboard.h"

#define NUM_BUF_SLOT 6

class SCdatabase
{
	JFILE jfile;
	JPARSE jparse;
	std::unordered_map<std::string, std::string> mapGeoLayers;
	const std::string marker;

	void createData(std::string sYear, std::string sCata, const std::vector<std::vector<std::string>>& vvsGeo, int numCol);
	void dataParser(std::atomic_int& fileDepleted, std::string sYear, std::string sCata, JBUFFER<std::string, NUM_BUF_SLOT>& jbufRaw, JBUFFER<std::vector<std::string>, NUM_BUF_SLOT>& jbufSQL);
	void dataReader(std::atomic_int& fileDepleted, std::string cataDir, std::string sYear, std::string sCata, JBUFFER<std::string, NUM_BUF_SLOT>& jbufRaw);
	void err(std::string message);
	bool hasGeoGap(std::vector<std::string>& vsGeoLayer, std::vector<std::vector<std::string>>& vvsGeoLevel, std::vector<int>& viGeoLevel, std::vector<std::vector<std::string>>& vvsGeo);
	void insertCensusYear(std::string sYear, std::string sCata, std::string sTopic);
	void insertData(SWITCHBOARD& sbgui, std::string cataDir, std::string sYear, std::string sCata, int numThread);
	void insertDataIndex(const std::vector<int> vDIM, std::string sYear, std::string sCata);
	std::vector<int> insertDIM(JTXML*& jtxml, std::unordered_map<std::string, std::string>& mapMeta, std::string sYear, std::string sCata);
	void insertDIMIndex(JTXML*& jtxml, std::unordered_map<std::string, std::string>& mapMeta, std::string sYear, std::string sCata);
	void insertForWhom(JTXML*& jtxml, std::unordered_map<std::string, std::string>& mapMeta, std::string sYear, std::string sCata);
	void insertGeo(std::vector<std::vector<std::string>>& vvsGeo, std::string sYear, std::string sCata);
	void insertGeo(std::vector<std::string>& vsGeoLayer, std::vector<std::vector<std::string>>& vvsGeo, std::string sYear, std::string sCata);
	void insertGeoLayer(std::vector<std::string>& vsGeoLayer, std::string sYear, std::string sCata);
	void insertTopicYear(std::string sYear, std::string sTopic);
	void loadData(JTXML*& jtxml, std::unordered_map<std::string, std::string>& mapData, std::string cataDir, std::string sYear, std::string sCata);
	void loadGeo(std::vector<std::vector<std::string>>& vvsGeo, std::vector<std::string>& vsGeoLayer, std::string cataDir, std::string sCata);
	void loadMeta(JTXML*& jtxml, std::unordered_map<std::string, std::string>& mapMeta, std::string cataDir, std::string sYear, std::string sCata);
	void loadTopic(std::string& sTopic, std::string cataDir);
	void log(std::string message);
	std::uintmax_t makeDataIndex(const std::vector<int>& vMID, const std::vector<int>& vSize);
	void prepareLocal(std::vector<std::string>& vsLocalPath, std::string cataDir, std::string sCata);
	void xmlToColTitle(std::vector<std::vector<std::string>>& vvsColTitle, std::vector<std::string>& vsUnique, std::vector<std::vector<std::string>>& vvsTag);

public:
	SCdatabase() : marker("$") {}
	~SCdatabase() = default;

	std::string configXML;
	SQLFUNC sf;

	void deleteTable(std::string tname);
	void deleteTableRow(std::string tname, std::vector<std::string>& vsCell);
	void init(std::string& xml);
	void insertCata(SWITCHBOARD& sbgui, int numThread);
	void insertCensus(std::string sYear);
	void insertGeoTree(std::string yearDir);
	void insertGeoTreeTemplate(std::string yearDir);
	void loadTable(std::vector<std::vector<std::string>>& vvsData, std::vector<std::vector<std::string>>& vvsColTitle, std::string tname);
	void makeTreeCata(SWITCHBOARD& sbgui, JTREE& jt);
	void makeTreeGeo(SWITCHBOARD& sbgui, JTREE& jt, int branchID);
	bool safeInsertRow(std::string tname, std::vector<std::vector<std::string>>& vvsRow);
	void searchTable(SWITCHBOARD& sbgui, JTREE& jt, std::vector<std::string>& vsTable);
};