#include "qjdelegate.h"

using namespace std;

QJDELEGATE::QJDELEGATE(int parentType, QList<QColor>& qlColour, QObject* parent)
    : QStyledItemDelegate(parent), type(parentType),
    dataRoleBG(Qt::UserRole), dataRoleFG(Qt::UserRole + 1),
    dataRoleWidget(Qt::UserRole + 2), dataRoleGradient(Qt::UserRole + 3),
    dataRoleXYOffset(Qt::UserRole + 4), dataRoleSelected(Qt::UserRole + 5),
    dataRoleBGSel(Qt::UserRole + 6), dataRoleFGSel(Qt::UserRole + 7), bgDefault(qlColour[0]), 
    fgDefault(qlColour[1]), bgSelected(qlColour[2]), fgSelected(qlColour[3]) {}
QJDELEGATE::QJDELEGATE(int parentType, QObject* parent) 
    : QStyledItemDelegate(parent), type(parentType),
dataRoleBG(Qt::UserRole), dataRoleFG(Qt::UserRole + 1),
dataRoleWidget(Qt::UserRole + 2), dataRoleGradient(Qt::UserRole + 3),
dataRoleXYOffset(Qt::UserRole + 4), dataRoleSelected(Qt::UserRole + 5),
dataRoleBGSel(Qt::UserRole + 6), dataRoleFGSel(Qt::UserRole + 7), bgDefault(QColor("#FFFFFF")),
fgDefault(QColor("#000000")), bgSelected(QColor("#000080")), fgSelected(QColor("#FFFFFF")) {}

void QJDELEGATE::enableDisable(const QModelIndex& index) const
{
    // Examines the given dataRole for a nonzero number - indicates a widget is present.
    // Positive value indicates the widget should be enabled, negative is disable.
    QVariant qvWidget = index.data(dataRoleWidget);
    QString qsWidget = qvWidget.toString();
    bool success;
    int iWidget = qsWidget.toInt(&success);
    if (success) {
        QTableView* table = nullptr;
        QListView* list = nullptr;
        QWidget* qwCell = nullptr;
        switch (type) {
        case 0:
            table = (QTableView*)this->parent();
            qwCell = table->indexWidget(index);
            break;
        case 1:
            list = (QListView*)this->parent();
            qwCell = list->indexWidget(index);
            break;
        }
        if (iWidget > 0) { qwCell->setEnabled(1); }
        else if (iWidget < 0) { qwCell->setEnabled(0); }
    }
}
void QJDELEGATE::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    bool success;
    QString qsTemp;
    QRect cell = option.rect;
    QVariant qvOffset = index.data(dataRoleXYOffset);
    QString qsOffset = qvOffset.toString();
    int numComma = qsOffset.count(',');
    if (numComma == 1) {
        int posComma = qsOffset.indexOf(',');
        qsTemp = qsOffset.left(posComma);
        int offsetX = qsTemp.toInt(&success);
        if (!success) { return; }
        qsTemp = qsOffset.mid(posComma + 1);
        int offsetY = qsTemp.toInt(&success);
        if (!success) { return; }
        cell.moveLeft(cell.x() + offsetX);
        cell.moveTop(cell.y() + offsetY);
    }

    int inum;
    bool selected = 0;
    QVariant qVar = index.data(dataRoleSelected);
    if (!qVar.isNull()) {
        qsTemp = qVar.toString();
        inum = qsTemp.toInt(&success);
        if (success && inum > 0) { selected = 1; }
    }
    
    QVariant qvCoord = index.data(dataRoleGradient);
    QString qsCoord = qvCoord.toString();

    QString qsBG, qsFG;
    QVariant qvBG, qvFG;
    if (selected) { qvBG = index.data(dataRoleBGSel); }
    else { qvBG = index.data(dataRoleBG); }

    if (qsCoord.size() > 0) { 
        qsBG = qvBG.toString();  // Form #RRGGBB or #AARRGGBB#RRGGBB (background over primer).
        paintBGGradient(painter, cell, qsBG, qsCoord);
    }
    else { 
        if (qvBG.isValid()) {
            qsBG = qvBG.toString();  // Form #RRGGBB or #AARRGGBB#RRGGBB (background over primer).
            QColor qcBG(qsBG);
            paintBGSolid(painter, cell, qcBG);
        }
        else if (selected && bgSelected.isValid()) {
            paintBGSolid(painter, cell, bgSelected);
        }
        else if (!selected && bgDefault.isValid()) {
            paintBGSolid(painter, cell, bgDefault);
        }
    }

    if (selected) { qvFG = index.data(dataRoleFGSel); }
    else { qvFG = index.data(dataRoleFG); }
   
    QPen pen;
    if (qvFG.isValid()) {
        qsFG = qvFG.toString();  // Form #RRGGBB.
        QColor qcFG(qsFG);
        pen.setColor(qcFG);
    }
    else if (selected && fgSelected.isValid()) { pen.setColor(fgSelected); }
    else if (!selected && fgDefault.isValid()) { pen.setColor(fgDefault); }

    QVariant qvText = index.data(Qt::DisplayRole);
    QString qsText = qvText.toString();
    int x = cell.x();
    cell.setLeft(x + 2);
    QFont font = option.font;
    painter->save();
    painter->setPen(pen);
    painter->setFont(font);
    painter->drawText(cell, Qt::AlignLeft | Qt::AlignVCenter, qsText);
    painter->restore();

    enableDisable(index);
}
void QJDELEGATE::paintBGGradient(QPainter*& painter, QRect cell, QString qsBG, QString qsCoord) const
{
    // qsCoord has form N@doubleA,doubleB,doubleC,doubleD@doubleP@doubleQ@...
    // Linear: doubleA-D are the start/stop points for the gradient [0.0, 1.0]. 
    // Radial: doubleAB are coords for center, doubleC is radius [0.0, anything]. 
    // Conical: doubleAB are coords for center, doubleC is angle for the start/stop fold [0.0, 360.0].
    // doublePQ... are relative positions within the start/stop path [0.0, 1.0].
    QString qsTemp(1, qsCoord[0]);
    bool success;
    int gradientType = qsTemp.toInt(&success);  // 0 = Linear, 1 = radial, 2 = conical
    if (!success) { return; }
    qsCoord.remove(0, 1);

    double x = (double)cell.x();
    double y = (double)cell.y();
    double w = (double)cell.width();
    double h = (double)cell.height();
    double dr, dx, dy;

    QStringList qslColour = qsBG.split('#', Qt::SkipEmptyParts);
    QStringList qslCoord = qsCoord.split('@', Qt::SkipEmptyParts);
    QStringList qslStartStop = qslCoord[0].split(',');
    vector<double> vdStartStop;
    if (gradientType == 0) {
        for (int ii = 0; ii < 2; ii++) {
            dx = qslStartStop[ii * 2].toDouble(&success);
            if (!success) { return; }
            dx *= w;
            dx += x;
            vdStartStop.push_back(dx);
            dy = qslStartStop[(ii * 2) + 1].toDouble(&success);
            if (!success) { return; }
            dy *= h;
            dy += y;
            vdStartStop.push_back(dy);
        }
    }
    else {
        dx = qslStartStop[0].toDouble(&success);
        if (!success) { return; }
        dx *= w;
        dx += x;
        vdStartStop.push_back(dx);
        dy = qslStartStop[1].toDouble(&success);
        if (!success) { return; }
        dy *= h;
        dy += y;
        vdStartStop.push_back(dy);
        dr = qslStartStop[2].toDouble(&success);
        if (!success) { return; }
        vdStartStop.push_back(dr);
    }
    qslCoord.removeAt(0);

    vector<double> vdPos(qslCoord.size());
    for (int ii = 0; ii < qslCoord.size(); ii++) {
        vdPos[ii] = qslCoord[ii].toDouble(&success);
        if (!success) { return; }
    }

    QList<QColor> qcList;
    QColor qcTemp;
    for (int ii = 0; ii < qslColour.size(); ii++) {
        qslColour[ii].insert(0, '#');
        qcTemp = QColor(qslColour[ii]);
        qcList.append(qcTemp);
    }

    if (gradientType == 0) {
        QLinearGradient qlGrad(vdStartStop[0], vdStartStop[1], vdStartStop[2], vdStartStop[3]);
        for (int ii = 0; ii < vdPos.size(); ii++) {
            qlGrad.setColorAt(vdPos[ii], qcList[ii]);
        }

        if (qcList.size() > vdPos.size()) {
            QBrush qbPrimer = QBrush(qcList.last(), Qt::SolidPattern);
            painter->fillRect(cell, qbPrimer);
        }

        QBrush qbGrad(qlGrad);
        painter->fillRect(cell, qbGrad);
    }
    else if (gradientType == 1) {
        QRadialGradient qrGrad(vdStartStop[0], vdStartStop[1], vdStartStop[2]);
        for (int ii = 0; ii < vdPos.size(); ii++) {
            qrGrad.setColorAt(vdPos[ii], qcList[ii]);
        }

        if (qcList.size() > vdPos.size()) {
            QBrush qbPrimer = QBrush(qcList.last(), Qt::SolidPattern);
            painter->fillRect(cell, qbPrimer);
        }

        QBrush qbGrad(qrGrad);
        painter->fillRect(cell, qbGrad);
    }
    else if (gradientType == 2) {
        QConicalGradient qcGrad(vdStartStop[0], vdStartStop[1], vdStartStop[2]);
        for (int ii = 0; ii < vdPos.size(); ii++) {
            qcGrad.setColorAt(vdPos[ii], qcList[ii]);
        }

        if (qcList.size() > vdPos.size()) {
            QBrush qbPrimer = QBrush(qcList.last(), Qt::SolidPattern);
            painter->fillRect(cell, qbPrimer);
        }

        QBrush qbGrad(qcGrad);
        painter->fillRect(cell, qbGrad);
    }
}
void QJDELEGATE::paintBGSolid(QPainter*& painter, QRect cell, const QColor& qcBG) const
{
    /*
    int numColour = qsBG.count('#');
    if (numColour == 0) { qsBG = "#FFFFFF"; }
    else if (numColour == 2) {  // Two colours: translucent background over opaque primer.
        int posColour = qsBG.indexOf('#', 1);
        int lenPrimer = qsBG.size() - posColour;
        QString qsPrimer = qsBG.right(lenPrimer);
        qsBG.chop(lenPrimer);
        QColor qcPrimer(qsPrimer);
        QBrush qbPrimer = QBrush(qcPrimer, Qt::SolidPattern);
        painter->fillRect(cell, qbPrimer);
    }
    QColor qcBG(qsBG);
    */

    QBrush qbBG = QBrush(qcBG, Qt::SolidPattern);
    painter->fillRect(cell, qbBG);
}
