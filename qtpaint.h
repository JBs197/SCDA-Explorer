#pragma once
#include "qtfunc.h"
#include "binmap.h"

class QTPAINT : public QWidget
{
	Q_OBJECT

private:
	JFUNC jf;
	unordered_map<string, int> mapDotIndex;
	QTFUNC qf;
	double Width, Height;

public:
	explicit QTPAINT(QWidget* qwParent) : QWidget(qwParent) {}
	~QTPAINT() {}

	vector<QColor> areaColour;
	vector<vector<QPointF>> areas;
	bool debug = 0;
	double defaultDotWidth = 5.0, defaultMargin = 20.0;  // pixels
	vector<QPointF> dots, crosshairs;
	vector<vector<double>> parentTLBR;
	QColor qcBlue = QColor(0, 112, 255);
	int selectedArea = -1, selectedDot = -1;

	void addArea(vector<vector<double>>& border);
	void addAreaColour(vector<int> rgb);
	void drawFamilyBlue(vector<BINMAP>& binFamily);
	void drawSelectedDot(string regionName);
	vector<QPointF> getTLBR(vector<QPointF>& vQPF);  // Form [top, left, bot, right].
	void initialize();

protected:
	void paintEvent(QPaintEvent* event) override;
};

