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
	JFUNC jf;
	QAction* qaDelete;

	void err(string message);
	void init();
	void initAction();

public:
	SCDAtable(string& configXML) {
		initItemColour(configXML);
		init();
	}
	SCDAtable() { 
		itemColourDefault = make_pair("#FFFFFF", "#000000");
		itemColourSelected = make_pair("#000080", "#FFFFFF");
		init(); 
	}
	~SCDAtable() {}

	enum index { OnDemand, Search };

	// Solid colours (background, foreground)
	pair<string, string> itemColourDefault, itemColourFail, itemColourHover;  
	pair<string, string> itemColourSelected, itemColourWarning;

	shared_ptr<QStandardItemModel> modelOnDemand = nullptr;
	shared_ptr<QJTREEMODEL> modelSearch = nullptr;

	void displayTable(vector<vector<string>>& vvsData, vector<vector<string>>& vvsColTitle, string title);
	QStandardItemModel* getModelTable(int index);
	QJTREEMODEL* getModelTree(int index);
	void initItemColour(string& configXML);

signals:
	void fetchTable(string tname);
	void sendDeleteTable(string tname);

public slots:
	void cellRightClicked(const QPoint& globalPos, const QModelIndex& qmiCell, int indexTable);
	void deleteTable();
	void nodeDoubleClicked(const QModelIndex& qmiNode, int indexTree);
	void nodeRightClicked(const QPoint& globalPos, const QModelIndex& qmIndex, int indexTree);
	void resetModel(int indexModel);
};