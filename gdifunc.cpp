#include "stdafx.h"
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
        p1.x = stoi(sCoord.substr(0, pos1));
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
