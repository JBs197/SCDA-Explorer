#pragma once
#include "stb_image.h"
#include "stb_image_write.h"
#include "stb_truetype.h"
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
    int candidateRelativeLengthMin = 50;
    int candidateRelativeWidthMin = 50;
    vector<unsigned char> dataPNG, debugDataPNG;
	bool debug = 1;
    int defaultCharSpace = 1;
    int defaultDotWidth = 5;
    string defaultFont = "Sylfaen";
    int defaultLineWidth = 4;
    int defaultOctogonWidth = 3;
    vector<int> defaultExtractDim = { 400, 400 };
    int defaultSearchRadius = 15;
    double defaultWidthTestRatio = 3.0;
    vector<vector<unsigned char>> font;  // Index is ascii minus 32.
    int fontHeight = 32;  // Pixels.
    string pathActivePNG;
	JFUNC jf;
	unordered_map<string, string> mapColour;
    unordered_map<int, int> mapFontWidth;  // Input ascii, output glyph width (pixels).  
	vector<int> pointOfOrigin, revisedExtractDim;
    vector<vector<unsigned char>> pngTextColourBuffer;
	int width, height, numComponents, recordVictor;
    int rabbitHole = 0;
    int searchRadiusIncrease = 0;
    double stretchFactor;

    vector<unsigned char> Black = { 0, 0, 0 };
    vector<unsigned char> Blue = { 0, 0, 255 };
    vector<unsigned char> Gold = { 255, 170, 0 };
    vector<unsigned char> Green = { 0, 255, 0 };
    vector<unsigned char> Orange = { 255, 155, 55 };
    vector<unsigned char> Pink = { 255, 0, 255 };
    vector<unsigned char> Purple = { 160, 50, 255 };
    vector<unsigned char> Red = { 255, 0, 0 };
    vector<unsigned char> Teal = { 0, 155, 255 };
    vector<unsigned char> White = { 255, 255, 255 };
    vector<unsigned char> Yellow = { 255, 255, 0 };

public:
	explicit IMGFUNC() {}
	~IMGFUNC() {}
	vector<int> borderFindNext(SWITCHBOARD& sbgui, vector<vector<int>> tracks);
	vector<int> borderFindStart();
    void buildFont(string filePath);
    vector<vector<int>> checkBoundary(vector<int>& center, vector<int>& sourceDim, vector<int>& extractDim);
    double clockwisePercentage(SWITCHBOARD& sbgui, vector<vector<int>>& tracks, string sZone);
    vector<vector<int>> coordPath(vector<vector<int>> startStop);
    int coordRGB(vector<vector<int>> startStop, string szone);
    vector<int> coordStoi(string& sCoords);
	void drawMarker(vector<unsigned char>& img, vector<int>& vCoord);
    vector<vector<double>> frameCorners();
    double getStretchFactor(string& widthHeight);
    void initGlyph(string& filePath, int ascii);
	void initMapColours();
	bool isInit();
    bool jobsDone(vector<int> vCoord);
	vector<vector<int>> linePath(vector<vector<int>>& startStop);
    vector<vector<int>> linePathToEdge(vector<vector<int>>& startMid);
	vector<vector<unsigned char>> lineRGB(vector<vector<int>>& vVictor, int length);
	bool mapIsInit();
    vector<vector<int>> octogonPath(vector<int> origin, int radius);
    string pixelDecToHex(vector<unsigned char>& rgb);
	void pixelPaint(vector<unsigned char>& img, int widthImg, vector<unsigned char> rgb, vector<int> coord);
	vector<unsigned char> pixelRGB(vector<int>& coord);
	string pixelZone(vector<unsigned char>& rgb);
    vector<unsigned char> pngBlankCanvas(vector<int>& dim);
    vector<unsigned char> pngExtractRow(int row, vector<unsigned char>& img, vector<int>& sourceDim);
	void pngLoad(string& pathPNG);
    void pngToBinLive(SWITCHBOARD& sbgui, vector<vector<double>>& border);
    void pngToBinLiveDebug(SWITCHBOARD& sbgui, vector<vector<double>>& border);
    int testCandidatesInteriorZone(SWITCHBOARD& sbgui, vector<vector<int>>& tracks, string sZone, vector<vector<int>>& candidates);
    int testTextHumanFeature(vector<vector<unsigned char>>& Lrgb);
    vector<int> testZoneLength(vector<vector<int>>& pastPresent, vector<vector<int>>& candidates, string sZone);
    int testZoneSweepLetters(vector<vector<int>>& zonePath, vector<vector<unsigned char>>& Lrgb, vector<vector<int>>& candidates, unordered_map<string, int>& mapIndexCandidate);
    vector<vector<int>> zoneChangeLinear(vector<string>& szones, vector<vector<int>>& ivec);
    void zoneSweepDebug(vector<vector<int>>& vCoord, int radius);

	// TEMPLATES

    template<typename ... Args> void coordDist(Args& ... args)
    {
        jf.err("coordDist.im");
    }
    template<> void coordDist<vector<int>, vector<vector<int>>, vector<double>>(vector<int>& origin, vector<vector<int>>& cList, vector<double>& distances)
    {
        distances.resize(cList.size());
        int inumX, inumY;
        for (int ii = 0; ii < cList.size(); ii++)
        {
            inumX = (cList[ii][0] - origin[0]) * (cList[ii][0] - origin[0]);
            inumY = (cList[ii][1] - origin[1]) * (cList[ii][1] - origin[1]);
            distances[ii] = sqrt(inumX + inumY);
        }
    }
    template<> void coordDist<vector<vector<double>>, double>(vector<vector<double>>& vCoords, double& distance)
    {
        double inumX, inumY;
        inumX = (vCoords[1][0] - vCoords[0][0]) * (vCoords[1][0] - vCoords[0][0]);
        inumY = (vCoords[1][1] - vCoords[0][1]) * (vCoords[1][1] - vCoords[0][1]);
        distance = sqrt(inumX + inumY);
    }

    template<typename ... Args> vector<int> coordMid(vector<vector<int>>& vCoords, Args& ... args)
    {
        // Returns the mid point between the two points specified in vCoords.
        // If no additional parameter is given, the mid point is taken along
        // the straight line between vCoords. Otherwise, the mid point is taken
        // along a circle arc between vCoords. The second 2D vector gives the 
        // origin point and the radius for the arc. 
        jf.err("coordMid template.im");
    }
    template<> vector<int> coordMid< >(vector<vector<int>>& vCoords)
    {
        vector<int> vMid(2);
        vMid[0] = (vCoords[0][0] + vCoords[1][0]) / 2;
        vMid[1] = (vCoords[0][1] + vCoords[1][1]) / 2;
        return vMid;
    }
    template<> vector<int> coordMid<vector<int>>(vector<vector<int>>& vCoords, vector<int>& originRadius)
    {
        // originRadius has form [xCoord, yCoord, radius].
        vector<int> vMidArc(2);
        vector<double> vMidTemp(2);
        vMidTemp[0] = ((double)vCoords[0][0] + (double)vCoords[1][0]) / 2.0;
        vMidTemp[1] = ((double)vCoords[0][1] + (double)vCoords[1][1]) / 2.0;
        double Dx = vMidTemp[0] - originRadius[0];
        double Dy = vMidTemp[1] - originRadius[1];
        double theta = atan(Dy / Dx);
        vector<vector<double>> radialCoords(2, vector<double>(2));
        radialCoords[0] = { (double)originRadius[0], (double)originRadius[1] };
        radialCoords[1] = vMidTemp;
        double distance;
        coordDist(radialCoords, distance);
        bool pushOut;
        double radius = (double)originRadius[2];
        if (distance < radius) { pushOut = 1; }
        else if (distance > radius) { pushOut = 0; }
        else 
        {
            vMidArc[0] = int(vMidTemp[0]);
            vMidArc[1] = int(vMidTemp[1]);
            return vMidArc;
        }
        
        double DxNew = radius * cos(abs(theta));
        double DyNew = radius * sin(abs(theta));
        if (Dx >= 0.0) { vMidArc[0] = originRadius[0] + int(DxNew); }
        else { vMidArc[0] = originRadius[0] - int(round(DxNew)); }
        if (Dy >= 0.0) { vMidArc[1] = originRadius[1] + int(DyNew); }
        else { vMidArc[1] = originRadius[1] - int(round(DyNew)); }
        
        return vMidArc;
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

    template<typename ... Args> void deleteColumn(int col, Args& ... args)
    {
        jf.err("deleteColumn template-im");
    }
    template<> void deleteColumn<vector<unsigned char>, vector<int>>(int col, vector<unsigned char>& img, vector<int>& sourceDim)
    {
        if (col >= sourceDim[0]) { jf.err("Column out of bounds-im.deleteColumn"); }
        int offset;
        vector<int> coord(2);
        coord[0] = col;
        for (int ii = sourceDim[1] - 1; ii >= 0; ii--)
        {
            coord[1] = ii;
            offset = getOffset(coord, sourceDim[0]);
            img.erase(img.begin() + offset, img.begin() + offset + 3);
        }
        sourceDim[0] -= 1;
    }

    template<typename ... Args> void dotPaint(vector<int> coord, vector<unsigned char> rgb, Args& ... args)
    {
        jf.err("dotPaint template-im");
    }
    template<> void dotPaint< >(vector<int> coord, vector<unsigned char> rgb)
    {
        // Uses defaultDotWidth and debugDataPNG.
        if (debugDataPNG.size() < 1) { jf.err("debugDataPNG not prepared-im.dotPaint"); }
        vector<int> vTemp(2);
        for (int ii = 0; ii < defaultDotWidth; ii++)
        {
            vTemp[1] = coord[1] - (defaultDotWidth / 2) + ii;
            for (int jj = 0; jj < defaultDotWidth; jj++)
            {
                vTemp[0] = coord[0] - (defaultDotWidth / 2) + jj;
                pixelPaint(debugDataPNG, width, rgb, vTemp);
            }
        }
    }
    template<> void dotPaint<vector<unsigned char>, int, int>(vector<int> coord, vector<unsigned char> rgb, vector<unsigned char>& img, int& widthImg, int& widthDot)
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
        jf.err("getOffset template-im");
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

    template<typename ... Args> void linePaint(vector<vector<int>> startStop, vector<unsigned char> rgb, Args& ... args)
    {
        jf.err("linePaint template-im");
    }
    template<> void linePaint< >(vector<vector<int>> startStop, vector<unsigned char> rgb)
    {
        vector<vector<int>> path = linePath(startStop);
        for (int ii = 0; ii < path.size(); ii++)
        {
            dotPaint(path[ii], rgb, debugDataPNG, width, defaultLineWidth);
        }
    }
    template<> void linePaint<vector<unsigned char>, int, int>(vector<vector<int>> startStop, vector<unsigned char> rgb, vector<unsigned char>& img, int& widthImg, int& widthDot)
    {
        vector<vector<int>> path = linePath(startStop);
        for (int ii = 0; ii < path.size(); ii++)
        {
            dotPaint(path[ii], rgb, img, widthImg, widthDot);
        }
    }

    template<typename ... Args> void makeMapBorderFindNext(vector<vector<int>>& tracks, int radius, vector<vector<int>> candidates, Args& ... args)
    {
        // Debug map maker for the 'borderFindNext' function. 
        jf.err("makeMapBorderFindNext template-im");
    }
    template<> void makeMapBorderFindNext< >(vector<vector<int>>& tracks, int radius, vector<vector<int>> candidates)
    {
        if (tracks.size() < 1) { jf.err("No tracks-im.makeMapBorderFindNext"); }
        if (!mapIsInit()) { initMapColours(); }
        debugDataPNG = dataPNG;
        string dotLegend;
        pngTextColourBuffer.clear();
        if (radius > 0) { octogonPaint(tracks[tracks.size() - 1], radius, Orange); }        
        octogonPaint(tracks[tracks.size() - 1], defaultSearchRadius, Gold);

        vector<vector<int>> startStop(2, vector<int>());
        for (int ii = 0; ii < tracks.size() - 1; ii++)
        {
            startStop[0] = tracks[ii];
            startStop[1] = tracks[ii + 1];
            linePaint(startStop, Teal);
        }

        for (int ii = 0; ii < tracks.size() - 1; ii++)
        {
            dotPaint(tracks[ii], Green);
            pngAppendText(dotLegend, tracks[ii]);
            pngTextColourBuffer.push_back(Green);
        }
        dotPaint(tracks[tracks.size() - 1], Yellow);
        pngAppendText(dotLegend, tracks[tracks.size() - 1]);
        pngTextColourBuffer.push_back(Yellow);
        
        for (int ii = 0; ii < candidates.size(); ii++)
        {
            dotPaint(candidates[ii], Red);
            pngAppendText(dotLegend, candidates[ii]);
            pngTextColourBuffer.push_back(Red);
        }

        vector<int> legendDim;
        vector<unsigned char> legendImg;
        //string legendPath = "F:\\debug\\Legend Test.png";
        makeText(dotLegend, legendImg, legendDim, pngTextColourBuffer);
        //pngPrint(legendImg, legendDim, legendPath);

        vector<int> sourceDim = { width, height };
        vector<unsigned char> cropped = pngExtractRect(tracks[tracks.size() - 1], debugDataPNG, sourceDim, defaultExtractDim);
        int newHeight = max(defaultExtractDim[1], legendDim[1]);
        vector<int> oldNew = { defaultExtractDim[0], defaultExtractDim[1], defaultExtractDim[0] + legendDim[0], newHeight };
        pngExtend(cropped, oldNew);
        sourceDim = { oldNew[2], oldNew[3] };
        vector<int> coordPaste = { oldNew[0], 0 };
        pngPaste(cropped, sourceDim, legendImg, legendDim, coordPaste);

        int imgSize = cropped.size();
        int channels = 3;
        auto bufferUC = new unsigned char[imgSize];
        for (int ii = 0; ii < imgSize; ii++)
        {
            bufferUC[ii] = cropped[ii];
        }
        string pathImg = sroot + "\\debug\\borderFindNextDebug.png";
        int error = stbi_write_png(pathImg.c_str(), oldNew[2], oldNew[3], channels, bufferUC, 0);
        delete[] bufferUC;
    }
    template<> void makeMapBorderFindNext<string>(vector<vector<int>>& tracks, int radius, vector<vector<int>> candidates, string& pathImg)
    {
        if (tracks.size() < 1) { jf.err("No tracks-im.makeMapBorderFindNext"); }
        if (!mapIsInit()) { initMapColours(); }
        debugDataPNG = dataPNG;
        string dotLegend;
        pngTextColourBuffer.clear();
        if (radius > 0) { octogonPaint(tracks[tracks.size() - 1], radius, Orange); }
        octogonPaint(tracks[tracks.size() - 1], defaultSearchRadius, Gold);

        vector<vector<int>> startStop(2, vector<int>());
        for (int ii = 0; ii < tracks.size() - 1; ii++)
        {
            startStop[0] = tracks[ii];
            startStop[1] = tracks[ii + 1];
            linePaint(startStop, Teal);
        }

        for (int ii = 0; ii < tracks.size() - 1; ii++)
        {
            dotPaint(tracks[ii], Green);
            pngAppendText(dotLegend, tracks[ii]);
            pngTextColourBuffer.push_back(Green);
        }
        dotPaint(tracks[tracks.size() - 1], Yellow);
        pngAppendText(dotLegend, tracks[tracks.size() - 1]);
        pngTextColourBuffer.push_back(Yellow);

        for (int ii = 0; ii < candidates.size(); ii++)
        {
            dotPaint(candidates[ii], Red);
            pngAppendText(dotLegend, candidates[ii]);
            pngTextColourBuffer.push_back(Red);
        }

        vector<int> legendDim;
        vector<unsigned char> legendImg;
        //string legendPath = "F:\\debug\\Legend Test.png";
        makeText(dotLegend, legendImg, legendDim, pngTextColourBuffer);
        //pngPrint(legendImg, legendDim, legendPath);

        vector<int> sourceDim = { width, height };
        vector<unsigned char> cropped = pngExtractRect(tracks[tracks.size() - 1], debugDataPNG, sourceDim, defaultExtractDim);
        int newHeight = max(defaultExtractDim[1], legendDim[1]);
        vector<int> oldNew = { defaultExtractDim[0], defaultExtractDim[1], defaultExtractDim[0] + legendDim[0], newHeight };
        pngExtend(cropped, oldNew);
        sourceDim = { oldNew[2], oldNew[3] };
        vector<int> coordPaste = { oldNew[0], 0 };
        pngPaste(cropped, sourceDim, legendImg, legendDim, coordPaste);

        int imgSize = cropped.size();
        int channels = 3;
        auto bufferUC = new unsigned char[imgSize];
        for (int ii = 0; ii < imgSize; ii++)
        {
            bufferUC[ii] = cropped[ii];
        }
        int error = stbi_write_png(pathImg.c_str(), oldNew[2], oldNew[3], channels, bufferUC, 0);
        delete[] bufferUC;
    }

    template<typename ... Args> void makeMapOctogonBearing(Args& ... args)
    {
        // Given two (past, present) points, as well as a minimum of one 
        // candidate point, make a debug PNG showing the search octogon, 
        // the pastPresent input vector, all the candidate vectors, and
        // all the input points as dots. 
        jf.err("makeMapDebug template-im");
    }
    template<> void makeMapOctogonBearing<vector<vector<int>>>(vector<vector<int>>& pastPresentFuture)
    {
        if (!mapIsInit()) { initMapColours(); }
        debugDataPNG = dataPNG;
        int widthDot = 5;
        int widthLine = 5;
        vector<vector<int>> startStop(2, vector<int>());
        startStop[0] = pastPresentFuture[0];
        startStop[1] = pastPresentFuture[1];
        linePaint(startStop, debugDataPNG, width, Teal, widthLine);
        startStop[0] = pastPresentFuture[1];
        startStop[1] = pastPresentFuture[2];
        linePaint(startStop, debugDataPNG, width, Orange, widthLine);
        dotPaint(pastPresentFuture[0], debugDataPNG, width, Green, widthDot);
        dotPaint(pastPresentFuture[1], debugDataPNG, width, Yellow, widthDot);
        dotPaint(pastPresentFuture[2], debugDataPNG, width, Red, widthDot);
        vector<int> sourceDim = { width, height };
        vector<unsigned char> cropped = pngExtractRect(pastPresentFuture[1], debugDataPNG, sourceDim, defaultExtractDim);
        int imgSize = cropped.size();
        int channels = 3;
        auto bufferUC = new unsigned char[imgSize];
        for (int ii = 0; ii < imgSize; ii++)
        {
            bufferUC[ii] = cropped[ii];
        }
        string pathImg = sroot + "\\debug\\tempDebug.png";
        int error = stbi_write_png(pathImg.c_str(), defaultExtractDim[0], defaultExtractDim[1], channels, bufferUC, 0);
        delete[] bufferUC;
    }
    template<> void makeMapOctogonBearing<vector<vector<double>>>(vector<vector<double>>& pPF)
    {
        vector<vector<int>> pastPresentFuture;
        jf.toInt(pPF, pastPresentFuture);
        makeMapOctogonBearing(pastPresentFuture);
    }

    template<typename ... Args> void makeMapZoneSweepDebug(vector<vector<unsigned char>>& Lrgb, vector<vector<int>>& zonePath, vector<vector<int>>& goldilocks, Args& ... args)
    {
        jf.err("makeMapZoneSweepDebug template.im");
    }
    template<> void makeMapZoneSweepDebug< >(vector<vector<unsigned char>>& Lrgb, vector<vector<int>>& zonePath, vector<vector<int>>& goldilocks)
    {
        if (!mapIsInit()) { initMapColours(); }
        debugDataPNG = dataPNG;
        int left = zonePath[0][0];
        int right = left;
        int top = zonePath[0][1];
        int bot = top;
        for (int ii = 0; ii < zonePath.size(); ii++)
        {
            if (zonePath[ii][0] < 0) { jf.err("negative xCoord-im.makeMapZoneSweepDebug"); }
            if (zonePath[ii][1] < 0) { jf.err("negative yCoord-im.makeMapZoneSweepDebug"); }
            if (zonePath[ii][0] < left) { left = zonePath[ii][0]; }
            if (zonePath[ii][0] > right) { right = zonePath[ii][0]; }
            if (zonePath[ii][1] < top) { top = zonePath[ii][1]; }
            if (zonePath[ii][1] > bot) { bot = zonePath[ii][1]; }
        }
        int widthTemp = right - left + 1;
        int heightTemp = bot - top + 1;
        vector<int> viTemp(2);
        vector<int> centerTemp(2);
        centerTemp[0] = left + (widthTemp / 2);
        centerTemp[1] = top + (heightTemp / 2);
        int leftCropped = max(0, left - 1);
        int rightCropped = min(width - 1, right + 1);
        int topCropped = max(0, top - 1);
        int botCropped = min(height - 1, bot + 1);
        vector<int> extractDim(2);
        extractDim[0] = rightCropped - leftCropped + 1;
        extractDim[1] = botCropped - topCropped + 1;
        for (int ii = topCropped; ii <= botCropped; ii++)
        {
            viTemp[1] = ii;
            for (int jj = leftCropped; jj <= rightCropped; jj++)
            {
                viTemp[0] = jj;
                pixelPaint(debugDataPNG, width, Black, viTemp);
            }
        }
        for (int ii = 0; ii < zonePath.size(); ii++)
        {
            pixelPaint(debugDataPNG, width, Lrgb[ii], zonePath[ii]);
        }
        for (int ii = 0; ii < goldilocks.size(); ii++)
        {
            pixelPaint(debugDataPNG, width, Red, goldilocks[ii]);
        }
        vector<int> sourceDim = { width, height };
        vector<unsigned char> cropped = pngExtractRect(centerTemp, debugDataPNG, sourceDim, extractDim);
        int imgSize = cropped.size();
        int channels = 3;
        auto bufferUC = new unsigned char[imgSize];
        for (int ii = 0; ii < imgSize; ii++)
        {
            bufferUC[ii] = cropped[ii];
        }
        string pathImg = sroot + "\\debug\\ZoneSweepDebug.png";
        int error = stbi_write_png(pathImg.c_str(), extractDim[0], extractDim[1], channels, bufferUC, 0);
        delete[] bufferUC;
    }

    template<typename ... Args> void makeText(string text, Args& ... args)
    {
        // If a 1D uchar vector is provided, then the produced image will be written 
        // in standard PNG rgb data buffer format (dimensions returned as well). 
        // However, if a 2D uchar vector is provided, then the image will be written
        // per-row such that the first index specifies the image row.
        jf.err("makeText template-im");
    }
    template<> void makeText<vector<unsigned char>, vector<int>>(string text, vector<unsigned char>& img, vector<int>& imgDim)
    {
        // Break the text into a vector of strings, corresponding to output lines.
        vector<string> vText;
        string temp;
        size_t pos1 = 0;
        size_t pos2 = text.find('\n');
        while (pos2 < text.size())
        {
            temp = text.substr(pos1, pos2 - pos1);
            vText.push_back(temp);
            pos1 = pos2 + 1;
            pos2 = text.find('\n', pos1);
        }
        temp = text.substr(pos1);
        vText.push_back(temp);

        // Determine the width (in pixels) needed by each line. 
        int charSpace = defaultCharSpace;
        vector<int> lineWidth(vText.size(), 0);
        vector<int> widest(2, -1);  // Form [index, width].
        int charWidth, ascii;
        for (int ii = 0; ii < lineWidth.size(); ii++)
        {
            lineWidth[ii] += charSpace;
            for (int jj = 0; jj < vText[ii].size(); jj++)
            {
                try { charWidth = mapFontWidth.at(vText[ii][jj]); }
                catch (out_of_range& oor) { jf.err("mapFontWidth-im.makeText"); }
                lineWidth[ii] += charWidth + charSpace;
            }
            if (lineWidth[ii] > widest[1])
            {
                widest[0] = ii;
                widest[1] = lineWidth[ii];
            }
        }
        imgDim.resize(2);
        imgDim[0] = widest[1];
        imgDim[1] = fontHeight * vText.size();

        // Build the image by appending lines.
        vector<unsigned char> imgLine;
        for (int ii = 0; ii < vText.size(); ii++)
        {
            imgLine = makeTextLine(vText[ii], imgDim[0]);
            img.insert(img.end(), imgLine.begin(), imgLine.end());
        }
    }
    template<> void makeText<vector<unsigned char>, vector<int>, vector<vector<unsigned char>>>(string text, vector<unsigned char>& img, vector<int>& imgDim, vector<vector<unsigned char>>& listColour)
    {
        // Break the text into a vector of strings, corresponding to output lines.
        vector<string> vText;
        string temp;
        size_t pos1 = 0;
        size_t pos2 = text.find('\n');
        while (pos2 < text.size())
        {
            temp = text.substr(pos1, pos2 - pos1);
            vText.push_back(temp);
            pos1 = pos2 + 1;
            pos2 = text.find('\n', pos1);
        }
        temp = text.substr(pos1);
        vText.push_back(temp);

        // Determine the width (in pixels) needed by each line. 
        int charSpace = defaultCharSpace;
        vector<int> lineWidth(vText.size(), 0);
        vector<int> widest(2, -1);  // Form [index, width].
        int charWidth, ascii;
        for (int ii = 0; ii < lineWidth.size(); ii++)
        {
            lineWidth[ii] += charSpace;
            for (int jj = 0; jj < vText[ii].size(); jj++)
            {
                try { charWidth = mapFontWidth.at(vText[ii][jj]); }
                catch (out_of_range& oor) { jf.err("mapFontWidth-im.makeText"); }
                lineWidth[ii] += charWidth + charSpace;
            }
            if (lineWidth[ii] > widest[1])
            {
                widest[0] = ii;
                widest[1] = lineWidth[ii];
            }
        }
        imgDim.resize(2);
        imgDim[0] = widest[1] + fontHeight;  // For the square colour box.
        imgDim[1] = fontHeight * vText.size();

        // Make the legend colour box.
        img = pngBlankCanvas(imgDim);
        vector<int> topLeft = { 8, 8 };
        vector<int> boxDim = { 16, 16 };
        int thickness = 3;
        for (int ii = 0; ii < vText.size(); ii++)
        {
            rectanglePaint(topLeft, boxDim, Black, img, imgDim, thickness, listColour[ii]);
            topLeft[1] += fontHeight;
        }

        // Build the image by pasting lines.
        vector<unsigned char> imgLine;
        vector<int> imgLineDim = { widest[1], fontHeight };
        vector<int> pasteTopLeft = { fontHeight, 0 };
        for (int ii = 0; ii < vText.size(); ii++)
        {
            imgLine = makeTextLine(vText[ii], widest[1]);
            pngPaste(img, imgDim, imgLine, imgLineDim, pasteTopLeft);
            pasteTopLeft[1] += fontHeight;
        }
    }

    template<typename ... Args> vector<unsigned char> makeTextLine(string textLine, Args& ... args)
    {
        jf.err("makeTextLine template-im");
    }
    template<> vector<unsigned char> makeTextLine< >(string textLine)
    {
        vector<unsigned char> imgLine, imgRow;
        vector<vector<unsigned char>> imgTemp(textLine.size(), vector<unsigned char>());
        vector<vector<int>> glyphDim(textLine.size(), vector<int>(2, fontHeight));
        int ascii;
        for (int ii = 0; ii < imgTemp.size(); ii++)
        {
            ascii = textLine[ii];
            imgTemp[ii] = font[ascii - 32];
            try { glyphDim[ii][0] = mapFontWidth.at(ascii); }
            catch (out_of_range& oor) { jf.err("mapFontWidth-im.makeTextLine"); }
        }
        int charSpace = defaultCharSpace;
        for (int ii = 0; ii < fontHeight; ii++)
        {
            for (int cs = 0; cs < charSpace; cs++) { imgLine.insert(imgLine.end(), White.begin(), White.end()); }
            for (int jj = 0; jj < imgTemp.size(); jj++)
            {
                imgRow = pngExtractRow(ii, imgTemp[jj], glyphDim[jj]);
                imgLine.insert(imgLine.end(), imgRow.begin(), imgRow.end());
                for (int cs = 0; cs < charSpace; cs++) { imgLine.insert(imgLine.end(), White.begin(), White.end()); }
            }
        }
        return imgLine;
    }
    template<> vector<unsigned char> makeTextLine<int>(string textLine, int& imgWidth)
    {
        // This version fills extra spaces at the end with white pixels.
        vector<unsigned char> imgLine, imgRow;
        vector<vector<unsigned char>> imgTemp(textLine.size(), vector<unsigned char>());
        vector<vector<int>> glyphDim(textLine.size(), vector<int>(2, fontHeight));
        int ascii, numPixels, gap;
        for (int ii = 0; ii < imgTemp.size(); ii++)
        {
            ascii = textLine[ii];
            imgTemp[ii] = font[ascii - 32];
            try { glyphDim[ii][0] = mapFontWidth.at(ascii); }
            catch (out_of_range& oor) { jf.err("mapFontWidth-im.makeTextLine"); }
        }
        int charSpace = defaultCharSpace;
        for (int ii = 0; ii < fontHeight; ii++)
        {
            for (int cs = 0; cs < charSpace; cs++) { imgLine.insert(imgLine.end(), White.begin(), White.end()); }
            numPixels = 1;
            for (int jj = 0; jj < imgTemp.size(); jj++)
            {
                imgRow = pngExtractRow(ii, imgTemp[jj], glyphDim[jj]);
                imgLine.insert(imgLine.end(), imgRow.begin(), imgRow.end());
                numPixels += (imgRow.size() / 3);
                for (int cs = 0; cs < charSpace; cs++) { imgLine.insert(imgLine.end(), White.begin(), White.end()); }
                numPixels++;
            }
            gap = imgWidth - numPixels;
            if (gap < 0) { jf.err("imgWidth-im.makeTextLine"); }
            for (int jj = 0; jj < gap; jj++)
            {
                for (int cs = 0; cs < charSpace; cs++) { imgLine.insert(imgLine.end(), White.begin(), White.end()); }
            }
        }
        return imgLine;
    }

    template<typename ... Args> void octogonBearing(SWITCHBOARD& sbgui, vector<vector<int>>& pastPresent, string sZone, int radius, Args& ... args)
    {
        // Uses the past [0] and present [1] coordinates to define an initial 
        // vector. A ring of pixels around [1] are scanned for the given sZone. 
        // New vectors are defined from [1] to the center of each such sZone found
        // in the ring. The returned values are the angles in degrees [0, 360) 
        // between the inital vector and the new vector. 
        // 
        // If only one angle is accepted as theta, then a series of tests are 
        // performed to determine the best candidate. 
        jf.err("octogonBearing template-im");
    }
    template<> void octogonBearing<double, int>(SWITCHBOARD& sbgui, vector<vector<int>>& pastPresent, string sZone, int radius, double& theta, int& widthZone)
    {
        unordered_map<string, int> mapIndexCandidate;
        unordered_map<string, int> mapWidth;
        vector<vector<int>> octoPath = octogonPath(pastPresent[1], radius);
        vector<vector<unsigned char>> octoRGB = octogonRGB(octoPath);
        vector<vector<int>> candidates = zoneSweep(sZone, octoRGB, octoPath, mapIndexCandidate, mapWidth);
        int numCandidates = candidates.size();
        int lettersExist = 0;
        vector<vector<int>> pastPresentFuture = pastPresent;
        string sCandidate;
        int iWidth;
        
        // Test for letter zone sandwiches.
        if (numCandidates > 1)
        {
            lettersExist = testTextHumanFeature(octoRGB);
            if (lettersExist > 0)
            {
                numCandidates = testZoneSweepLetters(octoPath, octoRGB, candidates, mapIndexCandidate);
            }
        }

        // Relaunch octogonBearing with a larger or smaller search radius. If 
        // the search radius passes outside the interval (2, 4*default), then 
        // indicate failure by changing theta to -2.0 from -1.0 (default).
        if (numCandidates > 1)
        {
            if (radius > 2)
            {
                octogonBearing(sbgui, pastPresent, sZone, radius - 2, theta, widthZone);
                if (radius != defaultSearchRadius) { return; }
            }
            else if (radius <= 2)
            {
                theta = -2.0;
                return;
            }
        }
        else if (numCandidates < 1)
        {
            if (radius < 4 * defaultSearchRadius)
            {
                octogonBearing(sbgui, pastPresent, sZone, radius + 2, theta, widthZone);
                if (radius != defaultSearchRadius) { return; }
            }
            else
            {
                theta = -2.0;
                return;
            }
        }

        // Apply the zone width and zone length tests. If neither of them yields
        // sufficiently strong results, then make a debug PNG and admit defeat.
        if (numCandidates != 1 && theta < 0.0)
        {
            // Zone width test.
            vector<int> candidateWide;  // Form [xCoord, yCoord, runnerup's relative width (percent)]
            int sizeMax = 0;
            int sizeSecond = 0;
            for (int ii = 0; ii < numCandidates; ii++)
            {
                sCandidate = to_string(candidates[ii][0]) + "," + to_string(candidates[ii][1]);
                try { iWidth = mapWidth.at(sCandidate); }
                catch (out_of_range& oor) { jf.err("mapWidth-im.octogonBearing"); }
                if (iWidth > sizeMax)
                {
                    sizeSecond = sizeMax;
                    sizeMax = iWidth;
                    candidateWide = candidates[ii];
                }
                else if (iWidth > sizeSecond)
                {
                    sizeSecond = iWidth;
                }
            }
            candidateWide.push_back((100 * sizeSecond) / sizeMax);

            // Zone length test.
            vector<int> candidateLong = testZoneLength(pastPresent, candidates, sZone);

            // Last chance.
            if (candidateLong[2] >= candidateWide[2])
            {
                if (candidateLong[2] >= candidateRelativeLengthMin)
                {
                    pastPresentFuture.push_back({ candidateLong[0], candidateLong[1] });
                    theta = jf.angleBetweenVectors(pastPresentFuture);
                    return;
                }
                else if (candidateWide[2] >= candidateRelativeWidthMin)
                {
                    pastPresentFuture.push_back({ candidateWide[0], candidateWide[1] });
                    theta = jf.angleBetweenVectors(pastPresentFuture);
                    return;
                }
            }
            else
            {
                if (candidateWide[2] >= candidateRelativeWidthMin)
                {
                    pastPresentFuture.push_back({ candidateWide[0], candidateWide[1] });
                    theta = jf.angleBetweenVectors(pastPresentFuture);
                    return;
                }
                else if (candidateLong[2] >= candidateRelativeLengthMin)
                {
                    pastPresentFuture.push_back({ candidateLong[0], candidateLong[1] });
                    theta = jf.angleBetweenVectors(pastPresentFuture);
                    return;
                }
            }

            // Defeat.
            return;
        }

        if (numCandidates == 1)
        {
            pastPresentFuture.push_back(candidates[0]);
            theta = jf.angleBetweenVectors(pastPresentFuture);
            sCandidate = to_string(candidates[0][0]) + "," + to_string(candidates[0][1]);
            try { iWidth = mapWidth.at(sCandidate); }
            catch (out_of_range& oor) { jf.err("mapWidth-im.octogonBearing"); }
            widthZone = iWidth;
            return;
        }
    }
    template<> void octogonBearing<vector<double>>(SWITCHBOARD& sbgui, vector<vector<int>>& pastPresent, string sZone, int radius, vector<double>& theta)
    {
        unordered_map<string, int> mapIndexCandidate;
        vector<int> intervalSize;
        vector<vector<int>> octoPath = octogonPath(pastPresent[1], radius);
        vector<vector<unsigned char>> octoRGB = octogonRGB(octoPath);
        vector<vector<int>> lightHouse = zoneSweep(sZone, octoRGB, octoPath, mapIndexCandidate, intervalSize);
        int numCandidates = lightHouse.size();
        vector<vector<int>> pastPresentFuture = pastPresent;
        pastPresentFuture.push_back(vector<int>());
        for (int ii = 0; ii < numCandidates; ii++)
        {
            pastPresentFuture[2] = lightHouse[ii];
            theta.push_back(jf.angleBetweenVectors(pastPresentFuture));
        }
    }

    template<typename ... Args> void octogonPaint(vector<int> origin, int radius, vector<unsigned char> rgb, Args& ... args)
    {
        jf.err("octogonPaint template-im");
    }
    template<> void octogonPaint< >(vector<int> origin, int radius, vector<unsigned char> rgb)
    {
        if (debugDataPNG.size() < 1) { jf.err("debugDataPNG not initialized-im.octogonPaint"); }
        vector<vector<int>> path = octogonPath(origin, radius);
        for (int ii = 0; ii < path.size(); ii++)
        {
            dotPaint(path[ii], rgb, debugDataPNG, width, defaultOctogonWidth);
        }
    }
    template<> void octogonPaint<vector<unsigned char>, int, int>(vector<int> origin, int radius, vector<unsigned char> rgb, vector<unsigned char>& img, int& widthImg, int& widthDot)
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
        vector<int> viTemp = origin;
        viTemp[1] = origin[1] - radius;
        vector<vector<unsigned char>> octoRGB(1, vector<unsigned char>(3));
        octoRGB[0] = pixelRGB(viTemp);
        vector<vector<int>> vVictor = { viTemp, {1, 0} };
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
            octoRGB[ii] = pixelRGB(octoPath[ii]);
        }
        return octoRGB;
    }

    template<typename ... Args> void pngAppendText(string& text, Args& ... args)
    {
        jf.err("pngAppendText-im");
    }
    template<> void pngAppendText<vector<int>>(string& text, vector<int>& coord)
    {
        string temp;
        if (text.size() < 1)
        {
            temp = "(" + to_string(coord[0]) + ", " + to_string(coord[1]) + ")";
        }
        else
        {
            temp = "\n(" + to_string(coord[0]) + ", " + to_string(coord[1]) + ")";
        }
        text.append(temp);
    }

    template<typename ... Args> void pngExtend(Args& ... args)
    {
        jf.err("pngExtend-im");
    }
    template<> void pngExtend<vector<unsigned char>, vector<int>>(vector<unsigned char>& img, vector<int>& oldNew)
    {
        // oldNew has form [oldWidth, oldHeight, newWidth, newHeight].
        if (oldNew.size() != 4) { jf.err("Improper dimensions-im.pngExtend"); }
        if (oldNew[2] < oldNew[0] || oldNew[3] < oldNew[1]) { jf.err("New dimensions smaller than original-im.pngExtend"); }

        int newSpace, offset;
        vector<int> coord(2);
        if (oldNew[2] > oldNew[0])  // Extend horizontally.
        {
            newSpace = oldNew[2] - oldNew[0];
            coord[0] = oldNew[0] - 1;
            for (int ii = oldNew[1] - 1; ii >= 0; ii--)
            {
                coord[1] = ii;
                offset = getOffset(coord, oldNew[0]);
                for (int cs = 0; cs < newSpace; cs++) { img.insert(img.begin() + offset, White.begin(), White.end()); }
            }
        }
        if (oldNew[3] > oldNew[1])  // Extend vertically.
        {
            newSpace = oldNew[3] - oldNew[1];
            newSpace *= oldNew[2];
            for (int cs = 0; cs < newSpace; cs++) { img.insert(img.end(), White.begin(), White.end()); }
        }
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
        //center = corners[0];
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
        //center = corners[0];
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

    template<typename ... Args> vector<unsigned char> pngExtractRectTopLeft(vector<int>& topLeft, Args& ... args)
    {
        jf.err("pngExtractRect template");
    }
    template<> vector<unsigned char> pngExtractRectTopLeft< >(vector<int>& topLeft)
    {
        // Dimension vectors have form [width, height]. Also, assume 3 components (RGB).
        vector<unsigned char> pngExtract;
        vector<int> sourceDim = { width, height };
        vector<int> extractDim = defaultExtractDim;
        if (topLeft[0] < 0 || topLeft[0] + extractDim[0] >= width) { jf.err("xCoord out of bounds-im.pngExtractRectTopLeft"); }
        if (topLeft[1] < 0 || topLeft[1] + extractDim[1] >= height) { jf.err("yCoord out of bounds-im.pngExtractRectTopLeft"); }
        int bytesPerRow, offsetStart, offset;
        bytesPerRow = extractDim[0] * numComponents;
        offsetStart = getOffset(topLeft, sourceDim[0]);
        for (int ii = 0; ii < defaultExtractDim[1]; ii++)  // For every row in the extracted image...
        {
            offset = offsetStart + (ii * width * numComponents);
            pngExtract.insert(pngExtract.end(), dataPNG.begin() + offset, dataPNG.begin() + offset + bytesPerRow);
        }
        return pngExtract;
    }
    template<> vector<unsigned char> pngExtractRectTopLeft<vector<unsigned char>, vector<int>, vector<int>>(vector<int>& topLeft, vector<unsigned char>& source, vector<int>& sourceDim, vector<int>& extractDim)
    {
        // Dimension vectors have form [width, height]. Also, assume 3 components (RGB).
        vector<unsigned char> pngExtract;
        int bytesPerRow, offsetStart, offset;
        if (topLeft[0] < 0 || topLeft[0] + extractDim[0] >= sourceDim[0]) 
        {
            jf.err("xCoord out of bounds-im.pngExtractRectTopLeft");        
        }
        if (topLeft[1] < 0 || topLeft[1] + extractDim[1] >= sourceDim[1]) { jf.err("yCoord out of bounds-im.pngExtractRectTopLeft"); }
        bytesPerRow = extractDim[0] * numComponents;
        offsetStart = getOffset(topLeft, sourceDim[0]);
        for (int ii = 0; ii < extractDim[1]; ii++)  // For every row in the extracted image...
        {
            offset = offsetStart + (ii * sourceDim[0] * numComponents);
            pngExtract.insert(pngExtract.end(), source.begin() + offset, source.begin() + offset + bytesPerRow);
        }
        return pngExtract;
    }

    template<typename ... Args> void pngPaste(Args& ... args)
    {
        jf.err("pngPaste template-im");
    }
    template<> void pngPaste<vector<unsigned char>, vector<int>, vector<unsigned char>, vector<int>, vector<int>>(vector<unsigned char>& sourceImg, vector<int>& sourceDim, vector<unsigned char>& pasteImg, vector<int>& pasteDim, vector<int>& coord)
    {
        // Note: coord refers to the top-left corner of the pasted image.
        if (coord[0] < 0 || coord[0] + pasteDim[0] > sourceDim[0])
        {
            jf.err("xCoord out of bounds-im.pngPaste");
        }
        if (coord[1] < 0 || coord[1] + pasteDim[1] > sourceDim[1])
        {
            jf.err("yCoord out of bounds-im.pngPaste");
        }
        
        vector<unsigned char> newRow;
        vector<int> offsetCoord(2);
        offsetCoord[0] = coord[0];
        int offset;
        for (int ii = 0; ii < pasteDim[1]; ii++)
        {
            offsetCoord[1] = coord[1] + ii;
            offset = getOffset(offsetCoord, sourceDim[0]);
            newRow = pngExtractRow(ii, pasteImg, pasteDim);
            for (int jj = 0; jj < newRow.size(); jj++)
            {
                sourceImg[offset + jj] = newRow[jj];
            }
        }
    }

    template<typename ... Args> void pngPrint(Args& ... args)
    {
        jf.err("pngPrint template-im");
    }
    template<> void pngPrint<vector<vector<unsigned char>>>(vector<vector<unsigned char>>& img)
    {
        if (img[0].size() != img[img.size() - 1].size()) { jf.err("Width mismatch-im.pngPrint"); }
        string output = sroot + "\\debug\\pngPrintOutput.png";
        int widthImg = img[0].size();
        int heightImg = img.size();
        int sizeImg = widthImg * heightImg;
        int channels = 3;
        auto bufferUC = new unsigned char[sizeImg];
        int pos = 0;
        for (int ii = 0; ii < heightImg; ii++)
        {
            for (int jj = 0; jj < widthImg; jj++)
            {
                bufferUC[pos] = img[ii][jj];
                pos++;
            }
        }
        int error = stbi_write_png(output.c_str(), widthImg, heightImg, channels, bufferUC, 0);
        delete[] bufferUC;
        int barbecue = 1;
    }
    template<> void pngPrint<vector<unsigned char>, vector<int>, string>(vector<unsigned char>& img, vector<int>& sourceDim, string& filePath)
    {
        int sizeImg = sourceDim[0] * sourceDim[1] * 3;
        int channels = 3;
        auto bufferUC = new unsigned char[sizeImg];
        for (int ii = 0; ii < sizeImg; ii++)
        {
            bufferUC[ii] = img[ii];
        }
        int error = stbi_write_png(filePath.c_str(), sourceDim[0], sourceDim[1], channels, bufferUC, 0);
        delete[] bufferUC;
        int barbecue = 1;
    }

    template<typename ... Args> void rectanglePaint(vector<int> topLeft, vector<int> dim, vector<unsigned char> rgb, Args& ... args)
    {
        jf.err("rectanglePaint template-im");
    }
    template<> void rectanglePaint<vector<unsigned char>, vector<int>, int>(vector<int> topLeft, vector<int> dim, vector<unsigned char> rgb, vector<unsigned char>& img, vector<int>& imgDim, int& thickness)
    {
        vector<vector<int>> startStop(2, vector<int>(2));
        startStop[0] = topLeft;
        startStop[1] = { topLeft[0] + dim[0], topLeft[1] };
        linePaint(startStop, rgb, img, imgDim[0], thickness);
        startStop[0] = startStop[1];
        startStop[1] = { topLeft[0] + dim[0], topLeft[1] + dim[1] };
        linePaint(startStop, rgb, img, imgDim[0], thickness);
        startStop[0] = startStop[1];
        startStop[1] = { topLeft[0], topLeft[1] + dim[1] };
        linePaint(startStop, rgb, img, imgDim[0], thickness);
        startStop[0] = startStop[1];
        startStop[1] = topLeft;
        linePaint(startStop, rgb, img, imgDim[0], thickness);
    }
    template<> void rectanglePaint<vector<unsigned char>, vector<int>, int, vector<unsigned char>>(vector<int> topLeft, vector<int> dim, vector<unsigned char> rgb, vector<unsigned char>& img, vector<int>& imgDim, int& thickness, vector<unsigned char>& rgbFill)
    {
        vector<vector<int>> startStop(2, vector<int>(2));
        startStop[0] = topLeft;
        startStop[1] = { topLeft[0] + dim[0], topLeft[1] };
        linePaint(startStop, rgb, img, imgDim[0], thickness);
        startStop[0] = startStop[1];
        startStop[1] = { topLeft[0] + dim[0], topLeft[1] + dim[1] };
        linePaint(startStop, rgb, img, imgDim[0], thickness);
        startStop[0] = startStop[1];
        startStop[1] = { topLeft[0], topLeft[1] + dim[1] };
        linePaint(startStop, rgb, img, imgDim[0], thickness);
        startStop[0] = startStop[1];
        startStop[1] = topLeft;
        linePaint(startStop, rgb, img, imgDim[0], thickness);

        vector<int> pixelFillTL = { topLeft[0] + (thickness / 2) + 1, topLeft[1] + (thickness / 2) + 1 };
        vector<int> pixelFill(2);
        int widthFill = dim[0] - (thickness / 2) - (thickness % 2) - 1;
        int heightFill = dim[1] - (thickness / 2) - (thickness % 2) - 1;
        for (int ii = 0; ii < widthFill; ii++)
        {
            pixelFill[0] = pixelFillTL[0] + ii;
            for (int jj = 0; jj < heightFill; jj++)
            {
                pixelFill[1] = pixelFillTL[1] + jj;
                pixelPaint(img, imgDim[0], rgbFill, pixelFill);
            }
        }
    }

    template<typename ... Args> vector<vector<unsigned char>> scanColumn(int col, Args& ... args)
    {
        jf.err("scanColumn template-im");
    }
    template<> vector<vector<unsigned char>> scanColumn< >(int col)
    {
        if (!isInit()) { jf.err("No PNG loaded-im.scanColumn"); }
        if (col >= width) { jf.err("Column beyond width-im.scanColumn"); }
        vector<vector<unsigned char>> column(height, vector<unsigned char>(3));
        vector<int> coord(2);
        coord[0] = col;
        int offset;
        for (int ii = 0; ii < height; ii++)
        {
            coord[1] = ii;
            offset = getOffset(coord);
            column[ii][0] = dataPNG[offset + 0];
            column[ii][1] = dataPNG[offset + 1];
            column[ii][2] = dataPNG[offset + 2];
        }
        return column;
    }
    template<> vector<vector<unsigned char>> scanColumn<vector<unsigned char>, vector<int>>(int col, vector<unsigned char>& img, vector<int>& sourceDim)
    {
        // sourceDim has form [imgWidth, imgHeight].
        if (col >= sourceDim[0]) { jf.err("Column beyond width-im.scanColumn"); }
        vector<vector<unsigned char>> column(sourceDim[1], vector<unsigned char>(3));
        vector<int> coord(2);
        coord[0] = col;
        int offset;
        for (int ii = 0; ii < sourceDim[1]; ii++)
        {
            coord[1] = ii;
            offset = getOffset(coord, sourceDim[0]);
            column[ii][0] = img[offset + 0];
            column[ii][1] = img[offset + 1];
            column[ii][2] = img[offset + 2];
        }
        return column;
    }

    template<typename ... Args> void trimWidth(vector<unsigned char>& img, vector<int>& sourceDim, Args& ... args)
    {
        jf.err("trimWidth template-im");
    }
    template<> void trimWidth<vector<unsigned char>>(vector<unsigned char>& img, vector<int>& sourceDim, vector<unsigned char>& rgb)
    {
        int BBQ = 0;
        string nameBBQ;
        vector<vector<unsigned char>> column;
        for (int ii = sourceDim[0] - 1; ii >= 0; ii--)
        {
            column = scanColumn(ii, img, sourceDim);
            for (int jj = 0; jj < column.size(); jj++)
            {
                if (column[jj] != rgb) { break; }
                else if (jj == column.size() - 1)
                {
                    deleteColumn(ii, img, sourceDim);
                }
            }
        }
    }

    template<typename ... Args> vector<vector<int>> zoneSweep(string sZone, vector<vector<unsigned char>>& Lrgb, vector<vector<int>>& zonePath, unordered_map<string, int>& mapIndexCandidate, Args& ... args)
    {
        jf.err("zoneSweep template.im");
    }
    template<> vector<vector<int>> zoneSweep< >(string sZone, vector<vector<unsigned char>>& Lrgb, vector<vector<int>>& zonePath, unordered_map<string, int>& mapIndexCandidate)
    {
        // For a given list of RGB values, return the middle pixel coordinates of every
        // (desired) szone interval. Commonly used to scan the perimeter of a shape.
        vector<vector<int>> goldilocks;
        string sCandidate;
        int zoneFreezer = -1;
        vector<int> onOff(2);
        int half, inum1, inum2;
        //vector<vector<int>> startStop(2, vector<int>(2));
        int index = 0;
        bool zoneActive = 0;
        string szone = pixelZone(Lrgb[index]);
        if (szone == sZone)
        {
            while (szone == sZone)
            {
                index++;
                szone = pixelZone(Lrgb[index]);
            }
            zoneFreezer = index - 1;  // Keep for the end.
        }
        index++;
        while (index < Lrgb.size())
        {
            szone = pixelZone(Lrgb[index]);
            if (szone == sZone)
            {
                if (!zoneActive)
                {
                    zoneActive = 1;
                    onOff[0] = index;
                }
            }
            else
            {
                if (zoneActive)
                {
                    zoneActive = 0;
                    onOff[1] = index;
                    half = (onOff[1] - onOff[0]) / 2;
                    inum1 = onOff[0] + half;
                    goldilocks.push_back(zonePath[inum1]);
                    sCandidate = to_string(zonePath[inum1][0]) + "," + to_string(zonePath[inum1][1]);
                    mapIndexCandidate.emplace(sCandidate, inum1);
                }
            }
            index++;
        }
        if (zoneFreezer >= 0)
        {
            if (zoneActive)
            {
                inum1 = index - onOff[0];
                half = (inum1 + zoneFreezer) / 2;
                inum2 = onOff[0] + half;
                if (inum2 >= zonePath.size())
                {
                    inum2 -= zonePath.size();
                }
                goldilocks.push_back(zonePath[inum2]);
                sCandidate = to_string(zonePath[inum2][0]) + "," + to_string(zonePath[inum2][1]);
                mapIndexCandidate.emplace(sCandidate, inum2);
            }
        }
        if (debug == 1 && goldilocks.size() != (size_t)1) { makeMapZoneSweepDebug(Lrgb, zonePath, goldilocks); }
        return goldilocks;
    }
    template<> vector<vector<int>> zoneSweep<unordered_map<string, int>>(string sZone, vector<vector<unsigned char>>& Lrgb, vector<vector<int>>& zonePath, unordered_map<string, int>& mapIndexCandidate, unordered_map<string, int>& mapWidth)
    {
        // For a given list of RGB values, return the middle pixel coordinates of every
        // (desired) szone interval. Commonly used to scan the perimeter of a shape.
        vector<vector<int>> goldilocks;
        string sCandidate;
        int zoneFreezer = -1;
        vector<int> onOff(2);
        int half, inum1, inum2;
        //vector<vector<int>> startStop(2, vector<int>(2));
        int index = 0;
        bool zoneActive = 0;
        string szone = pixelZone(Lrgb[index]);
        if (szone == sZone)
        {
            while (szone == sZone)
            {
                index++;
                szone = pixelZone(Lrgb[index]);
            }
            zoneFreezer = index - 1;  // Keep for the end.
        }
        index++;
        while (index < Lrgb.size())
        {
            szone = pixelZone(Lrgb[index]);
            if (szone == sZone)
            {
                if (!zoneActive)
                {
                    zoneActive = 1;
                    onOff[0] = index;
                }
            }
            else
            {
                if (zoneActive)
                {
                    zoneActive = 0;
                    onOff[1] = index;
                    half = (onOff[1] - onOff[0]) / 2;
                    inum1 = onOff[0] + half;
                    goldilocks.push_back(zonePath[inum1]);
                    sCandidate = to_string(zonePath[inum1][0]) + "," + to_string(zonePath[inum1][1]);
                    mapIndexCandidate.emplace(sCandidate, inum1);
                    mapWidth.emplace(sCandidate, onOff[1] - onOff[0]);
                }
            }
            index++;
        }
        if (zoneFreezer >= 0)
        {
            if (zoneActive)
            {
                inum1 = index - onOff[0];
                half = (inum1 + zoneFreezer) / 2;
                inum2 = onOff[0] + half;
                if (inum2 >= zonePath.size())
                {
                    inum2 -= zonePath.size();
                }
                goldilocks.push_back(zonePath[inum2]);
                sCandidate = to_string(zonePath[inum2][0]) + "," + to_string(zonePath[inum2][1]);
                mapIndexCandidate.emplace(sCandidate, inum2);
                mapWidth.emplace(sCandidate, inum1 + zoneFreezer);
            }
        }
        if (debug == 1 && goldilocks.size() != (size_t)1) { makeMapZoneSweepDebug(Lrgb, zonePath, goldilocks); }
        return goldilocks;
    }

};

