#pragma once
#include <windows.h>
#include "winfunc.h"

using namespace std;

class GSFUNC
{
	wstring gsPath = L"C:\\Program Files\\gs\\gs9.54.0\\bin\\gswin64c.exe";
	JFUNC jf;
	WINFUNC wf;

public:
	explicit GSFUNC() {}
	~GSFUNC() {}
	void folderConvert(string& dirPDF);
	void pdfToPng(string& pdfPath, string& pngPath);
	void pdfToTxt(string& pdfPath, string& txtPath);
};

