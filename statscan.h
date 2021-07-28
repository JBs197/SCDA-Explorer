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
	int activeCSVpart, geoCodeCol = -1, geoLevelCol = -1, geoNameCol = -1, nls;
	vector<string> cataColTitles, geoHistory;
	string cataName, cataPath, cataYear, description, metaFile, topic;
	vector<vector<string>> geo;
	bool ignoreSplitRegions = 1;
	JFUNC jf;
	string nl;
	unordered_map<string, int> mapGeoCode;  // Passively updated by loadGeoList
	unordered_map<string, string> mapGeoLayer;  // Listed layer name->internal name
	unordered_map<int, int> mapDIM, mapDim;  // Form indexDIM->cataColTitles[index]
	WINFUNC wf;
	ZIPFUNC zf;

public:
	STATSCAN() {}
	~STATSCAN() {}

	void advanceNextCSVpart();
	vector<string> compareGeoListDB(vector<vector<string>>& geoList, vector<string>& dbList);
	void convertSCgeo(string& geoPathOld);
	string extractCSVLineValue(string& csvLine, int colIndex);
	vector<string> extractCSVLineValue(string& csvLine, vector<int> colIndex);
	string getCSVPath(int PART);
	int getGeoCodeIndex(string& sActiveCSVgeocode);
	string getGeoLayer(string geoLayerExternal);
	void init(string cataPath);
	void initCSV(string activeGeoCode, string sActivePART);
	void initGeo();

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
	vector<string> makeCreateData(vector<string>& DIMList, vector<string>& dimList);
	string makeCreateGeo();
	string makeCreateGeoLayers(string sYear);
	vector<string> makeCreateInsertDIMIndex(vector<string>& DIMList);
	vector<string> makeCreateInsertDIM();
	vector<string> makeCreateInsertDim(vector<string>& dimList);
	vector<string> makeCreateInsertTopic(vector<string>& colTitles);
	string makeCreateMap(string tname);
	string makeCreateMapFrame(string tname);
	string makeCreateYear();

	string makeInsertCensus();
	vector<string> makeInsertData(string GEO_CODE, string& geoStmt);
	vector<string> makeInsertGeo(vector<vector<string>>& geoList);
	vector<string> makeInsertMap(string tname, string mapPath);
	vector<string> makeInsertMapFrame(string tname, string mapPath);
	string makeInsertYear();

	unordered_map<string, string> mapGeoCodeToPart(vector<vector<string>>& geoList);
	vector<vector<string>> parseNavSearch(string& navSearchBlob);

	vector<vector<int>> readBinBorder(string& binFile);
	vector<vector<vector<int>>> readBinFrames(string& binFile);
	string readBinParent(string& binFile);
	vector<double> readBinPositionGPS(string& binFile);
	vector<double> readBinPositionPNG(string& binFile);
	double readBinScale(string& binFile);

	void trimMID(string& MID);
	void uptickBookmark(vector<vector<string>>& geoList);

	string urlCatalogue(string sYear, string sCata);
	string urlCSVDownload(string sYear, string sCata);
	string urlGeo(string urlCata);
	string urlYear(string syear);
};

