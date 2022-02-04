#pragma once
#include <QComboBox>
#include <QDialog>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include "jfile.h"
#include "jparse.h"
#include "jsort.h"
#include "qjtreeview.h"
#include "SConline.h"

class SCDAcatalogue : public QWidget
{
	Q_OBJECT

private:
	JFILE jfile;
	JPARSE jparse;
	JSORT jsort;
	QAction* qaDownload, *qaInsert, *qaSearch, *qaTemplate;
	WINFUNC wf;

	void err(std::string message);
	void init();
	void initAction();
	void insertCata();
	void setGeoTreeTemplate();

public:
	SCDAcatalogue(std::string& configXML) {
		initItemColour(configXML);
		init(); 
	}
	SCDAcatalogue() {
		itemColourDefault = std::make_pair("#FFFFFF", "#000000");
		itemColourSelected = std::make_pair("#000080", "#FFFFFF");
		init(); 
	}
	~SCDAcatalogue() = default;

	enum index { Statscan, Local, Database };

	// Solid colours (background, foreground)
	std::pair<std::string, std::string> itemColourDefault, itemColourFail;
	std::pair<std::string, std::string> itemColourSelected, itemColourWarning;
	std::shared_ptr<QJTREEMODEL> modelDatabase = nullptr;
	std::shared_ptr<QJTREEMODEL> modelLocal = nullptr;
	std::shared_ptr<QJTREEMODEL> modelStatscan = nullptr;

	void displayOnlineCata(SWITCHBOARD& sbgui, SCDAcatalogue*& cata, SConline& sco);
	void downloadCata();
	QJTREEMODEL* getModel(int indexTree);
	void initItemColour(std::string& configXML);
	void scanLocal(SWITCHBOARD& sbgui, SCDAcatalogue*& cata, std::string& configXML);
	void searchCata();

signals:
	void sendDownloadCata(std::string prompt);
	void sendInsertCata(std::string prompt);
	void sendSearchCata(std::string prompt);
	void setTextIO(std::string message);

public slots:
	void getConfigXML(std::string configXML);
	void nodeClicked(const QModelIndex& qmIndex, int indexTree);
	void nodeRightClicked(const QPoint& globalPos, const QModelIndex& qmIndex, int indexTree);
	void resetModel(int indexTree);
};