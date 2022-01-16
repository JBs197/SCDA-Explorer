#pragma once
#include <QApplication>
#include <QFileDialog>
#include <QMainWindow>
#include <QProgressBar>
#include <QScreen>
#include <QTabWidget>
#include "qjbusy.h"
#include "qjprogressbar.h"
#include "SCDAcatalogue.h"
#include "SCDAcontrol.h"
#include "SCDAstructure.h"
#include "SCDAtable.h"
#include "SCdatabase.h"
#include "SConline.h"
#include "sqlfunc.h"
#include "switchboard.h"

using namespace std;
extern mutex m_bar;

class SCDA : public QMainWindow
{
	Q_OBJECT

private:
	int commLength, indexControl, indexDisplay, indexPBar;
	int indexTab, labelCharHeight, labelCharWidth, sleepTime;
	string configXML;
	JFUNC jf;
	string sExecFolder, sLocalStorage;
	SWITCHBOARD sb;
	WINFUNC wf;

	enum indexTab { Catalogue, Table, Structure };

	void busyWheel(SWITCHBOARD& sb, vector<vector<int>> comm);
	void err(string message);
	QRect getDesktop();
	void initBusy(QJBUSY*& dialogBusy);
	void initConfig();
	void initControl(SCDAcontrol*& control);
	void initDatabase();
	void initGUI();
	void initStatscan();

public:
	SCDA(string execFolder, QWidget* parent = nullptr);
	~SCDA() {}

	SCdatabase scdb;
	SConline sco;

	void postRender();

signals:
	void appendTextIO(string message);
	void barMessage(string message);
	void initProgress(vector<double> vdProgress, vector<string> vsProgress);
	void sendConfigXML(string configXML);
	void setTextIO(string message);

public slots:
	void debug();
	void deleteTable(string tname);
	void dialogStructureStart();
	void displayOnlineCata();
	void downloadCata(string prompt);
	void driveSelected(string drive);
	void fetchDBTable(string tname);
	void insertCata(string prompt);
	void searchDBTable(string sQuery);
	void updateCataDB();
};