#pragma once
#include "stb_image.h"
#include "stb_image_write.h"
#include <assert.h>
#include <string>
#include <vector>
#include "jfunc.h"
#include "switchboard.h"

using namespace std;
extern const string sroot;

class IMGFUNC
{
	vector<vector<int>> borderRegion;  // Sequence of coords that represent the region's border.
	int borderThickness = 6;
    double candidateDistanceTolerance = 0.4;  // Final two candidates: colour count or distance! 
	vector<unsigned char> dataPNG, debugDataPNG;
	bool debug = 0;
    int defaultDotWidth = 5;
    vector<int> defaultExtractDim = { 400, 400 };
    string pathActivePNG;
	JFUNC jf;
	unordered_map<string, string> mapColour;
	vector<int> pointOfOrigin, revisedExtractDim;
	int width, height, numComponents, recordVictor;
    double stretchFactor;

    vector<unsigned char> Blue = { 0, 0, 255 };
    vector<unsigned char> Orange = { 255, 155, 55 };
    vector<unsigned char> Pink = { 255, 0, 255 };
    vector<unsigned char> Purple = { 160, 50, 255 };
    vector<unsigned char> Red = { 255, 0, 0 };
    vector<unsigned char> Teal = { 0, 155, 255 };

public:
	explicit IMGFUNC() {}
	~IMGFUNC() {}
	vector<int> borderFindNext(SWITCHBOARD& sbgui, vector<vector<int>> tracks);
	vector<int> borderFindStart();
    vector<vector<int>> checkBoundary(vector<int>& center, vector<int>& sourceDim, vector<int>& extractDim);
    double clockwisePercentage(SWITCHBOARD& sbgui, vector<vector<int>>& tracks, string sZone);
	vector<double> coordDist(vector<int> zeroZero, vector<vector<int>> coordList);
	vector<int> coordMid(vector<vector<int>>& vCoords);
    vector<vector<int>> coordPath(vector<vector<int>> startStop);
    int coordRGB(vector<vector<int>> startStop, string szone);
    vector<int> coordStoi(string& sCoords);
	void drawMarker(vector<unsigned char>& img, vector<int>& vCoord);
    vector<vector<double>> frameCorners();
    double getStretchFactor(string& widthHeight);
	void initMapColours();
	bool isInit();
    bool jobsDone(vector<int> vCoord);
	vector<vector<int>> linePath(vector<vector<int>>& startStop);
	vector<vector<unsigned char>> lineRGB(vector<vector<int>>& vVictor, int length);
	bool mapIsInit();
    vector<double> octogonBearing(SWITCHBOARD& sbgui, vector<vector<int>>& tracks, string sZone, int radius);
    vector<vector<int>> octogonPath(vector<int> origin, int radius);
    string pixelDecToHex(vector<unsigned char>& rgb);
	void pixelPaint(vector<unsigned char>& img, int widthImg, vector<unsigned char> rgb, vector<int> coord);
	vector<unsigned char> pixelRGB(int x, int y);
	string pixelZone(vector<unsigned char>& rgb);
	void pngLoad(string& pathPNG);
    void pngPrint();
    void pngToBinLive(SWITCHBOARD& sbgui, vector<vector<double>>& border);
    void pngToBinLiveDebug(SWITCHBOARD& sbgui, vector<vector<double>>& border);
    vector<vector<int>> zoneChangeLinear(vector<string>& szones, vector<vector<int>>& ivec);
	vector<vector<int>> zoneSweep(string szone, vector<vector<unsigned char>>& Lrgb, vector<vector<int>>& zonePath);
    void zoneSweepDebug(vector<vector<int>>& vCoord, int radius);

	// TEMPLATES

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

    template<typename ... Args> void dotPaint(vector<int> coord, Args& ... args)
    {
        jf.err("drawDot template-im");
    }
    template<> void dotPaint< >(vector<int> coord)
    {
        // Default values are 5x5 red dots on the dataPNG buffer.
        vector<int> vTemp(2);
        for (int ii = 0; ii < defaultDotWidth; ii++)
        {
            vTemp[1] = coord[1] - (defaultDotWidth / 2) + ii;
            for (int jj = 0; jj < defaultDotWidth; jj++)
            {
                vTemp[0] = coord[0] - (defaultDotWidth / 2) + jj;
                pixelPaint(dataPNG, width, Red, vTemp);
            }
        }
    }
    template<> void dotPaint<vector<unsigned char>, int, vector<unsigned char>, int>(vector<int> coord, vector<unsigned char>& img, int& widthImg, vector<unsigned char>& rgb, int& widthDot)
    {
        vector<int> vTemp(2);
        for (int ii = 0; ii < widthDot; ii++)
        {
            vTemp[1] = coord[1] - (widthDot / 2) + ii;
            for (int jj = 0; jj < widthDot; jj++)
            {
                vTemp[0] = coord[0] - (widthDot / 2) + jj;
                pixelPaint(img, widthImg, rgb, vTemp);
            }
        }
    }

    template<typename ... Args> int getOffset(vector<int>& vCoord, Args& ... args)
    {
        jf.err("getOffset template");
    }
    template<> int getOffset< >(vector<int>& vCoord)
    {
        int offRow = vCoord[1] * width * numComponents;
        int offCol = vCoord[0] * numComponents;
        return offRow + offCol;
    }
    template<> int getOffset<int>(vector<int>& vCoord, int& widthRect)
    {
        int offRow = vCoord[1] * widthRect * numComponents;
        int offCol = vCoord[0] * numComponents;
        return offRow + offCol;
    }

    template<typename ... Args> void linePaint(vector<vector<int>> startStop, Args& ... args)
    {
        jf.err("linePaint template-im");
    }
    template<> void linePaint< >(vector<vector<int>> startStop)
    {
        vector<vector<int>> path = linePath(startStop);
        for (int ii = 0; ii < path.size(); ii++)
        {
            dotPaint(path[ii]);
        }
    }
    template<> void linePaint<vector<unsigned char>, int, vector<unsigned char>, int>(vector<vector<int>> startStop, vector<unsigned char>& img, int& widthImg, vector<unsigned char>& rgb, int& widthDot)
    {
        vector<vector<int>> path = linePath(startStop);
        for (int ii = 0; ii < path.size(); ii++)
        {
            dotPaint(path[ii], img, widthImg, rgb, widthDot);
        }
    }

    template<typename ... Args> void octogonPaint(vector<int> origin, int radius, Args& ... args)
    {
        jf.err("octogonPaint template-im");
    }
    template<> void octogonPaint< >(vector<int> origin, int radius)
    {
        vector<vector<int>> path = octogonPath(origin, radius);
        for (int ii = 0; ii < path.size(); ii++)
        {
            dotPaint(path[ii]);
        }
    }
    template<> void octogonPaint<vector<unsigned char>, int, vector<unsigned char>, int>(vector<int> origin, int radius, vector<unsigned char>& img, int& widthImg, vector<unsigned char>& rgb, int& widthDot)
    {
        vector<vector<int>> path = octogonPath(origin, radius);
        for (int ii = 0; ii < path.size(); ii++)
        {
            dotPaint(path[ii], img, widthImg, rgb, widthDot);
        }
    }


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

    template<typename ... Args> vector<unsigned char> pngExtractRect(vector<int>& center, Args& ... args)
    {
        jf.err("pngExtractRect template");
    }
    template<> vector<unsigned char> pngExtractRect< >(vector<int>& center)
    {
        // Dimension vectors have form [width, height]. Also, assume 3 components (RGB).
        vector<unsigned char> pngExtract;
        vector<int> sourceDim = { width, height };
        vector<int> extractDim = defaultExtractDim;
        int bytesPerRow, offsetStart, offset;
        vector<vector<int>> corners = checkBoundary(center, sourceDim, extractDim);
        center = corners[0];
        if (corners.size() > 1)
        {
            extractDim[0] = corners[1][0] - corners[0][0];
            extractDim[1] = corners[1][1] - corners[0][1];
        }
        bytesPerRow = extractDim[0] * numComponents;
        offsetStart = getOffset(corners[0], sourceDim[0]);
        for (int ii = 0; ii < defaultExtractDim[1]; ii++)  // For every row in the extracted image...
        {
            offset = offsetStart + (ii * width * numComponents);
            pngExtract.insert(pngExtract.end(), dataPNG.begin() + offset, dataPNG.begin() + offset + bytesPerRow);
        }
        return pngExtract;
    }
    template<> vector<unsigned char> pngExtractRect<vector<unsigned char>, vector<int>, vector<int>>(vector<int>& center, vector<unsigned char>& source, vector<int>& sourceDim, vector<int>& extractDim)
    {
        // Dimension vectors have form [width, height]. Also, assume 3 components (RGB).
        vector<unsigned char> pngExtract;
        int bytesPerRow, offsetStart, offset;
        vector<vector<int>> corners = checkBoundary(center, sourceDim, extractDim);
        center = corners[0];
        if (corners.size() > 1)
        {
            extractDim[0] = corners[1][0] - corners[0][0];
            extractDim[1] = corners[1][1] - corners[0][1];
        }
        bytesPerRow = extractDim[0] * numComponents;
        offsetStart = getOffset(corners[0], sourceDim[0]);
        for (int ii = 0; ii < extractDim[1]; ii++)  // For every row in the extracted image...
        {
            offset = offsetStart + (ii * sourceDim[0] * numComponents);
            pngExtract.insert(pngExtract.end(), source.begin() + offset, source.begin() + offset + bytesPerRow);
        }
        return pngExtract;
    }

};
