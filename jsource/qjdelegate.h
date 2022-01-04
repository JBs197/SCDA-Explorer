#pragma once
#include <QPainter>
#include <QStyledItemDelegate>
#include "qjlistview.h"
#include "qjtableview.h"

using namespace std;

class QJDELEGATE : public QStyledItemDelegate
{
	Q_OBJECT

public:
	int type;
	const int dataRoleBG = Qt::UserRole;
	const int dataRoleFG = Qt::UserRole + 1;
	const int dataRoleWidget = Qt::UserRole + 2;
	const int dataRoleGradient = Qt::UserRole + 3;
	const int dataRoleXYOffset = Qt::UserRole + 4;
	const int dataRoleSelected = Qt::UserRole + 5;
	const int dataRoleBGSel = Qt::UserRole + 6;
	const int dataRoleFGSel = Qt::UserRole + 7;

	QJDELEGATE(int parentType, QObject* parent = nullptr)
		: QStyledItemDelegate(parent), type(parentType) {}

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
//

