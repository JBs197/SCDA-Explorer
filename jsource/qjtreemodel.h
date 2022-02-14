#pragma once
#include <QAbstractItemModel>
#include "qjtreeitem.h"

class QJTREEMODEL : public QAbstractItemModel
{
	Q_OBJECT

private:
	JSTRING jstr;
	QJTREEITEM* qjtiRoot;
	int treeType;

	void addChildrenAll(int parentID, QJTREEITEM*& qjtiParent);
	void err(std::string message);

public:
	QJTREEMODEL(std::vector<std::string> vsHeader, QObject* parent = nullptr);
	~QJTREEMODEL() { delete qjtiRoot; }

	enum tree{ jtree, jtxml };

	bool headerTitles;
	JTREE jt;
	JTXML jtx;

	int columnCount(const QModelIndex& parent = QModelIndex()) const override;
	QVariant data(const QModelIndex& index, int role) const override;
	Qt::ItemFlags flags(const QModelIndex& index) const override;
	std::vector<std::string> getGenealogy(const QModelIndex& index);
	QJTREEITEM* getNode(const QModelIndex& qmiNode);
	QJTREEITEM* getRoot();
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
	QModelIndex index(int row, int column, const QModelIndex& index = QModelIndex()) const override;
	QModelIndex parent(const QModelIndex& parent = QModelIndex()) const override;
	void populate(int enumTree);
	int rowCount(const QModelIndex& parent = QModelIndex()) const override;
	bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
	void setHeaderData(std::vector<std::string> vsHeader, int role = Qt::DisplayRole);
};