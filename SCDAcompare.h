#pragma once
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTableWidget>
#include "pngfunc.h"
#include "sconline.h"
#include "switchboard.h"

class SCDAcompare : public QWidget
{
	Q_OBJECT

private:
	JFILE jfile;
	JSTRING jstr;
	QList<QColor> listColour;
	PNGFUNC pngf;

	void doRemoveCol(int iCol);
	void doShift(int iRow, int iCol, int direction);
	void err(std::string message);
	void initColour();

public:
	SCDAcompare(QWidget* parent = nullptr);
	~SCDAcompare() = default;

	enum index { Text, Button, Table };
	enum indexText { Marker = 1, Title = 4, Input = 7, Add = 8 };

	void testScalePos(SWITCHBOARD& sbgui, SConline& sco);

signals:
	void sendDownloadMap(std::string url);

public slots:
	void addText();
	void downloadColPage();
	void shiftDown();
	void shiftUp();
	void removeCol();
	void updateComparison();
};
