#pragma once
#include <QAbstractItemModel>
#include "qjtreeitem.h"

using namespace std;

class QJTREEMODEL : public QAbstractItemModel
{
	Q_OBJECT

private:
	QJTREEITEM* qjtiRoot;
	int treeType;

	void addChildrenAll(int parentID, QJTREEITEM*& qjtiParent);
	void err(string message);

public:
	explicit QJTREEMODEL(vector<string> vsHeader, QObject* parent = nullptr);
	~QJTREEMODEL() { delete qjtiRoot; }

	enum tree{ jtree, jtxml };

	bool headerTitles;
	JTREE jt;
	JTXML jtx;

	int columnCount(const QModelIndex& parent = QModelIndex()) const override;
	QVariant data(const QModelIndex& index, int role) const override;
	Qt::ItemFlags flags(const QModelIndex& index) const override;
	vector<string> getGenealogy(const QModelIndex& index) const;
	QJTREEITEM* getRoot() { return qjtiRoot; }
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
	QModelIndex index(int row, int column, const QModelIndex& index = QModelIndex()) const override;
	QModelIndex parent(const QModelIndex& parent = QModelIndex()) const override;
	void populate(int enumTree);
	int rowCount(const QModelIndex& parent = QModelIndex()) const override;
	bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
	void setHeaderData(vector<string> vsHeader, int role = Qt::DisplayRole);
};