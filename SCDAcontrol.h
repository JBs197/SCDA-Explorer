#pragma once
#include <QComboBox>
#include <QFrame>
#include <QLabel>
#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>
#include "jfunc.h"

class SCDAcontrol : public QFrame
{
	Q_OBJECT

private:
	JFUNC jf;

	void err(std::string message);
	void init();

public:
	SCDAcontrol() { init(); }
	~SCDAcontrol() {}

	enum index{ Drive, Fetch, DBTable, Structure, Debug, Spacing0, Clear, Text };

	std::string sLastQuery;

signals:
	void driveSelected(std::string drive);
	void sendDebug();
	void sendOnlineCata();
	void sendSearchDBTable(std::string sQuery);
	void sendStructure();

public slots:
	void clearText();
	void driveChanged(const QString& qsDrive);
	void prepSearchDBTable();
	void textAppend(std::string sMessage);
	void textOutput(std::string sMessage);
};