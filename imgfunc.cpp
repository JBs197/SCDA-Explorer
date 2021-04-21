#include "imgfunc.h"

vector<int> IMGFUNC::borderFindNext(vector<vector<int>> tracks)
{
    vector<int> origin = tracks[tracks.size() - 1];
    int radius = 10 * borderThickness;
    vector<vector<int>> octoPath = octogonPath(origin, radius);
    vector<vector<unsigned char>> octoRGB = octogonRGB(octoPath);
    if (debug) { octogonPaint(debugDataPNG, origin, radius, Red); }
    string sZone = "zoneBorder";
    vector<vector<int>> candidates = zoneSweep(sZone, octoRGB, octoPath);
    vector<vector<int>> vvTemp, cPath;
    while (candidates.size() < 2)
    {
        radius += (4 * borderThickness);
        octoPath = octogonPath(origin, radius);
        octoRGB = octogonRGB(octoPath);
        if (debug) { octogonPaint(debugDataPNG, origin, radius, Red); }
        candidates = zoneSweep(sZone, octoRGB, octoPath);
    }
    if (tracks.size() < 2) { return candidates[0]; }
    vector<int> originPast = tracks[tracks.size() - 2];
    vector<double> distances = coordDist(originPast, candidates);
    double minDistance = distances[0];
    int elimIndex = 0;
    for (int ii = 1; ii < distances.size(); ii++)
    {
        if (distances[ii] < minDistance)
        {
            minDistance = distances[ii];
            elimIndex = ii;
        }
    }
    candidates.erase(candidates.begin() + elimIndex);
    if (candidates.size() == 1) { return candidates[0]; }

    sZone = "white";
    vector<int> rgbCount(candidates.size());
    vvTemp.resize(2);
    vvTemp[0] = origin;
    for (int ii = 0; ii < candidates.size(); ii++)
    {
        vvTemp[1] = candidates[ii];
        cPath = coordPath(vvTemp);
        rgbCount[ii] = coordRGB(cPath, sZone);
    }
    int winner = 0;
    int bestCount = rgbCount[0];
    for (int ii = 1; ii < rgbCount.size(); ii++)
    {
        if (rgbCount[ii] > bestCount)
        {
            bestCount = rgbCount[ii];
            winner = ii;
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
vector<double> IMGFUNC::coordDist(vector<int> zZ, vector<vector<int>> cList)
{
    vector<double> listDist(cList.size());
    int inumX, inumY;
    for (int ii = 0; ii < listDist.size(); ii++)
    {
        inumX = (cList[ii][0] - zZ[0]) * (cList[ii][0] - zZ[0]);
        inumY = (cList[ii][1] - zZ[1]) * (cList[ii][1] - zZ[1]);
        listDist[ii] = sqrt(inumX + inumY);
    }
    return listDist;
}
vector<int> IMGFUNC::coordMid(vector<vector<int>>& vCoords)
{
    vector<int> vMid(2);
    vMid[0] = (vCoords[0][0] + vCoords[1][0]) / 2;
    vMid[1] = (vCoords[0][1] + vCoords[1][1]) / 2;
    return vMid;
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
            pixelPaint(img, Red, vTemp);
        }
    }
}
vector<vector<int>> IMGFUNC::frameCorners()
{
    vector<string> sZones = { "white", "black" };
    vector<string> sZonesReverse = { "black", "white" };
    vector<vector<int>> corners(4, vector<int>(2));
    vector<vector<int>> vVictor(2, vector<int>(2));
    vVictor[0] = { 0, height / 2 };
    vVictor[1] = { 1, 0 };
    vector<vector<int>> vvTemp = zoneChangeLinear(sZones, vVictor);
    vVictor[0] = { vvTemp[1][0], height / 2 };
    vVictor[1] = { 0, -1 };
    vvTemp = zoneChangeLinear(sZonesReverse, vVictor);
    corners[0] = vvTemp[0];
    vVictor[0] = vvTemp[0];
    vVictor[1] = { 1, 0 };
    vvTemp = zoneChangeLinear(sZonesReverse, vVictor);
    corners[1] = vvTemp[0];
    vVictor[0] = vvTemp[0];
    vVictor[1] = { 0, 1 };
    vvTemp = zoneChangeLinear(sZonesReverse, vVictor);
    corners[2] = vvTemp[0];
    vVictor[0] = vvTemp[0];
    vVictor[1] = { -1, 0 };
    vvTemp = zoneChangeLinear(sZonesReverse, vVictor);
    corners[3] = vvTemp[0];
    return corners;
}
int IMGFUNC::getOffset(vector<int>& vCoord)
{
    int offRow = vCoord[1] * width * numComponents;
    int offCol = vCoord[0] * numComponents;
    return offRow + offCol;
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
    vector<double> dist = coordDist(pointOfOrigin, vvTemp);
    if (dist[0] < winCondition) { return 1; }
    return 0;
}
void IMGFUNC::linePaint(vector<unsigned char>& img, vector<vector<int>>& vVictor, int length, vector<unsigned char>& colour)
{
    vector<int> vCoord = vVictor[0];
    for (int ii = 0; ii < length; ii++)
    {
        vCoord[0] += vVictor[1][0];
        vCoord[1] += vVictor[1][1];
        pixelPaint(img, colour, vCoord);
    }
    vVictor[0] = vCoord;
}
vector<vector<int>> IMGFUNC::linePath(vector<vector<int>>& vVictor, int length)
{
    // Returns the xy coordinates for a given straight line and length.
    // Does NOT return coords for the starting point in vVictor.
    // Will update the coordinate row of vVictor with the final point measured.
    vector<vector<int>> lineP(length, vector<int>(2));
    vector<int> vTemp = vVictor[0];
    for (int ii = 0; ii < length; ii++)
    {
        vTemp[0] += vVictor[1][0];
        vTemp[1] += vVictor[1][1];
        lineP[ii] = vTemp;
    }
    vVictor[0] = vTemp;
    return lineP;
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
void IMGFUNC::pixelPaint(vector<unsigned char>& img, vector<unsigned char>& colour, vector<int>& vCoord)
{
    int offset = getOffset(vCoord);
    img[offset + 0] = colour[0];
    img[offset + 1] = colour[1];
    img[offset + 2] = colour[2];
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
void IMGFUNC::pngToBin(string& pathPNG, string& pathBIN)
{
    if (!mapIsInit()) { initMapColours(); }
    pngLoad(pathPNG);
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
    vector<vector<int>> corners = frameCorners();

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

}
bool IMGFUNC::mapIsInit()
{
    size_t size = mapColour.size();
    if (size > 0) { return 1; }
    return 0;
}
void IMGFUNC::octogonPaint(vector<unsigned char>& img, vector<int>& origin, int radius, vector<unsigned char>& colour)
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
    vector<int> vCoord = origin;
    vCoord[1] -= radius;
    pixelPaint(img, colour, vCoord);
    vector<vector<int>> vVictor = { vCoord, {1, 0} };
    linePaint(img, vVictor, radius / 2, colour);

    vVictor[1] = { 1, 1 };
    linePaint(img, vVictor, lenDiag, colour);
    vVictor[1] = { 0, 1 };
    linePaint(img, vVictor, lenPerp, colour);
    vVictor[1] = { -1, 1 };
    linePaint(img, vVictor, lenDiag, colour);
    vVictor[1] = { -1, 0 };
    linePaint(img, vVictor, lenPerp, colour);
    vVictor[1] = { -1, -1 };
    linePaint(img, vVictor, lenDiag, colour);
    vVictor[1] = { 0, -1 };
    linePaint(img, vVictor, lenPerp, colour);
    vVictor[1] = { 1, -1 };
    linePaint(img, vVictor, lenDiag, colour);
    vVictor[1] = { 1, 0 };
    linePaint(img, vVictor, radius / 2, colour);
}
vector<vector<int>> IMGFUNC::octogonPath(vector<int>& origin, int radius)
{
    // Returns a list of xy coordinates for the perimeter of a given octogon.
    // List starts at 12 o'clock and progresses clockwise.
    // Radius is measured along a vertical or horizontal line. 
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
    vector<vector<int>> octoPath(1, vector<int>(2));
    vector<vector<int>> vvTemp;
    octoPath[0][0] = origin[0];
    octoPath[0][1] = origin[1] - radius;
    vector<vector<int>> vVictor(2, vector<int>(2));
    vVictor[0] = octoPath[0];
    vVictor[1] = { 1, 0 };
    vvTemp = linePath(vVictor, radius / 2);
    octoPath.insert(octoPath.end(), vvTemp.begin(), vvTemp.end());

    vVictor[1] = { 1, 1 };
    vvTemp = linePath(vVictor, lenDiag);
    octoPath.insert(octoPath.end(), vvTemp.begin(), vvTemp.end());
    vVictor[1] = { 0, 1 };
    vvTemp = linePath(vVictor, lenPerp);
    octoPath.insert(octoPath.end(), vvTemp.begin(), vvTemp.end());
    vVictor[1] = { -1, 1 };
    vvTemp = linePath(vVictor, lenDiag);
    octoPath.insert(octoPath.end(), vvTemp.begin(), vvTemp.end());
    vVictor[1] = { -1, 0 };
    vvTemp = linePath(vVictor, lenPerp);
    octoPath.insert(octoPath.end(), vvTemp.begin(), vvTemp.end());
    vVictor[1] = { -1, -1 };
    vvTemp = linePath(vVictor, lenDiag);
    octoPath.insert(octoPath.end(), vvTemp.begin(), vvTemp.end());
    vVictor[1] = { 0, -1 };
    vvTemp = linePath(vVictor, lenPerp);
    octoPath.insert(octoPath.end(), vvTemp.begin(), vvTemp.end());
    vVictor[1] = { 1, -1 };
    vvTemp = linePath(vVictor, lenDiag);
    octoPath.insert(octoPath.end(), vvTemp.begin(), vvTemp.end());
    vVictor[1] = { 1, 0 };
    vvTemp = linePath(vVictor, radius / 2);
    octoPath.insert(octoPath.end(), vvTemp.begin(), vvTemp.end());

    return octoPath;
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
vector<vector<int>> IMGFUNC::zoneSweep(string sZone, vector<vector<unsigned char>>& Lrgb, vector<vector<int>>& zonePath)
{
    // For a given list of RGB values, return the middle pixel coordinates of every
    // (desired) szone interval. Commonly used to scan the perimeter of a shape.
    vector<vector<int>> goldilocks;
    vector<int> zoneFreezer;
    vector<vector<int>> startStop(2, vector<int>(2));
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
        zoneFreezer = zonePath[index - 1];  // Keep for the end.
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
                startStop[0] = zonePath[index];
            }
        }
        else
        {
            if (zoneActive)
            {
                zoneActive = 0;
                startStop[1] = zonePath[index];
                goldilocks.push_back(coordMid(startStop));
            }
        }
        index++;
    }
    if (zoneFreezer.size() > 0)
    {
        if (!zoneActive)
        {
            startStop[0] = zonePath[0];
        }
        startStop[1] = zoneFreezer;
        goldilocks.push_back(coordMid(startStop));
    }
    return goldilocks;
}
