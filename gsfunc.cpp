#include "gsfunc.h"

string GSFUNC::binToPng(vector<unsigned char>& binPDF, string sessionID)
{
	string pathPDF = docFolder + "/temp/" + sessionID + ".pdf";
	wf.printer(pathPDF, binPDF);
	string pathPNG = docFolder + "/temp/" + sessionID + ".png";
	pdfToPng(pathPDF, pathPNG);
	return pathPNG;
}
string GSFUNC::binToPng(vector<unsigned char>& binPDF, string sessionID, long long& sizePNG)
{
	string pathPDF = docFolder + "/temp/" + sessionID + ".pdf";
	wf.printer(pathPDF, binPDF);
	string pathPNG = docFolder + "/temp/" + sessionID + ".png";
	pdfToPng(pathPDF, pathPNG);
	bool exist = wf.file_exist(pathPNG);
	while (!exist) {
		Sleep(20);
		exist = wf.file_exist(pathPNG);
	}
	sizePNG = wf.getFileSize(pathPNG);
	while (sizePNG < binPDF.size()) {
		Sleep(40);
		sizePNG = wf.getFileSize(pathPNG);
	}
	int count = 1;
	vector<long long> sizeMeasurement = {0, 1, 2, 3, 4, 5};
	while (count > 0)
	{
		sizeMeasurement[count % 6] = wf.getFileSize(pathPNG);
		for (int ii = 0; ii < 5; ii++)
		{
			if (sizeMeasurement[ii] != sizeMeasurement[ii + 1]) { 
				Sleep(20);
				break; 
			}
			else if (ii == 2) { count = -2; }
		}
		count++;
	}
	return pathPNG;
}
void GSFUNC::folderConvert(string& dirPDF)
{
	vector<string> dirt = { "PDF", ".pdf" };
	vector<string> soap = { "PNG", ".png" };
	string dirPNG = dirPDF;
	jstr.clean(dirPNG, dirt, soap);
	wf.makeDir(dirPNG);
	vector<string> listPDF = wf.get_file_list(dirPDF, "*.pdf");
	vector<string> listPNG = listPDF;
	string tempPDF, tempPNG;
	for (int ii = 0; ii < listPNG.size(); ii++)
	{
		jstr.clean(listPNG[ii], dirt, soap);
		tempPDF = dirPDF + "\\" + listPDF[ii];
		tempPNG = dirPNG + "\\" + listPNG[ii];
		pdfToPng(tempPDF, tempPNG);
	}
}
void GSFUNC::init(string& exe, string& doc)
{
	exePath = jf.utf8to16(exe);
	docFolder = doc;
}
void GSFUNC::pdfToPng(string& pdfPath, string& pngPath)
{
	wstring wPDF = jf.utf8to16(pdfPath);
	wstring wPNG = jf.utf8to16(pngPath);
	wstring params = L"-dSAFER -sDEVICE=png16m -dGraphicsAlphaBits=4 "; 
	params += L"-r288 -dMinFeatureSize=2 -o \"" + wPNG + L"\" \"" + wPDF + L"\"";
	HINSTANCE openGS = ShellExecuteW(NULL, L"open", exePath.c_str(), params.c_str(), NULL, SW_SHOWNORMAL);	
	INT_PTR oGS = (INT_PTR)openGS;
	if (oGS <= 32) { jf.err("ShellExecuteW-gs.pdfToPng"); }
}
void GSFUNC::pdfToTxt(string& pdfPath, string& txtPath)
{
	wstring wPDF = jf.asciiToUTF16(pdfPath);
	wstring wTXT = jf.asciiToUTF16(txtPath);
	wstring params = L"-dNOPAUSE -sDEVICE=txtwrite ";
	params += L"-o \"" + wTXT + L"\" \"" + wPDF + L"\"";
	HINSTANCE openGS = ShellExecuteW(NULL, L"open", exePath.c_str(), params.c_str(), NULL, SW_SHOWNORMAL);
	int oGS = (int)openGS;
	int bbq = 1;
}

