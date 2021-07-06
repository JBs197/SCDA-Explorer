#include "stdafx.h"
#include "binmap.h"

void BINMAP::extrapolatePlusMinus()
{
	if (bHome.size() < 3) { jf.err("bHome no init-bm.extrapolatePlusMinus"); }
	bPlus = bHome;
	bMinus = bHome;
	for (int ii = 0; ii < 3; ii++)
	{
		bPlus[ii].y -= homeHeight;
		bMinus[ii].y += homeHeight;
	}
}
void BINMAP::findFrames(vector<unsigned char>& img, vector<int>& imgSpec, POINT bHome)
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
				p1.x -= 1;
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
POINT BINMAP::getOrigin(vector<unsigned char>& img, vector<int>& imgSpec)
{
	// Returns the coords of the origin point within the overview minimap.
	if (fOverview.size() < 3) { jf.err("No fOverview init-bm.getOrigin"); }
	if (originPatternH.size() < 1 || originPatternV.size() < 1) { jf.err("No originPattern init-bm.getOrigin"); }
	POINT origin;
	vector<int> viResult;
	if (imgSpec == pngRes)
	{
		im.scanPatternLineH(img, imgSpec, originPatternH, viResult, fOVCropped);
	}
	else if (imgSpec == fullRes)
	{
		im.scanPatternLineH(img, imgSpec, originPatternH, viResult, fOverview);
	}
	else { jf.err("Unknown resolution-bm.getOrigin"); }
	origin.y = im.getBandCenter(viResult);
	if (imgSpec == pngRes)
	{
		im.scanPatternLineV(img, imgSpec, originPatternV, viResult, fOVCropped);
	}
	else if (imgSpec == fullRes)
	{
		im.scanPatternLineV(img, imgSpec, originPatternV, viResult, fOverview);
	}
	origin.x = im.getBandCenter(viResult);
	return origin;
}
void BINMAP::init(QPlainTextEdit*& qPTE)
{
	pte = qPTE;
	string circlePath = sroot + "\\ScanCircles6.png";
	im.initScanCircles(circlePath);
	pngRes = { 1632, 817, 4 };
	fullRes = { 3840, 1080, 4 };
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
	originPatternH = {
			{ 215, 215, 215, 255 },
			{ 179, 217, 247, 255 },
			{ 215, 215, 215, 255 },
			{ 179, 217, 247, 255 },
			{ 215, 215, 215, 255 },
			{ 179, 217, 247, 255 }
	};
	originPatternV = {
			{ 179, 217, 247, 255 },
			{ 240, 240, 240, 255 },
			{ 179, 217, 247, 255 },
			{ 215, 215, 215, 255 },
			{ 179, 217, 247, 255 },
			{ 215, 215, 215, 255 }
	};
	Canada = { 240, 240, 240, 255 };
	Navy = { 51, 80, 117, 255 };
	Usa = { 215, 215, 215, 255 };
	Water = { 179, 217, 247, 255 };
}
void BINMAP::makeCanada(string& canadaPath)
{
	vector<vector<unsigned char>> rgbList;
	vector<unsigned char> img, rgb, Green = { 34, 177, 76, 255 };
	vector<int> imgSpec, viResult;
	im.pngLoadHere(canadaPath, img, imgSpec);
	if (imgSpec[0] == 3840 && imgSpec[1] == 1080)
	{
		im.crop(img, imgSpec, fMain);
		if (imgSpec[0] != pngRes[0] || imgSpec[1] != pngRes[1]) { jf.err("Bad crop resolution-bm.makeCanada"); }
		im.pngPrint(img, imgSpec, canadaPath);
	}
	POINT origin = getOrigin(img, imgSpec);

	im.pngLoad(canadaPath);
	string temp;
	unordered_map<string, int> mapBorder;  // svalue->vector index
	vector<POINT> border, circle, candidates, tracks;
	vector<double> vdResult;
	double dnum;
	int count = 0, radius, inum;
	POINT p1;
	p1.x = imgSpec[0] - 1;
	p1.y = 0;
	while (1)
	{
		rgb = im.pixelRGB(img, imgSpec, p1);
		if (rgb == Green)
		{
			count = border.size();
			border.push_back(p1);
			tracks.push_back(p1);
			temp = jf.stringifyCoord(p1);
			mapBorder.emplace(temp, count);
			break;
		}
		if (p1.x - 1 < 1 || p1.y + 1 >= imgSpec[1])
		{
			count++;
			p1.x = imgSpec[0] - 1 - count;
			p1.y = 0;
			continue;
		}
		p1.x--;
		p1.y++;
	}
	for (int ii = 2; ii < 7; ii++)
	{
		circle = im.getCircle(*border.begin(), ii);
		rgbList = im.lineRGB(img, imgSpec, circle);
		im.filterColour(circle, rgbList, Green);
		if (rgbList.size() == 0) { continue; }
		else if (rgbList.size() == 1)
		{
			count = border.size();
			border.push_back(circle[0]);
			tracks.push_back(circle[0]);
			temp = jf.stringifyCoord(circle[0]);
			mapBorder.emplace(temp, count);
			break;
		}
		else
		{
			viResult = {-1, -1};
			for (int jj = 0; jj < circle.size(); jj++)
			{
				if (circle[jj].x > viResult[0])
				{
					viResult[0] = circle[jj].x;
					viResult[1] = jj;
				}
			}
			count = border.size();
			border.push_back(circle[viResult[1]]);
			tracks.push_back(circle[viResult[1]]);
			temp = jf.stringifyCoord(circle[viResult[1]]);
			mapBorder.emplace(temp, count);
			break;
		}
	}

	while (1)
	{
		radius = 2;
		count = border.size();
		while (1)
		{
			circle = im.getCircle(*border.rbegin(), radius);
			rgbList = im.lineRGB(img, imgSpec, circle);
			im.filterColour(circle, rgbList, Green);
			for (int ii = circle.size() - 1; ii >= 0; ii--)
			{
				temp = jf.stringifyCoord(circle[ii]);
				try { inum = mapBorder.at(temp); }
				catch (out_of_range) { inum = -1; }
				if (inum >= 0) { circle.erase(circle.begin() + ii); }
			}
			if (circle.size() > 0) { break; }
			radius++;
		}
		mf.coordDistPointSumList(circle, tracks, vdResult);
		viResult = jf.minMax(vdResult);
		border.push_back(circle[viResult[1]]);
		tracks.resize(3);
		if (count == 2) { tracks[2] = circle[viResult[1]]; }
		else
		{
			tracks[count % 3] = circle[viResult[1]];
		}
		dnum = mf.coordDistPoint(*border.begin(), *border.rbegin());
		inum = border.size() % 40;
		if (border.size() > 10 && dnum < 4.0) { break; }
	}

	string binMap = "//position\n" + jf.stringifyCoord(origin);
	binMap += "\n\n//border";
	for (int ii = 0; ii < border.size(); ii++)
	{
		binMap += "\n" + jf.stringifyCoord(border[ii]);
	}
	binMap += "\n\n";
	size_t pos1 = canadaPath.rfind(".png");
	string binPath = canadaPath.substr(0, pos1) + ".bin";
	wf.printer(binPath, binMap);
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
