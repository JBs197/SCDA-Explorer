#include "SCDAcompare.h"

using namespace std;

SCDAcompare::SCDAcompare(QWidget* parent) : QWidget(parent) 
{
	initColour();

	QVBoxLayout* vLayout = new QVBoxLayout;
	this->setLayout(vLayout);

	QHBoxLayout* textLayout = new QHBoxLayout;
	vLayout->addLayout(textLayout);
	QLabel* labelMarker = new QLabel("Marker chars: ");
	textLayout->insertWidget(indexText::Marker - 1, labelMarker, 0, Qt::AlignCenter);
	QLineEdit* leMarker = new QLineEdit;
	textLayout->insertWidget(indexText::Marker, leMarker, 0, Qt::AlignCenter);
	textLayout->insertSpacing(indexText::Marker + 1, 20);
	
	QLabel* labelTitle = new QLabel("Title: ");
	textLayout->insertWidget(indexText::Title - 1, labelTitle, 0, Qt::AlignCenter);
	QLineEdit* leTitle = new QLineEdit;
	textLayout->insertWidget(indexText::Title, leTitle, 0, Qt::AlignCenter);
	textLayout->insertSpacing(indexText::Title + 1, 20);

	QLabel* labelText = new QLabel("Text: ");
	textLayout->insertWidget(indexText::Input - 1, labelText, 0, Qt::AlignCenter);
	QLineEdit* leText = new QLineEdit;
	textLayout->insertWidget(indexText::Input, leText, 1, Qt::AlignVCenter);
	QPushButton* pbAdd = new QPushButton("Add");
	textLayout->insertWidget(indexText::Add, pbAdd, 0, Qt::AlignCenter);
	connect(pbAdd, &QPushButton::clicked, this, &SCDAcompare::addText);

	QHBoxLayout* buttonLayout = new QHBoxLayout;
	vLayout->addLayout(buttonLayout);
	QPushButton* pbRemove = new QPushButton("Remove Column");
	buttonLayout->addWidget(pbRemove);
	connect(pbRemove, &QPushButton::clicked, this, &SCDAcompare::removeCol);
	QPushButton* pbShiftUp = new QPushButton("Shift Up");
	buttonLayout->addWidget(pbShiftUp);
	connect(pbShiftUp, &QPushButton::clicked, this, &SCDAcompare::shiftUp);
	QPushButton* pbShiftDown = new QPushButton("Shift Down");
	buttonLayout->addWidget(pbShiftDown);
	connect(pbShiftDown, &QPushButton::clicked, this, &SCDAcompare::shiftDown);
	QPushButton* pbDownload = new QPushButton("Download Page");
	buttonLayout->addWidget(pbDownload);
	connect(pbDownload, &QPushButton::clicked, this, &SCDAcompare::downloadColPage);

	QTableWidget* table = new QTableWidget(0, 0);
	vLayout->addWidget(table, 100);

	vLayout->addStretch(1);
}

void SCDAcompare::addText()
{
	QVBoxLayout* vLayout = (QVBoxLayout*)this->layout();
	QLayoutItem* qlItem = vLayout->itemAt(index::Text);
	QHBoxLayout* textLayout = (QHBoxLayout*)qlItem->layout();
	qlItem = textLayout->itemAt(indexText::Marker);
	QLineEdit* leMarker = (QLineEdit*)qlItem->widget();
	QString qsMarker = leMarker->text();
	string marker = qsMarker.toStdString();

	qlItem = textLayout->itemAt(indexText::Title);
	QLineEdit* leTitle = (QLineEdit*)qlItem->widget();
	QString qsTitle = leTitle->text();
	string title = qsTitle.toStdString();
	leTitle->clear();
	
	qlItem = textLayout->itemAt(indexText::Input);
	QLineEdit* leText = (QLineEdit*)qlItem->widget();
	QString qsText = leText->text();
	string text = qsText.toStdString();	
	leText->clear();
	
	qlItem = vLayout->itemAt(index::Table);
	QTableWidget* table = (QTableWidget*)qlItem->widget();
	int iCol = table->columnCount();
	table->setColumnCount(iCol + 1);
	QTableWidgetItem* header = nullptr;
	if (title.size() > 0) {
		header = new QTableWidgetItem(title.c_str());
		table->setHorizontalHeaderItem(iCol, header);
	}

	vector<string> vsText;
	jstr.splitByMarker(vsText, text, marker);
	int numText = (int)vsText.size();
	int numRow = table->rowCount();
	table->setRowCount(max(1 + numText, numRow));
	QTableWidgetItem* cell = new QTableWidgetItem(text.c_str());
	table->setItem(0, iCol, cell);
	for (int ii = 0; ii < numText; ii++) {
		cell = new QTableWidgetItem(vsText[ii].c_str());
		table->setItem(1 + ii, iCol, cell);
	}
	updateComparison();
}
void SCDAcompare::doRemoveCol(int iCol)
{
	QVBoxLayout* vLayout = (QVBoxLayout*)this->layout();
	QLayoutItem* qlItem = vLayout->itemAt(index::Table);
	QTableWidget* table = (QTableWidget*)qlItem->widget();
	QTableWidgetItem* cell = nullptr;
	int numRow = table->rowCount();
	int numCol = table->columnCount();
	if (iCol >= numCol) { return; }
	else if (iCol < numCol - 1) {
		for (int ii = iCol + 1; ii < numCol; ii++) {
			for (int jj = 0; jj < numRow; jj++) {
				cell = table->takeItem(jj, ii);
				if (cell == nullptr) {
					cell = table->takeItem(jj, ii - 1);
					if (cell != nullptr) { delete cell; }
				}
				else { table->setItem(jj, ii - 1, cell); }
			}
		}
	}
	numCol--;
	table->setColumnCount(numCol);
	int numCell;
	for (int ii = numRow - 1; ii >= 0; ii--) {
		numCell = 0;
		for (int jj = 0; jj < numCol; jj++) {
			cell = table->item(ii, jj);
			if (cell != nullptr) { numCell++; }
		}
		if (numCell > 0) { break; }
		else {
			numRow--;
			table->setRowCount(numRow);
		}
	}
	updateComparison();
}
void SCDAcompare::doShift(int iRow, int iCol, int direction)
{
	// Note: +direction is shift down, -direction is shift up.
	QVBoxLayout* vLayout = (QVBoxLayout*)this->layout();
	QLayoutItem* qlItem = vLayout->itemAt(index::Table);
	QTableWidget* table = (QTableWidget*)qlItem->widget();
	QTableWidgetItem* cell = nullptr;
	int numRow = table->rowCount();
	int numCol = table->columnCount();
	if (direction > 0) {
		cell = table->item(numRow - 1, iCol);
		if (cell != nullptr) {
			numRow++;
			table->setRowCount(numRow);
		}
		for (int ii = numRow - 2; ii >= iRow; ii--) {
			cell = table->takeItem(ii, iCol);
			if (cell == nullptr) { continue; }
			table->setItem(ii + 1, iCol, cell);
		}
		cell = table->item(iRow + 1, iCol);
		cell->setSelected(1);
	}
	else if (direction < 0) {
		iRow = max(iRow, 1);
		for (int ii = iRow; ii < numRow; ii++) {
			cell = table->takeItem(ii, iCol);
			if (cell == nullptr) { continue; }
			table->setItem(ii - 1, iCol, cell);
		}
		int numCell = 0;
		for (int ii = 0; ii < numCol; ii++) {
			cell = table->item(numRow - 1, ii);
			if (cell != nullptr) { numCell++; }
		}
		if (numCell == 0) { table->setRowCount(numRow - 1); }
		cell = table->item(iRow - 1, iCol);
		cell->setSelected(1);
	}
	updateComparison();
}
void SCDAcompare::downloadColPage()
{
	QVBoxLayout* vLayout = (QVBoxLayout*)this->layout();
	QLayoutItem* qlItem = vLayout->itemAt(index::Table);
	QTableWidget* table = (QTableWidget*)qlItem->widget();
	QList<QTableWidgetItem*> listCell = table->selectedItems();
	if (listCell.size() == 0) { return; }
	int iCol = listCell[0]->column();
	QTableWidgetItem* cell = table->item(0, iCol);
	QString qsURL = cell->text();
	string url = qsURL.toStdString();
	if (url.size() > 0) { emit sendDownloadMap(url); }
}
void SCDAcompare::err(string message)
{
	string errorMessage = "SCDAcompare error:\n" + message;
	JLOG::getInstance()->err(errorMessage);
}
void SCDAcompare::initColour()
{
	listColour.append(QColor("#FF8080"));
	listColour.append(QColor("#80FFFF"));
	listColour.append(QColor("#FF80FF"));
	listColour.append(QColor("#80FF80"));
	listColour.append(QColor("#8080FF"));
	listColour.append(QColor("#FFFF80"));
}
void SCDAcompare::removeCol()
{
	QVBoxLayout* vLayout = (QVBoxLayout*)this->layout();
	QLayoutItem* qlItem = vLayout->itemAt(index::Table);
	QTableWidget* table = (QTableWidget*)qlItem->widget();
	QList<QTableWidgetItem*> listCell = table->selectedItems();
	if (listCell.size() == 0) { return; }
	int iCol = listCell[0]->column();
	doRemoveCol(iCol);
}
void SCDAcompare::shiftDown()
{
	QVBoxLayout* vLayout = (QVBoxLayout*)this->layout();
	QLayoutItem* qlItem = vLayout->itemAt(index::Table);
	QTableWidget* table = (QTableWidget*)qlItem->widget();
	QList<QTableWidgetItem*> listCell = table->selectedItems();
	if (listCell.size() == 0) { return; }
	int iCol = listCell[0]->column();
	int iRow = listCell[0]->row();
	doShift(iRow, iCol, 1);  // Down is positive.
}
void SCDAcompare::shiftUp()
{
	QVBoxLayout* vLayout = (QVBoxLayout*)this->layout();
	QLayoutItem* qlItem = vLayout->itemAt(index::Table);
	QTableWidget* table = (QTableWidget*)qlItem->widget();
	QList<QTableWidgetItem*> listCell = table->selectedItems();
	if (listCell.size() == 0) { return; }
	int iCol = listCell[0]->column();
	int iRow = listCell[0]->row();
	doShift(iRow, iCol, -1);  // Up is negative.
}
void SCDAcompare::testScalePos(SWITCHBOARD& sbgui, SConline& sco)
{
	thread::id myid = this_thread::get_id();
	vector<int> mycomm;
	sbgui.answerCall(myid, mycomm);
	vector<string> vsPrompt;
	sbgui.getPrompt(vsPrompt);  // Form [url, filePath]
	size_t posExt = vsPrompt[1].rfind('.');

	if (!jfile.exist(vsPrompt[1])) {
		size_t pos1 = vsPrompt[1].find_last_of("/\\");
		string cataDir = vsPrompt[1].substr(0, pos1);
		sco.wf.makeDir(cataDir);
		sco.wf.download(vsPrompt[0], vsPrompt[1]);
	}

	vector<double> baseLTWH(4);  // Left, Top, Width, Height (unknown unit).
	size_t pos1 = vsPrompt[0].rfind("&bbox=") + 6;
	size_t pos2 = vsPrompt[0].find("%2C", pos1);
	baseLTWH[0] = stod(vsPrompt[0].substr(pos1, pos2 - pos1));
	for (int ii = 1; ii < 4; ii++) {
		pos1 = pos2 + 3;
		pos2 = vsPrompt[0].find("%2C", pos1);
		baseLTWH[ii] = stod(vsPrompt[0].substr(pos1, pos2 - pos1));
	}

	vector<unsigned char> pngData, pngNull{ 0, 0, 0, 0 };
	vector<int> pngSpec;
	pngf.load(vsPrompt[1], pngData, pngSpec);
	vector<vector<pair<int, int>>> vTLBR(2, vector<pair<int, int>>(2));
	pngf.boxFrame(vTLBR[0], pngData, pngSpec, pngNull, -1);

	// Determine a meaningful interval of change for the region's position.
	int iPower = 0;
	while (1) {
		iPower++;


	}


	sbgui.endCall(myid);
}
void SCDAcompare::updateComparison()
{
	QVBoxLayout* vLayout = (QVBoxLayout*)this->layout();
	QLayoutItem* qlItem = vLayout->itemAt(index::Table);
	QTableWidget* table = (QTableWidget*)qlItem->widget();
	QTableWidgetItem* cell = nullptr, *cellOther = nullptr;
	int numCol = table->columnCount();
	int numRow = table->rowCount();
	QString qsItem, qsItemOther;
	int indexColour = -1, numColour = listColour.size();
	unordered_map<int, int> mapColour;  // column index -> colour index
	for (int ii = 0; ii < numRow; ii++) {
		mapColour.clear();
		for (int jj = 1; jj < numCol; jj++) {
			cell = table->item(ii, jj);
			if (cell == nullptr) { continue; }
			qsItem = cell->text();
			for (int kk = 0; kk < jj; kk++) {
				cellOther = table->item(ii, kk);
				if (cellOther == nullptr) { continue; }
				qsItemOther = cellOther->text();
				if (qsItemOther == qsItem) {
					if (mapColour.count(kk)) {
						QBrush brushOther = cellOther->background();
						cell->setBackground(brushOther);
					}
					else {
						indexColour = (indexColour + 1) % numColour;
						mapColour.emplace(kk, indexColour);
						QBrush brush(listColour[indexColour]);
						cellOther->setBackground(brush);
						cell->setBackground(brush);
					}
					break;
				}
			}
		}
	}
	
	// Balance table width between columns.
	QSize tableSize = table->frameSize();
	int colWidth = tableSize.width() / numCol;
	for (int ii = 0; ii < numCol; ii++) {
		table->setColumnWidth(ii, colWidth);
	}
	this->update();
}
