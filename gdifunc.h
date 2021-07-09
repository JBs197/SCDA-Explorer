#pragma once

#include "jfunc.h"

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
	void screenshot(std::string& pngPath);

};

