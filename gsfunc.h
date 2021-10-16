#pragma once
#include <windows.h>
#include "winfunc.h"

using namespace std;

class GSFUNC
{
	string docFolder;
	wstring exePath;
	JFUNC jf;
	WINFUNC wf;

public:
	explicit GSFUNC() {}
	~GSFUNC() {}
	string binToPng(vector<unsigned char>& binPDF, string sessionID);
	string binToPng(vector<unsigned char>& binPDF, string sessionID, long long& sizePNG);
	void folderConvert(string& dirPDF);
	void init(string& exe, string& doc);
	void pdfToPng(string& pdfPath, string& pngPath);
	void pdfToTxt(string& pdfPath, string& txtPath);
};

