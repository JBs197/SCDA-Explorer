#pragma once
#define WINDOWS

#ifdef WINDOWS
#include <direct.h>
#define GetCurrentDir _getcwd
#else
#include <unistd.h>
#define GetCurrentDir getcwd
#endif

#include <cstdio>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <array>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <stdexcept>
#include <locale>
#include <codecvt>
#include <algorithm>
#include <chrono>
#include <mutex>
#include <cmath>

using namespace std;
extern mutex m_err;

class JFUNC
{
	int defaultDecimalPlaces = 2;  // For percentages.
	string error_path, execFolder, log_path;
	chrono::high_resolution_clock::time_point t1, t2;

public:
	JFUNC() { init(); }
	~JFUNC() {}

	enum order { Increasing, Decreasing };

	void asciiNearestFit(string& input);
	string asciiOnly(string& input);
	wstring asciiToUTF16(string aFile);
	vector<unsigned char> baseConvert(int inputWidth, vector<unsigned char>& input, int x, int y);
	string bind(string&, vector<string>&);
	bool checkPercent(string& sNum);
	bool checkPercent(string& sNum, double tolerance);
	bool checkPercent(vector<string>& list);
	bool checkPercent(vector<string>& list, double tolerance);
	int clean(string& bbq, vector<string> dirt);
	int clean(string& bbq, vector<string> dirt, string twins);
	vector<vector<string>> compareList(vector<string>& list0, vector<string>& list1);
	vector<vector<string>> compareList(vector<vector<string>>& list0, vector<vector<string>>& list1, vector<int>& activeColumn);
	int countChar(string& bbq, char target);
	string decToHex(int idec);
	string decToHex(unsigned char ucdec);
	vector<int> destringifyCoord(string& sCoord);
	vector<double> destringifyCoordD(string& sCoord);
	string doubleToCommaString(double dNum);
	string doubleToCommaString(double dNum, int decimalPlaces);
	vector<string> doubleToCommaString(vector<double> vdNum, int decimalPlaces);
	void err(string);
	vector<string> horizontalCentering(vector<string> vsList);
	string get_error_path();
	string getExecFolder() { return execFolder; }
	string getExtension(string& spath);
	int getPivot(vector<int>& treeSTrow);
	vector<int> get_roots(vector<vector<int>>&);
	void hexDec(string& sHex, int& iDec);
	void hexToDec(string sHex, int& iDec);
	void init();
	string intToCommaString(long long iNum);
	void isort_ilist(vector<int>& iList, int type);
	void isort_ilist(vector<string>& sList, int type);
	int is_numeric(string&);
	vector<string> ivectorToSvector(vector<int>&);
	vector<string> list_from_marker(string&, char);
	string load(string);
	vector<unsigned char> loadBin(string filePath);
	void log(string message);
	void logTime(string func, long long timer);
	vector<unsigned> makeCRC32(vector<int> vPolyPower);
	int maxNumCol(vector<vector<wstring>>& task);
	vector<int> minMax(vector<double>& dList);
	vector<int> minMax(vector<double>* dList);
	vector<int> minMax(vector<int>& iList);
	void mutexDelay(mutex& m_delay);
	string nameFromPath(string& path);
	void navParser(string& sfile, vector<vector<string>>& search);
	string numericToCommaString(string sNumeric);
	string numericToCommaString(string sNumeric, int mode);
	string parent_from_marker(string&, char);
	void printer(string path, string& sfile);
	void printer(string path, wstring& wfile);
	void printer(string path, vector<unsigned char>& binFile);
	void quicksort(vector<int>&, int, int);
	void removeBlanks(vector<string>& task);
	void removeBlanks(vector<vector<string>>& task);
	void removeBlanks(vector<vector<wstring>>& task);
	void removeBlanks(vector<vector<vector<string>>>& task);
	vector<double> rgbxToDouble(vector<unsigned char>& vRGBX);
	vector<double> rgbxToDouble(vector<int>& vRGBX);
	void setErrorPath(string sEP) { error_path = sEP; }
	void setLogPath(string sLP) { log_path = sLP; }
	void sleep(int ms);
	void sortAlphabetically(vector<string>& vsList);
	void sortAlphabetically(vector<vector<string>>& vvsList, int iCol);
	void sortAlphabetically(vector<vector<string>>& vvsList, vector<int> viSort);
	vector<string> splitByMarker(string& text, char marker);
	vector<vector<string>> splitByMarker(vector<string>& vsText, char marker);
	string stringifyCoord(vector<int>& coord);
	string stringifyCoord(vector<unsigned char>& coord);
	vector<int> svectorToIvector(vector<string>&);
	void tclean(string&, char, string);
	bool testInt(string sNum);
	vector<string> textParser(string&, vector<string>&);
	void timerStart();
	long long timerReport();
	long long timerRestart();
	long long timerStop();
	string timestamper();
	void toDouble(vector<int>& input, vector<double>& output);
	void toDouble(vector<vector<int>>& input, vector<vector<double>>& output);
	void toInt(vector<double>& input, vector<int>& output);
	void toInt(vector<vector<double>>& input, vector<vector<int>>& output);
	int tree_from_marker(vector<vector<int>>&, vector<string>&);
	void uptick(vector<int>& viCounter, vector<int> viMax);
	string utf16to8(wstring);
	wstring utf8to16(string);
	string utf8ToAscii(string);
	void UTF16clean(wstring&);
	int xDom(double angle);


	// TEMPLATES
	template<typename S> void sortByColumn(vector<vector<S>>& vvSList, int iCol, char posNeg)
	{
		if (posNeg != '+' && posNeg != '-') { err("Invalid posNeg-jfunc.sortByColumn"); }
		int count = 1;
		vector<S> vSTemp;
		while (count > 0) {
			count = 0;
			if (posNeg == '+') {
				for (int ii = 0; ii < vvSList.size() - 1; ii++) {
					if (vvSList[ii + 1] < vvSList[ii]) {
						vSTemp = vvSList[ii];
						vvSList[ii] = vvSList[ii + 1];
						vvSList[ii + 1] = vSTemp;
						count++;
					}
				}
			}
			else {
				for (int ii = 0; ii < vvSList.size() - 1; ii++) {
					if (vvSList[ii + 1] > vvSList[ii]) {
						vSTemp = vvSList[ii];
						vvSList[ii] = vvSList[ii + 1];
						vvSList[ii + 1] = vSTemp;
						count++;
					}
				}
			}
		}
	}
	template<typename S> void sortLinkedList(vector<int>& viList, vector<S>& vSList, int mode)
	{
		int count = 1, iTemp;
		S STemp;
		switch (mode)
		{
		case 0:  // Ascending
		{
			while (count > 0)
			{
				count = 0;
				for (int ii = 0; ii < viList.size() - 1; ii++)
				{
					if (viList[ii + 1] < viList[ii])
					{
						iTemp = viList[ii];
						STemp = vSList[ii];
						viList[ii] = viList[ii + 1];
						vSList[ii] = vSList[ii + 1];
						viList[ii + 1] = iTemp;
						vSList[ii + 1] = STemp;
						count++;
					}
				}
			}
			break;
		}
		case 1: // Descending
		{
			while (count > 0)
			{
				count = 0;
				for (int ii = 0; ii < viList.size() - 1; ii++)
				{
					if (viList[ii + 1] > viList[ii])
					{
						iTemp = viList[ii];
						STemp = vSList[ii];
						viList[ii] = viList[ii + 1];
						vSList[ii] = vSList[ii + 1];
						viList[ii + 1] = iTemp;
						vSList[ii + 1] = STemp;
						count++;
					}
				}
			}
			break;
		}
		}
	}
};

