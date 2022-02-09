#pragma once
#include <QGridLayout>
#include <QLabel>
#include "jfile.h"
#include "jpixel.h"
#include "pngfunc.h"
#include "qjpaint.h"
#include "qjtreeview.h"

class SCDAmap : public QWidget
{
	Q_OBJECT

private:
	JFILE jfile;
	JPIXEL jpixel;
	JSTRING jstr;
	PNGFUNC pngf;
	std::shared_ptr<QAction> qaLoadTree;

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
	std::shared_ptr<QJTREEMODEL> modelCataMap = nullptr;

signals:
	void sendLoadGeoTree(std::string sYear, std::string sCata);

public slots:
	void nodeRightClicked(const QPoint& globalPos, const QModelIndex& qmIndex, int indexTree);
};
