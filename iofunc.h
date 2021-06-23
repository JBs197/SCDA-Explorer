#pragma once

#include "winfunc.h"
#include <QPlainTextEdit>
#include <QCoreApplication>

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
	bool askYesNo(QPlainTextEdit*& box, string question);
	string copyText();
	vector<double> getBinPosition(QPlainTextEdit*& box, string regionName, string sParent);
	POINT getCoord(int vKey);
	QString getQSCoord(POINT& p1);
	string getSCoord(POINT& p1);
	HWND getWindow(int vKey);
	void kbHoldPress(int holdKey, int pressKey);
	void kbInput(int vKey);
	void kbInput(char ch);
	void kbInput(string str);
	void mouseClick(POINT p1);
	void mouseClickKey(int vKey, POINT p1);
	void mouseClickTriple(POINT p1);
};

