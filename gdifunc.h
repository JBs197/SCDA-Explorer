#pragma once
#include <windows.h>
#include <gdiplus.h>
#include <atlstr.h>
#include <atlimage.h>
#include "jfunc.h"
#pragma comment(lib, "gdiplus.lib")

using namespace Gdiplus;

class GDIFUNC
{
	GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR           gdiplusToken;
	JFUNC jf;

	BITMAPINFOHEADER createBitmapHeader(int width, int height);
	HBITMAP GdiPlusScreenCapture(HWND hWnd);
	bool saveToMemory(HBITMAP* hbitmap, std::vector<BYTE>& data, std::string dataFormat );

public:
	GDIFUNC() 
	{
		GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
	}
	~GDIFUNC() 
	{
		GdiplusShutdown(gdiplusToken);
	}

	void capture(vector<unsigned char>& img, vector<int>& imgSpec, vector<POINT> TLBR);
	POINT destringifyCoordP(string& sCoord);
	vector<vector<int>> minMax(vector<POINT>& vpList);
	void screenshot(std::string& pngPath);
	string stringifyCoord(POINT coord);
};

