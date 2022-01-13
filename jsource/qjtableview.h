#pragma once
#include <QApplication>
#include <QDate>
#include <QDebug>
#include <QDrag>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QFlags>
#include <QHeaderView>
#include <QItemSelectionModel>
#include <QLabel>
#include <QMenu>
#include <QMimeData>
#include <QMouseEvent>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QTableView>
#include "jfunc.h"
#include "qjdelegate.h"

using namespace std;

class QJTABLEVIEW : public QTableView
{
	Q_OBJECT

private:
	int acceptActions = 0;
	JFUNC jf;
	unordered_map<string, int> mapColIndex;  // Column header -> column index
	QString mimeColHeader;
	int numBlankRow;
	QPoint posStart;
	QModelIndex qmiHover;
	int recentIndex = -1, startDragDist;
	vector<int> viDisplayCol, viName;

	void err(string message);
	void init();
	void updateHeight();

public:
	QJTABLEVIEW(QWidget* parent = nullptr) 
		: QTableView(parent) { init(); }
	~QJTABLEVIEW() {}

	int indexChecksum, indexTable;
	pair<string, string> itemColourDefault, itemColourFail, itemColourHover;
	pair<string, string> itemColourSelected, itemColourTemp, itemColourWarning;

	void applyFilter(string filter);
	void applyFilter(QString qsFilter);
	void deleteRow(int iRow);
	vector<int> getCellDimensions(int iRow, int iCol);
	int getHeight();
	vector<int> getName() { return viName; }
	QMimeData* getRowMimeData(int iRow);
	QModelIndexList getSelected();
	void setCellColour(QModelIndex qmi, pair<string, string> colour);
	void setCellColour(QModelIndexList qmiList, pair<string, string> colour);
	void setColColour(QModelIndex qmi, pair<string, string> colour);
	void setRowColour(QModelIndex qmi, pair<string, string> colour);
	QPixmap makeCellPixmap(int iRow, int iCol);
	QList<QStandardItem*> mimeTextToQRow(QString& qsMime);
	void resetModel(int keepLast);
	void select(int iRow, int iCol);
	void setColTitles(vector<vector<string>>& vvsColTitles);
	void setModel(QAbstractItemModel* model) override;
	void setName(int indexTab, int indexRow, int indexCol);
	void setTableData(vector<vector<string>>& vvsData);
	void updateModel();
	void updateModel(vector<vector<int>> vviCol);

signals:
	void cellClicked(int iRow, int iCol);
	void cellRightClicked(const QPoint& globalPos, const QModelIndex& qmIndex, int indexTable);
	void colourFreq(int iRow);
	void focusSearch();
	void rowAdded(int iRow);
	void rowRemoved(int iRow);
	void updateCSList(int indexTab, int indexRow, int indexCol);
	void updateFreq(unsigned checksum, int deltaCount);

public slots:
	void selectionChanged(const QItemSelection& selected, const QItemSelection& deselected);

protected:
	void contextMenuEvent(QContextMenuEvent* event) override;
	virtual void dragEnterEvent(QDragEnterEvent* event) override;
	virtual void dragLeaveEvent(QDragLeaveEvent* event) override;
	virtual void dragMoveEvent(QDragMoveEvent* event) override;
	virtual void dropEvent(QDropEvent* event) override;
	virtual void mouseMoveEvent(QMouseEvent* event) override;
	virtual void mousePressEvent(QMouseEvent* event) override;
	virtual void mouseReleaseEvent(QMouseEvent* event) override;
	virtual QItemSelectionModel::SelectionFlags selectionCommand(const QModelIndex &index, const QEvent *event = nullptr) const override;
	virtual void startDrag(Qt::DropActions supportedActions) override;
};

