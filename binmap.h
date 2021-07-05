#pragma once

#include "gdifunc.h"
#include "imgfunc.h"
#include "iofunc.h"
#include "qtfunc.h"
#include "statscan.h"

using namespace std;

class BINMAP
{
	vector<POINT> bHome, bHomeExt, bPanel, fMain, fOverview;  // Form [TL, BR, mid]
	GDIFUNC gdi;
	IOFUNC io;
	IMGFUNC im;
	JFUNC jf;
	QPlainTextEdit* pte;
	QTFUNC qf;

	vector<unsigned char> Canada = { 240, 240, 240, 255 };
	vector<unsigned char> Usa = { 215, 215, 215, 255 };
	vector<unsigned char> Water = { 179, 217, 247, 255 };

public:
	BINMAP() {}
	~BINMAP() {}
	
	void findFrames(POINT bHome);
	void recordButton(vector<POINT>& button, string buttonName);
	void setPTE(QPlainTextEdit*& qPTE);
};

