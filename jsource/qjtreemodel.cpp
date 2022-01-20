#include "qjtreemodel.h"

using namespace std;

QJTREEMODEL::QJTREEMODEL(vector<string> vsHeader, QObject* parent) 
	: QAbstractItemModel(parent) {
	qjtiRoot = new QJTREEITEM(vsHeader);
	setHeaderData(vsHeader);
	treeType = -1;
}

void QJTREEMODEL::addChildrenAll(int parentID, QJTREEITEM*& qjtiParent)
{
	// Recursive function which takes matching nodes from a JTREE and this
	// model, and adds the node's immediate children to the model from the JTREE.
	QJTREEITEM* qjtiChild = nullptr;
	vector<int> childrenID; 	
	int numChildren; 
	switch (treeType) {
	case tree::jtree:
	{
		childrenID = jt.getChildrenID(parentID);
		numChildren = (int)childrenID.size();
		for (int ii = 0; ii < numChildren; ii++) {
			JNODE& jn = jt.getNode(childrenID[ii]);
			qjtiChild = new QJTREEITEM(jn, qjtiParent);
			qjtiParent->addChild(qjtiChild);
			addChildrenAll(childrenID[ii], qjtiChild);
		}
		break;
	}
	case tree::jtxml:
	{
		childrenID = jtx.getChildrenID(parentID);
		numChildren = (int)childrenID.size();
		for (int ii = 0; ii < numChildren; ii++) {
			JNODE& jn = jtx.getNode(childrenID[ii]);
			qjtiChild = new QJTREEITEM(jn, qjtiParent);
			qjtiParent->addChild(qjtiChild);
			addChildrenAll(childrenID[ii], qjtiChild);
		}
		break;
	}
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
	if (role < Qt::UserRole) { return qjti->data(role); }
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
vector<string> QJTREEMODEL::getGenealogy(const QModelIndex& index) const
{
	// Return form [node col0, parent col0, grandparent col0, ...]
	vector<string> vsGenealogy, vsTemp(1);
	QJTREEITEM* node = static_cast<QJTREEITEM*>(index.internalPointer());
	QVariant qVar = node->data(0);
	QString qsTemp = qVar.toString();
	string temp = qsTemp.toUtf8();
	vsTemp[0] = temp;
	QJTREEITEM* parent = node->getParent();
	while (parent != nullptr) {
		node = parent;
		qVar = node->data(0);
		qsTemp = qVar.toString();
		temp = qsTemp.toUtf8();
		vsTemp.push_back(temp);
		parent = node->getParent();
	}
	vsTemp.pop_back();
	int numNode = (int)vsTemp.size();
	vsGenealogy.resize(numNode);
	for (int ii = 0; ii < numNode; ii++) {
		vsGenealogy[ii] = vsTemp[numNode - 1 - ii];
	}
	return vsGenealogy;
}
QJTREEITEM* QJTREEMODEL::getNode(const QModelIndex& qmiNode)
{
	if (!qmiNode.isValid()) { return nullptr; }
	return static_cast<QJTREEITEM*>(qmiNode.internalPointer());
}
QJTREEITEM* QJTREEMODEL::getRoot()
{
	return qjtiRoot;
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
void QJTREEMODEL::populate(int enumTree)
{
	// Loads a JTREE (or derivative) into this model.
	treeType = enumTree;
	qjtiRoot->treeType = enumTree;

	switch (enumTree) {
	case tree::jtree:
	{
		JNODE jnRoot = jt.getRoot();
		int rootID = jnRoot.ID;
		addChildrenAll(rootID, qjtiRoot);
		break;
	}
	case tree::jtxml:
	{
		JNODE jnRoot = jtx.getRoot();
		int rootID = jnRoot.ID;
		addChildrenAll(rootID, qjtiRoot);
		break;
	}
	}
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
bool QJTREEMODEL::setData(const QModelIndex& index, const QVariant& value, int role)
{
	if (!index.isValid()) { return 0; }
	QJTREEITEM* qjti = static_cast<QJTREEITEM*>(index.internalPointer());
	qjti->setData(role, value);
	return 1;
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
