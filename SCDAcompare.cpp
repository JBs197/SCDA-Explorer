#include "SCDAcompare.h"

using namespace std;

SCDAcompare::SCDAcompare(QWidget* parent) : QWidget(parent) 
{
	QVBoxLayout* vLayout = new QVBoxLayout;
	this->setLayout(vLayout);

	QHBoxLayout* hLayout = new QHBoxLayout;
	vLayout->addLayout(hLayout);
	QLabel* labelMarker = new QLabel("Marker chars: ");
	hLayout->addWidget(labelMarker, 0, Qt::AlignCenter);
	QLineEdit* leMarker = new QLineEdit;
	hLayout->addWidget(leMarker, 0, Qt::AlignCenter);
	hLayout->addSpacing(20);
	QLabel* labelText = new QLabel("Text: ");
	hLayout->addWidget(labelText, 0, Qt::AlignCenter);
	QLineEdit* leText = new QLineEdit;
	hLayout->addWidget(leText, 1, Qt::AlignCenter);
	QPushButton* pbAdd = new QPushButton("Add");
	hLayout->addWidget(pbAdd, 0, Qt::AlignCenter);
	connect(pbAdd, &QPushButton::clicked, this, &SCDAcompare::addText);

	QGridLayout* gLayout = new QGridLayout;
	vLayout->addLayout(gLayout, 10);
	vLayout->addStretch(1);
}

void SCDAcompare::addText()
{
	QVBoxLayout* vLayout = (QVBoxLayout*)this->layout();
	QLayoutItem* qlItem = vLayout->itemAt(0);
	QHBoxLayout* hLayout = (QHBoxLayout*)qlItem->layout();
	qlItem = hLayout->itemAt(1);
	QLineEdit* leMarker = (QLineEdit*)qlItem->widget();
	QString qsMarker = leMarker->text();
	string marker = qsMarker.toStdString();
	
	qlItem = hLayout->itemAt(4);
	QLineEdit* leText = (QLineEdit*)qlItem->widget();
	QString qsText = leText->text();
	string text = qsText.toStdString();	
	
	qlItem = vLayout->itemAt(1);
	QGridLayout* gLayout = (QGridLayout*)qlItem->layout();
	int iCol = gLayout->columnCount();
	QPushButton* pbRemove = new QPushButton("Remove");
	pbRemove->setObjectName(QString::number(iCol));
	gLayout->addWidget(pbRemove, 0, iCol, Qt::AlignCenter);
	connect(pbRemove, &QPushButton::clicked, [pbRemove]() {
		bool success;
		QString qsName = pbRemove->objectName();
		int col = qsName.toInt(&success);
		if (!success) {//}
		});
	
	vector<string> vsText;
	jstr.splitByMarker(vsText, text, marker);	



}
void SCDAcompare::err(string message)
{
	string errorMessage = "SCDAcompare error:\n" + message;
	JLOG::getInstance()->err(errorMessage);
}
