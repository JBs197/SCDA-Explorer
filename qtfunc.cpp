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
	int widthPm = pm.width();
	int heightPm = pm.height();
	int widthFrame = frameCorners[2][0] - frameCorners[0][0];
	int heightFrame = frameCorners[2][1] - frameCorners[0][1];
	double ratioWidth = (double)widthFrame / (double)widthPm;
	double ratioHeight = (double)heightFrame / (double)heightPm;
	double scaleFactor;
	if (ratioWidth >= 1.0 || ratioHeight >= 1.0)
	{
		if (ratioHeight >= ratioWidth)
		{
			scaleFactor = 1.0 / ratioHeight;
		}
		else
		{
			scaleFactor = 1.0 / ratioWidth;
		}
	}
	vector<int> DxDy(2);
	DxDy[0] = -1 * frameCorners[0][0];
	DxDy[1] = -1 * frameCorners[0][1];
	vector<vector<int>> frameCornersShifted = im.coordShift(frameCorners, DxDy);
	vector<vector<int>> frameCornersScaled;
	im.coordScale(frameCornersShifted, scaleFactor, frameCornersScaled);
	vector<vector<int>> borderShifted = im.coordShift(border, DxDy);
	vector<vector<double>> borderScaled;
	im.coordScale(borderShifted, scaleFactor, borderScaled);

	// Paint the coordinates onto the GUI window.
	QRect frame = QRect(frameCornersScaled[0][0], frameCornersScaled[0][1], frameCornersScaled[2][0] - frameCornersScaled[0][0], frameCornersScaled[2][1] - frameCornersScaled[0][1]);
	QPainter* painter = new QPainter(&pm);
	QBrush brush = QBrush(Qt::black, Qt::SolidPattern);
	QPen pen = QPen(brush, 4.0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
	painter->setPen(pen);
	painter->drawRect(frame);
	QPainterPath path = pathMake(borderScaled);
	painter->drawPath(path);
	qlabel->setPixmap(pm);
	int height = pm.height();
	int width = pm.width();
	int bbq = 1;
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
	pm = QPixmap(width, height);
	pm.fill();
}
void QTFUNC::set_display_root(QTreeWidget* name, int val)
{
	map_display_root.insert(name, val);
}

