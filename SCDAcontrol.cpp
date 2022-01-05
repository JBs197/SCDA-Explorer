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

	indexDebug = 2;
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
