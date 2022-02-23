#pragma once
#include "jfile.h"
#include "jparse.h"
#include "jsort.h"
#include "jtree.h"
#include "switchboard.h"
#include "winfunc.h"

class SConline
{
	JFILE jfile;
	JPARSE jparse;
	JSORT jsort;
	JTREE jt;
	std::unordered_map<int, int> mapGeoIndent;
	std::unordered_map<std::string, std::string> mapGeoLayer;
	std::unordered_map<std::string, int> mapGeoLevel;
	long long maxFileSize = -1;
	std::vector<std::vector<std::string>> vvsCata;  // Form [index][year, cata0, cata1, ...]

	std::string cataPID(std::string sYear, std::string sCata);
	
	void err(std::string message);
	void makeGeo(std::string cataFolder, std::string sYear, std::string sCata);
	void makeGeoTree(std::string yearFolder, std::string sYear, SWITCHBOARD& sbgui);
	void urlCataDownload(std::string& url, std::string sYear, std::string sCata);
	void urlCataGeo(std::string& url, std::string sYear, std::string sCata);
	void urlCataTopic(std::string& url, std::string sYear, std::string sCata);
	void urlYear(std::string& url, std::string sYear);

public:
	SConline() {}
	~SConline() = default;

	std::string configXML, urlRoot;
	WINFUNC wf;

	void downloadTopicDesc(std::string cataDir);

	void downloadCata(SWITCHBOARD& sbgui);
	void getCataTree(JTREE& jt);
	void init(std::string& xml);
	std::vector<std::string> getListCata(std::string sYear);
	std::vector<std::string> getListYear();
};