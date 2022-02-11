#include "qjtableview.h"

using namespace std;

void QJTABLEVIEW::applyFilter(string filter)
{
    QString qsFilter = QString::fromUtf8(filter.c_str());
    applyFilter(qsFilter);
}
void QJTABLEVIEW::applyFilter(QString qsFilter)
{
    QStandardItemModel* model = (QStandardItemModel*)this->model();
    int numRow = model->rowCount();
    if (qsFilter.size() < 1) {
        for (int ii = 0; ii < numRow; ii++) {
            this->setRowHidden(ii, 0);
        }
        return;
    }

    int pos1;
    QString qsTemp;
    QStringMatcher qsm = QStringMatcher(qsFilter, Qt::CaseInsensitive);
    QStandardItem* qsItem = nullptr;
    for (int ii = 0; ii < numRow; ii++) {
        qsItem = model->item(ii, 1);
        qsTemp = qsItem->text();
        pos1 = qsm.indexIn(qsTemp);
        if (pos1 >= 0) {
            this->setRowHidden(ii, 0);
            continue;
        }
        qsItem = model->item(ii, 2);
        qsTemp = qsItem->text();
        pos1 = qsm.indexIn(qsTemp);
        if (pos1 >= 0) { this->setRowHidden(ii, 0); }
        else { this->setRowHidden(ii, 1); }
    }
}
void QJTABLEVIEW::contextMenuEvent(QContextMenuEvent* event)
{
    QPoint globalPos = event->globalPos();
    QPoint pos = event->pos();
    QModelIndex qmi = indexAt(pos);
    if (qmi.isValid()) {
        emit cellRightClicked(globalPos, qmi, indexTable);
    }
}
void QJTABLEVIEW::deleteRow(int iRow)
{
    QStandardItemModel* model = (QStandardItemModel*)this->model();
    QList<QStandardItem*> qList = model->takeRow(iRow);
    for (int ii = qList.size() - 1; ii >= 0; ii--) {
        if (qList[ii] != nullptr) {
            delete qList[ii];
        }
    }
}
void QJTABLEVIEW::dragEnterEvent(QDragEnterEvent* event)
{
    QPoint qPos, labelPos;
    int action = (int)event->dropAction();
    if (acceptActions == 3 || acceptActions == action) {
        event->accept(); 
        qPos = event->pos();
        labelPos = mapToGlobal(QPoint(0, 0)) + qPos;
        if (this->acceptDrops()) {
            QModelIndex qmIndex = indexAt(qPos);
            if (qmIndex.isValid()) {
                int selBehaviour = (int)this->selectionBehavior();
                switch (selBehaviour) {
                case 0:
                    setCellColour(qmIndex, itemColourHover);
                    break;
                case 1:
                    setRowColour(qmIndex, itemColourHover);
                    break;
                case 2:
                    setColColour(qmIndex, itemColourHover);
                    break;
                }
            }
        }
    }
    else { event->ignore(); }
}
void QJTABLEVIEW::dragLeaveEvent(QDragLeaveEvent* event)
{
    posStart = QPoint();
    if (qmiHover.isValid()) {
        int selBehaviour = (int)this->selectionBehavior();
        switch (selBehaviour) {
        case 0:
            setCellColour(qmiHover, itemColourTemp);
            break;
        case 1:
            setRowColour(qmiHover, itemColourTemp);
            break;
        case 2:
            setColColour(qmiHover, itemColourTemp);
            break;
        }
        qmiHover = QModelIndex();
    }
    QTableView::leaveEvent(event);
}
void QJTABLEVIEW::dragMoveEvent(QDragMoveEvent* event)
{
    QPoint qPos, labelPos;
    vector<int> viOrigin(2), viPoint(2);
    int action = (int)event->dropAction();
    if (acceptActions == 3 || acceptActions == action) {
        event->accept();
        qPos = event->pos();
        labelPos = mapToGlobal(QPoint(0, 0)) + qPos;
        if (this->acceptDrops()) {
            QModelIndex qmIndex = indexAt(qPos);
            if (qmIndex.isValid()) {
                int selBehaviour = (int)this->selectionBehavior();
                switch (selBehaviour) {
                case 0:
                {
                    if (qmiHover != qmIndex) {
                        setCellColour(qmIndex, itemColourHover);                        
                        if (qmiHover.isValid()) {
                            setCellColour(qmiHover, itemColourTemp);
                        }
                        qmiHover = qmIndex;
                    }
                    break;
                }
                case 1:
                {
                    if (qmiHover.row() != qmIndex.row()) {
                        setRowColour(qmIndex, itemColourHover);
                        if (qmiHover.isValid()) {
                            setRowColour(qmiHover, itemColourTemp);
                        }
                        qmiHover = qmIndex;
                    }
                    break;
                }
                case 2:
                {
                    if (qmiHover.column() != qmIndex.column()) {
                        setColColour(qmIndex, itemColourHover);
                        if (qmiHover.isValid()) {
                            setColColour(qmiHover, itemColourTemp);
                        }
                        qmiHover = qmIndex;
                    }
                    break;
                }
                }
            }
        }
    }
    else { event->ignore(); }
}
void QJTABLEVIEW::dropEvent(QDropEvent* event)
{
    // Qt::CopyAction = 1, and Qt::MoveAction = 2.
    int action = (int)event->dropAction();
    if (acceptActions == 3 || acceptActions == action) {
        event->acceptProposedAction();
        QPoint dropPos = event->pos();
        QModelIndex qmIndex = this->indexAt(dropPos);
        if (!qmIndex.isValid()) { return; }

        int selBehaviour = (int)this->selectionBehavior();
        switch (selBehaviour) {
        case 0:
            setCellColour(qmIndex, itemColourTemp);
            break;
        case 1:
            setRowColour(qmIndex, itemColourTemp);
            break;
        case 2:
            setColColour(qmIndex, itemColourTemp);
            break;
        }
        qmiHover = QModelIndex();

        auto mimeData = event->mimeData();
        QString qsMime = mimeData->text();
        QList<QStandardItem*> qRow = mimeTextToQRow(qsMime);
        //
    }
    else { event->ignore(); }
}
void QJTABLEVIEW::err(string message)
{
    string errorMessage = "QJTABLEVIEW error:\n" + message;
    JLOG::getInstance()->err(errorMessage);
}
vector<int> QJTABLEVIEW::getCellDimensions(int iRow, int iCol)
{
    vector<int> widthHeight(2);
    QHeaderView* header = horizontalHeader();
    widthHeight[0] = header->sectionSize(iCol);
    header = verticalHeader();
    widthHeight[1] = header->sectionSize(iRow);
    return widthHeight;
}
int QJTABLEVIEW::getHeight()
{
    QHeaderView* headerV = this->verticalHeader();
    int height = headerV->defaultSectionSize();
    int numRow = headerV->count();
    height *= (numRow + 1);
    return height;
}
QMimeData* QJTABLEVIEW::getRowMimeData(int iRow)
{
    QStandardItemModel* model = (QStandardItemModel*)this->model();
    int numCol = model->columnCount();
    QStandardItem* qsItem = nullptr;
    QString qsData = mimeColHeader;
    for (int ii = 0; ii < numCol; ii++) {
        qsItem = model->item(iRow, ii);
        if (ii > 0) { qsData.append("|"); }
        qsData.append(qsItem->text());
    }
    QMimeData* qmData = new QMimeData();
    qmData->setText(qsData);
    return qmData;
}
QModelIndexList QJTABLEVIEW::getSelected()
{
    QModelIndexList qmiList = this->selectedIndexes();
    return qmiList;
}
void QJTABLEVIEW::init()
{
    indexTable = -1;
    startDragDist = 10;  // Pixels
    posStart = QPoint();  // Null
    qmiHover = QModelIndex();
}
QPixmap QJTABLEVIEW::makeCellPixmap(int iRow, int iCol)
{
    vector<int> cellWidthHeight = getCellDimensions(iRow, iCol);
    QRect rect(0, 0, cellWidthHeight[0], cellWidthHeight[1]);
    QStandardItemModel* model = (QStandardItemModel*)this->model();
    QStandardItem* qsItem = model->item(iRow, iCol);
    QVariant qVar = qsItem->data(Qt::UserRole + 5);
    QString qsTemp = qVar.toString();
    bool success;
    int selected = qsTemp.toInt(&success);
    if (!success) { err("QStringToInt-makeCellPixmap"); }

    QString qsBG, qsFG;
    if (selected > 0) {
        qVar = qsItem->data(Qt::UserRole + 6);
        qsBG = qVar.toString();
        qVar = qsItem->data(Qt::UserRole + 7);
        qsFG = qVar.toString();
    }
    else {
        qVar = qsItem->data(Qt::UserRole);
        qsBG = qVar.toString();
        qVar = qsItem->data(Qt::UserRole + 1);
        qsFG = qVar.toString();
    }

    QColor qcBG(qsBG);
    QBrush cellBrush = QBrush(qcBG);
    QColor qcFG(qsFG);
    QPen cellPen = QPen(qcFG);
    QFontInfo qfi = fontInfo();
    QFont cellFont = qsItem->font();
    qsTemp = qfi.family();
    cellFont.setFamily(qsTemp);
    int pixel = qfi.pixelSize();
    cellFont.setPixelSize(pixel);
    QString cellText = qsItem->text();
    QPixmap qpmCell(cellWidthHeight[0], cellWidthHeight[1]);
    QPainter painter(&qpmCell);
    painter.setBrush(cellBrush);
    painter.setPen(cellPen);
    painter.setFont(cellFont);
    painter.fillRect(rect, cellBrush);
    rect.adjust(3, 0, -3, 0);
    painter.drawText(rect, Qt::AlignLeft | Qt::AlignVCenter, cellText);
    return qpmCell;
}
QList<QStandardItem*> QJTABLEVIEW::mimeTextToQRow(QString& qsMime)
{
    QStandardItemModel* model = (QStandardItemModel*)this->model();
    int numCol = model->columnCount();
    QList<QStandardItem*> qRow;
    qRow.reserve(numCol);
    vector<int> vCellFilled;
    vCellFilled.assign(numCol, 0);

    int indexCol;
    string sColHeader;
    wstring wsTemp;
    QStringList qslData, qslHeader, qslTemp;
    QStandardItem* qsItem = nullptr;
    int numTilde = qsMime.count('~');
    switch (numTilde) {
    case 0:
    {
        qslData = qsMime.split('|');
        for (int ii = 0; ii < qslData.size(); ii++) {
            qsItem = new QStandardItem;
            qsItem->setText(qslData[ii]);
            qRow.append(qsItem);
        }
        break;
    }
    case 1:
    {
        qslTemp = qsMime.split('~');
        qslHeader = qslTemp[0].split('|');
        qslData = qslTemp[1].split('|');
        for (int ii = 0; ii < qslHeader.size(); ii++) {
            wsTemp = qslHeader[ii].toStdWString();
            jstr.utf16To8(sColHeader, wsTemp);
            if (mapColIndex.count(sColHeader)) {
                indexCol = mapColIndex.at(sColHeader);
                qsItem = new QStandardItem;
                qsItem->setEditable(0);
                qsItem->setText(qslData[ii]);
                qRow.insert(indexCol, qsItem);
                vCellFilled[indexCol]++;
            }
        }
        for (int ii = 0; ii < numCol; ii++) {
            if (vCellFilled[ii] < 1) {
                qsItem = new QStandardItem(" ");
                qsItem->setEditable(0);
                qRow.insert(ii, qsItem);
            }
        }
        break;
    }
    }

    return qRow;
}
void QJTABLEVIEW::mouseMoveEvent(QMouseEvent* event)
{
    auto buttonsPressed = event->buttons();
    if (buttonsPressed == Qt::LeftButton && !posStart.isNull()) {
        if ((event->pos() - posStart).manhattanLength() >= startDragDist) { 
            QModelIndex qmIndex = indexAt(posStart);
            if (qmIndex.isValid()) {
                QStandardItemModel* model = (QStandardItemModel*)this->model();
                QStandardItem* qsItem = model->itemFromIndex(qmIndex);
                bool dragEnabled = qsItem->isDragEnabled();
                if (dragEnabled) {
                    startDrag(Qt::CopyAction | Qt::MoveAction);
                }
            }
        }
    }
    QTableView::mouseMoveEvent(event);
}
void QJTABLEVIEW::mousePressEvent(QMouseEvent* event)
{
    int iCol, iRow, maxRow;
    if (event->button() == Qt::LeftButton || event->button() == Qt::RightButton) {
        QPoint qPos = event->pos();
        QModelIndex qmIndex = indexAt(qPos);
        if (qmIndex.isValid()) {
            QStandardItemModel* model = (QStandardItemModel*)this->model();
            iRow = qmIndex.row();
            maxRow = model->rowCount() - 1 - numBlankRow;
            if (iRow > maxRow) { return; }

            posStart = qPos;
            iCol = qmIndex.column();
            emit cellClicked(iRow, iCol);

            if (event->button() == Qt::RightButton) {

            }
            //QItemSelectionModel::SelectionFlags command = selectionCommand(qmIndex, event);
        }
        QTableView::mousePressEvent(event);
    }
}
void QJTABLEVIEW::mouseReleaseEvent(QMouseEvent* event)
{
    posStart = QPoint();
    QTableView::mouseReleaseEvent(event);
}
void QJTABLEVIEW::resetModel(int keepLast)
{
    // Removes all table core data without affecting the headers. 
    // Does not remove the final "keepLast" rows at the bottom !
    QStandardItemModel* model = (QStandardItemModel*)this->model();
    if (model == nullptr) { return; }
    QList<QStandardItem*> list;
    int numRow = model->rowCount();
    for (int ii = numRow - 1 - keepLast; ii >= 0; ii--) {
        list = model->takeRow(ii);
        for (int jj = list.size() - 1; jj >= 0; jj--) {
            delete list[jj];
        }
    }
}
void QJTABLEVIEW::select(int iRow, int iCol)
{
    int selMode = (int)this->selectionMode();
    QItemSelectionModel* selModel = this->selectionModel();
    if (selMode < 2) { selModel->reset(); }
    auto model = this->model();
    QModelIndex qmi = model->index(iRow, iCol);
    selModel->select(qmi, QItemSelectionModel::Select);
}
void QJTABLEVIEW::selectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
    // Update BG/FG displayed colours for selected and deselected items.
    QStandardItemModel* model = (QStandardItemModel*)this->model();
    QVariant qvData = "0";
    QModelIndexList qmiList = deselected.indexes();
    int numItem = qmiList.size();
    for (int ii = 0; ii < numItem; ii++) {
        model->setData(qmiList[ii], qvData, Qt::UserRole + 5);
    }
    qvData = "1";
    qmiList = selected.indexes();
    numItem = qmiList.size();
    for (int ii = 0; ii < numItem; ii++) {
        model->setData(qmiList[ii], qvData, Qt::UserRole + 5);
    }
    update();
}
QItemSelectionModel::SelectionFlags QJTABLEVIEW::selectionCommand(const QModelIndex &index, const QEvent *event) const
{
    return QAbstractItemView::selectionCommand(index, event);
}
void QJTABLEVIEW::setCellColour(QModelIndex qmi, pair<string, string> colour)
{
    QModelIndexList qmiList;
    qmiList.append(qmi);
    setCellColour(qmiList, colour);
}
void QJTABLEVIEW::setCellColour(QModelIndexList qmiList, pair<string, string> colour)
{
    QStandardItemModel* model = (QStandardItemModel*)this->model();
    QVariant qvBG = get<0>(colour).c_str();
    QVariant qvFG = get<1>(colour).c_str();
    int numItem = qmiList.size();
    for (int ii = 0; ii < numItem; ii++) {
        model->setData(qmiList[ii], qvBG, Qt::UserRole);
        model->setData(qmiList[ii], qvFG, Qt::UserRole + 1);
    }
    update();
}
void QJTABLEVIEW::setColColour(QModelIndex qmi, pair<string, string> colour)
{
    QModelIndexList qmiList;
    int iCol = qmi.column();
    QStandardItemModel* model = (QStandardItemModel*)this->model();
    int numRow = model->rowCount();
    for (int ii = 0; ii < numRow; ii++) {
        qmiList.append(model->index(ii, iCol));
    }
    setCellColour(qmiList, colour);
}
void QJTABLEVIEW::setColTitles(vector<vector<string>>& vvsColTitle)
{
    QStandardItemModel* model = (QStandardItemModel*)this->model();
    if (model == nullptr) { return; }
    QHeaderView* headerH = horizontalHeader();
    QStringList qsl;
    if (vvsColTitle.size() < 1) {  // Clear the horizontal header.
        model->setHorizontalHeaderLabels(qsl);
        headerH->setVisible(0);
    }
    else {
        int numCol = (int)vvsColTitle[0].size();
        for (int ii = 0; ii < numCol; ii++) {
            qsl.append(vvsColTitle[0][ii].c_str());
        }
        model->setHorizontalHeaderLabels(qsl);
        headerH->setVisible(1);
    }
}
void QJTABLEVIEW::setModel(QAbstractItemModel* model)
{
    QTableView::setModel(model);
    setItemDelegate(new QJDELEGATE(0, this));
}
void QJTABLEVIEW::setName(int indexTab, int indexRow, int indexCol)
{
    viName = { indexTab, indexRow, indexCol };
}
void QJTABLEVIEW::setRowColour(QModelIndex qmi, pair<string, string> colour)
{
    QModelIndexList qmiList;
    int iRow = qmi.row();
    QStandardItemModel* model = (QStandardItemModel*)this->model();
    int numCol = model->columnCount();
    for (int ii = 0; ii < numCol; ii++) {
        qmiList.append(model->index(iRow, ii));
    }
    setCellColour(qmiList, colour);
}
void QJTABLEVIEW::setTableData(vector<vector<string>>& vvsData)
{
    int numRow = (int)vvsData.size();
    if (numRow < 1) { return; }
    int numCol = 0;
    QStandardItemModel* model = (QStandardItemModel*)this->model();
    if (model == nullptr) { return; }

    QVariant qVar;
    QStandardItem* qsItem = nullptr;
    for (int ii = 0; ii < numRow; ii++) {
        for (int jj = 0; jj < vvsData[ii].size(); jj++) {
            qsItem = new QStandardItem(vvsData[ii][jj].c_str());
            model->setItem(ii, jj, qsItem);
        }
        if (vvsData[ii].size() > numCol) { numCol = vvsData[ii].size(); }
    }
    model->setRowCount(numRow);
    model->setColumnCount(numCol);
}
void QJTABLEVIEW::startDrag(Qt::DropActions supportedActions)
{
    // supportedActions should be Qt::CopyAction or Qt::MoveAction.
    QModelIndex qmIndex = indexAt(posStart);
    if (!qmIndex.isValid()) { return; }
    int iRow = qmIndex.row();
    int iCol = qmIndex.column();
    if (iCol == 0) { iCol = 1; }
    QMimeData* qmData = getRowMimeData(iRow);
    vector<int> cellWidthHeight = getCellDimensions(iRow, iCol);
    QPoint hotspot(cellWidthHeight[0] / 2, cellWidthHeight[1]);
    QPixmap qpmCell = makeCellPixmap(iRow, iCol);
    QDrag* qDrag = new QDrag(this);
    qDrag->setPixmap(qpmCell);
    qDrag->setHotSpot(hotspot);
    qDrag->setMimeData(qmData);
    Qt::DropAction dropAction = qDrag->exec(supportedActions);
    if (dropAction == Qt::MoveAction || dropAction == Qt::CopyAction) {
        posStart = QPoint();
        if (dropAction == Qt::MoveAction) {
            if (recentIndex >= 0) {  // Moved row onto self.
                if (iRow >= recentIndex) { iRow++; }
                recentIndex = -1;
            }
            deleteRow(iRow);
            emit rowRemoved(iRow);
            emit updateCSList(viName[0], viName[1], viName[2]);
        }
        emit focusSearch();
    }
}
void QJTABLEVIEW::updateHeight()
{
    QStandardItemModel* model = (QStandardItemModel*)this->model();
    int numRow = model->rowCount();
    QHeaderView* headerV = this->verticalHeader();
    int rowH = headerV->sectionSize(0);
    int tableH = (numRow + 1) * rowH;
    this->setMaximumHeight(tableH + 5);
    this->setMinimumHeight(tableH + 5);
}
void QJTABLEVIEW::updateModel()
{
    QStandardItemModel* model = (QStandardItemModel*)this->model();
    int numCol = model->columnCount();
    for (int ii = 0; ii < numCol; ii++) {
        this->setColumnHidden(ii, 1);
    }
    for (int ii = 0; ii < viDisplayCol.size(); ii++) {
        this->setColumnHidden(viDisplayCol[ii], 0);
    }
}
void QJTABLEVIEW::updateModel(vector<vector<int>> vviCol)
{
    // vviCol has form [index to show, width][iVal0, iVal1, ...]
    viDisplayCol = vviCol[0];
    auto header = this->horizontalHeader();
    for (int ii = 0; ii < viDisplayCol.size(); ii++) {
        if (vviCol[1][ii] > 0) {
            header->resizeSection(viDisplayCol[ii], vviCol[1][ii]);
            header->setSectionResizeMode(viDisplayCol[ii], QHeaderView::Fixed);
        }
        else {
            header->setSectionResizeMode(viDisplayCol[ii], QHeaderView::Stretch);
        }
    }
    updateModel();
}
