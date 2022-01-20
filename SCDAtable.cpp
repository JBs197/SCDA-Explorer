#include "SCDAtable.h"

using namespace std;

void SCDAtable::cellRightClicked(const QPoint& globalPos, const QModelIndex& qmIndex, int indexTable)
{
	//
}
void SCDAtable::deleteTable()
{
	QVariant qVar = qaDelete->data();
	QString qsTemp = qVar.toString();
	string tname = qsTemp.toUtf8();
	emit sendDeleteTable(tname);
}
void SCDAtable::displayTable(vector<vector<string>>& vvsData, vector<vector<string>>& vvsColTitle, string title)
{
	int numCol = -1, numRow = (int)vvsData.size();
	if (numRow > 0) { numCol = (int)vvsData[0].size(); }

	QGridLayout* gLayout = (QGridLayout*)this->layout();
	QLayoutItem* qlItem = gLayout->itemAtPosition(0, index::OnDemand);
	QLabel* label = (QLabel*)qlItem->widget();
	label->setText(title.c_str());

	qlItem = gLayout->itemAtPosition(1, index::OnDemand);
	QJTABLEVIEW* qjTable = (QJTABLEVIEW*)qlItem->widget();
	qjTable->resetModel(0);
	QHeaderView* headerH = qjTable->horizontalHeader();
	if (vvsColTitle[0].size() < 1) { headerH->setVisible(0); }
	else if (vvsColTitle[0].size() != numCol) { err("Column titles do not match columns-displayTable"); }
	if (numRow < 1 || numCol < 1) { return; }

	qjTable->setColTitles(vvsColTitle);
	qjTable->setTableData(vvsData);
}
void SCDAtable::err(string message)
{
	string errorMessage = "SCDAtable error:\n" + message;
	JLOG::getInstance()->err(errorMessage);
}
QStandardItemModel* SCDAtable::getModelTable(int index)
{
	switch (index) {
	case index::OnDemand:
		return modelOnDemand.get();
	}
}
QJTREEMODEL* SCDAtable::getModelTree(int index)
{
	switch (index) {
	case index::Search:
		return modelSearch.get();
	}
}
void SCDAtable::init()
{
	QGridLayout* gLayout = new QGridLayout;
	this->setLayout(gLayout);
	vector<string> vsHeader = { "" };

	modelOnDemand = make_shared<QStandardItemModel>(this);
	QLabel* label = new QLabel("Table (none)");
	gLayout->addWidget(label, 0, index::OnDemand);
	QJTABLEVIEW* qjTable = new QJTABLEVIEW;
	gLayout->addWidget(qjTable, 1, index::OnDemand);
	qjTable->setModel(modelOnDemand.get());
	qjTable->indexTable = index::OnDemand;
	qjTable->itemColourDefault = itemColourDefault;
	qjTable->itemColourSelected = itemColourSelected;
	QHeaderView* headerV = qjTable->verticalHeader();
	headerV->setVisible(0);
	connect(qjTable, &QJTABLEVIEW::cellRightClicked, this, &SCDAtable::cellRightClicked);

	modelSearch = make_shared<QJTREEMODEL>(vsHeader, this);
	modelSearch->jt.setNodeColour(-1, itemColourDefault, itemColourSelected);
	label = new QLabel("Search Results");
	gLayout->addWidget(label, 0, index::Search);
	QJTREEVIEW* treeSearch = new QJTREEVIEW;
	treeSearch->indexTree = index::Search;
	gLayout->addWidget(treeSearch, 1, index::Search);
	connect(treeSearch, &QJTREEVIEW::nodeDoubleClicked, this, &SCDAtable::nodeDoubleClicked);
	connect(treeSearch, &QJTREEVIEW::nodeRightClicked, this, &SCDAtable::nodeRightClicked);

	initAction();
}
void SCDAtable::initAction()
{
	qaDelete = new QAction("Delete Table", this);
	connect(qaDelete, &QAction::triggered, this, &SCDAtable::deleteTable);
}
void SCDAtable::initItemColour(string& configXML)
{
	int indexBG = -1, indexFG = -1;
	vector<string> vsTag = { "colour", "solid", "item_default" };
	vector<vector<string>> vvsTag = jf.getXML(configXML, vsTag);
	for (int ii = 0; ii < vvsTag.size(); ii++) {
		if (vvsTag[ii][0] == "background") { indexBG = ii; }
		else if (vvsTag[ii][0] == "foreground") { indexFG = ii; }
	}
	itemColourDefault = make_pair(vvsTag[indexBG][1], vvsTag[indexFG][1]);

	vsTag = { "colour", "solid", "item_fail" };
	vvsTag = jf.getXML(configXML, vsTag);
	for (int ii = 0; ii < vvsTag.size(); ii++) {
		if (vvsTag[ii][0] == "background") { indexBG = ii; }
		else if (vvsTag[ii][0] == "foreground") { indexFG = ii; }
	}
	itemColourFail = make_pair(vvsTag[indexBG][1], vvsTag[indexFG][1]);

	vsTag = { "colour", "solid", "item_selected" };
	vvsTag = jf.getXML(configXML, vsTag);
	for (int ii = 0; ii < vvsTag.size(); ii++) {
		if (vvsTag[ii][0] == "background") { indexBG = ii; }
		else if (vvsTag[ii][0] == "foreground") { indexFG = ii; }
	}
	itemColourSelected = make_pair(vvsTag[indexBG][1], vvsTag[indexFG][1]);
	itemColourHover = itemColourSelected;

	vsTag = { "colour", "solid", "item_warning" };
	vvsTag = jf.getXML(configXML, vsTag);
	for (int ii = 0; ii < vvsTag.size(); ii++) {
		if (vvsTag[ii][0] == "background") { indexBG = ii; }
		else if (vvsTag[ii][0] == "foreground") { indexFG = ii; }
	}
	itemColourWarning = make_pair(vvsTag[indexBG][1], vvsTag[indexFG][1]);
}
void SCDAtable::nodeDoubleClicked(const QModelIndex& qmiNode, int indexTree)
{
	string tname;
	vector<string> vsGenealogy;
	switch (indexTree) {
	case index::Search:
	{
		auto model = modelSearch.get();
		vsGenealogy = model->getGenealogy(qmiNode);
		tname = vsGenealogy[0];
		for (int ii = 1; ii < vsGenealogy.size(); ii++) {
			tname += "$" + vsGenealogy[ii];
		}
		emit fetchTable(tname);
	}
	}
}
void SCDAtable::nodeRightClicked(const QPoint& globalPos, const QModelIndex& qmIndex, int indexTree)
{
	QMenu menu(this);
	QJTREEMODEL* qjtm = modelSearch.get();
	vector<string> vsGenealogy = qjtm->getGenealogy(qmIndex);
	int numNode = (int)vsGenealogy.size();
	string tname = "";
	for (int ii = 0; ii < numNode; ii++) {
		if (ii > 0) { tname += "$"; }
		tname += vsGenealogy[ii];
	}
	qaDelete->setData(tname.c_str());
	menu.addAction(qaDelete);
	menu.exec(globalPos);
}
void SCDAtable::resetModel(int indexModel)
{
	vector<string> vsHeader = { "" };
	shared_ptr<QStandardItemModel> modelStd = nullptr;
	shared_ptr<QJTREEMODEL> modelJTree = nullptr;
	switch (indexModel) {
	case index::OnDemand:
		modelStd = make_shared<QStandardItemModel>(this);
		modelOnDemand.swap(modelStd);
		break;
	case index::Search:
		modelJTree = make_shared<QJTREEMODEL>(vsHeader, this);
		modelSearch.swap(modelJTree);
		modelSearch->jt.setNodeColour(-1, itemColourDefault, itemColourSelected);
		break;
	}
}
