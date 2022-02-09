#pragma once
#include <QVariant>
#include "jtxml.h"

class QJTREEITEM
{
	QJTREEITEM* qjtiParent;
	QList<QJTREEITEM*> qlChildren;
	QList<QVariant> qlData;
	QList<QVariant> qlDataUserRole;

public:
	explicit QJTREEITEM(const JNODE& jn, QJTREEITEM* parent = nullptr);
	explicit QJTREEITEM(const std::vector<std::string>& vsData, QJTREEITEM* parent = nullptr);
	explicit QJTREEITEM(const QList<QVariant>& qlVarData, QJTREEITEM* parent = nullptr);
	~QJTREEITEM() { qDeleteAll(qlChildren); }

	int treeType = -1;

	void addChild(QJTREEITEM* qjti);
	QJTREEITEM* getChild(int iRow);
	QVariant data(int iCol) const;
	void dataAll(QList<QVariant>& qlVarData) const;
	QVariant dataUserRole(int role) const;
	std::string getName();
	int getNumChildren();
	int getNumCol();
	QJTREEITEM* getParent();
	int getRow() const;
	void removeChildren();
	void setData(int role, std::string sData);
	void setData(int role, QVariant qvData);
};