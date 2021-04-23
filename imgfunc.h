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
    vector<vector<int>> coordShift(vector<vector<int>>& coordList, vector<int> DxDy);
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

    template<typename ... Args> void coordScale(vector<vector<int>>& coordList, double scaleFactor, Args& ... args)
    {

    }
    template<> void coordScale<vector<vector<int>>>(vector<vector<int>>& coordList, double scaleFactor, vector<vector<int>>& coordListScaled)
    {
        coordListScaled.resize(coordList.size(), vector<int>(2));
        double dtemp0, dtemp1;
        for (int ii = 0; ii < coordList.size(); ii++)
        {
            dtemp0 = (double)coordList[ii][0] * scaleFactor;
            dtemp0 = round(dtemp0);
            coordListScaled[ii][0] = int(dtemp0);
            dtemp1 = (double)coordList[ii][1] * scaleFactor;
            dtemp1 = round(dtemp1);
            coordListScaled[ii][1] = int(dtemp1);
        }
    }
    template<> void coordScale<vector<vector<double>>>(vector<vector<int>>& coordList, double scaleFactor, vector<vector<double>>& coordListScaled)
    {
        coordListScaled.resize(coordList.size(), vector<double>(2));
        double dtemp0, dtemp1;
        for (int ii = 0; ii < coordList.size(); ii++)
        {
            dtemp0 = (double)coordList[ii][0] * scaleFactor;
            coordListScaled[ii][0] = round(dtemp0);
            dtemp1 = (double)coordList[ii][1] * scaleFactor;
            coordListScaled[ii][1] = round(dtemp1);
        }
    }

    template<typename ... Args> void pngToBin(SWITCHBOARD& sbgui, Args& ... args)
    {

    }
    template<> void pngToBin< >(SWITCHBOARD& sbgui)
    {
        vector<int> mycomm;
        vector<vector<int>> comm_gui;
        thread::id myid = this_thread::get_id();
        sbgui.answer_call(myid, mycomm);
        vector<string> prompt = sbgui.get_prompt();

        if (!mapIsInit()) { initMapColours(); }
        pngLoad(prompt[0]);
        vector<vector<int>> vBorderPath(1, vector<int>());
        vBorderPath[0] = borderFindStart();
        vector<vector<int>> tracks;
        while (1)
        {
            if (vBorderPath.size() > 3)
            {
                tracks[0] = tracks[1];
                tracks[1] = tracks[2];
                tracks[2] = vBorderPath[vBorderPath.size() - 1];
            }
            else
            {
                tracks = vBorderPath;
            }
            vBorderPath.push_back(borderFindNext(tracks));
            if (vBorderPath.size() > 10)
            {
                if (jobsDone(vBorderPath[vBorderPath.size() - 1])) { break; }
            }
        }
        vector<vector<double>> corners = frameCorners();

        ofstream sPrinter(prompt[1].c_str(), ios::trunc);
        auto report = sPrinter.rdstate();
        sPrinter << "//frame" << endl;
        for (int ii = 0; ii < corners.size(); ii++)
        {
            sPrinter << to_string(corners[ii][0]) << "," << to_string(corners[ii][1]) << endl;
        }
        sPrinter << endl;

        sPrinter << "//border" << endl;
        for (int ii = 0; ii < vBorderPath.size(); ii++)
        {
            sPrinter << to_string(vBorderPath[ii][0]) << "," << to_string(vBorderPath[ii][1]) << endl;
        }

    }
    template<> void pngToBin<vector<vector<int>>>(SWITCHBOARD& sbgui, vector<vector<int>>& border)
    {
        vector<int> mycomm;
        vector<vector<int>> comm_gui;
        thread::id myid = this_thread::get_id();
        sbgui.answer_call(myid, mycomm);
        vector<string> prompt = sbgui.get_prompt();

        if (!mapIsInit()) { initMapColours(); }
        pngLoad(prompt[0]);
        vector<vector<int>> vBorderPath(1, vector<int>());
        vBorderPath[0] = borderFindStart();
        vector<vector<int>> tracks;
        while (1)
        {
            if (vBorderPath.size() > 3)
            {
                tracks[0] = tracks[1];
                tracks[1] = tracks[2];
                tracks[2] = vBorderPath[vBorderPath.size() - 1];
            }
            else
            {
                tracks = vBorderPath;
            }
            vBorderPath.push_back(borderFindNext(tracks));
            if (vBorderPath.size() > 10)
            {
                if (jobsDone(vBorderPath[vBorderPath.size() - 1])) { break; }
            }
        }
        vector<vector<double>> corners = frameCorners();

        ofstream sPrinter(prompt[1].c_str(), ios::trunc);
        auto report = sPrinter.rdstate();
        sPrinter << "//frame" << endl;
        for (int ii = 0; ii < corners.size(); ii++)
        {
            sPrinter << to_string(corners[ii][0]) << "," << to_string(corners[ii][1]) << endl;
        }
        sPrinter << endl;

        sPrinter << "//border" << endl;
        for (int ii = 0; ii < vBorderPath.size(); ii++)
        {
            sPrinter << to_string(vBorderPath[ii][0]) << "," << to_string(vBorderPath[ii][1]) << endl;
        }

    }

};

