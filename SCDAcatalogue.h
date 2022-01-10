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
	QAction* qaDownload, *qaInsert;
	WINFUNC wf;

	void err(string message);
	void init();
	void initAction();
	void insertCata();

public:
	SCDAcatalogue(string& configXML) { 
		initItemColour(configXML);
		init(); 
	}
	SCDAcatalogue() { 
		itemColourDefault = make_pair("#FFFFFF", "#000000");
		itemColourSelected = make_pair("#000080", "#FFFFFF");
		init(); 
	}
	~SCDAcatalogue() {}

	int indexDatabase, indexLocal, indexStatscan;
	pair<string, string> itemColourDefault, itemColourFail;  // Solid colours (background, foreground)
	pair<string, string> itemColourSelected, itemColourWarning;
	shared_ptr<QJTREEMODEL> modelDatabase = nullptr;
	shared_ptr<QJTREEMODEL> modelLocal = nullptr;
	shared_ptr<QJTREEMODEL> modelStatscan = nullptr;

	void displayOnlineCata(SWITCHBOARD& sbgui, SCDAcatalogue*& cata, SConline& sco);
	void downloadCata();
	QJTREEMODEL* getModel(int indexTree);
	void initItemColour(string& configXML);
	void scanLocal(SWITCHBOARD& sbgui, SCDAcatalogue*& cata, string& configXML);

signals:
	void sendDownloadCata(string prompt);
	void sendInsertCata(string prompt);

public slots:
	void getConfigXML(string configXML);
	void nodeClicked(const QModelIndex& qmIndex, int indexTree);
	void nodeRightClicked(const QPoint& globalPos, const QModelIndex& qmIndex, int indexTree);
	void resetModel(int indexTree);
};