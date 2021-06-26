#include "qtpaint.h"

void QTPAINT::addArea(vector<vector<double>>& border)
{
	int areaIndex = areas.size();
	areas.push_back(vector<QPointF>(border.size()));
	for (int ii = 0; ii < border.size(); ii++)
	{
		if (border[ii][0] < 0.0 || border[ii][0] > windowDim[0]) { jf.err("Bad xCoord-qp.addArea"); }
		if (border[ii][1] < 0.0 || border[ii][1] > windowDim[1]) { jf.err("Bad yCoord-qp.addArea"); }
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
void QTPAINT::drawFamily(vector<BINMAP>& binFamily, int selected)
{
	areas.clear();
	areaColour.clear();
	for (int ii = 0; ii < binFamily.size(); ii++)
	{

	}
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
}
