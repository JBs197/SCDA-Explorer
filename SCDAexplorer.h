#pragma once
#include <future>
#include <QFileDialog>
#include <QMainWindow>
#include <QProgressBar>
#include <QScreen>
#include <QTabWidget>
#include "iofunc.h"
#include "qjbusy.h"
#include "qjprogressbar.h"
#include "SCDAcatalogue.h"
#include "SCDAcompare.h"
#include "SCDAcontrol.h"
#include "SCDAstructure.h"
#include "SCDAtable.h"
#include "SCdatabase.h"
#include "SCDAmap.h"
#include "SConline.h"
#include "sqlfunc.h"
#include "switchboard.h"

class SCDA : public QMainWindow
{
	Q_OBJECT

private:
	int commLength, labelCharHeight, labelCharWidth, numCore, sleepTime;
	std::string configXML;
	JFILE jfile;
	JPARSE jparse;
	JTIME jtime;
	std::string sExecFolder, sLocalStorage;
	SWITCHBOARD sb;
	WINFUNC wf;

	enum indexH { Control, Display };
	enum indexTab { Catalogue, Table, Structure, Compare, Map };
	enum indexV { Tab, PBar1, PBar0 };

	void busyWheel(SWITCHBOARD& sb);
	void err(std::string message);
	QList<QRect> getDesktop();
	void initBusy(QJBUSY*& dialogBusy);
	void initConfig();
	void initControl(SCDAcontrol*& control);
	void initGUI();

public:
	SCDA(std::string sExecFolder, QWidget* parent = nullptr);
	~SCDA() = default;

	SCdatabase scdb;
	SConline sco;

	void postRender();

signals:
	void appendTextIO(std::string message);
	void barMessage(std::string message, int pbarIndex = 0);
	void initProgress(std::vector<double> vdProgress, std::vector<std::string> vsProgress, bool soundOn = 0, int pbarIndex = 0);
	void pbarHide(int pbarIndex);
	void pbarShow(int pbarIndex);
	void sendConfigXML(std::string configXML);
	void setTextIO(std::string message);

public slots:
	void busyScreen(bool onOff);
	void debug();
	void deleteTable(std::string tname);
	void deleteTableRow(std::string tnameRow);
	void dialogStructureStart();
	void displayOnlineCata();
	void displayMap(std::string sYear, std::string sCata, std::string sGeoCode);
	void downloadCata(std::string prompt);
	void fetchDBTable(std::string tname);
	void insertCata(std::string prompt);
	void insertMap(std::string sYear, std::string sCata);
	void loadGeoTree(std::string sYear, std::string sCata);
	void scanLocalCata(std::string drive = "");
	void searchDBTable(std::string sQuery);
	void testMap(std::string url);
	void updateCataDB();
};