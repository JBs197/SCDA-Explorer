#pragma once
#include "qtfunc.h"
#include "binmap.h"

class QTPAINT : public QWidget
{
	Q_OBJECT

private:
	JFUNC jf;
	const double height, width;

public:
	explicit QTPAINT(double wD, double hD) : width(wD), height(hD) {}
	~QTPAINT() {}

	vector<QColor> areaColour;
	vector<vector<QPointF>> areas;
	double defaultMargin = 20.0;  // pixels
	//QColor qcBlack = QColor(Qt::black);
	int selectedArea;
	vector<double> windowDim;

	void addArea(vector<vector<double>>& border);
	void addAreaColour(vector<int> rgb);
	void drawFamily(vector<BINMAP>& binFamily, int selected);

protected:
	void paintEvent(QPaintEvent* event) override;
};

