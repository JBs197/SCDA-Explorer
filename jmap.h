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
	vector<unsigned char> CanadaSel, Green, WaterSel;
	GDIFUNC gdi;
	IOFUNC io;
	IMGFUNC im;
	JFUNC jf;
	MATHFUNC mf;
	vector<vector<vector<unsigned char>>> originPatternH, originPatternV;
	vector<int> ovRes, pngRes, fullRes, viScale;
	QTFUNC qf;
	vector<POINT> scaleLargeTLBR, scaleSmallTLBR, vpOVCropped;
	vector<vector<POINT>> scaleMST;
	vector<set<string>> setScaleMST;
	WINFUNC wf;

	void cropToOverview(vector<unsigned char>& img, vector<int>& imgSpec);
	double extractScale(vector<unsigned char> img, vector<int> imgSpec);
	POINT getOrigin(vector<unsigned char>& img, vector<int>& imgSpec, vector<POINT> TLBR);
	vector<POINT> getPixel(vector<unsigned char>& img, vector<int>& imgSpec, vector<unsigned char>& rgba);
	POINT getScaleCorner(vector<unsigned char>& img, vector<int>& imgSpec);
	POINT getScaleCorner(vector<unsigned char>& img, vector<int>& imgSpec, int& startCol);
	bool scanColourSquare(vector<POINT>& TLBR, vector<unsigned char> rgba);
	vector<double> testScale(vector<unsigned char>& img, vector<int>& imgSpec, POINT pCorner);
};

class BINMAP : public MAP
{
	bool isgui;
	QPlainTextEdit* pte = nullptr;

public:
	BINMAP() {}
	~BINMAP() {}

	bool checkSpec(vector<int>& imgSpec);
	void drawRect(vector<unsigned char>& img, vector<int>& imgSpec, vector<vector<unsigned char>> rgba, int iMargin);
	vector<int> extractPosition(vector<unsigned char>& imgSuper, vector<int>& imgSpecSuper, vector<unsigned char>& imgHome, vector<int>& imgSpecHome);
	void findFrames(vector<unsigned char>& img, vector<int>& imgSpec, POINT bHome);
	void initialize();
	void qshow(string sMessage);
	void recordPoint(POINT& point, string pointName);
	void setPTE(QPlainTextEdit*& qPTE, bool isGUI);
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

	void createAllChildren(SWITCHBOARD& sbgui, vector<int> mycomm);
	int createAllParents();
	void findFrames(vector<unsigned char>& img, vector<int>& imgSpec, vector<POINT>& fOVCropped);
	void findFrames(vector<unsigned char>& img, vector<int>& imgSpec, POINT bHome, vector<POINT>& fMain, vector<POINT>& fOverview);
	int getParentScaleIndex(string& searchText);
	int getNumChildren();
	int getScaleIndex();
	void initPaint();
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
