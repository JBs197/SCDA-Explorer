#pragma once
#include <QGridLayout>
#include <QLabel>
#include "qjtreeview.h"

class SCDAstructure : public QWidget
{
	Q_OBJECT

private:
	JSTRING jstr;
	shared_ptr<QAction> qaSaveForWhom, qaSaveGeoCode, qaSaveGeoLayer;
	shared_ptr<QAction> qaSaveGeoRegionName, qaSaveParameterTitle;
	shared_ptr<QAction> qaSaveMemberNumber, qaSaveMemberDescription;

	void err(string message);
	void init();
	void initAction();
	void saveSetting(string sObjectName, string sData);

public:
	SCDAstructure() {
		itemColourDefault = make_pair("#FFFFFF", "#000000");
		itemColourSelected = make_pair("#000080", "#FFFFFF");
		init();
	}
	~SCDAstructure() {}

	enum index{ Tree, Settings };

	pair<string, string> itemColourDefault, itemColourFail;
	pair<string, string> itemColourSelected, itemColourWarning;
	shared_ptr<QJTREEMODEL> modelStructure = nullptr;

	void loadXML(string xmlPath);

public slots:
	void nodeRightClicked(const QPoint& globalPos, const QModelIndex& qmIndex, int indexTree);
};