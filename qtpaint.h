#pragma once
#include "qtfunc.h"
#include "jmap.h"

class QTPAINT : public QWidget
{
	Q_OBJECT

private:
	JFUNC jf;
	vector<vector<double>> keyColour, keyColourBand;
	vector<double> keyColourExtra;
	unordered_map<string, int> mapDotIndex;
	QPointF parentPositionKM;  // Relative to "Home".
	QColor qBG;
	QTFUNC qf;
	double widgetPPKM = -1.0;
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
	int selectedArea = -1, selectedDot = -1;

	void addArea(vector<vector<double>>& border);
	void addArea(vector<QPointF>& border);
	void addAreaColour(vector<int> rgbx);
	void addAreaColour(vector<double> rgbx);
	void addChild(vector<QPointF>& border, double scale, QPointF position, vector<POINT> frameTLBR);
	void addParent(vector<QPointF>& border, double scale, QPointF position, vector<POINT> frameTLBR);
	void addParent(vector<QPointF>& border, double scale, QPointF position, vector<POINT> frameTLBR, vector<int> rgbaBG);
	void areaColourFillSpectrum();
	void clear();
	void displaceChildToParent(vector<QPointF>& vQPF, QPointF disp);
	void drawAreas();
	void drawSelectedDot(string regionName);
	QColor getColourFromSpectrum(double zeroOne);
	vector<QPointF> getTLBR(vector<QPointF>& vQPF);  // Form [top, left, bot, right].
	void initialize();
	void paintArea(QPainter& painter);
	void paintArea(QPainter& painter, vector<int>& viArea);
	void scaleChildToWidget(vector<QPointF>& vQPF, double PPKM);
	void scaleParentToWidget(vector<QPointF>& vQPF, double PPKM, vector<POINT> frameTLBR);
	void scaleToWidget(vector<QPointF>& vQPF);

protected:
	void paintEvent(QPaintEvent* event) override;
};

