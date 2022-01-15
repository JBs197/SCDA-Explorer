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

	enum index{ Drive, Fetch, DBTable, Debug, Spacing0, Clear, Text };

	string sLastQuery;

signals:
	void driveSelected(string drive);
	void sendDebug();
	void sendOnlineCata();
	void sendSearchDBTable(string sQuery);

public slots:
	void clearText();
	void driveChanged(const QString& qsDrive);
	void prepSearchDBTable();
	void textAppend(string sMessage);
	void textOutput(string sMessage);
};