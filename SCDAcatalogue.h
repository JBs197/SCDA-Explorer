#pragma once
#include <QHBoxLayout>
#include <QLabel>
#include "qjtree.h"

using namespace std;

class SCDAcatalogue : public QWidget
{
	Q_OBJECT

private:
	JFUNC jf;

	void err(string message);
	void init();

public:
	SCDAcatalogue() { init(); }
	~SCDAcatalogue() {}

	int indexDatabase, indexLocal, indexStatscan;

};