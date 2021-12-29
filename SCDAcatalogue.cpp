#include "SCDAcatalogue.h"

void SCDAcatalogue::err(string message)
{
	string errorMessage = "SCDAcatalogue error:\n" + message;
	JLOG::getInstance()->err(errorMessage);
}
void SCDAcatalogue::init()
{
	vector<vector<string>> vvsCol = { {"sValue", ""} };
	QGridLayout* gLayout = new QGridLayout;
	this->setLayout(gLayout);

	indexStatscan = 0;
	QLabel* label = new QLabel("Online Catalogues");
	gLayout->addWidget(label, 0, indexStatscan);
	QJTREE* treeStatscan = new QJTREE;
	gLayout->addWidget(treeStatscan, 1, indexStatscan);

	indexLocal = 1;
	label = new QLabel("Local Catalogues");
	gLayout->addWidget(label, 0, indexLocal);
	QJTREE* treeLocal = new QJTREE;
	treeLocal->initCol(vvsCol);
	gLayout->addWidget(treeLocal, 1, indexLocal);

	indexDatabase = 2;
	label = new QLabel("Database Catalogues");
	gLayout->addWidget(label, 0, indexDatabase);
	QJTREE* treeDatabase = new QJTREE;
	treeDatabase->initCol(vvsCol);
	gLayout->addWidget(treeDatabase, 1, indexDatabase);
}
