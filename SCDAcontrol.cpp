#include "SCDAcontrol.h"

void SCDAcontrol::driveChanged(const QString& qsDrive)
{
	string drive = qsDrive.toUtf8();
	emit driveSelected(drive);
}
void SCDAcontrol::err(string message)
{
	string errorMessage = "SCDAcontrol error:\n" + message;
	JLOG::getInstance()->err(errorMessage);
}
void SCDAcontrol::init()
{
	QVBoxLayout* vLayout = new QVBoxLayout;
	this->setLayout(vLayout);

	indexDrive = 0;
	QHBoxLayout* driveLayout = new QHBoxLayout;
	vLayout->insertLayout(indexDrive, driveLayout, 0);
	QLabel* labelDrive = new QLabel("Local Drive:");
	driveLayout->addWidget(labelDrive);
	driveLayout->addStretch(1);
	QComboBox* cb = new QComboBox;
	driveLayout->addWidget(cb);
	connect(cb, &QComboBox::currentTextChanged, this, &SCDAcontrol::driveChanged);

	indexFetch = 1;
	QPushButton* pbFetch = new QPushButton("Fetch Online\nCatalogues");
	vLayout->insertWidget(indexFetch, pbFetch, 0);
	connect(pbFetch, &QPushButton::clicked, this, &SCDAcontrol::sendOnlineCata);

	indexDBTable = 2;
	QPushButton* pbDBTable = new QPushButton("Search\nDatabase Tables");
	vLayout->insertWidget(indexDBTable, pbDBTable, 0);
	connect(pbDBTable, &QPushButton::clicked, this, &SCDAcontrol::prepSearchDBTable);

	indexDebug = 3;
	QPushButton* pbDebug = new QPushButton("Debug");
	vLayout->insertWidget(indexDebug, pbDebug, 0);
	connect(pbDebug, &QPushButton::clicked, this, &SCDAcontrol::sendDebug);

	vLayout->addSpacing(20);
	QLabel* labelText = new QLabel("I/O Text Box");
	vLayout->addWidget(labelText, 0, Qt::AlignLeft);

	indexText = 5;
	QTextEdit* teIO = new QTextEdit;
	vLayout->insertWidget(indexText, teIO, 1);
}
void SCDAcontrol::prepSearchDBTable()
{
	QVBoxLayout* vLayout = (QVBoxLayout*)this->layout();
	QLayoutItem* qlItem = vLayout->itemAt(indexText);
	QTextEdit* teIO = (QTextEdit*)qlItem->widget();
	QString qsTemp = teIO->toPlainText();
	string sQuery = qsTemp.toUtf8();

	vector<string> dirt = { " ", "\r", "\n" }, soap = { "", "", "" };
	jf.clean(sQuery, dirt, soap);
	emit sendSearchDBTable(sQuery);
}
void SCDAcontrol::textOutput(string sMessage)
{
	QVBoxLayout* vLayout = (QVBoxLayout*)this->layout();
	QLayoutItem* qlItem = vLayout->itemAt(indexText);
	QTextEdit* teIO = (QTextEdit*)qlItem->widget();
	teIO->setText(sMessage.c_str());
}
