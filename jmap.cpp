#include "stdafx.h"
#include "jmap.h"

void MAP::cropToOverview(vector<unsigned char>& img, vector<int>& imgSpec)
{
	if (imgSpec != pngRes) { jf.err("No init or bad parameter-map.cropToOverview"); }
	im.crop(img, imgSpec, vpOVCropped);
}
double MAP::extractScale(vector<unsigned char> img, vector<int> imgSpec)
{
	// Returns this map's "pixels per kilometer" scale. 
	vector<POINT> TLBR(2);
	TLBR[0].x = 14;
	TLBR[0].y = 758;
	TLBR[1].x = 300;
	TLBR[1].y = 786;
	im.crop(img, imgSpec, TLBR);
	int leftCol = 4;
	POINT scaleBR = getScaleCorner(img, imgSpec, leftCol);
	int barLength = scaleBR.x - leftCol + 1;
	vector<double> matchScore = testScale(img, imgSpec, scaleBR);
	vector<int> minMax = jf.minMax(matchScore);
	int iKM = viScale[minMax[1]];
	double pixelPerKM = (double)barLength / (double)iKM;
	return pixelPerKM;
}
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
	int startCol = 0;
	POINT pCorner = getScaleCorner(img, imgSpec, startCol);
	return pCorner;
}
POINT MAP::getScaleCorner(vector<unsigned char>& img, vector<int>& imgSpec, int& startCol)
{
	// Return POINT is the BR corner, and startCol is changed into the scale's left-most column.
	vector<unsigned char> rgba, rgba1, rgba2;
	POINT pAbove, pBelow, pCorner;
	pBelow.x = startCol;
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
	if (startCol > 0)
	{
		pCorner = pBelow;
		rgba = im.pixelRGB(img, imgSpec, pCorner);
		while (rgba != MSBlack)
		{
			pCorner.x -= 1;
			if (pCorner.x < 0) { jf.err("lost-map.getScaleIndex"); }
			rgba = im.pixelRGB(img, imgSpec, pCorner);
		}
		startCol = pCorner.x;
	}
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
void MAP::initScanCircles(string& pngPath)
{
	im.initScanCircles(pngPath);
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
vector<double> MAP::testScale(vector<unsigned char>& img, vector<int>& imgSpec, POINT pCorner)
{
	// For a given image and scaleBR (pCorner), return a list of match scores for each scale index. 
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


void BINMAP::borderCheck(vector<POINT>& vpBorder, vector<POINT>& vpCandidate, vector<vector<unsigned char>>& rgbaList)
{
	// Look back into the entire border list, and eliminate candidates that already exist there. 
	int borderSize = vpBorder.size();
	borderCheck(vpBorder, vpCandidate, rgbaList, borderSize);
}
void BINMAP::borderCheck(vector<POINT>& vpBorder, vector<POINT>& vpCandidate, vector<vector<unsigned char>>& rgbaList, int depth)
{
	// Look back into the border list a certain depth, and eliminate candidates that already exist there. 
	if (depth < 1) { jf.err("Invalid depth-bm.borderCheck"); }
	if (vpCandidate.size() != rgbaList.size()) { jf.err("Parameter size mismatch-bm.borderCheck"); }
	for (int ii = vpCandidate.size() - 1; ii >= 0; ii--)
	{
		for (int jj = 1; jj <= depth; jj++)
		{
			if (vpCandidate[ii].x == vpBorder[vpBorder.size() - jj].x && vpCandidate[ii].y == vpBorder[vpBorder.size() - jj].y)
			{
				vpCandidate.erase(vpCandidate.begin() + ii);
				rgbaList.erase(rgbaList.begin() + ii);
				break;
			}
		}
	}
}
void BINMAP::borderComplete(vector<unsigned char>& img, vector<int>& imgSpec, vector<POINT>& vpBorder)
{
	// Jump along the painted border, adding to the border list, until the region's border list is complete.
	if (vpBorder.size() < 3) { jf.err("vpBorder missing starters-bm.borderComplete"); }
	vector<vector<unsigned char>> rgbaList;
	vector<double> vdCandidateDistance;
	vector<int> minMax;
	vector<POINT> vpCandidate;
	int borderIndex = vpBorder.size() - 1, radius, candidateSize;
	double distHome;
	bool noplacelikehome = 0;
	while (!noplacelikehome)  // For every point along the border...
	{
		radius = 2;
		while (1)  // For every list of candidates obtained with this radius...
		{
			vpCandidate = im.getCircle(vpBorder[borderIndex], radius);
			rgbaList = im.lineRGB(img, imgSpec, vpCandidate);
			im.filterColour(vpCandidate, rgbaList, Green);
			borderCheck(vpBorder, vpCandidate, rgbaList, 3);
			im.filterDeadCone(vpBorder, vpCandidate, rgbaList);
			candidateSize = vpCandidate.size();
			if (candidateSize == 0) 
			{ 
				radius++;
				if (radius > 13) { jf.err("Failed to locate border candidates-bm.borderComplete"); }
				continue;
			}
			vdCandidateDistance = mf.coordDistPointSumList(vpCandidate, vpBorder, 3);
			if (candidateSize == 1)  // Is this candidate worthy? 
			{
				if (vdCandidateDistance[0] < 4.0 * (double)radius)
				{
					radius++;
					if (radius > 13) { jf.err("Failed to locate border candidates-bm.borderComplete"); }
					continue;
				}
				else
				{
					vpBorder.push_back(vpCandidate[0]);
					break;
				}
			}
			else
			{
				minMax = jf.minMax(vdCandidateDistance);
				vpBorder.push_back(vpCandidate[minMax[1]]);
				break;
			}
		}
		borderIndex = vpBorder.size() - 1;
		if (borderIndex >= 20)
		{
			distHome = mf.coordDistPoint(vpBorder[0], vpBorder[borderIndex]);
			if (distHome < 6.0) { noplacelikehome = 1; }
		}
	}
}
void BINMAP::borderPlus(vector<unsigned char>& img, vector<int>& imgSpec, vector<POINT>& vpBorder)
{
	// Given a starter border point, add another along a clockwise direction.
	int radius = 2, candidateSize, borderSize = vpBorder.size();
	if (borderSize < 1) { jf.err("pBorder invalid-bm.borderInit"); }
	vector<POINT> vpCandidate; 
	vector<vector<unsigned char>> rgbaList; 
	vector<int> xMax, yMax;
	while (1)
	{
		vpCandidate = im.getCircle(vpBorder[borderSize - 1], radius, imgSpec);
		rgbaList = im.lineRGB(img, imgSpec, vpCandidate);
		im.filterColour(vpCandidate, rgbaList, Green);
		borderCheck(vpBorder, vpCandidate, rgbaList);
		candidateSize = vpCandidate.size();
		if (candidateSize == 0) { radius++; }
		else if (candidateSize == 1)
		{
			vpBorder.push_back(vpCandidate[0]);
			return;
		}
		else
		{
			xMax = { -1, -1 };
			for (int ii = 0; ii < candidateSize; ii++)
			{
				if (vpCandidate[ii].x > xMax[0])
				{
					xMax.resize(2);
					xMax[0] = vpCandidate[ii].x;
					xMax[1] = ii;
				}
				else if (vpCandidate[ii].x == xMax[0])
				{
					xMax.push_back(ii);
				}
			}
			if (xMax.size() == 2)
			{
				vpBorder.push_back(vpCandidate[xMax[1]]);
				return;
			}
			else
			{
				yMax = { -1, -1 };
				for (int ii = 1; ii < xMax.size(); ii++)
				{
					if (vpCandidate[xMax[ii]].y > yMax[0])
					{
						yMax[0] = vpCandidate[xMax[ii]].y;
						yMax[1] = xMax[ii];
					}
				}
				vpBorder.push_back(vpCandidate[yMax[1]]);
				return;
			}
		}
	}


}
bool BINMAP::checkSpec(vector<int>& imgSpec)
{
	// Returns TRUE if the given specifications match a main frame PNG, or an overview PNG. Else FALSE.
	if (pngRes.size() != 3 || ovRes.size() != 3) { jf.err("No init-bm.checkSpec"); }
	if (imgSpec == pngRes) { return 1; }
	if (imgSpec == ovRes) { return 1; }
	return 0;
}
vector<POINT> BINMAP::drawRect(vector<unsigned char>& img, vector<int>& imgSpec, int iMargin)
{
	vector<POINT> TLBR = drawRect(img, imgSpec, iMargin, { CanadaSel, Red });
	return TLBR;
}
vector<POINT> BINMAP::drawRect(vector<unsigned char>& img, vector<int>& imgSpec, int iMargin, vector<vector<unsigned char>> rgba)
{
	// Upon a given image, draw a rectangle encompassing the given colour, with a margin on all sides.
	// rgba has form [target colour, rectangle colour]. Image will be resized if necessary. 
	vector<vector<int>> selBox = im.makeBox(img, imgSpec, rgba[0], iMargin);
	int rightMargin = imgSpec[0] - selBox[1][0] - 1;
	int botMargin = imgSpec[1] - selBox[1][1] - 1;
	if (selBox[0][0] < 0 || selBox[0][1] < 0 || rightMargin < 0 || botMargin < 0)
	{
		vector<unsigned char> imgOld = img;
		vector<int> imgSpecOld = imgSpec;
		img.clear();
		POINT TL;
		TL.x = 0;
		TL.y = 0;
		int space;
		if (selBox[0][0] < 0) 
		{ 
			space = abs(selBox[0][0]);
			TL.x += space;
			imgSpec[0] += space; 
			selBox[0][0] = 0;
			selBox[1][0] += space;
		}
		if (selBox[0][1] < 0) 
		{ 
			space = abs(selBox[0][1]);
			TL.y += space;
			imgSpec[1] += space; 
			selBox[0][1] = 0;
			selBox[1][1] += space;
		}
		if (rightMargin < 0) { imgSpec[0] += abs(rightMargin); }
		if (botMargin < 0) { imgSpec[1] += abs(botMargin); }
		im.pngCanvas(imgSpec, img, Black);
		im.pngPaste(img, imgSpec, imgOld, imgSpecOld, TL);
	}
	vector<POINT> TLBR(2);
	TLBR[0].x = selBox[0][0];
	TLBR[0].y = selBox[0][1];
	TLBR[1].x = selBox[1][0];
	TLBR[1].y = selBox[1][1];
	im.rectPaint(img, imgSpec, TLBR, rgba[1]);
	return TLBR;
}
vector<int> BINMAP::extractPosition(vector<unsigned char>& imgSuper, vector<int>& imgSpecSuper, vector<unsigned char>& imgHome, vector<int>& imgSpecHome)
{
	// Returns the region's displacement relative to the HomeOV reference image. 
	vector<int> DxDy(2);
	POINT TL;
	TL.x = 0;
	TL.y = imgSpecHome[1] - 1;
	vector<unsigned char> botRowHome = im.pngExtractRow(imgHome, imgSpecHome, TL);
	TL.y = 0;
	vector<unsigned char> topRowHome = im.pngExtractRow(imgHome, imgSpecHome, TL);
	vector<unsigned char> topRowSuper, botRowSuper, tempRow;
	vector<unsigned char> rgba = im.pixelRGB(imgSuper, imgSpecSuper, TL);
	if (rgba == Black)
	{
		while (rgba == Black)
		{
			TL.x++;
			rgba = im.pixelRGB(imgSuper, imgSpecSuper, TL);
		}
		DxDy[0] = TL.x;
		TL.x--;
		rgba = im.pixelRGB(imgSuper, imgSpecSuper, TL);
		while (rgba == Black)
		{
			TL.y++;
			rgba = im.pixelRGB(imgSuper, imgSpecSuper, TL);
		}
		DxDy[1] = TL.y;
		TL.x = 0;
		TL.y = 0;
		tempRow = im.pngExtractRow(imgSuper, imgSpecSuper, TL);
		topRowSuper.assign(tempRow.begin() + (DxDy[0] * imgSpecSuper[2]), tempRow.end());
		if (topRowSuper == topRowHome) 
		{ 
			DxDy[0] *= -1;
			return DxDy; 
		}  
		TL.y = imgSpecSuper[1] - 1;
		botRowSuper = im.pngExtractRow(imgSuper, imgSpecSuper, TL);
		botRowSuper.resize(botRowSuper.size() - (DxDy[0] * imgSpecSuper[2]));
		if (botRowSuper == botRowHome) { DxDy[1] *= -1; }
		else { jf.err("Failure-bm.extractPosition"); }
	}
	else
	{
		TL.y = imgSpecSuper[1] - 1;
		rgba = im.pixelRGB(imgSuper, imgSpecSuper, TL);
		while (rgba == Black)
		{
			TL.x++;
			rgba = im.pixelRGB(imgSuper, imgSpecSuper, TL);
		}
		DxDy[0] = TL.x;
		TL.x--;
		rgba = im.pixelRGB(imgSuper, imgSpecSuper, TL);
		while (rgba == Black)
		{
			TL.y--;
			rgba = im.pixelRGB(imgSuper, imgSpecSuper, TL);
		}
		TL.y++;
		DxDy[1] = imgSpecSuper[1] - TL.y;
		TL.x = 0;
		TL.y = 0;
		topRowSuper = im.pngExtractRow(imgSuper, imgSpecSuper, TL);
		topRowSuper.resize(topRowSuper.size() - (DxDy[0] * imgSpecSuper[2]));
		if (topRowSuper == topRowHome) { return DxDy; }  // Both displacements were positive.
		TL.y = imgSpecSuper[1] - 1;
		tempRow = im.pngExtractRow(imgSuper, imgSpecSuper, TL);
		botRowSuper.assign(tempRow.begin() + (DxDy[0] * imgSpecSuper[2]), tempRow.end());
		if (botRowSuper == botRowHome)
		{
			DxDy[0] *= -1;
			DxDy[1] *= -1;
		}
		else { jf.err("Failure-bm.extractPosition"); }
	}
	return DxDy;
}
void BINMAP::findFrames(vector<unsigned char>& img, vector<int>& imgSpec, POINT bHome) {}
void BINMAP::initialize()
{
	string circlePath = sroot + "\\ScanCircles13.png";
	im.initScanCircles(circlePath);
	pngRes = { 1632, 818, 4 };
	fullRes = { 3840, 1080, 4 };
	ovRes = { 300, 213, 4 };
	viScale = { 600, 300, 200, 100, 40, 20, 10 };
	vpOVCropped.resize(2);
	vpOVCropped[0].x = 1332;
	vpOVCropped[0].y = 605;
	vpOVCropped[1].x = 1631;
	vpOVCropped[1].y = 817;

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

	Black = { 0, 0, 0, 255 };
	Canada = { 240, 240, 240, 255 };
	CanadaSel = { 192, 192, 243, 255 };
	Green = { 34, 177, 76, 255 };
	MSBlack = { 68, 68, 68, 255 };
	MSText = { 102, 102, 102, 255 };
	Navy = { 51, 80, 117, 255 };
	Red = { 255, 0, 0, 255 };
	Usa = { 215, 215, 215, 255 };
	Water = { 179, 217, 247, 255 };
	WaterSel = { 143, 174, 249, 255 };
	White = { 255, 255, 255, 255 };

	greenBlueOutside = { 0.8, 1.01 };
	greenBlueInside = { 0.69, 0.8 };
	redBlueInside = { 0.72, 1.01 };
	redGreenInside = { 0.92, 1.01 };

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
void BINMAP::qshow(string sMessage)
{
	if (pte == nullptr) { jf.err("No init-bm.qshow"); }
	QString qMessage = QString::fromStdString(sMessage);
	pte->setPlainText(qMessage);
	if (isgui)
	{
		QCoreApplication::processEvents();
	}
}
void BINMAP::recordPoint(POINT& point, string pointName)
{
	QString qMessage = "Middle Mouse Button: " + QString::fromStdString(pointName);
	pte->setPlainText(qMessage);
	QCoreApplication::processEvents();
	point = io.getCoord(VK_MBUTTON);
}
void BINMAP::setPTE(QPlainTextEdit*& qPTE, bool isGUI)
{
	pte = qPTE;
	isgui = isGUI;
}
void BINMAP::sprayRegion(vector<unsigned char>& img, vector<int>& imgSpec, vector<POINT> TLBR)
{
	// Travel along TLBR and paint the midpoint pixel between a region's "out" and "in" colour ratios.
	vector<vector<double>> vvdInside = { greenBlueInside, redBlueInside, redGreenInside };
	POINT pBorder;
	vector<POINT> startStop(2);
	startStop[0].x = TLBR[0].x + 1;
	startStop[0].y = TLBR[0].y + 1;
	startStop[1].x = startStop[0].x;
	startStop[1].y = TLBR[1].y - 1;
	while (startStop[0].x < TLBR[1].x)
	{
		pBorder = im.getBorderPoint(img, imgSpec, startStop, greenBlueOutside, vvdInside);
		if (pBorder.x >= 0)
		{
			im.pixelPaint(img, imgSpec, Green, pBorder);
		}
		startStop[0].x++;
		startStop[1].x++;
	}
	startStop[0].x = TLBR[1].x - 1;
	startStop[0].y = TLBR[0].y + 1;
	startStop[1].x = TLBR[0].x + 1;
	startStop[1].y = startStop[0].y;
	while (startStop[0].y < TLBR[1].y)
	{
		pBorder = im.getBorderPoint(img, imgSpec, startStop, greenBlueOutside, vvdInside);
		if (pBorder.x >= 0)
		{
			im.pixelPaint(img, imgSpec, Green, pBorder);
		}
		startStop[0].y++;
		startStop[1].y++;
	}
	startStop[0].x = TLBR[1].x - 1;
	startStop[0].y = TLBR[1].y - 1;
	startStop[1].x = startStop[0].x;
	startStop[1].y = TLBR[0].y + 1;
	while (startStop[0].x > TLBR[0].x)
	{
		pBorder = im.getBorderPoint(img, imgSpec, startStop, greenBlueOutside, vvdInside);
		if (pBorder.x >= 0)
		{
			im.pixelPaint(img, imgSpec, Green, pBorder);
		}
		startStop[0].x--;
		startStop[1].x--;
	}
	startStop[0].x = TLBR[0].x + 1;
	startStop[0].y = TLBR[1].y - 1;
	startStop[1].x = TLBR[1].x - 1;
	startStop[1].y = startStop[0].y;
	while (startStop[0].y > TLBR[0].y)
	{
		pBorder = im.getBorderPoint(img, imgSpec, startStop, greenBlueOutside, vvdInside);
		if (pBorder.x >= 0)
		{
			im.pixelPaint(img, imgSpec, Green, pBorder);
		}
		startStop[0].y--;
		startStop[1].y--;
	}
}


int PNGMAP::createAllParents()
{
	POINT pOrigin, pTL, p1;
	string sMessage, pngPath, pngPathManual, temp;
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

	bool manual = 0;
	int count = 0, scaleHome, scaleParent;
	for (int ii = 0; ii < geoLayers.size(); ii++)
	{
		if (geoLayers[ii] == "") { sMessage = "NUM1: Configure map screen for canada"; }
		else if (nameParent[ii].size() < 1) { continue; }
		else { sMessage = "NUM1: Configure map screen for " + geoLayers[ii]; }
		qshow(sMessage);
		if (io.signal(VK_NUMPAD1)) {}
		for (int jj = 0; jj < nameParent[ii].size(); jj++)
		{
			count++;
			pngPath = folderPathParent[ii] + "\\" + nameParent[ii][jj] + ".png";
			pngPathManual = folderPathParent[ii] + "\\" + nameParent[ii][jj] + "(manual).png";
			if (wf.file_exist(pngPath)) { continue; }
			if (wf.file_exist(pngPathManual)) { manual = 1; }
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
			if (!manual)
			{
				scaleParent = getScaleIndex();
				gdi.capture(img, imgSpec, vpMain);  // Main frame only.
				Sleep(1000);
				for (int ii = scaleHome; ii < scaleParent; ii++)
				{
					io.mouseClick(bMinus);
					Sleep(2500);
				}
				gdi.capture(imgMini, imgSpecMini, vpOverview);  // Shadeless Overview.
			}
			else
			{
				sMessage = "NUM1: Position for " + nameParent[ii][jj];
				qshow(sMessage);
				if (io.signal(VK_NUMPAD1)) { gdi.capture(img, imgSpec, vpMain); }
				Sleep(1000);
				sMessage = "NUM1: Zoom out to parent scale";
				qshow(sMessage);
				if (io.signal(VK_NUMPAD1)) { gdi.capture(imgMini, imgSpecMini, vpOverview); }
				manual = 0;
			}
			im.pngPaste(img, imgSpec, imgMini, imgSpecMini, vpOVCropped[0]);
			im.pngPrint(img, imgSpec, pngPath);
		}
	}
	return count;
}
void PNGMAP::createAllChildren(SWITCHBOARD& sbgui, vector<int> mycomm)
{
	string pngPath, sMessage, childPath, childPathManual, parentPath, temp, childPathANSI;
	vector<unsigned char> img, imgMini;
	vector<int> imgSpec, imgSpecMini;
	thread::id myid = this_thread::get_id();
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

	POINT p1;
	bool manual = 0;
	int scaleHome = -1, scaleChild;
	for (int ii = 0; ii < geoLayers.size(); ii++)
	{
		for (int jj = 0; jj < nameChild[ii].size(); jj++)
		{
			temp = nameChild[ii][jj];
			nameChild[ii][jj] = jf.utf8ToAscii(temp);
			childPath = folderPathChild[ii] + "\\" + nameChild[ii][jj] + ".png";
			parentPath = folderPathParent[ii] + "\\" + nameChild[ii][jj] + ".png";
			if (wf.file_exist(childPath))
			{
				nameChild[ii].erase(nameChild[ii].begin() + jj);
				jj--;
				mycomm[1]++;
				sbgui.update(myid, mycomm);
			}
			else if (wf.file_exist(parentPath))
			{
				wf.copyFile(parentPath, childPath);
				nameChild[ii].erase(nameChild[ii].begin() + jj);
				jj--;
				mycomm[1]++;
				sbgui.update(myid, mycomm);
			}
		}
		if (nameChild[ii].size() < 1) { continue; }

		sMessage = "NUM1: Configure map screen for " + geoLayers[ii];
		qshow(sMessage);
		if (io.signal(VK_NUMPAD1)) {}
		for (int jj = 0; jj < nameChild[ii].size(); jj++)
		{
			childPath = folderPathChild[ii] + "\\" + nameChild[ii][jj] + ".png";
			childPathManual = folderPathChild[ii] + "\\" + nameChild[ii][jj] + "(manual).png";
			if (wf.file_exist(childPathManual)) { manual = 1; }
			if (scaleHome < 0)
			{
				Sleep(1000);
				io.mouseClick(bHome);
				Sleep(3000);
				scaleHome = getScaleIndex();
			}
			Sleep(1000);
			io.mouseClick(searchBarClear);
			Sleep(1000);
			io.mouseClick(searchBar);
			Sleep(1000);
			temp = nameChild[ii][jj];
			jf.asciiNearestFit(temp);
			io.kbInput(temp);
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
			Sleep(1000);
			if (!manual)
			{
				scaleChild = getScaleIndex();
				gdi.capture(img, imgSpec, vpMain);  // Main frame only.
				Sleep(1000);
				for (int ii = scaleHome; ii < scaleChild; ii++)
				{
					io.mouseClick(bMinus);
					Sleep(2500);
				}
				gdi.capture(imgMini, imgSpecMini, vpOverview);  // Shadeless Overview.
			}
			else
			{
				sMessage = "NUM1: Position for " + nameChild[ii][jj];
				qshow(sMessage);
				if (io.signal(VK_NUMPAD1)) { gdi.capture(img, imgSpec, vpMain); }
				Sleep(1000);
				sMessage = "NUM1: Zoom out to home scale";
				qshow(sMessage);
				if (io.signal(VK_NUMPAD1)) { gdi.capture(imgMini, imgSpecMini, vpOverview); }
				manual = 0;
			}
			im.pngPaste(img, imgSpec, imgMini, imgSpecMini, vpOVCropped[0]);
			childPathANSI = jf.asciiOnly(childPath);
			im.pngPrint(img, imgSpec, childPathANSI);
			if (childPathANSI != childPath) { wf.renameFile(childPathANSI, childPath); }
		}
	}

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
int PNGMAP::getNumChildren()
{
	int count = 0;
	for (int ii = 0; ii < nameChild.size(); ii++)
	{
		count += nameChild[ii].size();
	}
	return count;
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
void PNGMAP::recordButton(POINT& button, string buttonName)
{
	if (buttonName == "") { button = io.getCoord(VK_MBUTTON); return; }
	string sMessage = "Middle Mouse Button:  " + buttonName;
	qshow(sMessage);
	button = io.getCoord(VK_MBUTTON);
	sMessage = buttonName + ":  (" + to_string(button.x) + ",";
	sMessage += to_string(button.y) + ")";
	qshow(sMessage);
}
void PNGMAP::qshow(string sMessage)
{
	if (pte == nullptr) { jf.err("No init-bm.qshow"); }
	QString qMessage = QString::fromStdString(sMessage);
	pte->setPlainText(qMessage);
	if (isgui)
	{
		QCoreApplication::processEvents();
	}
}
void PNGMAP::setPTE(QPlainTextEdit*& qPTE, bool isGUI)
{
	pte = qPTE;
	isgui = isGUI;
}


int OVERLAY::checkColour(vector<unsigned char>& rgb)
{
	// Return the colourIndex if the given colour matches one of the rows in 'colours'. Else -1.
	for (int ii = 0; ii < depth; ii++)
	{
		for (int jj = 0; jj < length; jj++)
		{
			if (rgb[jj] != colours[ii][jj]) { break; }
			else if (jj == length - 1) { return ii; }
		}
	}
	return -1;
}
void OVERLAY::createSuperposition(vector<unsigned char>& imgTop, vector<int>& imgSpecTop, vector<unsigned char>& imgSuper, vector<int>& imgSpecSuper)
{
	if (substrate.size() < 1) { jf.err("No init-ov.createSuperposition"); }
	if (imgSpecTop[2] != length) { jf.err("Pixel size mismatch-ov.createSuperposition"); }
	vector<vector<int>> matchTest = matchAll(imgTop, imgSpecTop);
	int maxMatch = 0, maxIndex = -1;
	for (int ii = 0; ii < matchTest.size(); ii++)
	{
		if (matchTest[ii][0] > maxMatch)
		{
			maxMatch = matchTest[ii][0];
			maxIndex = ii;
		}
	}
	if (maxIndex < 0) { jf.err("No matches found-ov.createSuperposition"); }

	imgSuper.clear();
	imgSpecSuper.resize(3);
	if (matchTest[maxIndex][1] < 0) { imgSpecSuper[0] = BR.x - matchTest[maxIndex][1] + 1; }
	else { imgSpecSuper[0] = matchTest[maxIndex][1] + imgSpecTop[0]; }
	if (matchTest[maxIndex][2] < 0) { imgSpecSuper[1] = BR.y - matchTest[maxIndex][2] + 1; }
	else { imgSpecSuper[1] = matchTest[maxIndex][2] + imgSpecTop[1]; }
	imgSpecSuper[2] = length;
	vector<unsigned char> rgbx;
	if (length == 3) { rgbx = { 0, 0, 0 }; }
	else if (length == 4) { rgbx = { 0, 0, 0, 255 }; }
	im.pngCanvas(imgSpecSuper, imgSuper, rgbx);
	POINT topTL, botTL;
	if (matchTest[maxIndex][1] < 0)
	{
		topTL.x = 0;
		botTL.x = abs(matchTest[maxIndex][1]);
		if (matchTest[maxIndex][2] < 0)
		{
			topTL.y = 0;
			botTL.y = abs(matchTest[maxIndex][2]);
			im.pngPaste(imgSuper, imgSpecSuper, pngBot, pngSpecBot, botTL);
			im.pngPaste(imgSuper, imgSpecSuper, imgTop, imgSpecTop, topTL);
		}
		else
		{
			topTL.y = matchTest[maxIndex][2];
			botTL.y = 0;
			im.pngPaste(imgSuper, imgSpecSuper, pngBot, pngSpecBot, botTL);
			im.pngPaste(imgSuper, imgSpecSuper, imgTop, imgSpecTop, topTL);
		}
	}
	else
	{
		topTL.x = matchTest[maxIndex][1];
		botTL.x = 0;
		if (matchTest[maxIndex][2] < 0)
		{
			topTL.y = 0;
			botTL.y = abs(matchTest[maxIndex][2]);
			im.pngPaste(imgSuper, imgSpecSuper, pngBot, pngSpecBot, botTL);
			im.pngPaste(imgSuper, imgSpecSuper, imgTop, imgSpecTop, topTL);
		}
		else
		{
			topTL.y = matchTest[maxIndex][2];
			botTL.y = 0;
			im.pngPaste(imgSuper, imgSpecSuper, pngBot, pngSpecBot, botTL);
			im.pngPaste(imgSuper, imgSpecSuper, imgTop, imgSpecTop, topTL);
		}
	}

}
void OVERLAY::initPNG(vector<unsigned char>& imgBot, vector<int>& imgSpecBot)
{
	pngBot = imgBot;
	pngSpecBot = imgSpecBot;
	BR.x = imgSpecBot[0] - 1;
	BR.y = imgSpecBot[1] - 1;
	length = pngSpecBot[2];

	Canada = { 240, 240, 240, 255 };  // colourIndex = 0
	Usa = { 215, 215, 215, 255 };     // colourIndex = 1
	Water = { 179, 217, 247, 255 };   // colourIndex = 2

	colours.resize(3);
	colours[0] = Canada;
	colours[1] = Usa;
	colours[2] = Water;
	depth = 3;

	substrate.clear();
	substrate.resize(imgSpecBot[1], vector<int>(imgSpecBot[0], -1));
	int index = 0, colourIndex;
	vector<unsigned char> rgbx(length);
	for (int ii = 0; ii < imgSpecBot[1]; ii++)
	{
		for (int jj = 0; jj < imgSpecBot[0]; jj++)
		{
			for (int kk = 0; kk < length; kk++)
			{
				rgbx[kk] = imgBot[index + kk];
			}
			index += length;
			colourIndex = checkColour(rgbx);
			substrate[ii][jj] = colourIndex;
		}
	}

}
vector<vector<int>> OVERLAY::matchAll(vector<unsigned char>& img, vector<int>& imgSpec)
{
	// Return form [possibility index][# of matches, img TLx, img TLy]
	// All coordinates are relative to the substrate's dimensions [0, BR]
	if (imgSpec[2] != length) { jf.err("rgbx mismatch-ov.matchAll"); }
	int matchSize = 10, colourIndexTop;
	vector<vector<int>> match(matchSize, vector<int>(3, 0));
	vector<int> matchTemp(3);
	vector<unsigned char> rgbxTop;
	POINT TL, p1;

	TL.y = 1 - imgSpec[1];
	while (TL.y <= BR.y)
	{
		TL.x = 1 - imgSpec[0];
		while (TL.x <= BR.x)
		{
			matchTemp = { 0, TL.x, TL.y };
			for (int ii = 0; ii < imgSpec[1]; ii++)
			{
				p1.y = TL.y + ii;
				if (p1.y < 0 || p1.y > BR.y) { continue; }
				for (int jj = 0; jj < imgSpec[0]; jj++)
				{
					p1.x = TL.x + jj;
					if ( p1.x < 0 || p1.x > BR.x) { continue; }
					rgbxTop = im.pixelRGB(img, imgSpec, p1);
					colourIndexTop = checkColour(rgbxTop);
					if (colourIndexTop < 0) { continue; }
					if (colourIndexTop == substrate[ii][jj]) { matchTemp[0]++; }
				}
			}
			if (matchTemp[0] > match[matchSize - 1][0])
			{
				for (int ii = 0; ii < matchSize; ii++)
				{
					if (matchTemp[0] > match[ii][0])
					{
						for (int jj = matchSize - 1; jj >= 1; jj--)
						{
							if (jj == ii) { break; }
							match[jj] = match[jj - 1];
						}
						match[ii] = matchTemp;
						break;
					}
				}
			}
			TL.x++;
		}
		TL.y++;
	}
	return match;
}
void OVERLAY::printSuperposition(string& pngPath, vector<int> TL)
{
	vector<unsigned char> imgSuper;
	vector<int> imgSpecSuper(3);
	if (TL[0] < 0) { imgSpecSuper[0] = BR.x - TL[0] + 1; }
	else { imgSpecSuper[0] = TL[0] + pngSpecTop[0]; }
	if (TL[1] < 0) { imgSpecSuper[1] = BR.y - TL[1] + 1; }
	else { imgSpecSuper[1] = TL[1] + pngSpecTop[1]; }
	imgSpecSuper[2] = length;
	vector<unsigned char> rgbx;
	if (length == 3) { rgbx = { 0, 0, 0 }; }
	else if (length == 4) { rgbx = { 0, 0, 0, 255 }; }
	im.pngCanvas(imgSpecSuper, imgSuper, rgbx);
	POINT topTL, botTL;
	if (TL[0] > 0)
	{
		topTL.x = 0;
		botTL.x = TL[0];
		if (TL[1] > 0)
		{
			topTL.y = 0;
			botTL.y = TL[1];
			im.pngPaste(imgSuper, imgSpecSuper, pngBot, pngSpecBot, botTL);
			im.pngPaste(imgSuper, imgSpecSuper, pngTop, pngSpecTop, topTL);
		}
		else
		{
			topTL.y = abs(TL[1]);
			botTL.y = 0;
			im.pngPaste(imgSuper, imgSpecSuper, pngBot, pngSpecBot, botTL);
			im.pngPaste(imgSuper, imgSpecSuper, pngTop, pngSpecTop, topTL);
		}
	}
	else
	{
		topTL.x = abs(TL[0]);
		botTL.x = 0;
		if (TL[1] > 0)
		{
			topTL.y = 0;
			botTL.y = TL[1];
			im.pngPaste(imgSuper, imgSpecSuper, pngBot, pngSpecBot, botTL);
			im.pngPaste(imgSuper, imgSpecSuper, pngTop, pngSpecTop, topTL);
		}
		else
		{
			topTL.y = abs(TL[1]);
			botTL.y = 0;
			im.pngPaste(imgSuper, imgSpecSuper, pngBot, pngSpecBot, botTL);
			im.pngPaste(imgSuper, imgSpecSuper, pngTop, pngSpecTop, topTL);
		}
	}
	im.pngPrint(pngPath, imgSuper, imgSpecSuper);
}
void OVERLAY::reportSuperposition(SWITCHBOARD& sbgui, vector<vector<int>>& match, OVERLAY ov)
{
	thread::id myid = this_thread::get_id();
	vector<int> mycomm;
	int myIndex = sbgui.answer_call(myid, mycomm) - 2;  // GUI and manager occupy 0,1
	int startRow = match[0][0];
	match[0][0] = 0;
	int stopRow = match[0][1];
	match[0][1] = 0;
	if (ov.substrate.size() < 1) { jf.err("No init-ov.createSuperposition"); }
	int matchSize = match.size(), colourIndexTop;
	vector<int> matchTemp(3);
	vector<unsigned char> rgbxTop;
	POINT TL, p1;

	TL.y = startRow;
	while (TL.y <= stopRow)
	{
		TL.x = 1 - ov.pngSpecTop[0];
		while (TL.x <= ov.BR.x)
		{
			matchTemp = { 0, TL.x, TL.y };
			for (int ii = 0; ii < ov.pngSpecTop[1]; ii++)
			{
				p1.y = TL.y + ii;
				if (p1.y < 0 || p1.y > ov.BR.y) { continue; }
				for (int jj = 0; jj < ov.pngSpecTop[0]; jj++)
				{
					p1.x = TL.x + jj;
					if (p1.x < 0 || p1.x > ov.BR.x) { continue; }
					rgbxTop = im.pixelRGB(ov.pngTop, ov.pngSpecTop, p1);
					colourIndexTop = checkColour(rgbxTop);
					if (colourIndexTop < 0) { continue; }
					if (colourIndexTop == ov.substrate[ii][jj]) { matchTemp[0]++; }
				}
			}
			if (matchTemp[0] > match[matchSize - 1][0])
			{
				for (int ii = 0; ii < matchSize; ii++)
				{
					if (matchTemp[0] > match[ii][0])
					{
						for (int jj = matchSize - 1; jj >= 1; jj--)
						{
							if (jj == ii) { break; }
							match[jj] = match[jj - 1];
						}
						match[ii] = matchTemp;
						break;
					}
				}
			}
			TL.x++;
		}
		mycomm[1]++;
		sbgui.update(myid, mycomm);
		TL.y++;
	}
	mycomm[0] = 1;
	sbgui.update(myid, mycomm);
	sbgui.terminateSelf(myid);
}
vector<int> OVERLAY::setTopPNG(vector<unsigned char>& img, vector<int>& imgSpec)
{
	pngTop = img;
	pngSpecTop = imgSpec;
	vector<int> minMax = { 1 - imgSpec[1], BR.y };
	return minMax;
}
