#pragma once
#include <QApplication>
#include <QMainWindow>
#include <QProgressBar>
#include <QTabWidget>
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
	int commLength, indexCata, indexControl, indexDisplay; 
	int indexMap, indexTable, labelCharHeight, labelCharWidth, sleepTime;
	string configXML;
	JFUNC jf;
	string sExecFolder;
	SQLFUNC sf;
	SWITCHBOARD sb;
	WINFUNC wf;

	void barMessage(string message);
	void err(string message);
	void initConfig();
	void initControl(SCDAcontrol*& control);
	void initDatabase();
	void initGUI();
	void scanCataLocal(SWITCHBOARD& sbgui, JTREE& jtgui);

public:
	SCDA(string execFolder, QWidget* parent = nullptr);
	~SCDA() {}

	void postRender();

signals:
	void sendConfigXML(string configXML);

public slots:
	void driveSelected(string drive);
	void updateCataDB();
};