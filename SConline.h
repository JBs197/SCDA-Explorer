#pragma once
#include "jfunc.h"
#include "jtree.h"
#include "switchboard.h"
#include "winfunc.h"

class SConline
{
	JFUNC jf;
	JTREE jt;
	std::unordered_map<std::string, std::string> mapGeoLayer;
	std::unordered_map<int, int> mapGeoLevel;
	long long maxFileSize = -1;
	std::vector<std::vector<std::string>> vvsCata;  // Form [index][year, cata0, cata1, ...]
	WINFUNC wf;

	void downloadTopic(std::string filePath);
	void err(std::string message);
	void makeGeo(std::string cataFolder, std::string sYear, std::string sCata);
	void makeGeoTree(std::string yearFolder, std::string sYear, SWITCHBOARD& sbgui);
	std::string urlCataDownload(std::string sYear, std::string sCata);
	std::string urlCataGeo(std::string sYear, std::string sCata);
	std::string urlCataTopic(std::string sYear, std::string sCata);
	std::string urlYear(std::string sYear);

public:
	SConline() {}
	~SConline() = default;

	std::string configXML, urlRoot;

	void downloadCata(SWITCHBOARD& sbgui);
	void getCataTree(JTREE& jt);
	void init(std::string& xml);
	std::vector<std::string> getListCata(std::string sYear);
	std::vector<std::string> getListYear();
};