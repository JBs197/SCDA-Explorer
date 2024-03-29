#include "SCDAcontrol.h"

using namespace std;

void SCDAcontrol::clearText()
{
	QVBoxLayout* vLayout = (QVBoxLayout*)this->layout();
	QLayoutItem* qlItem = vLayout->itemAt(index::Text);
	QTextEdit* teIO = (QTextEdit*)qlItem->widget();
	teIO->clear();
	teIO->setFocus();
}
void SCDAcontrol::driveChanged(const QString& qsDrive)
{
	wstring wsTemp = qsDrive.toStdWString();
	string drive;
	jstr.utf16To8(drive, wsTemp);
	emit driveSelected(drive);
}
void SCDAcontrol::err(string message)
{
	string errorMessage = "SCDAcontrol error:\n" + message;
	JLOG::getInstance()->err(errorMessage);
}
string SCDAcontrol::getDrive()
{
	QVBoxLayout* vLayout = (QVBoxLayout*)this->layout();
	QLayoutItem* qlItem = vLayout->itemAt(index::Drive);
	QHBoxLayout* hLayout = (QHBoxLayout*)qlItem->layout();
	int numItem = hLayout->count();
	qlItem = hLayout->itemAt(numItem - 1);
	QComboBox* cb = (QComboBox*)qlItem->widget();
	QString qsDrive = cb->currentText();
	return qsDrive.toStdString();
}
void SCDAcontrol::init()
{
	sLastQuery = "";

	QVBoxLayout* vLayout = new QVBoxLayout;
	this->setLayout(vLayout);

	QHBoxLayout* driveLayout = new QHBoxLayout;
	vLayout->insertLayout(index::Drive, driveLayout, 0);
	QLabel* labelDrive = new QLabel("Local Drive:");
	driveLayout->addWidget(labelDrive);
	driveLayout->addStretch(1);
	QComboBox* cb = new QComboBox;
	driveLayout->addWidget(cb);
	connect(cb, &QComboBox::currentTextChanged, this, &SCDAcontrol::driveChanged);

	QPushButton* pbFetch = new QPushButton("Fetch Online\nCatalogues");
	vLayout->insertWidget(index::Fetch, pbFetch, 0);
	connect(pbFetch, &QPushButton::clicked, this, &SCDAcontrol::sendOnlineCata);

	QPushButton* pbDBTable = new QPushButton("Search\nDatabase Tables");
	vLayout->insertWidget(index::DBTable, pbDBTable, 0);
	connect(pbDBTable, &QPushButton::clicked, this, &SCDAcontrol::prepSearchDBTable);

	QPushButton* pbStructure = new QPushButton("Load Catalogue\nStructure File");
	vLayout->insertWidget(index::Structure, pbStructure, 0);
	connect(pbStructure, &QPushButton::clicked, this, &SCDAcontrol::sendStructure);

	QPushButton* pbDebug = new QPushButton("Debug");
	vLayout->insertWidget(index::Debug, pbDebug, 0);
	connect(pbDebug, &QPushButton::clicked, this, &SCDAcontrol::sendDebug);

	vLayout->insertSpacing(index::Spacing0, 20);

	QHBoxLayout* clearLayout = new QHBoxLayout;
	QLabel* labelText = new QLabel("I/O Text Box");
	clearLayout->addWidget(labelText, 0);
	clearLayout->addStretch(1);
	QPushButton* pbClear = new QPushButton("Clear");
	clearLayout->addWidget(pbClear);
	vLayout->insertLayout(index::Clear, clearLayout, 0);
	connect(pbClear, &QPushButton::clicked, this, &SCDAcontrol::clearText);

	QTextEdit* teIO = new QTextEdit;
	vLayout->insertWidget(index::Text, teIO, 1);
}
void SCDAcontrol::prepSearchDBTable()
{
	QVBoxLayout* vLayout = (QVBoxLayout*)this->layout();
	QLayoutItem* qlItem = vLayout->itemAt(index::Text);
	QTextEdit* teIO = (QTextEdit*)qlItem->widget();
	QString qsTemp = teIO->toPlainText();
	wstring wsTemp = qsTemp.toStdWString();
	string sQuery;
	jstr.utf16To8(sQuery, wsTemp);
	if (sQuery.size() < 1) { return; }

	vector<string> dirt = { " ", "\r", "\n" }, soap = { "", "", "" };
	jstr.clean(sQuery, dirt, soap);
	emit sendSearchDBTable(sQuery);
	sLastQuery = sQuery;

	string sMessage = sQuery + "\nSearching for ";
	if (sQuery == "" || sQuery == "*") { sMessage += "all tables"; }
	else { sMessage += sQuery; }
	sMessage += " ...";
	teIO->setText(sMessage.c_str());
}
void SCDAcontrol::textAppend(string sMessage)
{
	QVBoxLayout* vLayout = (QVBoxLayout*)this->layout();
	QLayoutItem* qlItem = vLayout->itemAt(index::Text);
	QTextEdit* teIO = (QTextEdit*)qlItem->widget();
	QString qsMessage = teIO->toPlainText();
	qsMessage.append(sMessage.c_str());
	teIO->setText(qsMessage);
	teIO->update();

	auto vScroll = teIO->verticalScrollBar();
	if (vScroll != nullptr) {
		vScroll->setValue(vScroll->maximum());
	}
}
void SCDAcontrol::textOutput(string sMessage)
{
	QVBoxLayout* vLayout = (QVBoxLayout*)this->layout();
	QLayoutItem* qlItem = vLayout->itemAt(index::Text);
	QTextEdit* teIO = (QTextEdit*)qlItem->widget();
	teIO->setText(sMessage.c_str());
}
