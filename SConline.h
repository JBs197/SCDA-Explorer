#pragma once
#include "jfunc.h"
#include "jtree.h"
#include "switchboard.h"
#include "winfunc.h"

using namespace std;

class SConline
{
	JFUNC jf;
	JTREE jt;
	long long maxFileSize = -1;
	vector<vector<string>> vvsCata;  // Form [index][year, cata0, cata1, ...]
	WINFUNC wf;

	void downloadTopic(string filePath);
	void err(string message);
	string urlCataDownload(string sYear, string sCata);
	string urlCataTopic(string sYear, string sCata);
	string urlYear(string sYear);

public:
	SConline() {}
	~SConline() {}

	string configXML, urlRoot;

	void downloadCata(SWITCHBOARD& sbgui);
	void getCataTree(JTREE& jt);
	vector<string> getListCata(string sYear);
	vector<string> getListYear();
};