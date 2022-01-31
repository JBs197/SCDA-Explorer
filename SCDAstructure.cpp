#include "SCDAstructure.h"

using namespace std;

void SCDAstructure::err(string message)
{
	string errorMessage = "SCDAstructure error:\n" + message;
	JLOG::getInstance()->err(errorMessage);
}
void SCDAstructure::init()
{
	vector<string> vsHeader = { "Tag Name", "Attr. 1", "Attr. 2" };

	QGridLayout* gLayout = new QGridLayout;
	this->setLayout(gLayout);

	modelStructure = make_shared<QJTREEMODEL>(vsHeader, this);
	modelStructure->jtx.setNodeColour(-1, itemColourDefault, itemColourSelected);
	
	QLabel* qlStructure = new QLabel("Catalogue Structure");
	gLayout->addWidget(qlStructure, 0, index::Tree);
	QJTREEVIEW* treeStructure = new QJTREEVIEW;
	treeStructure->indexTree = index::Tree;
	gLayout->addWidget(treeStructure, 1, index::Tree);
	connect(treeStructure, &QJTREEVIEW::nodeRightClicked, this, &SCDAstructure::nodeRightClicked);

	QLabel* qlSettings = new QLabel("Catalogue Template Settings");
	gLayout->addWidget(qlSettings, 0, index::Settings);
	QVBoxLayout* vLayout = new QVBoxLayout;
	gLayout->addLayout(vLayout, 1, index::Settings);
	
	QLabel* qlForWhom = new QLabel("ForWhom: [None]");
	qlForWhom->setWordWrap(1);
	qlForWhom->setFrameStyle(QFrame::Panel);
	qlForWhom->setMaximumWidth(400);
	qlForWhom->setObjectName("ForWhom");
	vLayout->addWidget(qlForWhom, 0);

	QLabel* qlGeoCode = new QLabel("GeoCode: [None]");
	qlGeoCode->setWordWrap(1);
	qlGeoCode->setFrameStyle(QFrame::Panel);
	qlGeoCode->setMaximumWidth(400);
	qlGeoCode->setObjectName("GeoCode");
	vLayout->addWidget(qlGeoCode, 0);

	QLabel* qlGeoRegionName = new QLabel("GeoRegionName: [None]");
	qlGeoRegionName->setWordWrap(1);
	qlGeoRegionName->setFrameStyle(QFrame::Panel);
	qlGeoRegionName->setMaximumWidth(400);
	qlGeoRegionName->setObjectName("GeoRegionName");
	vLayout->addWidget(qlGeoRegionName, 0);

	QLabel* qlGeoLayer = new QLabel("GeoLayer: [None]");
	qlGeoLayer->setWordWrap(1);
	qlGeoLayer->setFrameStyle(QFrame::Panel);
	qlGeoLayer->setMaximumWidth(400);
	qlGeoLayer->setObjectName("GeoLayer");
	vLayout->addWidget(qlGeoLayer, 0);

	QLabel* qlParameterTitle = new QLabel("ParameterTitle: [None]");
	qlParameterTitle->setWordWrap(1);
	qlParameterTitle->setFrameStyle(QFrame::Panel);
	qlParameterTitle->setMaximumWidth(400);
	qlParameterTitle->setObjectName("ParameterTitle");
	vLayout->addWidget(qlParameterTitle, 0);

	QLabel* qlMemberNumber = new QLabel("MemberNumber: [None]");
	qlMemberNumber->setWordWrap(1);
	qlMemberNumber->setFrameStyle(QFrame::Panel);
	qlMemberNumber->setMaximumWidth(400);
	qlMemberNumber->setObjectName("MemberNumber");
	vLayout->addWidget(qlMemberNumber, 0);

	QLabel* qlMemberDescription = new QLabel("MemberDescription: [None]");
	qlMemberDescription->setWordWrap(1);
	qlMemberDescription->setFrameStyle(QFrame::Panel);
	qlMemberDescription->setMaximumWidth(400);
	qlMemberDescription->setObjectName("MemberDescription");
	vLayout->addWidget(qlMemberDescription, 0);

	vLayout->addStretch(1);

	initAction();
}
void SCDAstructure::initAction()
{
	qaSaveForWhom = make_shared<QAction>("Save as ForWhom", this);
	auto saveForWhom = qaSaveForWhom.get();
	connect(saveForWhom, &QAction::triggered, this, [&]() {
		QVariant qVar = qaSaveForWhom->data();
		QString qsTemp = qVar.toString();
		wstring wsTemp = qsTemp.toStdWString();
		string sData;
		jstr.utf16To8(sData, wsTemp);
		saveSetting("ForWhom", sData);
		});

	qaSaveGeoCode = make_shared<QAction>("Save as GeoCode", this);
	auto saveGeoCode = qaSaveGeoCode.get();
	connect(saveGeoCode, &QAction::triggered, this, [&]() {
		QVariant qVar = qaSaveGeoCode->data();
		QString qsTemp = qVar.toString();
		wstring wsTemp = qsTemp.toStdWString();
		string sData;
		jstr.utf16To8(sData, wsTemp);
		saveSetting("GeoCode", sData);
		});

	qaSaveGeoLayer = make_shared<QAction>("Save as GeoLayer", this);
	auto saveGeoLayer = qaSaveGeoLayer.get();
	connect(saveGeoLayer, &QAction::triggered, this, [&]() {
		QVariant qVar = qaSaveGeoLayer->data();
		QString qsTemp = qVar.toString();
		wstring wsTemp = qsTemp.toStdWString();
		string sData;
		jstr.utf16To8(sData, wsTemp);
		saveSetting("GeoLayer", sData);
		});

	qaSaveGeoRegionName = make_shared<QAction>("Save as GeoRegionName", this);
	auto saveGeoRegionName = qaSaveGeoRegionName.get();
	connect(saveGeoRegionName, &QAction::triggered, this, [&]() {
		QVariant qVar = qaSaveGeoRegionName->data();
		QString qsTemp = qVar.toString();
		wstring wsTemp = qsTemp.toStdWString();
		string sData;
		jstr.utf16To8(sData, wsTemp);
		saveSetting("GeoRegionName", sData);
		});

	qaSaveParameterTitle = make_shared<QAction>("Save as ParameterTitle", this);
	auto saveParameterTitle = qaSaveParameterTitle.get();
	connect(saveParameterTitle, &QAction::triggered, this, [&]() {
		QVariant qVar = qaSaveParameterTitle->data();
		QString qsTemp = qVar.toString();
		wstring wsTemp = qsTemp.toStdWString();
		string sData;
		jstr.utf16To8(sData, wsTemp);
		saveSetting("ParameterTitle", sData);
		});

	qaSaveMemberNumber = make_shared<QAction>("Save as MemberNumber", this);
	auto saveMemberNumber = qaSaveMemberNumber.get();
	connect(saveMemberNumber, &QAction::triggered, this, [&]() {
		QVariant qVar = qaSaveMemberNumber->data();
		QString qsTemp = qVar.toString();
		wstring wsTemp = qsTemp.toStdWString();
		string sData;
		jstr.utf16To8(sData, wsTemp);
		saveSetting("MemberNumber", sData);
		});

	qaSaveMemberDescription = make_shared<QAction>("Save as MemberDescription", this);
	auto saveMemberDescription = qaSaveMemberDescription.get();
	connect(saveMemberDescription, &QAction::triggered, this, [&]() {
		QVariant qVar = qaSaveMemberDescription->data();
		QString qsTemp = qVar.toString();
		wstring wsTemp = qsTemp.toStdWString();
		string sData;
		jstr.utf16To8(sData, wsTemp);
		saveSetting("MemberDescription", sData);
		});

}
void SCDAstructure::loadXML(string xmlPath)
{
	QJTREEMODEL* model = modelStructure.get();
	model->jtx.loadXML(xmlPath);
	model->jtx.setExpandGeneration(30);
	model->populate(model->tree::jtxml);
	int numGen = model->jtx.getExpandGeneration(30);
	int numCol = model->columnCount(QModelIndex());

	QGridLayout* gLayout = (QGridLayout*)this->layout();
	QLayoutItem* qlItem = gLayout->itemAtPosition(1, 0);
	QJTREEVIEW* qjTree = (QJTREEVIEW*)qlItem->widget();
	qjTree->setModel(model);
	qjTree->expandRecursively(QModelIndex(), numGen);
	for (int ii = 0; ii < numCol; ii++) {
		qjTree->resizeColumnToContents(ii);
	}
}
void SCDAstructure::nodeRightClicked(const QPoint& globalPos, const QModelIndex& qmIndex, int indexTree)
{
	QMenu menu(this);
	QJTREEMODEL* qjtm = modelStructure.get();
	vector<string> vsGenealogy = qjtm->getGenealogy(qmIndex);
	string sData = "";
	for (int ii = 0; ii < vsGenealogy.size(); ii++) {
		sData += "$" + vsGenealogy[ii];
	}
	auto saveForWhom = qaSaveForWhom.get();
	saveForWhom->setData(sData.c_str());
	menu.addAction(saveForWhom);
	auto saveGeoCode = qaSaveGeoCode.get();
	saveGeoCode->setData(sData.c_str());
	menu.addAction(saveGeoCode);
	auto saveGeoRegionName = qaSaveGeoRegionName.get();
	saveGeoRegionName->setData(sData.c_str());
	menu.addAction(saveGeoRegionName);
	auto saveGeoLayer = qaSaveGeoLayer.get();
	saveGeoLayer->setData(sData.c_str());
	menu.addAction(saveGeoLayer);
	auto saveParameterTitle = qaSaveParameterTitle.get();
	saveParameterTitle->setData(sData.c_str());
	menu.addAction(saveParameterTitle);
	auto saveMemberNumber = qaSaveMemberNumber.get();
	saveMemberNumber->setData(sData.c_str());
	menu.addAction(saveMemberNumber);
	auto saveMemberDescription = qaSaveMemberDescription.get();
	saveMemberDescription->setData(sData.c_str());
	menu.addAction(saveMemberDescription);
	
	menu.exec(globalPos);
}
void SCDAstructure::saveSetting(string sObjectName, string sData)
{
	string sMarker(1, sData[0]);
	sData.erase(sData.begin());
	vector<string> dirt = { sMarker }, soap = { "->" };
	jstr.clean(sData, dirt, soap);
	QLabel* label = findChild<QLabel*>(sObjectName.c_str());
	sObjectName += ":\n" + sData;
	label->setText(sObjectName.c_str());
}
