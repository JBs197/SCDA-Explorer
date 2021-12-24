#pragma once
#include "winfunc.h"
#include <gdiplus.h>
#include <atlstr.h>
#include <atlimage.h>
#include "mathfunc.h"
#pragma comment(lib, "gdiplus.lib")

using namespace Gdiplus;

class GDIFUNC
{
	GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR           gdiplusToken;
	JFUNC jf;
	MATHFUNC mf;
	const double pi = 3.14159265;

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
	double coordDistPoint(POINT origin, POINT test);
	double coordDistPointSum(POINT& origin, vector<POINT>& testList);
	vector<double> coordDistPointSumList(vector<POINT>& originList, vector<POINT>& testList);
	vector<double> coordDistPointSumList(vector<POINT>& originList, vector<POINT>& testList, int depth);
	POINT destringifyCoordP(string& sCoord);
	vector<POINT> imgVectorPath(POINT pStart, double angleDeg, vector<POINT>& boundaryTLBR);
	vector<vector<int>> minMax(vector<POINT>& vpList);
	void screenshot(std::string& pngPath);
	string stringifyCoord(POINT coord);
};

