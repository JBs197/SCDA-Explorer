#pragma once

#include <QtCore>
#include <QColumnView>
#include <QListWidget>
#include <QListView>
#include <QTableWidget>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QLabel>
#include <QPainter>
#include <QPainterPath>
#include <QBitmap>
#include "jtree.h"
#include "imgfunc.h"
#include "winfunc.h"

using namespace std;

class QTFUNC
{
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
	QPixmap pmCanvas, pmPainting;
	vector<vector<int>> recentBorder, frameCorners;
	vector<vector<vector<int>>> recentBorderTemp;
	vector<double> recentMapShift;
	QRectF rect;
	int treeColumns;
	WINFUNC wf;

	QColor Eraser = QColor(233, 188, 181);
	vector<unsigned char> Violet = { 127, 0, 255 };

public:
	QTFUNC() {}
	~QTFUNC() {}
	int deleteChildren(QTreeWidgetItem*& qNode);
	int deleteLeaves(QTreeWidgetItem*& qNode);
	void displayBinList(QListWidget*& qLW, vector<string>& pathBin);
	void displayDebug(QLabel*& qlabel, vector<string>& pathPNG, vector<vector<int>>& debugMapCoord);
	void displayPainterPath(QLabel*& qlabel, QPainterPath& path);
	void displayTable(QTableWidget*& qTable, vector<vector<QString>>& data, vector<vector<string>>& header);
	void displayTable(QTableWidget*& qTable, vector<vector<string>>& data, vector<vector<string>>& header);
	void displayTable(QTableWidget*& qTable, vector<vector<int>>& data, vector<vector<string>>& header);
	void displayTable(QTableWidget*& qTable, vector<vector<double>>& data, vector<vector<string>>& header);
	void displayText(QLabel*, string stext);
	void display_subt(QTreeWidget*, QTreeWidgetItem*);
	void drawDotsDebug(QPainter& qpaint, vector<vector<double>>& dots);
	void drawFrame(QPixmap& pm, vector<vector<int>>& topleftBotright);
	void drawLinesDebug(QPainter& qpaint, vector<vector<double>>& lines);
	void eraser(QLabel*& qlabel, vector<vector<int>> TLBR);
	void err(string);
	int getBranchGen(QTreeWidgetItem*& qBranch);
	string getBranchPath(QTreeWidgetItem*& qBranch, string rootDir);
	vector<QPointF> getCrosshairs(QPointF center, double radius);
	int getLastMap() { return lastMap; }
	QBitmap getEraser(int width);
	int get_display_root(QTreeWidget*);
	vector<vector<int>> getTLBR(vector<vector<int>>& borderPath);
	void initPixmap(QLabel* qlabel);
	int loadBinMap(string& pathBin, vector<vector<vector<int>>>& frames, double& scale, vector<double>& position, string& sParent, vector<vector<int>>& border);
	vector<vector<int>> loadDebugMapCoord(string& pathBin);
	vector<double> makeBlueDot(string& miniPath);
	string makePathTree(QTreeWidgetItem*& qBranch);
	vector<double> makeShift(QLabel*& qlabel, vector<vector<int>>& frameBorderTLBR);
	QPainterPath pathMakeCircle(vector<double> origin, double radius, int sides);
	void pmPainterReset(QLabel*& qlabel);
	void populateTree(QTreeWidget*& qTree, JTREE& jt, int columns);
	void populateTreeWorker(QTreeWidgetItem*& qTree, JTREE& jt, int myIndex);
	void printEditedMap(string& pathBin);
	void setDebugMapPath(string spath);
	void set_display_root(QTreeWidget*, int);
	void undoEraser(QLabel*& qlabel);

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
		QString qtemp = QString::fromUtf8(defaultDebugMapPathSelected.c_str());
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
		QString qtemp = QString::fromUtf8(defaultDebugMapPathSelected.c_str());
		QImage qimg = QImage(qtemp);
		QPixmap qpm = QPixmap::fromImage(qimg);
		qlabel->setPixmap(qpm);
		int bbq = 1;
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

