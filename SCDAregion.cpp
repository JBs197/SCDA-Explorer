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

	vBorder.clear();
	vFrame.clear();
	try {
		vBorder.resize(numBorder);
		for (int ii = 0; ii < numBorder; ii++) {
			vBorder[ii] = make_pair(stod(vvsBorder[ii][0]), stod(vvsBorder[ii][1]));
		}
		vFrame.resize(numFrame);
		for (int ii = 0; ii < numFrame; ii++) {
			vFrame[ii] = make_pair(stod(vvsFrame[ii][0]), stod(vvsFrame[ii][1]));
		}
	}
	catch (invalid_argument) { err("Border/Frame stod-loadCoord"); }
}
