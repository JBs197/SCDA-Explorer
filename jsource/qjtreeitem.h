#pragma once
#include <QVariant>
#include "jstring.h"

using namespace std;

class QJTREEITEM
{
	QJTREEITEM* qjtiParent;
	QList<QJTREEITEM*> qlChildren;
	QList<QVariant> qlData;
	QList<QVariant> qlDataUserRole;

public:
	explicit QJTREEITEM(const vector<string>& vsData, QJTREEITEM* parent = nullptr);
	explicit QJTREEITEM(const QStringList& qslData, QJTREEITEM* parent = nullptr);
	~QJTREEITEM() { qDeleteAll(qlChildren); }

	void addChild(QJTREEITEM* qjti);
	QJTREEITEM* getChild(int iRow);
	QVariant data(int iCol) const;
	QVariant dataUserRole(int role) const;
	int getNumChildren();
	int getNumCol();
	QJTREEITEM* getParent();
	int getRow() const;
	void setDataUserRole(int role, string sDataUserRole);
};