#pragma once

#include "gdifunc.h"
#include "imgfunc.h"
#include "iofunc.h"
#include "mathfunc.h"
#include "qtfunc.h"
#include "statscan.h"
#include "winfunc.h"

using namespace std;

class BINMAP
{
	vector<POINT> bHome, bHomeExt, bPanel, bPlus, bMinus;  // Form [TL, BR, mid]
	GDIFUNC gdi;
	vector<POINT> fMain, fOverview, fOVCropped;  // Form [TL, BR, mid]
	const int homeHeight = 45;  // Plus, Home, Minus buttons always share this height.
	IOFUNC io;
	IMGFUNC im;
	JFUNC jf;
	MATHFUNC mf;
	vector<vector<unsigned char>> originPatternH, originPatternV;
	vector<int> pngRes, fullRes;
	QPlainTextEdit* pte;
	QTFUNC qf;
	POINT searchBar;
	WINFUNC wf;

	vector<unsigned char> Canada, Navy, Usa, Water;

public:
	BINMAP() {}
	~BINMAP() {}

	void extrapolatePlusMinus();
	void findFrames(vector<unsigned char>& img, vector<int>& imgSpec, POINT bHome);
	POINT getOrigin(vector<unsigned char>& img, vector<int>& imgSpec);
	void init(QPlainTextEdit*& qPTE);
	void makeCanada(string& canadaPath);
	void recordButton(vector<POINT>& button, string buttonName);
	void recordPoint(POINT& point, string pointName);
	void setPTE(QPlainTextEdit*& qPTE);
};

