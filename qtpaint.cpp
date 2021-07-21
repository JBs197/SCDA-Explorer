#include "stdafx.h"
#include "qtpaint.h"

void QTPAINT::addArea(vector<vector<double>>& border)
{
	int areaIndex = areas.size();
	areas.push_back(vector<QPointF>(border.size()));
	for (int ii = 0; ii < border.size(); ii++)
	{
		if (border[ii][0] < 0.0) 
		{ 
			if (border[ii][0] > -1.0 * defaultMargin) { border[ii][0] = 0.0; }
			else { jf.err("Bad xCoord-qp.addArea"); }
		}
		else if (border[ii][0] > Width)
		{
			if (border[ii][0] < Width + defaultMargin) { border[ii][0] = Width - 1.0; }
			else { jf.err("Bad xCoord-qp.addArea"); }
		}
		if (border[ii][1] < 0.0)
		{
			if (border[ii][1] > -1.0 * defaultMargin) { border[ii][1] = 0.0; }
			else { jf.err("Bad yCoord-qp.addArea"); }
		}
		else if (border[ii][1] > Height)
		{
			if (border[ii][1] < Height + defaultMargin) { border[ii][1] = Height - 1.0; }
			else { jf.err("Bad yCoord-qp.addArea"); }
		}
		areas[areaIndex][ii].setX(border[ii][0]);
		areas[areaIndex][ii].setY(border[ii][1]);
	}
}
void QTPAINT::addArea(vector<QPointF>& border)
{
	areas.push_back(border);
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
void QTPAINT::addChild(vector<QPointF>& border, double scale, QPointF position, vector<POINT> frameTLBR)
{
	if (parentPositionKM.isNull() || areas.size() < 1) { jf.err("No parent-qp.addChild"); }
	QPointF disp, myPositionKM;  // myPositionKM is relative to the parent's position relative to "Home".
	myPositionKM.setX(position.rx() - parentPositionKM.rx());
	myPositionKM.setY(position.ry() - parentPositionKM.ry());
	disp.setX(myPositionKM.rx() * widgetPPKM);
	disp.setY((myPositionKM.ry()) * widgetPPKM);
	scaleChildToWidget(border, scale);
	displaceChildToParent(border, disp);
	addArea(border);
}
void QTPAINT::addParent(vector<QPointF>& border, double scale, QPointF position, vector<POINT> frameTLBR)
{
	vector<int> viDummy;
	addParent(border, scale, position, frameTLBR, viDummy);
}
void QTPAINT::addParent(vector<QPointF>& border, double scale, QPointF position, vector<POINT> frameTLBR, vector<int> rgbaBG)
{
	clear();
	parentPositionKM = position;
	scaleParentToWidget(border, scale, frameTLBR);
	addArea(border);
	addAreaColour(keyColourExtra);
	if (rgbaBG.size() > 2)
	{
		qBG.setRgb(rgbaBG[0], rgbaBG[1], rgbaBG[2]);
	}
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
void QTPAINT::displaceChildToParent(vector<QPointF>& vQPF, QPointF disp)
{
	for (int ii = 0; ii < vQPF.size(); ii++)
	{
		vQPF[ii] += disp;
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
	if (keyColourBand.size() < 1) { jf.err("No init-qp.getColourFromSpectrum"); }
	double myPos = zeroOne * (double)keyColourBand.size();
	int iBase = (int)myPos;
	double remainder = myPos - (double)iBase;
	int iR = 0, iG = 0, iB = 0, iA = 0;
	if (iBase < keyColourBand.size())
	{
		iR = (int)(keyColourBand[iBase][0] * remainder);
		iG = (int)(keyColourBand[iBase][1] * remainder);
		iB = (int)(keyColourBand[iBase][2] * remainder);
		iA = (int)(keyColourBand[iBase][3] * remainder);
	}
	vector<int> rgba = {
		(int)keyColour[iBase][0],
		(int)keyColour[iBase][1],
		(int)keyColour[iBase][2],
		(int)keyColour[iBase][3]
	};
	QColor qColour(rgba[0] + iR, rgba[1] + iG, rgba[2] + iB, rgba[3] + iA);
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
	Width = (double)this->width();
	Height = (double)this->height();

	keyColour.resize(6);
	keyColour[0] = { 255.0, 0.0, 0.0, 255.0 };  // Red
	keyColour[1] = { 255.0, 255.0, 0.0, 255.0 };  // Yellow
	keyColour[2] = { 0.0, 255.0, 0.0, 255.0 };  // Green
	keyColour[3] = { 0.0, 255.0, 255.0, 255.0 };  // Teal
	keyColour[4] = { 0.0, 0.0, 255.0, 255.0 };  // Blue
	keyColour[5] = { 127.0, 0.0, 255.0, 255.0 };  // Violet

	keyColourBand.resize(5, vector<double>(4));  // Represents the RGBA change from kC[ii] to kC[ii + 1].
	for (int ii = 0; ii < keyColourBand.size(); ii++)
	{
		for (int jj = 0; jj < 4; jj++)
		{
			keyColourBand[ii][jj] = keyColour[ii + 1][jj] - keyColour[ii][jj];
		}
	}

	keyColourExtra = { 255.0, 0.0, 127.0, 255.0 };  // Pink
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
	QColor qColour;
	QPen pen(Qt::SolidLine);
	pen.setWidth(1);
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
		pen.setColor(qColour);
		painter.setPen(pen);
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
void QTPAINT::scaleChildToWidget(vector<QPointF>& vQPF, double PPKM)
{
	double ratio = widgetPPKM / PPKM;
	for (int ii = 0; ii < vQPF.size(); ii++)
	{
		vQPF[ii] *= ratio;
	}
}
void QTPAINT::scaleParentToWidget(vector<QPointF>& vQPF, double PPKM, vector<POINT> frameTLBR)
{
	int iWidth = this->width();
	int iHeight = this->height();
	double xRatio = (double)(frameTLBR[1].x - frameTLBR[0].x + 1) / iWidth;
	double yRatio = (double)(frameTLBR[1].y - frameTLBR[0].y + 1) / iHeight;
	double ratio = max(xRatio, yRatio);
	for (int ii = 0; ii < vQPF.size(); ii++)
	{
		vQPF[ii] /= ratio;
	}
	widgetPPKM = PPKM / ratio;
	int bbq = 1;
}
void QTPAINT::scaleToWidget(vector<QPointF>& vQPF)
{
	vector<QPointF> TLBRf = getTLBR(vQPF);  // top, left, bot, right ???
	//QPointF margin(defaultMargin, defaultMargin);
	//TLBRf[1] += margin;
	double xRatio = TLBRf[3].rx() / Width;
	double yRatio = TLBRf[2].ry() / Height;
	double ratio = max(xRatio, yRatio);
	for (int ii = 0; ii < vQPF.size(); ii++)
	{
		vQPF[ii] /= ratio;
	}
}
