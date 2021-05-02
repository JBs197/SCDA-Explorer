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
    stbtt_fontinfo defaultFont;
    vector<unsigned char> dataPNG, debugDataPNG;
	bool debug = 1;
    int defaultDotWidth = 5;
    int defaultLineWidth = 4;
    int defaultOctogonWidth = 3;
    vector<int> defaultExtractDim = { 400, 400 };
    int defaultSearchRadius = 15;
    string pathActivePNG;
	JFUNC jf;
	unordered_map<string, string> mapColour;
	vector<int> pointOfOrigin, revisedExtractDim;
	int width, height, numComponents, recordVictor;
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
	void initMapColours();
	bool isInit();
    bool jobsDone(vector<int> vCoord);
	vector<vector<int>> linePath(vector<vector<int>>& startStop);
    vector<vector<int>> linePathToEdge(vector<vector<int>>& startMid);
	vector<vector<unsigned char>> lineRGB(vector<vector<int>>& vVictor, int length);
    vector<vector<unsigned char>> makeText(string text);
	bool mapIsInit();
    vector<vector<int>> octogonPath(vector<int> origin, int radius);
    string pixelDecToHex(vector<unsigned char>& rgb);
	void pixelPaint(vector<unsigned char>& img, int widthImg, vector<unsigned char> rgb, vector<int> coord);
	vector<unsigned char> pixelRGB(vector<int>& coord);
	string pixelZone(vector<unsigned char>& rgb);
	void pngLoad(string& pathPNG);
    void pngPrint(vector<vector<unsigned char>>& img);
    void pngToBinLive(SWITCHBOARD& sbgui, vector<vector<double>>& border);
    void pngToBinLiveDebug(SWITCHBOARD& sbgui, vector<vector<double>>& border);
    vector<vector<unsigned char>> scanColumn(int col);
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
            dotPaint(path[ii], img, widthImg, rgb, widthDot);
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
        //int widthDot = 5;
        //int widthLine = 5;
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
        }
        dotPaint(tracks[tracks.size() - 1], Yellow);
        
        for (int ii = 0; ii < candidates.size(); ii++)
        {
            dotPaint(candidates[ii], Red);
        }

        vector<int> sourceDim = { width, height };
        vector<unsigned char> cropped = pngExtractRect(tracks[tracks.size() - 1], debugDataPNG, sourceDim, defaultExtractDim);
        int imgSize = cropped.size();
        int channels = 3;
        auto bufferUC = new unsigned char[imgSize];
        for (int ii = 0; ii < imgSize; ii++)
        {
            bufferUC[ii] = cropped[ii];
        }
        string pathImg = sroot + "\\debug\\borderFindNextDebug.png";
        int error = stbi_write_png(pathImg.c_str(), defaultExtractDim[0], defaultExtractDim[1], channels, bufferUC, 0);
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
    template<> void octogonBearing<double>(SWITCHBOARD& sbgui, vector<vector<int>>& pastPresent, string sZone, int radius, double& theta)
    {
        unordered_map<string, int> mapIndexCandidate;
        vector<int> intervalSize;
        vector<vector<int>> octoPath = octogonPath(pastPresent[1], radius);
        vector<vector<unsigned char>> octoRGB = octogonRGB(octoPath);
        vector<vector<int>> candidates = zoneSweep(sZone, octoRGB, octoPath, mapIndexCandidate, intervalSize);
        int numCandidates = candidates.size();
        int lettersExist = 0;
        vector<vector<int>> pastPresentFuture = pastPresent;
        
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
                octogonBearing(sbgui, pastPresent, sZone, radius - 2, theta);
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
                octogonBearing(sbgui, pastPresent, sZone, radius + 2, theta);
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
                if (intervalSize[ii] > sizeMax)
                {
                    sizeSecond = sizeMax;
                    sizeMax = intervalSize[ii];
                    candidateWide = candidates[ii];
                }
                else if (intervalSize[ii] > sizeSecond)
                {
                    sizeSecond = intervalSize[ii];
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
    template<> vector<vector<int>> zoneSweep<vector<int>>(string sZone, vector<vector<unsigned char>>& Lrgb, vector<vector<int>>& zonePath, unordered_map<string, int>& mapIndexCandidate, vector<int>& intervals)
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
                    intervals.push_back(onOff[1] - onOff[0]);
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
                intervals.push_back(inum1 + zoneFreezer);
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

};

