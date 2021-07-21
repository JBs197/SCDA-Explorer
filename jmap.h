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
	MATHFUNC mf;
	vector<vector<vector<unsigned char>>> originPatternH, originPatternV;
	const double xOverviewPPKM = 0.055;
	const double yOverviewPPKM = 0.055;
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
	void initScanCircles(string& pngPath);
	vector<POINT> makeBox(vector<unsigned char>& img, vector<int>& imgSpec, vector<unsigned char> rgba, int iMargin);
	vector<POINT> makeBoxSel(vector<unsigned char>& img, vector<int>& imgSpec, int iMargin);
	QPointF ovPixelToKM(vector<int> ovDisp);
	bool scanColourSquare(vector<POINT>& TLBR, vector<unsigned char> rgba);
	vector<double> testScale(vector<unsigned char>& img, vector<int>& imgSpec, POINT pCorner);
};

class BINMAP : public MAP
{
	vector<double> greenBlueOutside, greenBlueInside, redBlueInside, redGreenInside;
	bool isgui;
	int maxRadius, mode;
	QPlainTextEdit* pte = nullptr;

public:
	BINMAP() {}
	~BINMAP() {}

	void addBorderPoint(vector<POINT>& vpBorder, vector<POINT>& vpDeadEnd, POINT& pNew);
	void borderComplete(SWITCHBOARD& sbgui, vector<POINT>& vpBorder);
	POINT borderPreStart(vector<unsigned char>& img, vector<int>& imgSpec);
	void borderStart(vector<unsigned char>& img, vector<int>& imgSpec, vector<POINT>& vpBorder);
	bool checkSpec(vector<int>& imgSpec);
	vector<POINT> drawRect(vector<unsigned char>& img, vector<int>& imgSpec, int iMargin);
	vector<POINT> drawRect(vector<unsigned char>& img, vector<int>& imgSpec, int iMargin, vector<vector<unsigned char>> rgbaList);
	vector<int> extractImgDisplacement(vector<unsigned char>& imgSuper, vector<int>& imgSpecSuper, vector<unsigned char>& imgHome, vector<int>& imgSpecHome);
	void findFrames(vector<unsigned char>& img, vector<int>& imgSpec, POINT bHome);
	vector<POINT> getFrame(vector<unsigned char>& img, vector<int>& imgSpec);
	vector<POINT> getFrame(vector<unsigned char>& img, vector<int>& imgSpec, vector<unsigned char> rgbx);
	QPointF imgTLToFrameTL(QPointF& qpfImgTL, POINT& imgTL, POINT& frameTL, double imgPPKM);
	void initialize();
	void initialize(int iMode);
	vector<POINT> loadBorder(string& binFile);
	vector<POINT> loadFrame(string& binFile);
	QPointF loadPosition(string& binFile);
	double loadScale(string& binFile);
	void qshow(string sMessage);
	void recordPoint(POINT& point, string pointName);
	void setPTE(QPlainTextEdit*& qPTE, bool isGUI);
	void sprayRegion(vector<unsigned char>& img, vector<int>& imgSpec, vector<POINT> TLBR, vector<double> angleDeviation);
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

class OVERLAY : public MAP
{
	vector<vector<unsigned char>> colours;

public:
	OVERLAY() {}
	~OVERLAY() {}

	POINT BR;
	int depth, length;  // length is rgb or rgba
	vector<unsigned char> pngBot, pngTop;
	vector<int> pngSpecBot, pngSpecTop;
	vector<vector<int>> substrate;

	int checkColour(vector<unsigned char>& rgb);
	void createSuperposition(vector<unsigned char>& imgTop, vector<int>& imgSpecTop, vector<unsigned char>& imgSuper, vector<int>& imgSpecSuper);
	void initPNG(vector<unsigned char>& imgBot, vector<int>& imgSpecBot);
	vector<vector<int>> matchAll(vector<unsigned char>& img, vector<int>& imgSpec);
	void printSuperposition(string& pngPath, vector<int> TL);
	void reportSuperposition(SWITCHBOARD& sbgui, vector<vector<int>>& match, OVERLAY ov);
	vector<int> setTopPNG(vector<unsigned char>& img, vector<int>& imgSpec);
};
