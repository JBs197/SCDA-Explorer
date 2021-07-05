#pragma once

#include <cstdio>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <unordered_map>
#include <stdexcept>
#include <locale>
#include <codecvt>
#include <algorithm>
#include <chrono>
#include <mutex>
#include <cmath>

using namespace std;
extern mutex m_err;
extern const string sroot;
extern const string scroot;

class JFUNC
{
	string error_path = sroot + "\\SCDA Error Log.txt";
	string log_path = sroot + "\\SCDA Process Log.txt";
	string navigator_asset_path = sroot + "\\SCDA Navigator Asset.bin";
	vector<vector<string>> navigator_search;  // Form [search tree layer index][section start, section end, inside iteration, ...].

	chrono::high_resolution_clock::time_point t1, t2;

public:
	JFUNC() {}
	~JFUNC() {}

	enum order { Increasing, Decreasing };

	void asciiNearestFit(string& input);
	string asciiOnly(string& input);
	string asciiToUTF8(string input);
	wstring asciiToUTF16(string aFile);
	string bind(string&, vector<string>&);
	int clean(string& bbq, vector<string> dirt);
	int clean(string& bbq, vector<string> dirt, string twins);
	int clean(string& bbq, vector<string> dirt, vector<string> soap);
	vector<vector<string>> compareList(vector<string>& list0, vector<string>& list1);
	vector<vector<string>> compareList(vector<vector<string>>& list0, vector<vector<string>>& list1, vector<int>& activeColumn);
	string decToHex(int& idec);
	string decToHex(unsigned char& ucdec);
	vector<int> destringifyCoord(string& sCoord);
	vector<double> destringifyCoordD(string& sCoord);
	void err(string);
	string get_error_path();
	void isort_ilist(vector<int>& iList, int type);
	void isort_ilist(vector<string>& sList, int type);
	int is_numeric(string&);
	string getExtension(string& spath);
	int getPivot(vector<int>& treeSTrow);
	vector<int> get_roots(vector<vector<int>>&);
	vector<string> ivectorToSvector(vector<int>&);
	vector<string> list_from_marker(string&, char);
	string load(string);
	void log(string message);
	void logTime(string func, long long timer);
	int maxNumCol(vector<vector<wstring>>& task);
	vector<int> minMax(vector<double>& dList);
	string nameFromPath(string& path);
	void navigator(vector<vector<int>>&, vector<string>&, vector<string>&, string&, int);
	void navParser(string& sfile, vector<vector<string>>& search);
	string parent_from_marker(string&, char);
	void pngRead(string& pathPNG);
	void printer(string path, string& sfile);
	void printer(string path, wstring& wfile);
	void printer(string path, vector<unsigned char>& binFile);
	void quicksort(vector<int>&, int, int);
	void removeBlanks(vector<string>& task);
	void removeBlanks(vector<vector<wstring>>& task);
	void set_navigator_asset_path(string&);
	string stringifyCoord(vector<int>& coord);
	string stringifyCoord(vector<unsigned char>& coord);
	vector<int> svectorToIvector(vector<string>&);
	void tclean(string&, char, string);
	vector<string> textParser(string&, vector<string>&);
	void timerStart();
	long long timerRestart();
	long long timerStop();
	string timestamper();
	void toDouble(vector<int>& input, vector<double>& output);
	void toDouble(vector<vector<int>>& input, vector<vector<double>>& output);
	void toInt(vector<double>& input, vector<int>& output);
	void toInt(vector<vector<double>>& input, vector<vector<int>>& output);
	int tree_from_marker(vector<vector<int>>&, vector<string>&);
	string utf16to8(wstring);
	wstring utf8to16(string);
	string utf8ToAscii(string);
	void UTF16clean(wstring&);
	int xDom(double angle);


	// TEMPLATES

};

