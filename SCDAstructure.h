#pragma once
#include <QGridLayout>
#include <QLabel>
#include "qjtreeview.h"

class SCDAstructure : public QWidget
{
	Q_OBJECT

private:
	JSTRING jstr;
	std::shared_ptr<QAction> qaSaveForWhom, qaSaveGeoCode, qaSaveGeoLayer;
	std::shared_ptr<QAction> qaSaveGeoRegionName, qaSaveParameterTitle;
	std::shared_ptr<QAction> qaSaveMemberNumber, qaSaveMemberDescription;

	void err(std::string message);
	void init();
	void initAction();
	void saveSetting(std::string sObjectName, std::string sData);

public:
	SCDAstructure() {
		itemColourDefault = std::make_pair("#FFFFFF", "#000000");
		itemColourSelected = std::make_pair("#000080", "#FFFFFF");
		init();
	}
	~SCDAstructure() = default;

	enum index{ Tree, Settings };

	std::pair<std::string, std::string> itemColourDefault, itemColourFail;
	std::pair<std::string, std::string> itemColourSelected, itemColourWarning;
	std::shared_ptr<QJTREEMODEL> modelStructure = nullptr;

	void loadXML(std::string xmlPath);

public slots:
	void nodeRightClicked(const QPoint& globalPos, const QModelIndex& qmIndex, int indexTree);
};