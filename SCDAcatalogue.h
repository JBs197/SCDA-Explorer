#pragma once
#include <QGridLayout>
#include <QLabel>
#include "qjtreeview.h"
#include "SConline.h"
#include "switchboard.h"

using namespace std;

class SCDAcatalogue : public QWidget
{
	Q_OBJECT

private:
	pair<string, string> itemColourDefault, itemColourFail, itemColourSelected;  // Solid colours (background, foreground)
	JFUNC jf;
	SConline sco;

	void err(string message);
	void init();
	void initItemColour(string& configXML);

public:
	SCDAcatalogue() { init(); }
	~SCDAcatalogue() {}

	int indexDatabase, indexLocal, indexStatscan;
	shared_ptr<QJTREEMODEL> modelDatabase = nullptr;
	shared_ptr<QJTREEMODEL> modelLocal = nullptr;
	shared_ptr<QJTREEMODEL> modelStatscan = nullptr;

	void displayOnlineCata();
	QJTREEMODEL* getModel(int indexTree);

signals:
	void busyStop();

public slots:
	void getConfigXML(string configXML);
	void getStatscanURL(string url);
	void nodeClicked(const QModelIndex& qmIndex, int indexTree);
	void resetModel(int indexTree);
};