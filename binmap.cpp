#include "stdafx.h"
#include "binmap.h"

void BINMAP::findFrames(POINT bHome)
{
	// Panel must be collapsed.
	bool water = 0;
	vector<int> imgSpec;
	vector<unsigned char> img, rgba;
	io.mouseClick(bHome);
	Sleep(1000);
	string screenshotPath = sroot + "\\desktop.png";
	gdi.screenshot(screenshotPath);
	im.pngLoadHere(screenshotPath, img, imgSpec);
	while (1)
	{
		bHome.x -= 1;
		rgba = im.pixelRGB(img, imgSpec, bHome);
		if (!water && rgba == Water) { water = 1; }
		else if (rgba != Water)
		{
			water = 0;
			//RESUME HERE.
		}

	}
}
void BINMAP::recordButton(vector<POINT>& button, string buttonName)
{
	QString qMessage = "Middle Mouse Button, TL: " + QString::fromStdString(buttonName);
	pte->setPlainText(qMessage);
	QCoreApplication::processEvents();
	button.resize(3);
	button[0] = io.getCoord(VK_MBUTTON);
	qMessage = "Middle Mouse Button, BR: " + QString::fromStdString(buttonName);
	pte->setPlainText(qMessage);
	QCoreApplication::processEvents();
	button[1] = io.getCoord(VK_MBUTTON);
	button[2].x = (button[1].x + button[0].x) / 2;
	button[2].y = (button[1].y + button[0].y) / 2;
}
void BINMAP::setPTE(QPlainTextEdit*& qPTE)
{
	pte = qPTE;
}
