#include "qtfunc.h"

using namespace std;

void QTFUNC::displayBinList(QListWidget*& qLW, vector<string>& pathBin)
{
	QString qtemp;
	string temp;
	size_t pos1, pos2;
	listPathBin = pathBin;
	qLW->clear();
	for (int ii = 0; ii < pathBin.size(); ii++)
	{
		pos2 = pathBin[ii].rfind(".bin");
		if (pos2 > pathBin[ii].size()) { jfqf.err("Failed to locate .bin extension-qf.displayBinList"); }
		pos1 = pathBin[ii].rfind('\\', pos2) + 1;
		temp = pathBin[ii].substr(pos1, pos2 - pos1);
		qtemp = QString::fromUtf8(temp);
		mapListPathBin.insert(qtemp, ii);
		qLW->addItem(qtemp);
	}
}
void QTFUNC::displayDebug(QLabel*& qlabel, vector<string>& pathPNG)
{
	int widthImg, heightImg, widthPM, heightPM, squareDim;
	QString qtemp = QString::fromUtf8(pathPNG[0]);
	vector<int> origin;
	if (pathPNG.size() > 1)
	{
		origin = jfqf.destringifyCoord(pathPNG[1]);
	}
	QImage qimg = QImage(qtemp);
	widthImg = qimg.width();
	heightImg = qimg.height();
	widthPM = qlabel->width();
	heightPM = qlabel->height();
	squareDim = min(widthPM, heightPM);
	if (origin.size() > 0)
	{
		int xTL = origin[0] - (squareDim / 2);
		int yTL = origin[1] - (squareDim / 2);		
		QRect qrCrop = QRect(xTL, yTL, squareDim, squareDim);
		QImage qimgCrop = qimg.copy(qrCrop);
		QPixmap qpm = QPixmap::fromImage(qimgCrop);
		qlabel->setPixmap(qpm);
		if (origin.size() > 2)
		{
			pathPNG[1] = to_string(origin[2]); // Origin point's border index.
		}
		else { pathPNG.resize(1); }
	}
	else
	{
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
	}
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
string QTFUNC::getBranchPath(QTreeWidgetItem*& qBranch, string rootDir)
{
	QList<QTreeWidgetItem*> qGenealogy;  // In reverse (present->parent->ancestors).
	qGenealogy.append(qBranch);
	QTreeWidgetItem* qnode = qBranch->parent();
	while (qnode != nullptr)
	{
		qGenealogy.append(qnode);
		qnode = qGenealogy[qGenealogy.size() - 1]->parent();
	}
	string sPath = rootDir;
	string temp;
	QString qtemp;
	for (int ii = qGenealogy.size() - 2; ii >= 0; ii--)
	{
		qtemp = qGenealogy[ii]->text(0);
		temp = qtemp.toUtf8();
		sPath += "\\" + temp;
	}
	return sPath;
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

