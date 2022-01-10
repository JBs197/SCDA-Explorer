#pragma once
#include "jtree.h"
#include "sqlfunc.h"
#include "switchboard.h"

using namespace std;

class SCdatabase
{
	JFUNC jf;
	SQLFUNC sf;

	void err(string message);
	void log(string message);

public:
	SCdatabase() {}
	~SCdatabase() {}

	string configXML;

	void getCataTree(JTREE& jt);
	void initDatabase(string dbPath) { sf.init(dbPath); }
	void insertCata(SWITCHBOARD& sbgui);
};