#pragma once

#include "gdifunc.h"
#include "imgfunc.h"
#include "iofunc.h"
#include "mathfunc.h"
#include "qtfunc.h"
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
	GDIFUNC gdi;
	IOFUNC io;
	IMGFUNC im;
	bool isgui;
	JFUNC jf;
	MATHFUNC mf;
	vector<vector<vector<unsigned char>>> originPatternH, originPatternV;
	vector<int> pngRes, fullRes, viScale;
	QPlainTextEdit* pte = nullptr;
	QTFUNC qf;
	vector<POINT> scaleLargeTLBR, scaleSmallTLBR;
	vector<vector<POINT>> scaleMST;
	vector<set<string>> setScaleMST;
	WINFUNC wf;

	POINT getOrigin(vector<unsigned char>& img, vector<int>& imgSpec, vector<POINT> TLBR);
	vector<POINT> getPixel(vector<unsigned char>& img, vector<int>& imgSpec, vector<unsigned char>& rgba);
	POINT getScaleCorner(vector<unsigned char>& img, vector<int>& imgSpec);
	void qshow(string sMessage);
	void recordButton(POINT& button, string buttonName);
	bool scanColourSquare(vector<POINT>& TLBR, vector<unsigned char> rgba);
	void setPTE(QPlainTextEdit*& qPTE, bool isGUI);
	vector<double> testScale(vector<unsigned char>& img, vector<int>& imgSpec, POINT pCorner);
};

class BINMAP : public MAP
{
	POINT bHome, bHomeExt, bPanel, bPlus, bMinus, mapScaleStart, searchBar, searchBarClear;
	vector<POINT> fOVCropped;  // Form [TL, BR, mid]

public:
	BINMAP() {}
	~BINMAP() {}

	void findFrames(vector<unsigned char>& img, vector<int>& imgSpec, POINT bHome);
	void init(QPlainTextEdit*& qPTE, bool isGUI);
	void recordButtonTLBR(vector<POINT>& button, string buttonName);
	void recordPoint(POINT& point, string pointName);
	void setPTE(QPlainTextEdit*& qPTE);
};

class PNGMAP : public MAP
{
	string activeCata;
	vector<POINT> vpMain, vpOverview, vpOVCropped, vpQuery;  // Form [TL, BR, mid]
	vector<string> geoLayers, folderPathChild, folderPathParent;
	unordered_map<string, int> mapGCRow;
	vector<vector<string>> nameChild, nameParent;
	set<string> setGCParent;

public:
	PNGMAP() {}
	~PNGMAP() {}

	int createAllParents();
	void findFrames(vector<unsigned char>& img, vector<int>& imgSpec, vector<POINT>& fOVCropped);
	void findFrames(vector<unsigned char>& img, vector<int>& imgSpec, POINT bHome, vector<POINT>& fMain, vector<POINT>& fOverview);
	int getScaleIndex();
	int initParentChild(vector<vector<string>>& geo);
	void initialize(vector<string>& GeoLayers);

};
