#pragma once

#include <QtCore>
#include <QColumnView>
#include <QListWidget>
#include <QListView>
#include <QTableView>
#include <QTreeWidget>
#include <QLabel>
#include <QPainter>
#include <QPainterPath>
#include "imgfunc.h"

using namespace std;

class QTFUNC
{
	QBrush brush;
	QColor colourDefault = Qt::black;
	string defaultDebugMapPath, defaultDebugMapPathSelected;
	int defaultDotWidth = -1;
	double defaultMapZoom = 3.0;
	int diameterDefault = 5;
	IMGFUNC im;
	JFUNC jf;
	int lastMap = -1;  // -1 = nothing loaded yet, 0 = debug map, 1 = bin map
	vector<string> listPathBin;
	QMap<QString, int> mapListPathBin;
	QMap<QTreeWidget*, int> map_display_root;
	QPainter painter;
	QPen pen;
	QPixmap pmCanvas, pmPainting;
	QRectF rect;

	vector<unsigned char> Violet = { 127, 0, 255 };

public:
	explicit QTFUNC() {}
	~QTFUNC() {}
	void displayBinList(QListWidget*& qLW, vector<string>& pathBin);
	void displayDebug(QLabel*& qlabel, vector<string>& pathPNG, vector<vector<int>>& debugMapCoord);
	void displayPainterPath(QLabel*& qlabel, QPainterPath& path);
	void displayText(QLabel*, string stext);
	void display_subt(QTreeWidget*, QTreeWidgetItem*);
	void drawDotsDebug(QPainter& qpaint, vector<vector<double>>& dots);
	void drawFrame(QPixmap& pm, vector<vector<int>>& topleftBotright);
	void drawLinesDebug(QPainter& qpaint, vector<vector<double>>& lines);
	void err(string);
	int getBranchGen(QTreeWidgetItem*& qBranch);
	string getBranchPath(QTreeWidgetItem*& qBranch, string rootDir);
	int getLastMap() { return lastMap; }
	int get_display_root(QTreeWidget*);
	void initPixmap(QLabel* qlabel);
	vector<vector<int>> loadDebugMapCoord(string& pathBin);
	QPainterPath pathMakeCircle(vector<double> origin, double radius, int sides);
	void pmPainterReset(QLabel*& qlabel);
	void setDebugMapPath(string spath);
	void set_display_root(QTreeWidget*, int);

	// TEMPLATES
	template<typename ... Args> void debugMapSelected(QLabel*& qlabel, Args ... args)
	{
		jf.err("debugMapSelected template-qf");
	}
	template<> void debugMapSelected<int, vector<vector<int>>>(QLabel*& qlabel, int index, vector<vector<int>> debugMapCoord)
	{
		if (defaultDotWidth < 0) { defaultDotWidth = im.getDotWidth(); }
		if (defaultDebugMapPath.size() < 1) { setDebugMapPath(im.getMapPath(0)); }
		vector<int> mapDim(2), pixelCoord(2);
		int numComponents, offset, halfDot = defaultDotWidth / 2;
		pixelCoord[0] = debugMapCoord[index][0] - debugMapCoord[0][0] - halfDot;
		pixelCoord[1] = debugMapCoord[index][1] - debugMapCoord[0][1] - halfDot;
		unsigned char* dataTemp = stbi_load(defaultDebugMapPath.c_str(), &mapDim[0], &mapDim[1], &numComponents, 0);
		for (int ii = 0; ii < defaultDotWidth; ii++)
		{
			offset = im.getOffset(pixelCoord, mapDim[0]);
			for (int jj = 0; jj < defaultDotWidth; jj++)
			{
				dataTemp[offset] = Violet[0];
				offset++;
				dataTemp[offset] = Violet[1];
				offset++;
				dataTemp[offset] = Violet[2];
				offset++;
			}
			pixelCoord[1]++;
		}
		int error = stbi_write_png(defaultDebugMapPathSelected.c_str(), mapDim[0], mapDim[1], numComponents, dataTemp, 0);
		QString qtemp = QString::fromUtf8(defaultDebugMapPathSelected);
		QImage qimg = QImage(qtemp);
		QPixmap qpm = QPixmap::fromImage(qimg);
		qlabel->setPixmap(qpm);
		int bbq = 1;
	}
	template<> void debugMapSelected<vector<int>, vector<vector<int>>>(QLabel*& qlabel, vector<int> manualCoord, vector<vector<int>> debugMapCoord)
	{
		if (defaultDotWidth < 0) { defaultDotWidth = im.getDotWidth(); }
		if (defaultDebugMapPath.size() < 1) { setDebugMapPath(im.getMapPath(0)); }
		vector<int> mapDim(2), pixelCoord(2);
		int numComponents, offset, halfDot = defaultDotWidth / 2;
		pixelCoord[0] = manualCoord[0] - debugMapCoord[0][0] - halfDot;
		pixelCoord[1] = manualCoord[1] - debugMapCoord[0][1] - halfDot;
		unsigned char* dataTemp = stbi_load(defaultDebugMapPath.c_str(), &mapDim[0], &mapDim[1], &numComponents, 0);
		for (int ii = 0; ii < defaultDotWidth; ii++)
		{
			offset = im.getOffset(pixelCoord, mapDim[0]);
			for (int jj = 0; jj < defaultDotWidth; jj++)
			{
				dataTemp[offset] = Violet[0];
				offset++;
				dataTemp[offset] = Violet[1];
				offset++;
				dataTemp[offset] = Violet[2];
				offset++;
			}
			pixelCoord[1]++;
		}
		int error = stbi_write_png(defaultDebugMapPathSelected.c_str(), mapDim[0], mapDim[1], numComponents, dataTemp, 0);
		QString qtemp = QString::fromUtf8(defaultDebugMapPathSelected);
		QImage qimg = QImage(qtemp);
		QPixmap qpm = QPixmap::fromImage(qimg);
		qlabel->setPixmap(qpm);
		int bbq = 1;
	}

	template<typename Q> void display(Q*, vector<vector<int>>&, vector<string>&) 
	{
		err("display template-qf");
	}
	template<> void display<QTreeWidget>(QTreeWidget* qview, vector<vector<int>>& tree_st, vector<string>& tree_pl)
	{
		qview->clear();
		string temp1;
		wstring wtemp;
		QString qtemp;
		QTreeWidgetItem *qitem, *qparent;
		QList<QTreeWidgetItem*> qroots, qregistry, qTNG;
		int pivot, iparent;

		// Make a qitem for every node in the tree.
		for (int ii = 0; ii < tree_st.size(); ii++)
		{
			wtemp = jf.utf8to16(tree_pl[ii]);
			qtemp = QString::fromStdWString(wtemp);
			qtemp = QString::fromUtf8(tree_pl[ii]);
			qitem = new QTreeWidgetItem();
			qitem->setText(0, qtemp);
			qregistry.append(qitem);
		}

		// Add the roots to their list, and add every node's children to it.
		for (int ii = 0; ii < tree_st.size(); ii++)
		{
			for (int jj = 0; jj < tree_st[ii].size(); jj++)
			{
				if (tree_st[ii][jj] < 0)
				{
					pivot = jj;
					break;
				}
				else if (jj == tree_st[ii].size() - 1)
				{
					for (int kk = 0; kk < tree_st[ii].size(); kk++)
					{
						if (tree_st[ii][kk] == 0)
						{
							pivot = kk;
							break;
						}
					}
				}
			}
			if (pivot == 0)
			{
				qroots.append(qregistry[ii]);
			}
			if (tree_st[ii].size() > pivot + 1)  // If this node has children...
			{
				for (int jj = pivot + 1; jj < tree_st[ii].size(); jj++)
				{
					qregistry[ii]->addChild(qregistry[tree_st[ii][jj]]);
				}
			}
		}
		qview->addTopLevelItems(qroots);


		/*
		for (int ii = 0; ii < tree_st.size(); ii++) 
		{
			qtemp = QString::fromStdString(tree_pl[ii]);

			// For every node, determine its parent. Then, set its text.
			for (int jj = 0; jj < tree_st[ii].size(); jj++) 
			{
				if (tree_st[ii][jj] < 0)
				{
					pivot = jj;
					break;
				}
				else if (jj == tree_st[ii].size() - 1)
				{
					pivot = 0;
				}
			}
			if (pivot == 0)  // If this node is (part of) the root...
			{
				qitem = new QTreeWidgetItem();
				qitem->setText(0, qtemp);
				qroots.append(qitem);
				qregistry.append(qitem);
			}
			else
			{
				iparent = tree_st[ii][pivot - 1];
				qparent = qregistry[iparent];
				qitem = new QTreeWidgetItem(qparent);
				qitem->setText(0, qtemp);
				qregistry.append(qitem);
			}
		}
		int display_root = get_display_root(qview);
		if (display_root == 1)
		{
			qview->addTopLevelItems(qroots);
		}
		else if (display_root == 0)
		{
			for (int ii = 0; ii < qroots.size(); ii++)
			{
				qTNG.append(qroots[ii]->takeChildren());
				qroots[ii]->setHidden(1);
				qview->addTopLevelItem(qroots[ii]);
			}
			qview->addTopLevelItems(qTNG);
		}
		else
		{
			err("display_root-qt.display");
		}
		*/

		// Tidy up.
		if (tree_st.size() <= 20)
		{
			qview->expandAll();
		}
	}

	template<typename ... Args> void displayBin(QLabel*& qlabel, Args& ... args)
	{
		jf.err("displayBin template-qf");
	}
	template<> void displayBin<string>(QLabel*& qlabel, string& pathBIN)
	{
		jf.timerStart();
		// Load all coordinates into memory from the bin file.
		string sfile = jf.load(pathBIN);
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
		pmPainterReset(qlabel);
		pen.setWidth(5);
		QPainterPath path = pathMake(borderShifted);
		painter.drawPath(path);
		qlabel->setPixmap(pmPainting);

		long long timer = jf.timerStop();
		qDebug() << "Time to displayBin from file: " << timer;
		lastMap = 1;
	}
	template<> void displayBin<QString>(QLabel*& qlabel, QString& qName)
	{
		int listIndex = mapListPathBin.value(qName, -1);
		if (listIndex < 0) { jf.err("mapListPathBin-qf.displayBin"); }
		string pathBIN = listPathBin[listIndex];
		displayBin(qlabel, pathBIN);
	}
	template<> void displayBin<QPainterPath>(QLabel*& qlabel, QPainterPath& painterPathBorder)
	{
		// Paint the coordinates onto the GUI window.
		pmPainterReset(qlabel);
		pen.setWidth(5);
		painter.drawPath(painterPathBorder);
		qlabel->setPixmap(pmPainting);
		QCoreApplication::processEvents();
		lastMap = 1;
	}

	template<typename ... Args> void displayPainterPathDots(QLabel*& qlabel, QPainterPath& path, Args& ... args)
	{
		// On a given QLabel, draw a filled circle at each of the given coordinates.
		// If no colour list is given, all points are drawn in default red. Otherwise,
		// dots are drawn using listed RGB. Listed colours will be paired (by
		// index) to listed coordinates. If the list of coordinates exceeds the list
		// of colours, then the overflow points receive the last colour given. 
		err("displayDots template-qf");
	}
	template<> void displayPainterPathDots<vector<vector<int>>>(QLabel*& qlabel, QPainterPath& path, vector<vector<int>>& dots)
	{
		pmPainterReset(qlabel);
		painter.setPen(pen);
		painter.drawPath(path);
		for (int ii = 0; ii < dots.size(); ii++)
		{
			brush.setColor(getColour(dots[ii][2]));
			painter.fillRect(dots[ii][0] - (diameterDefault / 2), dots[ii][1] - (diameterDefault / 2), diameterDefault, diameterDefault, brush);
		}
		qlabel->setPixmap(pmPainting);
	}

	template<typename ... Args> vector<vector<int>> dotsMake(Args& ... args)
	{
		// For the given colours, make that many dots (coords + colour), starting 
		// with the most recent point and reaching back as far as necessary.
		err("dotsMake template-qf");
	}
	template<> vector<vector<int>> dotsMake<vector<vector<double>>, vector<int>>(vector<vector<double>>& coords, vector<int>& colours)
	{
		if (coords.size() < colours.size()) { err("coord-colour mismatch-qf.dotsMake"); }
		int offset = coords.size() - colours.size();
		vector<vector<int>> dots(colours.size(), vector<int>(3));
		for (int ii = 0; ii < colours.size(); ii++)
		{
			dots[ii][0] = int(coords[offset + ii][0]);
			dots[ii][1] = int(coords[offset + ii][1]);
			dots[ii][2] = colours[ii];
		}
		return dots;
	}

	template<typename ... Args> QColor getColour(Args& ... args)
	{
		// icolour code: 0=white, 1=red, 2=yellow, 3=green, 4=teal, 5=blue, 6=pink, 7+=black
		err("getColour template-qf");
	}
	template<> QColor getColour<int>(int& icolour)
	{
		QColor qcolour;
		if (icolour > 7) { icolour = 7; }
		switch (icolour)
		{
		case 0:
			qcolour = QColor(255, 255, 255);  // White
			break;
		case 1:
			qcolour = QColor(255, 0, 0);      // Red
			break;
		case 2:
			qcolour = QColor(255, 255, 0);    // Yellow
			break;
		case 3:
			qcolour = QColor(0, 255, 0);      // Green
			break;
		case 4:
			qcolour = QColor(0, 255, 255);    // Teal
			break;
		case 5:
			qcolour = QColor(0, 0, 255);      // Blue
			break;
		case 6:
			qcolour = QColor(255, 0, 255);    // Pink
			break;
		case 7:
			qcolour = QColor(0, 0, 0);        // Black
			break;
		}
		return qcolour;
	}
	template<> QColor getColour<vector<unsigned char>>(vector<unsigned char>& vcolour)
	{
		QColor qcolour = QColor(vcolour[0], vcolour[1], vcolour[2]);
		return qcolour;
	}

	template<typename ... Args> QPainterPath pathMake(Args& ... args)
	{
		err("pathMake template-qf");
	}
	template<> QPainterPath pathMake<vector<vector<int>>>(vector<vector<int>>& iCoordList)
	{
		vector<vector<double>> coordList;
		jf.toDouble(iCoordList, coordList);
		QPainterPath path = pathMake(coordList);
		return path;
	}
	template<> QPainterPath pathMake<vector<vector<double>>>(vector<vector<double>>& coordList)
	{
		QPainterPath path;
		path.moveTo(coordList[0][0], coordList[0][1]);
		for (int ii = 1; ii < coordList.size(); ii++)
		{
			path.lineTo(coordList[ii][0], coordList[ii][1]);
		}
		return path;
	}
	template<> QPainterPath pathMake<vector<vector<double>>, vector<double>>(vector<vector<double>>& coordList, vector<double>& DxDyGa)
	{
		vector<vector<double>> coordListShifted(coordList.size(), vector<double>(2));
		for (int ii = 0; ii < coordList.size(); ii++)
		{
			coordListShifted[ii][0] = (coordList[ii][0] + DxDyGa[0]) * DxDyGa[2];
			coordListShifted[ii][1] = (coordList[ii][1] + DxDyGa[1]) * DxDyGa[2];
		}
		QPainterPath path;
		path.moveTo(coordListShifted[0][0], coordListShifted[0][0]);
		for (int ii = 1; ii < coordListShifted.size(); ii++)
		{
			path.lineTo(coordListShifted[ii][0], coordListShifted[ii][1]);
		}
		return path;
	}


};

