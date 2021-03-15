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

		for (int ii = 0; ii < tree_st.size(); ii++)  // ii = pl_index
		{
			temp1 = tree_pl[ii];
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
			qDebug() << "root has children: " << qroots[0]->childCount();
			QList<QTreeWidgetItem*> qTNG = qroots[0]->takeChildren();
			qDebug() << "root has children: " << qroots[0]->childCount();
			qview->addTopLevelItems(qTNG);
		}
	}


};

