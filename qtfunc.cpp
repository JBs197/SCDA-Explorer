#include "stdafx.h"
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
		qtemp = QString::fromUtf8(temp8.c_str());
		mapListPathBin.insert(qtemp, ii);
		qLW->addItem(qtemp);
	}
}
void QTFUNC::displayTable(QTableWidget*& qTable, vector<vector<QString>>& data, vector<vector<string>>& header)
{
	QString qTemp;
	QTableWidgetItem* qCell;
	qTable->clear();
	qTable->setColumnCount(data[0].size());
	qTable->setRowCount(data.size());
	for (int ii = 0; ii < data.size(); ii++)
	{
		for (int jj = 0; jj < data[ii].size(); jj++)
		{
			qCell = new QTableWidgetItem(data[ii][jj]);
			qTable->setItem(ii, jj, qCell);
		}
	}
	QStringList hHeaderLabels, vHeaderLabels;
	if (header.size() > 0)  // Horizontal header.
	{
		for (int ii = 0; ii < header[0].size(); ii++)
		{
			qTemp = QString::fromStdString(header[0][ii]);
			hHeaderLabels.append(qTemp);
		}
		qTable->setHorizontalHeaderLabels(hHeaderLabels);
	}
	if (header.size() > 1)  // Vertical header.
	{
		for (int ii = 0; ii < header[1].size(); ii++)
		{
			qTemp = QString::fromStdString(header[1][ii]);
			vHeaderLabels.append(qTemp);
		}
		qTable->setVerticalHeaderLabels(vHeaderLabels);
	}
}
void QTFUNC::displayTable(QTableWidget*& qTable, vector<vector<string>>& data, vector<vector<string>>& header)
{
	vector<vector<QString>> qData(data.size(), vector<QString>());
	for (int ii = 0; ii < qData.size(); ii++)
	{
		qData[ii].resize(data[ii].size());
		for (int jj = 0; jj < qData[ii].size(); jj++)
		{
			qData[ii][jj] = QString::fromStdString(data[ii][jj]);
		}
	}
	displayTable(qTable, qData, header);
}
void QTFUNC::displayTable(QTableWidget*& qTable, vector<vector<int>>& data, vector<vector<string>>& header)
{
	vector<vector<QString>> qData(data.size(), vector<QString>());
	for (int ii = 0; ii < qData.size(); ii++)
	{
		qData[ii].resize(data[ii].size());
		for (int jj = 0; jj < qData[ii].size(); jj++)
		{
			qData[ii][jj] = QString::number(data[ii][jj]);
		}
	}
	displayTable(qTable, qData, header);
}
void QTFUNC::displayTable(QTableWidget*& qTable, vector<vector<double>>& data, vector<vector<string>>& header)
{
	vector<vector<QString>> qData(data.size(), vector<QString>());
	for (int ii = 0; ii < qData.size(); ii++)
	{
		qData[ii].resize(data[ii].size());
		for (int jj = 0; jj < qData[ii].size(); jj++)
		{
			qData[ii][jj] = QString::number(data[ii][jj]);
		}
	}
	displayTable(qTable, qData, header);
}
void QTFUNC::displayText(QLabel* ql, string stext)
{
	const QString qtemp = QString::fromUtf8(stext.c_str());
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
vector<QPointF> QTFUNC::getCrosshairs(QPointF center, double radius)
{
	vector<QPointF> crosshairs;
	double dTemp = center.x() + radius;
	QPointF qpf(dTemp, center.y());
	while (qpf.x() > center.x() - radius)
	{
		crosshairs.push_back(qpf);
		dTemp = qpf.x() - 1.0;
		qpf.setX(dTemp);
	}
	qpf.setX(center.x());
	dTemp = center.y() + radius;
	qpf.setY(dTemp);
	while (qpf.y() > center.y() - radius)
	{
		crosshairs.push_back(qpf);
		dTemp = qpf.y() - 1.0;
		qpf.setY(dTemp);
	}
	qpf.setX(center.x() - radius);
	qpf.setY(center.y() - radius);
	while (qpf.x() < center.x() + radius)
	{
		crosshairs.push_back(qpf);
		dTemp = qpf.x() + 1.0;
		qpf.setX(dTemp);
	}
	qpf.setX(center.x() + radius);
	while (qpf.y() < center.y() + radius)
	{
		crosshairs.push_back(qpf);
		dTemp = qpf.y() + 1.0;
		qpf.setY(dTemp);
	}
	qpf.setY(center.y() + radius);
	while (qpf.x() > center.x() - radius)
	{
		crosshairs.push_back(qpf);
		dTemp = qpf.x() - 1.0;
		qpf.setX(dTemp);
	}
	qpf.setX(center.x() - radius);
	while (qpf.y() > center.y() - radius)
	{
		crosshairs.push_back(qpf);
		dTemp = qpf.y() - 1.0;
		qpf.setY(dTemp);
	}
	return crosshairs;
}
QBitmap QTFUNC::getEraser(int width)
{
	QPixmap qPM(width, width);
	qPM.fill(Eraser);
	QBitmap qBM(qPM);
	return qBM;
}
int QTFUNC::get_display_root(QTreeWidget* name)
{
	int index = map_display_root.value(name, -1);
	return index;
}
vector<vector<int>> QTFUNC::getTLBR(vector<vector<int>>& borderPath)
{
	vector<vector<int>> TLBR = im.makeBox(borderPath);
	return TLBR;
}
void QTFUNC::initPixmap(QLabel* qlabel)
{
	int width = qlabel->width();
	int height = qlabel->height();
	pmCanvas = QPixmap(width, height);
	pmCanvas.fill();
}
int QTFUNC::loadBinMap(string& pathBin, vector<vector<vector<int>>>& frames, double& scale, vector<double>& position, string& sParent8, vector<vector<int>>& border)
{
	// Load all data into memory from the bin file.
	frames.clear();
	scale = -1.0;
	position.clear();
	sParent8.clear();
	border.clear();
	string sfile = wf.load(pathBin), temp, temp2, temp3, temp4;
	if (sfile.size() < 1) { err("load-qf.loadBinMap"); }
	size_t pos1, pos2, posStart;
	int row, index;
	pos1 = pathBin.rfind('\\') + 1;
	pos2 = pathBin.rfind('.');
	string sName = pathBin.substr(pos1, pos2 - pos1);
	pos1--;
	pos2 = pathBin.rfind('\\', pos1 - 1) + 1;
	string layer = pathBin.substr(pos2, pos1 - pos2);

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
	try { scale = stod(temp); }
	catch (invalid_argument) { jf.err("stod-qf.loadBinMap"); }

	if (layer == "province")
	{
		position = { -1.0, -1.0 };
		sParent8 = "Canada";
	}
	else
	{
		position.resize(2);
		posStart = sfile.find("//position(GPS)");
		if (posStart > sfile.size()) { jf.err("Missing header-qf.loadBinMap"); }
		pos1 = sfile.find('\n', posStart) + 1;
		pos2 = sfile.find(',', pos1);
		temp2 = sfile.substr(pos1, pos2 - pos1);
		pos1 = pos2 + 1;
		pos2 = sfile.find('\n', pos1);
		temp3 = sfile.substr(pos1, pos2 - pos1);
		try
		{
			position[0] = stod(temp2);
			position[1] = stod(temp3);
		}
		catch (invalid_argument) { jf.err("stod-qf.loadBinMap"); }
		
		pos1 = sfile.find("//parent");
		pos1 = sfile.find('\n', pos1) + 1;
		pos2 = sfile.find_first_of("\r\n", pos1);
		sParent8 = sfile.substr(pos1, pos2 - pos1);
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

	int loaded = 0;
	if (frames.size() > 0) { loaded++; }
	if (scale > 0.0) { loaded++; }
	if (position.size() > 0) { loaded++; }
	if (sParent8.size() > 0) { loaded++; }
	if (border.size() > 0) { loaded++; }
	return loaded;
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
vector<double> QTFUNC::makeBlueDot(string& miniPath)
{
	im.pngLoad(miniPath);
	vector<unsigned char> sourceImg, white = { 255, 255, 255 }, blue = { 0, 112, 255 };
	vector<int> sourceDim;
	vector<vector<int>> parentTLBR = im.makeBox(sourceImg, sourceDim, white);
	vector<vector<int>> dotTLBR = im.makeBox(sourceImg, sourceDim, blue);
	double xDot = (double)dotTLBR[0][0] + (((double)dotTLBR[1][0] - (double)dotTLBR[0][0]) / 2.0);
	double yDot = (double)dotTLBR[0][1] + (((double)dotTLBR[1][1] - (double)dotTLBR[0][1]) / 2.0);
	double pWidth = (double)(parentTLBR[1][0] - parentTLBR[0][0]);
	double pHeight = (double)(parentTLBR[1][1] - parentTLBR[0][1]);
	xDot -= (double)parentTLBR[0][0];
	yDot -= (double)parentTLBR[0][1];
	vector<double> blueDot(2);
	blueDot[0] = xDot / pWidth;
	if (blueDot[0] < 0.0) 
	{ 
		if (blueDot[0] > -0.1) { blueDot[0] = 0.0; }
		else { jf.err("xDot-qf.makeBlueDot"); }		 
	}
	else if (blueDot[0] > 1.0)
	{
		if (blueDot[0] < 1.1) { blueDot[0] = 1.0; }
		else { jf.err("xDot-qf.makeBlueDot"); }
	}
	blueDot[1] = yDot / pHeight;
	if (blueDot[1] < 0.0)
	{
		if (blueDot[1] > -0.1) { blueDot[1] = 0.0; }
		else { jf.err("yDot-qf.makeBlueDot"); }
	}
	else if (blueDot[1] > 1.0)
	{
		if (blueDot[1] < 1.1) { blueDot[1] = 1.0; }
		else { jf.err("yDot-qf.makeBlueDot"); }
	}	
	return blueDot;
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
void QTFUNC::populateTree(QTreeWidget*& qTree, JTREE& jt, int columns)
{
	// columns = 1 -> text only, columns = 2 -> text and integer
	treeColumns = columns;
	qTree->clear();
	QTreeWidgetItem* qRoot = new QTreeWidgetItem(0);      // Type indicates 
	QString qTemp = QString::fromStdString(jt.nameRoot);  // layers removed
	qRoot->setText(0, qTemp);                             // from root.
	populateTreeWorker(qRoot, jt, 0);
	qTree->addTopLevelItem(qRoot);
}
void QTFUNC::populateTreeWorker(QTreeWidgetItem*& qNode, JTREE& jt, int myIndex)
{
	// Note that 'myIndex' refers to the parent's internal index within JTREE.
	QString qTemp;
	wstring wTemp;
	string temp;
	bool removePath = jt.getRemovePath();
	int childIndex, myGeneration = jt.treeSTanc[myIndex].size();
	if (myIndex == 0) { myGeneration = 0; }
	for (int ii = 0; ii < jt.treeSTdes[myIndex].size(); ii++)
	{
		QTreeWidgetItem* qChild = new QTreeWidgetItem(qNode, myGeneration + 1);
		childIndex = jt.treeSTdes[myIndex][ii];
		if (removePath)
		{
			temp = jf.nameFromPath(jt.treePL[childIndex]);
			wTemp = jf.utf8to16(temp);
		}
		else { wTemp = jf.utf8to16(jt.treePL[childIndex]); }
		qTemp = QString::fromStdWString(wTemp);
		qChild->setText(0, qTemp);
		if (treeColumns > 1)
		{
			qTemp = QString::number(jt.treePLi[childIndex]);
			qChild->setText(1, qTemp);
		}
		populateTreeWorker(qChild, jt, childIndex);
	}
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

