#include "binmap.h"

void BINMAP::borderScaleToWindow(vector<vector<int>>& border, vector<vector<double>>& borderD, vector<int> windowDim)
{
	// Note: windowDim has form [width, height, margin].
	borderD.resize(border.size(), vector<double>(2));
	vector<int> borderDim = { 0, 0 };
	for (int ii = 0; ii < border.size(); ii++)
	{
		if (border[ii][0] > borderDim[0]) { borderDim[0] = border[ii][0]; }
		if (border[ii][1] > borderDim[1]) { borderDim[1] = border[ii][1]; }
	}
	borderDim[0]++;
	borderDim[1]++;
	double scaleFactor, marginD = (double)windowDim[2];
	vector<double> windowDimD(2);
	windowDimD[0] = (double)(windowDim[0] - (2 * windowDim[2]));
	windowDimD[1] = (double)(windowDim[1] - (2 * windowDim[2]));
	double xHaveWant = (double)borderDim[0] / windowDimD[0];
	double yHaveWant = (double)borderDim[1] / windowDimD[1];
	if (xHaveWant > yHaveWant) { scaleFactor = 1.0 / xHaveWant; }
	else { scaleFactor = 1.0 / yHaveWant; }
	for (int ii = 0; ii < borderD.size(); ii++)
	{
		borderD[ii][0] = (double)border[ii][0] * scaleFactor;
		borderD[ii][0] += marginD;
		borderD[ii][1] = (double)border[ii][1] * scaleFactor;
		borderD[ii][1] += marginD;
	}
}
void BINMAP::borderShiftFrames(vector<vector<int>>& border, vector<vector<vector<int>>>& frames)
{
	for (int ii = 0; ii < border.size(); ii++)
	{
		border[ii][0] -= frames[0][0][0];
		border[ii][1] -= frames[0][0][1];
	}
}
void BINMAP::borderShiftSetMargin(vector<vector<int>>& border, int margin)
{
	vector<int> topLeft = { 2147483647, 2147483647 };
	for (int ii = 0; ii < border.size(); ii++)
	{
		if (border[ii][0] < topLeft[0]) { topLeft[0] = border[ii][0]; }
		if (border[ii][1] < topLeft[1]) { topLeft[1] = border[ii][1]; }
	}
	for (int ii = 0; ii < border.size(); ii++)
	{
		border[ii][0] += margin - topLeft[0];
		if (border[ii][0] < margin) { jf.err("Broken xCoord-bm.borderShiftSetMargin"); }
		border[ii][1] += margin - topLeft[1];
		if (border[ii][1] < margin) { jf.err("Broken yCoord-bm.borderShiftSetMargin"); }
	}
}
void BINMAP::childrenFromGeo(string geoPath)
{
	vector<int> gidList, startStop(2);
	vector<string> regionList, layerList, geoLayers;
	if (myName.size() < 1) { jf.err("Cannot load geo without myName-bm.childrenFromGeo"); }
	int loaded = sc.loadGeo(geoPath, gidList, regionList, layerList, geoLayers);
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
	string path0 = geoPath.substr(0, pos1);
	pathChildren.resize(startStop[1] - startStop[0] + 1);
	int index = 0;
	for (int ii = startStop[0]; ii <= startStop[1]; ii++)
	{
		pathChildren[index] = path0 + regionList[ii] + ".bin";
		index++;
	}
}
void BINMAP::drawFamily(vector<BINMAP>& binFamily, int selChild, QLabel*& qLabel)
{
	// Note: we assume that the first map in binFamily is the parent region. 
	if (binFamily[0].myName != myName) { jf.err("Custody dispute-bm.drawFamily"); }
	if (myBorder.size() < 1) { jf.err("Parent region not initialized-bm.drawFamily"); }
	vector<int> windowDim = { qLabel->width(), qLabel->height(), defaultMargin };  // Form [width, height, margin].
	vector<vector<double>> parentBorderD;

	vector<vector<int>> parentBorder = myBorder;
	borderShiftSetMargin(parentBorder, 1);
	borderScaleToWindow(parentBorder, parentBorderD, windowDim);


}
vector<string> BINMAP::getPathChildren()
{
	if (pathChildren.size() < 1) { jf.err("No loaded children-bm.getPathChildren"); }
	return pathChildren;
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
	string pathParent = myPath.substr(0, pos1) + myParent + ".bin";
	return pathParent;
}
void BINMAP::loadFromPath(string binPath)
{
	size_t pos1 = binPath.find("(Canada)");
	int loaded = qf.loadBinMap(binPath, myFrames, myScale, myPosition, myParent, myBorder);
	if (pos1 < binPath.size() && loaded != 3) { jf.err("loadBinMap-bm.loadFromPath"); }
	else if (pos1 > binPath.size() && loaded != 5) { jf.err("loadBinMap-bm.loadFromPath"); }
	size_t pos2 = binPath.rfind(".bin");
	pos1 = binPath.rfind('\\') + 1;
	myName = binPath.substr(pos1, pos2 - pos1);
	myPath = binPath;
}

