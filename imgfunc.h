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
    bool backtrack = 0;
	vector<vector<int>> borderRegion;  // Sequence of coords that represent the region's border.
	int borderThickness = 6;
    int candidateRelativeLengthMin = 50;
    int candidateRelativeWidthMin = 50;
    vector<unsigned char> dataPNG, debugDataPNG;
	bool debug = 1;
    double defaultAngleIncrement = 5.0;
    double defaultCandidateDistanceTolerance = 0.33;  
    double defaultCenterOfMassTolerance = 45.0;  // Angular deviation.
    int defaultCharSpace = 1;
    string defaultDebugMapPath = sroot + "\\debug\\PTBdebug.png";
    int defaultDotWidth = 5;
    vector<int> defaultExtractDim;
    string defaultFont = "Sylfaen";
    int defaultLineWidth = 4;
    int defaultOctogonWidth = 3;
    int defaultPathLengthImageDebug = 1000;
    int defaultSearchRadius = 15;
    int defaultTextSeparatorWidth = 2;
    int defaultTracksLength = 10;
    double defaultWaterPercentage = 0.45;
    double defaultWidthTestRatio = 3.0;
    int defaultZoneRadialDistanceTolerance = 10;
    int deltaRadius = 0;
    vector<vector<unsigned char>> font;  // Index is ascii minus 32.
    int fontHeight = 32;  // Pixels.
	JFUNC jf;
    vector<unsigned char> legendColourBox;
	unordered_map<string, string> mapColour;
    unordered_map<int, int> mapFontWidth;  // Input ascii, output glyph width (pixels).  
    string pathActivePNG;
    string pathMapDebug;
    int pauseVBP;
    vector<int> pointOfOrigin, revisedExtractDim;
    vector<vector<unsigned char>> pngTextColourBuffer;
	int width, height, numComponents = 3, recordVictor;
    int rabbitHole = 0;
    const int rabbitHoleDepth = 16;
    vector<vector<vector<int>>> savePoints;  // Form [point index][sizeVBP, Origin, Candidate0, ... , CandidateChosen][x,y coords].
    int searchRadiusIncrease = 0;
    double stretchFactor;
    int sizeVBP;
    vector<int> textFound;

    vector<unsigned char> Black = { 0, 0, 0 };
    vector<unsigned char> Blue = { 0, 0, 255 };
    vector<unsigned char> Gold = { 255, 170, 0 };
    vector<unsigned char> Green = { 0, 255, 0 };
    vector<unsigned char> Orange = { 255, 155, 55 };
    vector<unsigned char> Pink = { 255, 0, 255 };
    vector<unsigned char> Purple = { 160, 50, 255 };
    vector<unsigned char> Red = { 255, 0, 0 };
    vector<unsigned char> Teal = { 0, 155, 255 };
    vector<unsigned char> Violet = { 127, 0, 255 };
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
    vector<unsigned char> getColour(string sColour);
    int getDotWidth();
    string getMapPath(int mode);
    int getQuadrant(vector<vector<int>>& startStop);
    double getStretchFactor(string& widthHeight);
    void initGlyph(string& filePath, int ascii);
	void initMapColours();
	bool isInit();
    bool jobsDone(vector<int> vCoord);
	vector<vector<int>> linePath(vector<vector<int>>& startStop);
    vector<vector<int>> linePathToEdge(vector<vector<int>>& startMid);
	vector<vector<unsigned char>> lineRGB(vector<vector<int>>& vVictor, int length);
    void loadRecentSavePoint(vector<vector<int>>& vBorderPath);
    bool mapIsInit();
    void octogonCheckBoundary(vector<vector<int>>& octoPath, vector<int>& sourceDim, int pathSpace);
    vector<vector<int>> octogonPath(vector<int> origin, int radius);
    void pauseMapDebug(SWITCHBOARD& sbgui, vector<vector<int>>& tracks, int radius, vector<vector<int>>& candidates);
    string pixelDecToHex(vector<unsigned char>& rgb);
	void pixelPaint(vector<unsigned char>& img, int widthImg, vector<unsigned char> rgb, vector<int> coord);
	vector<unsigned char> pixelRGB(vector<int>& coord);
	string pixelZone(vector<unsigned char>& rgb);
    vector<unsigned char> pngBlankCanvas(vector<int>& dim);
    vector<unsigned char> pngExtractRow(int row, vector<unsigned char>& img, vector<int>& sourceDim);
	void pngLoad(string& pathPNG);
    void pngToBin(SWITCHBOARD& sbgui, string& pathPNG, string& pathBIN);
    void pngToBinLive(SWITCHBOARD& sbgui, vector<vector<double>>& border);
    void pngToBinPause(SWITCHBOARD& sbgui);
    void removeColourCushion(vector<vector<unsigned char>>& Lrgb, vector<unsigned char> colourCore, vector<unsigned char> colourCushion, int length);
    void saveThisPoint(vector<vector<int>>& tracks, vector<vector<int>>& candidates);
    void setExtractDim(vector<int> extractDim);
    void setPauseVBP(int iLen);
    int testBacktrack(vector<vector<int>>& tracks, vector<vector<int>>& candidates);
    void testDistances(vector<vector<int>>& candidates, vector<double>& distances);
    int testCandidatesInteriorZone(SWITCHBOARD& sbgui, vector<vector<int>>& tracks, string sZone, vector<vector<int>>& candidates);
    void testCenterOfMass(vector<vector<int>>& tracks, vector<vector<int>>& candidates);
    int testHereBeDragons(vector<vector<unsigned char>> Lrgb, vector<int>& borderCoord, unordered_map<string, int>& mapIndexCandidate);
    int testOverWater(vector<vector<int>>& tracks, vector<vector<int>>& candidates);
    int testTextHumanFeature(vector<vector<unsigned char>>& Lrgb);
    vector<int> testZoneLength(vector<vector<int>>& pastPresent, vector<vector<int>>& candidates, string sZone);
    int testZoneSweepLetters(vector<vector<int>>& zonePath, vector<vector<unsigned char>>& Lrgb, vector<vector<int>>& candidates, unordered_map<string, int>& mapIndexCandidate);
    void thrMakeMapPTB(vector<vector<int>> vBorderPath, string outputPath, vector<unsigned char> source, vector<int> sourceDim);
    //void thrMakeMapPTBdebug(vector<unsigned char>& source, vector<vector<int>> tracks, int radius, vector<vector<int>> candidates, string pathImg);
    vector<vector<int>> zoneChangeLinear(vector<string>& szones, vector<vector<int>>& ivec);
    double zoneSweepPercentage(string sZone, vector<vector<unsigned char>>& Lrgb);

	// TEMPLATES

    template<typename ... Args> void appendMapLegend(vector<vector<unsigned char>>& source, vector<int>& sourceDim, vector<string>& sBlocks, Args& ... args)
    {
        jf.err("appendMapLegend template-im");
    }
    template<> void appendMapLegend<vector<vector<unsigned char>>>(vector<vector<unsigned char>>& source, vector<int>& sourceDim, vector<string>& sBlocks, vector<vector<unsigned char>>& colourList)
    {
        if (sBlocks.size() != colourList.size()) { jf.err("Text/colour mismatch-im.appendMapLegend"); }
        if (!mapIsInit()) { initMapColours(); }

        // Determine the sizes needed.
        int charSpace = defaultCharSpace;
        int textSeparatorWidth = defaultTextSeparatorWidth;
        int numLines = 1;
        int charWidth, ascii, maxWidth;
        vector<int> blockWidth(sBlocks.size(), 0);
        vector<int> textWidth(sBlocks.size(), 0);
        vector<int> lineWidth = { 0 };  // Form [width of first line, width of second line, ...]. 
        vector<int> blocksPerLine = { 0 };
        for (int ii = 0; ii < sBlocks.size(); ii++)  // For each block...
        {
            blockWidth[ii] += textSeparatorWidth;
            blockWidth[ii] += fontHeight;  // Colour box.
            textWidth[ii] += charSpace;
            for (int jj = 0; jj < sBlocks[ii].size(); jj++)
            {
                try { charWidth = mapFontWidth.at(sBlocks[ii][jj]); }
                catch (out_of_range& oor) { jf.err("mapFontWidth-im.makeText"); }
                textWidth[ii] += charWidth + charSpace;
            }
            blockWidth[ii] += textWidth[ii];
            blockWidth[ii] += textSeparatorWidth;

            if (blockWidth[ii] + lineWidth[numLines - 1] < sourceDim[0])
            {
                lineWidth[numLines - 1] += blockWidth[ii];
                blocksPerLine[numLines - 1]++;
            }
            else
            {
                numLines++;
                lineWidth.push_back(blockWidth[ii]);
                blocksPerLine.push_back(1);
            }
        }

        // Make the image to be inserted.
        vector<unsigned char> imgText;
        vector<vector<int>> startStop(2, vector<int>(2, 0));
        vector<int> topLeft, boxDim, pasteDim;
        pasteDim.assign(2, fontHeight);
        int blockIndex = 0;
        int thickness = 3;
        vector<int> legDim = { sourceDim[0], numLines * fontHeight };
        vector<unsigned char> legend = pngBlankCanvas(boxDim);
        for (int ii = 0; ii < numLines; ii++)
        {
            startStop[0][0] += textSeparatorWidth / 2;
            startStop[1][0] += textSeparatorWidth / 2;
            startStop[1][1] += fontHeight;
            for (int jj = 0; jj < blocksPerLine[ii]; jj++)
            {
                linePaint(startStop, Black, legend, legDim[0], textSeparatorWidth);
                topLeft = { startStop[0][0] + (textSeparatorWidth / 2), startStop[0][1] };
                topLeft[0] += 8;
                topLeft[1] += 8;
                boxDim = { 16, 16 };
                rectanglePaint(topLeft, boxDim, Black, legend, legDim, thickness, colourList[blockIndex]);
                topLeft[0] += fontHeight - 8;
                topLeft[1] -= 8;
                imgText = makeTextLine(sBlocks[blockIndex]);
                pasteDim[0] = textWidth[blockIndex];
                pngPaste(legend, legDim, imgText, pasteDim, topLeft);
                startStop[0][0] = topLeft[0] + pasteDim[0] + 8;
                startStop[1][0] = startStop[0][0];
                linePaint(startStop, Black, legend, legDim[0], textSeparatorWidth);
                startStop[0][0] += textSeparatorWidth;
                startStop[1][0] += textSeparatorWidth;
                blockIndex++;
            }
            startStop[0][0] = 0;
            startStop[0][1] += fontHeight;
            startStop[1][0] = 0;
            startStop[1][1] += fontHeight;
        }

        // Paste atop the source image. 
        vector<int> oldNew = { sourceDim[0], sourceDim[1], sourceDim[0], sourceDim[1] + (numLines * fontHeight) };
        pngExtend(source, oldNew);
        vector<int> coordPaste = { 0, sourceDim[1] };
        pngPaste(source, sourceDim, legend, legDim, coordPaste);
        int bbq = 1;
    }

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
            if (listCoord[ii].size() != 2)
            {
                jf.err("Not 2 coordinates-jf.coordShift");
            }
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

    template<typename ... Args> void deadConePaint(vector<vector<unsigned char>>& Lrgb, vector<int>& deadStartStop, Args& ... args)
    {
        jf.err("deadConePaint template-im");
    }
    template<> void deadConePaint< >(vector<vector<unsigned char>>& Lrgb, vector<int>& deadStartStop)
    {
        if (Lrgb.size() < 1 || deadStartStop.size() < 1) { jf.err("parameters empty-im.deadConePaint"); }
        int numCones = deadStartStop.size() / 2;
        int index, imax = Lrgb.size();
        for (int ii = 0; ii < numCones; ii++)
        {
            index = deadStartStop[ii * 2];
            while (index != deadStartStop[(ii * 2) + 1])
            {
                Lrgb[index] = Blue;  // Blue is arbitrary.
                index = (index + 1) % imax;
            }
        }
    }

    template<typename ... Args> void deadConeText(vector<int>& origin, Args& ... args)
    {
        jf.err("deadConeText template-im");
    }
    template<> void deadConeText<vector<vector<int>>, vector<int>>(vector<int>& origin, vector<vector<int>>& octoPath, vector<int>& deadStartStop)
    {
        int numCones = textFound.size();
        if (numCones < 1) { jf.err("textFound out of bounds-im.deadConeText"); }
        deadStartStop.clear();
        vector<unsigned char> colourText = { 38, 115, 0 };
        vector<vector<int>> pPF(3, vector<int>(2));
        vector<int> pixelFound;
        int currentIndex;
        double starterAngle, minAngle, currentAngle, angleDiff, maxAngle;
        for (int ii = 0; ii < numCones; ii++)
        {
            pPF[0] = { origin[0], origin[1] + defaultSearchRadius };  // Points north.
            pPF[1] = origin;
            pPF[2] = octoPath[textFound[ii]];
            starterAngle = jf.angleBetweenVectors(pPF);  // [0.0, 360.0)

            minAngle = starterAngle;
            do
            {
                minAngle -= defaultAngleIncrement;
                if (minAngle < 0.0) { minAngle += 360.0; }
                lineScan(colourText, origin, minAngle, pixelFound);
            } while (pixelFound[0] >= 0);
            minAngle += defaultAngleIncrement;  // Angle represents last pixel found.
            if (minAngle >= 360.0) { minAngle -= 360.0; }
            angleDiff = 360.0;
            currentIndex = textFound[ii];
            while (1)
            {
                pPF[2] = octoPath[currentIndex];
                currentAngle = jf.angleBetweenVectors(pPF);
                if (abs(currentAngle - minAngle) < angleDiff)
                {
                    angleDiff = abs(currentAngle - minAngle);
                    currentIndex--;
                    if (currentIndex < 0) { currentIndex = octoPath.size() - 1; }
                }
                else { break; }
            }
            currentIndex -= 7;
            if (currentIndex < 0) { currentIndex = octoPath.size() - 1 + currentIndex; }
            deadStartStop.push_back(currentIndex);

            maxAngle = starterAngle;
            do
            {
                maxAngle += defaultAngleIncrement;
                if (maxAngle >= 360.0) { maxAngle -= 360.0; }
                lineScan(colourText, origin, maxAngle, pixelFound);
            } while (pixelFound[0] >= 0);
            maxAngle -= defaultAngleIncrement;
            if (maxAngle < 0.0) { maxAngle += 360.0; }
            angleDiff = 360.0;
            currentIndex = textFound[ii];
            while (1)
            {
                pPF[2] = octoPath[currentIndex];
                currentAngle = jf.angleBetweenVectors(pPF);
                if (abs(currentAngle - maxAngle) < angleDiff)
                {
                    angleDiff = abs(currentAngle - maxAngle);
                    currentIndex++;
                    if (currentIndex >= octoPath.size()) { currentIndex = 0; }
                }
                else { break; }
            }
            currentIndex += 7;
            if (currentIndex >= octoPath.size()) { currentIndex -= octoPath.size(); }
            deadStartStop.push_back(currentIndex);
        }
        if (debug) { makeMapDeadCone(origin, octoPath, deadStartStop); }
        textFound.clear();
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

    template<typename ... Args> vector<vector<unsigned char>> getColourSpectrum(int numColours, Args& ... args)
    {
        // Returns a list of RGB values representing the EM spectrum (Red->Violet).
        
        // When a base colour is specified, avoid it by printing in black instead.
        jf.err("getColourSpectrum template-im");
    }
    template<> vector<vector<unsigned char>> getColourSpectrum< >(int numColours)
    {
        vector<vector<unsigned char>> colours(numColours, vector<unsigned char>(3));
        int sizeBand = numColours / 5;  // Red->Yellow->Green->Teal->Blue->Violet.
        colours[0] = { 255, 0, 0 };
        for (int ii = 1; ii < sizeBand; ii++)
        {
            colours[ii] = colours[ii - 1];
            colours[ii][1] += 255 / sizeBand;
        }
        colours[1 * sizeBand] = { 255, 255, 0 };
        for (int ii = sizeBand + 1; ii < 2 * sizeBand; ii++)
        {
            colours[ii] = colours[ii - 1];
            colours[ii][0] -= 255 / sizeBand;
        }
        colours[2 * sizeBand] = { 0, 255, 0 };
        for (int ii = (2 * sizeBand) + 1; ii < 3 * sizeBand; ii++)
        {
            colours[ii] = colours[ii - 1];
            colours[ii][2] += 255 / sizeBand;
        }
        colours[3 * sizeBand] = { 0, 255, 255 };
        for (int ii = (3 * sizeBand) + 1; ii < 4 * sizeBand; ii++)
        {
            colours[ii] = colours[ii - 1];
            colours[ii][1] -= 255 / sizeBand;
        }
        colours[4 * sizeBand] = { 0, 0, 255 };
        for (int ii = (4 * sizeBand) + 1; ii < 5 * sizeBand; ii++)
        {
            colours[ii] = colours[ii - 1];
            colours[ii][0] += 255 / sizeBand;
        }
        colours[numColours - 1] = { 255, 0, 255 };
        return colours;
    }
    template<> vector<vector<unsigned char>> getColourSpectrum<string>(int numColours, string& sColour)
    {
        vector<string> keyColours = { "Red", "Yellow", "Green", "Teal", "Blue", "Pink" };
        if (numColours < 0)
        {
            vector<string> vsTemp = keyColours;
            int inum = keyColours.size();
            for (int ii = 0; ii < inum; ii++)
            {
                keyColours[ii] = vsTemp[inum - 1 - ii];
            }
            numColours *= -1;
        }
        for (int ii = 0; ii < keyColours.size(); ii++)
        {
            if (keyColours[ii] == sColour)
            {
                keyColours[ii] = "Black";
                break;
            }
            else if (ii == keyColours.size() - 1) { jf.err("unknown colour to avoid-im.getColourSpectrum"); }
        }
        double deltaR, deltaG, deltaB, dR, dG, dB;
        int offset;
        int sizeBand = max(numColours / 5, 1);  // ->Green->Teal->Blue->Violet.
        double dSB = (double)sizeBand;
        vector<unsigned char> startColour, stopColour;
        vector<vector<unsigned char>> colours(numColours, vector<unsigned char>(3));
        int footprints = min(numColours, 5);
        for (int ii = 0; ii < footprints; ii++)
        {
            offset = sizeBand * ii;
            startColour = getColour(keyColours[ii]);
            dR = (double)startColour[0];
            dG = (double)startColour[1];
            dB = (double)startColour[2];
            stopColour = getColour(keyColours[ii + 1]);
            deltaR = ((double)stopColour[0] - dR) / dSB;
            deltaG = ((double)stopColour[1] - dG) / dSB;
            deltaB = ((double)stopColour[2] - dB) / dSB;
            colours[offset] = startColour;
            for (int jj = offset + 1; jj < offset + sizeBand; jj++)
            {
                dR += deltaR;
                dG += deltaG;
                dB += deltaB;
                colours[jj] = { (unsigned char)round(dR), (unsigned char)round(dG), (unsigned char)round(dB) };
            }
        }
        if (stopColour != Black)  // This is to correct a rounding
        {                         // error causing 255->0.
            int index = colours.size() - 1;
            while (colours[index] == Black)
            {
                colours[index] = stopColour;
                index--;
            }
        }
        return colours;
    }
    template<> vector<vector<unsigned char>> getColourSpectrum<vector<string>>(int numColours, vector<string>& colourStartStop)
    {
        vector<string> keyColours = { "Red", "Yellow", "Green", "Teal", "Blue", "Pink" };
        if (numColours == 0) { jf.err("numColours cannot be zero"); }
        int sizeKey = keyColours.size(), inum;
        if (numColours < 0)
        {
            vector<string> vsTemp = keyColours;
            for (int ii = 0; ii < sizeKey; ii++)
            {
                keyColours[ii] = vsTemp[sizeKey - 1 - ii];
            }
            numColours *= -1;
        }
        for (int ii = sizeKey - 1; ii > 0; ii--)
        {
            if (keyColours[ii] == colourStartStop[1])
            {
                inum = ii + 1;
                keyColours.erase(keyColours.begin() + inum, keyColours.end());
            }
            else if (keyColours[ii] == colourStartStop[0])
            {
                inum = ii;
                keyColours.erase(keyColours.begin(), keyColours.begin() + inum);
                break;
            }
        }
        sizeKey = keyColours.size() - 1;
        if (sizeKey < 1) { jf.err("keyColours is too small"); }
        double deltaR, deltaG, deltaB, dR, dG, dB;
        int offset;
        int sizeBand = max(numColours / sizeKey, 1);
        double dSB = (double)sizeBand;
        vector<unsigned char> startColour, stopColour;
        vector<vector<unsigned char>> colours(numColours, vector<unsigned char>(3));
        int numBands = min(numColours, sizeKey);
        for (int ii = 0; ii < numBands; ii++)
        {
            offset = sizeBand * ii;
            startColour = getColour(keyColours[ii]);
            dR = (double)startColour[0];
            dG = (double)startColour[1];
            dB = (double)startColour[2];
            stopColour = getColour(keyColours[ii + 1]);
            deltaR = ((double)stopColour[0] - dR) / dSB;
            deltaG = ((double)stopColour[1] - dG) / dSB;
            deltaB = ((double)stopColour[2] - dB) / dSB;
            colours[offset] = startColour;
            for (int jj = offset + 1; jj < offset + sizeBand; jj++)
            {
                dR += deltaR;
                dG += deltaG;
                dB += deltaB;
                colours[jj] = { (unsigned char)round(dR), (unsigned char)round(dG), (unsigned char)round(dB) };
            }
        }
        if (stopColour != Black)  // This is to correct a rounding
        {                         // error causing 255->0.
            int index = colours.size() - 1;
            while (colours[index] == Black)
            {
                colours[index] = stopColour;
                index--;
            }
        }
        return colours;
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

    template<typename ... Args> void lineScan(vector<unsigned char> rgb, Args& ... args)
    {
        // NOTE: Image coordinates use a reversed y-axis (positive points down) !
        //       7
        //     2 | 3
        //   6---+---4      <--- Quadrant diagram.
        //     1 | 0
        //       5
        jf.err("lineScan template-im");
    }
    template<> void lineScan<vector<int>, double, vector<int>>(vector<unsigned char> rgb, vector<int>& origin, double& angle, vector<int>& coord)
    {
        double xCoord = (double)origin[0];
        double yCoord = (double)origin[1];
        int maxRadius = 4 * defaultSearchRadius;
        vector<unsigned char> rgbTemp;
        for (int radius = 1; radius <= maxRadius; radius++)
        {
            jf.coordOnCircle(origin, radius, angle, coord);
            rgbTemp = pixelRGB(coord);
            if (rgbTemp == rgb) 
            {
                return; 
            }
        }
        coord = { -1, -1 };
    }

    template<typename ... Args> void makeLegendV(vector<unsigned char>& legend, vector<int>& legendDim, vector<string>& listText, Args& ... args)
    {
        jf.err("makeLegendV template-im");
    }
    template<> void makeLegendV<vector<vector<unsigned char>>>(vector<unsigned char>& legend, vector<int>& legendDim, vector<string>& listText, vector<vector<unsigned char>>& listColour)
    {
        int charSpace = defaultCharSpace;
        int textSeparatorWidth = defaultTextSeparatorWidth;
        int inum, charWidth, ascii, maxWidth, maxWidthText;
        vector<int> blockWidth(listText.size(), 0);
        vector<int> textWidth(listText.size(), 0);
        maxWidth = -1;
        maxWidthText = -1;
        for (int ii = 0; ii < blockWidth.size(); ii++)  // For each block...
        {
            blockWidth[ii] += fontHeight;
            textWidth[ii] += charSpace;
            for (int jj = 0; jj < listText[ii].size(); jj++)
            {
                try { charWidth = mapFontWidth.at(listText[ii][jj]); }
                catch (out_of_range& oor) { jf.err("mapFontWidth-im.makeText"); }
                textWidth[ii] += charWidth + charSpace;
            }
            blockWidth[ii] += textWidth[ii];
            if (blockWidth[ii] > maxWidth) { maxWidth = blockWidth[ii]; }
            if (textWidth[ii] > maxWidthText) { maxWidthText = textWidth[ii]; }
        }

        inum = listText.size() * fontHeight;
        legendDim = { maxWidth, inum };
        legend = pngBlankCanvas(legendDim);
        vector<int> topLeft, boxDim, pasteTL;
        topLeft.assign(2, 8);
        boxDim.assign(2, 16);
        pasteTL.assign(2, fontHeight);
        vector<int> pasteDim = { maxWidthText, fontHeight };
        int thickness = 3;
        vector<unsigned char> textLine;
        for (int ii = 0; ii < listText.size(); ii++)
        {
            topLeft[1] = 8 + (ii * fontHeight);
            rectanglePaint(topLeft, boxDim, Black, legend, legendDim, thickness, listColour[ii]);
            textLine = makeTextLine(listText[ii], maxWidthText);
            pasteTL[1] = ii * fontHeight;
            pngPaste(legend, legendDim, textLine, pasteDim, pasteTL);
        }        
    }

    template<typename ... Args> void makeMapBorderFindNext(vector<vector<int>>& tracks, int radius, vector<vector<int>> candidates, Args& ... args)
    {
        // Debug map maker for the 'borderFindNext' function. 
        jf.err("makeMapBorderFindNext template-im");
    }
    template<> void makeMapBorderFindNext< >(vector<vector<int>>& tracks, int radius, vector<vector<int>> candidates)
    {
        string pathImg = sroot + "\\debug\\borderFindNextDebug.png";
        makeMapBorderFindNext(tracks, radius, candidates, pathImg);
    }
    template<> void makeMapBorderFindNext<string>(vector<vector<int>>& tracks, int radius, vector<vector<int>> candidates, string& pathImg)
    {
        if (tracks.size() < 1) { jf.err("No tracks-im.makeMapBorderFindNext"); }
        if (!mapIsInit()) { initMapColours(); }
        debugDataPNG = dataPNG;
        vector<string> listText, colourStartStop;
        vector<vector<unsigned char>> listColour;

        // Paint on the full image. 
        if (radius > 0) { octogonPaint(Orange, tracks[tracks.size() - 1], radius); }
        octogonPaint(Gold, tracks[tracks.size() - 1], defaultSearchRadius);
        int lineThickness = 2;
        vector<vector<int>> startStop(2, vector<int>());
        for (int ii = 0; ii < tracks.size() - 1; ii++)
        {
            startStop[0] = tracks[ii];
            startStop[1] = tracks[ii + 1];
            linePaint(startStop, Black, debugDataPNG, width, lineThickness);
        }
        int numColours = tracks.size() - 1;
        colourStartStop = { "Pink", "Yellow" };
        listColour = getColourSpectrum(-1 * numColours, colourStartStop);
        for (int ii = 0; ii < numColours; ii++)
        {
            dotPaint(tracks[ii], listColour[ii]);
            pngAppendText(listText, tracks[ii]);
        }
        dotPaint(tracks[tracks.size() - 1], Orange);
        pngAppendText(listText, tracks[tracks.size() - 1]);
        listColour.push_back(Orange);
        for (int ii = 0; ii < candidates.size(); ii++)
        {
            dotPaint(candidates[ii], Red);
            pngAppendText(listText, candidates[ii]);
            listColour.push_back(Red);
        }

        // Switch to the cropped image, and add the legend.
        vector<int> sourceDim = { width, height };
        int smallDim = min(defaultExtractDim[0], defaultExtractDim[1]);
        int largeDim = max(defaultExtractDim[0], defaultExtractDim[1]);
        vector<int> croppedDim = { smallDim, smallDim };
        vector<unsigned char> cropped = pngExtractRect(tracks[tracks.size() - 1], debugDataPNG, sourceDim, croppedDim);
        vector<unsigned char> legendImg;
        vector<int> legendDim;
        makeLegendV(legendImg, legendDim, listText, listColour);
        int xDim = min(smallDim + legendDim[0], largeDim);
        vector<int> oldNew = { croppedDim[0], croppedDim[1], xDim, croppedDim[1] };
        pngExtend(cropped, oldNew);
        croppedDim = { oldNew[2], oldNew[3] };
        vector<int> coordPaste = { croppedDim[0] - legendDim[0], 0 };
        pngPaste(cropped, croppedDim, legendImg, legendDim, coordPaste);

        // Print the cropped image to file.
        int imgSize = cropped.size();
        int channels = 3;
        auto bufferUC = new unsigned char[imgSize];
        for (int ii = 0; ii < imgSize; ii++)
        {
            bufferUC[ii] = cropped[ii];
        }
        int error = stbi_write_png(pathImg.c_str(), croppedDim[0], croppedDim[1], channels, bufferUC, 0);
        delete[] bufferUC;

        // Print the cropped image's top-left, origin, and candidate coordinates.
        string pathBin = pathImg.substr(0, pathImg.size() - 4);
        pathBin += ".bin";
        ofstream sPrinter(pathBin.c_str(), ios::trunc);
        auto report = sPrinter.rdstate();
        vector<int> viTemp(2);
        viTemp[0] = tracks[tracks.size() - 1][0] - (oldNew[0] / 2);
        viTemp[1] = tracks[tracks.size() - 1][1] - (oldNew[1] / 2);
        sPrinter << "//topleft" << endl;
        sPrinter << to_string(viTemp[0]) << "," << to_string(viTemp[1]) << endl << endl;
        sPrinter << "//origin" << endl;
        sPrinter << to_string(tracks[tracks.size() - 1][0]) << "," << to_string(tracks[tracks.size() - 1][1]) << endl << endl;
        sPrinter << "//candidate" << endl;
        for (int ii = 0; ii < candidates.size(); ii++)
        {
            sPrinter << to_string(candidates[ii][0]) << "," << to_string(candidates[ii][1]) << endl;
        }
        sPrinter << endl;
        sPrinter.close();
    }

    template<typename ... Args> void makeMapDeadCone(Args& ... args)
    {
        jf.err("makeMapDeadCone template-im");
    }
    template<> void makeMapDeadCone<vector<int>, vector<vector<int>>, vector<int>>(vector<int>& origin, vector<vector<int>>& octoPath, vector<int>& deadStartStop)
    {
        debugDataPNG = dataPNG;
        vector<string> listStartStop;
        vector<vector<unsigned char>> listColour;
        vector<vector<int>> ringOuter, ringInner;
        vector<int> sourceDim = { width, height };
        int widthDot = 1, pathSpace = 1, indexStart, indexStop, inum;
        octogonPaint(Black, debugDataPNG, sourceDim, widthDot, octoPath, pathSpace);
        pathSpace = -1;
        octogonPaint(Black, debugDataPNG, sourceDim, widthDot, octoPath, pathSpace);
        vector<vector<int>> startStop(2, vector<int>(2));
        startStop[0] = origin;
        for (int ii = 0; ii < deadStartStop.size(); ii++)
        {
            startStop[1] = octoPath[deadStartStop[ii]];
            linePaint(startStop, Orange, debugDataPNG, sourceDim[0], widthDot);
            if (ii % 2 == 0)
            {
                indexStart = deadStartStop[ii] + 1;
                if (indexStart >= octoPath.size()) { indexStart -= octoPath.size(); }
                indexStop = deadStartStop[ii + 1];
                while (indexStart != indexStop)
                {
                    pixelPaint(debugDataPNG, sourceDim[0], Blue, octoPath[indexStart]);
                    indexStart++;
                    if (indexStart >= octoPath.size()) { indexStart -= octoPath.size(); }
                }
            }
        }
        for (int ii = 0; ii < deadStartStop.size(); ii++)
        {
            startStop[1] = octoPath[deadStartStop[ii]];
            pngAppendText(listStartStop, startStop[1]);
            listColour.push_back(Red);
            pixelPaint(debugDataPNG, sourceDim[0], Red, startStop[1]);
        }
        pixelPaint(debugDataPNG, sourceDim[0], Yellow, origin);
        pngAppendText(listStartStop, origin);
        listColour.push_back(Yellow);
        vector<unsigned char> legend;
        vector<int> legendDim;
        makeLegendV(legend, legendDim, listStartStop, listColour);
        inum = ((origin[1] - octoPath[0][1]) * 2) + 40;
        vector<int> croppedDim(2);
        croppedDim[1] = max(legendDim[1], inum);
        croppedDim[0] = croppedDim[1] + legendDim[0];
        vector<int> originTilted = { origin[0] + (legendDim[0] / 2), origin[1] };
        vector<unsigned char> cropped = pngExtractRect(originTilted, debugDataPNG, sourceDim, croppedDim);
        vector<int> pasteTL = { croppedDim[0] - legendDim[0], 0 };
        pngPaste(cropped, croppedDim, legend, legendDim, pasteTL);
        string filePath = sroot + "\\debug\\Dead Cone Debug.png";
        pngPrint(cropped, croppedDim, filePath);
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
        // pPF has form [past, origin, candidate0, candidate1, ...].
        if (!mapIsInit()) { initMapColours(); }
        debugDataPNG = dataPNG;
        int widthDot = 5;
        int widthLine = 5;
        vector<vector<int>> startStop(2, vector<int>());
        startStop[0] = pastPresentFuture[0];
        startStop[1] = pastPresentFuture[1];
        linePaint(startStop, Teal, debugDataPNG, width, widthLine);
        startStop[0] = pastPresentFuture[1];
        for (int ii = 2; ii < pastPresentFuture.size(); ii++)
        {
            startStop[1] = pastPresentFuture[ii];
            linePaint(startStop, Orange, debugDataPNG, width, widthLine);
        }
        dotPaint(pastPresentFuture[0], Green, debugDataPNG, width, widthDot);
        dotPaint(pastPresentFuture[1], Yellow, debugDataPNG, width, widthDot);
        for (int ii = 2; ii < pastPresentFuture.size(); ii++)
        {
            dotPaint(pastPresentFuture[ii], Red, debugDataPNG, width, widthDot);
        }
        vector<int> sourceDim = { width, height };
        vector<unsigned char> cropped = pngExtractRect(pastPresentFuture[1], debugDataPNG, sourceDim, defaultExtractDim);
        int imgSize = cropped.size();
        int channels = 3;
        auto bufferUC = new unsigned char[imgSize];
        for (int ii = 0; ii < imgSize; ii++)
        {
            bufferUC[ii] = cropped[ii];
        }
        string pathImg = sroot + "\\debug\\OctogonBearingDebug.png";
        int error = stbi_write_png(pathImg.c_str(), defaultExtractDim[0], defaultExtractDim[1], channels, bufferUC, 0);
        delete[] bufferUC;
    }
    template<> void makeMapOctogonBearing<vector<vector<double>>>(vector<vector<double>>& pPF)
    {
        vector<vector<int>> pastPresentFuture;
        jf.toInt(pPF, pastPresentFuture);
        makeMapOctogonBearing(pastPresentFuture);
    }

    template<typename ... Args> void makeMapPngToBin(vector<vector<int>>& vBorderPath, Args& ... args)
    {
        jf.err("makeMapPngToBin template-im");
    }
    template<> void makeMapPngToBin< >(vector<vector<int>>& vBorderPath)
    {
        string outputPath = sroot + "\\debug\\pngToBin Debug.png";
        makeMapPngToBin(vBorderPath, outputPath);
    }
    template<> void makeMapPngToBin<string>(vector<vector<int>>& vBorderPath, string& outputPath)
    {
        debugDataPNG = dataPNG;  // Note that the bands are Red->Yellow->Green,
        int widthDot = 5;        // Green->Teal->Blue->Violet
        int numDots = vBorderPath.size();
        vector<int> sourceDim = { width, height };
        string avoidColour = "Blue";
        vector<vector<unsigned char>> colours = getColourSpectrum(numDots, avoidColour);
        for (int ii = 0; ii < numDots; ii++)
        {
            dotPaint(vBorderPath[ii], colours[ii], debugDataPNG, sourceDim[0], widthDot);
        }
        pngPrint(debugDataPNG, sourceDim, outputPath);
    }

    template<typename ... Args> void makeMapshift(vector<int> windowDim, Args& ... args)
    {
        jf.err("makeMapshift template-im");
    }
    template<> void makeMapshift<vector<vector<double>>, vector<double>>(vector<int> windowDim, vector<vector<double>>& frame, vector<double>& DxDyGa)
    {
        DxDyGa.resize(3);
        DxDyGa[0] = -1.0 * frame[0][0];
        DxDyGa[1] = -1.0 * frame[0][1];

        double widthWindow = (double)windowDim[0];
        double heightWindow = (double)windowDim[1];
        double widthImg, heightImg;
        if (frame.size() == 2)
        {
            widthImg = frame[1][0] - frame[0][0];
            heightImg = frame[1][1] - frame[0][1];
        }
        else if (frame.size() == 4)
        {
            widthImg = frame[2][0] - frame[0][0];
            heightImg = frame[2][1] - frame[0][1];
        }
        else { jf.err("frame input-im.makeMapshift"); }

        if (widthImg < widthWindow || heightImg < heightWindow)
        {
            DxDyGa[2] = 1.0;
            return;
        }

        double xRatio = widthWindow / widthImg;
        double yRatio = heightWindow / heightImg;
        if (xRatio > yRatio)
        {
            DxDyGa[2] = yRatio;
        }
        else
        {
            DxDyGa[2] = xRatio;
        }
        return;
    }

    template<typename ... Args> void makeMapZoneSweep(vector<vector<int>>& zonePath, Args& ... args)
    {
        jf.err("makeMapZoneSweepDebug template.im");
    }
    template<> void makeMapZoneSweep< >(vector<vector<int>>& zonePath)
    {
        if (!mapIsInit()) { initMapColours(); }
        debugDataPNG = dataPNG;
        vector<vector<int>> ringOuter, ringInner;
        vector<int> sourceDim = { width, height };
        int widthDot = 1, pathSpace = 1;
        octogonPaint(Black, debugDataPNG, sourceDim, widthDot, zonePath, pathSpace);
        pathSpace = -1;
        octogonPaint(Black, debugDataPNG, sourceDim, widthDot, zonePath, pathSpace);
        int imgSize = debugDataPNG.size();
        int channels = 3;
        auto bufferUC = new unsigned char[imgSize];
        for (int ii = 0; ii < imgSize; ii++)
        {
            bufferUC[ii] = debugDataPNG[ii];
        }
        string pathImg = sroot + "\\debug\\ZoneSweepDebug.png";
        int error = stbi_write_png(pathImg.c_str(), sourceDim[0], sourceDim[1], channels, bufferUC, 0);
        delete[] bufferUC;
    }
    template<> void makeMapZoneSweep<vector<vector<unsigned char>>>(vector<vector<int>>& zonePath, vector<vector<unsigned char>>& Lrgb)
    {
        if (zonePath.size() != Lrgb.size()) { jf.err("parameter size mismatch-im.makeMapZoneSweep"); }
        if (!mapIsInit()) { initMapColours(); }
        debugDataPNG = dataPNG;
        vector<vector<int>> ringOuter, ringInner;
        vector<int> sourceDim = { width, height };
        int widthDot = 1, pathSpace = 1;
        octogonPaint(Black, debugDataPNG, sourceDim, widthDot, zonePath, pathSpace);
        pathSpace = -1;
        octogonPaint(Black, debugDataPNG, sourceDim, widthDot, zonePath, pathSpace);
        for (int ii = 0; ii < zonePath.size(); ii++)
        {
            pixelPaint(debugDataPNG, sourceDim[0], Lrgb[ii], zonePath[ii]);
        }
        int imgSize = debugDataPNG.size();
        int channels = 3;
        auto bufferUC = new unsigned char[imgSize];
        for (int ii = 0; ii < imgSize; ii++)
        {
            bufferUC[ii] = debugDataPNG[ii];
        }
        string pathImg = sroot + "\\debug\\ZoneSweepDebug.png";
        int error = stbi_write_png(pathImg.c_str(), sourceDim[0], sourceDim[1], channels, bufferUC, 0);
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
        // Break the text into a vector of strings, corresponding to output blocks.
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

        // Determine the width (in pixels) needed by each block, and how many lines will be needed. 
        bool colourBox = 0;
        if (listColour.size() > 0) { colourBox = 1; }
        int charSpace = defaultCharSpace;
        int textSeparatorWidth = defaultTextSeparatorWidth;
        int numLines = 1;
        int charWidth, ascii, maxWidth;
        vector<int> blockWidth(vText.size(), 0);
        vector<int> textWidth(vText.size(), 0);
        vector<int> lineWidth = { 0 };  // Form [width of first line, width of second line, ...]. 
        vector<int> blocksPerLine = { 0 };
        imgDim.resize(2);
        if (imgDim[0] > 0) { maxWidth = imgDim[0]; }  // If more width is needed past this, text wrap to new line.
        else { maxWidth = 2147483647; }
        for (int ii = 0; ii < blockWidth.size(); ii++)  // For each block...
        {
            blockWidth[ii] += textSeparatorWidth;
            if (colourBox)
            {
                blockWidth[ii] += fontHeight;
            }
            textWidth[ii] += charSpace;
            for (int jj = 0; jj < vText[ii].size(); jj++)
            {
                try { charWidth = mapFontWidth.at(vText[ii][jj]); }
                catch (out_of_range& oor) { jf.err("mapFontWidth-im.makeText"); }
                textWidth[ii] += charWidth + charSpace;
            }
            blockWidth[ii] += textWidth[ii];
            blockWidth[ii] += textSeparatorWidth;

            if (blockWidth[ii] > maxWidth) { jf.err("Single block wider than line-im.makeText"); }
            if (blockWidth[ii] + lineWidth[numLines - 1] < maxWidth)
            {
                lineWidth[numLines - 1] += blockWidth[ii];
                blocksPerLine[numLines - 1]++;
            }
            else
            {
                numLines++;
                lineWidth.push_back(blockWidth[ii]);
                blocksPerLine.push_back(1);
            }
        }
        if (imgDim[0] < 1) { imgDim[0] = lineWidth[0]; }
        imgDim[1] = fontHeight * numLines;

        // Build the image by pasting blocks.
        vector<unsigned char> imgText;
        vector<vector<int>> startStop(2, vector<int>(2, 0));
        vector<int> topLeft, boxDim, pasteDim;
        pasteDim.assign(2, fontHeight);
        int blockIndex = 0;
        int thickness = 3;
        img = pngBlankCanvas(imgDim);
        for (int ii = 0; ii < numLines; ii++)
        {
            startStop[0][0] += textSeparatorWidth / 2;
            startStop[1][0] += textSeparatorWidth / 2;
            startStop[1][1] += fontHeight;
            for (int jj = 0; jj < blocksPerLine[ii]; jj++)
            {
                linePaint(startStop, Black, img, imgDim[0], textSeparatorWidth);
                topLeft = { startStop[0][0] + (textSeparatorWidth / 2), startStop[0][1] };
                if (colourBox)
                {
                    topLeft[0] += 8;
                    topLeft[1] += 8; 
                    boxDim = { 16, 16 };
                    rectanglePaint(topLeft, boxDim, Black, img, imgDim, thickness, listColour[jj]);
                    topLeft[0] += fontHeight - 8;
                    topLeft[1] -= 8;
                }
                imgText = makeTextLine(vText[blockIndex]);
                pasteDim[0] = textWidth[blockIndex];
                pngPaste(img, imgDim, imgText, pasteDim, topLeft);
                startStop[0][0] = topLeft[0] + pasteDim[0] + 8;
                startStop[1][0] = startStop[0][0];
                linePaint(startStop, Black, img, imgDim[0], textSeparatorWidth);
                startStop[0][0] += textSeparatorWidth;
                startStop[1][0] += textSeparatorWidth;
                blockIndex++;
            }
            startStop[0][0] = 0;
            startStop[0][1] += fontHeight;
            startStop[1][0] = 0;
            startStop[1][1] += fontHeight;
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

    template<typename ... Args> void mapBinLoad(string& pathBIN, Args& ... args)
    {
        jf.err("mapBinLoad template-im");
    }
    template<> void mapBinLoad<vector<vector<int>>, vector<vector<int>>>(string& pathBIN, vector<vector<int>>& frame, vector<vector<int>>& border)
    {
        // Load all coordinates into memory from the bin file.
        frame.clear();
        border.clear();
        string sfile = jf.load(pathBIN);
        if (sfile.size() < 1) { jf.err("load-im.mapBinLoad"); }
        size_t pos1, pos2, posStart, posStop;
        string temp;
        int row;
        posStart = sfile.find("//frame");
        posStop = sfile.find("//", posStart + 7);
        if (posStop > sfile.size()) { posStop = sfile.size(); }
        pos1 = sfile.find(',', posStart);
        while (pos1 < posStop)
        {
            row = frame.size();
            frame.push_back(vector<int>(2));
            pos2 = sfile.find_last_not_of("1234567890.", pos1 - 1) + 1;
            temp = sfile.substr(pos2, pos1 - pos2);
            try { frame[row][0] = stoi(temp); }
            catch (invalid_argument& ia) { jf.err("stoi-im.mapBinLoad"); }
            pos2 = sfile.find_first_not_of("1234567890.", pos1 + 1);
            temp = sfile.substr(pos1 + 1, pos2 - pos1 - 1);
            try { frame[row][1] = stoi(temp); }
            catch (invalid_argument& ia) { jf.err("stoi-im.mapBinLoad"); }
            pos1 = sfile.find(',', pos1 + 1);
        }
        posStart = sfile.find("//border");
        posStop = sfile.find("//", posStart + 8);
        if (posStop > sfile.size()) { posStop = sfile.size(); }
        pos1 = sfile.find(',', posStart);
        while (pos1 < posStop)
        {
            row = border.size();
            border.push_back(vector<int>(2));
            pos2 = sfile.find_last_not_of("1234567890", pos1 - 1) + 1;
            temp = sfile.substr(pos2, pos1 - pos2);
            try { border[row][0] = stoi(temp); }
            catch (invalid_argument& ia) { jf.err("stoi-im.mapBinLoad"); }
            pos2 = sfile.find_first_not_of("1234567890", pos1 + 1);
            temp = sfile.substr(pos1 + 1, pos2 - pos1 - 1);
            try { border[row][1] = stoi(temp); }
            catch (invalid_argument& ia) { jf.err("stoi-im.mapBinLoad"); }
            pos1 = sfile.find(',', pos1 + 1);
        }
    }
    template<> void mapBinLoad<vector<vector<double>>, vector<vector<double>>>(string& pathBIN, vector<vector<double>>& frame, vector<vector<double>>& border)
    {
        vector<vector<int>> iFrame, iBorder;
        mapBinLoad(pathBIN, iFrame, iBorder);
        jf.toDouble(iFrame, frame);
        jf.toDouble(iBorder, border);
    }

    template<typename ... Args> void octogonBearing(SWITCHBOARD& sbgui, vector<vector<int>>& pastPresent, string sZone, int& radius, Args& ... args)
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
    template<> void octogonBearing<double, int>(SWITCHBOARD& sbgui, vector<vector<int>>& pastPresent, string sZone, int& radius, double& theta, int& widthZone)
    {
        unordered_map<string, int> mapIndexCandidate;
        unordered_map<string, int> mapWidth;
        vector<vector<int>> octoPath = octogonPath(pastPresent[1], radius);
        vector<vector<unsigned char>> octoRGB = octogonRGB(octoPath);
        vector<int> deadStartStop;
        if (textFound.size() > 0) // Undesirable white cushions should be painted blue.
        {
            deadConeText(pastPresent[1], octoPath, deadStartStop);
            deadConePaint(octoRGB, deadStartStop);
            if (sizeVBP == pauseVBP - 1)
            {
                makeMapZoneSweep(octoPath, octoRGB);
            }
        }
        vector<vector<int>> candidates = zoneSweep(sZone, octoRGB, octoPath, mapIndexCandidate, mapWidth);
        int numCandidates = candidates.size();
        int lettersExist = 0;
        vector<vector<int>> pastPresentFuture = pastPresent;
        string sCandidate;
        int iWidth, newRadius;
        
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
                if (radius == defaultSearchRadius)
                {
                    deltaRadius = -2;
                }
                else if (deltaRadius > 0)
                {
                    theta = -2.0;
                    return;
                }
                newRadius = radius - 2;
                octogonBearing(sbgui, pastPresent, sZone, newRadius, theta, widthZone);
                if (theta >= 0.0) { radius = newRadius; }
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
                if (radius == defaultSearchRadius)
                {
                    deltaRadius = 2;
                }
                else if (deltaRadius < 0)
                {
                    theta = -2.0;
                    return;
                }
                newRadius = radius + 2;
                octogonBearing(sbgui, pastPresent, sZone, newRadius, theta, widthZone);
                if (theta >= 0.0) { radius = newRadius; }
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
            if (numCandidates < 1)
            {
                //makeMapZoneSweep(octoPath);
                return;
            }

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
            if (pastPresentFuture.size() == 2)
            {
                for (int ii = 0; ii < candidates.size(); ii++)
                {
                    pastPresentFuture.push_back(candidates[ii]);
                }
            }
            makeMapOctogonBearing(pastPresentFuture);
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
    template<> void octogonBearing<vector<double>>(SWITCHBOARD& sbgui, vector<vector<int>>& pastPresent, string sZone, int& radius, vector<double>& theta)
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

    template<typename ... Args> void octogonPaint(vector<unsigned char> rgb, Args& ... args)
    {
        jf.err("octogonPaint template-im");
    }
    template<> void octogonPaint<vector<int>, int>(vector<unsigned char> rgb, vector<int>& origin, int& radius)
    {
        if (debugDataPNG.size() < 1) { jf.err("debugDataPNG not initialized-im.octogonPaint"); }
        vector<vector<int>> path = octogonPath(origin, radius);
        for (int ii = 0; ii < path.size(); ii++)
        {
            dotPaint(path[ii], rgb, debugDataPNG, width, defaultOctogonWidth);
        }
    }
    template<> void octogonPaint<vector<int>, int, vector<unsigned char>, vector<int>, int>(vector<unsigned char> rgb, vector<int>& origin, int& radius, vector<unsigned char>& img, vector<int>& sourceDim, int& widthDot)
    {
        vector<vector<int>> path = octogonPath(origin, radius);

        for (int ii = 0; ii < path.size(); ii++)
        {
            dotPaint(path[ii], img, sourceDim[0], rgb, widthDot);
        }
    }
    template<> void octogonPaint<vector<unsigned char>, vector<int>, int, vector<vector<int>>, int>(vector<unsigned char> rgb, vector<unsigned char>& img, vector<int>& sourceDim, int& widthDot, vector<vector<int>>& octoPath, int& pathSpace)
    {
        // The pathSpace variable defines whether to paint the octogonal path which is 
        // outside the given octogon (pathSpace positive) or inside the given octogon (pS neg).
        vector<vector<int>> path = octoPath;
        int inum = min(0, pathSpace);
        octogonCheckBoundary(path, sourceDim, inum);
        vector<vector<int>> ring;
        vector<int> coord(2);
        int pathIndex = 0;
        if (pathSpace > 0)
        {
            coord[0] = path[0][0];
            coord[1] = path[0][1] - pathSpace;
            while (path[pathIndex + 1][0] > path[pathIndex][0])
            {

                ring.push_back(coord);
                coord[0] += 1;
                coord[1] += 1;
                pathIndex++;
            }
            while (coord[1] < path[pathIndex][1])
            {
                ring.push_back(coord);
                coord[0] += 1;
                coord[1] += 1;
            }
            while (path[pathIndex + 1][0] == path[pathIndex][0])
            {
                ring.push_back(coord);
                coord[1] += 1;
                pathIndex++;
            }
            while (path[pathIndex + 1][1] > path[pathIndex][1])
            {
                ring.push_back(coord);
                coord[0] -= 1;
                coord[1] += 1;
                pathIndex++;
            }
            while (coord[1] < path[pathIndex][1] + pathSpace)
            {
                ring.push_back(coord);
                coord[0] -= 1;
                coord[1] += 1;
            }
            while (path[pathIndex + 1][1] == path[pathIndex][1])
            {
                ring.push_back(coord);
                coord[0] -= 1;
                pathIndex++;
            }
            while (path[pathIndex + 1][0] < path[pathIndex][0])
            {
                ring.push_back(coord);
                coord[0] -= 1;
                coord[1] -= 1;
                pathIndex++;
            }
            while (coord[1] > path[pathIndex][1])
            {
                ring.push_back(coord);
                coord[0] -= 1;
                coord[1] -= 1;
            }
            while (path[pathIndex + 1][0] == path[pathIndex][0])
            {
                ring.push_back(coord);
                coord[1] -= 1;
                pathIndex++;
            }
            while (path[pathIndex + 1][1] < path[pathIndex][1])
            {
                ring.push_back(coord);
                coord[0] += 1;
                coord[1] -= 1;
                pathIndex++;
            }
            while (coord[0] < path[pathIndex][0])
            {
                ring.push_back(coord);
                coord[0] += 1;
                coord[1] -= 1;
            }
            while (pathIndex < path.size())
            {
                ring.push_back(coord);
                coord[0] += 1;
                pathIndex++;
            }
            if (ring[ring.size() - 1] != ring[0]) { jf.err("Octogon path incomplete-im.octogonPaint"); }
        }
        else if (pathSpace < 0)
        {
            coord[0] = path[0][0];
            coord[1] = path[0][1] - pathSpace;
            while (path[pathIndex][1] < coord[1]) { pathIndex++; }
            while (path[pathIndex + 1][0] > path[pathIndex][0])
            {

                ring.push_back(coord);
                coord[0] += 1;
                coord[1] += 1;
                pathIndex++;
            }
            while (path[pathIndex + 1][0] == path[pathIndex][0])
            {
                ring.push_back(coord);
                coord[1] += 1;
                pathIndex++;
            }
            while (path[pathIndex][0] > coord[0]) { pathIndex++; }
            while (path[pathIndex + 1][1] > path[pathIndex][1])
            {

                ring.push_back(coord);
                coord[0] -= 1;
                coord[1] += 1;
                pathIndex++;
            }
            while (path[pathIndex + 1][1] == path[pathIndex][1])
            {
                ring.push_back(coord);
                coord[0] -= 1;
                pathIndex++;
            }
            while (path[pathIndex][1] > coord[1]) { pathIndex++; }
            while (path[pathIndex + 1][0] < path[pathIndex][0])
            {
                ring.push_back(coord);
                coord[0] -= 1;
                coord[1] -= 1;
                pathIndex++;
            }
            while (path[pathIndex + 1][0] == path[pathIndex][0])
            {
                ring.push_back(coord);
                coord[1] -= 1;
                pathIndex++;
            }
            while (path[pathIndex][0] < coord[0]) { pathIndex++; }
            while (path[pathIndex + 1][1] < path[pathIndex][1])
            {
                ring.push_back(coord);
                coord[0] += 1;
                coord[1] -= 1;
                pathIndex++;
            }
            while (pathIndex < path.size())
            {
                ring.push_back(coord);
                coord[0] += 1;
                pathIndex++;
            }

            if (ring[ring.size() - 1] != ring[0]) 
            {
                string sDebug = sroot + "\\debug\\Octogon Paint Debug.png";
                vector<unsigned char> canvas = pngBlankCanvas(sourceDim);
                pathPaintDebug(Red, canvas, sourceDim, path);
                pathPaintDebug(Black, canvas, sourceDim, ring);
                pngPrint(canvas, sourceDim, sDebug);
                jf.err("Octogon path incomplete-im.octogonPaint"); 
            }
        }
        else { ring = path; }

        for (int ii = 0; ii < ring.size(); ii++)
        {
            dotPaint(ring[ii], rgb, img, sourceDim[0], widthDot);
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

        removeColourCushion(octoRGB, { 38, 115, 0 }, White, 7);
        if (octoRGB[0] != octoRGB[octoRGB.size() - 1])
        {
            jf.err("first-last RGB mismatch-im.scanOctogon");
        }
        return octoRGB;
    }
    template<> vector<vector<unsigned char>> octogonRGB<vector<vector<int>>>(vector<vector<int>>& octoPath)
    {
        vector<vector<unsigned char>> octoRGB(octoPath.size(), vector<unsigned char>(3));
        vector<unsigned char> textColour = { 38, 115, 0 };
        textFound.clear();
        bool textActive;
        octoRGB[0] = pixelRGB(octoPath[0]);
        if (octoRGB[0] == textColour || octoRGB[0] == Black) { textActive = 1; }
        else { textActive = 0; }

        for (int ii = 1; ii < octoRGB.size(); ii++)
        {
            octoRGB[ii] = pixelRGB(octoPath[ii]);
            if (octoRGB[ii] == textColour || octoRGB[ii] == Black)
            {
                if (!textActive)
                {
                    textActive = 1;
                    textFound.push_back(ii);
                }
            }
            else if (textActive)
            {
                textActive = 0;
            }
        }
        return octoRGB;
    }

    template<typename ... Args> void pathPaintDebug(vector<unsigned char> rgb, Args& ... args)
    {
        jf.err("pathPaint template-im");
    }
    template<> void pathPaintDebug<vector<unsigned char>, vector<int>, vector<vector<int>>>(vector<unsigned char> rgb, vector<unsigned char>& img, vector<int>& sourceDim, vector<vector<int>>& path)
    {
        int widthPixel = 1;
        for (int ii = 0; ii < path.size(); ii++)
        {
            dotPaint(path[ii], rgb, img, sourceDim[0], widthPixel);
        }
    }

    template<typename ... Args> void pngAppendText(Args& ... args)
    {
        jf.err("pngAppendText-im");
    }
    template<> void pngAppendText<string, vector<int>>(string& text, vector<int>& coord)
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
    template<> void pngAppendText<vector<string>, vector<int>>(vector<string>& listText, vector<int>& coord)
    {
        if (coord.size() < 2) { jf.err("Bad coordinates-im.pngAppendText"); }
        string temp = "(" + to_string(coord[0]) + "," + to_string(coord[1]) + ")";
        listText.push_back(temp);
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

    template<typename ... Args> void waitMapDebug(SWITCHBOARD& sbgui, Args& ... args)
    {
        jf.err("waitMapDebug template-im");
    }
    template<> void waitMapDebug<string>(SWITCHBOARD& sbgui, string& mapPath)
    {
        vector<string> newPrompt = { mapPath };
        sbgui.set_prompt(newPrompt);
        thread::id myid = this_thread::get_id();
        vector<int> myComm = sbgui.getMyComm(myid);
        myComm[0] = 3;
        vector<vector<int>> comm = sbgui.update(myid, myComm);
        while (1)
        {
            this_thread::sleep_for(40ms);
            comm = sbgui.update(myid, myComm);
            if (comm[0][0] == 3)
            {
                myComm[0] = 1;
                sbgui.update(myid, myComm);
                break;
            }
        }
    }
    template<> void waitMapDebug<string, vector<vector<int>>>(SWITCHBOARD& sbgui, string& mapPath, vector<vector<int>>& tracks)
    {
        string sCoord = jf.stringifyCoord(tracks[tracks.size() - 1]);
        vector<string> newPrompt = { mapPath, sCoord };
        sbgui.set_prompt(newPrompt);
        thread::id myid = this_thread::get_id();
        vector<int> myComm = sbgui.getMyComm(myid);
        myComm[0] = 3;
        vector<vector<int>> comm = sbgui.update(myid, myComm);
        while (1)
        {
            this_thread::sleep_for(40ms);
            comm = sbgui.update(myid, myComm);
            if (comm[0][0] == 3)
            {
                myComm[0] = 0;
                sbgui.update(myid, myComm);
                break;
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
        return goldilocks;
    }

};

