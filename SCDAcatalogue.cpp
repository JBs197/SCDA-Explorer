#include "SCDAcatalogue.h"

void SCDAcatalogue::displayOnlineCata()
{
	QJTREEMODEL* qjtm = modelStatscan.get();
	qjtm->jt.reset();
	qjtm->reset();
	sco.getCataTree(qjtm->jt);

	qjtm->jt.compare(modelDatabase->jt);
	vector<int> viNoTwin = qjtm->jt.hasTwin(0);
	string sBG = "#80FF0000#FFFFFF";  // #AARRGGBB#RRGGBB BG over primer.
	qjtm->jt.setColourBG(viNoTwin, sBG);

	qjtm->populate();  // RESUME HERE - NEEDS TO CHECK FOR BG,FG COLOUR
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
	//treeStatscan->setModel(modelStatscan.get());
	gLayout->addWidget(treeStatscan, 1, indexStatscan);

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
