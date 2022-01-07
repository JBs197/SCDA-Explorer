#pragma once
#include <QApplication>
#include <QMainWindow>
#include <QProgressBar>
#include <QScreen>
#include <QTabWidget>
#include "qjbusy.h"
#include "qjprogressbar.h"
#include "SCDAcatalogue.h"
#include "SCDAcontrol.h"
#include "SConline.h"
#include "sqlfunc.h"
#include "switchboard.h"

using namespace std;
extern mutex m_bar;

class SCDA : public QMainWindow
{
	Q_OBJECT

private:
	int commLength, indexCata, indexControl, indexDisplay, indexMap, indexPBar;
	int indexTab, indexTable, labelCharHeight, labelCharWidth, sleepTime;
	string configXML;
	JFUNC jf;
	string sExecFolder;
	SQLFUNC sf;
	SWITCHBOARD sb;
	SConline sco;
	WINFUNC wf;

	void busyWheel(SWITCHBOARD& sb, vector<vector<int>> comm);
	void err(string message);
	QRect getDesktop();
	void initBusy(QJBUSY*& dialogBusy);
	void initConfig();
	void initControl(SCDAcontrol*& control);
	void initDatabase();
	void initGUI();
	void initStatscan();
	void scanCataLocal(SWITCHBOARD& sbgui, JTREE& jtgui);

public:
	SCDA(string execFolder, QWidget* parent = nullptr);
	~SCDA() {}

	void postRender();

signals:
	void barMessage(string message);
	void initProgress(vector<double> vdProgress, vector<string> vsProgress);
	void sendConfigXML(string configXML);

public slots:
	void debug();
	void displayOnlineCata();
	void downloadCata(string prompt);
	void driveSelected(string drive);
	void updateCataDB();
};