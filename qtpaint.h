#pragma once
#include "qtfunc.h"
#include "jmap.h"

class QTPAINT : public QWidget
{
	Q_OBJECT

private:
	JFUNC jf;
	vector<QColor> keyColour;
	QColor keyColourExtra;
	unordered_map<string, int> mapDotIndex;
	vector<double> parentFrameTLKM;  // Relative to "Home".
	QColor qBG;
	QTFUNC qf;
	double widgetPPKM = -1.0;

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
	void addAreaColour(QColor& qColour);
	void addAreaColour(vector<int> rgbx);
	void addAreaColour(vector<double> rgbx);
	void addChild(double scale, vector<vector<double>> frameTLBR, vector<vector<double>>& border);
	void addParent(double scale, vector<vector<double>> frameTLBR, vector<vector<double>>& border);
	void addParentBG(double scale, vector<vector<double>> frameTLBR, vector<vector<double>>& border, vector<unsigned char> rgbxBG);
	void areaColourFillSpectrum();
	void clear();
	void displaceChildToParent(vector<double> dispParentTL, vector<double> TL, vector<vector<double>>& border);
	void displaceParentToWidget(vector<vector<double>>& border);
	void drawAreas();
	void drawSelectedDot(string regionName);
	QColor getColourFromSpectrum(double zeroOne);
	vector<QPointF> getTLBR(vector<QPointF>& vQPF);  // Form [top, left, bot, right].
	void initialize();
	void paintArea(QPainter& painter);
	void paintArea(QPainter& painter, vector<int>& viArea);
	void scaleChildToWidget(vector<vector<double>>& frameTLBR, vector<vector<double>>& border);
	void scaleParentToWidget(vector<vector<double>>& border, double PPKM, vector<vector<double>> frameTLBR);

protected:
	void paintEvent(QPaintEvent* event) override;
};

