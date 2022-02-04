#pragma once
#include <QAction>
#include <QContextMenuEvent>
#include <QHeaderView>
#include <QMenu>
#include <QTreeView>
#include "jlog.h"
#include "qjdelegate.h"
#include "qjtreemodel.h"

class QJTREEVIEW : public QTreeView
{
	Q_OBJECT

private:
	void err(std::string message);
	void init();

public:
	QJTREEVIEW(QWidget* parent = nullptr) : QTreeView(parent) { init(); }
	~QJTREEVIEW() = default;

	int indexTree;

	void setModel(QJTREEMODEL* qjtm);

signals:
	void nodeClicked(const QModelIndex& qmIndex, int indexTree);
	void nodeDeselected(const QModelIndex& qmIndex);
	void nodeDoubleClicked(const QModelIndex& qmIndex, int indexTree);
	void nodeRightClicked(const QPoint& globalPos, const QModelIndex& qmIndex, int indexTree);
	void nodeSelected(const QModelIndex& qmIndex);

private slots:
	void nodeExpanded(const QModelIndex& qmiNode);
	void relayClicked(const QModelIndex& qmIndex);

public slots:
	void nodeSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);

protected:
	void contextMenuEvent(QContextMenuEvent* event) override;
	void mouseDoubleClickEvent(QMouseEvent* event) override;
};