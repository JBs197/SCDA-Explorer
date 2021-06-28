#pragma once

#include "switchboard.h"
#include "winfunc.h"
#include "zipfunc.h"

using namespace std;

class STATSCAN
{
	string cataName, cataPath, cataYear, description, metaFile, topic;
	JFUNC jf;
	string nl;
	int nls;
	WINFUNC wf;
	ZIPFUNC zf;

public:
	STATSCAN() {}
	~STATSCAN() {}

	void init(string cataPath);
	vector<string> makeCreateInsertDefinitions();
	vector<vector<string>> makeCreateInsertDIM();
	void trimMID(string& MID);

};