#include "SCDAcatalogue.h"

void SCDAcatalogue::displayOnlineCata(SWITCHBOARD& sbgui, SCDAcatalogue*& cata, SConline& sco)
{
	thread::id myid = this_thread::get_id();
	vector<int> mycomm;
	sbgui.answer_call(myid, mycomm);

	QJTREEMODEL* qjtm = cata->modelStatscan.get();
	qjtm->jt.reset();
	qjtm->reset();

	sco.getCataTree(qjtm->jt);
	qjtm->jt.setNodeColourSel({}, get<0>(itemColourSelected), get<1>(itemColourSelected));
	qjtm->jt.compare(cata->modelDatabase->jt);
	vector<int> viNoTwin = qjtm->jt.hasTwinList(0);
	qjtm->jt.setNodeColour(viNoTwin, get<0>(itemColourFail), get<1>(itemColourFail));
	vector<int> viTwin = qjtm->jt.hasTwinList(1);
	qjtm->jt.setNodeColour(viTwin, get<0>(itemColourDefault), get<1>(itemColourFail));
	qjtm->populate(); 

	mycomm[0] = 1;
	sbgui.update(myid, mycomm);
}
void SCDAcatalogue::downloadCata()
{
	QVariant qVar = qaDownload->data();
	QString qsTemp = qVar.toString();
	string prompt = qsTemp.toUtf8();
	emit sendDownloadCata(prompt);
}
void SCDAcatalogue::err(string message)
{
	string errorMessage = "SCDAcatalogue error:\n" + message;
	JLOG::getInstance()->err(errorMessage);
}
void SCDAcatalogue::getConfigXML(string configXML)
{
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
	connect(treeStatscan, &QJTREEVIEW::nodeRightClicked, this, &SCDAcatalogue::nodeRightClicked);

	indexLocal = 1;
	modelLocal = make_shared<QJTREEMODEL>(vsHeader, this);
	label = new QLabel("Local Catalogues");
	gLayout->addWidget(label, 0, indexLocal);
	QJTREEVIEW* treeLocal = new QJTREEVIEW;
	treeLocal->setModel(modelLocal.get());
	gLayout->addWidget(treeLocal, 1, indexLocal);
	connect(treeStatscan, &QJTREEVIEW::nodeRightClicked, this, &SCDAcatalogue::nodeRightClicked);

	indexDatabase = 2;
	modelDatabase = make_shared<QJTREEMODEL>(vsHeader, this);
	label = new QLabel("Database Catalogues");
	gLayout->addWidget(label, 0, indexDatabase);
	QJTREEVIEW* treeDatabase = new QJTREEVIEW;
	treeDatabase->setModel(modelDatabase.get());
	gLayout->addWidget(treeDatabase, 1, indexDatabase);
	connect(treeStatscan, &QJTREEVIEW::nodeRightClicked, this, &SCDAcatalogue::nodeRightClicked);

	initAction();
}
void SCDAcatalogue::initAction()
{
	qaDownload = new QAction("Download", this);
	connect(qaDownload, &QAction::triggered, this, &SCDAcatalogue::downloadCata);
}
void SCDAcatalogue::initItemColour(string& configXML)
{
	int indexBG, indexFG;
	vector<string> vsTag = { "colour", "solid", "item_default" };
	vector<vector<string>> vvsTag = jf.getXML(configXML, vsTag);
	for (int ii = 0; ii < vvsTag.size(); ii++) {
		if (vvsTag[ii][0] == "background") { indexBG = ii; }
		else if (vvsTag[ii][0] == "foreground") { indexFG = ii;	}
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
}
void SCDAcatalogue::nodeClicked(const QModelIndex& qmIndex, int indexTree)
{
	int bbq = 1;
}
void SCDAcatalogue::nodeRightClicked(const QPoint& globalPos, const QModelIndex& qmIndex, int indexTree)
{
	QMenu menu(this);
	QJTREEMODEL* qjtm = getModel(indexTree);
	vector<string> vsGenealogy = qjtm->getGenealogy(qmIndex);
	int numNode = (int)vsGenealogy.size();
	switch (indexTree) {
	case 0:
	{
		string sData = "";
		for (int ii = 0; ii < numNode; ii++) {
			sData += "@" + vsGenealogy[ii];
		}
		qaDownload->setData(sData.c_str());
		menu.addAction(qaDownload);
		break;
	}
	}
	menu.exec(globalPos);
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
