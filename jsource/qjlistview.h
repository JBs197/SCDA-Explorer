#pragma once
#include <QListView>
#include <QMouseEvent>
#include <QStandardItemModel>
#include "jfunc.h"
#include "qjdelegate.h"

using namespace std;

class QJLISTVIEW : public QListView
{
	Q_OBJECT

private:
	JFUNC jf;
	shared_ptr<QStandardItemModel>& model;

	void init();

public:
	QJLISTVIEW(shared_ptr<QStandardItemModel>& modelRef, QWidget* parent = nullptr)
		: QListView(parent), model(modelRef) { init(); }
	~QJLISTVIEW() {}

	int numBlankRow;

	void highlightItem(int iRow, string sColourBG, string sColourText);
	QModelIndexList getSelected();

protected:
	virtual void mousePressEvent(QMouseEvent* event) override;

signals:
	void itemClicked(int iRow);
};