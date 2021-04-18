#include "gsfunc.h"

void GSFUNC::pdfToPng(string& pdfPath, string& pngPath)
{
	wstring wPDF = jf.utf8to16(pdfPath);
	wstring wPNG = jf.utf8to16(pngPath);
	wstring params = L"-dSAFER -sDEVICE=png16m -dGraphicsAlphaBits=4 "; 
	params += L"-r288 -dMinFeatureSize=2 -o \"" + wPNG + L"\" \"" + wPDF + L"\"";
	HINSTANCE openGS = ShellExecuteW(NULL, L"open", gsPath.c_str(), params.c_str(), NULL, SW_SHOWNORMAL);	
	int oGS = (int)openGS;
	int bbq = 1;
}
