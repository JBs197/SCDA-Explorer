#pragma once

#include "gdifunc.h"
#include "imgfunc.h"
#include "iofunc.h"
#include "mathfunc.h"
#include "qtfunc.h"
#include "qtpaint.h"
#include "statscan.h"
#include "winfunc.h"

using namespace std;

class MAP
{
public:
	MAP() {}
	~MAP() {}

	POINT bHome, bHomeExt, bPanel, bPlus, bMinus, mapScaleStart, searchBar, searchBarClear;
	vector<unsigned char> Black, MSBlack, MSText, Navy, Red, White;
	vector<unsigned char> Canada, CanadaShaded, Usa, UsaShaded, Water, WaterShaded;
	vector<unsigned char> CanadaSel, City, CitySel, Green, Province, ProvinceSel, WaterSel;
	CANDIDATES cd;
	GDIFUNC gdi;
	IOFUNC io;
	IMGFUNC im;
	JFUNC jf;
	vector<double> mainPPKM;
	MATHFUNC mf;
	vector<vector<vector<unsigned char>>> originPatternH, originPatternV;
	vector<vector<double>> OVPPKM;  // Overview pixels per km. Form [scale index][xAxis, yAxis].
	vector<int> ovRes, pngRes, fullRes, viScale;
	QTFUNC qf;
	vector<POINT> scaleLargeTLBR, scaleSmallTLBR, vpOVCropped;
	vector<vector<POINT>> scaleMST;
	vector<set<string>> setScaleMST;
	WINFUNC wf;

	QPointF centerToImgTL(QPointF& qpfCenter, double imgPPKM);
	void cropToOverview(vector<unsigned char>& img, vector<int>& imgSpec);
	double extractScale(vector<unsigned char> img, vector<int> imgSpec);
	POINT getImgTL(vector<unsigned char>& img, vector<int>& imgSpec);
	POINT getOrigin(vector<unsigned char>& img, vector<int>& imgSpec, vector<POINT> TLBR);
	vector<POINT> getPixel(vector<unsigned char>& img, vector<int>& imgSpec, vector<unsigned char>& rgba);
	POINT getScaleCorner(vector<unsigned char>& img, vector<int>& imgSpec);
	POINT getScaleCorner(vector<unsigned char>& img, vector<int>& imgSpec, int& startCol);
	int getScaleIndex(double PPKM);
	void initScanCircles(string& pngPath);
	vector<POINT> makeBox(vector<unsigned char>& img, vector<int>& imgSpec, vector<unsigned char> rgba, int iMargin);
	vector<POINT> makeBoxSel(vector<unsigned char>& img, vector<int>& imgSpec, int iMargin);
	vector<double> ovPixelToKM(vector<int> ovDisp, int scaleIndex);
	bool scanColourSquare(vector<POINT>& TLBR, vector<unsigned char> rgba);
	vector<double> testScale(vector<unsigned char>& img, vector<int>& imgSpec, POINT pCorner);
};

class BINMAP : public MAP
{
	vector<double> greenBlueOutside, greenBlueInside, redBlueInside, redGreenInside;
	vector<unsigned char> imgPainted, imgBorder;
	vector<int> imgSpecPainted, imgSpecBorder;
	bool isgui;
	int maxRadius, mode;
	string paintedPath;
	QPlainTextEdit* pte = nullptr;
	vector<POINT> vpBorder, vpDeadEnd;

public:
	BINMAP() {}
	~BINMAP() {}

	void addBorderPoint(vector<POINT>& vpBorder, vector<POINT>& vpDeadEnd, POINT& pNew);
	void borderComplete(SWITCHBOARD& sbgui);
	void borderStart(vector<POINT>& tempTLBR);
	vector<vector<double>> borderTempToKM(vector<vector<double>>& TLBR, double imgPPKM);
	bool checkSpec(vector<int>& imgSpec);
	vector<POINT> drawRect(vector<unsigned char>& img, vector<int>& imgSpec, int iMargin);
	vector<POINT> drawRect(vector<unsigned char>& img, vector<int>& imgSpec, int iMargin, vector<vector<unsigned char>> rgbaList);
	void findFrames(vector<unsigned char>& img, vector<int>& imgSpec, POINT bHome);
	vector<vector<double>> getFrame(vector<double> centerKM, double imgPPKM, vector<POINT>& tempTLBR);
	vector<POINT> getFrame(vector<unsigned char>& img, vector<int>& imgSpec);
	vector<POINT> getFrame(vector<unsigned char>& img, vector<int>& imgSpec, vector<unsigned char> rgbx);
	QPointF imgTLToFrameTL(QPointF& qpfImgTL, POINT& imgTL, POINT& frameTL, double imgPPKM);
	void initialize();
	void initialize(int iMode);
	bool initPainted(string& cleanPath);
	vector<vector<double>> loadBorder(string& binFile);
	vector<vector<double>> loadFrame(string& binFile);
	vector<double> loadPosition(string& binFile);
	double loadScale(string& binFile);
	vector<POINT> paintRegionFrame();
	void printPaintedMap();
	void qshow(string sMessage);
	void recordPoint(POINT& point, string pointName);
	void setPTE(QPlainTextEdit*& qPTE, bool isGUI);
	void sprayRegion(vector<unsigned char>& img, vector<int>& imgSpec, vector<double> angleDeviation);
};

class OVERLAY : public MAP
{
	string botPath, superPath, topPath;
	POINT BR;
	vector<vector<unsigned char>> colours;
	int depth, length;  // length is rgb or rgba
	vector<int> minMaxTL, imgSpecBot, imgSpecSuper, imgSpecTop;
	vector<unsigned char> imgBot, imgSuper, imgTop;
	vector<vector<int>> substrate;

public:
	OVERLAY() {}
	~OVERLAY() {}

	POINT bestMatch(vector<vector<vector<int>>>& matchResult);
	int checkColour(vector<unsigned char>& rgbx);
	vector<int> extractImgDisplacement();
	int initBotTop();
	bool initialize(string& topPath, string& botPath);
	vector<vector<vector<int>>> initMatchResult();
	void loadSuperpos();
	void printSuperposition(POINT& TL);
	void reportSuperposition(SWITCHBOARD& sbgui, vector<vector<int>>& match, OVERLAY ov);
};

class PNGMAP : public MAP
{
	string activeCata;
	vector<POINT> vpMain, vpOverview, vpQuery;  // Form [TL, BR, mid]
	vector<string> geoLayers, folderPathChild, folderPathParent;
	bool isgui;
	unordered_map<string, int> mapGCRow;
	vector<vector<string>> nameChild, nameParent;
	QPlainTextEdit* pte = nullptr;
	set<string> setGCParent;

public:
	PNGMAP() {}
	~PNGMAP() {}

	void createAllChildren(QProgressBar*& qpb, int& progress);
	void createAllParents(QProgressBar*& qpb, int& progress);
	void findFrames(vector<unsigned char>& img, vector<int>& imgSpec, vector<POINT>& fOVCropped);
	void findFrames(vector<unsigned char>& img, vector<int>& imgSpec, POINT bHome, vector<POINT>& fMain, vector<POINT>& fOverview);
	int getParentScaleIndex(string& searchText);
	int getNumChildren();
	int getScaleIndex();
	int initParentChild(vector<vector<string>>& geo);
	void initialize(vector<string>& GeoLayers);
	void qshow(string sMessage);
	void recordButton(POINT& button, string buttonName);
	void setPTE(QPlainTextEdit*& qPTE, bool isGUI);
};

