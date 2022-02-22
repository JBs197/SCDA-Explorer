#include "qjpaint.h"

using namespace std;

QJPAINT::QJPAINT(QWidget* parent) : QWidget(parent) 
{
	bg = { 0, 0, 0, 0 };
}
QJPAINT::~QJPAINT()
{
	for (int ii = 0; ii < qlShape.size(); ii++) {
		delete qlShape[ii];
	}
}

void QJPAINT::clearShape()
{
	for (int ii = qlShape.size() - 1; ii >= 0; ii--) {
		delete qlShape[ii];
	}
	qlShape.clear();
}
void QJPAINT::drawBackground(QPainter* painter)
{
	QColor qcBG(bg[0], bg[1], bg[2], bg[3]);
	QSize canvas = this->size();
	painter->fillRect(0, 0, canvas.width() - 1, canvas.height() - 1, qcBG);

}
void QJPAINT::err(string message)
{
	string errorMessage = "QJPAINT error:\n" + message;
	JLOG::getInstance()->err(errorMessage);
}
QJSHAPE* QJPAINT::getShape(int index)
{
	if (index < 0 || index >= qlShape.size()) { return nullptr; }
	return qlShape[index];
}
QJSHAPE* QJPAINT::insertShape(int index)
{
	// Make a new (blank) shape at the specified index, then return a pointer to it.
	if (index < 0) { 
		index = qlShape.size();
		qlShape.append(new QJSHAPE); 
	}
	else { qlShape.insert(index, new QJSHAPE); }
	return qlShape[index];
}
void QJPAINT::paintEvent(QPaintEvent* event)
{

	QPainter painter(this);
	painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
	drawBackground(&painter);

	//painter.save();
	
	QBrush brush(Qt::SolidPattern);
	QPen pen(Qt::SolidLine);
	//pen.setColor(qcBG);
	//painter.setBrush(brush);
	//painter.setPen(pen);
	//painter.fillRect(0, 0, canvas.width() - 1, canvas.height() - 1, brush);
	//this->update();
	//painter.restore();

	int numChannel;
	int numShape = qlShape.size();
	for (int ii = 0; ii < numShape; ii++) {
		//painter.save();
		switch (qlShape[ii]->type) {
		case QJSHAPE::Path:
		{
			//QBrush brush(Qt::NoBrush);
			QColor qcFill;
			numChannel = (int)qlShape[ii]->rgbxFill.size();
			if (numChannel < 3) { brush.setStyle(Qt::NoBrush); }
			else if (numChannel == 3) {
				qcFill.setRgb(qlShape[ii]->rgbxFill[0], qlShape[ii]->rgbxFill[1], qlShape[ii]->rgbxFill[2]);
				//brush.setStyle(Qt::SolidPattern);
				brush.setColor(qcFill);
			}
			else if (numChannel > 3) {
				qcFill.setRgb(qlShape[ii]->rgbxFill[0], qlShape[ii]->rgbxFill[1], qlShape[ii]->rgbxFill[2], qlShape[ii]->rgbxFill[3]);
				//brush.setStyle(Qt::SolidPattern);
				brush.setColor(qcFill);
			}
			painter.setBrush(brush);
			QColor qcOutline(qlShape[ii]->rgbxOutline[0], qlShape[ii]->rgbxOutline[1], qlShape[ii]->rgbxOutline[2], qlShape[ii]->rgbxOutline[3]);
			pen.setWidth(qlShape[ii]->thickness);
			pen.setColor(qcOutline);
			painter.setPen(pen);
			painter.drawPath(qlShape[ii]->path);
			break;
		}
		}
		//painter.restore();
	}

}
void QJPAINT::setBG(vector<unsigned char> rgbx)
{
	int numChannel = (int)rgbx.size();
	if (numChannel < 3) { return; }
	bg[0] = rgbx[0];
	bg[1] = rgbx[1];
	bg[2] = rgbx[2];
	if (numChannel > 3) { bg[3] = rgbx[3]; }
	else { bg[3] = 255; }
}
