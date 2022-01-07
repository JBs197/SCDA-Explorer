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
	pair<string, string> itemColourDefault, itemColourFail, itemColourSelected;  // Solid colours (background, foreground)
	JFUNC jf;
	QAction* qaDownload;

	void err(string message);
	void init();
	void initAction();
	void initItemColour(string& configXML);

public:
	SCDAcatalogue() { init(); }
	~SCDAcatalogue() {}

	int indexDatabase, indexLocal, indexStatscan;
	shared_ptr<QJTREEMODEL> modelDatabase = nullptr;
	shared_ptr<QJTREEMODEL> modelLocal = nullptr;
	shared_ptr<QJTREEMODEL> modelStatscan = nullptr;

	void displayOnlineCata(SWITCHBOARD& sbgui, SCDAcatalogue*& cata, SConline& sco);
	void downloadCata();
	QJTREEMODEL* getModel(int indexTree);

signals:
	void sendDownloadCata(string prompt);

public slots:
	void getConfigXML(string configXML);
	void nodeClicked(const QModelIndex& qmIndex, int indexTree);
	void nodeRightClicked(const QPoint& globalPos, const QModelIndex& qmIndex, int indexTree);
	void resetModel(int indexTree);
};