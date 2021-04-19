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
void GSFUNC::pdfDecrypt(string& pathPDF)
{
	wstring wPDF = jf.utf8to16(pathPDF);
	size_t pos1 = wPDF.rfind(L".pdf");
	wstring wPDF2 = wPDF;
	wPDF2.insert(pos1, L"2");
	wstring params = L"-dNOPAUSE -dBATCH -sDEVICE=pdfwrite -sPDFPassword= -dPDFSETTINGS=/prepress ";
	params += L"-sOutputFile=\"" + wPDF2 + L"\" -c .setpdfwrite -f \"" + wPDF + L"\"";
	HINSTANCE openGS = ShellExecuteW(NULL, L"open", gsPath.c_str(), params.c_str(), NULL, SW_SHOWNORMAL);
	int oGS = (int)openGS;
	int bbq = 1;
}
