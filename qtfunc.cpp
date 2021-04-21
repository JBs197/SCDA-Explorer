#include "qtfunc.h"

using namespace std;

void QTFUNC::displayBin(QLabel*& qlabel, string& pathBIN)
{
	string sfile = jfqf.load(pathBIN);
	if (sfile.size() < 1) { err("load-qf.displayBin"); }
	size_t pos1, pos2, posStart, posStop;
	vector<vector<int>> frameCorners, border;
	string temp;
	int row;
	posStart = sfile.find("//frame");
	posStop = sfile.find("//", posStart + 7);
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
	QRect frame = QRect(frameCorners[0][0], frameCorners[0][1], frameCorners[2][0] - frameCorners[0][0], frameCorners[2][1] - frameCorners[0][1]);
	QPainter* painter = new QPainter(&pm);
	QBrush brush = QBrush(Qt::black, Qt::SolidPattern);
	QPen pen = QPen(brush, 4.0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
	painter->setPen(pen);
	painter->drawRect(frame);
	//painter->drawPixmap(0, 0, pm);
	qlabel->setPixmap(pm);
	int height = pm.height();
	int width = pm.width();
	int bbq = 1;
}
void QTFUNC::displayText(QLabel* ql, string stext)
{
	const QString qtemp = QString::fromUtf8(stext);
	ql->setText(qtemp);
}
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
void QTFUNC::initPixmap(QLabel* qlabel)
{
	int width = qlabel->width();
	int height = qlabel->height();
	pm = QPixmap(width, height);
	pm.fill();
}
void QTFUNC::set_display_root(QTreeWidget* name, int val)
{
	map_display_root.insert(name, val);
}

