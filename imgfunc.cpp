#include "imgfunc.h"

vector<int> IMGFUNC::borderFindNext(SWITCHBOARD& sbgui, vector<vector<int>> tracks)
{
    vector<int> origin = tracks[tracks.size() - 1];
    int radius = 10 * borderThickness;
    vector<int> originRadius = origin;
    originRadius.push_back(radius);
    vector<vector<int>> octoPath = octogonPath(origin, radius);
    vector<vector<unsigned char>> octoRGB = octogonRGB(octoPath);
    if (debug) { octogonPaint(origin, radius); }
    string sZone = "zoneBorder";
    vector<vector<int>> candidates = zoneSweep(sZone, octoRGB, octoPath);
    vector<vector<int>> vvTemp, cPath;
    while (candidates.size() < 2)
    {
        radius += (4 * borderThickness);
        octoPath = octogonPath(origin, radius);
        octoRGB = octogonRGB(octoPath);
        if (debug) { octogonPaint(origin, radius); }
        candidates = zoneSweep(sZone, octoRGB, octoPath, originRadius);
    }
    if (tracks.size() < 2) { return candidates[0]; }
    vector<int> originPast = tracks[tracks.size() - 2];
    vector<double> distances;
    coordDist(originPast, candidates, distances);
    double minDistance, dTemp;
    int elimIndex;

    do
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
    } while (candidates.size() > 2);
    if (candidates.size() == 1) { return candidates[0]; }

    // There can only be two candidates remaining.  
    // Perform a series of tests to determine the right path.
    vector<int> interiorCounts(2);
    sZone = "white";
    vvTemp.resize(2);
    vvTemp[0] = tracks[tracks.size() - 1];
    vvTemp[1] = candidates[0];
    vector<double> bearings0 = octogonBearing(sbgui, vvTemp, sZone, radius / 4);
    interiorCounts[0] = bearings0.size();
    vvTemp[1] = candidates[1];
    vector<double> bearings1 = octogonBearing(sbgui, vvTemp, sZone, radius / 4);
    interiorCounts[1] = bearings1.size();
    vector<int> goClock(2, 0);
    if (interiorCounts[0] == 0 && interiorCounts[1] > 0)
    {
        return candidates[1];
    }
    else if (interiorCounts[1] == 0 && interiorCounts[0] > 0)
    {
        return candidates[0];
    }
    else if (interiorCounts[0] > 0 && interiorCounts[1] > 0)
    {
        if (bearings0[0] >= 0.0 && bearings0[0] < 180.0) { goClock[0] = 1; }
        if (bearings1[0] >= 0.0 && bearings1[0] < 180.0) { goClock[1] = 1; }
        if (goClock[0] + goClock[1] == 1)
        {
            double percentClock = clockwisePercentage(sbgui, tracks, sZone);
            if (percentClock > 0.5)
            {
                if (goClock[0] == 1) { return candidates[0]; }
                else if (goClock[1] == 1) { return candidates[1]; }
            }
            else
            {
                if (goClock[0] == 1) { return candidates[1]; }
                else if (goClock[1] == 1) { return candidates[0]; }
            }
        }
    }

    // Plan B...
    if (distances[0] > distances[1])
    {
        dTemp = (distances[0] - distances[1]) / distances[0];
        if (dTemp > candidateDistanceTolerance) { return candidates[0]; }
    }
    else
    {
        dTemp = (distances[1] - distances[0]) / distances[1];
        if (dTemp > candidateDistanceTolerance) { return candidates[1]; }
    }

    // Plan C...
    vector<int> rgbCount(candidates.size());
    vvTemp.resize(2);
    vvTemp[0] = origin;
    for (int ii = 0; ii < candidates.size(); ii++)
    {
        vvTemp[1] = candidates[ii];
        cPath = coordPath(vvTemp);
        rgbCount[ii] = coordRGB(cPath, sZone);
    }
    bool rgbDiff = 0;
    for (int ii = 1; ii < rgbCount.size(); ii++)
    {
        if (rgbCount[ii] != rgbCount[0])
        {
            rgbDiff = 1;
            break;
        }
    }
    int winner = 0;
    if (rgbDiff)
    {
        int bestCount = rgbCount[0];
        for (int ii = 1; ii < rgbCount.size(); ii++)
        {
            if (rgbCount[ii] > bestCount)
            {
                bestCount = rgbCount[ii];
                winner = ii;
            }
        }
    }
    else
    {
        double maxDist = distances[0];
        for (int ii = 0; ii < distances.size(); ii++)
        {
            if (distances[ii] > maxDist)
            {
                maxDist = distances[ii];
                winner = ii;
            }
        }
    }

    return candidates[winner];
}
vector<int> IMGFUNC::borderFindStart()
{
    if (!isInit()) { jf.err("No init-im.borderFindStart"); }
    vector<string> szones = { "zoneBorder", "white" };
    int pixelJump = 2;
    vector<vector<int>> vCorners = { {0,0}, {width-1,0}, {0,height-1}, {width-1,height-1} }; 
    vector<vector<int>> vVictors = { {pixelJump, pixelJump}, {-1 * pixelJump, pixelJump}, {pixelJump, -1 * pixelJump}, {-1 * pixelJump, -1 * pixelJump} };
    vector<vector<int>> vvI(2, vector<int>(2)), vCross1, vCross2;
    for (int ii = 0; ii < 4; ii++)
    {
        vvI[0] = vCorners[ii];
        vvI[1] = vVictors[ii];
        vCross1 = zoneChangeLinear(szones, vvI);
        if (vCross1.size() == 0) { continue; }

        szones = { "zoneBorder", "known" };
        vvI[0] = vCross1[0];
        vvI[1][0] *= -1;
        vvI[1][1] *= -1;
        vCross2 = zoneChangeLinear(szones, vvI);
        if (vCross2.size() == 0) { continue; }
        else { break; }
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

    if (debug) { drawMarker(debugDataPNG, vStart); }
    return vStart;
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
    vector<double> bearings;
    int radiusDefault = 2 * borderThickness;
    int radius, attemptRadius;
    for (int ii = 0; ii < tracks.size() - 1; ii++)
    {
        pastPresent[0] = tracks[ii];
        pastPresent[1] = tracks[ii + 1];
        radius = radiusDefault;
        attemptRadius = 0;
        bearings = octogonBearing(sbgui, pastPresent, sZone, radius);
        while (bearings.size() != 1)
        {
            if (bearings.size() > 1) { radius--; }
            else if (bearings.size() < 1) { radius += 2; }
            bearings = octogonBearing(sbgui, pastPresent, sZone, radius);
            attemptRadius++;
            if (attemptRadius > 10) { jf.err("Cannot determine bearings-im.clockwisePercentage"); }
        }
        if (bearings[0] >= 0.0 && bearings[0] < 180.0) { numClockwise++; }
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
        rgb = pixelRGB(cPath[ii][0], cPath[ii][1]);
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
    sval = "river";
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
vector<vector<unsigned char>> IMGFUNC::lineRGB(vector<vector<int>>& vVictor, int length)
{
    vector<vector<unsigned char>> Lrgb(length, vector<unsigned char>(3));
    int coordX = vVictor[0][0];
    int coordY = vVictor[0][1];
    for (int ii = 0; ii < length; ii++)
    {
        coordX += vVictor[1][0];
        coordY += vVictor[1][1];
        Lrgb[ii] = pixelRGB(coordX, coordY);
    }
    vVictor[0][0] = coordX;
    vVictor[0][1] = coordY;
    return Lrgb;
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
    int offset = getOffset(coord, widthImg);
    img[offset + 0] = rgb[0];
    img[offset + 1] = rgb[1];
    img[offset + 2] = rgb[2];
}
vector<unsigned char> IMGFUNC::pixelRGB(int x, int y)
{
	if (dataPNG.size() < 1) { jf.err("No loaded image-im.pixelRGB"); } 
	vector<unsigned char> rgb(3);
	if (x >= width || y >= height)
	{
		rgb.resize(0);  // Indicates range error.
		return rgb;
	}
    vector<int> vCoord = { x, y };
    int offset = getOffset(vCoord);
	rgb[0] = dataPNG[offset + 0];
	rgb[1] = dataPNG[offset + 1];
	rgb[2] = dataPNG[offset + 2];
    if (rgb[0] == 255 && rgb[1] == 0)
    {
        int bbq = 1;
    }
	return rgb;
}
string IMGFUNC::pixelZone(vector<unsigned char>& rgb)
{
    string szone;
    string shex = pixelDecToHex(rgb);
    try { szone = mapColour.at(shex); }
    catch (out_of_range& oor) { return "unknown"; }
    return szone;
}
void IMGFUNC::pngLoad(string& pathPNG)
{
	unsigned char* dataTemp = stbi_load(pathPNG.c_str(), &width, &height, &numComponents, 0);
	int sizeTemp = width * height * numComponents;
	dataPNG.resize(sizeTemp);
	copy(dataTemp, dataTemp + sizeTemp, dataPNG.begin());
    pathActivePNG = pathPNG;
    debugDataPNG = dataPNG;

}
void IMGFUNC::pngPrint()
{
    int imgSize = debugDataPNG.size();
    int channels = 3;
    auto bufferUC = new unsigned char[imgSize + 1];
    for (int ii = 0; ii < imgSize; ii++)
    {
        bufferUC[ii] = debugDataPNG[ii];
    }
    int error = stbi_write_png("F:\\debug\\testing.png", width, height, channels, bufferUC, 0);

    delete[] bufferUC;
    int barbecue = 1;
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
        sizeVBP++;
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
void IMGFUNC::pngToBinLiveDebug(SWITCHBOARD& sbgui, vector<vector<double>>& border)
{
    vector<int> mycomm;
    vector<vector<int>> comm_gui;
    thread::id myid = this_thread::get_id();
    sbgui.answer_call(myid, mycomm);
    vector<string> prompt = sbgui.get_prompt();
    bool success, frameDone;
    if (!mapIsInit()) { initMapColours(); }
    pngLoad(prompt[0]);

    //vector<int> sourceDim = { width, height };
    vector<int> center = { 2545, 2630 };  
    vector<int> topLeft = center;            // Changes to top-left after pngER.
    //vector<int> extractDim = { 200, 200 };
    vector<int> viTemp = center;
    for (int ii = 0; ii < 4; ii++)
    {
        viTemp[0] += 30;
        viTemp[1] -= 20;
        dotPaint(viTemp);
    }
    vector<vector<int>> vviTemp(2, vector<int>());
    vviTemp[0] = { 2630, 2545 };
    vviTemp[1] = viTemp;
    vector<unsigned char> pink = { 255, 0, 255 };
    int widthLine = 5;
    linePaint(vviTemp, dataPNG, width, pink, widthLine);
    vector<unsigned char> mystery = { 255, 155, 55 };
    vviTemp[0][0] = center[0] + 177;
    vviTemp[0][1] = center[1] + 177;
    vviTemp[1][0] = center[0] - 100;
    vviTemp[1][1] = center[1] - 50;
    octogonPaint(vviTemp[0], 40, dataPNG, width, mystery, widthLine);
    octogonPaint(vviTemp[1], 30, dataPNG, width, mystery, widthLine);

    vector<unsigned char> cropped = pngExtractRect(topLeft);        
    int imgSize = cropped.size();
    int channels = 3;
    auto bufferUC = new unsigned char[imgSize + 1];
    for (int ii = 0; ii < imgSize; ii++)
    {
        bufferUC[ii] = cropped[ii];
    }
    string pathImg = sroot + "\\debug\\tempDebug.png";
    int error = stbi_write_png(pathImg.c_str(), defaultExtractDim[0], defaultExtractDim[1], channels, bufferUC, 0);
    delete[] bufferUC;

    mycomm[0] = 3;   
    sbgui.update(myid, mycomm);
    int bbq = 1;
}
bool IMGFUNC::mapIsInit()
{
    size_t size = mapColour.size();
    if (size > 0) { return 1; }
    return 0;
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
vector<double> IMGFUNC::octogonBearing(SWITCHBOARD& sbgui, vector<vector<int>>& tracks, string sZone, int radius)
{
    // Scans a ring of pixels around the present point for a given zone.
    // If the zone is not found, the returned value is negative. If the 
    // zone is found, its center point is approximated, and then that  
    // center point's angular deviation from the previous direction of 
    // motion is calculated. The return value is this angle (in degrees)
    // given within the interval [0, 360), for every such zone found. 
    vector<double> theta;
    vector<int> originRadius = { tracks[tracks.size() - 1][0], tracks[tracks.size() - 1][1], radius };
    vector<vector<int>> octoPath = octogonPath(tracks[tracks.size() - 1], radius);
    vector<vector<unsigned char>> octoRGB = octogonRGB(octoPath);
    vector<vector<int>> lightHouse = zoneSweep(sZone, octoRGB, octoPath, originRadius);
    vector<vector<int>> vvDebug;
    if (lightHouse.size() != 1)
    {
        if (lightHouse.size() < 1)
        {
            return theta;
        }
        else if (lightHouse.size() > 1 && radius > 0)
        {
            theta = octogonBearing(sbgui, tracks, sZone, radius - 2);
            return theta;
        }
        else 
        { 
            theta = { -1.0 };
            return theta;
        }
        vvDebug.resize(lightHouse.size() + 2, vector<int>(2));
        vvDebug[0] = tracks[tracks.size() - 2];
        vvDebug[1] = tracks[tracks.size() - 1];
        for (int ii = 0; ii < lightHouse.size(); ii++)
        {
            vvDebug[ii + 2] = lightHouse[ii];
        }
        zoneSweepDebug(vvDebug, radius);
        vector<int> mycomm = { 3, 0, 0, 0 };
        thread::id myid = this_thread::get_id();
        sbgui.update(myid, mycomm);
        while (1)
        {
            this_thread::sleep_for(chrono::milliseconds(100));
            int bbq = 1;
        }
    }

    vector<vector<int>> vvTemp(1, vector<int>(2));
    vvTemp[0] = tracks[tracks.size() - 1];
    for (int ii = 0; ii < lightHouse.size(); ii++)
    {
        vvTemp.push_back(lightHouse[ii]);
    }
    vector<double> triangleSides(1);  // r, a, b, ...
    triangleSides[0] = (double)radius;
    vector<double> vdTemp;
    coordDist(tracks[tracks.size() - 2], vvTemp, vdTemp);
    triangleSides.insert(triangleSides.end(), vdTemp.begin(), vdTemp.end());

    int numTri = triangleSides.size() - 2;
    int clockwise;
    double phi, cosPhi;
    vector<vector<double>> pastPresentFuture(3, vector<double>(2));
    jf.toDouble(tracks[tracks.size() - 2], pastPresentFuture[0]);
    jf.toDouble(tracks[tracks.size() - 1], pastPresentFuture[1]);
    for (int ii = 2; ii < triangleSides.size(); ii++)
    {
        cosPhi = (pow(triangleSides[0], 2.0) + pow(triangleSides[1], 2.0) - pow(triangleSides[ii], 2.0)) / (2 * triangleSides[0] * triangleSides[1]);
        if (abs(cosPhi) > 1.0 && abs(cosPhi) <= 1.1)
        {
            if (cosPhi > 0.0) { cosPhi = 0.9999; }
            else { cosPhi = -0.9999; }
        }
        phi = 180.0 / 3.1415926535 * acos(cosPhi);
        pastPresentFuture[2].clear();
        jf.toDouble(lightHouse[ii - 2], pastPresentFuture[2]);
        clockwise = jf.coordCircleClockwise(pastPresentFuture);
        if (clockwise == 1)
        {
            theta.push_back(180.0 - phi);
        }
        else if (clockwise == 0)
        {
            theta.push_back(180.0 + phi);
        }
        else { jf.err("Indeterminate clockwise-im.octogonBearing"); }
    }
    if (theta.size() > 1 || theta[0] > 180.0)
    {
        makeMapDebug(pastPresentFuture);
        clockwise = jf.coordCircleClockwise(pastPresentFuture);
        int bbq = 1;
    }
    return theta;
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
    vector<unsigned char> rgb = pixelRGB(coordA[0], coordA[1]);
    zoneNew = pixelZone(rgb);
    coordB[0] = coordA[0] + dx;
    coordB[1] = coordA[1] + dy;
    rgb = pixelRGB(coordB[0], coordB[1]);
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
            }  // RESUME HERE. Handle multiple border paths.
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
            coordA[1] = coordB[1] + dy;
            coord = 0;
            rgb = pixelRGB(coordA[0], coordA[1]);
        }
        else
        {
            coordB[0] = coordA[0] + dx;
            coordB[1] = coordA[1] + dy;
            coord = 1;
            rgb = pixelRGB(coordB[0], coordB[1]);
        }
    }

    // Function failed to locate the specified border.

    vBorder.resize(0);
    return vBorder;
}
void IMGFUNC::zoneSweepDebug(vector<vector<int>>& vCoord, int radius)
{
    vector<int> topLeft = vCoord[1];    // Changes to top-left after pngER.
    int widthLine = 3;
    int widthDot = 5;
    octogonPaint(vCoord[1], radius, dataPNG, width, Orange, widthLine);
    vector<vector<int>> vviTemp(2, vector<int>(2));
    vviTemp[0] = vCoord[0];
    vviTemp[1] = vCoord[1];
    linePaint(vviTemp, dataPNG, width, Purple, widthLine);
    vviTemp[0] = vCoord[1];
    for (int ii = 2; ii < vCoord.size(); ii++)
    {
        vviTemp[1] = vCoord[ii];
        linePaint(vviTemp, dataPNG, width, Pink, widthLine);
    }
    dotPaint(vCoord[0], dataPNG, width, Teal, widthDot);
    dotPaint(vCoord[1], dataPNG, width, Blue, widthDot);
    for (int ii = 2; ii < vCoord.size(); ii++)
    {
        dotPaint(vCoord[ii], dataPNG, width, Red, widthDot);
    }

    vector<unsigned char> cropped = pngExtractRect(topLeft);
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
    int bbq = 1;
}
