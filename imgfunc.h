#pragma once
#include "stb_image.h"
#include <string>
#include <vector>
#include "jfunc.h"

using namespace std;

class IMGFUNC
{
	vector<vector<int>> borderRegion;  // Sequence of coords that represent the region's border.
	vector<unsigned char> dataPNG;
	JFUNC jf;
	unordered_map<string, string> mapColour;
	vector<vector<int>> vvStar;
	int width, height, numComponents, recordVictor;

public:
	explicit IMGFUNC() {}
	~IMGFUNC() {}
	vector<int> borderFindNext(vector<vector<int>> tracks);
	vector<int> borderFindStart();
	vector<double> coordDist(vector<int> zeroZero, vector<vector<int>> coordList);
	vector<int> coordMid(vector<vector<int>>& vCoords);
	vector<int> coordStoi(string& sCoords);
	void initMapColours();
	bool isInit();
	string pixelDecToHex(vector<unsigned char>& rgb);
	vector<unsigned char> pixelRGB(int x, int y);
	string pixelZone(vector<unsigned char>& rgb);
	void pngLoad(string& pathPNG);
	bool mapIsInit();
	vector<vector<int>> zoneChangeLinear(vector<string>& szones, vector<vector<int>>& ivec);
	vector<vector<int>> zoneStar(vector<vector<int>>& tracks);
};

