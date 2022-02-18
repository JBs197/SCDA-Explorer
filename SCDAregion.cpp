#include "SCDAregion.h"

using namespace std;

void SCDAregion::err(string message)
{
	string errorMessage = "SCregion error:\n" + message;
	JLOG::getInstance()->err(errorMessage);
}
void SCDAregion::load(vector<vector<string>>& vvsBorder, vector<vector<string>>& vvsFrame)
{
	int numBorder = (int)vvsBorder.size();
	int numFrame = (int)vvsFrame.size();
	if (numBorder == 0 || numFrame != 2) { return; }

	vFrame.clear();
	vFrame.resize(numFrame);
	vBorder.clear();
	vBorder.resize(numBorder);
	try {
		for (int ii = 0; ii < numFrame; ii++) {
			vFrame[ii] = make_pair(stod(vvsFrame[ii][0]), stod(vvsFrame[ii][1]));			
		}
		if (get<0>(vFrame[1]) - get<0>(vFrame[0]) == 0.0 || get<1>(vFrame[1]) - get<1>(vFrame[0]) == 0.0) {
			err("Invalid Frame dimensions-load");
		}
		for (int ii = 0; ii < numBorder; ii++) {
			vBorder[ii] = make_pair(stod(vvsBorder[ii][0]), stod(vvsBorder[ii][1]));
		}
	}
	catch (invalid_argument) { err("Frame/Border stod-load"); }
}
void SCDAregion::makePath(QPainterPath& qpPath, double width, double height, vector<pair<double, double>> parentTLBR)
{
	// Width and height represent the pixels of the displaying widget. If a parent 
	// TLBR is not specified, the region will use its own frame.
	if (width <= 0.0 || height <= 0.0) { err("Non-positive canvas dimensions-makePath"); }
	if (vFrame.size() != 2 || vBorder.size() == 0) { return; }
	double deltaX, deltaY, scaleX, scaleY, x, y;
	bool yFlip = 0, parentFrame = 0;
	if (parentTLBR.size() == 2) { 
		parentFrame = 1; 
		scaleX = width / (get<0>(parentTLBR[1]) - get<0>(parentTLBR[0]));
		scaleY = height / (get<1>(parentTLBR[1]) - get<1>(parentTLBR[0]));
		deltaX = -1.0 * get<0>(parentTLBR[0]);
		deltaY = -1.0 * get<1>(parentTLBR[0]);
	}
	else {
		scaleX = width / (get<0>(vFrame[1]) - get<0>(vFrame[0]));
		scaleY = height / (get<1>(vFrame[1]) - get<1>(vFrame[0]));
		deltaX = -1.0 * get<0>(vFrame[0]);
		deltaY = -1.0 * get<1>(vFrame[0]);
	}
	if (scaleY < 0.0) { yFlip = 1; }

	qpPath.clear();
	int numPixel = (int)vBorder.size();
	x = (get<0>(vBorder[0]) + deltaX) * scaleX;
	y = (get<1>(vBorder[0]) + deltaY) * scaleY;
	qpPath.moveTo(x, y);
	for (int ii = 1; ii < numPixel; ii++) {
		qpPath.lineTo((get<0>(vBorder[ii]) + deltaX) * scaleX, (get<1>(vBorder[ii]) + deltaY) * scaleY);
	}
	qpPath.closeSubpath();
}
