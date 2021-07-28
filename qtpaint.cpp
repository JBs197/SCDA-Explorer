#include "stdafx.h"
#include "qtpaint.h"

void QTPAINT::addArea(vector<vector<double>>& border)
{
	double dWidth = (double)this->width();
	double dHeight = (double)this->height();
	int areaIndex = areas.size();
	areas.push_back(vector<QPointF>(border.size()));
	for (int ii = 0; ii < border.size(); ii++)
	{
		areas[areaIndex][ii].setX(border[ii][0]);
		areas[areaIndex][ii].setY(border[ii][1]);
	}
}
void QTPAINT::addArea(vector<QPointF>& border)
{
	areas.push_back(border);
}
void QTPAINT::addAreaColour(QColor& qColour)
{
	areaColour.push_back(qColour);
}
void QTPAINT::addAreaColour(vector<int> rgbx)
{
	if (rgbx.size() < 3) { jf.err("Missing rgb-qp.addAreaColour"); }
	if (rgbx.size() == 3)
	{
		QColor qC(rgbx[0], rgbx[1], rgbx[2]);
		areaColour.push_back(qC);
	}
	else
	{
		QColor qC(rgbx[0], rgbx[1], rgbx[2], rgbx[3]);
		areaColour.push_back(qC);
	}
}
void QTPAINT::addAreaColour(vector<double> rgbx)
{
	if (rgbx.size() < 3) { jf.err("Missing rgb-qp.addAreaColour"); }
	if (rgbx.size() == 3)
	{
		QColor qC((int)rgbx[0], (int)rgbx[1], (int)rgbx[2]);
		areaColour.push_back(qC);
	}
	else
	{
		QColor qC((int)rgbx[0], (int)rgbx[1], (int)rgbx[2], (int)rgbx[3]);
		areaColour.push_back(qC);
	}
}
void QTPAINT::addChild(double scale, vector<vector<double>> frameTLBR, vector<vector<double>>& border)
{
	if (areas.size() < 1) { jf.err("No parent-qp.addChild"); }
	vector<double> dispParentTL(2);  // Child's TL widget pixel displacement from (0,0). 
	dispParentTL[0] = frameTLBR[0][0] - parentFrameTLKM[0];
	dispParentTL[1] = frameTLBR[0][1] - parentFrameTLKM[1];
	dispParentTL[0] *= widgetPPKM;
	dispParentTL[1] *= widgetPPKM;
	scaleChildToWidget(frameTLBR, border);
	displaceChildToParent(dispParentTL, frameTLBR[0], border);	
	addArea(border);
}
void QTPAINT::addChild(vector<vector<double>>& border, vector<vector<double>>& frame)
{
	if (areas.size() < 1) { jf.err("No parent-qp.addChild"); }
	vector<double> dispParentTL(2);  // Child's TL widget pixel displacement from (0,0). 
	dispParentTL[0] = frame[0][0] - parentFrameTLKM[0];
	dispParentTL[1] = frame[0][1] - parentFrameTLKM[1];
	dispParentTL[0] *= widgetPPKM;
	dispParentTL[1] *= widgetPPKM;
	scaleChildToWidget(frame, border);
	displaceChildToParent(dispParentTL, frame[0], border);
	addArea(border);
}
void QTPAINT::addParent(double scale, vector<vector<double>> frameTLBR, vector<vector<double>>& border)
{
	// Used when loading from bin files.
	clear();
	parentFrameTLKM = frameTLBR[0];
	displaceParentToWidget(border);
	scaleParentToWidget(border, scale, frameTLBR);
	addArea(border);
	addAreaColour(keyColourExtra);
}
void QTPAINT::addParent(vector<vector<double>>& border, vector<vector<double>>& frame)
{
	// Used when loading from the database.
	if (border.size() < 1 || frame.size() < 2) { jf.err("Missing parameters-qp.addParent"); }
	clear();
	parentFrameTLKM = frame[0];
	displaceParentToWidget(border);
	scaleParentToWidget(border, frame);
	addArea(border);
	QColor qBlack(Qt::black);
	addAreaColour(qBlack);
}
void QTPAINT::addParentBG(double scale, vector<vector<double>> frameTLBR, vector<vector<double>>& border, vector<unsigned char> rgbxBG)
{
	// Adds the parent region and a background colour for it. 
	if (rgbxBG.size() < 3) { jf.err("No BG colour given-qp.addParentBG"); }
	clear();
	parentFrameTLKM = frameTLBR[0];
	displaceParentToWidget(border);
	scaleParentToWidget(border, scale, frameTLBR);
	addArea(border);
	addAreaColour(keyColourExtra);
	qBG.setRgb((int)rgbxBG[0], (int)rgbxBG[1], (int)rgbxBG[2]);
}
void QTPAINT::areaColourFillSpectrum()
{
	QColor qColour;
	int numToFill = areas.size() - areaColour.size();
	int index = areaColour.size();
	areaColour.resize(areas.size());
	vector<double> vdColourPos;
	double colourInterval;
	if (numToFill == 0) { return; }
	else if (numToFill == 1) { vdColourPos = { 0.0 }; }
	else if (numToFill == 2) { vdColourPos = { 0.0, 1.0 }; }
	else if (numToFill > 2) 
	{ 
		colourInterval = 1.0 / (double)(numToFill - 1);
		vdColourPos.resize(numToFill);
		for (int ii = 0; ii < numToFill; ii++)
		{
			vdColourPos[ii] = (double)ii * colourInterval;
		}
	}
	else { jf.err("More colours than areas-qp.areaColourFillSpectrum"); }
	for (int ii = 0; ii < numToFill; ii++)
	{
		areaColour[index + ii] = getColourFromSpectrum(vdColourPos[ii]);
	}
}
void QTPAINT::clear()
{
	areas.clear();
	areaColour.clear();
}
void QTPAINT::drawAreas()
{
	areaColourFillSpectrum();
	update();
}
void QTPAINT::displaceChildToParent(vector<double> dispParentTL, vector<double> TL, vector<vector<double>>& border)
{
	vector<double> vdShift(2);
	vdShift[0] = dispParentTL[0] - TL[0];
	vdShift[1] = dispParentTL[1] - TL[1];
	for (int ii = 0; ii < border.size(); ii++)
	{
		border[ii][0] += vdShift[0];
		border[ii][1] += vdShift[1];
	}
}
void QTPAINT::displaceParentToWidget(vector<vector<double>>& border)
{
	for (int ii = 0; ii < border.size(); ii++)
	{
		border[ii][0] -= parentFrameTLKM[0];
		border[ii][1] -= parentFrameTLKM[1];
	}
}
void QTPAINT::drawSelectedDot(string regionName)
{
	if (dots.size() < 1) { return; }
	int index;
	try { index = mapDotIndex.at(regionName); }
	catch (out_of_range) { jf.err("Dot name not found-qp.drawSelectedDot"); }
	selectedDot = index;
	update();
}
QColor QTPAINT::getColourFromSpectrum(double zeroOne)
{
	// Return a QColor from a position [0.0, 1.0] within the full colour spectrum (red->violet).
	if (keyColour.size() < 1) { jf.err("No init-qp.getColourFromSpectrum"); }
	double myPos = zeroOne * (double)(keyColour.size() - 1);  // [0.0, 5.0]
	int iBase = (int)myPos;  // [0, 5]
	double remainder = myPos - (double)iBase;  // [0.0, 1.0)
	QColor qColour;
	double dR, dG, dB, dA;
	if (iBase < keyColour.size() - 1)
	{
		dR = remainder * (keyColour[iBase + 1].redF() - keyColour[iBase].redF());
		dG = remainder * (keyColour[iBase + 1].greenF() - keyColour[iBase].greenF());
		dB = remainder * (keyColour[iBase + 1].blueF() - keyColour[iBase].blueF());
		dA = remainder * (keyColour[iBase + 1].alphaF() - keyColour[iBase].alphaF());
		qColour.setRedF(keyColour[iBase].redF() + dR);
		qColour.setGreenF(keyColour[iBase].greenF() + dG);
		qColour.setBlueF(keyColour[iBase].blueF() + dB);
		qColour.setAlphaF(keyColour[iBase].alphaF() + dA);
	}
	else { qColour = keyColour[keyColour.size() - 1]; }
	return qColour;
}
vector<QPointF> QTPAINT::getTLBR(vector<QPointF>& vQPF)
{
	vector<QPointF> TLBR(4);
	double top = 1000000.0, left = 1000000.0, bot = 0.0, right = 0.0;
	for (int ii = 0; ii < vQPF.size(); ii++)
	{
		if (vQPF[ii].y() < top) { top = vQPF[ii].y(); TLBR[0] = vQPF[ii]; }
		else if (vQPF[ii].y() > bot) { bot = vQPF[ii].y(); TLBR[2] = vQPF[ii]; }
		if (vQPF[ii].x() < left) { left = vQPF[ii].x(); TLBR[1] = vQPF[ii]; }
		else if (vQPF[ii].x() > right) { right = vQPF[ii].x(); TLBR[3] = vQPF[ii]; }
	}
	return TLBR;
}
void QTPAINT::initialize()
{
	keyColour.resize(6);
	keyColour[0].setRgbF(1.0, 0.0, 0.0, 1.0 );  // Red
	keyColour[1].setRgbF(1.0, 1.0, 0.0, 1.0 );  // Yellow
	keyColour[2].setRgbF(0.0, 1.0, 0.0, 1.0 );  // Green
	keyColour[3].setRgbF(0.0, 1.0, 1.0, 1.0 );  // Teal
	keyColour[4].setRgbF(0.0, 0.0, 1.0, 1.0 );  // Blue
	keyColour[5].setRgbF(0.5, 0.0, 1.0, 1.0 );  // Violet

	keyColourExtra.setRgbF(1.0, 0.0, 0.5, 1.0 );  // Pink
}
void QTPAINT::paintArea(QPainter& painter)
{
	// Paints all areas.
	vector<int> viArea(areas.size());
	for (int ii = 0; ii < viArea.size(); ii++)
	{
		viArea[ii] = ii;
	}
	paintArea(painter, viArea);
}
void QTPAINT::paintArea(QPainter& painter, vector<int>& viArea)
{
	QColor qBlack(Qt::black), qColour;
	QPen pen(Qt::SolidLine);
	pen.setWidth(1);
	pen.setColor(qBlack);
	painter.setPen(pen);
	QBrush brush(Qt::SolidPattern);
	if (qBG.isValid())
	{
		QRect qGeom = this->geometry();
		pen.setColor(qBG);
		painter.setPen(pen);
		brush.setColor(qBG);
		painter.setBrush(brush);
		painter.drawRect(qGeom);
	}
	for (int ii = 0; ii < viArea.size(); ii++)
	{
		qColour = areaColour[viArea[ii]];
		brush.setColor(qColour);
		painter.setBrush(brush);
		QPainterPath qPP(areas[viArea[ii]][0]);
		for (int jj = 1; jj < areas[viArea[ii]].size(); jj++)
		{
			qPP.lineTo(areas[viArea[ii]][jj]);
		}
		qPP.closeSubpath();
		painter.drawPath(qPP);
	}
	int bbq = 1;
}
void QTPAINT::paintEvent(QPaintEvent* event)
{
	if (areas.size() < 1) { return; }
	QPainter painter(this);
	paintArea(painter);
}
void QTPAINT::scaleChildToWidget(vector<vector<double>>& frameTLBR, vector<vector<double>>& border)
{
	frameTLBR[0][0] *= widgetPPKM;
	frameTLBR[0][1] *= widgetPPKM;
	frameTLBR[1][0] *= widgetPPKM;
	frameTLBR[1][1] *= widgetPPKM;
	for (int ii = 0; ii < border.size(); ii++)
	{
		border[ii][0] *= widgetPPKM;
		border[ii][1] *= widgetPPKM;
	}
}
void QTPAINT::scaleParentToWidget(vector<vector<double>>& border, double PPKM, vector<vector<double>> frameTLBR)
{
	double widgetWidth = (double)this->width();
	double widgetHeight = (double)this->height();
	double imgWidth = (frameTLBR[1][0] - frameTLBR[0][0]) * PPKM;
	double imgHeight = (frameTLBR[1][1] - frameTLBR[0][1]) * PPKM;
	double xRatio = (imgWidth + 1.0) / widgetWidth;
	double yRatio = (imgHeight + 1.0) / widgetHeight;
	double ratio = max(xRatio, yRatio);
	for (int ii = 0; ii < border.size(); ii++)
	{
		border[ii][0] *= (PPKM / ratio);
		border[ii][1] *= (PPKM / ratio);
	}
	widgetPPKM = PPKM / ratio;
}
void QTPAINT::scaleParentToWidget(vector<vector<double>>& border, vector<vector<double>> frameTLBR)
{
	// Used when painting from database.
	double widgetWidth = (double)this->width();
	double widgetHeight = (double)this->height();
	double imgWidthKM = frameTLBR[1][0] - frameTLBR[0][0];
	double imgHeightKM = frameTLBR[1][1] - frameTLBR[0][1];
	double xRatio = imgWidthKM / widgetWidth;  // km per pixel
	double yRatio = imgHeightKM / widgetHeight;
	double ratio = max(xRatio, yRatio);
	for (int ii = 0; ii < border.size(); ii++)
	{
		border[ii][0] /= ratio;
		border[ii][1] /= ratio;
	}
	widgetPPKM = 1.0 / ratio;
}
