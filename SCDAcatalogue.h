#pragma once
#include <QGridLayout>
#include <QLabel>
#include "qjtreeview.h"
#include "SConline.h"

using namespace std;

class SCDAcatalogue : public QWidget
{
	Q_OBJECT

private:
	JFUNC jf;
	shared_ptr<QJTREEMODEL> modelDatabase = nullptr;
	shared_ptr<QJTREEMODEL> modelLocal = nullptr;
	shared_ptr<QJTREEMODEL> modelStatscan = nullptr;
	SConline sco;

	void err(string message);
	void init();

public:
	SCDAcatalogue() { init(); }
	~SCDAcatalogue() {}

	int indexDatabase, indexLocal, indexStatscan;

	QJTREEMODEL* getModel(int indexTree);

public slots:
	void displayOnlineCata();
	void getConfigXML(string configXML);
	void getStatscanURL(string url);
	void resetModel(int indexTree);
};