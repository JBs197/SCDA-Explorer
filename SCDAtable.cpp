#include "SCDAtable.h"

void SCDAtable::cellRightClicked(const QPoint& globalPos, const QModelIndex& qmIndex, int indexTable)
{
	//
}
void SCDAtable::err(string message)
{
	string errorMessage = "SCDAtable error:\n" + message;
	JLOG::getInstance()->err(errorMessage);
}
void SCDAtable::init()
{
	QGridLayout* gLayout = new QGridLayout;
	this->setLayout(gLayout);

	indexOnDemand = 0;
	modelOnDemand = make_shared<QStandardItemModel>(this);
	QLabel* label = new QLabel;
	gLayout->addWidget(label, 0, indexOnDemand);
	QJTABLEVIEW* qjTable = new QJTABLEVIEW;
	gLayout->addWidget(qjTable, 1, indexOnDemand);
	qjTable->setModel(modelOnDemand.get());
	qjTable->indexTable = indexOnDemand;
	connect(qjTable, &QJTABLEVIEW::cellRightClicked, this, &SCDAtable::cellRightClicked);

	initAction();
}
void SCDAtable::initAction()
{
	//
}
void SCDAtable::initItemColour(QJTABLEVIEW*& qjTable, string& configXML)
{
	int indexBG = -1, indexFG = -1;
	vector<string> vsTag = { "colour", "solid", "item_default" };
	vector<vector<string>> vvsTag = jf.getXML(configXML, vsTag);
	for (int ii = 0; ii < vvsTag.size(); ii++) {
		if (vvsTag[ii][0] == "background") { indexBG = ii; }
		else if (vvsTag[ii][0] == "foreground") { indexFG = ii; }
	}
	qjTable->itemColourDefault = make_pair(vvsTag[indexBG][1], vvsTag[indexFG][1]);

	vsTag = { "colour", "solid", "item_fail" };
	vvsTag = jf.getXML(configXML, vsTag);
	for (int ii = 0; ii < vvsTag.size(); ii++) {
		if (vvsTag[ii][0] == "background") { indexBG = ii; }
		else if (vvsTag[ii][0] == "foreground") { indexFG = ii; }
	}
	qjTable->itemColourFail = make_pair(vvsTag[indexBG][1], vvsTag[indexFG][1]);

	vsTag = { "colour", "solid", "item_selected" };
	vvsTag = jf.getXML(configXML, vsTag);
	for (int ii = 0; ii < vvsTag.size(); ii++) {
		if (vvsTag[ii][0] == "background") { indexBG = ii; }
		else if (vvsTag[ii][0] == "foreground") { indexFG = ii; }
	}
	qjTable->itemColourSelected = make_pair(vvsTag[indexBG][1], vvsTag[indexFG][1]);
	qjTable->itemColourHover = qjTable->itemColourSelected;

	vsTag = { "colour", "solid", "item_warning" };
	vvsTag = jf.getXML(configXML, vsTag);
	for (int ii = 0; ii < vvsTag.size(); ii++) {
		if (vvsTag[ii][0] == "background") { indexBG = ii; }
		else if (vvsTag[ii][0] == "foreground") { indexFG = ii; }
	}
	qjTable->itemColourWarning = make_pair(vvsTag[indexBG][1], vvsTag[indexFG][1]);
}

