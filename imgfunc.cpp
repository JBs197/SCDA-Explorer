#include "imgfunc.h"

void CANDIDATES::centerOfMass(vector<double>& vdDist)
{
    // The Hail Mary test. Eliminates all candidates except the one closest to the image center. 
    POINT pCenter;
    pCenter.x = imgSpec[0] / 2;
    pCenter.y = imgSpec[1] / 2;
    vector<double> vdCenter(pList.size());
    for (int ii = 0; ii < vdCenter.size(); ii++)
    {
        vdCenter[ii] = gdi.coordDistPoint(pCenter, pList[ii]);
    }
    vector<int> minMax = jf.minMax(vdCenter);
    POINT pSurvivor = pList[minMax[0]];
    double dDist = vdDist[minMax[0]];
    pList = { pSurvivor };
    vdDist = { dDist };
}
int CANDIDATES::colourCount(vector<unsigned char>& rgbx)
{
    // Returns the number of candidates possessing the given colour. 
    int count = 0;
    for (int ii = 0; ii < rgbaList.size(); ii++)
    {
        for (int jj = 0; jj < rgbx.size(); jj++)
        {
            if (rgbx[jj] != rgbaList[ii][jj]) { break; }
            else if (jj == rgbx.size() - 1) { count++; }
        }
    }
    return count;
}
int CANDIDATES::fromCircle(POINT pCenter, int radius)
{
    if (scanCircles.size() < 1) { jf.err("No init for scanCircles-cd.fromCircle"); }
    if (radius < 1 || radius > scanCircles.size()) { jf.err("Bad radius-cd.fromCircle"); }
    pList = scanCircles[radius - 1];
    rgbaList.resize(pList.size(), vector<unsigned char>(imgSpec[2]));
    int offset;
    for (int ii = 0; ii < pList.size(); ii++)
    {
        pList[ii].x += pCenter.x - pScanCircleCenter.x;
        if (pList[ii].x < 0) { pList[ii].x = 0; }
        if (pList[ii].x >= imgSpec[0]) { pList[ii].x = imgSpec[0] - 1; }
        pList[ii].y += pCenter.y - pScanCircleCenter.y;
        if (pList[ii].y < 0) { pList[ii].y = 0; }
        if (pList[ii].y >= imgSpec[1]) { pList[ii].y = imgSpec[1] - 1; }
        offset = getOffset(pList[ii]);
        rgbaList[ii][0] = img[offset + 0];
        rgbaList[ii][1] = img[offset + 1];
        rgbaList[ii][2] = img[offset + 2];
        if (imgSpec[2] > 3) { rgbaList[ii][3] = img[offset + 3]; }      
    }
    int iSize = pList.size();
    return iSize;
}
POINT CANDIDATES::getCandidate(int index)
{
    return pList[index];
}
POINT CANDIDATES::getClockwiseCandidate()
{
    // Returns the rightmost and (if necessary) lowest candidate. 
    // To be used when a POINT history is unavailable.
    vector<int> xMax = { -1, -1 };
    for (int ii = 0; ii < pList.size(); ii++)
    {
        if (pList[ii].x > xMax[0])
        {
            xMax.resize(2);
            xMax[0] = pList[ii].x;
            xMax[1] = ii;
        }
        else if (pList[ii].x == xMax[0])
        {
            xMax.push_back(ii);
        }
    }
    if (xMax.size() == 2) { return pList[xMax[1]]; }
    vector<int> yMax = { -1, -1 };
    for (int ii = 1; ii < xMax.size(); ii++)
    {
        if (pList[xMax[ii]].y > yMax[0])
        {
            yMax[0] = pList[xMax[ii]].y;
            yMax[1] = xMax[ii];
        }
    }
    return pList[yMax[1]];
}
void CANDIDATES::getImg(vector<unsigned char>& imgOut, vector<int>& imgSpecOut)
{
    imgOut = img;
    imgSpecOut = imgSpec;
}
int CANDIDATES::getOffset(POINT& p1)
{
    int offRow = p1.y * imgSpec[0] * imgSpec[2];
    int offCol = p1.x * imgSpec[2];
    return offRow + offCol;
}
void CANDIDATES::initialize(vector<vector<POINT>>& ScanCircles, POINT& pCenter)
{
    scanCircles = ScanCircles;
    pScanCircleCenter = pCenter;
}
int CANDIDATES::judgeDist(vector<double>& vdDist)
{
    if (pList.size() != vdDist.size()) { jf.err("Parameter size mismatch-cd.judgeDist"); }
    double percent, maxDist;
    vector<int> minMax = jf.minMax(vdDist);
    maxDist = vdDist[minMax[1]];
    for (int ii = vdDist.size() - 1; ii >= 0; ii--)
    {
        percent = vdDist[ii] / maxDist;
        if (percent < distRunnerUp)
        {
            rgbaList.erase(rgbaList.begin() + ii);
            pList.erase(pList.begin() + ii);
            vdDist.erase(vdDist.begin() + ii);
        }
    }
    int iSize = pList.size();
    return iSize;
}
int CANDIDATES::keepColour(vector<unsigned char> rgbx)
{
    // Remove all candidates not of the given colour. Return the number of candidates remaining.
    for (int ii = rgbaList.size() - 1; ii >= 0; ii--)
    {
        for (int jj = 0; jj < rgbx.size(); jj++)
        {
            if (rgbaList[ii][jj] != rgbx[jj])
            {
                rgbaList.erase(rgbaList.begin() + ii);
                pList.erase(pList.begin() + ii);
                break;
            }
        }
    }
    int iSize = pList.size();
    return iSize;
}
int CANDIDATES::removeColour(vector<unsigned char> rgbx)
{
    // Remove all candidates of the given colour. Return the number of candidates remaining.
    for (int ii = rgbaList.size() - 1; ii >= 0; ii--)
    {
        if (rgbaList[ii] == rgbx)
        {
            rgbaList.erase(rgbaList.begin() + ii);
            pList.erase(pList.begin() + ii);
            break;
        }
    }
    int iSize = pList.size();
    return iSize;
}
int CANDIDATES::removeColourRatio(int ratio, vector<double> vdInterval)
{
    // vdInterval is inclusive on both ends. Ratio is given by the class enum.
    if (vdInterval.size() < 2) { jf.err("No interval given-cd.removeColourRatio"); }
    double dRatio;
    for (int ii = rgbaList.size() - 1; ii >= 0; ii--)
    {
        switch (ratio)
        {
        case 0:  // RG
            dRatio = (double)rgbaList[ii][0] / (double)rgbaList[ii][1];
            break;
        case 1:  // RB
            dRatio = (double)rgbaList[ii][0] / (double)rgbaList[ii][2];
            break;
        case 2:  // GB
            dRatio = (double)rgbaList[ii][1] / (double)rgbaList[ii][2];
            break;
        }
        if (dRatio >= vdInterval[0] && dRatio <= vdInterval[1])
        {
            rgbaList.erase(rgbaList.begin() + ii);
            pList.erase(pList.begin() + ii);
        }
    }
    int iSize = pList.size();
    return iSize;
}
int CANDIDATES::removeDeadEnd(vector<POINT>& vpDeadEnd)
{
    for (int ii = 0; ii < vpDeadEnd.size(); ii++)
    {
        for (int jj = pList.size() - 1; jj >= 0; jj--)
        {
            if (pList[jj].x == vpDeadEnd[ii].x && pList[jj].y == vpDeadEnd[ii].y)
            {
                rgbaList.erase(rgbaList.begin() + jj);
                pList.erase(pList.begin() + jj);
                break;
            }
        }
    }
    int iSize = pList.size();
    return iSize;
}
int CANDIDATES::removeRearCone(vector<POINT>& vpPast)
{
    // Define a forward vector using the last three border points. Remove all candidates
    // which are within (plus or minus) deadCone/2 of the forward vector's opposite. 
    if (vpPast.size() < 3) { jf.err("Insufficient vpPast length-cd.removeRearCone"); }
    int pastIndex = vpPast.size() - 1;
    double deviation;
    vector<vector<double>> pastPresentCandidate(3);
    double xPast = ((double)vpPast[pastIndex - 1].x + (double)vpPast[pastIndex - 2].x) / 2.0;
    double yPast = ((double)vpPast[pastIndex - 1].y + (double)vpPast[pastIndex - 2].y) / 2.0;
    pastPresentCandidate[0] = { xPast, yPast };
    pastPresentCandidate[1] = { (double)vpPast[pastIndex].x, (double)vpPast[pastIndex].y };
    for (int ii = pList.size() - 1; ii >= 0; ii--)
    {
        pastPresentCandidate[2] = { (double)pList[ii].x, (double)pList[ii].y };
        if (pastPresentCandidate[2] == pastPresentCandidate[0])
        {
            pList.erase(pList.begin() + ii);
            rgbaList.erase(rgbaList.begin() + ii);
            continue;
        }
        deviation = mf.angleBetweenVectors(pastPresentCandidate);
        if (abs(180.0 - deviation) <= deadCone / 2.0)
        {
            pList.erase(pList.begin() + ii);
            rgbaList.erase(rgbaList.begin() + ii);
        }
    }
    int iSize = pList.size();
    return iSize;
}
int CANDIDATES::removePast(vector<POINT>& vpPast, int depth)
{
    // Candidates matching the given list (going back "depth") are removed. Number of survivors is returned. 
    for (int ii = vpPast.size() - 1; ii >= vpPast.size() - depth; ii--)
    {
        for (int jj = pList.size() - 1; jj >= 0; jj--)
        {
            if (vpPast[ii].x == pList[jj].x && vpPast[ii].y == pList[jj].y)
            {
                rgbaList.erase(rgbaList.begin() + jj);
                pList.erase(pList.begin() + jj);
                break;
            }
        }
        if (ii == vpPast.size() - depth) { break; }
    }
    int iSize = pList.size();
    return iSize;
}
vector<double> CANDIDATES::reportDist(vector<POINT>& vpPast)
{
    // For each candidate, determine the sum total distance between it and the previous two points. 
    if (vpPast.size() < 3) { jf.err("Insufficient vpPast length-cd.reportDist"); }
    int previous = min(4, (int)vpPast.size());
    vector<POINT> vpPrevious(previous);
    for (int ii = 0; ii < previous; ii++)
    {
        vpPrevious[ii] = vpPast[vpPast.size() - previous + ii];
    }
    vector<double> vdDist = gdi.coordDistPointSumList(pList, vpPrevious);
    return vdDist;
}
void CANDIDATES::setImg(vector<unsigned char>& image, vector<int>& imageSpec)
{
    img = image;
    imgSpec = imageSpec;
}
bool CANDIDATES::siblings()
{
    // Return TRUE if there are only two candidates, and they are adjacent.
    if (pList.size() != 2) { return 0; }
    double dist = gdi.coordDistPoint(pList[0], pList[1]);
    if (dist > 1.43) { return 0; }
    return 1;
}

int IMGFUNC::checkColour(vector<vector<unsigned char>>& rgbaList, vector<unsigned char>& rgba)
{
    // Return the colourIndex if the given colour matches one of the rows in rgbaList. Else -1.
    for (int ii = 0; ii < rgbaList.size(); ii++)
    {
        for (int jj = 0; jj < rgba.size(); jj++)
        {
            if (rgba[jj] != rgbaList[ii][jj]) { break; }
            else if (jj == rgba.size() - 1) { return ii; }
        }
    }
    return -1;
}
string IMGFUNC::convertColour(vector<unsigned char>& rgbx)
{
    if (mapDecHex.size() < 1) { initHex(); }
    string srgbx;
    unsigned char quotient, remainder;
    char quot, rem;
    for (int ii = 0; ii < rgbx.size(); ii++)
    {
        quotient = rgbx[ii] / 16;
        remainder = rgbx[ii] % 16;
        try
        {
            quot = mapDecHex.at(quotient);
            rem = mapDecHex.at(remainder);
        }
        catch (out_of_range) { jf.err("mapDecHex-im.convertColour"); }
        srgbx.append(1, quot);
        srgbx.append(1, rem);
    }
    return srgbx;
}
vector<unsigned char> IMGFUNC::convertColour(string& srgbx)
{
    vector<unsigned char> rgbx;
    if (srgbx.size() == 6) { rgbx.resize(3); }
    else if (srgbx.size() == 8) { rgbx.resize(4); }
    else { jf.err("Parameter size-im.convertColour"); }
    unsigned char quotient, remainder;
    for (int ii = 0; ii < rgbx.size(); ii++)
    {
        try
        {
            quotient = mapHexDec.at(srgbx[(2 * ii)]);
            remainder = mapHexDec.at(srgbx[(2 * ii) + 1]);
        }
        catch (out_of_range) { jf.err("mapHexDec-im.convertColour"); }
        rgbx[ii] = (16 * quotient) + remainder;
    }
    return rgbx;
}
POINT IMGFUNC::coordFromOffset(int offset, vector<int> imgSpec)
{
    // imgSpec has form [width, height, numComponents].
    POINT coord;
    coord.y = offset / (imgSpec[0] * imgSpec[2]);
    coord.x = (offset % (imgSpec[0] * imgSpec[2])) / imgSpec[2];
    return coord;
}
void IMGFUNC::countPixelColour(string& filePath, vector<vector<unsigned char>>& colourList, vector<int>& freqList)
{
    vector<unsigned char> img, rgbx;
    vector<int> imgSpec;
    unordered_map<string, int> mapColour;
    int index;
    string srgbx;
    pngLoadHere(filePath, img, imgSpec);
    POINT p1;
    colourList.clear();
    freqList.clear();
    for (int ii = 0; ii < imgSpec[1]; ii++)
    {
        p1.y = ii;
        for (int jj = 0; jj < imgSpec[0]; jj++)
        {
            p1.x = jj;
            rgbx = pixelRGB(img, imgSpec, p1);
            srgbx = jf.stringifyCoord(rgbx);
            if (mapColour.count(srgbx))  // Colour already encountered.
            {
                index = mapColour.at(srgbx);
                freqList[index]++;
            }
            else
            {
                index = freqList.size();
                freqList.push_back(1);
                colourList.push_back(rgbx);
                mapColour.emplace(srgbx, index);
            }
        }
    }
}
void IMGFUNC::countPixelColour(vector<unsigned char>& img, vector<int>& imgSpec, vector<vector<unsigned char>>& colourList, vector<int>& freqList, vector<POINT> TLBR)
{
    vector<unsigned char> rgbx;
    unordered_map<string, int> mapColour;
    int index;
    string srgbx;
    POINT p1;
    colourList.clear();
    freqList.clear();
    for (int ii = TLBR[0].y; ii < TLBR[1].y; ii++)
    {
        p1.y = ii;
        for (int jj = TLBR[0].x; jj < TLBR[1].x; jj++)
        {
            p1.x = jj;
            rgbx = pixelRGB(img, imgSpec, p1);
            srgbx = jf.stringifyCoord(rgbx);
            if (mapColour.count(srgbx))  // Colour already encountered.
            {
                index = mapColour.at(srgbx);
                freqList[index]++;
            }
            else
            {
                index = freqList.size();
                freqList.push_back(1);
                colourList.push_back(rgbx);
                mapColour.emplace(srgbx, index);
            }
        }
    }
}
void IMGFUNC::crop(vector<unsigned char>& img, vector<int>& imgSpec, vector<POINT> TLBR)
{
    // Replaces the original image with the cropped image. 
    if (img.size() < 1 || imgSpec.size() < 1 || TLBR.size() < 2) { jf.err("Bad input parameters-im.crop"); }
    vector<unsigned char> image = img;
    vector<int> imageSpec = imgSpec;
    imgSpec.resize(3);
    imgSpec[0] = TLBR[1].x - TLBR[0].x + 1;
    imgSpec[1] = TLBR[1].y - TLBR[0].y + 1;
    imgSpec[2] = imageSpec[2];
    img.resize(imgSpec[0] * imgSpec[1] * imgSpec[2]);
    int index = 0, offsetRow;
    int offsetTL = getOffset(TLBR[0], imageSpec);
    for (int ii = 0; ii < imgSpec[1]; ii++)
    {
        offsetRow = offsetTL + (ii * imageSpec[0] * imageSpec[2]);
        for (int jj = 0; jj < (imgSpec[0] * imgSpec[2]); jj++)
        {
            img[index] = image[offsetRow + jj];
            index++;
        }
    }
}
void IMGFUNC::crop(vector<vector<unsigned char>>& img, vector<vector<int>>& imgSpec, vector<POINT> TLBR)
{
    // Index 0 is input, index 1 is output.
    if (img.size() < 1 || imgSpec.size() < 1 || TLBR.size() != 2) { jf.err("Bad input parameters-im.crop"); }
    int offsetTL = getOffset(TLBR[0], imgSpec[0]);
    imgSpec.resize(2);
    imgSpec[1].resize(3);
    imgSpec[1][0] = TLBR[1].x - TLBR[0].x;
    imgSpec[1][1] = TLBR[1].y - TLBR[0].y;
    imgSpec[1][2] = imgSpec[0][2];
    img.resize(2);
    img[1].resize(imgSpec[1][0] * imgSpec[1][1] * imgSpec[1][2]);
    int index = 0, offsetRow;
    for (int ii = 0; ii < imgSpec[1][1]; ii++)
    {
        offsetRow = offsetTL + (ii * imgSpec[0][0] * imgSpec[0][2]);
        for (int jj = 0; jj < (imgSpec[1][0] * imgSpec[0][2]); jj++)
        {
            img[1][index] = img[0][offsetRow + jj];
            index++;
        }
    }
}
void IMGFUNC::cropSave(vector<string>& filePath, vector<POINT> TLBR)
{
    // filePath has form [input path, output path].
    if (filePath.size() < 2 || TLBR.size() < 2) { jf.err("Bad input parameters-im.cropSave"); }
    vector<vector<unsigned char>> img(2);
    vector<vector<int>> imgSpec(2);
    pngLoadHere(filePath[0], img[0], imgSpec[0]);
    crop(img, imgSpec, TLBR);
    pngPrint(img[1], imgSpec[1], filePath[1]);
}
void IMGFUNC::cropSaveOld(vector<string>& filePath, vector<POINT> TLBR)
{
    vector<unsigned char> imgInput, imgOutput;
    vector<int> imgSpec(3), TL = { TLBR[0].x, TLBR[0].y };
    vector<int> extractDim = { TLBR[1].x - TLBR[0].x, TLBR[1].y - TLBR[0].y };
    pngLoadHere(filePath[0], imgInput, imgSpec);
    imgOutput = pngExtractRectTopLeft(TL, imgInput, imgSpec, extractDim);
    extractDim.push_back(imgSpec[2]);
    pngPrint(imgOutput, extractDim, filePath[1]);
}
void IMGFUNC::drawSquare(vector<unsigned char>& img, vector<int>& imgSpec, POINT coord)
{
    // Paint a 5x5 red square on the selected map/point.
    drawSquare(img, imgSpec, coord, 5, Red);
}
void IMGFUNC::drawSquare(vector<unsigned char>& img, vector<int>& imgSpec, POINT coord, int width, vector<unsigned char> rgba)
{
    // Paint a (width x width) rgba square on the selected map/point.
    POINT p1;
    for (int ii = 0; ii < width; ii++)
    {
        p1.y = coord.y - (width / 2) + ii;
        for (int jj = 0; jj < width; jj++)
        {
            p1.x = coord.x - (width / 2) + jj;
            pixelPaint(img, imgSpec, rgba, p1);
        }
    }
}
vector<POINT> IMGFUNC::findTLBRColour(vector<unsigned char>& img, vector<int>& imgSpec, vector<unsigned char> RGB, vector<POINT> TLBR)
{
    // Return a list of points that match the given colour, within the specified TLBR.
    vector<unsigned char> rgb;
    vector<POINT> result;
    POINT p1;
    for (int ii = TLBR[0].y; ii <= TLBR[1].y; ii++)
    {
        p1.y = ii;
        for (int jj = TLBR[0].x; jj <= TLBR[1].x; jj++)
        {
            p1.x = jj;
            rgb = pixelRGB(img, imgSpec, p1);
            if (rgb == RGB) { result.push_back(p1); }
        }
    }
    return result;
}
void IMGFUNC::flatten(string& filePath)
{
    // Load a PNG, and if it has alpha, remove those bytes and save it as RGB only.
    vector<vector<unsigned char>> img(2);
    vector<int> imgSpec(3);
    string newPath, temp;
    size_t pos1;
    pngLoadHere(filePath, img[0], imgSpec);
    if (imgSpec[2] == 3) { return; }
    else if (imgSpec[2] == 4)
    {
        img[1] = img[0];
        removePeriodic(img[1], 4);
        pos1 = filePath.rfind(".png");
        temp = filePath.substr(0, pos1);
        newPath = temp + " (Unflattened).png";
        pngPrint(img[0], imgSpec, newPath);
        imgSpec[2]--;
        newPath = temp + " (Flattened).png";
        pngPrint(img[1], imgSpec, newPath);
    }
    else { jf.err("Unknown bits-per-pixel-im.flatten"); }
}
int IMGFUNC::getBandCenter(vector<int>& candidates)
{
    // Return the center element from the largest interval of continuous values.
    vector<int> frontrunner = { -1, -1 };
    int count = 0;
    for (int ii = 1; ii < candidates.size(); ii++)
    {
        if (candidates[ii - 1] + 1 == candidates[ii]) { count++; }
        else { count = 0; }
        if (count > frontrunner[0])
        {
            frontrunner[0] = count;
            frontrunner[1] = candidates[ii];
        }
    }
    frontrunner[1] -= frontrunner[0] / 2;
    return frontrunner[1];
}
POINT IMGFUNC::getBorderPoint(vector<unsigned char>& img, vector<int>& imgSpec, vector<POINT> startStop, vector<vector<double>> vvdInside)
{
    // greenBlue is the decimal ratio of green divided by blue in a pixel. The greenBlue vectors
    // indicate min/max values for the acceptable interval, outside or inside. 
    // vdInside has form [greenBlueInside, redBlueInside, redGreenInside].
    if (startStop[0].x < 0 || startStop[1].x < 0) { jf.err("startStop out of bounds-im.getBorderPoint"); }
    if (startStop[0].y < 0 || startStop[1].y < 0) { jf.err("startStop out of bounds-im.getBorderPoint"); }
    if (startStop[0].x >= imgSpec[0] || startStop[1].x >= imgSpec[0]) { jf.err("startStop out of bounds-im.getBorderPoint"); }
    if (startStop[0].y >= imgSpec[1] || startStop[1].y >= imgSpec[1]) { jf.err("startStop out of bounds-im.getBorderPoint"); }
    CANDIDATES cd;
    initCandidates(cd);
    cd.initialize(scanCircles, pScanCircleCenter);
    cd.setImg(img, imgSpec);
    vector<unsigned char> rgba;
    int idx, idy, colourIndex, size1, size2;
    double ddx, ddy, dDx, dDy, xMid, yMid, greenBlue, redBlue, redGreen;
    bool perpendicular = 0;
    if (startStop[0].x == startStop[1].x)
    {
        idx = 0;
        if (startStop[1].y > startStop[0].y) { idy = 1; }
        else { idy = -1; }
        perpendicular = 1;
    }
    if (startStop[0].y == startStop[1].y)
    {
        idy = 0;
        if (startStop[1].x > startStop[0].x) { idx = 1; }
        else { idx = -1; }
        perpendicular = 1;
    }
    if (!perpendicular)
    {
        double Dx = (double)(startStop[1].x - startStop[0].x);
        double Dy = (double)(startStop[1].y - startStop[0].y);
        if (abs(Dx) > abs(Dy))
        {
            ddy = Dy / abs(Dx);
            if (Dx > 0) { idx = 1; }
            else { idx = -1; }
            idy = 0;
        }
        else
        {
            ddx = Dx / abs(Dy);
            if (Dy > 0) { idy = 1; }
            else { idy = -1; }
            idx = 0;
        }
        dDx = (double)startStop[0].x;
        dDy = (double)startStop[0].y;
    }
    POINT p1 = startStop[0], pInside, pBorder;
    pBorder.x = -1;
    pBorder.y = -1;
    pInside = pBorder;
    while (p1.x != startStop[1].x || p1.y != startStop[1].y)
    {
        // Move to the next pixel.
        if (perpendicular)
        {
            p1.x += idx;
            p1.y += idy;
        }
        else if (!idx)
        {
            p1.y += idy;
            dDx += ddx;
            p1.x = int(round(dDx));
        }
        else
        {
            p1.x += idx;
            dDy += ddy;
            p1.y = int(round(dDy));
        }

        rgba = pixelRGB(img, imgSpec, p1);
        greenBlue = getGB(rgba);
        redBlue = getRB(rgba);
        redGreen = getRG(rgba);

        // Certainly outside.
        if (greenBlue >= vvdInside[0][1] || greenBlue < vvdInside[0][0]) { continue; }
        else if (redBlue >= vvdInside[1][1] || redBlue < vvdInside[1][0]) { continue; }
        else if (redGreen >= vvdInside[2][1] || redGreen < vvdInside[2][0]) { continue; }
        else if (rgba[0] == rgba[1] && rgba[1] == rgba[2]) { continue; } // Outside map text is a shade of grey.

        // Wrong side of the border.
        size1 = cd.fromCircle(p1, 2);
        size2 = cd.removeColour(Canada);
        if (size2 != size1) { continue; }
        size2 = cd.removeColour(Water);
        if (size2 != size1) { continue; }
        size2 = cd.removeColourRatio(CANDIDATES::GB, { 0.81, 1.0 });
        if (size2 != size1) { continue; }

        return p1;
    }
    return pBorder;
}
POINT IMGFUNC::getBorderPoint(vector<unsigned char>& img, vector<int>& imgSpec, POINT pStart, double angle, vector<POINT> TLBR, vector<vector<double>> vvdInside)
{
    // vdInside has form [greenBlueInside, redBlueInside, redGreenInside].
    // This function variant starts at a point, travels along a path given by "angle", and stops when
    // it locates a border point, or hits the TLBR boundary. Angle is measured from the positive 
    // x-axis, and travels CLOCKWISE within the interval [0.0, 360.0)

    if (angle < 0.0 || angle >= 360.0) { jf.err("Invalid angle-im.getBorderPoint"); }
    if (pStart.x < 0 || pStart.x >= imgSpec[0]) { jf.err("Invalid pStart-im.getBorderPoint"); }
    if (pStart.y < 0 || pStart.y >= imgSpec[1]) { jf.err("Invalid pStart-im.getBorderPoint"); }

    POINT pFail;
    pFail.x = -1;
    pFail.y = -1;
    CANDIDATES cd;
    initCandidates(cd);
    cd.initialize(scanCircles, pScanCircleCenter);
    cd.setImg(img, imgSpec);
    vector<unsigned char> rgba;
    int size1, size2;
    double greenBlue, redBlue, redGreen;
    vector<POINT> vpPath = gdi.imgVectorPath(pStart, angle, TLBR);

    for (int ii = 0; ii < vpPath.size(); ii++)
    {
        rgba = pixelRGB(img, imgSpec, vpPath[ii]);
        greenBlue = getGB(rgba);
        redBlue = getRB(rgba);
        redGreen = getRG(rgba);

        // Certainly outside.
        if (greenBlue >= vvdInside[0][1] || greenBlue < vvdInside[0][0]) { continue; }
        else if (redBlue >= vvdInside[1][1] || redBlue < vvdInside[1][0]) { continue; }
        else if (redGreen >= vvdInside[2][1] || redGreen < vvdInside[2][0]) { continue; }
        else if (rgba[0] == rgba[1] && rgba[1] == rgba[2]) { continue; } // Outside map text is a shade of grey.

        // Wrong side of the border.
        size1 = cd.fromCircle(vpPath[ii], 2);
        size2 = cd.removeColour(Canada);
        if (size2 != size1) { continue; }
        size2 = cd.removeColour(City);
        if (size2 != size1) { continue; }
        size2 = cd.removeColour(Province);
        if (size2 != size1) { continue; }
        size2 = cd.removeColour(Water);
        if (size2 != size1) { continue; }
        size2 = cd.removeColourRatio(CANDIDATES::GB, { 0.9, 1.0 });
        if (size2 != size1) { continue; }

        return vpPath[ii];
    }
    return pFail;
}
vector<POINT> IMGFUNC::getCircle(POINT pCenter, int radius)
{
    // Return a list of POINTs that form a rough circle around the given center and radius.
    if (scanCircles.size() < 1) { jf.err("No init for scanCircles-im.getCircle"); }
    if (radius < 1 || radius >= scanCircles.size()) { jf.err("Bad radius-im.getCircle"); }
    vector<POINT> circle = scanCircles[radius - 1];
    for (int ii = 0; ii < circle.size(); ii++)
    {
        circle[ii].x += pCenter.x - pScanCircleCenter.x;
        circle[ii].y += pCenter.y - pScanCircleCenter.y;
    }
    return circle;
}
vector<POINT> IMGFUNC::getCircle(POINT pCenter, int radius, vector<int>& imgSpec)
{
    // Return a list of POINTs that form a rough circle around the given center and radius.
    // If a circle point would step out of bounds, constrain it to the boundary.
    if (scanCircles.size() < 1) { jf.err("No init for scanCircles-im.getCircle"); }
    if (radius < 1 || radius >= scanCircles.size()) { jf.err("Bad radius-im.getCircle"); }
    vector<POINT> circle = scanCircles[radius - 1];
    for (int ii = 0; ii < circle.size(); ii++)
    {
        circle[ii].x += pCenter.x - pScanCircleCenter.x;
        if (circle[ii].x < 0) { circle[ii].x = 0; }
        if (circle[ii].x >= imgSpec[0]) { circle[ii].x = imgSpec[0] - 1; }
        circle[ii].y += pCenter.y - pScanCircleCenter.y;
        if (circle[ii].y < 0) { circle[ii].y = 0; }
        if (circle[ii].y >= imgSpec[1]) { circle[ii].y = imgSpec[1] - 1; }
    }
    return circle;
}
void IMGFUNC::getColourSpectrum(vector<vector<unsigned char>>& rgbaList)
{
    if (rgbaList.size() < 1) { jf.err("No size specified-im.getColourSpectrum"); }
    if (keyColour.size() < 1)
    {
        keyColour.resize(6);
        keyColour[0] = { 255.0, 0.0, 0.0, 255.0 };  // Red
        keyColour[1] = { 255.0, 255.0, 0.0, 255.0 };  // Yellow
        keyColour[2] = { 0.0, 255.0, 0.0, 255.0 };  // Green
        keyColour[3] = { 0.0, 255.0, 255.0, 255.0 };  // Teal
        keyColour[4] = { 0.0, 0.0, 255.0, 255.0 };  // Blue
        keyColour[5] = { 127.0, 0.0, 255.0, 255.0 };  // Violet
    }
    vector<vector<double>> keyColourBand(keyColour.size() - 1, vector<double>(4, 0.0));
    for (int ii = 0; ii < keyColourBand.size(); ii++)
    {
        for (int jj = 0; jj < 4; jj++)
        {
            keyColourBand[ii][jj] = keyColour[ii + 1][jj] - keyColour[ii][jj];
        }
    }
    vector<double> vdColour(4);
    double colourMax = (double)(keyColour.size() - 1), myColour, myColourStart, myColourStop;
    double colourInterval = colourMax / (double)rgbaList.size();
    double dR, dG, dB;
    int iBand;
    for (int ii = 0; ii < rgbaList.size(); ii++)
    {
        myColour = (double)ii * colourInterval;
        myColourStart = floor(myColour);
        myColourStop = myColour - myColourStart;
        iBand = (int)myColourStart;
        dR = keyColourBand[iBand][0] * myColourStop;
        dG = keyColourBand[iBand][1] * myColourStop;
        dB = keyColourBand[iBand][2] * myColourStop;
        vdColour = {
            keyColour[iBand][0] + dR,
            keyColour[iBand][1] + dG,
            keyColour[iBand][2] + dB,
            255.0
        };
        for (int jj = 0; jj < 4; jj++)
        {
            rgbaList[ii][jj] = (unsigned char)vdColour[jj];
        }
    }
}
POINT IMGFUNC::getFirstColour(vector<unsigned char>& img, vector<int>& imgSpec, vector<unsigned char> RGBX)
{
    // Reading through the image, return the first POINT which has the given colour.
    POINT p1;
    vector<unsigned char> rgbx;
    bool letmeout = 0;
    for (int ii = 0; ii < imgSpec[1]; ii++)
    {
        p1.y = ii;
        for (int jj = 0; jj < imgSpec[0]; jj++)
        {
            p1.x = jj;
            rgbx = pixelRGB(img, imgSpec, p1);
            if (rgbx == RGBX)
            {
                letmeout = 1;
                break;
            }
        }
        if (letmeout) { break; }
    }
    return p1;
}
double IMGFUNC::getGB(vector<unsigned char>& rgbx)
{
    // Return the green / blue decimal fraction for this pixel.
    if (rgbx.size() < 3) { jf.err("No init colour-im.getGB"); }
    double GB = (double)rgbx[1] / (double)rgbx[2];
    return GB;
}
double IMGFUNC::getRB(vector<unsigned char>& rgbx)
{
    // Return the red / blue decimal fraction for this pixel.
    if (rgbx.size() < 3) { jf.err("No init colour-im.getRB"); }
    double RB = (double)rgbx[0] / (double)rgbx[2];
    return RB;
}
double IMGFUNC::getRG(vector<unsigned char>& rgbx)
{
    // Return the red / green decimal fraction for this pixel.
    if (rgbx.size() < 3) { jf.err("No init colour-im.getRG"); }
    double RG = (double)rgbx[0] / (double)rgbx[1];
    return RG;
}
POINT IMGFUNC::getTLImgImg(string& targetPath, string& bgPath)
{
    bool fail;
    string pngDataTarget, pngDataBG, sRowTarget, sRowBG;
    vector<int> specTarget, specBG, coordTarget(2), coordBG(2);
    int offsetTarget, offsetBG;
    pngLoadString(targetPath, pngDataTarget, specTarget);
    pngLoadString(bgPath, pngDataBG, specBG);
    // Error funcs...
    string topRow(pngDataTarget.begin(), pngDataTarget.begin() + (specTarget[0] * specTarget[2]));
    int posTL = pngDataBG.find(topRow);
    POINT TL;
    TL.x = -1;
    TL.y = -1;
    while (posTL < pngDataBG.size())
    {
        fail = 0;
        for (int ii = 1; ii < specTarget[1]; ii++)
        {
            offsetTarget = specTarget[0] * ii * specTarget[2];
            offsetBG = posTL + (specBG[0] * ii * specBG[2]);
            sRowTarget.assign(pngDataTarget.begin() + offsetTarget, pngDataTarget.begin() + offsetTarget + (specTarget[0] * specTarget[2]));
            sRowBG.assign(pngDataBG.begin() + offsetBG, pngDataBG.begin() + offsetBG + (specTarget[0] * specBG[2]));
            if (sRowTarget != sRowBG)
            {
                fail = 1;
                break;
            }
        }
        if (!fail)
        {
            TL = coordFromOffset(posTL, specBG);
            return TL;
        }
        posTL = pngDataBG.find(topRow, posTL + 1);
    }
    return TL;
}
void IMGFUNC::initCandidates(CANDIDATES& cd)
{
    if (scanCircles.size() < 1) { jf.err("No scanCircles init-im.initCandidates"); }
    cd.initialize(scanCircles, pScanCircleCenter);
}
void IMGFUNC::initHex()
{
    mapHexDec.emplace('0', 0);
    mapHexDec.emplace('1', 1);
    mapHexDec.emplace('2', 2);
    mapHexDec.emplace('3', 3);
    mapHexDec.emplace('4', 4);
    mapHexDec.emplace('5', 5);
    mapHexDec.emplace('6', 6);
    mapHexDec.emplace('7', 7);
    mapHexDec.emplace('8', 8);
    mapHexDec.emplace('9', 9);
    mapHexDec.emplace('a', 10);
    mapHexDec.emplace('b', 11);
    mapHexDec.emplace('c', 12);
    mapHexDec.emplace('d', 13);
    mapHexDec.emplace('e', 14);
    mapHexDec.emplace('f', 15);

    mapDecHex.emplace(0, '0');
    mapDecHex.emplace(1, '1');
    mapDecHex.emplace(2, '2');
    mapDecHex.emplace(3, '3');
    mapDecHex.emplace(4, '4');
    mapDecHex.emplace(5, '5');
    mapDecHex.emplace(6, '6');
    mapDecHex.emplace(7, '7');
    mapDecHex.emplace(8, '8');
    mapDecHex.emplace(9, '9');
    mapDecHex.emplace(10, 'a');
    mapDecHex.emplace(11, 'b');
    mapDecHex.emplace(12, 'c');
    mapDecHex.emplace(13, 'd');
    mapDecHex.emplace(14, 'e');
    mapDecHex.emplace(15, 'f');
}
int IMGFUNC::initScanCircles(string& pngPath)
{
    vector<unsigned char> img, rgb;
    vector<int> imgSpec;
    int index = 0;
    string sRGB;
    unordered_map<string, int> mapRgbIndex;
    pngLoadHere(pngPath, img, imgSpec);
    POINT p1;
    pScanCircleCenter.x = imgSpec[0] / 2;
    pScanCircleCenter.y = imgSpec[1] / 2;
    p1 = pScanCircleCenter;
    while (p1.x < imgSpec[0])
    {
        p1.x++;
        rgb = pixelRGB(img, imgSpec, p1);
        sRGB = jf.stringifyCoord(rgb);
        if (!mapRgbIndex.count(sRGB))
        {
            mapRgbIndex.emplace(sRGB, index);
            index++;
        }
    }
    scanCircles.resize(mapRgbIndex.size(), vector<POINT>());
    for (int ii = 0; ii < imgSpec[1]; ii++)
    {
        p1.y = ii;
        for (int jj = 0; jj < imgSpec[0]; jj++)
        {
            p1.x = jj;
            rgb = pixelRGB(img, imgSpec, p1);
            if (rgb == White4 || rgb == Black4) { continue; }
            sRGB = jf.stringifyCoord(rgb);
            index = mapRgbIndex.at(sRGB);
            scanCircles[index].push_back(p1);
        }
    }
    return scanCircles.size();
}
vector<POINT> IMGFUNC::loadBox(vector<unsigned char>& img, vector<int>& imgSpec, vector<unsigned char> RGBX)
{
    // Given an image with an existing rgbx rectangle on it, return that rectangle's TLBR.
    if (RGBX.size() != imgSpec[2]) { jf.err("rgbx parameter mismatch-im.loadBox"); }
    vector<POINT> TLBR(2);
    POINT p1 = getFirstColour(img, imgSpec, RGBX);
    TLBR[0] = p1;
    vector<unsigned char> rgbx = pixelRGB(img, imgSpec, p1);
    while (rgbx == RGBX)
    {
        p1.x++;
        if (p1.x == imgSpec[0]) { break; }
        rgbx = pixelRGB(img, imgSpec, p1);
    }
    p1.x--;
    rgbx = pixelRGB(img, imgSpec, p1);
    while (rgbx == RGBX)
    {
        p1.y++;
        if (p1.y == imgSpec[1]) { break; }
        rgbx = pixelRGB(img, imgSpec, p1);
    }
    p1.y--;
    TLBR[1] = p1;
    return TLBR;
}
void IMGFUNC::linePaint(vector<unsigned char>& img, vector<int>& imgSpec, vector<POINT> startStop, vector<unsigned char> rgbx)
{
    // Paint every pixel inside startStop, extrapolating a straight line between the points.
    vector<POINT> path = linePath(startStop);
    for (int ii = 0; ii < path.size(); ii++)
    {
        pixelPaint(img, imgSpec, rgbx, path[ii]);
    }
}
vector<vector<unsigned char>> IMGFUNC::lineRGB(vector<unsigned char>& img, vector<int>& imgSpec, vector<POINT> vpList)
{
    // Return a list of colours, one for each pixel (POINT) given.
    vector<vector<unsigned char>> rgbList(vpList.size(), vector<unsigned char>());
    for (int ii = 0; ii < rgbList.size(); ii++)
    {
        rgbList[ii] = pixelRGB(img, imgSpec, vpList[ii]);
    }
    return rgbList;
}
vector<POINT> IMGFUNC::makeTLBR(vector<POINT>& vpRegion)
{
    vector<POINT> TLBR(2);
    TLBR[0].x = 2147483647;
    TLBR[0].y = 2147483647;
    TLBR[1].x = 0;
    TLBR[1].y = 0;
    for (int ii = 0; ii < vpRegion.size(); ii++)
    {
        if (vpRegion[ii].x < TLBR[0].x) { TLBR[0].x = vpRegion[ii].x; }
        else if (vpRegion[ii].x > TLBR[1].x) { TLBR[1].x = vpRegion[ii].x; }
        if (vpRegion[ii].y < TLBR[0].y) { TLBR[0].y = vpRegion[ii].y; }
        else if (vpRegion[ii].y > TLBR[1].y) { TLBR[1].y = vpRegion[ii].y; }
    }
    return TLBR;
}
void IMGFUNC::markTargetTL(string& targetPath, string& bgPath)
{
    POINT TL = getTLImgImg(targetPath, bgPath);
    vector<unsigned char> imgOutput;
    vector<int> imgOutputDim;
    pngLoadHere(bgPath, imgOutput, imgOutputDim);
    drawSquare(imgOutput, imgOutputDim, TL);
    string resultPath = sroot + "\\Test-1.png";
    pngPrint(imgOutput, imgOutputDim, resultPath);
}
void IMGFUNC::pngCanvas(vector<int>& imgSpec, vector<unsigned char>& img, vector<unsigned char> rgbx)
{
    // Given imgSpec, create the img whereby every pixel is rgbx.
    if (imgSpec.size() < 3) { jf.err("No imgSpec given-im.pngCanvas"); }
    if (rgbx.size() < 3) { jf.err("No rgbx given-im.pngCanvas"); }
    long long imgSize = imgSpec[0] * imgSpec[1] * imgSpec[2];
    int pixelSize = rgbx.size();
    img.clear();
    img.resize(imgSize);
    long long index = 0;
    while (index < imgSize)
    {
        for (int ii = 0; ii < pixelSize; ii++)
        {
            img[index + ii] = rgbx[ii];
        }
        index += pixelSize;
    }
}
vector<unsigned char> IMGFUNC::pngExtractCol(vector<unsigned char>& img, vector<int>& imgSpec, POINT pTop)
{
    vector<unsigned char> vCol;
    vCol.resize(imgSpec[1] * imgSpec[2]);
    int offset, index = 0; 
    for (int ii = 0; ii < imgSpec[1]; ii++)
    {
        pTop.y = ii;
        offset = getOffset(pTop, imgSpec);
        for (int jj = 0; jj < imgSpec[2]; jj++)
        {
            vCol[index] = img[offset + jj];
            index++;
        }
    }
    return vCol;
}
vector<unsigned char> IMGFUNC::pngExtractRow(vector<unsigned char>& img, vector<int>& imgSpec, POINT pLeft)
{
    vector<unsigned char> vRow;
    vRow.resize(imgSpec[0] * imgSpec[2]);
    int offset = getOffset(pLeft, imgSpec);
    for (int ii = 0; ii < vRow.size(); ii++)
    {
        vRow[ii] = img[offset + ii];
    }
    return vRow;
}
void IMGFUNC::pngLoadHere(string& pngPath, vector<unsigned char>& pngData, vector<int>& spec)
{
    // spec has form [width, height, numComponents].
    pngData.clear();
    spec.clear();
    spec.resize(3);
    wstring wsPath = jf.asciiToUTF16(pngPath);
    size_t bufferSize = wsPath.size() * 2;
    char* bufferPath = new char[bufferSize];
    stbi_convert_wchar_to_utf8(bufferPath, bufferSize, wsPath.c_str());
    unsigned char* dataTemp = stbi_load(bufferPath, &spec[0], &spec[1], &spec[2], 0);
    if (dataTemp == NULL) { jf.err("stbi_load-im.pngLoadString"); }
    int sizeTemp = spec[0] * spec[1] * spec[2];
    pngData.resize(sizeTemp);
    copy(dataTemp, dataTemp + sizeTemp, pngData.begin());
    delete[] bufferPath;
}
void IMGFUNC::pngLoadString(string& pngPath, string& pngData, vector<int>& spec)
{
    // spec has form [width, height, numComponents].
    pngData.clear();
    spec.clear();
    spec.resize(3);
    string pathASCII;
    unsigned char* dataTemp = stbi_load(pngPath.c_str(), &spec[0], &spec[1], &spec[2], 0);
    if (dataTemp == NULL)
    {
        pathASCII = jf.asciiOnly(pngPath);
        dataTemp = stbi_load(pathASCII.c_str(), &spec[0], &spec[1], &spec[2], 0);
        if (dataTemp == NULL) { jf.err("stbi_load-im.pngLoadString"); }
    }
    int sizeTemp = spec[0] * spec[1] * spec[2];
    pngData.resize(sizeTemp);
    copy(dataTemp, dataTemp + sizeTemp, pngData.begin());
}
string IMGFUNC::printDebugMap(vector<unsigned char> img, vector<int> imgSpec, vector<POINT>& vpBorder)
{
    if (keyColour.size() < 1)
    {
        keyColour.resize(6);
        keyColour[0] = { 255.0, 0.0, 0.0, 255.0 };  // Red
        keyColour[1] = { 255.0, 255.0, 0.0, 255.0 };  // Yellow
        keyColour[2] = { 0.0, 255.0, 0.0, 255.0 };  // Green
        keyColour[3] = { 0.0, 255.0, 255.0, 255.0 };  // Teal
        keyColour[4] = { 0.0, 0.0, 255.0, 255.0 };  // Blue
        keyColour[5] = { 127.0, 0.0, 255.0, 255.0 };  // Violet
    }
    vector<vector<unsigned char>> rgbaList(vpBorder.size(), vector<unsigned char>(4));
    int numBand = keyColour.size() - 1;
    int rgbaBandWidth = vpBorder.size() / numBand, index = 0, iNum;
    int rgbaBandRemainder = vpBorder.size() % numBand;
    double dNum, dR, dG, dB, DR, DG, DB;
    for (int ii = 0; ii < numBand; ii++)
    {
        if (ii > 0) { iNum = rgbaBandWidth; }
        else { iNum = rgbaBandWidth + rgbaBandRemainder; }
        dNum = (double)iNum;
        dR = (keyColour[ii + 1][0] - keyColour[ii][0]) / dNum;
        dG = (keyColour[ii + 1][1] - keyColour[ii][1]) / dNum;
        dB = (keyColour[ii + 1][2] - keyColour[ii][2]) / dNum;
        for (int jj = 0; jj < iNum; jj++)
        {
            DR = (double)jj * dR;
            DG = (double)jj * dG;
            DB = (double)jj * dB;
            rgbaList[index][0] = (unsigned char)(keyColour[ii][0] + round(DR));
            rgbaList[index][1] = (unsigned char)(keyColour[ii][1] + round(DG));
            rgbaList[index][2] = (unsigned char)(keyColour[ii][2] + round(DB));
            rgbaList[index][3] = 255;
            index++;
        }
    }
    for (int ii = 0; ii < vpBorder.size(); ii++)
    {
        drawSquare(img, imgSpec, vpBorder[ii], 3, rgbaList[ii]);
    }
    string outputPath = sroot + "\\debug\\ImgDebugMap\\ErrorMap" + to_string(vpBorder.size()) + ".png";
    pngPrint(outputPath, img, imgSpec);
    return outputPath;
}
void IMGFUNC::rectPaint(vector<unsigned char>& img, vector<int>& imgSpec, vector<POINT> TLBR, vector<unsigned char> rgba)
{
    vector<POINT> startStop = TLBR;
    startStop[1].y = startStop[0].y;
    linePaint(img, imgSpec, startStop, rgba);
    startStop[0] = startStop[1];
    startStop[1].y = TLBR[1].y;
    linePaint(img, imgSpec, startStop, rgba);
    startStop[0] = startStop[1];
    startStop[1].x = TLBR[0].x;
    linePaint(img, imgSpec, startStop, rgba);
    startStop[0] = startStop[1];
    startStop[1].y = TLBR[0].y;
    linePaint(img, imgSpec, startStop, rgba);
}
void IMGFUNC::rectPaint(vector<unsigned char>& img, vector<int>& imgSpec, vector<POINT> TLBR, vector<vector<unsigned char>> rgba)
{
    // rgba has form [border, interior][colour].
    vector<POINT> startStop = TLBR;
    startStop[1].y = startStop[0].y;
    linePaint(img, imgSpec, startStop, rgba[0]);
    startStop[0] = startStop[1];
    startStop[1].y = TLBR[1].y;
    linePaint(img, imgSpec, startStop, rgba[0]);
    startStop[0] = startStop[1];
    startStop[1].x = TLBR[0].x;
    linePaint(img, imgSpec, startStop, rgba[0]);
    startStop[0] = startStop[1];
    startStop[1].y = TLBR[0].y;
    linePaint(img, imgSpec, startStop, rgba[0]);

    int thickness = 1;
    POINT p1;
    for (int ii = TLBR[0].y + thickness; ii <= TLBR[1].y - thickness; ii++)
    {
        p1.y = ii;
        for (int jj = TLBR[0].x + thickness; jj <= TLBR[1].x - thickness; jj++)
        {
            p1.x = jj;
            pixelPaint(img, imgSpec, rgba[1], p1);
        }
    }
}
void IMGFUNC::removePeriodic(vector<unsigned char>& img, int modulus)
{
    // Every 'modulus' value is erased from the image.
    vector<unsigned char> vucTemp = img;
    int index = 0;
    for (int ii = 0; ii < vucTemp.size(); ii++)
    {
        if (ii % modulus != modulus - 1)
        {
            img[index] = vucTemp[ii];
            index++;
        }
    }
    img.resize(index);
}
void IMGFUNC::scanPatternLineH(vector<unsigned char>& img, vector<int>& imgSpec, vector<vector<vector<unsigned char>>>& colourList, vector<int>& viResult)
{
    vector<vector<int>> startStop = { {0, imgSpec[0] - 1}, { 0, imgSpec[1] - 1 } };
    scanPatternLineH(img, imgSpec, colourList, viResult, startStop);
}
void IMGFUNC::scanPatternLineH(vector<unsigned char>& img, vector<int>& imgSpec, vector<vector<vector<unsigned char>>>& colourList, vector<int>& viResult, vector<POINT> TLBR)
{
    vector<vector<int>> startStop = { { TLBR[0].x, TLBR[1].x }, { TLBR[0].y, TLBR[1].y } };
    scanPatternLineH(img, imgSpec, colourList, viResult, startStop);
}
void IMGFUNC::scanPatternLineH(vector<unsigned char>& img, vector<int>& imgSpec, vector<vector<vector<unsigned char>>>& colourList, vector<int>& viResult, vector<vector<int>> startStop)
{
    // viResult contains the indices of the columns that contained the pattern. Other 
    // colours not specified are ignored completely. Only checks within startStop,
    // which has form [horizontal, vertical][first pixel, last pixel].
    // colourList has form [standard, shaded][colour0, colour1, ...][rgb]
    if (colourList[0].size() != colourList[1].size()) { jf.err("colourList size mismatch-im.scanPatternLineH"); }
    vector<unsigned char> rgbx;
    unordered_map<string, int> mapColour;
    vector<int> pattern, mask;
    int index = 0, iColour, maxColour;
    string srgbx, srgbx0, srgbx1;
    POINT p1;
    viResult.clear();
    for (int ii = 0; ii < colourList[0].size(); ii++)
    {
        srgbx0 = jf.stringifyCoord(colourList[0][ii]);
        srgbx1 = jf.stringifyCoord(colourList[1][ii]);
        if (mapColour.count(srgbx0))
        {
            iColour = mapColour.at(srgbx0);
        }
        else
        {
            mapColour.emplace(srgbx0, index);
            mapColour.emplace(srgbx1, index);
            iColour = index;
            index++;
        }
        pattern.push_back(iColour);
    }
    maxColour = index - 1;
    for (int ii = startStop[1][0]; ii <= startStop[1][1]; ii++)
    {
        p1.y = ii;
        mask.assign(pattern.size(), 1);
        for (int jj = startStop[0][0]; jj <= startStop[0][1]; jj++)
        {
            p1.x = jj;
            rgbx = pixelRGB(img, imgSpec, p1);
            srgbx = jf.stringifyCoord(rgbx);
            try { iColour = mapColour.at(srgbx); }
            catch (out_of_range)
            {
                mapColour.emplace(srgbx, index);
                index++;
                continue;
            }
            if (iColour > maxColour) { continue; }
            for (int kk = 0; kk < pattern.size(); kk++)
            {
                if (mask[kk])
                {
                    if (iColour == pattern[kk])
                    {
                        mask[kk] = 0;
                    }
                    else if (kk > 0 && iColour == pattern[kk - 1]) {}
                    else
                    {
                        mask.assign(pattern.size(), 1);
                    }
                    break;
                }
            }
            if (mask[0]) // If this pixel failed to advance the pattern, we can always try to restart from the beginning.
            {
                if (iColour == pattern[0]) { mask[0] = 0; }
            }
            if (mask[pattern.size() - 1] == 0)
            {
                viResult.push_back(ii);
                break;
            }
        }
    }
}
void IMGFUNC::scanPatternLineV(vector<unsigned char>& img, vector<int>& imgSpec, vector<vector<vector<unsigned char>>>& colourList, vector<int>& viResult)
{
    vector<vector<int>> startStop = { {0, imgSpec[0] - 1}, { 0, imgSpec[1] - 1 } };
    scanPatternLineV(img, imgSpec, colourList, viResult, startStop);
}
void IMGFUNC::scanPatternLineV(vector<unsigned char>& img, vector<int>& imgSpec, vector<vector<vector<unsigned char>>>& colourList, vector<int>& viResult, vector<POINT> TLBR)
{
    vector<vector<int>> startStop = { { TLBR[0].x, TLBR[1].x }, { TLBR[0].y, TLBR[1].y } };
    scanPatternLineV(img, imgSpec, colourList, viResult, startStop);
}
void IMGFUNC::scanPatternLineV(vector<unsigned char>& img, vector<int>& imgSpec, vector<vector<vector<unsigned char>>>& colourList, vector<int>& viResult, vector<vector<int>> startStop)
{
    // viResult contains the indices of the columns that contained the pattern. Other 
    // colours not specified are ignored completely. Only checks within startStop,
    // which has form [horizontal, vertical][first pixel, last pixel].
    // colourList has form [standard, shaded][colour0, colour1, ...][rgb]
    vector<unsigned char> rgbx;
    unordered_map<string, int> mapColour;
    vector<int> pattern, mask;
    int index = 0, iColour, maxColour;
    string srgbx, srgbx0, srgbx1;
    POINT p1;
    viResult.clear();
    for (int ii = 0; ii < colourList[0].size(); ii++)
    {
        srgbx0 = jf.stringifyCoord(colourList[0][ii]);
        srgbx1 = jf.stringifyCoord(colourList[1][ii]);
        if (mapColour.count(srgbx0))
        {
            iColour = mapColour.at(srgbx0);
        }
        else
        {
            mapColour.emplace(srgbx0, index);
            mapColour.emplace(srgbx1, index);
            iColour = index;
            index++;
        }
        pattern.push_back(iColour);
    }
    maxColour = index - 1;
    for (int ii = startStop[0][0]; ii <= startStop[0][1]; ii++)
    {
        p1.x = ii;
        mask.assign(pattern.size(), 1);
        for (int jj = startStop[1][0]; jj <= startStop[1][1]; jj++)
        {
            p1.y = jj;
            rgbx = pixelRGB(img, imgSpec, p1);
            srgbx = jf.stringifyCoord(rgbx);
            try { iColour = mapColour.at(srgbx); }
            catch (out_of_range) 
            { 
                mapColour.emplace(srgbx, index);
                index++;
                continue; 
            }
            if (iColour > maxColour) { continue; }
            for (int kk = 0; kk < pattern.size(); kk++)
            {
                if (mask[kk])
                {
                    if (iColour == pattern[kk])
                    {
                        mask[kk] = 0;
                    }
                    else if (kk > 0 && iColour == pattern[kk - 1]) {}
                    else
                    {
                        mask.assign(pattern.size(), 1);
                    }
                    break;
                }
            }
            if (mask[0]) // If this pixel failed to advance the pattern, we can always try to restart from the beginning.
            {
                if (iColour == pattern[0]) { mask[0] = 0; }
            }
            if (mask[pattern.size() - 1] == 0)
            {
                viResult.push_back(ii);
                break;
            }
        }
    }
}
void IMGFUNC::stretch(vector<unsigned char>& img, vector<int>& imgSpec, vector<int> imgSpecNew)
{
    // NOTE: UNFINISHED.
    vector<unsigned char> imgOld, pngRow;
    double oldNewRatio, dRow;
    long long index;
    POINT p1;
    if (imgSpecNew[1] < imgSpec[1])
    {
        imgOld = img;
        oldNewRatio = (double)imgSpec[1] / (double)imgSpecNew[1];
        dRow = 0.0;
        p1.x = 0;
        index = 0;
        for (int ii = 0; ii < imgSpecNew[1]; ii++)
        {
            dRow += oldNewRatio;
            p1.y = int(floor(dRow));
            pngRow = pngExtractRow(imgOld, imgSpec, p1);
            for (int jj = 0; jj < pngRow.size(); jj++)
            {
                img[index + jj] = pngRow[jj];
            }
            index += pngRow.size();
        }
        img.resize(index);
        imgSpec[1] = imgSpecNew[1];
    }
}


int IMGFUNC::areaRect(vector<vector<int>> TLBR)
{
    int area = (TLBR[1][0] - TLBR[0][0]) * (TLBR[1][1] - TLBR[0][1]);
    if (area < 0) { jf.err("Negative area-im.areaRect"); }
    return area;
}
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
    int pixelJump = 1, offset = 0; 
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
vector<vector<int>> IMGFUNC::getFrame(vector<vector<int>> vVictor, vector<string> inOut)
{
    // Returns the TLBR for the frame coloured inOut[1] against a background of inOut[0]. 
    // Travels along the path vVictor, which has form [origin, direction][coords].
    vector<string> outIn = { inOut[1], inOut[1] };
    vector<vector<int>> crossing = zoneChangeLinear(inOut, vVictor);
    string sZone = "white";
    vector<int> dxdy = normalForce(crossing[1], sZone);
    if (dxdy[0] > 0) { dxdy[0] = 1; }
    else if (dxdy[0] < 0) { dxdy[0] = -1; }
    if (dxdy[1] > 0) { dxdy[1] = 1; }
    else if (dxdy[1] < 0) { dxdy[1] = -1; }
    turnClockwise(dxdy);
    vVictor[0] = crossing[1];
    vVictor[1] = dxdy;
    int inum;
    vector<unsigned char> rgb;
    vector<vector<int>> corners(4, vector<int>(2));
    for (int ii = 0; ii < 4; ii++)
    {
        crossing = zoneChangeLinear(outIn, vVictor);
        rgb = pixelRGB(crossing[1]);
        inum = rgb[0] + rgb[1] + rgb[2];
        if (inum > 382)  // Closer to white than black.
        {
            corners[ii] = crossing[0];
        }
        else  // Closer to black than white.
        {
            corners[ii] = crossing[1];
        }
        vVictor[0] = corners[ii];
        turnClockwise(vVictor[1]);
        vVictor[0][0] += (2 * vVictor[1][0]);  // Jump over 2 pixels to avoid 
        vVictor[0][1] += (2 * vVictor[1][1]);  // antialiasing problems.
    }
    vector<vector<int>> TLBR = getTLBR(corners);
    return TLBR;
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
vector<double> IMGFUNC::getPosition(string pathPNGpos, vector<vector<unsigned char>> rgb)
{
    // Returns the center of rgb[0] within rgb[1]'s rectangular frame.
    pngLoad(pathPNGpos);
    vector<int> sourceDim = { width, height };
    vector<vector<int>> parentTLBR = makeBox(dataPNG, sourceDim, rgb[1]); // Parent region frame.
    vector<vector<int>> dotTLBR = makeBox(dataPNG, sourceDim, rgb[0]); // Child dot frame.
    int centerX = ((dotTLBR[1][0] - dotTLBR[0][0]) / 2) + dotTLBR[0][0];
    int centerY = ((dotTLBR[1][1] - dotTLBR[0][1]) / 2) + dotTLBR[0][1];
    if (centerX < parentTLBR[0][0]) { parentTLBR[0][0] = centerX; }
    else if (centerX > parentTLBR[1][0]) { parentTLBR[1][0] = centerX; }
    if (centerY < parentTLBR[0][1]) { parentTLBR[0][1] = centerY; }
    else if (centerY > parentTLBR[1][1]) { parentTLBR[1][1] = centerY; }
    double frameWidth = (double)(parentTLBR[1][0] - parentTLBR[0][0]);
    double frameHeight = (double)(parentTLBR[1][1] - parentTLBR[0][1]);
    vector<double> output(2);
    output[0] = (double)(centerX - parentTLBR[0][0]) / frameWidth;
    output[1] = (double)(centerY - parentTLBR[0][1]) / frameHeight;
    return output;
}
double IMGFUNC::getPPKM(string& pathTXT, int scalePixels)
{
    // Read a map's scale bar, and return the number of pixels per kilometer.
    string sfile = jf.load(pathTXT), temp;
    size_t pos2 = sfile.rfind("km"), pos1;
    if (pos2 > sfile.size())
    {
    FTL1:
        pos2 = sfile.rfind('m');
        while (pos2 < sfile.size())
        {
            pos1 = sfile.find_last_not_of("1234567890", pos2 - 1) + 1;
            if (pos2 - pos1 > 0) { break; }
            pos2 = sfile.rfind('m', pos2 - 1);
        }
        if (pos2 > sfile.size()) { jf.err("Cannot find kilometer-im.getPPKM"); }
        temp = sfile.substr(pos1, pos2 - pos1);
    }
    else
    {
        while (pos2 < sfile.size())
        {
            pos1 = sfile.find_last_not_of("1234567890", pos2 - 1) + 1;
            if (pos2 - pos1 > 0) { break; }
            pos2 = sfile.rfind("km", pos2 - 1);
        }
        if (pos2 > sfile.size()) { goto FTL1; }
        temp = sfile.substr(pos1, pos2 - pos1);
    }

    int iKM;
    try { iKM = stoi(temp); }
    catch (invalid_argument& ia) { jf.err("stoi-im.getPPKM"); }    
    double PPKM = (double)scalePixels / (double)iKM;  // Pixels Per Kilometer.
    return PPKM;
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
int IMGFUNC::getScalePixels(vector<vector<int>> TLBR)
{
    // Determine the length (in pixels) that spans the scaling bar. 
    int scaleLength, thickness = 0, count = 0;
    bool black = 0;
    vector<int> sourceDim = { width, height };
    vector<int> extractDim = { TLBR[1][0] - TLBR[0][0], TLBR[1][1] - TLBR[0][1] };
    vector<unsigned char> scaleFrame = pngExtractRectTopLeft(TLBR[0], dataPNG, sourceDim, extractDim);
    vector<int> coord = { 0, extractDim[1] / 2 };
    vector<unsigned char> rgb = pixelRGB(coord, scaleFrame, extractDim);
    string sZone = pixelZone(rgb);
    if (sZone != "black") { jf.err("Failed to measure wall thickness-im.getScalePixels"); }
    while (sZone == "black")
    {
        thickness++;
        coord[0]++;
        rgb = pixelRGB(coord, scaleFrame, extractDim);
        sZone = pixelZone(rgb);
    }
    thickness += 2;  // Compensating for antialiasing...
    vector<int> olympicJump = { -1, -1 };  // Form [greatest length in pixels, row index].
    for (int ii = thickness; ii < extractDim[1] - thickness; ii++)
    {  // Thickness is here to exclude the black frame wall from the search.
        for (int jj = 0; jj < extractDim[0]; jj++)
        {
            coord = { jj, ii };
            rgb = pixelRGB(coord, scaleFrame, extractDim);
            sZone = pixelZone(rgb);
            if (sZone == "black")
            {
                if (black) { count++; }
                else
                {
                    black = 1;
                    count = 1;
                }
            }
            else
            {
                if (black)
                {
                    black = 0;
                    if (count > olympicJump[0])
                    {
                        olympicJump[0] = count;
                        olympicJump[1] = ii;
                    }
                }
            }
        }
    }
    olympicJump[0] -= 2;  // Measuring from the 'middle' of each black notch on the scale bar.
    return olympicJump[0];
}
vector<int> IMGFUNC::getSourceDim()
{
    vector<int> sd = { width, height };
    return sd;
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
vector<vector<int>> IMGFUNC::getTLBR(vector<vector<int>> corners)
{
    vector<vector<int>> TLBR(2, vector<int>(2));
    vector<int> minMax = { 2147483647, 2147483647 };
    int index = -1;
    for (int ii = 0; ii < 4; ii++)
    {
        if (corners[ii][0] < minMax[0])
        {
            minMax[0] = corners[ii][0]; 
            minMax[1] = corners[ii][1];
            index = ii;
        }
        else if (corners[ii][0] == minMax[0])
        {
            if (corners[ii][1] < minMax[1])
            {
                minMax[1] = corners[ii][1];
                index = ii;
            }
        }
    }
    TLBR[0] = corners[index];
    
    minMax = { 0, 0 };
    index = -1;
    for (int ii = 0; ii < 4; ii++)
    {
        if (corners[ii][0] > minMax[0])
        {
            minMax[0] = corners[ii][0];
            minMax[1] = corners[ii][1];
            index = ii;
        }
        else if (corners[ii][0] == minMax[0])
        {
            if (corners[ii][1] > minMax[1])
            {
                minMax[1] = corners[ii][1];
                index = ii;
            }
        }
    }
    TLBR[1] = corners[index];

    return TLBR;
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
vector<POINT> IMGFUNC::linePath(vector<POINT>& startStop)
{
    vector<POINT> path;
    double Dx = (double)(startStop[1].x - startStop[0].x);
    double Dy = (double)(startStop[1].y - startStop[0].y);
    double dx, dy, xCoord, yCoord;
    int ix, iy;
    if (abs(Dx) > abs(Dy))
    {
        dy = Dy / abs(Dx);
        if (Dx > 0) { ix = 1; }
        else if (Dx < 0) { ix = -1; }
        else { ix = 0; }
        path.resize(abs(Dx) + 1);
        path[0] = startStop[0];
        yCoord = (double)startStop[0].y;
        for (int ii = 1; ii < path.size(); ii++)
        {
            yCoord += dy;
            path[ii].x = path[ii - 1].x + ix;
            path[ii].y = int(round(yCoord));
        }
    }
    else
    {
        dx = Dx / abs(Dy);
        if (Dy > 0) { iy = 1; }
        else if (Dy < 0) { iy = -1; }
        else { iy = 0; }
        path.resize(abs(Dy) + 1);
        path[0] = startStop[0];
        xCoord = (double)startStop[0].x;
        for (int ii = 1; ii < path.size(); ii++)
        {
            xCoord += dx;
            path[ii].x = int(round(xCoord));
            path[ii].y = path[ii - 1].y + iy;
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
                dist = mf.coordDist(pointWrong, savePoints[ii][jj]);
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
void IMGFUNC::makePositionPNG(string& pngPath, string& pngFramePath)
{
    pngLoad(pngPath);
    vector<vector<vector<int>>> threeFrames = pngThreeFrames();
    vector<int> sourceDim = { width, height };
    vector<int> extractDim = { threeFrames[2][1][0] - threeFrames[2][0][0], threeFrames[2][1][1] - threeFrames[2][0][1] };
    vector<unsigned char> posFrameImg = pngExtractRectTopLeft(threeFrames[2][0], dataPNG, sourceDim, extractDim);
    int imgSize = posFrameImg.size();
    int channels = 3;
    auto bufferUC = new unsigned char[imgSize];
    for (int ii = 0; ii < imgSize; ii++)
    {
        bufferUC[ii] = posFrameImg[ii];
    }
    string pngFramePathASCII = jf.asciiOnly(pngFramePath);
    int success = stbi_write_png(pngFramePathASCII.c_str(), extractDim[0], extractDim[1], channels, bufferUC, 0);
    delete[] bufferUC;
    if (!success) { jf.err("stbi_write_png-im.makePositionPNG"); }
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
vector<int> IMGFUNC::normalForce(vector<int> origin, string sZone)
{
    // Return the cardinal (N, E, S, W) direction { +- X, +- Y }
    // along which we find the first sZone from origin. 
    vector<int> compassDir = { 0, 0 };
    vector<vector<int>> neswCoord(4, vector<int>(2));
    vector<unsigned char> rgb;
    int radius = 0;
    string szone;
    bool found = 0;
    while (!found)
    {
        radius++;
        neswCoord[0][0] = origin[0];
        neswCoord[0][1] = origin[1] - radius;
        neswCoord[1][0] = origin[0] + radius;
        neswCoord[1][1] = origin[1];
        neswCoord[2][0] = origin[0];
        neswCoord[2][1] = origin[1] + radius;
        neswCoord[3][0] = origin[0] - radius;
        neswCoord[3][1] = origin[1];
        for (int ii = 0; ii < 4; ii++)
        {
            rgb = pixelRGB(neswCoord[ii]);
            szone = pixelZone(rgb);
            if (szone == sZone)
            {
                found = 1;
                compassDir[0] += neswCoord[ii][0] - origin[0];
                compassDir[1] += neswCoord[ii][1] - origin[1];
            }
        }
    }
    return compassDir;
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
void IMGFUNC::pixelPaint(vector<unsigned char>& img, vector<int>& imgSpec, vector<unsigned char> rgbx, POINT coord)
{
    int offset = getOffset(coord, imgSpec);
    img[offset + 0] = rgbx[0];
    img[offset + 1] = rgbx[1];
    img[offset + 2] = rgbx[2];
    if (rgbx.size() == 4) { img[offset + 3] = rgbx[3]; }
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
    vector<unsigned char> vRow;
    vRow.resize(3 * sourceDim[0]);
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
    if (!mapIsInit()) { initMapColours(); }
    string pathASCII;
	unsigned char* dataTemp = stbi_load(pathPNG.c_str(), &width, &height, &numComponents, 0);
    if (dataTemp == NULL)
    {
        pathASCII = jf.asciiOnly(pathPNG);
        dataTemp = stbi_load(pathASCII.c_str(), &width, &height, &numComponents, 0);
        if (dataTemp == NULL) { jf.err("stbi_load-im.pngLoad"); }
    }
    int sizeTemp = width * height * numComponents;
	dataPNG.resize(sizeTemp);
	copy(dataTemp, dataTemp + sizeTemp, dataPNG.begin());
    pathActivePNG = pathPNG;
    pauseVBP = defaultPathLengthImageDebug;
}
vector<vector<vector<int>>> IMGFUNC::pngThreeFrames()
{
    // For the active PNG map, extract the TLBR of its three frames 
    // (map, scale, position). These frames always have a size hierarchy 
    // map > scale > position. Output has form ...
    // [map frame, scale frame, position frame][topLeft, botRight][coords].
    
    vector<vector<vector<int>>> threeFrames(3, vector<vector<int>>(2, vector<int>(2)));
    vector<vector<vector<int>>> fourFrames(4, vector<vector<int>>(2, vector<int>(2)));
    vector<vector<int>> vVictor;
    vector<int> viTemp;
    vector<vector<int>> areaFrame(4, vector<int>(2));  // Form [frame][area, index in threeFrames].
    vector<string> inOut = { "white", "black" };
    for (int ii = 0; ii < 4; ii++)
    {
        switch (ii)
        {
        case 0:
            vVictor = { { 0, 0 }, { 1, 1 } };
            break;
        case 1:
            vVictor = { { width - 1, 0 }, { -1, 1 } };
            break;
        case 2:
            vVictor = { { width - 1, height - 1 }, { -1, -1 } };
            break;
        case 3:
            vVictor = { { 0, height - 1 }, { 1, -1 } };
            break;
        }
        fourFrames[ii] = getFrame(vVictor, inOut);
        areaFrame[ii][0] = areaRect(fourFrames[ii]);
        areaFrame[ii][1] = ii;
    }
    
    // Sort the frames into the form [map, scale, position], and note the unwanted frame duplicate.
    int count = 1;
    while (count > 0)
    {
        count = 0;
        for (int ii = 0; ii < 3; ii++)
        {
            if (areaFrame[ii + 1][0] > areaFrame[ii][0])
            {
                viTemp = areaFrame[ii];
                areaFrame[ii] = areaFrame[ii + 1];
                areaFrame[ii + 1] = viTemp;
                count++;
            }
        }
    }
    int diff = 2147483647, index = -1;
    for (int ii = 0; ii < 3; ii++)
    {
        if (areaFrame[ii][0] - areaFrame[ii + 1][0] < diff)
        {
            diff = areaFrame[ii][0] - areaFrame[ii + 1][0];
            index = ii + 1;
        }
    }
    if (index < 1 || index > 3) { jf.err("bad index-im.pngThreeFrames"); }

    // Build and return the (map, scale, position) frame list.
    count = 0;
    for (int ii = 0; ii < 4; ii++)
    {
        if (ii == index) { continue; }
        threeFrames[count] = fourFrames[areaFrame[ii][1]];
        count++;
    }
    return threeFrames;
}
void IMGFUNC::pngToBin(SWITCHBOARD& sbgui, string& pathPNG, string& pathBIN)
{
    // Creates a "bin map" file, which is a list of coordinates describing a region border.
    pngLoad(pathPNG);
    string pathMapPTB = sroot + "\\debug\\PTB.png";
    string pathMapPTBdebug = sroot + "\\debug\\PTBdebug.png";
    vector<string> vsTemp, prompt = sbgui.get_prompt();
    bool single = 0;
    if (prompt.size() == 4) 
    { 
        single = 1; 
        pointOfOrigin = jf.destringifyCoord(prompt[1]);
    }
    vector<int> viTemp;
    vector<vector<int>> vBorderPath(1, vector<int>()), tracks, commGui;
    if (!single) { vBorderPath[0] = borderFindStart(); }
    else 
    { 
        vBorderPath[0] = jf.destringifyCoord(prompt[1]);
    }
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
            pathMapDebug = pathMapPTBdebug;
        }
    }
    if (!single) { printBinMap(pathBIN, vBorderPath); }
    else { printBinMapSingle(pathBIN, vBorderPath); }
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
    vector<vector<int>> corners = frameCorners();
    vector<vector<double>> cornersD;
    jf.toDouble(corners, cornersD);
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
        border = cornersD;
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
void IMGFUNC::printBinMap(string& pathBIN, vector<vector<int>>& vBorderPath)
{
    // Gather the data to be printed.
    vector<vector<vector<int>>> threeFrames = pngThreeFrames();  // [map, scale, position][topLeft, botRight][coords].
    int scalePixels = getScalePixels(threeFrames[1]);
    vector<string> dirt = { "mapsBIN", ".bin" };
    vector<string> soap = { "mapsPDF", ".txt" };
    string pathTXT = pathBIN;
    jstr.clean(pathTXT, dirt, soap);
    double PPKM = getPPKM(pathTXT, scalePixels);
    
    // Write the BIN map.
    string mapBIN = "//frames(topLeft@botRight, showing 'maps', 'scale', 'position')\n";
    for (int ii = 0; ii < threeFrames.size(); ii++)
    {
        mapBIN += to_string(threeFrames[ii][0][0]) + "," + to_string(threeFrames[ii][0][1]) + "@" + to_string(threeFrames[ii][1][0]) + "," + to_string(threeFrames[ii][1][1]) + "\n";
    }
    mapBIN += "\n//scale(pixels per km)\n";
    mapBIN += to_string(PPKM) + "\n";
    mapBIN += "\n//border\n";
    for (int ii = 0; ii < vBorderPath.size(); ii++)
    {
        mapBIN += to_string(vBorderPath[ii][0]) + "," + to_string(vBorderPath[ii][1]) + "\n";
    }
    jf.printer(pathBIN, mapBIN);
}
void IMGFUNC::printBinMapSingle(string& pathBIN, vector<vector<int>>& vBorderPath)
{
    // Load frames and scale from the parent map.
    string parentPath = "F:\\mapsBIN\\Canada.bin";
    string parentBin = jf.load(parentPath);
    size_t pos1 = parentBin.find("//scale");
    pos1 = parentBin.find('\n', pos1) + 1;
    pos1 = parentBin.find('\n', pos1) + 1;
    string mapBIN = parentBin.substr(0, pos1);

    // Write the new BIN map.
    mapBIN += "\n//border\n";
    for (int ii = 0; ii < vBorderPath.size(); ii++)
    {
        mapBIN += to_string(vBorderPath[ii][0]) + "," + to_string(vBorderPath[ii][1]) + "\n";
    }
    jf.printer(pathBIN, mapBIN);
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
            dist = mf.coordDist(candidates[ii], tracks[jj]);
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
        dTemp = mf.angleBetweenVectors(pastPresentFuture);
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
    shortestToEdge[1] = int(mf.hypotenuse(width, height));
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
void IMGFUNC::turnClockwise(vector<int>& dxdy)
{
	int magnitude;
	if (dxdy[1] < 0)  // Normal force is northward, so travel eastward.
	{
		if (dxdy[0] != 0) { jf.err("Input direction not NESW-im.turnClockwise"); }
		magnitude = -1 * dxdy[1];
		dxdy = { magnitude, 0 };
	}
	else if (dxdy[0] > 0)  // Normal force is eastward, so travel southward.
	{
		if (dxdy[1] != 0) { jf.err("Input direction not NESW-im.turnClockwise"); }
		magnitude = dxdy[0];
		dxdy = { 0, magnitude };
	}
	else if (dxdy[1] > 0)  // Normal force is southward, so travel westward.
	{
		if (dxdy[0] != 0) { jf.err("Input direction not NESW-im.turnClockwise"); }
		magnitude = dxdy[1];
		dxdy = { -1 * magnitude, 0 };
	}
	else if (dxdy[0] < 0)  // Normal force is westward, so travel northward.
	{
		if (dxdy[1] != 0) { jf.err("Input direction not NESW-im.turnClockwise"); }
		magnitude = dxdy[0];
		dxdy = { 0, magnitude };
	}
	else { jf.err("Cannot determine input direction-im.turnClockwise"); }
}
vector<vector<int>> IMGFUNC::zoneChangeLinear(vector<string>& szones, vector<vector<int>>& ivec)
{
    // This function returns the coordinates of the specified zone change,
    // by travelling along a given straight line. 
    // Form(szones): [pre-border szone, post-border szone]. Note: indepdendent of starting zone.
    // Form(ivec): [0][starting xCoord, starting yCoord], [1][delta x, delta y]. Note: delta can be negative, but coords cannot be.
    // Form(return): [0][final pre-border xCoord, final pre-border yCoord], [1][first post-border xCoord, first post-border yCoord].
    // The szone "known" counts as every saved szone except "unknown". 
    // If the same szone is given as pre and post, then the post-border szone is 
    // defined to be any szone (including "unknown") EXCEPT the given szone.

    vector<vector<int>> vBorder(2, vector<int>(2));
    bool coord = 1, anythingBut;
    if (szones[0] == szones[1]) { anythingBut = 1; }
    else { anythingBut = 0; }
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
        if (anythingBut)
        {
            temp = pixelZone(rgb);
            if (temp != zoneNew)
            {
                // New zone! Objective found!
                if (coord)
                {
                    vBorder[0] = coordA;
                    vBorder[1] = coordB;
                }
                else
                {
                    vBorder[0] = coordB;
                    vBorder[1] = coordA;
                }
                return vBorder;
            }
        }
        else
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
