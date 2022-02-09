#include "SCDAmap.h"

using namespace std;

void SCDAmap::err(string message)
{
	string errorMessage = "SCDAmap error:\n" + message;
	JLOG::getInstance()->err(errorMessage);
}
void SCDAmap::init()
{
	QGridLayout* gLayout = new QGridLayout;
	this->setLayout(gLayout);

	vector<string> vsHeader = { "" };
	modelCataMap = make_shared<QJTREEMODEL>(vsHeader, this);
	modelCataMap->jtx.setNodeColour(-1, itemColourDefault, itemColourSelected);

	QLabel* qlCataTree = new QLabel("Map Tree Per Catalogue");
	gLayout->addWidget(qlCataTree, 0, index::Tree);
	
	QJTREEVIEW* qjTree = new QJTREEVIEW;
	qjTree->indexTree = index::Tree;
	gLayout->addWidget(qjTree, 1, index::Tree);
	connect(qjTree, &QJTREEVIEW::nodeRightClicked, this, &SCDAmap::nodeRightClicked);

	QLabel* qlMap = new QLabel("Geographic Region Map");
	gLayout->addWidget(qlMap, 0, index::Map);

	QJPAINT* qjPaint = new QJPAINT;
	gLayout->addWidget(qjPaint, 1, index::Map);

	initAction();
}
void SCDAmap::initAction()
{
	qaLoadTree = make_shared<QAction>("Load Geo Tree", this);
	auto loadTree = qaLoadTree.get();
	connect(loadTree, &QAction::triggered, this, [&]() {
		QVariant qVar = qaLoadTree->data();
		QString qsTemp = qVar.toString();
		string sData = qsTemp.toStdString();
		vector<string> vsPrompt;
		jstr.splitByMarker(vsPrompt, sData);
		emit sendLoadGeoTree(vsPrompt[0], vsPrompt[1]);
		});
}
void SCDAmap::nodeRightClicked(const QPoint& globalPos, const QModelIndex& qmIndex, int indexTree)
{
	QMenu menu(this);
	QJTREEMODEL* qjtm = modelCataMap.get();
	vector<string> vsGenealogy = qjtm->getGenealogy(qmIndex);
	if (vsGenealogy.size() < 2) { return; }
	string sData = "";
	for (int ii = 0; ii < vsGenealogy.size(); ii++) {
		sData += "$" + vsGenealogy[ii];
	}
	auto loadTree = qaLoadTree.get();
	loadTree->setData(sData.c_str());
	menu.addAction(loadTree);

	menu.exec(globalPos);
}
void SCDAmap::shadeToBorder(string filePath, vector<unsigned char> rgbxBG)
{
	// For a given shaded map region and background, save a copy of that map as a border only.
	size_t pos1 = filePath.rfind(".png");
	if (pos1 > filePath.size() || !jfile.exist(filePath)) { err("Invalid input file-shadeToBorder"); }
	string outputPath = filePath;
	outputPath.insert(pos1, "[border]");

	vector<unsigned char> data;
	vector<int> spec;
	pngf.load(filePath, data, spec);

	vector<unsigned char> colour;
	pair<int, int> TL = make_pair(0, 0), BR = make_pair(spec[0] - 1, spec[1] - 1);
	set<pair<int, int>> solid;
	pngf.scanRect(solid, data, spec, rgbxBG, TL, BR, -1);

	vector<pair<int, int>> outline, segment;
	jpixel.solidToOutline(outline, solid);
	pngf.getPixel(colour, outline[0], data, spec);
	pngf.blankCanvas(data, spec, rgbxBG);
	pngf.paintPixel(outline, data, spec, colour);
	pngf.print(outputPath, data, spec);
}
