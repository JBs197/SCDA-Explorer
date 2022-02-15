#pragma once
#include <QApplication>
#include <QGridLayout>
#include <QLabel>
#include "jfile.h"
#include "jpixel.h"
#include "pngfunc.h"
#include "qjpaint.h"
#include "qjtreeview.h"
#include "SCautomation.h"

class SCDAmap : public QWidget
{
	Q_OBJECT

private:
	JFILE jfile;
	JPARSE jparse;
	JPIXEL jpixel;
	PNGFUNC pngf;
	std::shared_ptr<QAction> qaDownloadMap, qaInsertMap, qaLoadTree;

	void err(std::string message);
	void init();
	void initAction();
	void shadeToBorder(std::string filePath, std::vector<unsigned char> rgbxBG);

public:
	SCDAmap() 
	{ 
		itemColourDefault = std::make_pair("#FFFFFF", "#000000");
		itemColourSelected = std::make_pair("#000080", "#FFFFFF");
		init(); 
	}
	~SCDAmap() = default;
	enum index { Tree, Map };

	std::pair<std::string, std::string> itemColourDefault, itemColourSelected;
	std::pair<std::string, std::string> itemColourFail, itemColourWarning;
	std::shared_ptr<QJTREEMODEL> modelCataMap = nullptr;

	void initItemColour(std::string& configXML);
	void makeBorderMap(std::vector<std::pair<int, int>>& vOutline, std::string filePath);

signals:
	void appendTextIO(std::string message);
	void sendDownloadMap(std::vector<std::string> vsPrompt);
	void sendInsertMap(std::string sYear, std::string sCata);
	void sendLoadGeoTree(std::string sYear, std::string sCata);

public slots:
	void nodeRightClicked(const QPoint& globalPos, const QModelIndex& qmIndex, int indexTree);
};
