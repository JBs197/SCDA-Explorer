#pragma once

#include "qtfunc.h"
#include "statscan.h"

using namespace std;

class BINMAP
{

public:
	BINMAP() {}
	~BINMAP() {}
	
	double defaultMargin = 20.0;
	bool isSelected = 0;
	JFUNC jf;
	vector<vector<int>> myBorder;
	vector<vector<vector<int>>> myFrames;
	int myGid;
	string myLayer, myName, myParent, myPath;
	vector<double> myPosition, blueDot;
	double myScale;
	vector<vector<double>> myWindowBorder;
	vector<string> pathChildren;
	QTFUNC qf;
	STATSCAN sc;

	void borderScaleToWindow(vector<vector<int>>& border, vector<vector<double>>& borderD, vector<double> borderDim);
	void borderShiftSetMargin(vector<vector<int>>& border, int margin);
	void childrenFromGeo(string geoPath);
	vector<string> getPathChildren(int mode);
	string getPathGeo();
	string getPathParent();
	vector<vector<double>> getTLBR(vector<vector<double>>& borderPath);
	void loadFromPath(string binPath);
	void makeBlueDot();
	void makeWindowBorder(double width, double height);
};

