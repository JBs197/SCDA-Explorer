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
	JFUNC jf_qf;
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
		// Note that this function will automatically render the root invisible if there is only one root element. 
	}
	template<> void display<QTreeWidget>(QTreeWidget* qview, vector<vector<int>>& tree_st, vector<string>& tree_pl)
	{
		qview->clear();
		string temp1;
		QString qtemp;
		QTreeWidgetItem *qitem, *qparent;
		QList<QTreeWidgetItem*> qroots, qregistry, qTNG;
		int pivot, iparent;

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

		// Tidy up.
		if (tree_st.size() <= 20)
		{
			qview->expandAll();
		}
	}

};

