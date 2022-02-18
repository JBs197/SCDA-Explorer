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
#include "SCDAregion.h"

#define NUM_BUF_SLOT 6

class SCDAmap : public QWidget
{
	Q_OBJECT

private:
	std::deque<SCDAregion> dRegion;
	JFILE jfile;
	JPARSE jparse;
	JPIXEL jpixel;
	PNGFUNC pngf;
	std::shared_ptr<QAction> qaDisplayMap, qaDownloadMap, qaInsertMap, qaLoadTree;

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

	void addRegion(SCDAregion& region, int backFront = 1);
	void displayRegion(std::vector<std::vector<unsigned char>> vRGBX);
	void initItemColour(std::string& configXML);
	int insertMapWorker(JBUFFER<std::vector<std::vector<std::string>>, NUM_BUF_SLOT>& jbufSQL, 
		int parentID, std::string parentDir, std::string stmtMap, std::string stmtMapFrame, 
		std::vector<std::string>& vsColTitleMap, std::vector<std::string>& vsColTitleMapFrame);
	void makeBorderMap(std::vector<std::pair<double, double>>& vCoord, std::string filePath);
	void resetRegion();

signals:
	void appendTextIO(std::string message);
	void sendDisplayMap(std::string sYear, std::string sCata, std::string sGeoCode);
	void sendDownloadMap(std::vector<std::string> vsPrompt);
	void sendInsertMap(std::string sYear, std::string sCata);
	void sendLoadGeoTree(std::string sYear, std::string sCata);

public slots:
	void nodeDoubleClicked(const QModelIndex& qmIndex, int indexTree);
	void nodeRightClicked(const QPoint& globalPos, const QModelIndex& qmIndex, int indexTree);
};
