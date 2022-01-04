#include "SCDAcatalogue.h"

void SCDAcatalogue::displayOnlineCata()
{
	QJTREEMODEL* qjtm = modelStatscan.get();
	qjtm->jt.reset();
	qjtm->reset();
	sco.getCataTree(qjtm->jt);

	qjtm->jt.compare(modelDatabase->jt);
	qjtm->populate(); 

	QGridLayout* gLayout = (QGridLayout*)this->layout();
	QLayoutItem* qlItem = gLayout->itemAtPosition(1, indexStatscan);
	QJTREEVIEW* treeStatscan = (QJTREEVIEW*)qlItem->widget();
	treeStatscan->setModel(qjtm);
	treeStatscan->update();
}
void SCDAcatalogue::err(string message)
{
	string errorMessage = "SCDAcatalogue error:\n" + message;
	JLOG::getInstance()->err(errorMessage);
}
void SCDAcatalogue::getConfigXML(string configXML)
{
	sco.configXML = configXML;

	vector<string> vsTag = { "url", "statscan" };
	vector<vector<string>> vvsTag = jf.getXML(configXML, vsTag);
	sco.urlRoot = vvsTag[0][1];

	initItemColour(configXML);
}
QJTREEMODEL* SCDAcatalogue::getModel(int indexTree)
{
	switch (indexTree) {
	case 0:
		return modelStatscan.get();
	case 1:
		return modelLocal.get();
	case 2:
		return modelDatabase.get();
	}
	return nullptr;
}
void SCDAcatalogue::getStatscanURL(string url)
{
	sco.urlRoot = url;
}
void SCDAcatalogue::init()
{
	vector<string> vsHeader = {""};

	QGridLayout* gLayout = new QGridLayout;
	this->setLayout(gLayout);

	indexStatscan = 0;
	modelStatscan = make_shared<QJTREEMODEL>(vsHeader, this);
	QLabel* label = new QLabel("Online Catalogues");
	gLayout->addWidget(label, 0, indexStatscan);
	QJTREEVIEW* treeStatscan = new QJTREEVIEW;
	treeStatscan->indexTree = indexStatscan;
	gLayout->addWidget(treeStatscan, 1, indexStatscan);
	connect(treeStatscan, &QJTREEVIEW::nodeClicked, this, &SCDAcatalogue::nodeClicked);

	indexLocal = 1;
	modelLocal = make_shared<QJTREEMODEL>(vsHeader, this);
	label = new QLabel("Local Catalogues");
	gLayout->addWidget(label, 0, indexLocal);
	QJTREEVIEW* treeLocal = new QJTREEVIEW;
	treeLocal->setModel(modelLocal.get());
	gLayout->addWidget(treeLocal, 1, indexLocal);

	indexDatabase = 2;
	modelDatabase = make_shared<QJTREEMODEL>(vsHeader, this);
	label = new QLabel("Database Catalogues");
	gLayout->addWidget(label, 0, indexDatabase);
	QJTREEVIEW* treeDatabase = new QJTREEVIEW;
	treeDatabase->setModel(modelDatabase.get());
	gLayout->addWidget(treeDatabase, 1, indexDatabase);
}
void SCDAcatalogue::initItemColour(string& configXML)
{
	QMap<int, QVariant> itemColourDefault;
	vector<string> vsTag = { "colour", "item_default" };
	vector<vector<string>> vvsTag = jf.getXML(configXML, vsTag);
	for (int ii = 0; ii < vvsTag.size(); ii++) {
		if (vvsTag[ii][0] == "background") {
			itemColourDefault[Qt::UserRole] = vvsTag[ii][1].c_str();
		}
		else if (vvsTag[ii][0] == "foreground") {
			itemColourDefault[Qt::UserRole + 1] = vvsTag[ii][1].c_str();
		}
	}
	
	QMap<int, QVariant> itemColourFail;
	vsTag = { "colour", "item_fail" };
	vvsTag = jf.getXML(configXML, vsTag);
	for (int ii = 0; ii < vvsTag.size(); ii++) {
		if (vvsTag[ii][0] == "background") {
			itemColourFail[Qt::UserRole] = vvsTag[ii][1].c_str();
		}
		else if (vvsTag[ii][0] == "foreground") {
			itemColourFail[Qt::UserRole + 1] = vvsTag[ii][1].c_str();
		}
	}
	
	QMap<int, QVariant> itemColourSelected;
	vsTag = { "colour", "item_selected" };
	vvsTag = jf.getXML(configXML, vsTag);
	for (int ii = 0; ii < vvsTag.size(); ii++) {
		if (vvsTag[ii][0] == "background") {
			itemColourSelected[Qt::UserRole] = vvsTag[ii][1].c_str();
		}
		else if (vvsTag[ii][0] == "foreground") {
			itemColourSelected[Qt::UserRole + 1] = vvsTag[ii][1].c_str();
		}
	}

	QGridLayout* gLayout = (QGridLayout*)this->layout();
	QLayoutItem* qlItem = nullptr;
	QJTREEVIEW* qjTree = nullptr;
	for (int ii = 0; ii < 3; ii++) {
		qlItem = gLayout->itemAtPosition(1, ii);
		qjTree = (QJTREEVIEW*)qlItem->widget();
		qjTree->itemColourDefault = itemColourDefault;
		qjTree->itemColourFail = itemColourFail;
		qjTree->itemColourSelected = itemColourSelected;
	}
}
void SCDAcatalogue::nodeClicked(const QModelIndex& qmIndex, int indexTree)
{
	QGridLayout* gLayout = (QGridLayout*)this->layout();
	QLayoutItem* qlItem = gLayout->itemAtPosition(1, indexTree);
	QJTREEVIEW* qjTree = (QJTREEVIEW*)qlItem->widget();
	QItemSelectionModel* selModel = qjTree->selectionModel();
	QModelIndexList selList = selModel->selectedIndexes();
	int bbq = 1;
}
void SCDAcatalogue::resetModel(int indexTree)
{
	switch (indexTree) {
	case 0:
		modelStatscan.reset();
		break;
	case 1:
		modelLocal.reset();
		break;
	case 2:
		modelDatabase.reset();
		break;
	}
}
