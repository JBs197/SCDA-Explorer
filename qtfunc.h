#pragma once

#include <QtCore>
#include <QColumnView>
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
	int diameterDefault = 5;
	IMGFUNC im;
	JFUNC jfqf;
	QMap<QTreeWidget*, int> map_display_root;
	QPainter painter;
	QPen pen;
	QPixmap pmCanvas, pmPainting;
	QRectF rect;

public:
	explicit QTFUNC() {}
	~QTFUNC() {}
	void displayBin(QLabel*& qlabel, string& pathBIN);
	void displayDebug(QLabel*& qlabel, string& pathPNG);
	void displayPainterPath(QLabel*& qlabel, QPainterPath& path);
	void displayText(QLabel*, string stext);
	void display_subt(QTreeWidget*, QTreeWidgetItem*);
	void drawDotsDebug(QPainter& qpaint, vector<vector<double>>& dots);
	void drawFrame(QPixmap& pm, vector<vector<int>>& topleftBotright);
	void drawLinesDebug(QPainter& qpaint, vector<vector<double>>& lines);
	void initPixmap(QLabel* qlabel);
	void err(string);
	int get_display_root(QTreeWidget*);
	QPainterPath pathMakeCircle(vector<double> origin, double radius, int sides);
	void pmPainterReset(QLabel*& qlabel);
	void set_display_root(QTreeWidget*, int);

	// TEMPLATES
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
			wtemp = jfqf.utf8to16(tree_pl[ii]);
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
		err("dotsMake template-qf");
	}
	template<> vector<vector<int>> dotsMake<vector<vector<double>>, vector<int>>(vector<vector<double>>& coords, vector<int>& colours)
	{
		if (coords.size() > colours.size()) { err("coord-colour mismatch-qf.dotsMake"); }
		vector<vector<int>> dots(coords.size(), vector<int>(3));
		for (int ii = 0; ii < coords.size(); ii++)
		{
			dots[ii][0] = int(coords[ii][0]);
			dots[ii][1] = int(coords[ii][1]);
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
			qcolour = QColor(255, 255, 255);
			break;
		case 1:
			qcolour = QColor(255, 0, 0);
			break;
		case 2:
			qcolour = QColor(255, 255, 0);
			break;
		case 3:
			qcolour = QColor(0, 255, 0);
			break;
		case 4:
			qcolour = QColor(0, 255, 255);
			break;
		case 5:
			qcolour = QColor(0, 0, 255);
			break;
		case 6:
			qcolour = QColor(255, 0, 255);
			break;
		case 7:
			qcolour = QColor(0, 0, 0);
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
	template<> QPainterPath pathMake<vector<vector<int>>>(vector<vector<int>>& coordList)
	{
		QPainterPath path;
		path.moveTo((double)coordList[0][0], (double)coordList[0][0]);
		for (int ii = 1; ii < coordList.size(); ii++)
		{
			path.lineTo((double)coordList[ii][0], (double)coordList[ii][1]);
		}
		path.closeSubpath();
		return path;
	}
	template<> QPainterPath pathMake<vector<vector<double>>>(vector<vector<double>>& coordList)
	{
		QPainterPath path;
		path.moveTo(coordList[0][0], coordList[0][0]);
		for (int ii = 1; ii < coordList.size(); ii++)
		{
			path.lineTo(coordList[ii][0], coordList[ii][1]);
		}
		path.closeSubpath();
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
		if (coordListShifted[0] != coordListShifted[coordListShifted.size() - 1])
		{
			path.lineTo(coordListShifted[0][0], coordListShifted[0][1]);
		}
		return path;
	}


};

