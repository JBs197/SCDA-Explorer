#include "imgfunc.h"

vector<int> IMGFUNC::borderFindNext(vector<vector<int>> tracks)
{
    vector<vector<int>> vvStar = zoneStar(tracks);
    vector<vector<double>> listDist(tracks.size() + 1, vector<double>(8));
    for (int ii = 0; ii < tracks.size(); ii++)
    {
        listDist[ii + 1] = coordDist(tracks[ii], vvStar);
    }
    for (int ii = 0; ii < 8; ii++)
    {
        listDist[0][ii] = 0.0;
        for (int jj = 0; jj < tracks.size(); jj++)
        {
            listDist[0][ii] += (((double)jj + 1.0) / 2.0) * listDist[jj + 1][ii];
        }
    }
    double dLongest = 0.0;
    int iLongest = -1;
    int iForbidden = -1;
    if (recordVictor >= 0)
    {
        iForbidden = (recordVictor + 4) % 8;  // Path CANNOT reverse course.
    }
    for (int ii = 0; ii < listDist[0].size(); ii++)
    {
        if (ii == iForbidden) { continue; }
        if (listDist[0][ii] > dLongest)
        {
            dLongest = listDist[0][ii];
            iLongest = ii;
        }
    }
    recordVictor = iLongest;
    return vvStar[iLongest];
}
vector<int> IMGFUNC::borderFindStart()
{
    if (!isInit()) { jf.err("No init-im.borderFindStart"); }
    vector<string> szones = { "zoneOutside", "zoneBorder" };
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

        szones = { "zoneBorder", "white" };
        vvI[0] = vCross1[1];
        vCross2 = zoneChangeLinear(szones, vvI);
        if (vCross2.size() == 0) { continue; }
        else { break; }
    }
    vector<int> vStart;
    if (vCross1.size() == 2 && vCross2.size() == 2)
    {
        vvI[0] = vCross1[1];
        vvI[1] = vCross2[0];
        vStart = coordMid(vvI);
    }
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
vector<unsigned char> IMGFUNC::pixelRGB(int x, int y)
{
	if (dataPNG.size() < 1) { jf.err("No loaded image-im.pixelRGB"); } 
	vector<unsigned char> rgb(3);
	if (x >= width || y >= height)
	{
		rgb.resize(0);  // Indicates range error.
		return rgb;
	}
	int offsetRow = y * width * numComponents;
	int offsetCol = x * numComponents;
	rgb[0] = dataPNG[offsetRow + offsetCol + 0];
	rgb[1] = dataPNG[offsetRow + offsetCol + 1];
	rgb[2] = dataPNG[offsetRow + offsetCol + 2];
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

    if (vvStar.size() == 0)
    {
        vvStar.resize(8);
        vvStar[0] = { 0,1 };
        vvStar[1] = { 1,1 };
        vvStar[2] = { 1,0 };
        vvStar[3] = { 1,-1 };
        vvStar[4] = { 0,-1 };
        vvStar[5] = { -1,-1 };
        vvStar[6] = { -1,0 };
        vvStar[7] = { -1,1 };
    }
}
bool IMGFUNC::mapIsInit()
{
    size_t size = mapColour.size();
    if (size > 0) { return 1; }
    return 0;
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
vector<vector<int>> IMGFUNC::zoneStar(vector<vector<int>>& tracks)
{
    // This function takes one point inside a zone, and fires off
    // a uniform distribution of vectors away from that point. 
    // The returned list of coordinates represents the points where 
    // each vector left that zone. 
    vector<vector<int>> listStar(8, vector<int>(2));
    vector<vector<int>> vvTemp;
    vector<string> szones = { "zoneBorder", "known" };
    int pixelJump = 2;
    vector<vector<int>> vvI(2, vector<int>(2));
    vvI[0] = tracks[tracks.size() - 1];
    for (int ii = 0; ii < listStar.size(); ii++)
    {
        vvI[1][0] = vvStar[ii][0] * pixelJump;
        vvI[1][1] = vvStar[ii][1] * pixelJump;
        vvTemp = zoneChangeLinear(szones, vvI);
        if (vvTemp.size() == 2)
        {
            listStar[ii] = vvTemp[0];
        }
        else { jf.err("zoneChangeLinear-im.zoneStar"); }
    }
    return listStar;
}
// RESUME HERE. Try a scanOctogon method to make jumps. 