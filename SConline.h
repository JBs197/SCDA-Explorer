#pragma once
#include "jfunc.h"
#include "jtree.h"
#include "winfunc.h"

using namespace std;

class SConline
{
	JFUNC jf;
	JTREE jt;
	WINFUNC wf;

	void err(string message);
	string urlYear(string syear);

public:
	SConline() {}
	~SConline() {}

	string configXML, urlRoot;

	void getCataTree(JTREE& jt);
	vector<string> getListCata(string sYear);
	vector<string> getListYear();
};