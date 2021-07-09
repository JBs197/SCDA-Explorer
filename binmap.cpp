#include "stdafx.h"
#include "binmap.h"

vector<POINT> MAP::getPixel(vector<unsigned char>& img, vector<int>& imgSpec, vector<unsigned char>& RGBA)
{
	// Return the list of pixels matching the given colour for the given image.
	vector<POINT> vpPixel;
	POINT p1;
	vector<unsigned char> rgba;
	for (int ii = 0; ii < imgSpec[1]; ii++)
	{
		p1.y = ii;
		for (int jj = 0; jj < imgSpec[0]; jj++)
		{
			p1.x = jj;
			rgba = im.pixelRGB(img, imgSpec, p1);
			if (rgba == RGBA) { vpPixel.push_back(p1); }
		}
	}
	return vpPixel;
}
POINT MAP::getOrigin(vector<unsigned char>& img, vector<int>& imgSpec, vector<POINT> TLBR)
{
	// Returns the coords of the origin point within the overview minimap.
	if (TLBR.size() < 2) { jf.err("TLBR missing-map.getOrigin"); }
	if (originPatternH.size() < 1 || originPatternV.size() < 1) { jf.err("No originPattern init-map.getOrigin"); }
	POINT origin;
	vector<int> viResult;
	im.scanPatternLineH(img, imgSpec, originPatternH, viResult, TLBR);
	if (viResult.size() < 1)
	{
		//im.scanPatternLineH(img, imgSpec, originPatternShadedH, viResult, TLBR);
		if (viResult.size() < 1) { jf.err("scanPatternLineH-map.getOrigin"); }
	}
	origin.y = im.getBandCenter(viResult);
	im.scanPatternLineV(img, imgSpec, originPatternV, viResult, TLBR);
	if (viResult.size() < 1)
	{
		//im.scanPatternLineV(img, imgSpec, originPatternShadedV, viResult, TLBR);
		if (viResult.size() < 1) { jf.err("scanPatternLineV-map.getOrigin"); }
	}
	origin.x = im.getBandCenter(viResult);
	return origin;
}
POINT MAP::getScaleCorner(vector<unsigned char>& img, vector<int>& imgSpec)
{
	// Input img MUST have the MSBlack/White pattern along the zeroth column.
	vector<unsigned char> rgba, rgba1, rgba2;
	POINT pAbove, pBelow, pCorner;
	pBelow.x = 0;
	pBelow.y = 0;
	int count = 0;
	while (1)
	{
		rgba = im.pixelRGB(img, imgSpec, pBelow);
		if (rgba == MSBlack) { count++; }
		else if (count > 4 && rgba == White) { break; }
		else { count = 0; }
		pBelow.y++;
	}
	pAbove = pBelow;
	pAbove.y--;
	rgba = im.pixelRGB(img, imgSpec, pAbove);
	if (rgba != MSBlack) { jf.err("lost-pngm.getScaleIndex"); }
	bool inside = 1;
	while (inside)
	{
		pAbove.x += 1;
		pBelow.x += 1;
		rgba1 = im.pixelRGB(img, imgSpec, pAbove);
		rgba2 = im.pixelRGB(img, imgSpec, pBelow);
		if (rgba1 == White && rgba2 == MSBlack) { inside = 1; }
		else if (rgba1 == MSBlack && rgba2 == White) { inside = 1; }
		else { inside = 0; }
	}
	pCorner = pBelow;
	rgba = im.pixelRGB(img, imgSpec, pCorner);
	if (rgba != MSBlack) { jf.err("Lost-bm.mapScaleCorner"); }
	while (rgba == MSBlack)
	{
		pCorner.y += 1;
		rgba = im.pixelRGB(img, imgSpec, pCorner);
	}
	pCorner.y -= 1;
	return pCorner;
}
void MAP::qshow(string sMessage)
{
	if (pte == nullptr) { jf.err("No init-bm.qshow"); }
	QString qMessage = QString::fromStdString(sMessage);
	pte->setPlainText(qMessage);
	if (isgui) { QCoreApplication::processEvents(); }
}
void MAP::recordButton(POINT& button, string buttonName)
{
	if (buttonName == "") { button = io.getCoord(VK_MBUTTON); return; }
	string sMessage = "Middle Mouse Button:  " + buttonName;
	qshow(sMessage);
	button = io.getCoord(VK_MBUTTON);
	sMessage = buttonName + ":  (" + to_string(button.x) + ",";
	sMessage += to_string(button.y) + ")";
	qshow(sMessage);
}
bool MAP::scanColourSquare(vector<POINT>& TLBR, vector<unsigned char> RGBA)
{
	// Capture the TLBR square. Return TRUE if every pixel matches the given colour. FALSE otherwise.
	vector<unsigned char> img, rgba;
	vector<int> imgSpec;
	POINT p1;
	gdi.capture(img, imgSpec, TLBR);
	for (int ii = 0; ii < imgSpec[1]; ii++)
	{
		p1.y = ii;
		for (int jj = 0; jj < imgSpec[0]; jj++)
		{
			p1.x = jj;
			rgba = im.pixelRGB(img, imgSpec, p1);
			if (rgba != RGBA) { return 0; }
		}
	}
	return 1;
}
void MAP::setPTE(QPlainTextEdit*& qPTE, bool isGUI)
{
	pte = qPTE;
	isgui = isGUI;
}
vector<double> MAP::testScale(vector<unsigned char>& img, vector<int>& imgSpec, POINT pCorner)
{
	vector<POINT> largeTLBR = scaleLargeTLBR;
	largeTLBR[0].x += pCorner.x;
	largeTLBR[1].x += pCorner.x;
	largeTLBR[0].y += pCorner.y;
	largeTLBR[1].y += pCorner.y;
	vector<POINT> mediumTLBR = largeTLBR;
	mediumTLBR[0].x -= 2;
	mediumTLBR[1].x -= 2;
	vector<POINT> smallTLBR = scaleSmallTLBR;
	smallTLBR[0].x += pCorner.x;
	smallTLBR[1].x += pCorner.x;
	smallTLBR[0].y += pCorner.y;
	smallTLBR[1].y += pCorner.y;

	vector<unsigned char> imgLarge = img;
	vector<int> imgSpecLarge = imgSpec;
	im.crop(imgLarge, imgSpecLarge, largeTLBR);
	vector<unsigned char> imgMedium = img;
	vector<int> imgSpecMedium = imgSpec;
	im.crop(imgMedium, imgSpecMedium, mediumTLBR);
	vector<unsigned char> imgSmall = img;
	vector<int> imgSpecSmall = imgSpec;
	im.crop(imgSmall, imgSpecSmall, smallTLBR);

	set<string> setLarge, setMedium, setSmall;
	string temp;
	vector<POINT> vpLarge = getPixel(imgLarge, imgSpecLarge, MSText);
	for (int ii = 0; ii < vpLarge.size(); ii++)
	{
		temp = jf.stringifyCoord(vpLarge[ii]);
		setLarge.emplace(temp);
	}
	vector<POINT> vpMedium = getPixel(imgMedium, imgSpecMedium, MSText);
	for (int ii = 0; ii < vpMedium.size(); ii++)
	{
		temp = jf.stringifyCoord(vpMedium[ii]);
		setMedium.emplace(temp);
	}
	vector<POINT> vpSmall = getPixel(imgSmall, imgSpecSmall, MSText);
	for (int ii = 0; ii < vpSmall.size(); ii++)
	{
		temp = jf.stringifyCoord(vpSmall[ii]);
		setSmall.emplace(temp);
	}

	vector<double> vdResult(viScale.size());
	int errorOver, errorUnder;
	for (int ii = 0; ii < 2; ii++)
	{
		errorOver = 0;
		errorUnder = 0;
		for (int jj = 0; jj < scaleMST[ii].size(); jj++)
		{
			temp = jf.stringifyCoord(scaleMST[ii][jj]);
			if (!setLarge.count(temp)) { errorUnder++; }
		}
		for (int jj = 0; jj < vpLarge.size(); jj++)
		{
			temp = jf.stringifyCoord(vpLarge[jj]);
			if (!setScaleMST[ii].count(temp)) { errorOver++; }
		}
		vdResult[ii] = (double)(imgLarge.size() - errorOver - errorUnder) / (double)imgLarge.size();
	}
	for (int ii = 2; ii < 4; ii++)
	{
		errorOver = 0;
		errorUnder = 0;
		for (int jj = 0; jj < scaleMST[ii].size(); jj++)
		{
			temp = jf.stringifyCoord(scaleMST[ii][jj]);
			if (!setMedium.count(temp)) { errorUnder++; }
		}
		for (int jj = 0; jj < vpMedium.size(); jj++)
		{
			temp = jf.stringifyCoord(vpMedium[jj]);
			if (!setScaleMST[ii].count(temp)) { errorOver++; }
		}
		vdResult[ii] = (double)(imgMedium.size() - errorOver - errorUnder) / (double)imgMedium.size();
	}
	for (int ii = 4; ii < 7; ii++)
	{
		errorOver = 0;
		errorUnder = 0;
		for (int jj = 0; jj < scaleMST[ii].size(); jj++)
		{
			temp = jf.stringifyCoord(scaleMST[ii][jj]);
			if (!setSmall.count(temp)) { errorUnder++; }
		}
		for (int jj = 0; jj < vpSmall.size(); jj++)
		{
			temp = jf.stringifyCoord(vpSmall[jj]);
			if (!setScaleMST[ii].count(temp)) { errorOver++; }
		}
		vdResult[ii] = (double)(imgSmall.size() - errorOver - errorUnder) / (double)imgSmall.size();
	}
	return vdResult;
}

void BINMAP::findFrames(vector<unsigned char>& img, vector<int>& imgSpec, POINT bHome) {}
void BINMAP::init(QPlainTextEdit*& qPTE, bool isGUI)
{
	pte = qPTE;
	isgui = isGUI;
	string circlePath = sroot + "\\ScanCircles6.png";
	im.initScanCircles(circlePath);
	pngRes = { 1632, 817, 4 };
	fullRes = { 3840, 1080, 4 };
	viScale = { 600, 300, 200, 100, 40, 20, 10 };

	/*
	fMain.resize(3);
	fMain[0].x = 2207;
	fMain[0].y = 156;
	fMain[1].x = 3838;
	fMain[1].y = 972;
	fMain[2].x = 3022;
	fMain[2].y = 563;
	fOverview.resize(3);
	fOverview[0].x = 3540;
	fOverview[0].y = 760;
	fOverview[1].x = 3838;
	fOverview[1].y = 972;
	fOverview[2].x = 3689;
	fOverview[2].y = 866;
	fOVCropped.resize(3);
	fOVCropped[0].x = fOverview[0].x - fMain[0].x;
	fOVCropped[0].y = fOverview[0].y - fMain[0].y;
	fOVCropped[1].x = fOverview[1].x - fMain[0].x;
	fOVCropped[1].y = fOverview[1].y - fMain[0].y;
	fOVCropped[2].x = fOverview[2].x - fMain[0].x;
	fOVCropped[2].y = fOverview[2].y - fMain[0].y;
	*/

	originPatternH.resize(2);
	originPatternH[0] = {
		{ 215, 215, 215, 255 },
		{ 179, 217, 247, 255 },
		{ 215, 215, 215, 255 },
		{ 179, 217, 247, 255 },
		{ 215, 215, 215, 255 },
		{ 179, 217, 247, 255 }
	};
	originPatternH[1] = {
		{ 181, 181, 180, 255 },
		{ 158, 182, 201, 255 },
		{ 181, 181, 180, 255 },
		{ 158, 182, 201, 255 },
		{ 181, 181, 180, 255 },
		{ 158, 182, 201, 255 }
	};
	originPatternV.resize(2);
	originPatternV[0] = {
		{ 179, 217, 247, 255 },
		{ 240, 240, 240, 255 },
		{ 179, 217, 247, 255 },
		{ 215, 215, 215, 255 },
		{ 179, 217, 247, 255 },
		{ 215, 215, 215, 255 }
	};
	originPatternV[1] = {
		{ 158, 182, 201, 255 },
		{ 198, 197, 196, 255 },
		{ 158, 182, 201, 255 },
		{ 181, 181, 180, 255 },
		{ 158, 182, 201, 255 },
		{ 181, 181, 180, 255 }
	};
	
	Black = { 0, 0, 0, 255 };
	Canada = { 240, 240, 240, 255 };
	MSBlack = { 68, 68, 68, 255 };
	Navy = { 51, 80, 117, 255 };
	Red = { 255, 0, 0, 255 };
	Usa = { 215, 215, 215, 255 };
	Water = { 179, 217, 247, 255 };
	White = { 255, 255, 255, 255 };
}
void BINMAP::recordButtonTLBR(vector<POINT>& button, string buttonName)
{
	QString qMessage = "Middle Mouse Button, TL: " + QString::fromStdString(buttonName);
	pte->setPlainText(qMessage);
	QCoreApplication::processEvents();
	button.resize(3);
	button[0] = io.getCoord(VK_MBUTTON);
	qMessage = "Middle Mouse Button, BR: " + QString::fromStdString(buttonName);
	pte->setPlainText(qMessage);
	QCoreApplication::processEvents();
	Sleep(1000);
	button[1] = io.getCoord(VK_MBUTTON);
	button[2].x = (button[1].x + button[0].x) / 2;
	button[2].y = (button[1].y + button[0].y) / 2;
	qMessage = "Recorded TLBR:\n(" + QString::number(button[0].x) + ",";
	qMessage += QString::number(button[0].y) + ")\n(" + QString::number(button[1].x);
	qMessage += "," + QString::number(button[1].y) + ")";
	pte->setPlainText(qMessage);
	QCoreApplication::processEvents();
}
void BINMAP::recordPoint(POINT& point, string pointName)
{
	QString qMessage = "Middle Mouse Button: " + QString::fromStdString(pointName);
	pte->setPlainText(qMessage);
	QCoreApplication::processEvents();
	point = io.getCoord(VK_MBUTTON);
}
void BINMAP::setPTE(QPlainTextEdit*& qPTE)
{
	pte = qPTE;
}


int PNGMAP::createAllParents()
{
	POINT pOrigin, pTL, p1;
	string sMessage, pngPath, temp;
	vector<unsigned char> img, imgMini;
	vector<int> imgSpec, imgSpecMini;

	if (vpMain.size() < 3)
	{
		pngPath = sroot + "\\Home.png";
		if (!wf.file_exist(pngPath)) { jf.err("HomePNG-pngm.createAllParents"); }
		im.pngLoadHere(pngPath, img, imgSpec);
		findFrames(img, imgSpec, bHome, vpMain, vpOverview);
	}
	if (vpOVCropped.size() < 3)
	{
		pngPath = sroot + "\\Home(cropped).png";
		if (!wf.file_exist(pngPath)) { jf.err("HomePNG(cropped)-pngm.createAllParents"); }
		im.pngLoadHere(pngPath, img, imgSpec);
		findFrames(img, imgSpec, vpOVCropped);
	}

	int count = 0, scaleHome, scaleParent;
	for (int ii = 0; ii < geoLayers.size(); ii++)
	{
		if (geoLayers[ii] == "") { sMessage = "NUM1: Configure map screen for canada"; }
		else { sMessage = "NUM1: Configure map screen for " + geoLayers[ii]; }
		qshow(sMessage);
		if (io.signal(VK_NUMPAD1)) {}
		for (int jj = 0; jj < nameParent[ii].size(); jj++)
		{
			count++;
			pngPath = folderPathParent[ii] + "\\" + nameParent[ii][jj] + ".png";
			if (wf.file_exist(pngPath)) { continue; }
			Sleep(1000);
			io.mouseClick(bHome);
			Sleep(3000);
			scaleHome = getScaleIndex();
			Sleep(500);
			io.mouseClick(searchBarClear);
			Sleep(1000);
			io.mouseClick(searchBar);
			Sleep(1000);
			io.kbInput(nameParent[ii][jj]);
			Sleep(1500);
			while (!scanColourSquare(vpQuery, White))
			{
				io.kbInput(VK_BACK);
				Sleep(1500);
			}
			io.kbInput(VK_DOWN);
			Sleep(1000);
			io.kbInput(VK_RETURN);
			Sleep(3500);
			io.mouseClick(bPanel);
			Sleep(2500);
			p1.x = 3000;
			p1.y = 10;
			io.mouseMove(p1);
			Sleep(500);
			scaleParent = getScaleIndex();
			gdi.capture(img, imgSpec, vpMain);  // Main frame only.
			Sleep(1000);
			for (int ii = scaleHome; ii < scaleParent; ii++)
			{
				io.mouseClick(bMinus);
				Sleep(2500);
			}
			gdi.capture(imgMini, imgSpecMini, vpOverview);  // Shadeless Overview.
			im.pngPaste(img, imgSpec, imgMini, imgSpecMini, vpOVCropped[0]);
			pOrigin = getOrigin(img, imgSpec, vpOVCropped);
			im.pixelPaint(img, imgSpec, Red, pOrigin);
			im.pngPrint(img, imgSpec, pngPath);
		}
	}
	return count;
}
void PNGMAP::findFrames(vector<unsigned char>& img, vector<int>& imgSpec, vector<POINT>& fOVCropped)
{
	// Made for a cropped image.
	vector<unsigned char> rgba;
	fOVCropped.resize(3);
	fOVCropped[1].x = imgSpec[0] - 1;
	fOVCropped[1].y = imgSpec[1] - 1;
	POINT p1 = fOVCropped[1];  // BR
	int stage = 0;
	while (1)
	{
		switch (stage)
		{
		case 0:
		{
			p1.y -= 1;
			rgba = im.pixelRGB(img, imgSpec, p1);
			if (rgba == Navy) { stage = 1; }
			break;
		}
		case 1:
		{
			p1.x -= 1;
			rgba = im.pixelRGB(img, imgSpec, p1);
			if (rgba != Navy)
			{
				p1.x += 1;
				p1.y += 1;
				fOVCropped[0] = p1;
				stage = 2;
			}
			break;
		}
		}
		if (stage > 1) { break; }
	}

	fOVCropped[2].x = (fOVCropped[0].x + fOVCropped[1].x) / 2;
	fOVCropped[2].y = (fOVCropped[0].y + fOVCropped[1].y) / 2;
}
void PNGMAP::findFrames(vector<unsigned char>& img, vector<int>& imgSpec, POINT bHome, vector<POINT>& fMain, vector<POINT>& fOverview)
{
	// Panel must be collapsed.
	vector<unsigned char> rgba;
	POINT p1 = bHome;
	fMain.resize(3);
	fOverview.resize(3);
	int stage = 0;
	while (1)
	{
		switch (stage)
		{
		case 0:
		{
			p1.x -= 1;
			rgba = im.pixelRGB(img, imgSpec, p1);
			if (rgba == Water) { stage = 1; }
			break;
		}
		case 1:
		{
			p1.x -= 1;
			rgba = im.pixelRGB(img, imgSpec, p1);
			if (rgba != Water)
			{
				p1.x += 1;
				fMain[0].x = p1.x;
				stage = 2;
			}
		}
		case 2:
		{
			p1.y -= 1;
			rgba = im.pixelRGB(img, imgSpec, p1);
			if (rgba != Water)
			{
				p1.y += 1;
				stage = 3;
			}
			break;
		}
		case 3:
		{
			p1.x += 1;
			p1.y -= 1;
			rgba = im.pixelRGB(img, imgSpec, p1);
			if (rgba != Water)
			{
				p1.y += 1;
				fMain[0].y = p1.y;
				p1.x = imgSpec[0] - 20;
				stage = 4;
			}
			break;
		}
		case 4:
		{
			p1.x += 1;
			rgba = im.pixelRGB(img, imgSpec, p1);
			if (rgba != Water)
			{
				p1.x -= 1;
				stage = 5;
			}
			break;
		}
		case 5:
		{
			p1.x += 1;
			p1.y += 1;
			rgba = im.pixelRGB(img, imgSpec, p1);
			if (rgba != Water)
			{
				p1.x -= 2;
				fMain[1].x = p1.x;
				p1.y = imgSpec[1] - 1;
				stage = 6;
			}
			break;
		}
		case 6:
		{
			p1.y -= 1;
			rgba = im.pixelRGB(img, imgSpec, p1);
			if (rgba == Water)
			{
				fMain[1].y = p1.y;
				fOverview[1].y = p1.y;
				fOverview[1].x = p1.x;
				stage = 7;
			}
			break;
		}
		case 7:
		{
			p1.x -= 1;
			rgba = im.pixelRGB(img, imgSpec, p1);
			if (rgba != Water)
			{
				stage = 8;
			}
			break;
		}
		case 8:
		{
			p1.x -= 1;
			rgba = im.pixelRGB(img, imgSpec, p1);
			if (rgba == Water)
			{
				stage = 9;
			}
			break;
		}
		case 9:
		{
			p1.x -= 1;
			p1.y -= 1;
			rgba = im.pixelRGB(img, imgSpec, p1);
			if (rgba != Water)
			{
				p1.x += 3;
				fOverview[0].x = p1.x;
				stage = 10;
			}
			break;
		}
		case 10:
		{
			p1.y -= 1;
			rgba = im.pixelRGB(img, imgSpec, p1);
			if (rgba == Usa)
			{
				stage = 11;
			}
			break;
		}
		case 11:
		{
			p1.y -= 1;
			rgba = im.pixelRGB(img, imgSpec, p1);
			if (rgba == Water)
			{
				stage = 12;
			}
			break;
		}
		case 12:
		{
			p1.y -= 1;
			rgba = im.pixelRGB(img, imgSpec, p1);
			if (rgba == Navy)
			{
				p1.y += 1;
				fOverview[0].y = p1.y;
				stage = 13;
			}
			break;
		}
		}

		if (stage > 12) { break; }
	}

	fMain[2].x = (fMain[0].x + fMain[1].x) / 2;
	fMain[2].y = (fMain[0].y + fMain[1].y) / 2;
	fOverview[2].x = (fOverview[0].x + fOverview[1].x) / 2;
	fOverview[2].y = (fOverview[0].y + fOverview[1].y) / 2;
}
int PNGMAP::getScaleIndex()
{
	vector<POINT> TLBR(2);
	TLBR[0].x = mapScaleStart.x;
	TLBR[0].y = mapScaleStart.y - 10;
	TLBR[1].x = mapScaleStart.x + 246;
	TLBR[1].y = mapScaleStart.y + 28;
	vector<unsigned char> img, rgba;
	vector<int> imgSpec;
	gdi.capture(img, imgSpec, TLBR);
	POINT pCorner = getScaleCorner(img, imgSpec);
	vector<double> vdResult = testScale(img, imgSpec, pCorner);
	vector<int> minMax = jf.minMax(vdResult);
	return minMax[1];
}
void PNGMAP::initialize(vector<string>& GeoLayers)
{
	if (GeoLayers.size() < 3) { jf.err("Missing GeoLayers-pngm.initialize"); }
	activeCata = GeoLayers[0];
	geoLayers.assign(GeoLayers.begin() + 1, GeoLayers.end());
	string parentPath = sroot + "\\mapParent";
	string childPath = sroot + "\\mapChild";
	for (int ii = 1; ii < geoLayers.size(); ii++)
	{
		parentPath += "\\" + geoLayers[ii];
		wf.makeDir(parentPath);
		childPath += "\\" + geoLayers[ii];
		wf.makeDir(childPath);
	}

	Black = { 0, 0, 0, 255 };
	Canada = { 240, 240, 240, 255 };
	CanadaShaded = { 198, 197, 196, 255 };
	MSBlack = { 68, 68, 68, 255 };
	MSText = { 102, 102, 102, 255 };
	Navy = { 51, 80, 117, 255 };
	Red = { 255, 0, 0, 255 };
	Usa = { 215, 215, 215, 255 };
	UsaShaded = { 181, 181, 180, 255 };
	Water = { 179, 217, 247, 255 };
	WaterShaded = { 158, 182, 201, 255 };
	White = { 255, 255, 255, 255 };

	pngRes = { 1632, 818, 4 };
	fullRes = { 3840, 1080, 4 };
	viScale = { 600, 300, 200, 100, 40, 20, 10 };

	// Index 0 = standard, 1 = standardShaded, 2 = northwest, 3 = northwestShaded.
	originPatternH.resize(2);
	originPatternH[0] = { Usa, Water, Usa, Water, Usa, Water };
	originPatternH[1] = { UsaShaded, WaterShaded, UsaShaded, WaterShaded, UsaShaded, WaterShaded};
	originPatternV.resize(2);
	originPatternV[0] = { Water, Canada, Water, Usa, Water, Usa };
	originPatternV[1] = { WaterShaded, CanadaShaded, WaterShaded, UsaShaded, WaterShaded, UsaShaded };

	bHome.x = 2260;
	bHome.y = 300;
	bPlus = bHome;
	bPlus.y -= 45;
	bMinus = bHome;
	bMinus.y += 45;
	bPanel.x = 2480;
	bPanel.y = 187;
	mapScaleStart.x = 2226;
	mapScaleStart.y = 919;
	searchBar.x = 3175;
	searchBar.y = 116;
	searchBarClear.x = 3356;
	searchBarClear.y = 116;
	vpQuery.resize(2);
	vpQuery[0].x = 3379;
	vpQuery[0].y = 156;
	vpQuery[1].x = 3388;
	vpQuery[1].y = 165;

	scaleLargeTLBR.resize(2);
	scaleLargeTLBR[0].x = -8;
	scaleLargeTLBR[0].y = 2;
	scaleLargeTLBR[1].x = 53;
	scaleLargeTLBR[1].y = 15;
	scaleSmallTLBR.resize(2);
	scaleSmallTLBR[0].x = -9;
	scaleSmallTLBR[0].y = 2;
	scaleSmallTLBR[1].x = 42;
	scaleSmallTLBR[1].y = 15;

	string scalePath, temp;
	vector<int> imgSpec;
	vector<unsigned char> img;
	scaleMST.resize(viScale.size());
	setScaleMST.resize(viScale.size());
	for (int ii = 0; ii < scaleMST.size(); ii++)
	{
		scalePath = sroot + "\\font\\" + to_string(viScale[ii]) + "km.png";
		im.pngLoadHere(scalePath, img, imgSpec);
		scaleMST[ii] = getPixel(img, imgSpec, MSText);
		for (int jj = 0; jj < scaleMST[ii].size(); jj++)
		{
			temp = jf.stringifyCoord(scaleMST[ii][jj]);
			setScaleMST[ii].emplace(temp);
		}
	}

}
int PNGMAP::initParentChild(vector<vector<string>>& geo)
{
	if (geoLayers.size() < 1) { jf.err("No init-pngm.initParentChild"); }
	string temp, parentGC;
	int index, parentRow, numPNG = 0;
	folderPathParent.clear();
	folderPathChild.clear();
	mapGCRow.clear();
	setGCParent.clear();
	string folderParent = sroot + "\\mapParent";
	string folderChild = sroot + "\\mapChild";
	for (int ii = 0; ii < geoLayers.size(); ii++)
	{
		nameParent.push_back(vector<string>());
		nameChild.push_back(vector<string>());
		if (geoLayers[ii].size() > 0)
		{
			folderParent += "\\" + geoLayers[ii];
			folderChild += "\\" + geoLayers[ii];
		}
		folderPathParent.push_back(folderParent);
		folderPathChild.push_back(folderChild);
	}
	for (int ii = 0; ii < geo.size(); ii++)
	{
		mapGCRow.emplace(geo[ii][0], ii);
		if (geo[ii].size() < 4) { continue; }
		try { index = stoi(geo[ii][2]); }
		catch (invalid_argument) { jf.err("stoi-pngm.initParentChild"); }
		nameChild[index].push_back(geo[ii][1]);
		numPNG++;
		parentGC = geo[ii].back();
		if (!setGCParent.count(parentGC))
		{
			try { parentRow = mapGCRow.at(parentGC); }
			catch (out_of_range) { jf.err("mapGCRow-pngm.initParentChild"); }
			nameParent[index - 1].push_back(geo[parentRow][1]);
			numPNG++;
			setGCParent.emplace(parentGC);
		}
	}
	return numPNG;
}

