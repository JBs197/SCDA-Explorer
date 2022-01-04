#pragma once
#include <QHeaderView>
#include <QTreeView>
#include "jfunc.h"
#include "qjdelegate.h"
#include "qjtreemodel.h"

using namespace std;

class QJTREEVIEW : public QTreeView
{
	Q_OBJECT

private:
	JFUNC jf;

	void err(string message);
	void init();

public:
	QJTREEVIEW(QWidget* parent = nullptr) : QTreeView(parent) { init(); }
	~QJTREEVIEW() {}

	int indexTree;
	QMap<int, QVariant> itemColourDefault;
	QMap<int, QVariant> itemColourFail;
	QMap<int, QVariant> itemColourSelected;

	void setModel(QJTREEMODEL* qjtm);

signals:
	void nodeClicked(const QModelIndex& qmIndex, int indexTree);
	void nodeDeselected(const QModelIndex& qmIndex);
	void nodeSelected(const QModelIndex& qmIndex);

private slots:
	void relayClicked(const QModelIndex& qmIndex);

public slots:
	void nodeSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);
};