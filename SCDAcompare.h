#pragma once
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include "jstring.h"

class SCDAcompare : public QWidget
{
	Q_OBJECT

private:
	JSTRING jstr;

	void err(std::string message);

public:
	SCDAcompare(QWidget* parent = nullptr);
	~SCDAcompare() = default;

public slots:
	void addText();
};
