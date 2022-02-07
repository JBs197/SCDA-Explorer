#include "qjtreeview.h"

using namespace std;

void QJTREEVIEW::contextMenuEvent(QContextMenuEvent* event)
{
	QPoint globalPos = event->globalPos();
	QPoint pos = event->pos();
	QModelIndex qmi = indexAt(pos);
	if (qmi.isValid()) {
		emit nodeRightClicked(globalPos, qmi, indexTree);
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
	connect(this, &QJTREEVIEW::expanded, this, &QJTREEVIEW::nodeExpanded);
}
void QJTREEVIEW::mouseDoubleClickEvent(QMouseEvent* event)
{
	QPoint pos = event->pos();
	QModelIndex qmi = indexAt(pos);
	if (qmi.isValid()) {
		emit nodeDoubleClicked(qmi, indexTree);
	}
}
void QJTREEVIEW::nodeExpanded(const QModelIndex& qmiNode)
{
	QJTREEMODEL* model = (QJTREEMODEL*)this->model();
	int numCol = model->columnCount(qmiNode);
	for (int ii = 0; ii < numCol; ii++) {
		resizeColumnToContents(ii);
	}
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
}
void QJTREEVIEW::relayClicked(const QModelIndex& qmIndex)
{
	emit nodeClicked(qmIndex, indexTree);
}
QList<QModelIndex> QJTREEVIEW::selectedIndexes() const
{
	return QTreeView::selectedIndexes();
}
QList<QJTREEITEM*> QJTREEVIEW::selectedNodes() const
{
	QList<QJTREEITEM*> qjtiList;
	QJTREEMODEL* model = (QJTREEMODEL*)this->model();
	if (model == nullptr) { return qjtiList; }
	auto selectionBehaviour = selectionBehavior();
	QList<QModelIndex> qmiList = QTreeView::selectedIndexes();
	int numNode = qmiList.size();
	QJTREEITEM* qjti = nullptr;
	switch (selectionBehaviour) {
	case SelectionBehavior::SelectItems:
	{
		for (int ii = 0; ii < numNode; ii++) {
			qjti = model->getNode(qmiList[ii]);
			if (qjti != nullptr) { qjtiList.append(qjti); }
		}
		break;
	}
	case SelectionBehavior::SelectRows:
	{
		unordered_set<int> setRow;
		int iRow;
		for (int ii = 0; ii < numNode; ii++) {
			qjti = model->getNode(qmiList[ii]);
			iRow = qmiList[ii].row();
			if (qjti != nullptr && !setRow.count(iRow)) { 
				setRow.emplace(iRow);
				qjtiList.append(qjti); 
			}
		}
		break;
	}
	}

	return qjtiList;
}
void QJTREEVIEW::setModel(QJTREEMODEL* qjtm)
{
	QHeaderView* headerH = this->header();
	headerH->setVisible(qjtm->headerTitles);

	QTreeView::setModel(qjtm);

	this->setItemDelegate(new QJDELEGATE(2, this));
	QItemSelectionModel* selModel = this->selectionModel();
	connect(selModel, &QItemSelectionModel::selectionChanged, this, &QJTREEVIEW::nodeSelectionChanged);
}

