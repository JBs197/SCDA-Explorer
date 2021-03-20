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

public:
	explicit QTFUNC() {}
	~QTFUNC() {}
	void err(string);

	// TEMPLATES
	template<typename Q> void display(Q*, vector<vector<int>>&, vector<string>&) 
	{
		// Note that this function will automatically render the root invisible if there is only one root element. 
	}
	template<> void display<QTreeWidget>(QTreeWidget* qview, vector<vector<int>>& tree_st, vector<string>& tree_pl)
	{
		string temp1;
		QString qtemp;
		QTreeWidgetItem *qitem, *qparent;
		QList<QTreeWidgetItem*> qroots, qregistry;
		int pivot, iparent;

		// Make a list of the tree's root elements.
		vector<int> roots;
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
					pivot = 0;
				}
			}
			if (pivot == 0)
			{
				roots.push_back(ii);
			}
		}

		// Establish which parameters are redundant, and add them as hidden root items.
		vector<string> redundant_params;
		vector<string> params;  // First entry is full name.
		temp1 = tree_pl[roots[0]];
		params = jf_qf.list_from_marker(temp1, '$');
		if (roots.size() == 1)
		{
			redundant_params.assign(params.begin() + 1, params.end());
		}
		else if (roots.size() > 1)
		{
			redundant_params.assign(params.begin() + 1, params.end() - 1);
		}
		for (int ii = 0; ii < redundant_params.size(); ii++)
		{
			qtemp = QString::fromStdString(redundant_params[ii]);
			if (ii == 0)
			{
				qitem = new QTreeWidgetItem();
				qitem->setText(0, qtemp);
				qview->addTopLevelItem(qitem);
			}
			else
			{
				qparent = qitem;
				qitem = new QTreeWidgetItem(qparent);
				qitem->setText(0, qtemp);
			}
		}


		// Populate the widget, only displaying each item's final parameter.
		for (int ii = 0; ii < tree_st.size(); ii++) 
		{
			params = jf_qf.list_from_marker(tree_pl[ii], '$');
			temp1 = params[params.size() - 1];
			qtemp = QString::fromStdString(temp1);

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
		if (qroots.size() > 1)
		{
			qview->addTopLevelItems(qroots);
		}
		else
		{
			QList<QTreeWidgetItem*> qTNG = qroots[0]->takeChildren();
			qview->addTopLevelItems(qTNG);
		}

		// Tidy up.
		if (redundant_params.size() > 0)
		{
			qitem = qview->topLevelItem(0);
			qitem->setHidden(1);
		}
		if (tree_st.size() <= 20)
		{
			qview->expandAll();
		}
	}

};

