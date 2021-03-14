#pragma once

#include <QtCore>
#include <QColumnView>
#include <QListView>
#include <QTableView>
#include <QTreeWidget>

using namespace std;

class QTFUNC
{


public:
	explicit QTFUNC() {}
	~QTFUNC() {}


	// TEMPLATES
	template<typename Q> void display(Q*, vector<vector<int>>&, vector<string>&) {}
	template<> void display<QTreeWidget>(QTreeWidget* qview, vector<vector<int>>& tree_st, vector<string>& tree_pl)
	{
		string temp1;
		QString qtemp;
		QTreeWidgetItem *qitem, *qparent;
		QList<QTreeWidgetItem*> qroots, qitems;
		vector<int> first_gen(tree_st[0].begin() + 1, tree_st[0].end());  // List of the root's direct children.
		int pivot;
		QMap<int, QTreeWidgetItem*> map_qitems;

		for (int ii = 1; ii < tree_st.size(); ii++)  // ii = pl_index
		{
			// For every (non-root) node, determine its parent. Then, set its text.
			for (int jj = 0; jj < tree_st[ii].size(); jj++) 
			{
				if (tree_st[ii][jj] < 0)
				{
					pivot = jj;
					break;
				}
			}
			temp1 = tree_pl[ii];
			qtemp = QString::fromStdString(temp1);			
			if (tree_st[ii][pivot - 1] == 0) // If root is parent...
			{
				qitem = new QTreeWidgetItem();
				qitem->setText(0, qtemp);
				qroots.append(qitem);
			}
			else
			{
				qparent = map_qitems.value(ii);
				qitem = new QTreeWidgetItem(qparent);
				qitem->setText(0, qtemp);
			}

			if (tree_st[ii].size() > pivot + 1)  // If this node has children...
			{
				map_qitems.insert(ii, qitem);
			}
		}
		qview->addTopLevelItems(qroots);
	}
};

