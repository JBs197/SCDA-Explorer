#include "qtfunc.h"

using namespace std;

int QTFUNC::deleteChildren(QTreeWidgetItem*& qNode)
{
	int numKids = 0;
	QList<QTreeWidgetItem*> qChildren = qNode->takeChildren();
	if (qChildren.size() > 0)
	{
		foreach(QTreeWidgetItem * child, qChildren)
		{
			delete child;
			numKids++;
		}
		return numKids;
	}
	else { return 0; }
	return -1;
}
int QTFUNC::deleteLeaves(QTreeWidgetItem*& qNode)
{
	int count = 0;
	int numKids = qNode->childCount(), numGrandKids;
	if (numKids == 0) { return 0; }
	QTreeWidgetItem* qChild = nullptr;
	for (int ii = numKids - 1; ii >= 0; ii--)
	{
		qChild = qNode->child(ii);
		numGrandKids = qChild->childCount();
		if (numGrandKids == 0)
		{
			qChild = qNode->takeChild(ii);
			delete qChild;
			count++;
		}
	}
	return count;
}
void QTFUNC::displayBinList(QListWidget*& qLW, vector<string>& pathBin)
{
	QString qtemp;
	string temp, temp8;
	size_t pos1, pos2;
	listPathBin = pathBin;
	qLW->clear();
	for (int ii = 0; ii < pathBin.size(); ii++)
	{
		pos2 = pathBin[ii].rfind(".bin");
		if (pos2 > pathBin[ii].size()) { jf.err("Failed to locate .bin extension-qf.displayBinList"); }
		pos1 = pathBin[ii].rfind('\\', pos2) + 1;
		temp = pathBin[ii].substr(pos1, pos2 - pos1);
		temp8 = jf.asciiToUTF8(temp);
		qtemp = QString::fromUtf8(temp8);
		mapListPathBin.insert(qtemp, ii);
		qLW->addItem(qtemp);
	}
}
void QTFUNC::displayDebug(QLabel*& qlabel, vector<string>& pathPNG, vector<vector<int>>& debugMapCoord)
{
	// Load the interactive points into memory.
	string pathBin = pathPNG[0].substr(0, pathPNG[0].size() - 4);
	pathBin += ".bin";
	debugMapCoord = loadDebugMapCoord(pathBin);

	// Load the image onto the GUI.
	int widthImg, heightImg, widthPM, heightPM, squareDim;
	QString qtemp = QString::fromUtf8(pathPNG[0]);
	QImage qimg = QImage(qtemp);
	widthImg = qimg.width();
	heightImg = qimg.height();
	widthPM = qlabel->width();
	heightPM = qlabel->height();
	squareDim = min(widthPM, heightPM);
	QImage qimgScaled;
	if ((double)heightImg / (double)heightPM > (double)widthImg / (double)widthPM)
	{
		if (heightPM != heightImg)
		{
			qimgScaled = qimg.scaledToHeight(heightPM);
		}
		else
		{
			qimgScaled = qimg;
		}
	}
	else
	{
		if (widthPM != widthImg)
		{
			qimgScaled = qimg.scaledToWidth(widthPM);
		}
		else
		{
			qimgScaled = qimg;
		}
	}
	QPixmap qpm = QPixmap::fromImage(qimgScaled);
	qlabel->setPixmap(qpm);
	lastMap = 0;
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
void QTFUNC::eraser(QLabel*& qlabel, vector<vector<int>> TLBR)
{
	// TLBR is the topLeft and botRight set of coords clicked on,
	// relative to the GUI's shifted image (starting at 0,0).
	vector<vector<double>> shiftedTLBR;
	im.coordShiftRev(TLBR, recentMapShift, shiftedTLBR);
	vector<vector<int>> sTLBR, activeBorder;
	jf.toInt(shiftedTLBR, sTLBR);
	int index = recentBorderTemp.size() - 1;
	if (index < 0)
	{
		activeBorder = recentBorder;
	}
	else
	{
		activeBorder = recentBorderTemp[index];
	}
	for (int ii = activeBorder.size() - 1; ii >= 0; ii--)
	{
		if (activeBorder[ii][0] < sTLBR[0][0] || activeBorder[ii][0] > sTLBR[1][0]) { continue; }
		if (activeBorder[ii][1] < sTLBR[0][1] || activeBorder[ii][1] > sTLBR[1][1]) { continue; }
		activeBorder.erase(activeBorder.begin() + ii);
	}
	recentBorderTemp.push_back(activeBorder);
	vector<vector<double>> borderShifted;
	im.coordShift(activeBorder, recentMapShift, borderShifted);
	QPainterPath path = pathMake(borderShifted);
	displayBin(qlabel, path);
	int bbq = 1;  
}
void QTFUNC::err(string func)
{
	jf.err(func);
}
int QTFUNC::getBranchGen(QTreeWidgetItem*& qBranch)
{
	// Generation 0 is the root.
	int iGen = -1;
	QTreeWidgetItem* pNode = nullptr;
	QTreeWidgetItem* pParent = qBranch;
	do
	{
		pNode = pParent;
		pParent = pNode->parent();
		iGen++;
	} while (pParent != nullptr);
	return iGen;
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
QBitmap QTFUNC::getEraser(int width)
{
	QPixmap qPM(width, width);
	qPM.fill(Eraser);
	QBitmap qBM = QBitmap::fromPixmap(qPM);
	return qBM;
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
void QTFUNC::loadBinMap(string& pathBin, vector<vector<vector<int>>>& frames, double& scale, vector<double>& position, string& sParent8, vector<vector<int>>& border)
{
	// Load all data into memory from the bin file.
	frames.clear();
	position.clear();
	border.clear();
	string sfile = jf.load(pathBin), temp, temp2, temp3, temp4;
	if (sfile.size() < 1) { err("load-qf.loadBinMap"); }
	size_t pos1, pos2, posStart;
	int row, index;
	pos1 = pathBin.rfind('\\') + 1;
	pos2 = pathBin.rfind('.');
	string sName = pathBin.substr(pos1, pos2 - pos1);

	posStart = sfile.find("//frame");
	if (posStart > sfile.size()) { jf.err("Missing header-qf.loadBinMap"); }
	pos2 = sfile.find('\n', posStart);
	frames.resize(3, vector<vector<int>>(2, vector<int>(2)));
	for (int ii = 0; ii < 3; ii++)
	{
		pos1 = pos2 + 1;
		pos2 = sfile.find(',', pos1);
		temp = sfile.substr(pos1, pos2 - pos1);
		pos1 = pos2 + 1;
		pos2 = sfile.find('@', pos1);
		temp2 = sfile.substr(pos1, pos2 - pos1);
		pos1 = pos2 + 1;
		pos2 = sfile.find(',', pos1);
		temp3 = sfile.substr(pos1, pos2 - pos1);
		pos1 = pos2 + 1;
		pos2 = sfile.find('\n', pos1);
		temp4 = sfile.substr(pos1, pos2 - pos1);
		try
		{
			frames[ii][0][0] = stoi(temp);
			frames[ii][0][1] = stoi(temp2);
			frames[ii][1][0] = stoi(temp3);
			frames[ii][1][1] = stoi(temp4);
		}
		catch (invalid_argument& ia) { jf.err("stoi-qf.loadBinMap"); }
	}

	posStart = sfile.find("//scale");
	if (posStart > sfile.size()) { jf.err("Missing header-qf.loadBinMap"); }
	pos1 = sfile.find('\n', posStart) + 1;
	pos2 = sfile.find('\n', pos1);
	temp = sfile.substr(pos1, pos2 - pos1);

	pos1 = sName.find("Canada");
	if (pos1 > sName.size())
	{
		position.resize(2);
		posStart = sfile.find("//position");
		if (posStart > sfile.size()) { jf.err("Missing header-qf.loadBinMap"); }
		pos1 = sfile.find('\n', posStart) + 1;
		pos2 = sfile.find(',', pos1);
		temp2 = sfile.substr(pos1, pos2 - pos1);
		pos1 = pos2 + 1;
		pos2 = sfile.find('\n', pos1);
		temp3 = sfile.substr(pos1, pos2 - pos1);
		try
		{
			scale = stod(temp);
			position[0] = stod(temp2);
			position[1] = stod(temp3);
		}
		catch (invalid_argument& ia) { jf.err("stod-qf.loadBinMap"); }
		pos1 = sfile.find('(', posStart) + 1;
		pos2 = sfile.find(')', pos1);
		sParent8 = sfile.substr(pos1, pos2 - pos1);
	}
	else
	{
		try { scale = stod(temp); }
		catch (invalid_argument) { jf.err("stod-qf.loadBinMap"); }
	}

	posStart = sfile.find("//border");
	pos1 = sfile.find('\n', posStart) + 1;
	pos2 = sfile.find(',', pos1);
	while (pos2 < sfile.size())
	{
		index = border.size();
		border.push_back(vector<int>(2));
		temp = sfile.substr(pos1, pos2 - pos1);
		pos1 = pos2 + 1;
		pos2 = sfile.find('\n', pos1);
		temp2 = sfile.substr(pos1, pos2 - pos1);
		try
		{
			border[index][0] = stoi(temp);
			border[index][1] = stoi(temp2);
		}
		catch (invalid_argument& ia) { jf.err("stoi-qf.loadBinMap"); }
		pos1 = pos2 + 1;
		pos2 = sfile.find(',', pos1);
	}
}
vector<vector<int>> QTFUNC::loadDebugMapCoord(string& pathBin)
{
	vector<vector<int>> DMC(2, vector<int>(2));
	string sfile = jf.load(pathBin);
	size_t pos1 = sfile.find("//topleft");
	pos1 = sfile.find('\n', pos1 + 9) + 1;
	size_t pos2 = sfile.find('\n', pos1);
	string temp = sfile.substr(pos1, pos2 - pos1);
	DMC[0] = jf.destringifyCoord(temp);
	pos1 = sfile.find("//origin");
	pos1 = sfile.find('\n', pos1 + 8) + 1;
	pos2 = sfile.find('\n', pos1);
	temp = sfile.substr(pos1, pos2 - pos1);
	DMC[1] = jf.destringifyCoord(temp);
	pos1 = sfile.find("//candidate");
	pos1 = sfile.find('\n', pos1 + 10) + 1;
	pos2 = sfile.find('\n', pos1);
	while (pos2 < sfile.size())
	{
		temp = sfile.substr(pos1, pos2 - pos1);
		DMC.push_back(jf.destringifyCoord(temp));
		pos1 = pos2 + 1;
		pos2 = sfile.find('\n', pos1);
		if (pos1 == pos2) { break; }
	}
	return DMC;
}
string QTFUNC::makePathTree(QTreeWidgetItem*& qBranch)
{
	QString qtemp = qBranch->text(0);
	string sPath = qtemp.toStdString();
	QTreeWidgetItem* qParent = qBranch->parent();
	QTreeWidgetItem* qNode = nullptr;
	while (qParent != nullptr)
	{
		qtemp = qParent->text(0);
		sPath = qtemp.toStdString() + "\\" + sPath;
		qNode = qParent;
		qParent = qNode->parent();
	}
	size_t pos1 = sPath.find('\\');
	string temp = sPath.substr(pos1);
	sPath = sroot + temp;
	return sPath;
}
vector<double> QTFUNC::makeShift(QLabel*& qlabel, vector<vector<int>>& frameCorners)
{
	// Return a vector of the form [xShift, yShift, stretchFactor].
	if (pmCanvas.isNull()) { initPixmap(qlabel); }
	if (frameCorners.size() != 6 && frameCorners.size() != 2) { err("frameCorners size-qf.makeShift"); }
	vector<double> mapShift(3);
	mapShift[0] = -1.0 * (double)frameCorners[0][0];
	mapShift[1] = -1.0 * (double)frameCorners[0][1];
	double widthPm = (double)pmCanvas.width();
	double heightPm = (double)pmCanvas.height();
	double widthFrame = (double)(frameCorners[1][0] - frameCorners[0][0]);
	double heightFrame = (double)(frameCorners[1][1] - frameCorners[0][1]);
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
	recentMapShift = mapShift;
	return mapShift;
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
void QTFUNC::printEditedMap(string& pathBin)
{
	int sizeRBT = recentBorderTemp.size();
	if (sizeRBT < 1) { return; }
	ofstream sPrinter(pathBin.c_str(), ios::trunc);
	auto report = sPrinter.rdstate();
	sPrinter << "//frame" << endl;
	for (int ii = 0; ii < frameCorners.size(); ii++)
	{
		sPrinter << to_string(frameCorners[ii][0]) << "," << to_string(frameCorners[ii][1]) << endl;
	}
	sPrinter << endl;

	sPrinter << "//border" << endl;
	for (int ii = 0; ii < recentBorderTemp[sizeRBT - 1].size(); ii++)
	{
		sPrinter << to_string(recentBorderTemp[sizeRBT - 1][ii][0]) << "," << to_string(recentBorderTemp[sizeRBT - 1][ii][1]) << endl;
	}
	sPrinter.close();
}
void QTFUNC::setDebugMapPath(string spath)
{
	defaultDebugMapPath = spath;
	spath.insert(spath.size() - 4, "Selected");
	defaultDebugMapPathSelected = spath;
}
void QTFUNC::set_display_root(QTreeWidget* name, int val)
{
	map_display_root.insert(name, val);
}
void QTFUNC::undoEraser(QLabel*& qlabel)
{
	vector<vector<double>> borderShifted;
	vector<vector<int>> activeBorder;
	int sizeRBT = recentBorderTemp.size();
	if (sizeRBT == 0) { return; }
	else if (sizeRBT == 1)
	{
		recentBorderTemp.clear();
		activeBorder = recentBorder;
	}
	else
	{
		recentBorderTemp.pop_back();
		activeBorder = recentBorderTemp[sizeRBT - 2];
	}
	im.coordShift(activeBorder, recentMapShift, borderShifted);
	QPainterPath path = pathMake(borderShifted);
	displayBin(qlabel, path);
}
