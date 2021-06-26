#pragma once
#include "qtfunc.h"
#include "statscan.h"

using namespace std;

struct BINMAP
{
	explicit BINMAP() {}
	~BINMAP() {}

	int defaultMargin = 20;
	JFUNC jf;
	vector<vector<int>> myBorder;
	vector<vector<vector<int>>> myFrames;
	int myGid;
	string myLayer, myName, myParent, myPath;
	vector<double> myPosition;
	double myScale;
	vector<vector<double>> myWindowBorder;
	vector<string> pathChildren;
	QTFUNC qf;
	STATSCAN sc;

	void borderScaleToWindow(vector<vector<int>>& border, vector<vector<double>>& borderD, vector<int> windowDim);
	void borderShiftFrames(vector<vector<int>>& border, vector<vector<vector<int>>>& frames);
	void borderShiftSetMargin(vector<vector<int>>& border, int margin);
	void childrenFromGeo(string geoPath);
	void drawFamily(vector<BINMAP>& binFamily, int selChild, QLabel*& qLabel);
	vector<string> getPathChildren();
	string getPathGeo();
	string getPathParent();
	void loadFromPath(string binPath);
	void makeWindowBorder()
};

