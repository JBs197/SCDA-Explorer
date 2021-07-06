#pragma once

#include "winfunc.h"
#include <QPlainTextEdit>
#include <QCoreApplication>
#include <QDebug>

class IOFUNC : private WINFUNC
{
	HWND activeWindow = NULL;
	vector<int> defaultRes = { 3840, 1080 };
	vector<POINT> gpsWebpage;
	JFUNC jf;
	int reuse = 0;  // 1 = yes, 0 = no, -1 = no and stop asking

public:
	explicit IOFUNC() {}
	~IOFUNC() {}

	bool askYesNo(string question);
	string copyText(HWND& targetWindow);
	vector<double> getBinPosition(string regionName, string sParent);
	POINT getCoord(int vKey);
	QString getQSCoord(POINT& p1);
	string getSCoord(POINT& p1);
	vector<POINT> getUserTLBR(int vKey1, int vKey2);
	HWND getWindow(int vKey);
	void kbHoldPress(WORD holdKey, WORD pressKey, HWND& targetWindow);
	void kbInput(WORD vKey);
	void kbInput(WORD vKey, HWND& targetWindow);
	void kbInput(string str);
	void kbInput(string str, HWND& targetWindow);
	void mouseClick(POINT p1);
	void mouseClickKey(int vKey, POINT p1);
	void mouseClickTriple(POINT p1);
	void mouseClickTriple(POINT p1, HWND& targetWindow);
};

