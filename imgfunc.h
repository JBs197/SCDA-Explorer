#pragma once
#include "stb_image.h"
#include "stb_image_write.h"
#include <assert.h>
#include <string>
#include <vector>
#include "jfunc.h"
#include "switchboard.h"

using namespace std;

class IMGFUNC
{
	vector<vector<int>> borderRegion;  // Sequence of coords that represent the region's border.
	int borderThickness = 6;
    double candidateDistanceTolerance = 0.4;  // Final two candidates: colour count or distance! 
	vector<unsigned char> dataPNG, debugDataPNG;
	bool debug = 0;
    string pathActivePNG;
	JFUNC jf;
	unordered_map<string, string> mapColour;
	vector<unsigned char> Red = { 255, 0, 0 };
	vector<int> pointOfOrigin;
	int width, height, numComponents, recordVictor;
    double stretchFactor;

public:
	explicit IMGFUNC() {}
	~IMGFUNC() {}
	vector<int> borderFindNext(vector<vector<int>> tracks);
	vector<int> borderFindStart();
	vector<double> coordDist(vector<int> zeroZero, vector<vector<int>> coordList);
	vector<int> coordMid(vector<vector<int>>& vCoords);
    vector<vector<int>> coordPath(vector<vector<int>> startStop);
    int coordRGB(vector<vector<int>> startStop, string szone);
    vector<int> coordStoi(string& sCoords);
	void drawMarker(vector<unsigned char>& img, vector<int>& vCoord);
    vector<vector<double>> frameCorners();
	int getOffset(vector<int>& vCoord);
	void initMapColours();
	bool isInit();
    bool jobsDone(vector<int> vCoord);
	void linePaint(vector<unsigned char>& img, vector<vector<int>>& vVictor, int length, vector<unsigned char>& colour);
	vector<vector<int>> linePath(vector<vector<int>>& vVictor, int length);
	vector<vector<unsigned char>> lineRGB(vector<vector<int>>& vVictor, int length);
	bool mapIsInit();
	void octogonPaint(vector<unsigned char>& img, vector<int>& origin, int radius, vector<unsigned char>& colour);
	vector<vector<int>> octogonPath(vector<int>& origin, int radius);
	string pixelDecToHex(vector<unsigned char>& rgb);
	void pixelPaint(vector<unsigned char>& img, vector<unsigned char>& colour, vector<int>& vCoord);
	vector<unsigned char> pixelRGB(int x, int y);
	string pixelZone(vector<unsigned char>& rgb);
	void pngLoad(string& pathPNG);
    void pngPrint();
    void pngToBinLive(SWITCHBOARD& sbgui, vector<vector<double>>& border);
	vector<vector<int>> zoneChangeLinear(vector<string>& szones, vector<vector<int>>& ivec);
	vector<vector<int>> zoneSweep(string szone, vector<vector<unsigned char>>& Lrgb, vector<vector<int>>& zonePath);


	// TEMPLATES

	template<typename ... Args> vector<vector<unsigned char>> octogonRGB(Args& ... args) 
    {
        // Returns a list of RGB values for the perimeter of a given octogon.
        // List starts at 12 o'clock and progresses clockwise.
        // Radius is measured along a vertical or horizontal line. 
    }
	template<> vector<vector<unsigned char>> octogonRGB<vector<int>, int>(vector<int>& origin, int& radius)
	{
        int lenPerp, lenDiag;
        if (radius % 2 == 0)  // Even.
        {
            lenPerp = radius;
            lenDiag = radius / 2;
        }
        else if (radius % 2 == 1)  // Odd.
        {
            lenPerp = radius - 1;
            lenDiag = (radius / 2) + 1;
        }
        vector<vector<unsigned char>> octoRGB(1, vector<unsigned char>(3));
        octoRGB[0] = pixelRGB(origin[0], origin[1] - radius);
        vector<vector<int>> vVictor = { {origin[0], origin[1] - radius}, {1, 0} };
        vector<vector<unsigned char>> vvTemp = lineRGB(vVictor, radius / 2);
        octoRGB.insert(octoRGB.end(), vvTemp.begin(), vvTemp.end());

        vVictor[1] = { 1,1 };
        vvTemp = lineRGB(vVictor, lenDiag);
        octoRGB.insert(octoRGB.end(), vvTemp.begin(), vvTemp.end());
        vVictor[1] = { 0,1 };
        vvTemp = lineRGB(vVictor, lenPerp);
        octoRGB.insert(octoRGB.end(), vvTemp.begin(), vvTemp.end());
        vVictor[1] = { -1,1 };
        vvTemp = lineRGB(vVictor, lenDiag);
        octoRGB.insert(octoRGB.end(), vvTemp.begin(), vvTemp.end());
        vVictor[1] = { -1,0 };
        vvTemp = lineRGB(vVictor, lenPerp);
        octoRGB.insert(octoRGB.end(), vvTemp.begin(), vvTemp.end());
        vVictor[1] = { -1,-1 };
        vvTemp = lineRGB(vVictor, lenDiag);
        octoRGB.insert(octoRGB.end(), vvTemp.begin(), vvTemp.end());
        vVictor[1] = { 0,-1 };
        vvTemp = lineRGB(vVictor, lenPerp);
        octoRGB.insert(octoRGB.end(), vvTemp.begin(), vvTemp.end());
        vVictor[1] = { 1,-1 };
        vvTemp = lineRGB(vVictor, lenDiag);
        octoRGB.insert(octoRGB.end(), vvTemp.begin(), vvTemp.end());
        vVictor[1] = { 1,0 };
        vvTemp = lineRGB(vVictor, radius / 2);
        octoRGB.insert(octoRGB.end(), vvTemp.begin(), vvTemp.end());

        if (octoRGB[0] != octoRGB[octoRGB.size() - 1])
        {
            jf.err("first-last RGB mismatch-im.scanOctogon");
        }
        return octoRGB;
	}
    template<> vector<vector<unsigned char>> octogonRGB<vector<vector<int>>>(vector<vector<int>>& octoPath)
    {
        vector<vector<unsigned char>> octoRGB(octoPath.size(), vector<unsigned char>(3));
        for (int ii = 0; ii < octoRGB.size(); ii++)
        {
            octoRGB[ii] = pixelRGB(octoPath[ii][0], octoPath[ii][1]);
        }
        return octoRGB;
    }

    template<typename ... Args> void coordShift(Args& ... args)
    {
        jf.err("coordShift template-im");
    }
    template<> void coordShift<vector<vector<double>>, vector<double>>(vector<vector<double>>& listCoord, vector<double>& mapShift)
    {
        double coordX, coordY;
        for (int ii = 0; ii < listCoord.size(); ii++)
        {
            coordX = listCoord[ii][0];
            coordY = listCoord[ii][1];
            listCoord[ii][0] = (coordX + mapShift[0]) * mapShift[2];
            listCoord[ii][1] = (coordY + mapShift[1]) * mapShift[2];
        }
    }
    template<> void coordShift<vector<vector<int>>, vector<double>, vector<vector<double>>>(vector<vector<int>>& listCoord, vector<double>& mapShift, vector<vector<double>>& listShifted)
    {
        listShifted.resize(listCoord.size(), vector<double>(2));
        for (int ii = 0; ii < listCoord.size(); ii++)
        {
            listShifted[ii][0] = ((double)listCoord[ii][0] + mapShift[0]) * mapShift[2];
            listShifted[ii][1] = ((double)listCoord[ii][1] + mapShift[1]) * mapShift[2];
        }
    }


};

