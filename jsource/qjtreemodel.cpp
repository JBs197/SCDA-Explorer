#include "qjtreemodel.h"

QJTREEMODEL::QJTREEMODEL(vector<string> vsHeader, QObject* parent) 
	: QAbstractItemModel(parent) {
	qjtiRoot = new QJTREEITEM(vsHeader);
	setHeaderData(vsHeader);
}

void QJTREEMODEL::addChildrenAll(int parentID, QJTREEITEM*& qjtiParent)
{
	// Recursive function which takes matching nodes from a JTREE and this
	// model, and adds the node's immediate children to the model from the JTREE.
	vector<string> vsData, vsDataUserRole;
	QJTREEITEM* qjtiChild = nullptr;
	vector<int> childrenID = jt.getChildrenID(parentID);
	int numChildren = (int)childrenID.size();
	for (int ii = 0; ii < numChildren; ii++) {
		vsData = jt.getData(childrenID[ii]);
		vsDataUserRole = jt.getDataUserRole(childrenID[ii]);
		qjtiChild = new QJTREEITEM(vsData, qjtiParent);
		qjtiChild->setDataUserRole(Qt::UserRole + 5, "0");
		qjtiChild->setDataUserRole(Qt::UserRole + 5, "0");
		qjtiParent->addChild(qjtiChild);
		addChildrenAll(childrenID[ii], qjtiChild);
	}
}
int QJTREEMODEL::columnCount(const QModelIndex& parent) const
{
	if (parent.isValid()) {
		return static_cast<QJTREEITEM*>(parent.internalPointer())->getNumCol();
	}
	return qjtiRoot->getNumCol();
}
QVariant QJTREEMODEL::data(const QModelIndex& index, int role) const
{
	if (!index.isValid()) { return QVariant(); }	
	
	QJTREEITEM* qjti = static_cast<QJTREEITEM*>(index.internalPointer());
	if (role == Qt::DisplayRole) { return qjti->data(index.column()); }	
	else if (role >= Qt::UserRole) { return qjti->dataUserRole(role); }

	return QVariant();
}
void QJTREEMODEL::err(string message)
{
	string errorMessage = "QJTREE error:\n" + message;
	JLOG::getInstance()->err(errorMessage);
}
Qt::ItemFlags QJTREEMODEL::flags(const QModelIndex& index) const
{
	if (!index.isValid()) { return Qt::NoItemFlags; }
	
	return QAbstractItemModel::flags(index);
}
QVariant QJTREEMODEL::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
		return qjtiRoot->data(section);
	}	
	return QVariant();
}
QModelIndex QJTREEMODEL::index(int row, int column, const QModelIndex& parent) const
{
	if (!hasIndex(row, column, parent)) { return QModelIndex(); }		

	QJTREEITEM* qjtiParent;
	if (parent.isValid()) {
		qjtiParent = static_cast<QJTREEITEM*>(parent.internalPointer());
	}
	else { qjtiParent = qjtiRoot; }

	QJTREEITEM* qjtiChild = qjtiParent->getChild(row);
	if (qjtiChild == nullptr) { return QModelIndex(); }		
	return createIndex(row, column, qjtiChild);
}
QModelIndex QJTREEMODEL::parent(const QModelIndex& index) const
{
	if (!index.isValid()) { return QModelIndex(); }		

	QJTREEITEM* qjtiChild = static_cast<QJTREEITEM*>(index.internalPointer());
	QJTREEITEM* qjtiParent = qjtiChild->getParent();
	if (qjtiParent == qjtiRoot) { return QModelIndex(); }		

	return createIndex(qjtiParent->getRow(), 0, qjtiParent);
}
void QJTREEMODEL::populate()
{
	// Loads a JTREE into this model.
	JNODE jnRoot = jt.getRoot();
	int rootID = jnRoot.ID;
	addChildrenAll(rootID, qjtiRoot);
}
void QJTREEMODEL::reset()
{
	// Removes all tree items without affecting the headers. 
	int numCol = qjtiRoot->getNumCol();
	QStringList qslData;
	QVariant qVar;
	for (int ii = 0; ii < numCol; ii++) {
		qVar = qjtiRoot->data(ii);
		qslData.append(qVar.toString());
	}
	delete qjtiRoot;

	qjtiRoot = new QJTREEITEM(qslData);
}
int QJTREEMODEL::rowCount(const QModelIndex& parent) const
{
	if (parent.column() > 0) { return 0; }
	
	QJTREEITEM* qjtiParent;
	if (parent.isValid()) { 
		qjtiParent = static_cast<QJTREEITEM*>(parent.internalPointer()); 
	}
	else { qjtiParent = qjtiRoot; }
	return qjtiParent->getNumChildren();
}
void QJTREEMODEL::setHeaderData(vector<string> vsHeader, int role)
{
	headerTitles = 0;
	int numCol = (int)vsHeader.size();
	for (int ii = 0; ii < numCol; ii++) {
		if (vsHeader[ii].size() > 0) {
			QAbstractItemModel::setHeaderData(ii, Qt::Horizontal, vsHeader[ii].c_str(), role);
			headerTitles = 1;
		}
	}
}
