#include "imgfunc.h"

vector<int> IMGFUNC::borderFindNext(SWITCHBOARD& sbgui, vector<vector<int>> tracks)
{
    string pathImg = sroot + "\\debug\\borderFindNextDebug.png";
    vector<int> failure = { -1, -1 };
    vector<int> origin = tracks[tracks.size() - 1];
    int radius = defaultSearchRadius + searchRadiusIncrease;
    unordered_map<string, int> mapIndexCandidate;
    vector<vector<int>> octoPath = octogonPath(origin, radius);
    vector<vector<unsigned char>> octoRGB = octogonRGB(octoPath);
    vector<int> deadStartStop;
    if (textFound.size() > 0) // Undesirable white cushions should be painted blue.
    {
        deadConeText(origin, octoPath, deadStartStop); 
        deadConePaint(octoRGB, deadStartStop);
        if (sizeVBP == pauseVBP)
        {
            makeMapZoneSweep(octoPath, octoRGB);
        }
    }
    string sZone = "zoneBorder";
    vector<vector<int>> candidates = zoneSweep(sZone, octoRGB, octoPath, mapIndexCandidate);
    vector<vector<int>> vvTemp, cPath;
    int numIncreases = 0, inum = -1;
    vector<int> sourceDim = { width, height };
    while (candidates.size() < 2)
    {
        if (!debug && candidates.size() == 1)
        {
            inum = testHereBeDragons(octoRGB, candidates[0], mapIndexCandidate);
            if (inum > 0)
            {
                if (savePoints.size() > 0)
                {
                    backtrack = 1;
                }
                return failure;
            }
        }
        radius += (3 * borderThickness);
        octoPath = octogonPath(origin, radius);
        octogonCheckBoundary(octoPath, sourceDim, 0);
        octoRGB = octogonRGB(octoPath);
        mapIndexCandidate.clear();
        candidates = zoneSweep(sZone, octoRGB, octoPath, mapIndexCandidate);
        testBacktrack(tracks, candidates);
        numIncreases++;
    }
    if (tracks.size() < 2) { return candidates[0]; }     // We arbitrarily choose the path
    vector<int> originPast = tracks[tracks.size() - 2];  // of direction "earliest time" 
    vector<double> distances;                            // on an analog clock, for the 
    coordDist(originPast, candidates, distances);        // first step.
    double minDistance, dTemp;
    int elimIndex, numCandidates;

    testDistances(candidates, distances);
    while (candidates.size() < 1)
    {
        radius += (3 * borderThickness);
        octoPath = octogonPath(origin, radius);
        octogonCheckBoundary(octoPath, sourceDim, 0);
        octoRGB = octogonRGB(octoPath);
        mapIndexCandidate.clear();
        candidates = zoneSweep(sZone, octoRGB, octoPath, mapIndexCandidate);
        testBacktrack(tracks, candidates);
        numIncreases++;
    }
    while (candidates.size() > 2)
    {
        minDistance = distances[0];
        elimIndex = 0;
        for (int ii = 1; ii < distances.size(); ii++)
        {
            if (distances[ii] < minDistance)
            {
                minDistance = distances[ii];
                elimIndex = ii;
            }
        }
        candidates.erase(candidates.begin() + elimIndex);
        distances.erase(distances.begin() + elimIndex);
    }

    pauseMapDebug(sbgui, tracks, radius, candidates);
    if (sizeVBP < 0)
    {
        backtrack = 1;
        return failure;
    }
    if (candidates.size() == 1) { return candidates[0]; }

    // There can only be two candidates remaining.  
    
    // Perform a series of tests to determine the right path.

    // Test for access to the interior (white) zone, if on land.
    if (!testOverWater(tracks, candidates))
    {
        sZone = "white";
        numCandidates = testCandidatesInteriorZone(sbgui, tracks, sZone, candidates);
        if (numCandidates == 1) 
        {
            return candidates[0]; 
        }
    }

    // Test for greater distance from previous border point. 
    double candidateDistanceTolerance = defaultCandidateDistanceTolerance * (1.0 - ((0.5 * rabbitHole) / (rabbitHoleDepth - 1)));
    if (distances[0] > distances[1])                           // Note that the distance tolerance
    {                                                          // equation's purpose is to gradually
        dTemp = (distances[0] - distances[1]) / distances[0];  // lower the bar for this test, from
        if (dTemp > candidateDistanceTolerance)                // 100% to 50% of the default. 
        {
            return candidates[0]; 
        }
    }
    else
    {
        dTemp = (distances[1] - distances[0]) / distances[1];
        if (dTemp > candidateDistanceTolerance) { return candidates[1]; }
    }

    // Try again, larger radius.
    rabbitHole++;
    if (rabbitHole == 4)  
    {
        if (debug)
        {
            pathMapDebug = defaultDebugMapPath;
            pauseMapDebug(sbgui, tracks, radius, candidates);
            if (sizeVBP < 0) 
            {
                backtrack = 1;
                return failure; 
            }
            else if (candidates.size() == 1) { return candidates[0]; }
        }
        else
        {
            saveThisPoint(tracks, candidates);
        }
    }
    if (rabbitHole > 0 && rabbitHole < rabbitHoleDepth)
    {
        searchRadiusIncrease += 2;
        return borderFindNext(sbgui, tracks);
    }

    // Hail Mary.
    testCenterOfMass(tracks, candidates);
    if (candidates.size() == 1) 
    {
        return candidates[0]; 
    }

    // Abort this path, and backtrack to the most recent saved point.
    if (savePoints.size() > 0)
    {
        backtrack = 1;
        return failure;
    }

    // Defeat. Make a debug map.
    makeMapBorderFindNext(tracks, radius, candidates);
    jf.err("Failed to determine next border point-im.borderFindNext");
    return failure;
}
vector<int> IMGFUNC::borderFindStart()
{
    if (!isInit()) { jf.err("No init-im.borderFindStart"); }
    vector<string> szones = { "zoneBorder", "white" };
    int minDim = min(width, height);
    int interval = minDim / 10;
    int pixelJump = 2, offset = 0; 
    bool letMeOut = 0;
    vector<vector<int>> vCorners = { {0,0}, {width-1,0}, {0,height-1}, {width-1,height-1} }; 
    vector<vector<int>> vVictors = { {pixelJump, pixelJump}, {-1 * pixelJump, pixelJump}, {pixelJump, -1 * pixelJump}, {-1 * pixelJump, -1 * pixelJump} };
    vector<vector<int>> vvI(2, vector<int>(2)), vCross1, vCross2;
    while (!letMeOut)
    {
        for (int ii = 0; ii < 4; ii++)
        {
            vvI[0] = vCorners[ii];
            switch (ii)
            {
            case 0:
                vvI[0][0] += offset;
                break;
            case 1:
                vvI[0][1] += offset;
                break;
            case 2:
                vvI[0][1] -= offset;
                break;
            case 3:
                vvI[0][0] -= offset;
                break;
            }
            vvI[1] = vVictors[ii];
            vCross1 = zoneChangeLinear(szones, vvI);
            if (vCross1.size() == 0) { continue; }

            szones = { "zoneBorder", "known" };
            vvI[0] = vCross1[0];
            vvI[1][0] *= -1;
            vvI[1][1] *= -1;
            vCross2 = zoneChangeLinear(szones, vvI);
            if (vCross2.size() == 0) { continue; }
            else 
            {
                letMeOut = 1;
                break; 
            }
        }
        if (!letMeOut)
        {
            offset += interval;
        }
    }
    vector<int> vStart;
    vector<double> thickness;
    if (vCross1.size() == 2 && vCross2.size() == 2)
    {
        vvI[0] = vCross1[0];
        vvI[1] = vCross2[0];
        vStart = coordMid(vvI);
        pointOfOrigin = vStart;
    }

    //if (debug) { drawMarker(debugDataPNG, vStart); }
    return vStart;
}
void IMGFUNC::buildFont(string filePath)
{
    // Many assumptions are made for the sake of expediency. 
    // Assume the bitmap font is 512x512, starting from ASCII 32, 
    // each row having a height of 32 pixels, with 16 glyphs per
    // row. Also assume zero antialiasing, with glyphs as black on white.
    pngLoad(filePath);
    size_t pos2 = filePath.rfind(".png");
    size_t pos1 = filePath.rfind('\\', pos2) + 1;
    string temp = filePath.substr(pos1, pos2 - pos1);
    string fontDir = sroot + "\\font\\" + temp;

    vector<vector<unsigned char>> column;
    vector<int> sourceDim = { width, height };
    vector<unsigned char> white = { 255, 255, 255 };
    vector<int> columnSpaces = { 0 };
    int start, stop, middle;
    bool spaceStart = 0;
    for (int ii = 2; ii < width; ii++)
    {
        column = scanColumn(ii);
        for (int jj = 0; jj < column.size(); jj++)
        {
            if (column[jj] != white) 
            {
                if (spaceStart)
                {
                    spaceStart = 0;
                    stop = ii - 1;
                    middle = start + ((stop - start) / 2);
                    columnSpaces.push_back(middle);
                }
                break; 
            }
            else if (jj == column.size() - 1)
            {
                if (!spaceStart) 
                {
                    spaceStart = 1; 
                    start = ii;
                }
            }
        }
    }
    
    vector<unsigned char> glyph;
    vector<int> extractDim(2);
    extractDim[1] = 32;
    vector<int> topLeft(2);
    int ascii = 32;
    string glyphPath;
    for (int ii = 0; ii < 14; ii++)
    {
        topLeft[1] = 32 * ii;
        for (int jj = 0; jj < columnSpaces.size(); jj++)
        {
            if (jj < columnSpaces.size() - 1)
            {
                topLeft[0] = columnSpaces[jj];
                extractDim[0] = columnSpaces[jj + 1] - columnSpaces[jj];
                glyph = pngExtractRectTopLeft(topLeft, dataPNG, sourceDim, extractDim);
                if (ii != 0 || jj != 0) { trimWidth(glyph, extractDim, white); }
                glyphPath = fontDir + "\\" + to_string(ascii) + ".png";
                if (extractDim[0] > 0) { pngPrint(glyph, extractDim, glyphPath); }
                ascii++;
            }
            else
            {
                topLeft[0] = columnSpaces[jj];
                extractDim[0] = width - columnSpaces[jj] - 1;
                glyph = pngExtractRectTopLeft(topLeft, dataPNG, sourceDim, extractDim);
                trimWidth(glyph, extractDim, white);
                glyphPath = fontDir + "\\" + to_string(ascii) + ".png";
                if (extractDim[0] > 0) { pngPrint(glyph, extractDim, glyphPath); }
                ascii++;
            }
        }
    }
}
vector<vector<int>> IMGFUNC::checkBoundary(vector<int>& center, vector<int>& sourceDim, vector<int>& extractDim)
{
    // Check to see if the rectangle to be extracted goes past the edges
    // of the source image. If so, then revised coordinates for the 
    // rectangle's top left and bottom right corners are returned. If no 
    // edges are encountered, then only the top left corner coordinates 
    // of the rectangle are returned.
    vector<vector<int>> corners;
    bool errorFound = 0;
    vector<int> topLeft(2);
    topLeft[0] = center[0] - (extractDim[0] / 2);
    topLeft[1] = center[1] - (extractDim[1] / 2);
    vector<int> botRight(2);
    botRight[0] = topLeft[0] + extractDim[0];
    botRight[1] = topLeft[1] + extractDim[1];
    if (topLeft[0] < 0)
    {
        errorFound = 1;
        topLeft[0] = 0;
    }
    if (topLeft[1] < 0)
    {
        errorFound = 1;
        topLeft[1] = 0;
    }
    if (botRight[0] > sourceDim[0])
    {
        errorFound = 1;
        botRight[0] = sourceDim[0] - 1;
    }
    if (botRight[1] > sourceDim[1])
    {
        errorFound = 1;
        botRight[1] = sourceDim[1] - 1;
    }
    corners.push_back(topLeft);
    if (errorFound)
    {
        corners.push_back(botRight);
    }
    return corners;
}
double IMGFUNC::clockwisePercentage(SWITCHBOARD& sbgui, vector<vector<int>>& tracks, string sZone)
{
    // Returns a percentage [0.0, 1.0] representing how often the chosen
    // zone was clockwise along the given coordinate path. 
    if (tracks.size() < 2) { jf.err("Not enough coordinates-im.clockwisePercentage"); }
    int numClockwise = 0;
    vector<vector<int>> pastPresent(2, vector<int>(2));
    int numBearings = tracks.size() - 1;
    vector<double> bearings(numBearings, -1.0);
    vector<int> widthZone(numBearings);
    int radius;
    for (int ii = 0; ii < numBearings; ii++)
    {
        pastPresent[0] = tracks[ii];
        pastPresent[1] = tracks[ii + 1];
        radius = defaultSearchRadius;
        octogonBearing(sbgui, pastPresent, sZone, radius, bearings[ii], widthZone[ii]);
        if (bearings[ii] >= 0.0 && bearings[ii] < 180.0) { numClockwise++; }
    }
    double percent = (double)numClockwise / ((double)tracks.size() - 1.0);
    return percent;
}
vector<vector<int>> IMGFUNC::coordPath(vector<vector<int>> startStop)
{
    // Returns a list of coordinates connecting the start and 
    // stop points via straight line. Includes the start point.
    double deltax = (double)(startStop[1][0] - startStop[0][0]);
    double deltay = (double)(startStop[1][1] - startStop[0][1]);
    double dx, dy, xCoord, yCoord;
    if (abs(deltax) >= abs(deltay))
    {
        dx = deltax / abs(deltax);
        dy = deltay / abs(deltax);
    }
    else
    {
        dx = deltax / abs(deltay);
        dy = deltay / abs(deltay);
    }
    vector<vector<int>> cPath(1, vector<int>(2));
    vector<int> vTemp(2);
    cPath[0] = startStop[0];
    xCoord = (double)startStop[0][0];
    yCoord = (double)startStop[0][1];
    while (1)
    {
        xCoord += dx;
        yCoord += dy;
        vTemp[0] = (int)round(xCoord);
        vTemp[1] = (int)round(yCoord);
        cPath.push_back(vTemp);
        if (vTemp == startStop[1]) { break; }
    }
    return cPath;
}
int IMGFUNC::coordRGB(vector<vector<int>> cPath, string sZone)
{
    int count = 0;
    vector<unsigned char> rgb;
    string szone;
    for (int ii = 0; ii < cPath.size(); ii++)
    {
        rgb = pixelRGB(cPath[ii]);
        szone = pixelZone(rgb);
        if (szone == sZone) { count++; }
    }
    return count;
}
vector<int> IMGFUNC::coordStoi(string& sCoords)
{
	vector<int> vCoords(2);
	size_t pos1 = sCoords.find(',');
	size_t pos2 = sCoords.find_last_of("1234567890", pos1) + 1;
	size_t pos3 = sCoords.find_last_not_of("1234567890", pos2 - 1) + 1;
	string temp = sCoords.substr(pos3, pos2 - pos3);
	try { vCoords[0] = stoi(temp); }
	catch (invalid_argument& ia) { jf.err("stoi1-im.coordStoi"); }
	pos2 = sCoords.find_first_of("1234567890", pos1);
	pos3 = sCoords.find_first_not_of("1234567890", pos2);
	temp = sCoords.substr(pos2, pos3 - pos2);
	try { vCoords[1] = stoi(temp); }
	catch (invalid_argument& ia) { jf.err("stoi2-im.coordStoi"); }
	return vCoords;
}
void IMGFUNC::drawMarker(vector<unsigned char>& img, vector<int>& vCoord)
{
    // Paint a 3x3 red square on the selected map/point.
    vector<int> vTemp(2);
    for (int ii = 0; ii < 3; ii++)
    {
        vTemp[1] = vCoord[1] - 1 + ii;
        for (int jj = 0; jj < 3; jj++)
        {
            vTemp[0] = vCoord[0] - 1 + jj;
            pixelPaint(img, width, Red, vTemp);
        }
    }
}
vector<vector<double>> IMGFUNC::frameCorners()
{
    vector<string> sZones = { "white", "black" };
    vector<string> sZonesReverse = { "black", "white" };
    vector<vector<double>> corners(4, vector<double>(2));
    vector<vector<int>> vVictor(2, vector<int>(2));
    vVictor[0] = { 0, height / 2 };
    vVictor[1] = { 1, 0 };
    vector<vector<int>> vvTemp = zoneChangeLinear(sZones, vVictor);
    vVictor[0] = { vvTemp[1][0], height / 2 };
    vVictor[1] = { 0, -1 };
    vvTemp = zoneChangeLinear(sZonesReverse, vVictor);
    corners[0][0] = (double)vvTemp[0][0];
    corners[0][1] = (double)vvTemp[0][1];
    vVictor[0] = vvTemp[0];
    vVictor[1] = { 1, 0 };
    vvTemp = zoneChangeLinear(sZonesReverse, vVictor);
    corners[1][0] = (double)vvTemp[0][0];
    corners[1][1] = (double)vvTemp[0][1];
    vVictor[0] = vvTemp[0];
    vVictor[1] = { 0, 1 };
    vvTemp = zoneChangeLinear(sZonesReverse, vVictor);
    corners[2][0] = (double)vvTemp[0][0];
    corners[2][1] = (double)vvTemp[0][1];
    vVictor[0] = vvTemp[0];
    vVictor[1] = { -1, 0 };
    vvTemp = zoneChangeLinear(sZonesReverse, vVictor);
    corners[3][0] = (double)vvTemp[0][0];
    corners[3][1] = (double)vvTemp[0][1];
    return corners;
}
vector<unsigned char> IMGFUNC::getColour(string sColour)
{
    if (sColour == "Black") { return Black; }
    else if (sColour == "Blue") { return Blue; }
    else if (sColour == "Gold") { return Gold; }
    else if (sColour == "Green") { return Green; }
    else if (sColour == "Orange") { return Orange; }
    else if (sColour == "Pink") { return Pink; }
    else if (sColour == "Purple") { return Purple; }
    else if (sColour == "Red") { return Red; }
    else if (sColour == "Teal") { return Teal; }
    else if (sColour == "White") { return White; }
    else if (sColour == "Yellow") { return Yellow; }
    vector<unsigned char> failure;
    return failure;
}
int IMGFUNC::getDotWidth()
{
    return defaultDotWidth;
}
string IMGFUNC::getMapPath(int mode)
{
    // Modes: 0 = cropped debug.
    string path;
    switch (mode)
    {
    case 0:
        path = defaultDebugMapPath;
        break;
    }
    return path;
}
int IMGFUNC::getQuadrant(vector<vector<int>>& startStop)
{
    // NOTE: Image coordinates use a reversed y-axis (positive points down) !
    //       7
    //     2 | 3
    //   6---+---4      <--- Quadrant diagram.
    //     1 | 0
    //       5
    int Dx = startStop[1][0] - startStop[0][0];
    int Dy = startStop[1][1] - startStop[0][1];
    if (Dx > 0)
    {
        if (Dy > 0) { return 0; }
        else if (Dy < 0) { return 3; }
        else { return 4; }
    }
    else if (Dx < 0)
    {
        if (Dy > 0) { return 1; }
        else if (Dy < 0) { return 2; }
        else { return 6; }
    }
    else
    {
        if (Dy > 0) { return 5; }
        else if (Dy < 0) { return 7; }
        else { jf.err("startStop identical-im.getQuadrant"); }
    }
    return -1;
}
double IMGFUNC::getStretchFactor(string& widthHeight)
{
    size_t pos1 = widthHeight.find(',');
    string temp = widthHeight.substr(0, pos1);
    string temp2 = widthHeight.substr(pos1 + 1);
    double widthLabel, heightLabel;
    try
    {
        widthLabel = stod(temp);
        heightLabel = stod(temp2);
    }
    catch (invalid_argument& ia) { jf.err("stod-im.getStretchFactor"); }
    double widthDemSup = (double)width / widthLabel;
    double heightDemSup = (double)height / heightLabel;
    if (widthDemSup > heightDemSup)
    {
        stretchFactor = 1.0 / widthDemSup;
    }
    else
    {
        stretchFactor = 1.0 / heightDemSup;
    }
    return stretchFactor;
}
void IMGFUNC::initGlyph(string& filePath, int ascii)
{
    pngLoad(filePath);
    font.push_back(dataPNG);
    mapFontWidth.emplace(ascii, width);
}
void IMGFUNC::initMapColours()
{
    if (!mapColour.empty()) { return; }
    vector<unsigned char> rgb(3);
    string sval, shex;

    rgb[0] = 255;
    rgb[1] = 255;
    rgb[2] = 255;
    shex = pixelDecToHex(rgb);
    sval = "white";
    mapColour.emplace(shex, sval);
    rgb[0] = 0;
    rgb[1] = 0;
    rgb[2] = 0;
    shex = pixelDecToHex(rgb);
    sval = "black";
    mapColour.emplace(shex, sval);
    rgb[0] = 226;
    rgb[1] = 226;
    rgb[2] = 228;
    shex = pixelDecToHex(rgb);
    sval = "zoneOutside";
    mapColour.emplace(shex, sval);
    rgb[0] = 178;
    rgb[1] = 178;
    rgb[2] = 178;
    shex = pixelDecToHex(rgb);
    sval = "roadSmall";
    mapColour.emplace(shex, sval);
    rgb[0] = 156;
    rgb[1] = 156;
    rgb[2] = 156;
    shex = pixelDecToHex(rgb);
    sval = "roadMedium";
    mapColour.emplace(shex, sval);
    rgb[0] = 0;
    rgb[1] = 112;
    rgb[2] = 255;
    shex = pixelDecToHex(rgb);
    sval = "zoneBorder";
    mapColour.emplace(shex, sval);
    rgb[0] = 179;
    rgb[1] = 217;
    rgb[2] = 247;
    shex = pixelDecToHex(rgb);
    sval = "water";
    mapColour.emplace(shex, sval);
    rgb[0] = 0;
    rgb[1] = 0;
    rgb[2] = 220;
    shex = pixelDecToHex(rgb);
    sval = "textTract";
    mapColour.emplace(shex, sval);
    rgb[0] = 0;
    rgb[1] = 77;
    rgb[2] = 168;
    shex = pixelDecToHex(rgb);
    sval = "textGeoFeature";
    mapColour.emplace(shex, sval);
    rgb[0] = 38;
    rgb[1] = 115;
    rgb[2] = 0;
    shex = pixelDecToHex(rgb);
    sval = "textHumanFeature";
    mapColour.emplace(shex, sval);
    rgb[0] = 1;
    rgb[1] = 1;
    rgb[2] = 254;
    shex = pixelDecToHex(rgb);
    sval = "blottedGreen";
    mapColour.emplace(shex, sval);
    rgb[0] = 2;
    rgb[1] = 2;
    rgb[2] = 253;
    shex = pixelDecToHex(rgb);
    sval = "blottedBlack";
    mapColour.emplace(shex, sval);
}
bool IMGFUNC::isInit()
{
	if (dataPNG.size() < 1) { return 0; }
	if (width < 1) { return 0; }
	if (height < 1) { return 0; }
	if (numComponents < 1) { return 0; }
	return 1;
}
bool IMGFUNC::jobsDone(vector<int> vCoord)
{
    double winCondition = 10.0 * (double)borderThickness;
    vector<vector<int>> vvTemp(1, vector<int>());
    vvTemp[0] = vCoord;
    vector<double> dist;
    coordDist(pointOfOrigin, vvTemp, dist);
    if (dist[0] < winCondition) { return 1; }
    return 0;
}
vector<vector<int>> IMGFUNC::linePath(vector<vector<int>>& startStop)
{
    vector<vector<int>> path;
    double Dx = (double)(startStop[1][0] - startStop[0][0]);
    double Dy = (double)(startStop[1][1] - startStop[0][1]);
    double dx, dy, xCoord, yCoord;
    int ix, iy;
    if (abs(Dx) > abs(Dy))
    {
        dy = Dy / abs(Dx);
        if (Dx > 0) { ix = 1; }
        else if (Dx < 0) { ix = -1; }
        else { ix = 0; }
        path.resize(abs(Dx) + 1, vector<int>(2));
        path[0] = startStop[0];
        yCoord = (double)startStop[0][1];
        for (int ii = 1; ii < path.size(); ii++)
        {
            yCoord += dy;
            path[ii][0] = path[ii - 1][0] + ix;
            path[ii][1] = int(round(yCoord));
        }
    }
    else
    {
        dx = Dx / abs(Dy);
        if (Dy > 0) { iy = 1; }
        else if (Dy < 0) { iy = -1; }
        else { iy = 0; }
        path.resize(abs(Dy) + 1, vector<int>(2));
        path[0] = startStop[0];
        xCoord = (double)startStop[0][0];
        for (int ii = 1; ii < path.size(); ii++)
        {
            xCoord += dx;
            path[ii][0] = int(round(xCoord)); 
            path[ii][1] = path[ii - 1][1] + iy;
        }
    }
    return path;
}
vector<vector<int>> IMGFUNC::linePathToEdge(vector<vector<int>>& startMid)
{
    // Will use IMGFUNC's width and height derived from dataPNG.
    vector<vector<int>> path;
    vector<int> viTemp;
    double Dx = (double)(startMid[1][0] - startMid[0][0]);
    double Dy = (double)(startMid[1][1] - startMid[0][1]);
    double dx, dy, xCoord, yCoord;
    int ix, iy;
    int indexPath = -1;
    bool offTheEdge = 0;
    if (abs(Dx) > abs(Dy))
    {
        dy = Dy / abs(Dx);
        if (Dx > 0) { ix = 1; }
        else if (Dx < 0) { ix = -1; }
        else { ix = 0; }
        viTemp = startMid[0];
        yCoord = (double)startMid[0][1];
        while (!offTheEdge)
        {
            path.push_back(viTemp);
            indexPath++;
            yCoord += dy;
            viTemp[0] = path[indexPath][0] + ix;
            viTemp[1] = int(round(yCoord));
            if (viTemp[0] < 0 || viTemp[0] >= width) { offTheEdge = 1; }
            if (viTemp[1] < 0 || viTemp[1] >= height) { offTheEdge = 1; }
        }
    }
    else
    {
        dx = Dx / abs(Dy);
        if (Dy > 0) { iy = 1; }
        else if (Dy < 0) { iy = -1; }
        else { iy = 0; }
        viTemp = startMid[0];
        xCoord = (double)startMid[0][0];
        while (!offTheEdge)
        {
            path.push_back(viTemp); 
            indexPath++;
            xCoord += dx;
            viTemp[0] = int(round(xCoord));
            viTemp[1] = path[indexPath][1] + iy;
            if (viTemp[0] < 0 || viTemp[0] >= width) { offTheEdge = 1; }
            if (viTemp[1] < 0 || viTemp[1] >= height) { offTheEdge = 1; }
        }
    }
    return path;
}
vector<vector<unsigned char>> IMGFUNC::lineRGB(vector<vector<int>>& vVictor, int length)
{
    vector<vector<unsigned char>> Lrgb(length, vector<unsigned char>(3));
    vector<int> viTemp = vVictor[0];
    for (int ii = 0; ii < length; ii++)
    {
        viTemp[0] += vVictor[1][0];
        viTemp[1] += vVictor[1][1];
        Lrgb[ii] = pixelRGB(viTemp);
    }
    vVictor[0][0] = viTemp[0];
    vVictor[0][1] = viTemp[1];
    return Lrgb;
}
void IMGFUNC::loadRecentSavePoint(vector<vector<int>>& vBorderPath)
{
    if (savePoints.size() < 1) { jf.err("No saved points to load-im.loadRecentSavePoint"); }
    vector<int> pointWrong, pointRight, origin;
    double minDistance = 4294967295.0, dist;
    int minIndex = -1;
    for (int ii = savePoints.size() - 1; ii >= 0; ii--)
    {
        if (savePoints[ii][0].size() < 2)
        {
            savePoints.erase(savePoints.begin() + ii);
        }
        else
        {
            pointWrong = savePoints[ii][savePoints[ii].size() - 1];
            savePoints[ii].erase(savePoints[ii].begin() + savePoints[ii].size() - 1);
            for (int jj = 2; jj < savePoints[ii].size(); jj++)
            {
                dist = jf.coordDist(pointWrong, savePoints[ii][jj]);
                if (dist < minDistance)
                {
                    minDistance = dist;
                    minIndex = jj;
                }
            }
            savePoints[ii].erase(savePoints[ii].begin() + minIndex);
            if (savePoints[ii].size() != 3) { jf.err("savePoints-im.loadRecentSavePoint"); }
            pointRight = savePoints[ii][2];
            origin = savePoints[ii][1];
            savePoints.erase(savePoints.begin() + ii);
            break;
        }
    }
    if (pointRight.size() < 2) { jf.err("Failed to determine pointRight-im.loadRecentSavePoint"); }
    while (vBorderPath[vBorderPath.size() - 1] != origin)
    {
        vBorderPath.erase(vBorderPath.end() - 1);
        sizeVBP--;
    }
    vBorderPath.push_back(pointRight);
    pauseVBP = sizeVBP + 1;
}
bool IMGFUNC::mapIsInit()
{
    size_t size = mapColour.size();
    if (size > 0) { return 1; }
    return 0;
}
vector<vector<int>> IMGFUNC::minMaxPath2D(vector<vector<int>>& path)
{
    vector<vector<int>> TLBR(2, vector<int>(2));
    TLBR[0] = path[0];
    TLBR[1] = path[0];
    for (int ii = 1; ii < path.size(); ii++)
    {
        if (path[ii][0] < TLBR[0][0]) { TLBR[0][0] = path[ii][0]; }
        else if (path[ii][0] > TLBR[1][0]) { TLBR[1][0] = path[ii][0]; }
        if (path[ii][1] < TLBR[0][1]) { TLBR[0][1] = path[ii][1]; }
        else if (path[ii][1] > TLBR[1][1]) { TLBR[1][1] = path[ii][1]; }
    }
    return TLBR;
}
void IMGFUNC::octogonCheckBoundary(vector<vector<int>>& octoPath, vector<int>& sourceDim, int pathSpace)
{
    // If a proposed octogon path (plus pathSpace) would go past the source image
    // dimensions, then modify octoPath such that the boundaries are respected.
    vector<int> infractions = { 0, 0, 0, 0 };  // Top, bot, left, right.
    for (int ii = 0; ii < octoPath.size(); ii++)
    {
        if (octoPath[ii][0] - pathSpace < 0) { infractions[2] = 1; }
        else if (octoPath[ii][0] + pathSpace >= sourceDim[0]) { infractions[3] = 1; }
        if (octoPath[ii][1] - pathSpace < 0) { infractions[0] = 1; }
        else if (octoPath[ii][1] + pathSpace >= sourceDim[1]) { infractions[1] = 1; }
    }
    if (infractions[0] + infractions[1] + infractions[2] + infractions[3] == 0) { return; }

    for (int ii = 0; ii < 4; ii++)
    {
        if (infractions[ii] > 0)
        {
            switch (ii)
            {
            case 0:
            {
                for (int jj = 0; jj < octoPath.size(); jj++)
                {
                    if (octoPath[jj][1] - pathSpace < 0)
                    {
                        octoPath[jj][1] = pathSpace;
                    }
                }
                break;
            }
            case 1:
            {
                for (int jj = 0; jj < octoPath.size(); jj++)
                {
                    if (octoPath[jj][1] + pathSpace >= sourceDim[1])
                    {
                        octoPath[jj][1] = sourceDim[1] - 1 - pathSpace;
                    }
                }
                break;
            }
            case 2:
            {
                for (int jj = 0; jj < octoPath.size(); jj++)
                {
                    if (octoPath[jj][0] - pathSpace < 0)
                    {
                        octoPath[jj][0] = pathSpace;
                    }
                }
                break;
            }
            case 3:
            {
                for (int jj = 0; jj < octoPath.size(); jj++)
                {
                    if (octoPath[jj][0] + pathSpace >= sourceDim[0])
                    {
                        octoPath[jj][0] = sourceDim[0] - 1 - pathSpace;
                    }
                }
                break;
            }
            }
        }
    }

}
vector<vector<int>> IMGFUNC::octogonPath(vector<int> origin, int radius)
{
    vector<vector<int>> path(1, vector<int>(2));
    vector<int> dV;
    int lenPerp, lenDiag, index;
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
    path[0][0] = origin[0] + (radius / 2);
    path[0][1] = origin[1] - radius;

    dV = { 1, 1 };
    for (int ii = 0; ii < lenDiag; ii++)
    {
        index = path.size();
        path.push_back(vector<int>(2));
        path[index][0] = path[index - 1][0] + dV[0];
        path[index][1] = path[index - 1][1] + dV[1];
    }
    dV = { 0, 1 };
    for (int ii = 0; ii < lenPerp; ii++)
    {
        index = path.size();
        path.push_back(vector<int>(2));
        path[index][0] = path[index - 1][0] + dV[0];
        path[index][1] = path[index - 1][1] + dV[1];
    }
    dV = { -1, 1 };
    for (int ii = 0; ii < lenDiag; ii++)
    {
        index = path.size();
        path.push_back(vector<int>(2));
        path[index][0] = path[index - 1][0] + dV[0];
        path[index][1] = path[index - 1][1] + dV[1];
    }
    dV = { -1, 0 };
    for (int ii = 0; ii < lenPerp; ii++)
    {
        index = path.size();
        path.push_back(vector<int>(2));
        path[index][0] = path[index - 1][0] + dV[0];
        path[index][1] = path[index - 1][1] + dV[1];
    }
    dV = { -1, -1 };
    for (int ii = 0; ii < lenDiag; ii++)
    {
        index = path.size();
        path.push_back(vector<int>(2));
        path[index][0] = path[index - 1][0] + dV[0];
        path[index][1] = path[index - 1][1] + dV[1];
    }
    dV = { 0, -1 };
    for (int ii = 0; ii < lenPerp; ii++)
    {
        index = path.size();
        path.push_back(vector<int>(2));
        path[index][0] = path[index - 1][0] + dV[0];
        path[index][1] = path[index - 1][1] + dV[1];
    }
    dV = { 1, -1 };
    for (int ii = 0; ii < lenDiag; ii++)
    {
        index = path.size();
        path.push_back(vector<int>(2));
        path[index][0] = path[index - 1][0] + dV[0];
        path[index][1] = path[index - 1][1] + dV[1];
    }
    dV = { 1, 0 };
    for (int ii = 0; ii < lenPerp; ii++)
    {
        index = path.size();
        path.push_back(vector<int>(2));
        path[index][0] = path[index - 1][0] + dV[0];
        path[index][1] = path[index - 1][1] + dV[1];
    }
    return path;
}
void IMGFUNC::pauseMapDebug(SWITCHBOARD& sbgui, vector<vector<int>>& tracks, int radius, vector<vector<int>>& candidates)
{
    if (pathMapDebug.size() > 0)
    {
        int inum;
        vector<int> userCoord;
        vector<string> vsTemp = { pathMapDebug, to_string(sizeVBP), pathActivePNG };
        makeMapBorderFindNext(tracks, radius, candidates, vsTemp[0]);
        sbgui.set_prompt(vsTemp);
        pngToBinPause(sbgui);
        vsTemp = sbgui.get_prompt();
        if (vsTemp.size() == 1)  // Advance to prompt's index.
        {
            try { inum = stoi(vsTemp[0]); }
            catch (invalid_argument& ia) { jf.err("stoi-im.pauseMapDebug"); }
            if (inum >= 0) { pauseVBP = sizeVBP + inum; }
            else 
            { 
                sizeVBP += inum; 
                sizeVBP *= -1;
            }
            vsTemp = { "" };
            sbgui.set_prompt(vsTemp);
        }
        else if (vsTemp.size() == 2)  // Select the chosen candidate, and resume.
        {
            userCoord = jf.destringifyCoord(vsTemp[1]);
            if (userCoord != tracks[tracks.size() - 1])
            {
                for (int ii = candidates.size() - 1; ii >= 0; ii--)
                {
                    if (candidates[ii] != userCoord)
                    {
                        candidates.erase(candidates.begin() + ii);
                    }
                }
            }
            vsTemp = { "" };
            sbgui.set_prompt(vsTemp);
        }
        else if (vsTemp.size() == 3)  // Select the chosen pixel, discarding 
        {                             // all generated candidates. Advance 1.
            try { inum = stoi(vsTemp[0]); }
            catch (invalid_argument& ia) { jf.err("stoi-im.pauseMapDebug"); }
            pauseVBP = sizeVBP + inum;
            userCoord = jf.destringifyCoord(vsTemp[2]);
            candidates.resize(1);
            candidates[0] = userCoord;
            vsTemp = { "" };
            sbgui.set_prompt(vsTemp);
        }
    }
    pathMapDebug.clear();
}
string IMGFUNC::pixelDecToHex(vector<unsigned char>& rgb)
{
    string shex, temp;
    for (int ii = 0; ii < rgb.size(); ii++)
    {
        temp = jf.decToHex(rgb[ii]);
        shex += temp;
    }
    return shex;
}
void IMGFUNC::pixelPaint(vector<unsigned char>& img, int widthImg, vector<unsigned char> rgb, vector<int> coord)
{
    if (coord[0] < 0 || coord[1] < 0 || coord[0] >= widthImg) { return; }
    int offset = getOffset(coord, widthImg);
    if (offset + 2 >= img.size()) { return; }
    img[offset + 0] = rgb[0];
    img[offset + 1] = rgb[1];
    img[offset + 2] = rgb[2];
}
vector<unsigned char> IMGFUNC::pixelRGB(vector<int>& coord)
{
	if (dataPNG.size() < 1) { jf.err("No loaded image-im.pixelRGB"); } 
	if (coord[0] < 0 || coord[0] >= width) { jf.err("xCoord out of bounds-im.pixelRGB"); }
    if (coord[1] < 0 || coord[1] >= height) { jf.err("yCoord out of bounds-im.pixelRGB"); }
    vector<unsigned char> rgb(3);
    int offset = getOffset(coord);
	rgb[0] = dataPNG[offset + 0];
	rgb[1] = dataPNG[offset + 1];
	rgb[2] = dataPNG[offset + 2];
	return rgb;
}
string IMGFUNC::pixelZone(vector<unsigned char>& rgb)
{
    string szone;
    string shex = pixelDecToHex(rgb);
    try { szone = mapColour.at(shex); }
    catch (out_of_range& oor) { szone = "unknown"; }
    return szone;
}
vector<unsigned char> IMGFUNC::pngBlankCanvas(vector<int>& dim)
{
    int size = dim[0] * dim[1] * 3;
    vector<unsigned char> canvas(size, 255);
    return canvas;
}
vector<unsigned char> IMGFUNC::pngExtractRow(int row, vector<unsigned char>& img, vector<int>& sourceDim)
{
    if (row >= sourceDim[1] || row < 0) { jf.err("Row outside boundaries-im.pngExtractRow"); }
    vector<unsigned char> vRow(3 * sourceDim[0]);
    vector<int> viTemp(2, 0);
    viTemp[1] = row;
    int offset = getOffset(viTemp, sourceDim[0]);
    for (int ii = 0; ii < vRow.size(); ii++)
    {
        vRow[ii] = img[offset + ii];
    }
    return vRow;
}
void IMGFUNC::pngLoad(string& pathPNG)
{
	unsigned char* dataTemp = stbi_load(pathPNG.c_str(), &width, &height, &numComponents, 0);
    if (dataTemp == NULL)
    {
        auto errorSTB = stbi_failure_reason();
        int bbq = 1;
    }
    int sizeTemp = width * height * numComponents;
	dataPNG.resize(sizeTemp);
	copy(dataTemp, dataTemp + sizeTemp, dataPNG.begin());
    pathActivePNG = pathPNG;
    pauseVBP = defaultPathLengthImageDebug;
}
void IMGFUNC::pngToBin(SWITCHBOARD& sbgui, string& pathPNG, string& pathBIN)
{
    // Returns the border path generated, for easy display/human review.
    if (!mapIsInit()) { initMapColours(); }
    pngLoad(pathPNG);
    string temp;
    string pathMapPTB = sroot + "\\debug\\PTB.png";
    string pathMapPTBdebug = sroot + "\\debug\\PTBdebug.png";
    vector<string> vsTemp;
    vector<int> viTemp;
    vector<vector<double>> corners = frameCorners();
    vector<vector<int>> vBorderPath(1, vector<int>());
    vBorderPath[0] = borderFindStart();
    vector<vector<int>> tracks, commGui;
    thread::id myid = this_thread::get_id();
    vector<int> myComm = sbgui.getMyComm(myid);
    int inum;
    sizeVBP = 1;
    savePoints.clear();
    while (1)
    {
        if (sizeVBP > defaultTracksLength)
        {
            inum = vBorderPath.size();
            tracks.assign(vBorderPath.begin() + inum - defaultTracksLength, vBorderPath.end());
        }
        else
        {
            tracks = vBorderPath;
        }
        commGui = sbgui.update(myid, myComm);
        if (commGui[0][0] == 3)
        {
            viTemp = { width, height };
            //std::thread ptb(&IMGFUNC::thrMakeMapPTB, this, vBorderPath, pathMapPTB, dataPNG, viTemp);
            //ptb.detach();
            pathMapDebug = pathMapPTBdebug;
        }
        vBorderPath.push_back(borderFindNext(sbgui, tracks));
        if (backtrack)
        {
            backtrack = 0;
            if (sizeVBP >= 0)
            {
                loadRecentSavePoint(vBorderPath);
                if (sizeVBP > defaultTracksLength)
                {
                    inum = vBorderPath.size();
                    tracks.assign(vBorderPath.begin() + inum - defaultTracksLength - 1, vBorderPath.begin() + inum - 1);
                }
                else
                {
                    tracks = vBorderPath;
                    tracks.pop_back();
                }
            }
            else
            {
                sizeVBP *= -1;
                pauseVBP = sizeVBP + 1;
                while (vBorderPath.size() > sizeVBP + 1)
                {
                    vBorderPath.pop_back();
                }
                tracks.clear();
                if (sizeVBP > defaultTracksLength)
                {
                    inum = vBorderPath.size();
                    tracks.assign(vBorderPath.begin() + inum - defaultTracksLength - 1, vBorderPath.begin() + inum - 1);
                }
                else
                {
                    tracks = vBorderPath;
                    tracks.pop_back();
                }
            }
        }
        if (vBorderPath[vBorderPath.size() - 1][0] < 0 || vBorderPath[vBorderPath.size() - 1][1] < 0)
        {
            jf.err("vBorderPath error-im.pngToBin");
        }
        if (savePoints.size() > 0 && savePoints[savePoints.size() - 1][0].size() < 2)
        {   // Repeating sizeVBP signals that the chosen point was appended to the list.
            savePoints[savePoints.size() - 1][0].push_back(savePoints[savePoints.size() - 1][0][0]);
            savePoints[savePoints.size() - 1].push_back(vBorderPath[vBorderPath.size() - 1]);
        }
        sizeVBP++;
        searchRadiusIncrease = 0;
        rabbitHole = 0;
        if (sizeVBP > 10)
        {
            if (jobsDone(vBorderPath[vBorderPath.size() - 1])) 
            {
                vBorderPath.push_back(vBorderPath[0]);
                break; 
            }
        }
        if (sizeVBP == pauseVBP)
        {
            viTemp = { width, height };
            //std::thread ptb(&IMGFUNC::thrMakeMapPTB, this, vBorderPath, pathMapPTB, dataPNG, viTemp);
            //ptb.detach();
            pathMapDebug = pathMapPTBdebug;
        }
    }

    ofstream sPrinter(pathBIN.c_str(), ios::trunc);
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
    sPrinter.close();
}
void IMGFUNC::pngToBinLive(SWITCHBOARD& sbgui, vector<vector<double>>& border)
{
    jf.timerStart();
    vector<int> mycomm;
    vector<vector<int>> comm_gui;
    thread::id myid = this_thread::get_id();
    sbgui.answer_call(myid, mycomm);
    vector<string> prompt = sbgui.get_prompt();
    bool success, frameDone;

    if (!mapIsInit()) { initMapColours(); }
    pngLoad(prompt[0]);
    vector<vector<double>> corners = frameCorners();
    size_t pos1 = prompt[2].find(',');
    string temp = prompt[2].substr(0, pos1);
    string temp2 = prompt[2].substr(pos1 + 1);
    double widthLabel, heightLabel;
    try
    {
        widthLabel = stod(temp);
        heightLabel = stod(temp2);
    }
    catch (invalid_argument& ia) { jf.err("stod-im.pngToBinLive"); }
    double widthDemSup = (double)width / widthLabel;
    double heightDemSup = (double)height / heightLabel;
    if (widthDemSup > heightDemSup)
    {
        stretchFactor = 1.0 / widthDemSup;
    }
    else
    {
        stretchFactor = 1.0 / heightDemSup;
    }
    success = sbgui.push(myid);
    if (success)
    {
        border = corners;
        border.push_back({ stretchFactor });
        success = sbgui.done(myid);
        if (!success) { jf.err("sbgui.done-im.pngToBinLive"); }
        frameDone = 0;
        mycomm[1] = 1;
        sbgui.update(myid, mycomm);
    }

    vector<vector<int>> vBorderPath(1, vector<int>());
    vBorderPath[0] = borderFindStart();
    vector<vector<int>> tracks;
    int sizeVBP = 1;
    while (1)
    {
        if (sizeVBP > 6)
        {
            tracks[0] = tracks[1];
            tracks[1] = tracks[2];
            tracks[2] = tracks[3];
            tracks[3] = tracks[4];
            tracks[4] = tracks[5];
            tracks[5] = vBorderPath[vBorderPath.size() - 1];
        }
        else
        {
            tracks = vBorderPath;
        }
        vBorderPath.push_back(borderFindNext(sbgui, tracks));
        if (vBorderPath[vBorderPath.size() - 1][0] < 0 || vBorderPath[vBorderPath.size() - 1][1] < 0)
        {
            jf.err("vBorderPath error-im.pngToBinLive");
        }
        sizeVBP++;
        searchRadiusIncrease = 0;
        rabbitHole = 0;
        success = sbgui.push(myid);
        if (success)
        {
            if (frameDone)
            {
                jf.toDouble(vBorderPath, border);
                success = sbgui.done(myid);
                if (!success) { jf.err("sbgui.done-im.pngToBinLive"); }
            }
            else if (border.size() == 0)
            {
                frameDone = 1;
                jf.toDouble(vBorderPath, border);
                success = sbgui.done(myid);
                if (!success) { jf.err("sbgui.done-im.pngToBinLive"); }
            }
        }
        if (sizeVBP > 10)
        {
            if (jobsDone(vBorderPath[vBorderPath.size() - 1])) { break; }
        }
    }

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

    mycomm[0] = 1;
    sbgui.update(myid, mycomm);
    long long timer = jf.timerStop();
    jf.logTime("Converted " + prompt[0] + " to BIN coordinates", timer);
}
void IMGFUNC::pngToBinPause(SWITCHBOARD& sbgui)
{
    int statusGui;
    thread::id myid = this_thread::get_id();
    vector<int> myComm = sbgui.getMyComm(myid);
    myComm[0] = 3;
    vector<vector<int>> commGui = sbgui.update(myid, myComm);
    if (commGui[0][0] == 0) { statusGui = 0; }
    else if (commGui[0][0] == 3) { statusGui = 1; }
    else { jf.err("commGui-im.pngToBinPause"); }

    while (statusGui < 2)
    {
        this_thread::sleep_for(55ms);
        commGui = sbgui.update(myid, myComm);
        if (statusGui == 0 && commGui[0][0] == 3) { statusGui = 1; }
        else if (statusGui == 1 && commGui[0][0] == 0) { statusGui = 2; }
    }
    myComm[0] = 0;
    sbgui.update(myid, myComm);
}
void IMGFUNC::removeColourCushion(vector<vector<unsigned char>>& Lrgb, vector<unsigned char> colourCore, vector<unsigned char> colourCushion, int length)
{
    bool onCore;
    int inum;
    vector<unsigned char> pixel;
    if (Lrgb[0] == colourCore) { onCore = 1; }
    else { onCore = 0; }
    for (int ii = 1; ii < Lrgb.size(); ii++)
    {
        if (Lrgb[ii] == colourCore && onCore == 0)
        {
            onCore = 1;
            inum = min(length, ii);
            for (int jj = 1; jj <= inum; jj++)
            {
                if (Lrgb[ii - jj] == colourCushion) { Lrgb[ii - jj] = colourCore; }
            }
        }
        else if (Lrgb[ii] != colourCore && onCore == 1)
        {
            onCore = 0;
            inum = min(length, (int)Lrgb.size() - 1 - ii);
            for (int jj = 1; jj <= inum; jj++)
            {
                if (Lrgb[ii + jj] == colourCushion) { Lrgb[ii + jj] = colourCore; }
            }
            ii += inum;
        }
    }

}
void IMGFUNC::saveThisPoint(vector<vector<int>>& tracks, vector<vector<int>>& candidates)
{
    vector<vector<int>> savePoint(1, vector<int>(1));
    savePoint[0] = { sizeVBP };
    savePoint.push_back(tracks[tracks.size() - 1]);
    for (int ii = 0; ii < candidates.size(); ii++)
    {
        savePoint.push_back(candidates[ii]);
    }
    savePoints.push_back(savePoint);
}
void IMGFUNC::setExtractDim(vector<int> extractDim)
{
    defaultExtractDim = extractDim;
}
void IMGFUNC::setPauseVBP(int iLen)
{
    if (iLen > 0) { pauseVBP = iLen; }
    else { pauseVBP = defaultPathLengthImageDebug; }
}
int IMGFUNC::testBacktrack(vector<vector<int>>& tracks, vector<vector<int>>& candidates)
{
    double dist, nearest;
    double radius = (double)defaultSearchRadius / 2.0;
    for (int ii = candidates.size() - 1; ii >= 0; ii--)
    {
        nearest = 4294967295.0;
        for (int jj = 0; jj < tracks.size(); jj++)
        {
            dist = jf.coordDist(candidates[ii], tracks[jj]);
            if (dist < nearest)
            {
                nearest = dist;
            }
        }
        if (nearest < radius)
        {
            candidates.erase(candidates.begin() + ii);
        }
    }
    return candidates.size();
}
int IMGFUNC::testCandidatesInteriorZone(SWITCHBOARD& sbgui, vector<vector<int>>& tracks, string sZone, vector<vector<int>>& candidates)
{
    // Test two candidates for their access to a desired sZone. Attempts to 
    // eliminate one candidate. Return value is number of candidates remaining.
    if (candidates.size() != 2) { jf.err("Not given two candidates-im.testCandidatesInteriorZone"); }
    vector<double> bearingsInterior(2, -1.0);
    vector<int> widthZone(2);
    vector<int> radiusNeeded(2, defaultSearchRadius);
    double widthRatio;
    vector<vector<int>> pastPresent(2, vector<int>(2));
    pastPresent[0] = tracks[tracks.size() - 1];
    pastPresent[1] = candidates[0];
    octogonBearing(sbgui, pastPresent, sZone, radiusNeeded[0], bearingsInterior[0], widthZone[0]);
    pastPresent[1] = candidates[1];
    octogonBearing(sbgui, pastPresent, sZone, radiusNeeded[1], bearingsInterior[1], widthZone[1]);
    int radialDiff = abs(radiusNeeded[0] - radiusNeeded[1]);
    if (radialDiff > defaultZoneRadialDistanceTolerance)
    {
        if (radiusNeeded[0] < radiusNeeded[1])
        {
            candidates.erase(candidates.begin() + 1);
            return 1;
        }
        else
        {
            candidates.erase(candidates.begin() + 0);
            return 1;
        }
    }
    vector<int> goClock(2, 0);
    if (bearingsInterior[0] >= 0.0 && bearingsInterior[1] < 0.0)
    {
        candidates.erase(candidates.begin() + 1);
        return 1;
    }
    else if (bearingsInterior[1] >= 0.0 && bearingsInterior[0] < 0.0)
    {
        candidates.erase(candidates.begin() + 0);
        return 1;
    }
    else if (bearingsInterior[0] >= 0.0 && bearingsInterior[1] >= 0.0)
    {
        widthRatio = (double)widthZone[0] / (double)widthZone[1];
        if (widthRatio < 1.0)
        {
            widthRatio = 1.0 / widthRatio;
        }
        if (bearingsInterior[0] >= 0.0 && bearingsInterior[0] < 180.0) { goClock[0] = 1; }
        if (bearingsInterior[1] >= 0.0 && bearingsInterior[1] < 180.0) { goClock[1] = 1; }
        
        if (goClock[0] + goClock[1] == 1)  // If exactly one candidate goes clockwise and
        {                                  // the other goes counterclockwise...
            double percentClock = clockwisePercentage(sbgui, tracks, sZone);
            if (percentClock > 0.5)
            {
                if (goClock[0] == 1) 
                {
                    candidates.erase(candidates.begin() + 1);
                    return 1;
                }
                else if (goClock[1] == 1) 
                {
                    candidates.erase(candidates.begin() + 0);
                    return 1;
                }
            }
            else
            {
                if (goClock[0] == 0) 
                {
                    candidates.erase(candidates.begin() + 1);
                    return 1;
                }
                else if (goClock[1] == 0) 
                {
                    candidates.erase(candidates.begin() + 0);
                    return 1;
                }
            }
        }
        else if (widthRatio > defaultWidthTestRatio)
        {
            if (widthZone[0] > widthZone[1]) 
            {
                candidates.erase(candidates.begin() + 1);
                return 1;
            }
            else
            {
                candidates.erase(candidates.begin() + 0);
                return 1;
            }
        }
    }
    return 2;
}
void IMGFUNC::testCenterOfMass(vector<vector<int>>& tracks, vector<vector<int>>& candidates)
{
    double dTemp;
    vector<double> angles(candidates.size());
    vector<vector<int>> pastPresentFuture(3, vector<int>(2));
    pastPresentFuture[0] = { width / 2, height / 2 };  // Center of map.
    pastPresentFuture[1] = tracks[tracks.size() - 1];  // Origin.
    for (int ii = 0; ii < candidates.size(); ii++)
    {
        pastPresentFuture[2] = candidates[ii];
        dTemp = jf.angleBetweenVectors(pastPresentFuture);
        angles[ii] = abs(dTemp - 180.0);  // Because the center serves as 'past',
    }                                     // an angle of 180 degrees is ideal.
    vector<double> anglesSorted = { angles[0] };
    int inum = anglesSorted.size();
    for (int ii = 1; ii < angles.size(); ii++)
    {
        for (int jj = 0; jj < inum; jj++)
        {
            if (angles[ii] < anglesSorted[jj])
            {
                anglesSorted.insert(anglesSorted.begin() + jj, angles[ii]);
                break;
            }
            else if (jj == inum - 1)
            {
                anglesSorted.insert(anglesSorted.end(), angles[ii]);
            }
        }
    }
    if (anglesSorted[1] - anglesSorted[0] < defaultCenterOfMassTolerance) { return; }
    
    vector<int> winner;
    for (int ii = 0; ii < angles.size(); ii++)
    {
        if (angles[ii] == anglesSorted[0])
        {
            winner = candidates[ii];
            break;
        }
    }
    for (int ii = candidates.size() - 1; ii >= 0; ii--)
    {
        if (candidates[ii] != winner)
        {
            candidates.erase(candidates.begin() + ii);
        }
    }
    int bbq = 1;
}
void IMGFUNC::testDistances(vector<vector<int>>& candidates, vector<double>& distances)
{
    if (candidates.size() != distances.size()) { jf.err("Size mismatch-im.testDistances"); }
    double thisTall = (double)defaultSearchRadius / 2.0;
    for (int ii = distances.size() - 1; ii >= 0; ii--)
    {
        if (distances[ii] < thisTall)
        {
            candidates.erase(candidates.begin() + ii);
            distances.erase(distances.begin() + ii);
        }
    }
}
int IMGFUNC::testHereBeDragons(vector<vector<unsigned char>> Lrgb, vector<int>& borderCoord, unordered_map<string, int>& mapIndexCandidate)
{
    // Given a ring of colours to scan and a single border point, 
    // determine if we have reached the edge of the map.
    // Returns 0 = not on map edge, 1 = map edge
    string temp = jf.stringifyCoord(borderCoord);
    string szone;
    int inum, index, indexFinal, iScore = 0;
    try { index = mapIndexCandidate.at(temp); }
    catch (out_of_range& oor) { jf.err("mapIndexCandidate-im.testHereBeDragons"); }
    indexFinal = (index - 1) % Lrgb.size();
    while (index != indexFinal)
    {
        szone = pixelZone(Lrgb[index]);
        if (szone != "unknown" && szone != "zoneBorder")
        {
            if (iScore == 0)
            {
                if (szone == "water")
                {
                    iScore = 1;
                }
                else if (szone == "roadSmall")
                {
                    iScore = 2;
                }
                else if (szone == "zoneOutside")
                {
                    iScore = 3;
                }
            }
            else if (iScore < 10)
            {
                if (szone == "black")
                {
                    iScore += 10;
                }
                else if (szone == "blottedBlack")
                {
                    iScore += 20;
                }
            }
            else if (iScore < 100)
            {
                if (szone == "white")
                {
                    iScore += 100;
                }
            }
            else if (iScore < 1000)
            {
                inum = iScore % 100;
                if (szone == "black" && inum < 20)
                {
                    iScore += 1000;
                }
                else if (szone == "blottedBlack" && inum > 20)
                {
                    iScore += 2000;
                }
            }
            else if (iScore < 10000)
            {
                inum = iScore % 10;
                if (szone == "water" && inum == 1)
                {
                    iScore += 10000;
                }
                else if (szone == "roadSmall" && inum == 2)
                {
                    iScore += 20000;
                }
                else if (szone == "zoneOutside" && inum == 3)
                {
                    iScore += 30000;
                }
            }
        }
        index = (index + 1) % Lrgb.size();
    }
    if (iScore > 10000) 
    {
        return 1; 
    }
    return 0;
}
int IMGFUNC::testOverWater(vector<vector<int>>& tracks, vector<vector<int>>& candidates)
{
    // Test returns TRUE if it detects a minimum of two water zones, 
    // as well as having a high percentage of water pixels in the ring.
    string sZone = "water";
    vector<vector<int>> vviTemp(1, vector<int>(2));
    vviTemp[0] = tracks[tracks.size() - 1];
    vviTemp.insert(vviTemp.end(), candidates.begin(), candidates.end());
    unordered_map<string, int> mapIndexCandidate;
    vector<int> intervalSize;
    vector<vector<int>> octoPath, lightHouse;
    vector<vector<unsigned char>> octoRGB;
    double percentage; 
    for (int ii = 0; ii < vviTemp.size(); ii++)
    {
        octoPath = octogonPath(vviTemp[ii], (2 * borderThickness));
        octoRGB = octogonRGB(octoPath);
        lightHouse = zoneSweep(sZone, octoRGB, octoPath, mapIndexCandidate);
        if (lightHouse.size() > 1)
        {
            percentage = zoneSweepPercentage(sZone, octoRGB);
            if (percentage > defaultWaterPercentage) { return 1; }
        }
    }
    return 0;
}
int IMGFUNC::testTextHumanFeature(vector<vector<unsigned char>>& Lrgb)
{
    string test = "textHumanFeature";
    string pixelColour;
    for (int ii = 0; ii < Lrgb.size(); ii++)
    {
        pixelColour = pixelZone(Lrgb[ii]);
        if (pixelColour == test) { return 1; }
    }
    return 0;
}
vector<int> IMGFUNC::testZoneLength(vector<vector<int>>& pastPresent, vector<vector<int>>& candidates, string sZone)
{
    // Returns the candidate with the most sZone pixels along its vector from the origin.
    // The length of every vector scanned is given by the shortest candidate vector length,
    // if they were all permitted to travel to the edge of the PNG. Returned candidate is 
    // of the form [xCoord, yCoord, runnerup's relative length (percent)].
    int numCandidates = candidates.size();
    vector<vector<vector<int>>> candidatePaths(numCandidates, vector<vector<int>>());
    vector<int> lengthToEdge(numCandidates);
    vector<int> shortestToEdge(2, -1);  // Form [index, length].
    shortestToEdge[1] = int(jf.hypoteneuse(width, height));
    vector<vector<int>> vviTemp(2, vector<int>(2));
    vviTemp[0] = pastPresent[1];

    // Obtain every candidate path to its respective edge. 
    for (int ii = 0; ii < numCandidates; ii++)
    {
        vviTemp[1] = candidates[ii];
        candidatePaths[ii] = linePathToEdge(vviTemp);
        lengthToEdge[ii] = candidatePaths[ii].size();
        if (lengthToEdge[ii] < shortestToEdge[1])
        {
            shortestToEdge[0] = ii;
            shortestToEdge[1] = lengthToEdge[ii];
        }
    }
    
    // Standardize all paths to the same (shortest) length.
    for (int ii = 0; ii < numCandidates; ii++)
    {
        if (lengthToEdge[ii] > shortestToEdge[1])
        {
            candidatePaths[ii].erase(candidatePaths[ii].begin() + shortestToEdge[1], candidatePaths[ii].end());
        }
    }

    // Obtain the number of sZone pixels along each path.
    vector<int> sZonesNum(numCandidates, 0);
    vector<unsigned char> rgb;
    string sz;
    for (int ii = 0; ii < numCandidates; ii++)
    {
        for (int jj = 0; jj < candidatePaths[ii].size(); jj++)
        {
            rgb = pixelRGB(candidatePaths[ii][jj]);
            sz = pixelZone(rgb);
            if (sz == sZone) { sZonesNum[ii]++; }
        }
    }

    // Crown a candidate.
    vector<int> winner(3);
    int frontrunner = -1;
    int winnerLen = -1;
    int runnerupLen = -1;
    for (int ii = 0; ii < numCandidates; ii++)
    {
        if (sZonesNum[ii] > winnerLen)
        {
            runnerupLen = winnerLen;
            winnerLen = sZonesNum[ii];
            frontrunner = ii;
        }
        else if (sZonesNum[ii] > runnerupLen)
        {
            runnerupLen = sZonesNum[ii];
        }
    }
    winner[0] = candidates[frontrunner][0];
    winner[1] = candidates[frontrunner][1];
    winner[2] = (100 * runnerupLen) / winnerLen;
    return winner;
}
int IMGFUNC::testZoneSweepLetters(vector<vector<int>>& zonePath, vector<vector<unsigned char>>& Lrgb, vector<vector<int>>& candidates, unordered_map<string, int>& mapIndexCandidate)
{
    // Candidates between two map letter colours (textHumanFeature) are removed.
    // Returns the number of candidates remaining after the test is applied. 
    vector<int> indexDeathRow;
    int indexZone, indexCandidate;
    bool crossOver, leftBound, letMeOut;
    string sZone, sCandidate;
    for (int ii = 0; ii < candidates.size(); ii++)
    {
        crossOver = 0;
        leftBound = 0;
        letMeOut = 0;
        sCandidate = to_string(candidates[ii][0]) + "," + to_string(candidates[ii][1]);
        try { indexCandidate = mapIndexCandidate.at(sCandidate); }
        catch (out_of_range& oor) { jf.err("candidate map-im.testZoneSweepLetters"); }
        sZone = pixelZone(Lrgb[indexCandidate]);
        if (sZone != "white") { jf.err("invalid candidate pixel-im.testZoneSweepLetters"); }
        indexZone = indexCandidate;
        while (1)
        {
            indexZone -= 1;
            if (indexZone < 0)
            {
                if (crossOver > 0) { jf.err("infinite loop-im.testZoneSweepLetters"); }
                crossOver = 1;
                indexZone += zonePath.size();
            }
            sZone = pixelZone(Lrgb[indexZone]);
            if (sZone == "unknown") { continue; }
            else if (sZone == "white") { continue; }
            else if (sZone == "textHumanFeature")
            {
                leftBound = 1;
                break;
            }
            else
            {
                letMeOut = 1;
                break;
            }
        }
        if (letMeOut) { continue; }
        indexZone = indexCandidate;
        crossOver = 0;
        while (1)
        {
            indexZone += 1;
            if (indexZone >= zonePath.size())
            {
                if (crossOver > 0) { jf.err("infinite loop-im.testZoneSweepLetters"); }
                crossOver = 1;
                indexZone -= zonePath.size();
            }
            sZone = pixelZone(Lrgb[indexZone]);
            if (sZone == "unknown") { continue; }
            else if (sZone == "white") { continue; }
            else if (sZone == "textHumanFeature")
            {
                if (leftBound)
                {
                    indexDeathRow.push_back(ii);
                    break;
                }
                else { jf.err("no left boundary-im.testTextHumanFeature"); }
            }
            else
            {
                break;
            }
        }
    }
    for (int ii = indexDeathRow.size() - 1; ii >= 0; ii--)
    {
        candidates.erase(candidates.begin() + indexDeathRow[ii]);
    }
    return (int)candidates.size();
}
void IMGFUNC::thrMakeMapPTB(vector<vector<int>> vBorderPath, string outputPath, vector<unsigned char> source, vector<int> sourceDim)
{
    // Note that the bands are Red->Yellow->Green->Teal->Blue->Pink
    int widthDot = 5;        
    int numDots = vBorderPath.size();
    string avoidColour = "Blue";
    vector<vector<unsigned char>> colours = getColourSpectrum(numDots, avoidColour);
    for (int ii = 0; ii < numDots; ii++)
    {
        dotPaint(vBorderPath[ii], colours[ii], source, sourceDim[0], widthDot);
    }
    pngPrint(source, sourceDim, outputPath);
}
vector<vector<int>> IMGFUNC::zoneChangeLinear(vector<string>& szones, vector<vector<int>>& ivec)
{
    // This function returns the coordinates of the specified zone change,
    // by travelling along a given straight line. 
    // Form(szones): [pre-border szone, post-border szone]. Note: indepdendent of starting zone.
    // Form(ivec): [0][starting xCoord, starting yCoord], [1][delta x, delta y]. Note: delta can be negative, but coords cannot be.
    // Form(return): [0][final pre-border xCoord, final pre-border yCoord], [1][first post-border xCoord, first post-border yCoord].
    // The szone "known" counts as every saved szone except "unknown". 

    vector<vector<int>> vBorder(2, vector<int>(2));
    bool coord = 1;
    int dx = ivec[1][0];
    int dy = ivec[1][1];
    vector<int> coordA = ivec[0];
    vector<int> coordB(2);
    vector<int> coordOld = ivec[0];
    string temp, zoneOld, zoneNew;
    vector<unsigned char> rgb = pixelRGB(coordA);
    zoneNew = pixelZone(rgb);
    coordB[0] = coordA[0] + dx;
    coordB[1] = coordA[1] + dy;
    rgb = pixelRGB(coordB);
    while (rgb.size() == 3)
    {
        // We ignore the nonconformant colours produced by antialiasing.
        temp = pixelZone(rgb);
        if (temp != zoneNew && temp != "unknown")
        {
            // New zone!
            zoneOld = zoneNew;
            zoneNew = temp;
            if (szones[1] == "known" || (zoneOld == szones[0] && zoneNew == szones[1]))
            {
                // Objective found!
                vBorder[0] = coordOld;
                if (coord)
                {
                    vBorder[1] = coordB;
                }
                else
                {
                    vBorder[1] = coordA;
                }
                return vBorder;
            } 
        }

        // Same zone as before, or the new zone is not the goal. Record the coords, 
        // hoping each time the next leap will be the leap home... 
        if (temp != "unknown")
        {
            if (coord)
            {
                coordOld = coordB;
            }
            else
            {
                coordOld = coordA;
            }
        }

        // Hit the golf ball downrange, then chase after it...
        if (coord)
        {
            coordA[0] = coordB[0] + dx;
            if (coordA[0] < 0 || coordA[0] >= width) { break; }
            coordA[1] = coordB[1] + dy;
            if (coordA[1] < 0 || coordA[1] >= height) { break; }
            coord = 0;
            rgb = pixelRGB(coordA);
        }
        else
        {
            coordB[0] = coordA[0] + dx;
            if (coordB[0] < 0 || coordB[0] >= width) { break; }
            coordB[1] = coordA[1] + dy;
            if (coordB[1] < 0 || coordB[1] >= height) { break; }
            coord = 1;
            rgb = pixelRGB(coordB);
        }
    }

    // Function failed to locate the specified border.

    vBorder.resize(0);
    return vBorder;
}
double IMGFUNC::zoneSweepPercentage(string sZone, vector<vector<unsigned char>>& Lrgb)
{
    // For a given zone type, return its representation percentage from
    // a list of RGB pixels.
    string sZoneTemp;
    string border = "zoneBorder";
    int total = Lrgb.size();
    int count = 0, countNonBorder = 0;
    for (int ii = 0; ii < total; ii++)
    {
        sZoneTemp = pixelZone(Lrgb[ii]);
        if (sZoneTemp == sZone) { count++; }
        if (sZoneTemp != border) { countNonBorder++; }
    }
    double percentage = (double)count / (double)countNonBorder;
    return percentage;
}
