#include "qjtreeitem.h"

QJTREEITEM::QJTREEITEM(const vector<string>& vsData, QJTREEITEM* parent) 
	: qjtiParent(parent) {
	int numCol = (int)vsData.size();
	qlData.reserve(numCol);
	for (int ii = 0; ii < numCol; ii++) {
		qlData << vsData[ii].c_str();
	}
}
QJTREEITEM::QJTREEITEM(const QStringList& qslData, QJTREEITEM* parent) 
	: qjtiParent(parent) {
	int numCol = qslData.length();
	qlData.reserve(numCol);
	for (int ii = 0; ii < numCol; ii++) {
		qlData << qslData[ii];
	}
}

void QJTREEITEM::addChild(QJTREEITEM* qjti)
{
	qlChildren.append(qjti);
}
QJTREEITEM* QJTREEITEM::getChild(int iRow)
{
	if (iRow < 0 || iRow >= qlChildren.size()) { return nullptr; }
	return qlChildren.at(iRow);
}
QVariant QJTREEITEM::data(int iCol) const
{
	// Used for DisplayRole data.
	if (iCol < 0 || iCol >= qlData.size()) { return QVariant(); }
	return qlData.at(iCol);
}
QVariant QJTREEITEM::dataUserRole(int role) const
{
	// Used for UserRole data.
	if (role - Qt::UserRole < 0 || role - Qt::UserRole >= qlDataUserRole.size()) {
		return QVariant();
	}
	return qlDataUserRole.at(role - Qt::UserRole);
}
int QJTREEITEM::getNumChildren()
{
	return qlChildren.size();
}
int QJTREEITEM::getNumCol()
{
	return qlData.size();
}
QJTREEITEM* QJTREEITEM::getParent()
{
	return qjtiParent;
}
int QJTREEITEM::getRow() const
{
	if (qjtiParent != nullptr) {
		return qjtiParent->qlChildren.indexOf(const_cast<QJTREEITEM*>(this));
	}
	return -1;
}
void QJTREEITEM::setDataUserRole(int role, string sDataUserRole)
{
	if (role - Qt::UserRole < 0) { return; }
	else if (role - Qt::UserRole >= qlDataUserRole.size()) {
		qlDataUserRole.reserve(role - Qt::UserRole + 1);
	}
	qlDataUserRole[role - Qt::UserRole] = sDataUserRole.c_str();
}
