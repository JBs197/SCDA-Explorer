#pragma once
#include <QListView>
#include <QPainter>
#include <QStyledItemDelegate>
#include <QTableView>

class QJDELEGATE : public QStyledItemDelegate
{
	Q_OBJECT

private:
	const QColor bgDefault, bgSelected, fgDefault, fgSelected;

public:
	QJDELEGATE(int parentType, QList<QColor>& qlColour, QObject* parent = nullptr);
	QJDELEGATE(int parentType, QObject* parent = nullptr);
	~QJDELEGATE() = default;

	int type;
	const int dataRoleBG, dataRoleFG, dataRoleWidget, dataRoleGradient;
	const int dataRoleXYOffset, dataRoleSelected, dataRoleBGSel, dataRoleFGSel;

	void enableDisable(const QModelIndex& index) const;
	void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
	void paintBGGradient(QPainter*& painter, QRect cell, QString qsBG, QString qsCoord) const;
	void paintBGSolid(QPainter*& painter, QRect cell, const QColor& qcBG) const;
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

