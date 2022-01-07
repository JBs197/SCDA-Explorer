#include "qjtreeview.h"

void QJTREEVIEW::contextMenuEvent(QContextMenuEvent* event)
{
	QPoint globalPos = event->globalPos();
	QPoint pos = event->pos();
	QModelIndex qmi = indexAt(pos);
	if (qmi.isValid()) {
		emit nodeRightClicked(pos, qmi, indexTree);
	}
}
void QJTREEVIEW::err(string message)
{
	string errorMessage = "QJTREE error:\n" + message;
	JLOG::getInstance()->err(errorMessage);
}
void QJTREEVIEW::init()
{
	indexTree = -1;
	connect(this, &QJTREEVIEW::clicked, this, &QJTREEVIEW::relayClicked);
}
void QJTREEVIEW::nodeSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
	QJTREEMODEL* qjtm = (QJTREEMODEL*)this->model();

	QVariant qvData = "0";
	QModelIndexList qmiList = deselected.indexes();
	int numNode = qmiList.size();
	for (int ii = 0; ii < numNode; ii++) {
		qjtm->setData(qmiList[ii], qvData, Qt::UserRole + 5);
	}
	qvData = "1";
	qmiList = selected.indexes();
	numNode = qmiList.size();
	for (int ii = 0; ii < numNode; ii++) {
		qjtm->setData(qmiList[ii], qvData, Qt::UserRole + 5);
	}

	int bbq = 1;
}
void QJTREEVIEW::relayClicked(const QModelIndex& qmIndex)
{
	emit nodeClicked(qmIndex, indexTree);
}
void QJTREEVIEW::setModel(QJTREEMODEL* qjtm)
{
	this->setItemDelegate(new QJDELEGATE(2, this));

	QHeaderView* headerH = this->header();
	headerH->setVisible(qjtm->headerTitles);

	QTreeView::setModel(qjtm);

	QItemSelectionModel* selModel = this->selectionModel();
	connect(selModel, &QItemSelectionModel::selectionChanged, this, &QJTREEVIEW::nodeSelectionChanged);
}

