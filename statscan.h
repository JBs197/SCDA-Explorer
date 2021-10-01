#pragma once

#include <unordered_map>
#include "switchboard.h"
#include "winfunc.h"
#include "zipfunc.h"

using namespace std;

class STATSCAN
{
	string activeCSVgeocode, brokenLine;
	vector<int> activeColIndex, scoopIndex;
	int activeCSVpart, geoCodeCol = -1, geoLevelCol = -1, geoNameCol = -1, nls;
	vector<string> cataColTitles, geoHistory;
	string cataName, cataPath, cataYear, description, metaFile, topic;
	vector<vector<string>> geo;
	bool ignoreSplitRegions = 1;
	JFUNC jf;
	string nl;
	unordered_map<string, string> mapDataIndex;  // DIMparam->DataIndex
	unordered_map<string, string> mapGeoLayer;  // Listed layer name->internal name
	unordered_map<string, int> mapGeoPart;  // Form GEO_CODE->PART
	unordered_map<int, int> mapDIM;  // Form indexDIM->cataColTitles[index]
	unordered_map<int, int> mapDim;  // Form indexDim->cataColTitles[index]
	vector<string> vsBuffer;
	WINFUNC wf;
	ZIPFUNC zf;

public:
	STATSCAN() {}
	~STATSCAN() {}
	unordered_map<string, int> mapGeoCode;  // Form GEO_CODE->geoListRow

	vector<string> compareGeoListDB(vector<int>& viGeoCode, vector<string>& dbList);
	void convertSCgeo(string& geoPathOld);
	string extractCSVLineValue(string& csvLine, int colIndex);
	vector<string> extractCSVLineValue(string& csvLine, vector<int> colIndex);

	string getCSVPath(int PART);
	int getGeoCodeIndex(string& sActiveCSVgeocode);
	string getGeoLayer(string geoLayerExternal);
	vector<vector<string>> getMIDAncestry(vector<string>& nameList);
	int getPart(string GEO_CODE);
	void getVSBuffer(vector<string>& vsBuffer);

	void init(string cataPath);
	void initCSV(string activeGeoCode);
	void initCSV(string activeGeoCode, int activePART);
	void initGeo();

	int loadBinGeo(string& filePath, vector<int>& gidList, vector<string>& regionList, vector<string>& layerList, vector<string>& geoLayers);
	void loadBookmark(int& iActiveCSVpart, string& sActiveCSVgeocode);
	vector<string> loadColTitles();
	vector<vector<string>> loadGeoList(string geoPath);
	void loadGeoList(string geoPath, vector<int>& viGeoCode, vector<string>& vsRegion, vector<int>& viBegin, vector<int>& viEnd);

	string makeBinBorder(vector<vector<int>>& border);
	string makeBinFrames(vector<vector<vector<int>>>& frames);
	string makeBinParent(string parent);
	string makeBinParentNew(string binPath, string& sParent);
	string makeBinPositionGPS(vector<double> position);
	string makeBinPositionPNG(vector<double> position);
	string makeBinScale(double scale);
	void makeBookmark(int iActiveCSVpart, string sActiveCSVgeocode);

	string makeCreateCensus();
	int makeCreateData(vector<int>& viGeoCode);
	string makeCreateDataIndex();
	string makeCreateForWhom();
	string makeCreateGeo();
	string makeCreateGeoLayers(string sYear);
	vector<string> makeCreateInsertDIMIndex();
	vector<string> makeCreateInsertDIM(vector<vector<string>>& vvsDIM);
	vector<string> makeCreateInsertDim();
	vector<string> makeCreateInsertTopic(vector<string>& colTitles);
	string makeCreateMap(string tname);
	string makeCreateMapFrame(string tname);
	string makeCreateTopic(string tname);
	string makeCreateYear();

	string makeInsertCensus();
	void makeInsertData(string& csvFile, size_t& nextLine, vector<string>& vsStmt);
	int makeInsertDataIndex(int numDI, vector<vector<string>>& vvsDIM);
	string makeInsertForWhom();
	string makeInsertGeo(string& csvFile, size_t& nextLine);
	vector<string> makeInsertMap(string tname, string mapPath);
	vector<string> makeInsertMapFrame(string tname, string mapPath);
	string makeInsertTopic(string tname, vector<string>& vsTopic);
	string makeInsertYear();

	vector<vector<string>> parseNavSearch(string& navSearchBlob);

	vector<vector<int>> readBinBorder(string& binFile);
	vector<vector<vector<int>>> readBinFrames(string& binFile);
	string readBinParent(string& binFile);
	vector<double> readBinPositionGPS(string& binFile);
	vector<double> readBinPositionPNG(string& binFile);
	double readBinScale(string& binFile);

	void removeFootnote(string& sLine);
	void setMapDataIndex(unordered_map<string, string>& mDI);
	void trimMID(string& MID);
	void uptickBookmark(vector<vector<string>>& geoList);

	string urlCatalogue(string sYear, string sCata);
	string urlCSVDownload(string sYear, string sCata);
	string urlGeo(string urlCata);
	string urlYear(string syear);
};

