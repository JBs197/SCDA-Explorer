#include "qjlistview.h"

QModelIndexList QJLISTVIEW::getSelected()
{
    QModelIndexList qmiList = this->selectedIndexes();
    return qmiList;
}
void QJLISTVIEW::highlightItem(int iRow, string sColourBG, string sColourText)
{
    auto qsItem = model->item(iRow, 0);
    qsItem->setData(sColourBG.c_str(), Qt::UserRole);
    qsItem->setData(sColourText.c_str(), Qt::UserRole + 1);
}
void QJLISTVIEW::init()
{
	auto modelRaw = model.get();
	this->setModel(modelRaw);
	this->setItemDelegate(new QJDELEGATE(1, this));

    numBlankRow = 0;
}
void QJLISTVIEW::mousePressEvent(QMouseEvent* event)
{
    int iRow, iRowOld, maxRow;
    if (event->button() == Qt::LeftButton || event->button() == Qt::RightButton) {
        QPoint qPos = event->pos();
        QModelIndex qmIndex = indexAt(qPos);
        if (qmIndex.isValid()) {
            iRow = qmIndex.row();
            maxRow = model->rowCount() - 1 - numBlankRow;
            if (iRow > maxRow) { return; }
            emit itemClicked(iRow);
        }
        QListView::mousePressEvent(event);
    }
}
