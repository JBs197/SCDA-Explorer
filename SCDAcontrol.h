#pragma once
#include <QComboBox>
#include <QFrame>
#include <QLabel>
#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>
#include "jfunc.h"

using namespace std;

class SCDAcontrol : public QFrame
{
	Q_OBJECT

private:
	JFUNC jf;

	void err(string message);
	void init();

public:
	SCDAcontrol() { init(); }
	~SCDAcontrol() {}

	int indexDBTable, indexDebug, indexDrive, indexFetch, indexText;

signals:
	void driveSelected(string drive);
	void sendDebug();
	void sendOnlineCata();
	void sendSearchDBTable(string sQuery);

public slots:
	void driveChanged(const QString& qsDrive);
	void prepSearchDBTable();
	void textOutput(string sMessage);
};