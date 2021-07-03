#include "stdafx.h"
#include "gsfunc.h"

void GSFUNC::folderConvert(string& dirPDF)
{
	vector<string> dirt = { "PDF", ".pdf" };
	vector<string> soap = { "PNG", ".png" };
	string dirPNG = dirPDF;
	jf.clean(dirPNG, dirt, soap);
	wf.makeDir(dirPNG);
	vector<string> listPDF = wf.get_file_list(dirPDF, "*.pdf");
	vector<string> listPNG = listPDF;
	string tempPDF, tempPNG;
	for (int ii = 0; ii < listPNG.size(); ii++)
	{
		jf.clean(listPNG[ii], dirt, soap);
		tempPDF = dirPDF + "\\" + listPDF[ii];
		tempPNG = dirPNG + "\\" + listPNG[ii];
		pdfToPng(tempPDF, tempPNG);
	}
}
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
void GSFUNC::pdfToTxt(string& pdfPath, string& txtPath)
{
	wstring wPDF = jf.asciiToUTF16(pdfPath);
	wstring wTXT = jf.asciiToUTF16(txtPath);
	wstring params = L"-dNOPAUSE -sDEVICE=txtwrite ";
	params += L"-o \"" + wTXT + L"\" \"" + wPDF + L"\"";
	HINSTANCE openGS = ShellExecuteW(NULL, L"open", gsPath.c_str(), params.c_str(), NULL, SW_SHOWNORMAL);
	int oGS = (int)openGS;
	int bbq = 1;
}

