#include "qjtreeitem.h"

using namespace std;

QJTREEITEM::QJTREEITEM(const JNODE& jn, QJTREEITEM* parent) : qjtiParent(parent)
{
	for (int ii = 0; ii < 8; ii++) {
		qlDataUserRole.append("");
	}
	qlDataUserRole[0] = get<0>(jn.colour).c_str();
	qlDataUserRole[1] = get<1>(jn.colour).c_str();
	qlDataUserRole[5] = "0";
	qlDataUserRole[6] = get<0>(jn.colourSelected).c_str();
	qlDataUserRole[7] = get<1>(jn.colourSelected).c_str();

	if (parent == nullptr) { treeType = -1; }
	else { treeType = parent->treeType; }

	int numCol = (int)jn.vsData.size() + (int)jn.mapAttribute.size();
	qlData.reserve(numCol);
	switch (treeType) {
	case 0:
	{
		for (int ii = 0; ii < numCol; ii++) {
			qlData << jn.vsData[ii].c_str();
		}
		break;
	}
	case 1:
	{
		qlData << jn.vsData[0].c_str();
		for (auto it = jn.mapAttribute.begin(); it != jn.mapAttribute.end(); ++it) {
			qlData << it->second.c_str();
		}
		break;
	}
	}

}
QJTREEITEM::QJTREEITEM(const vector<string>& vsData, QJTREEITEM* parent) 
	: qjtiParent(parent) {
	int numCol = (int)vsData.size();
	qlData.reserve(numCol);
	for (int ii = 0; ii < numCol; ii++) {
		qlData << vsData[ii].c_str();
	}

	if (parent != nullptr) {
		if (parent->qlDataUserRole.size() > 0) {
			qlDataUserRole = parent->qlDataUserRole;
		}
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
string QJTREEITEM::getName()
{
	// Note: this returns the node's vsData[0] string (name visible on tree), not 
	// Qt's ObjectName property. 
	string name;
	QString qsTemp;
	if (qlData.size() > 0) { 
		qsTemp = qlData[0].toString();
		name = qsTemp.toStdString();
	}
	return name;
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
void QJTREEITEM::removeChildren()
{
	qDeleteAll(qlChildren);
}
void QJTREEITEM::setData(int role, string sData)
{
	// If setting DisplayRole data, then sData is assumed to be a 
	// MIME-friendly list of column values, with the first char 
	// being the marker/splitter char. 
	if (role < 0) { return; }

	if (role == Qt::DisplayRole) {
		QString qsData = QString::fromUtf8(sData.c_str());
		QStringList qslData = qsData.split(qsData[0], Qt::KeepEmptyParts, Qt::CaseSensitive);
		qlData.clear();
		qlData.reserve(qslData.size());
		for (int ii = 1; ii < qslData.size(); ii++) {
			qlData << qslData[ii];
		}
	}
	else if (role >= Qt::UserRole) {
		while (role - Qt::UserRole >= qlDataUserRole.size()) {
			qlDataUserRole.append("");
		}
		qlDataUserRole[role - Qt::UserRole] = sData.c_str();
	}
}
void QJTREEITEM::setData(int role, QVariant qvData)
{
	// If setting DisplayRole data, then qvData is assumed to be a 
	// MIME-friendly list of column values, with the first char 
	// being the marker/splitter char. 
	if (role < 0) { return; }

	QString qsData = qvData.toString();
	if (role == Qt::DisplayRole) {
		QStringList qslData = qsData.split(qsData[0], Qt::KeepEmptyParts, Qt::CaseSensitive);
		qlData.clear();
		qlData.reserve(qslData.size());
		for (int ii = 1; ii < qslData.size(); ii++) {
			qlData << qslData[ii];
		}
	}
	else if (role >= Qt::UserRole) {
		while (role - Qt::UserRole >= qlDataUserRole.size()) {
			qlDataUserRole.append("");
		}
		qlDataUserRole[role - Qt::UserRole] = qsData;
	}
}
