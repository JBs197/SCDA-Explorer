#pragma once
#include "sqlfunc.h"
#include "switchboard.h"

using namespace std;

class SCdatabase
{
	JFUNC jf;
	SQLFUNC sf;

	void err(string message);

public:
	SCdatabase() {}
	~SCdatabase() {}

	string configXML;

	void insertCata(SWITCHBOARD& sbgui);
};