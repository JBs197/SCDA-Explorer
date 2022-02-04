#pragma once
#include <QApplication>
#include <QFileDialog>
#include <QMainWindow>
#include <QMessageBox>
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
	enum indexTab { Catalogue, Table, Structure, Compare };
	enum indexV { Tab, PBar };

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
	void barMessage(std::string message);
	void initProgress(std::vector<double> vdProgress, std::vector<std::string> vsProgress);
	void sendConfigXML(std::string configXML);
	void setTextIO(std::string message);

public slots:
	void debug();
	void deleteTable(std::string tname);
	void deleteTableRow(std::string tnameRow);
	void dialogStructureStart();
	void displayOnlineCata();
	void downloadCata(std::string prompt);
	void fetchDBTable(std::string tname);
	void insertCata(std::string prompt);
	void scanLocalCata(std::string drive = "");
	void searchDBTable(std::string sQuery);
	void testMap(std::string url);
	void updateCataDB();
};