#ifndef QT_GENERAL_H
#define QT_GENERAL_H
#pragma once

#include <QObject>
#include <QString>
#include <QTreeWidgetItem>
#include <QStandardItemModel>
#include <QTableView>

using namespace std;

template<typename T> vector<int> tree_framing(T&, int, int, int);
void add_children(QTreeWidgetItem*, QVector<QVector<QString>>&);
template<typename T> void populate_qtreewidget(QTreeWidget&, T&, int);


// Use an N-dimensional std::vector of std::strings to populate a QTreeWidget, with one dimension per generation in the tree. 
template<typename T> void populate_qtreewidget(QTreeWidget& qtw, T& stree, int dimensions)
{
    vector<vector<int>> index_list;
    vector<int> index_row;
    for (int ii = 0; ii < dimensions; ii++)
    {
        index_row = qt
    }
    qtree_dimensions(qtree, dimensions);


    QTreeWidgetItem* item;
    QVector<QTreeWidgetItem*> qroots;
    switch (window)  // Window:  1 = Catalogues on Drive, 2 = Catalogues in Database
    {
    case 1:
        ui->TW_cataondrive->clear();
        for (int ii = 0; ii < qtree.size(); ii++)
        {
            item = new QTreeWidgetItem(ui->TW_cataondrive);  // For every year, make a root pointer.
            item->setText(0, qtree[ii][0][0]);
            item->setText(1, " ");
            auto item_flags = item->flags();
            item_flags.setFlag(Qt::ItemIsSelectable, false);
            item->setFlags(item_flags);
            qroots.append(item);
            add_children(item, qtree[ii]);
        }
        ui->TW_cataondrive->addTopLevelItems(qroots);
        break;
    case 2:
        ui->TW_cataindb->clear();
        for (int ii = 0; ii < qtree.size(); ii++)
        {
            item = new QTreeWidgetItem(ui->TW_cataindb);  // For every year, make a root pointer.
            item->setText(0, qtree[ii][0][0]);
            item->setText(1, " ");
            item->setText(2, " ");
            auto item_flags = item->flags();
            item_flags.setFlag(Qt::ItemIsSelectable, false);
            item->setFlags(item_flags);
            qroots.append(item);
            add_children(item, qtree[ii]);
        }
        ui->TW_cataindb->addTopLevelItems(qroots);
        break;
    }
}

#endif