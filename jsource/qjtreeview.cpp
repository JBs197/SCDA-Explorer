#include "qjtreeview.h"

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
	QModelIndexList qmiList = deselected.indexes();
	int numNode = qmiList.size();
	for (int ii = 0; ii < numNode; ii++) {
		// RESUME HERE
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

