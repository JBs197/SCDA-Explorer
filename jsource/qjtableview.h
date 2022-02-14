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
#include "jlog.h"
#include "jstring.h"
#include "qjdelegate.h"

class QJTABLEVIEW : public QTableView
{
	Q_OBJECT

private:
	int acceptActions = 0;
	JSTRING jstr;
	std::unordered_map<std::string, int> mapColIndex;  // Column header -> column index
	QString mimeColHeader;
	int numBlankRow;
	QPoint posStart;
	QModelIndex qmiHover;
	int recentIndex = -1, startDragDist;
	std::vector<int> viDisplayCol, viName;

	void err(std::string message);
	void init();
	void updateHeight();

public:
	QJTABLEVIEW(QWidget* parent = nullptr)
		: QTableView(parent) {
		init();
	}
	~QJTABLEVIEW() {}

	int indexChecksum, indexTable;
	std::pair<std::string, std::string> itemColourDefault, itemColourFail, itemColourHover;
	std::pair<std::string, std::string> itemColourSelected, itemColourTemp, itemColourWarning;

	void applyFilter(std::string filter);
	void applyFilter(QString qsFilter);
	void deleteRow(int iRow);
	std::vector<int> getCellDimensions(int iRow, int iCol);
	int getHeight();
	std::vector<int> getName() { return viName; }
	QMimeData* getRowMimeData(int iRow);
	QModelIndexList getSelected();
	void setCellColour(QModelIndex qmi, std::pair<std::string, std::string> colour);
	void setCellColour(QModelIndexList qmiList, std::pair<std::string, std::string> colour);
	void setColColour(QModelIndex qmi, std::pair<std::string, std::string> colour);
	void setRowColour(QModelIndex qmi, std::pair<std::string, std::string> colour);
	QPixmap makeCellPixmap(int iRow, int iCol);
	QList<QStandardItem*> mimeTextToQRow(QString& qsMime);
	void resetModel(int keepLast);
	void select(int iRow, int iCol);
	void setColTitles(std::vector<std::vector<std::string>>& vvsColTitles);
	void setModel(QAbstractItemModel* model) override;
	void setName(int indexTab, int indexRow, int indexCol);
	void setTableData(std::vector<std::vector<std::string>>& vvsData);
	void updateModel();
	void updateModel(std::vector<std::vector<int>> vviCol);

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
	virtual QItemSelectionModel::SelectionFlags selectionCommand(const QModelIndex& index, const QEvent* event = nullptr) const override;
	virtual void startDrag(Qt::DropActions supportedActions) override;
};
