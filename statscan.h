#pragma once

#include <unordered_set>
#include "switchboard.h"
#include "winfunc.h"
#include "zipfunc.h"

using namespace std;

class STATSCAN
{
	string activeCSV;
	int activeCSVpart, activeCSVgeocode, nls;
	vector<string> cataColTitles, geoHistory;
	string cataName, cataPath, cataYear, description, metaFile, topic;
	JFUNC jf;
	string nl;
	WINFUNC wf;
	ZIPFUNC zf;

public:
	STATSCAN() {}
	~STATSCAN() {}

	void init(string cataPath);
	void initCSV();

	vector<string> loadColTitles();
	vector<vector<string>> loadGeoList(string geoPath);

	string makeBinBorder(vector<vector<int>>& border);
	string makeBinFrames(vector<vector<vector<int>>>& frames);
	string makeBinParent(string parent);
	string makeBinParentNew(string binPath, string& sParent);
	string makeBinPositionGPS(vector<double> position);
	string makeBinPositionPNG(vector<double> position);
	string makeBinScale(double scale);

	string makeCreateCensus();
	vector<string> makeCreateData();
	vector<string> makeCreateInsertCatalogue();
	vector<string> makeCreateInsertDefinitions();
	vector<vector<string>> makeCreateInsertDIM();
	vector<string> makeCreateInsertTopic(vector<string>& colTitles);
	string makeCreateYear();
	string makeInsertCensus();
	vector<string> makeInsertData(int GEO_CODE, vector<vector<string>>& geoList);
	string makeInsertYear();

	string processCSVline(int& GEO_CODE, string csvLine, vector<vector<string>>& geoList);

	vector<vector<int>> readBinBorder(string& binFile);
	vector<vector<vector<int>>> readBinFrames(string& binFile);
	string readBinParent(string& binFile);
	vector<double> readBinPositionGPS(string& binFile);
	vector<double> readBinPositionPNG(string& binFile);
	double readBinScale(string& binFile);

	void trimMID(string& MID);
};