#pragma once
#include <QPainter>
#include <QStyledItemDelegate>
#include "qjlistview.h"
#include "qjtableview.h"

class QJDELEGATE : public QStyledItemDelegate
{
	Q_OBJECT

public:
	int type;
	const int dataRoleBG, dataRoleFG, dataRoleWidget, dataRoleGradient;
	const int dataRoleXYOffset, dataRoleSelected, dataRoleBGSel, dataRoleFGSel;

	QJDELEGATE(int parentType, QObject* parent = nullptr)
		: QStyledItemDelegate(parent), type(parentType), 
	dataRoleBG(Qt::UserRole), dataRoleFG(Qt::UserRole + 1), 
		dataRoleWidget(Qt::UserRole + 2), dataRoleGradient(Qt::UserRole + 3),
		dataRoleXYOffset(Qt::UserRole + 4), dataRoleSelected(Qt::UserRole + 5),
		dataRoleBGSel(Qt::UserRole + 6), dataRoleFGSel(Qt::UserRole + 7) {}

	void enableDisable(const QModelIndex& index) const;
	void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
	void paintBGGradient(QPainter*& painter, QRect cell, QString qsBG, QString qsCoord) const;
	void paintBGSolid(QPainter*& painter, QRect cell, QString qsBG) const;
};

// QJ User Roles:
// 256 = BG colour, in the form #AARRGGBB#RRGGBB or #RRGGBB
// 257 = FG colour, in the form #RRGGBB
// 258 = widget present (negative means disabled), 1->spinbox, 2->checkbox
// 259 = (doubleX, doubleY) pixel rendering offset from standard
// 260 = selected (0 or 1)
// 261 = BG colour(selected), in the form #AARRGGBB#RRGGBB or #RRGGBB
// 262 = FG colour(selected), in the form #RRGGBB
// 263 = unique ID
//

