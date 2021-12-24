#include "gdifunc.h"

void GDIFUNC::capture(vector<unsigned char>& img, vector<int>& imgSpec, vector<POINT> TLBR)
{
    if (TLBR.size() < 2) { jf.err("TLBR missing-gdi.capture"); }
    HWND hWindow = GetDesktopWindow();
    HBITMAP hBitmap = GdiPlusScreenCapture(hWindow);
    CImage cImg;
    cImg.Attach(hBitmap);
    COLORREF cr1;
    imgSpec.resize(3);
    imgSpec[0] = TLBR[1].x - TLBR[0].x + 1;
    imgSpec[1] = TLBR[1].y - TLBR[0].y + 1;
    imgSpec[2] = 4;
    img.clear();
    img.resize(imgSpec[0] * imgSpec[1] * imgSpec[2]);
    long long index = 0;
    for (int ii = TLBR[0].y; ii <= TLBR[1].y; ii++)
    {
        for (int jj = TLBR[0].x; jj <= TLBR[1].x; jj++)
        {
            cr1 = cImg.GetPixel(jj, ii);
            img[index] = GetRValue(cr1);
            img[index + 1] = GetGValue(cr1);
            img[index + 2] = GetBValue(cr1);
            img[index + 3] = 255;
            index += 4;
        }
    }
}
double GDIFUNC::coordDistPoint(POINT origin, POINT test)
{
    // Return the decimal distance between two points.
    double xTemp = pow((double)(test.x - origin.x), 2.0);
    double yTemp = pow((double)(test.y - origin.y), 2.0);
    double radius = sqrt(xTemp + yTemp);
    return radius;
}
double GDIFUNC::coordDistPointSum(POINT& origin, vector<POINT>& testList)
{
    // Return the sum total of distances between the origin point and each listed test point.
    double dsum = 0.0;
    for (int ii = 0; ii < testList.size(); ii++)
    {
        dsum += coordDistPoint(origin, testList[ii]);
    }
    return dsum;
}
vector<double> GDIFUNC::coordDistPointSumList(vector<POINT>& originList, vector<POINT>& testList)
{
    // For each point in the origin (candidate) list, determine the sum total of distances between
    // it and the list of test points. The returned list matches the given list of origin points. 
    vector<double> resultList(originList.size());
    for (int ii = 0; ii < resultList.size(); ii++)
    {
        resultList[ii] = coordDistPointSum(originList[ii], testList);
    }
    return resultList;
}
vector<double> GDIFUNC::coordDistPointSumList(vector<POINT>& originList, vector<POINT>& testList, int depth)
{
    // For each point in the origin (candidate) list, determine the sum total of distances between
    // it and the list of test points (going back up to "depth"). The returned list matches the 
    // given list of origin points. 
    vector<double> resultList(originList.size());
    int testSize = testList.size();
    vector<POINT> vpTest;
    vpTest.assign(testList.begin() + testSize - depth, testList.end());
    for (int ii = 0; ii < resultList.size(); ii++)
    {
        resultList[ii] = coordDistPointSum(originList[ii], vpTest);
    }
    return resultList;
}
BITMAPINFOHEADER GDIFUNC::createBitmapHeader(int width, int height)
{
    BITMAPINFOHEADER  bi;

    // create a bitmap
    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = width;
    bi.biHeight = -height;  //this is the line that makes it draw upside down or not
    bi.biPlanes = 1;
    bi.biBitCount = 32;
    bi.biCompression = BI_RGB;
    bi.biSizeImage = 0;
    bi.biXPelsPerMeter = 0;
    bi.biYPelsPerMeter = 0;
    bi.biClrUsed = 0;
    bi.biClrImportant = 0;

    return bi;
}
POINT GDIFUNC::destringifyCoordP(string& sCoord)
{
    POINT p1;
    size_t pos1 = sCoord.find(',');
    if (pos1 > sCoord.size()) { jf.err("No comma found-gdi.destringifyCoordP"); }
    try
    {
        p1.x = std::stoi(sCoord.substr(0, pos1));
        p1.y = stoi(sCoord.substr(pos1 + 1));
    }
    catch (invalid_argument) { jf.err("stoi-gdi.destringifyCoordP"); }
    return p1;
}
HBITMAP GDIFUNC::GdiPlusScreenCapture(HWND hWnd)
{
    // get handles to a device context (DC)
    HDC hwindowDC = GetDC(hWnd);
    HDC hwindowCompatibleDC = CreateCompatibleDC(hwindowDC);
    SetStretchBltMode(hwindowCompatibleDC, COLORONCOLOR);

    // define scale, height and width
    int scale = 1;
    int screenx = GetSystemMetrics(SM_XVIRTUALSCREEN);
    int screeny = GetSystemMetrics(SM_YVIRTUALSCREEN);
    int width = GetSystemMetrics(SM_CXVIRTUALSCREEN);
    int height = GetSystemMetrics(SM_CYVIRTUALSCREEN);

    // create a bitmap
    HBITMAP hbwindow = CreateCompatibleBitmap(hwindowDC, width, height);
    BITMAPINFOHEADER bi = createBitmapHeader(width, height);

    // use the previously created device context with the bitmap
    SelectObject(hwindowCompatibleDC, hbwindow);

    // Starting with 32-bit Windows, GlobalAlloc and LocalAlloc are implemented as wrapper functions that call HeapAlloc using a handle to the process's default heap.
    // Therefore, GlobalAlloc and LocalAlloc have greater overhead than HeapAlloc.
    DWORD dwBmpSize = ((width * bi.biBitCount + 31) / 32) * 4 * height;
    HANDLE hDIB = GlobalAlloc(GHND, dwBmpSize);
    char* lpbitmap = (char*)GlobalLock(hDIB);

    // copy from the window device context to the bitmap device context
    StretchBlt(hwindowCompatibleDC, 0, 0, width, height, hwindowDC, screenx, screeny, width, height, SRCCOPY);   //change SRCCOPY to NOTSRCCOPY for wacky colors !
    GetDIBits(hwindowCompatibleDC, hbwindow, 0, height, lpbitmap, (BITMAPINFO*)&bi, DIB_RGB_COLORS);

    // avoid memory leak
    DeleteDC(hwindowCompatibleDC);
    ReleaseDC(hWnd, hwindowDC);

    return hbwindow;
}
vector<POINT> GDIFUNC::imgVectorPath(POINT p1, double angle, vector<POINT>& TLBR)
{
    // The final path point will be along the TLBR rectangle.
    // NOTE: Image coordinates use a reversed y-axis (positive points down) !
    // Angle is measured from the positive x-axis, and travels CLOCKWISE within [0.0, 360.0)
    //   6       7
    //    \  3  /
    //     \   /
    //  2    +    0      <--- Quadrant diagram. Lines are 90deg apart.
    //     /   \ 
    //    /  1  \
	//   5       4

    if (angle < 0.0 || angle >= 360.0) { jf.err("Invalid angle-mf.initImgVector"); }
    if (p1.x < TLBR[0].x || p1.y < TLBR[0].y) { jf.err("Invalid pStart-mf.initImgVector"); }
    if (p1.x > TLBR[1].x || p1.y > TLBR[1].y) { jf.err("Invalid pStart-mf.initImgVector"); }

    vector<POINT> vpPath = { p1 };
    double ddx, ddy;
    double xCoord = (double)p1.x;
    double yCoord = (double)p1.y;
    int quadrant, idx = 0, idy = 0;

    // Determine this vector's quadrant on an image canvas.
    if (angle == 45.0) { quadrant = 4; }
    else if (angle == 135.0) { quadrant = 5; }
    else if (angle == 225.0) { quadrant = 6; }
    else if (angle == 315.0) { quadrant = 7; }
    else if (angle < 45.0) { quadrant = 0; }
    else if (angle < 135.0) { quadrant = 1; }
    else if (angle < 225.0) { quadrant = 2; }
    else if (angle < 315.0) { quadrant = 3; }
    else { quadrant = 0; }

    // Determine the pixel increments.
    switch (quadrant)
    {
    case 0:
    {
        idx = 1;
        ddy = tan(angle * pi / 180.0);
        break;
    }
    case 1:
    {
        idy = 1;
        ddx = 1.0 / tan(angle * pi / 180.0);
        break;
    }
    case 2:
    {
        idx = -1;
        ddy = -1.0 * tan(angle * pi / 180.0);
        break;
    }
    case 3:
    {
        idy = -1;
        ddx = -1.0 / tan(angle * pi / 180.0);
        break;
    }
    case 4:
    {
        idx = 1;
        idy = 1;
        break;
    }
    case 5:
    {
        idx = -1;
        idy = 1;
        break;
    }
    case 6:
    {
        idx = -1;
        idy = -1;
        break;
    }
    case 7:
    {
        idx = 1;
        idy = -1;
        break;
    }
    }

    // Fly straight and true.
    while (1)
    {
        if (!idx)
        {
            p1.y += idy;
            xCoord += ddx;
            p1.x = int(round(xCoord));
        }
        else if (!idy)
        {
            p1.x += idx;
            yCoord += ddy;
            p1.y = int(round(yCoord));
        }
        else
        {
            p1.x += idx;
            p1.y += idy;
        }
        vpPath.push_back(p1);
        if (p1.x == TLBR[0].x || p1.x == TLBR[1].x) { break; }
        if (p1.y == TLBR[0].y || p1.y == TLBR[1].y) { break; }
    }

    return vpPath;
}
vector<vector<int>> GDIFUNC::minMax(vector<POINT>& vpList)
{
    // Return form [xCoords, yCoords][minIndex, maxIndex]
    vector<vector<int>> vviMinMax(2, vector<int>(2, 0));
    int xMin = vpList[0].x, xMax = vpList[0].x, yMin = vpList[0].y, yMax = vpList[0].y;
    for (int ii = 1; ii < vpList.size(); ii++)
    {
        if (vpList[ii].x < xMin)
        {
            xMin = vpList[ii].x;
            vviMinMax[0][0] = ii;
        }
        else if (vpList[ii].x > xMax)
        {
            xMax = vpList[ii].x;
            vviMinMax[0][1] = ii;
        }
        if (vpList[ii].y < yMin)
        {
            yMin = vpList[ii].y;
            vviMinMax[1][0] = ii;
        }
        else if (vpList[ii].y > yMax)
        {
            yMax = vpList[ii].y;
            vviMinMax[1][1] = ii;
        }
    }
    return vviMinMax;
}
void GDIFUNC::screenshot(std::string& pngPath)
{
    HWND hWindow = GetDesktopWindow();
    HBITMAP hBitmap = GdiPlusScreenCapture(hWindow);
    wstring wPngPath = jf.asciiToUTF16(pngPath);
    CImage img;
    img.Attach(hBitmap);
    img.Save(wPngPath.c_str(), ImageFormatPNG);
}
bool GDIFUNC::saveToMemory(HBITMAP* hbitmap, std::vector<BYTE>& data, std::string dataFormat = "png")
{
    Bitmap bmp(*hbitmap, nullptr);
    // write to IStream
    IStream* istream = nullptr;
    CreateStreamOnHGlobal(NULL, TRUE, &istream);

    // define encoding
    CLSID clsid;
    if (dataFormat.compare("bmp") == 0) { CLSIDFromString(L"{557cf400-1a04-11d3-9a73-0000f81ef32e}", &clsid); }
    else if (dataFormat.compare("jpg") == 0) { CLSIDFromString(L"{557cf401-1a04-11d3-9a73-0000f81ef32e}", &clsid); }
    else if (dataFormat.compare("gif") == 0) { CLSIDFromString(L"{557cf402-1a04-11d3-9a73-0000f81ef32e}", &clsid); }
    else if (dataFormat.compare("tif") == 0) { CLSIDFromString(L"{557cf405-1a04-11d3-9a73-0000f81ef32e}", &clsid); }
    else if (dataFormat.compare("png") == 0) { CLSIDFromString(L"{557cf406-1a04-11d3-9a73-0000f81ef32e}", &clsid); }

    Gdiplus::Status status = bmp.Save(istream, &clsid, NULL);
    if (status != Gdiplus::Status::Ok)
        return false;

    // get memory handle associated with istream
    HGLOBAL hg = NULL;
    GetHGlobalFromStream(istream, &hg);

    // copy IStream to buffer
    int bufsize = GlobalSize(hg);
    data.resize(bufsize);

    // lock & unlock memory
    LPVOID pimage = GlobalLock(hg);
    memcpy(&data[0], pimage, bufsize);
    GlobalUnlock(hg);
    istream->Release();
    return true;
}
string GDIFUNC::stringifyCoord(POINT coord)
{
    string sCoord = to_string(coord.x) + "," + to_string(coord.y);
    return sCoord;
}
