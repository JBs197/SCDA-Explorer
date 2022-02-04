#pragma once
#include <QAction>
#include <QGridLayout>
#include <QLabel>
#include "qjtableview.h"
#include "qjtreeview.h"

class SCDAtable : public QWidget
{
	Q_OBJECT

private:
	JPARSE jparse;
	QAction* qaDeleteRow, *qaDeleteTable;

	void err(std::string message);
	void init();
	void initAction();

public:
	SCDAtable(std::string& configXML) {
		initItemColour(configXML);
		init();
	}
	SCDAtable() { 
		itemColourDefault = std::make_pair("#FFFFFF", "#000000");
		itemColourSelected = std::make_pair("#000080", "#FFFFFF");
		init(); 
	}
	~SCDAtable() {}

	enum index { OnDemand, Search };

	// Solid colours (background, foreground)
	std::pair<std::string, std::string> itemColourDefault, itemColourFail, itemColourHover;
	std::pair<std::string, std::string> itemColourSelected, itemColourWarning;

	std::shared_ptr<QStandardItemModel> modelOnDemand = nullptr;
	std::shared_ptr<QJTREEMODEL> modelSearch = nullptr;

	void displayTable(std::vector<std::vector<std::string>>& vvsData, std::vector<std::vector<std::string>>& vvsColTitle, std::string title);
	QStandardItemModel* getModelTable(int index);
	QJTREEMODEL* getModelTree(int index);
	void initItemColour(std::string& configXML);

signals:
	void fetchTable(std::string tname);
	void sendDeleteRow(std::string tnameRow);
	void sendDeleteTable(std::string tname);

public slots:
	void cellRightClicked(const QPoint& globalPos, const QModelIndex& qmiCell);
	void deleteRow();
	void deleteTable();
	void nodeDoubleClicked(const QModelIndex& qmiNode, int indexTree);
	void nodeRightClicked(const QPoint& globalPos, const QModelIndex& qmIndex);
	void resetModel(int indexModel);
};