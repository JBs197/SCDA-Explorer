#pragma once
#include <QAction>
#include <QGridLayout>
#include <QLabel>
#include "qjtableview.h"

using namespace std;

class SCDAtable : public QWidget
{
	Q_OBJECT

private:
	JFUNC jf;

	void err(string message);
	void init();
	void initAction();

public:
	SCDAtable() {}
	~SCDAtable() {}

	int indexOnDemand = -1;
	shared_ptr<QStandardItemModel> modelOnDemand = nullptr;

	void initItemColour(QJTABLEVIEW*& qjTable, string& configXML);

public slots:
	void cellRightClicked(const QPoint& globalPos, const QModelIndex& qmIndex, int indexTable);
};