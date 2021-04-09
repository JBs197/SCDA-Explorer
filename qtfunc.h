#pragma once

#include <QtCore>
#include <QColumnView>
#include <QListView>
#include <QTableView>
#include <QTreeWidget>
#include "jfunc.h"

using namespace std;

class QTFUNC
{
	JFUNC jfqf;
	QMap<QTreeWidget*, int> map_display_root;

public:
	explicit QTFUNC() {}
	~QTFUNC() {}
	void display_subt(QTreeWidget*, QTreeWidgetItem*);
	void err(string);
	int get_display_root(QTreeWidget*);
	void set_display_root(QTreeWidget*, int);

	// TEMPLATES
	template<typename Q> void display(Q*, vector<vector<int>>&, vector<string>&) 
	{
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



};

