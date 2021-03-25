#include "qtfunc.h"

using namespace std;

void QTFUNC::display_subt(QTreeWidget* qview, QTreeWidgetItem* qparent)
{
	qview->clear();
	qview->addTopLevelItem(qparent);
	qview->expandAll();
}
void QTFUNC::err(string func)
{
	jfqf.err(func);
}
int QTFUNC::get_display_root(QTreeWidget* name)
{
	int index = map_display_root.value(name, -1);
	return index;
}
void QTFUNC::set_display_root(QTreeWidget* name, int val)
{
	map_display_root.insert(name, val);
}

