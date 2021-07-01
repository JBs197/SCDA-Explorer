#pragma once

#include <unordered_map>
#include "switchboard.h"
#include "winfunc.h"
#include "zipfunc.h"

using namespace std;

class STATSCAN
{
	string activeCSV, activeCSVgeocode;
	vector<int> activeColIndex;
	int activeCSVpart, geoCodeCol = -1, geoLevelCol = -1, geoMaxLevel = -1, nls;
	vector<string> cataColTitles, geoHistory;
	string cataName, cataPath, cataYear, description, metaFile, topic;
	JFUNC jf;
	string nl;
	unordered_map<string, int> mapGeoCode;  // Passively updated by loadGeoList
	unordered_map<int, int> mapDIM, mapDim;  // Form indexDIM->cataColTitles[index]
	WINFUNC wf;
	ZIPFUNC zf;

public:
	STATSCAN() {}
	~STATSCAN() {}

	void advanceNextCSVpart();
	void convertSCgeo(string& geoPathOld);
	string extractCSVLineValue(string& csvLine, int colIndex);
	vector<string> extractCSVLineValue(string& csvLine, vector<int> colIndex);

	int getGeoCodeIndex(string& sActiveCSVgeocode);
	void init(string cataPath);
	int initCSV(vector<vector<string>>& geoList);

	int loadBinGeo(string& filePath, vector<int>& gidList, vector<string>& regionList, vector<string>& layerList, vector<string>& geoLayers);
	void loadBookmark(int& iActiveCSVpart, string& sActiveCSVgeocode);
	vector<string> loadColTitles();
	vector<vector<string>> loadGeoList(string geoPath);

	string makeBinBorder(vector<vector<int>>& border);
	string makeBinFrames(vector<vector<vector<int>>>& frames);
	string makeBinParent(string parent);
	string makeBinParentNew(string binPath, string& sParent);
	string makeBinPositionGPS(vector<double> position);
	string makeBinPositionPNG(vector<double> position);
	string makeBinScale(double scale);
	void makeBookmark(int iActiveCSVpart, string sActiveCSVgeocode);

	string makeCreateCensus();
	vector<string> makeCreateData(vector<vector<string>>& geoList);
	string makeCreateGeo(vector<vector<string>>& geoList);
	vector<string> makeCreateInsertCatalogue();
	vector<string> makeCreateInsertDefinitions();
	vector<vector<string>> makeCreateInsertDIM();
	vector<string> makeCreateInsertTopic(vector<string>& colTitles);
	string makeCreateYear();
	string makeInsertCensus();
	vector<string> makeInsertData(vector<vector<string>>& geoList);
	vector<string> makeInsertGeo(vector<vector<string>>& geoList);
	string makeInsertYear();

	int processCSVline(string& csvLine, vector<string>& insertData);

	vector<vector<int>> readBinBorder(string& binFile);
	vector<vector<vector<int>>> readBinFrames(string& binFile);
	string readBinParent(string& binFile);
	vector<double> readBinPositionGPS(string& binFile);
	vector<double> readBinPositionPNG(string& binFile);
	double readBinScale(string& binFile);

	void trimMID(string& MID);
	void uptickBookmark(vector<vector<string>>& geoList);
};