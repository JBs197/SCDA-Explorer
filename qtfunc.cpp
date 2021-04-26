#include "qtfunc.h"

using namespace std;

void QTFUNC::displayBin(QLabel*& qlabel, string& pathBIN)
{
	// Load all coordinates into memory from the bin file.
	string sfile = jfqf.load(pathBIN);
	if (sfile.size() < 1) { err("load-qf.displayBin"); }
	size_t pos1, pos2, posStart, posStop;
	vector<vector<int>> frameCorners, border;
	string temp;
	int row;
	posStart = sfile.find("//frame");
	posStop = sfile.find("//", posStart + 7);
	if (posStop > sfile.size()) { posStop = sfile.size(); }
	pos1 = sfile.find(',', posStart);
	while (pos1 < posStop)
	{
		row = frameCorners.size();
		frameCorners.push_back(vector<int>(2));
		pos2 = sfile.find_last_not_of("1234567890", pos1 - 1) + 1;
		temp = sfile.substr(pos2, pos1 - pos2);
		try { frameCorners[row][0] = stoi(temp); }
		catch (invalid_argument& ia) { err("stoi-qf.displayBin"); }
		pos2 = sfile.find_first_not_of("1234567890", pos1 + 1);
		temp = sfile.substr(pos1 + 1, pos2 - pos1 - 1);
		try { frameCorners[row][1] = stoi(temp); }
		catch (invalid_argument& ia) { err("stoi-qf.displayBin"); }
		pos1 = sfile.find(',', pos1 + 1);
	}
	posStart = sfile.find("//border");
	posStop = sfile.find("//", posStart + 8);
	if (posStop > sfile.size()) { posStop = sfile.size(); }
	pos1 = sfile.find(',', posStart);
	while (pos1 < posStop)
	{
		row = border.size();
		border.push_back(vector<int>(2));
		pos2 = sfile.find_last_not_of("1234567890", pos1 - 1) + 1;
		temp = sfile.substr(pos2, pos1 - pos2);
		try { border[row][0] = stoi(temp); }
		catch (invalid_argument& ia) { err("stoi-qf.displayBin"); }
		pos2 = sfile.find_first_not_of("1234567890", pos1 + 1);
		temp = sfile.substr(pos1 + 1, pos2 - pos1 - 1);
		try { border[row][1] = stoi(temp); }
		catch (invalid_argument& ia) { err("stoi-qf.displayBin"); }
		pos1 = sfile.find(',', pos1 + 1);
	}
	
	// Scale and shift the coordinates to fit the display window.
	if (pmCanvas.isNull()) { initPixmap(qlabel); }
	vector<double> mapShift(3);
	mapShift[0] = -1.0 * (double)frameCorners[0][0];
	mapShift[1] = -1.0 * (double)frameCorners[0][1];
	double widthPm = (double)pmCanvas.width();
	double heightPm = (double)pmCanvas.height();
	double widthFrame = (double)(frameCorners[2][0] - frameCorners[0][0]);
	double heightFrame = (double)(frameCorners[2][1] - frameCorners[0][1]);
	double ratioWidth = widthFrame / widthPm;
	double ratioHeight = heightFrame / heightPm;
	if (ratioHeight >= ratioWidth)
	{
		mapShift[2] = 1.0 / ratioHeight;
	}
	else
	{
		mapShift[2] = 1.0 / ratioWidth;
	}
	vector<vector<double>> borderShifted;
	im.coordShift(border, mapShift, borderShifted);

	// Paint the coordinates onto the GUI window.
	pmPainting = QPixmap(pmCanvas);
	QPainter* painter = new QPainter(&pmPainting);
	QPen pen = QPen(colourDefault, 3.0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
	painter->setPen(pen);
	QPainterPath path = pathMake(borderShifted);
	painter->drawPath(path);
	qlabel->setPixmap(pmPainting);
	int bbq = 1;
}
void QTFUNC::displayDebug(QLabel*& qlabel, string& pathPNG)
{
	int widthImg, heightImg, widthPM, heightPM;
	QString qtemp = QString::fromUtf8(pathPNG);
	QImage qimg = QImage(qtemp);
	widthImg = qimg.width();
	heightImg = qimg.height();
	widthPM = qlabel->width();
	heightPM = qlabel->height();
	QImage qimgScaled;
	if ((double)heightImg / (double)heightPM > (double)widthImg / (double)widthPM)
	{
		qimgScaled = qimg.scaledToHeight(heightPM);
	}
	else
	{
		qimgScaled = qimg.scaledToWidth(widthPM);
	}
	QPixmap qpm = QPixmap::fromImage(qimgScaled);
	qlabel->setPixmap(qpm);
	int bbq = 1;
}
void QTFUNC::displayPainterPath(QLabel*& qlabel, QPainterPath& path)
{
	if (pmCanvas.isNull()) { initPixmap(qlabel); }
	pmPainting = QPixmap(pmCanvas);
	QPainter* painter = new QPainter(&pmPainting);
	QPen pen = QPen(colourDefault, 3.0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
	painter->setPen(pen);
	painter->drawPath(path);
	qlabel->setPixmap(pmPainting);
}
void QTFUNC::displayText(QLabel* ql, string stext)
{
	const QString qtemp = QString::fromUtf8(stext);
	ql->setText(qtemp);
}
void QTFUNC::display_subt(QTreeWidget* qview, QTreeWidgetItem* qparent)
{
	qview->clear();
	qview->addTopLevelItem(qparent);
	qview->expandAll();
}
void QTFUNC::drawDotsDebug(QPainter& qpaint, vector<vector<double>>& dots)
{
	double topLeftX, topLeftY;
	QRectF qRF;
	QBrush qB = qpaint.brush();
	for (int ii = 0; ii < dots.size(); ii++)
	{
		topLeftX = dots[ii][0] - (double)(diameterDefault / 2);
		topLeftY = dots[ii][1] - (double)(diameterDefault / 2);
		qRF = QRectF(topLeftX, topLeftY, (double)diameterDefault, (double)diameterDefault);
		qpaint.fillRect(qRF, qB);
	}
}
void QTFUNC::drawLinesDebug(QPainter& qpaint, vector<vector<double>>& lines)
{
	QPainterPath qPP;
	qPP.moveTo(lines[0][0], lines[0][1]);
	for (int ii = 1; ii < lines.size(); ii++)
	{
		qPP.lineTo(lines[ii][0], lines[ii][1]);
	}
	qpaint.drawPath(qPP);
}
void QTFUNC::err(string func)
{
	jfqf.err(func);
}
int QTFUNC::get_display_root(QTreeWidget* name)
{
	int index = map_display_root.value(name, -1);
	return index;
}
void QTFUNC::initPixmap(QLabel* qlabel)
{
	int width = qlabel->width();
	int height = qlabel->height();
	pmCanvas = QPixmap(width, height);
	pmCanvas.fill();
}
QPainterPath QTFUNC::pathMakeCircle(vector<double> origin, double radius, int sides)
{
	double angleInc = 6.283185307 / (double)sides;
	vector<vector<double>> vCoords(sides, vector<double>(2));
	for (int ii = 0; ii < sides; ii++)
	{
		vCoords[ii][0] = origin[0] + (radius * cos((double)ii * angleInc));
		vCoords[ii][1] = origin[1] + (radius * sin((double)ii * angleInc));
	}
	QPainterPath path;
	path.moveTo(vCoords[0][0], vCoords[0][1]);
	for (int ii = 1; ii < sides; ii++)
	{
		path.lineTo(vCoords[ii][0], vCoords[ii][1]);
	}
	path.closeSubpath();
	return path;
}
void QTFUNC::pmPainterReset(QLabel*& qlabel)
{
	if (pmCanvas.isNull()) { initPixmap(qlabel); }
	if (painter.isActive()) { painter.end(); }
	pmPainting = QPixmap(pmCanvas);
	painter.begin(&pmPainting);

	pen.setColor(colourDefault);
	pen.setWidth(3);
	pen.setStyle(Qt::SolidLine);
	pen.setCapStyle(Qt::RoundCap);
	pen.setJoinStyle(Qt::RoundJoin);

	brush.setColor(colourDefault);
	brush.setStyle(Qt::SolidPattern);

}
void QTFUNC::set_display_root(QTreeWidget* name, int val)
{
	map_display_root.insert(name, val);
}

