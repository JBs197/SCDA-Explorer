#pragma once
#include <QAbstractItemModel>
#include "jtree.h"
#include "qjtreeitem.h"

using namespace std;

class QJTREEMODEL : public QAbstractItemModel
{
	Q_OBJECT

private:
	QJTREEITEM* qjtiRoot;

	void addChildrenAll(int parentID, QJTREEITEM*& qjtiParent);
	void err(string message);

public:
	explicit QJTREEMODEL(vector<string> vsHeader, QObject* parent = nullptr);
	~QJTREEMODEL() { delete qjtiRoot; }

	bool headerTitles;
	JTREE jt;

	int columnCount(const QModelIndex& parent = QModelIndex()) const override;
	QVariant data(const QModelIndex& index, int role) const override;
	Qt::ItemFlags flags(const QModelIndex& index) const override;
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
	QModelIndex index(int row, int column, const QModelIndex& index = QModelIndex()) const override;
	QModelIndex parent(const QModelIndex& parent = QModelIndex()) const override;
	void populate();
	void reset();
	int rowCount(const QModelIndex& parent = QModelIndex()) const override;
	void setHeaderData(vector<string> vsHeader, int role = Qt::DisplayRole);
};