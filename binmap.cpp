#include "binmap.h"

void BINMAP::borderScaleToWindow(vector<vector<int>>& border, vector<vector<double>>& borderD, vector<double> borderDim)
{
	borderD.clear();
	borderD.resize(border.size(), vector<double>(2));
	vector<vector<int>> TLBR = qf.getTLBR(border);
	double scaleFactor;
	double xHaveWant = borderDim[0] / (double)(TLBR[1][0] - TLBR[0][0]);
	double yHaveWant = borderDim[1] / (double)(TLBR[1][1] - TLBR[0][1]);
	if (xHaveWant < yHaveWant) { scaleFactor = xHaveWant; }
	else { scaleFactor = yHaveWant; }
	for (int ii = 0; ii < borderD.size(); ii++)
	{
		borderD[ii][0] = (double)border[ii][0] * scaleFactor;
		borderD[ii][0] += defaultMargin;
		borderD[ii][1] = (double)border[ii][1] * scaleFactor;
		borderD[ii][1] += defaultMargin;
	}
}
void BINMAP::borderShiftSetMargin(vector<vector<int>>& border, int margin)
{
	vector<vector<int>> TLBR = qf.getTLBR(border);
	for (int ii = 0; ii < border.size(); ii++)
	{
		border[ii][0] += margin - TLBR[0][0];
		if (border[ii][0] < margin) { jf.err("Broken xCoord-bm.borderShiftSetMargin"); }
		border[ii][1] += margin - TLBR[0][1];
		if (border[ii][1] < margin) { jf.err("Broken yCoord-bm.borderShiftSetMargin"); }
	}
}
void BINMAP::childrenFromGeo(string geoPath)
{
	vector<int> gidList, startStop(2);
	vector<string> regionList, layerList, geoLayers;
	if (myName.size() < 1) { jf.err("Cannot load geo without myName-bm.childrenFromGeo"); }
	int loaded = sc.loadBinGeo(geoPath, gidList, regionList, layerList, geoLayers);
	if (loaded != 4) { jf.err("loadGeo-bm.childrenFromGeo"); }
	bool parentFound = 0;
	for (int ii = 0; ii < regionList.size(); ii++)
	{
		if (regionList[ii] == myName)
		{
			myGid = gidList[ii];
			myLayer = layerList[ii];
			if (ii == regionList.size() - 1 || layerList[ii + 1] == myLayer) { return; }
			startStop[0] = ii + 1;
			parentFound = 1;
		}
		else if (parentFound == 1 && layerList[ii] == myLayer)
		{
			startStop[1] = ii - 1;
			break;
		}
		else if (parentFound == 1 && ii == regionList.size() - 1)
		{
			startStop[1] = ii;
		}
	}
	size_t pos1 = geoPath.rfind('\\') + 1;
	string path0 = geoPath.substr(0, pos1), temp;
	pathChildren.resize(startStop[1] - startStop[0] + 1);
	int index = 0;
	for (int ii = startStop[0]; ii <= startStop[1]; ii++)
	{
		temp = jf.utf8ToAscii(regionList[ii]);
		pathChildren[index] = path0 + temp + ".bin";
		index++;
	}
}
vector<string> BINMAP::getPathChildren(int mode)
{
	// Modes: 0 = no restrictions, 1 = ignore 'partie' regions
	if (pathChildren.size() < 1) { jf.err("No loaded children-bm.getPathChildren"); }
	vector<string> pathKids;
	size_t pos1;
	switch (mode)
	{
	case 0:
		pathKids = pathChildren;
		break;
	case 1:
	{
		for (int ii = 0; ii < pathChildren.size(); ii++)
		{
			pos1 = pathChildren[ii].find("partie");
			if (pos1 > pathChildren[ii].size())
			{
				pathKids.push_back(pathChildren[ii]);
			}
		}
		break;
	}
	}
	return pathKids;
}
string BINMAP::getPathGeo()
{
	if (myPath.size() < 1) { jf.err("No initialization-bm.getPathGeo"); }
	size_t pos1 = myPath.rfind('\\') + 1;
	string pathGeo = myPath.substr(0, pos1) + "geo layers.txt";
	return pathGeo;
}
string BINMAP::getPathParent()
{
	if (myPath.size() < 1 || myParent.size() < 1) { jf.err("No initialization-bm.getPathParent"); }
	size_t pos1 = myPath.rfind('\\');
	pos1 = myPath.rfind('\\', pos1 - 1) + 1;
	string pathParent8 = myPath.substr(0, pos1) + myParent + ".bin";
	string pathParent = jf.utf8ToAscii(pathParent8);
	return pathParent;
}
vector<vector<double>> BINMAP::getTLBR(vector<vector<double>>& borderPath)
{
	vector<vector<double>> TLBR(2, vector<double>(2));
	TLBR[0] = { 1000000.0, 1000000.0 };
	TLBR[1] = { -1000000.0, -1000000.0 };
	for (int ii = 0; ii < borderPath.size(); ii++)
	{
		if (borderPath[ii][0] < TLBR[0][0]) { TLBR[0][0] = borderPath[ii][0]; }
		else if (borderPath[ii][0] > TLBR[1][0]) { TLBR[1][0] = borderPath[ii][0]; }
		if (borderPath[ii][1] < TLBR[0][1]) { TLBR[0][1] = borderPath[ii][1]; }
		else if (borderPath[ii][1] > TLBR[1][1]) 
		{ 
			TLBR[1][1] = borderPath[ii][1]; 
		}
	}
	return TLBR;
}
void BINMAP::loadFromPath(string binPath)
{
	size_t pos1 = binPath.find("(Canada)");
	int loaded = qf.loadBinMap(binPath, myFrames, myScale, myPosition, myParent, myBorder);
	if (pos1 < binPath.size() && loaded != 3) { jf.err("loadBinMap-bm.loadFromPath"); }
	else if (pos1 > binPath.size() && loaded != 5) { jf.err("loadBinMap-bm.loadFromPath"); }
	size_t pos2 = binPath.rfind(".bin");
	pos1 = binPath.rfind('\\') + 1;
	string temp = binPath.substr(pos1, pos2 - pos1);
	myName = jf.asciiToUTF8(temp);
	myPath = binPath;
}
void BINMAP::makeBlueDot()
{
	// Note: for child regions only!
	if (myPath.size() < 1) { jf.err("No initialization-bm.makeBlueDot"); }
	vector<string> dirt = { "mapsBIN", ".bin" }, soap = { "pos", ".png" };
	string miniPath = myPath;
	jf.clean(miniPath, dirt, soap);
	blueDot = qf.makeBlueDot(miniPath);
}
void BINMAP::makeWindowBorder(double width, double height)
{
	// Note: for parent regions only!
	if (myBorder.size() < 1) { jf.err("Parent region not initialized-bm.drawFamily"); }
	vector<double> borderDim = { width - (2.0 * defaultMargin), height - (2.0 * defaultMargin)};
	vector<vector<int>> parentBorder = myBorder;
	borderShiftSetMargin(parentBorder, 1);
	borderScaleToWindow(parentBorder, myWindowBorder, borderDim);
}
