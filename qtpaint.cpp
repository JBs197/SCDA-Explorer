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
void QTPAINT::addAreaColour(vector<int> rgb)
{
	if (rgb.size() < 3) { jf.err("Missing rgb-qp.addAreaColour"); }
	if (rgb.size() == 3)
	{
		QColor qC(rgb[0], rgb[1], rgb[2]);
		areaColour.push_back(qC);
	}
	else
	{
		QColor qC(rgb[0], rgb[1], rgb[2], rgb[3]);
		areaColour.push_back(qC);
	}
}
void QTPAINT::drawFamilyBlue(vector<BINMAP>& binFamily)
{
	areas.clear();
	areaColour.clear();
	mapDotIndex.clear();
	dots.clear();
	dots.resize(binFamily.size() - 1);
	binFamily[0].makeWindowBorder(Width, Height);
	addArea(binFamily[0].myWindowBorder);
	addAreaColour({ 255, 255, 255 });
	parentTLBR = binFamily[0].getTLBR(binFamily[0].myWindowBorder);
	double xC, yC, dx, dy; 
	double boxWidth = parentTLBR[1][0] - parentTLBR[0][0];
	double boxHeight = parentTLBR[1][1] - parentTLBR[0][1];
	for (int ii = 1; ii < binFamily.size(); ii++)
	{
		if (binFamily[ii].blueDot.size() < 2) { jf.err("Missing blue dot-qp.drawFamilyBlue"); }
		xC = binFamily[ii].blueDot[0];
		yC = binFamily[ii].blueDot[1];
		xC *= boxWidth;
		yC *= boxHeight;
		xC += parentTLBR[0][0];
		yC += parentTLBR[0][1];
		QPointF qpf(xC, yC);
		dots[ii - 1] = qpf;
		mapDotIndex.emplace(binFamily[ii].myName, ii - 1);
		if (binFamily[ii].isSelected) { selectedDot = ii - 1; }
	}

	crosshairs.clear();
	if (debug)
	{
		vector<QPointF> dotTLBR = getTLBR(dots);
		vector<vector<double>> dotGPS(binFamily.size() - 1, vector<double>(2));
		for (int ii = 1; ii < binFamily.size(); ii++)
		{
			dotGPS[ii - 1] = binFamily[ii].myPosition;
		}
		vector<vector<double>> gpsTLBR = binFamily[0].getTLBR(dotGPS);
		double ySpanGPS = gpsTLBR[1][0] - gpsTLBR[0][0];
		double xSpanGPS = gpsTLBR[1][1] - gpsTLBR[0][1];
		double ySpanPixel = dotTLBR[2].y() - dotTLBR[0].y();
		double xSpanPixel = dotTLBR[3].x() - dotTLBR[1].x();
		QPointF qpfTemp;
		crosshairs.resize(dots.size());
		for (int ii = 0; ii < crosshairs.size(); ii++)
		{
			dx = dotGPS[ii][1] - gpsTLBR[0][1];
			dx *= (xSpanPixel / xSpanGPS);
			qpfTemp.setX(dotTLBR[1].x() + dx);
			dy = gpsTLBR[1][0] - dotGPS[ii][0];
			dy *= (ySpanPixel / ySpanGPS);
			qpfTemp.setY(dotTLBR[0].y() + dy);
			crosshairs[ii] = qpfTemp;
		}
		int bbq = 1;
	}
	
	update();
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
}
void QTPAINT::paintEvent(QPaintEvent* event)
{
	QPainter painter(this);
	QBrush brush(Qt::SolidPattern);
	QPen pen(brush, 2.0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
	if (areas.size() < 1 || areas.size() != areaColour.size()) { return; }
	pen.setColor(Qt::black);
	painter.setPen(pen);
	for (int ii = 0; ii < areas.size(); ii++)
	{
		brush.setColor(areaColour[ii]);
		painter.setBrush(brush);
		QPainterPath qPP(areas[ii][0]);
		for (int jj = 1; jj < areas[ii].size(); jj++)
		{
			qPP.lineTo(areas[ii][jj]);
		}
		qPP.closeSubpath();
		painter.drawPath(qPP);
	}
	
	pen.setColor(qcBlue);
	painter.setPen(pen);
	brush.setColor(qcBlue);
	painter.setBrush(brush);
	double radius = defaultDotWidth / 2.0;
	QPointF TL, BR;
	for (int ii = 0; ii < dots.size(); ii++)
	{
		TL.setX(dots[ii].rx() - radius);
		TL.setY(dots[ii].ry() - radius);
		BR.setX(dots[ii].rx() + radius);
		BR.setY(dots[ii].ry() + radius);
		QRectF qrf(TL, BR);
		if (ii == selectedDot)
		{
			painter.save();
			pen.setColor(Qt::red);
			painter.setPen(pen);
			brush.setColor(Qt::red);
			painter.setBrush(brush);
			painter.drawRect(qrf);
			painter.restore();
		}
		else { painter.drawRect(qrf); }
	}
	radius = (defaultDotWidth + 2.0) / 2.0;
	painter.setBrush(Qt::NoBrush);
	pen.setWidth(1);
	pen.setColor(Qt::green);
	painter.setPen(pen);
	int pixels;
	vector<QPointF> crosshairsPixel;
	for (int ii = 0; ii < crosshairs.size(); ii++)
	{
		crosshairsPixel = qf.getCrosshairs(crosshairs[ii], radius);
		pixels = crosshairsPixel.size();
		painter.drawPoints(&crosshairsPixel[0], pixels);
	}

}
