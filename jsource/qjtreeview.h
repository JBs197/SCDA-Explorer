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

	void setModel(QJTREEMODEL* qjtm);

signals:
	void nodeClicked(const QModelIndex& qmIndex, int indexTree);
	void nodeDeselected(const QModelIndex& qmIndex);
	void nodeDoubleClicked(const QModelIndex& qmIndex, int indexTree);
	void nodeRightClicked(const QPoint& globalPos, const QModelIndex& qmIndex, int indexTree);
	void nodeSelected(const QModelIndex& qmIndex);

private slots:
	void relayClicked(const QModelIndex& qmIndex);

public slots:
	void nodeSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);

protected:
	void contextMenuEvent(QContextMenuEvent* event) override;
	void mouseDoubleClickEvent(QMouseEvent* event) override;
};