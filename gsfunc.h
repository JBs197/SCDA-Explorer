#pragma once
#include <windows.h>
#include "jfunc.h"

using namespace std;

class GSFUNC
{
	wstring gsPath = L"C:\\Program Files\\gs\\gs9.54.0\\bin\\gswin64c.exe";
	JFUNC jf;

public:
	explicit GSFUNC() {}
	~GSFUNC() {}
	void pdfToPng(string& pdfPath, string& pngPath);

};

