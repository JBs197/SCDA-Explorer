#include "qjtreeitem.h"

QJTREEITEM::QJTREEITEM(const JNODE& jn, QJTREEITEM* parent) : qjtiParent(parent)
{
	int numCol = (int)jn.vsData.size();
	qlData.reserve(numCol);
	for (int ii = 0; ii < numCol; ii++) {
		qlData << jn.vsData[ii].c_str();
	}
	for (int ii = 0; ii < 8; ii++) {
		qlDataUserRole.append("");
	}
	if (jn.sColourBG.size() > 0) { qlDataUserRole[0] = jn.sColourBG.c_str(); }
	if (jn.sColourFG.size() > 0) { qlDataUserRole[1] = jn.sColourFG.c_str(); }
	qlDataUserRole[5] = "0";
	if (jn.sColourBGSel.size() > 0) { qlDataUserRole[6] = jn.sColourBGSel.c_str(); }
	if (jn.sColourFGSel.size() > 0) { qlDataUserRole[7] = jn.sColourFGSel.c_str(); }
}
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
