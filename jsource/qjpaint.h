#pragma once
#include <Array>
#include <QPainter>
#include <QPainterPath>
#include <QWidget>
#include "jlog.h"
#include "qjshape.h"

class QJPAINT : public QWidget
{
	Q_OBJECT

private:
	std::array<unsigned char, 4> bg;  // R, G, B, A
	QList<QJSHAPE*> qlShape;

	void drawBackground(QPainter* painter);
	void err(std::string message);

public:
	QJPAINT(QWidget* parent = nullptr);
	~QJPAINT();

	void clearShape();
	QJSHAPE* getShape(int index);
	QJSHAPE* insertShape(int index = -1);
	void setBG(std::vector<unsigned char> rgbx);

protected:
	void paintEvent(QPaintEvent* event) override;
};

