#include "jtree.h"

using namespace std;

void JTREE::addChild(int parentID, JNODE& jnChild)
{
	// Note: child node will inherit the parent node's BG/FG colours. If
	// different colours are desired, specify them after this function completes.
	lock_guard<mutex> lg(m_tree);
	auto itParent = mapIDIndex.find(parentID);
	if (itParent == mapIDIndex.end()) {
		err("Failed to locate parentID " + to_string(parentID) + "-addChild");
	}

	vector<int> viAncestry;
	if (itParent->second == 0) {
		viAncestry = { 0 };
	}
	else {
		viAncestry = treeSTanc[itParent->second];
		viAncestry.emplace_back(itParent->second);
	}

	if (get<0>(jnChild.colour) == "") {
		jnChild.colour = vNode[viAncestry.back()].colour;
	}
	if (get<0>(jnChild.colourSelected) == "") {
		jnChild.colourSelected = vNode[viAncestry.back()].colourSelected;
	}
	
	int index;
	if (setBlankIndex.size() < 1) {
		index = (int)vNode.size();
		vNode.emplace_back(move(jnChild));
		treeSTanc.emplace_back(viAncestry);
		treeSTdes.emplace_back(vector<int>());
	}
	else {
		auto itChild = setBlankIndex.rbegin();
		index = *itChild;
		vNode[index] = move(jnChild);
		setBlankIndex.erase(index);
		treeSTanc[index] = viAncestry;
		treeSTdes[index] = vector<int>();
	}
	treeSTdes[viAncestry.back()].emplace_back(index);
	mapIDIndex.emplace(vNode[index].ID, index);
}
void JTREE::appendChildrenID(vector<int>& childrenID, int parentID)
{
	// This variant appends the IDs of the parent's children to the given list.
	lock_guard<mutex> lg(m_tree);
	auto it = mapIDIndex.find(parentID);
	if (it == mapIDIndex.end()) {
		err("Failed to locate ID " + to_string(parentID) + "-appendChildrenID");
	}

	int numChildren = (int)treeSTdes[it->second].size();
	int index = (int)childrenID.size();
	childrenID.resize(index + numChildren);
	for (int ii = 0; ii < numChildren; ii++) {
		childrenID[index + ii] = vNode[treeSTdes[it->second][ii]].ID;
	}
}
void JTREE::compare(JTREE& jtOther)
{
	// For every non-root node in this tree, compare it to jtOther
	// and note within each node whether it has a twin within jtOther.
	vector<string> myGenealogy, otherGenealogy;
	vector<int> otherID;
	bool match;
	int numGenealogy, numNode = (int)mapIDIndex.size();
	for (int ii = 1; ii < numNode; ii++) {
		myGenealogy.clear();
		match = 0;
		otherID = jtOther.searchData(vNode[ii].vsData[0]);
		if (otherID.size() < 1) { continue; }
		myGenealogy = getGenealogy(vNode[ii].ID);
		numGenealogy = (int)myGenealogy.size();
		for (int jj = 0; jj < otherID.size(); jj++) {
			otherGenealogy = jtOther.getGenealogy(otherID[jj]);
			if (otherGenealogy.size() != numGenealogy) { break; }
			else if (numGenealogy == 0) {
				match = 1;
				vNode[ii].hasTwin = 1;
			}
			else {
				for (int kk = 0; kk < numGenealogy; kk++) {
					if (myGenealogy[kk] != otherGenealogy[kk]) { break; }
					else if (kk == numGenealogy - 1) {
						match = 1;
						vNode[ii].hasTwin = 1;
					}
				}
			}

			if (match) { break; }
		}
		if (!match) { vNode[ii].hasTwin = 0; }
	}
}
void JTREE::deleteChildren(int ID)
{
	vector<reference_wrapper<JNODE>> vJN = getChildrenAll(ID);
	int inum;
	int numChildren = (int)vJN.size();
	for (int ii = numChildren - 1; ii >= 0; ii--) {
		inum = vJN[ii].get().ID;
		deleteNode(inum);
	}
}
void JTREE::deleteNode(int ID)
{
	lock_guard<mutex> lg(m_tree);
	auto it = mapIDIndex.find(ID);
	if (it == mapIDIndex.end()) {
		err("Failed to locate ID " + to_string(ID) + "-deleteNode");
	}
	int index = it->second;

	if (treeSTdes[index].size() > 0) {
		deleteChildren(ID);
	}
	treeSTdes[index] = vector<int>();
	treeSTanc[index] = vector<int>();
	setBlankIndex.emplace(index);
	mapIDIndex.erase(ID);
}
void JTREE::err(string message)
{
	string errorMessage = "JTREE error:\n" + message;
	JLOG::getInstance()->err(errorMessage);
}
vector<reference_wrapper<JNODE>> JTREE::getChildren(int ID)
{
	lock_guard<mutex> lg(m_tree);
	auto it = mapIDIndex.find(ID);
	if (it == mapIDIndex.end()) {
		err("Failed to locate ID " + to_string(ID) + "-getChildren");
	}
	
	vector<reference_wrapper<JNODE>> vJN;
	int numChildren = (int)treeSTdes[it->second].size();
	for (int ii = 0; ii < numChildren; ii++) {
		vJN.emplace_back(ref(vNode[treeSTdes[it->second][ii]]));
	}
	return vJN;
}
vector<reference_wrapper<JNODE>> JTREE::getChildrenAll(int ID)
{
	// Recursively populates a list of all ID's descendants.
	lock_guard<mutex> lg(m_tree);
	auto it = mapIDIndex.find(ID);
	if (it == mapIDIndex.end()) {
		err("Failed to locate ID " + to_string(ID) + "-getChildrenAll");
	}
	int indexParent = it->second;

	int numChildren = (int)treeSTdes[indexParent].size();
	vector<int> viChildren = treeSTdes[indexParent];
	for (int ii = 0; ii < numChildren; ii++) {
		getChildrenAllWorker(viChildren[ii], viChildren);
	}

	vector<reference_wrapper<JNODE>> vJN;
	numChildren = (int)viChildren.size();
	for (int ii = 0; ii < numChildren; ii++) {
		vJN.emplace_back(ref(vNode[viChildren[ii]]));
	}
	return vJN;
}
void JTREE::getChildrenAllWorker(int ID, vector<int>& viChildren)
{
	auto it = mapIDIndex.find(ID);
	if (it == mapIDIndex.end()) {
		err("Failed to locate ID " + to_string(ID) + "-getChildrenAll");
	}

	int numChildren = (int)treeSTdes[it->second].size();
	for (int ii = 0; ii < numChildren; ii++) {
		viChildren.push_back(treeSTdes[it->second][ii]);
		getChildrenAllWorker(treeSTdes[it->second][ii], viChildren);
	}
}
vector<int> JTREE::getChildrenID(int parentID)
{
	lock_guard<mutex> lg(m_tree);
	auto it = mapIDIndex.find(parentID);
	if (it == mapIDIndex.end()) {
		err("Failed to locate ID " + to_string(parentID) + "-getChildrenID");
	}

	int numChildren = (int)treeSTdes[it->second].size();
	vector<int> childrenID(numChildren);
	for (int ii = 0; ii < numChildren; ii++) {
		childrenID[ii] = vNode[treeSTdes[it->second][ii]].ID;
	}
	return childrenID;
}
vector<string> JTREE::getData(int ID)
{
	JNODE jn = getNode(ID);
	return jn.vsData;
}
vector<string> JTREE::getDataUserRole(int ID)
{
	JNODE jn = getNode(ID);
	vector<string> vsDataUserRole(4);
	vsDataUserRole[0] = get<0>(jn.colour);
	vsDataUserRole[1] = get<1>(jn.colour);
	vsDataUserRole[2] = get<0>(jn.colourSelected);
	vsDataUserRole[3] = get<1>(jn.colourSelected);
	return vsDataUserRole;
}
int JTREE::getExpandGeneration(int numRow)
{
	// Expand as many generations as possible without exceeding "numRow"
	// nodes on display. Note that internal indices are used here, rather
	// than unique IDs.
	vector<vector<int>> vvGenerationIndex = { {0} };
	vector<int> viChildren;
	int count = 0, index = 0, numChildren, numGen, numPrevious;
	while (1) {
		index++;
		vvGenerationIndex.push_back(vector<int>());
		numPrevious = (int)vvGenerationIndex[index - 1].size();
		for (int ii = 0; ii < numPrevious; ii++) {
			viChildren = treeSTdes[vvGenerationIndex[index - 1][ii]];
			numChildren = (int)viChildren.size();
			for (int jj = 0; jj < numChildren; jj++) {
				vvGenerationIndex[index].push_back(viChildren[jj]);
			}
			count += numChildren;
		}
		if (vvGenerationIndex[index].size() < 1) { return index - 1; }
		if (count > numRow) { break; }
	}
	if (index - 2 < 0) { numGen = 0; }
	else { numGen = index - 2; }
	return numGen;
}
vector<string> JTREE::getGenealogy(int ID, int iCol)
{
	// For the given iCol, return the list of sValues stretching from 
	// the first generation after the root, up to ID itself.
	int index;
	auto it = mapIDIndex.find(ID);
	if (it == mapIDIndex.end()) { err("ID not found-getGenealogy"); }
	else { index = it->second; }

	vector<int> viAncestry = treeSTanc[index];
	int numGenealogy = (int)viAncestry.size() - 1;
	vector<string> vsGenealogy(numGenealogy + 1);
	for (int ii = 0; ii < numGenealogy; ii++) {
		vsGenealogy[ii] = vNode[viAncestry[1 + ii]].vsData[iCol];
	}
	vsGenealogy.back() = vNode[index].vsData[iCol];
	return vsGenealogy;
}
vector<int> JTREE::getGenealogyID(int nodeID)
{
	// For the given node, return the list of IDs stretching from the 
	// first generation after the root, up to the node's immediate parent.
	int index, nodeIndex;
	auto it = mapIDIndex.find(nodeID);
	if (it == mapIDIndex.end()) { err("ID not found-getGenealogyID"); }
	else { nodeIndex = it->second; }

	int numParent = (int)treeSTanc[nodeIndex].size() - 1;
	vector<int> vID(numParent);
	for (int ii = 0; ii < numParent; ii++) {
		index = treeSTanc[nodeIndex][1 + ii];
		vID[ii] = vNode[index].ID;
	}
	return vID;
}
JNODE& JTREE::getNode(int ID)
{
	lock_guard<mutex> lg(m_tree);
	auto it = mapIDIndex.find(ID);
	if (it == mapIDIndex.end()) { 
		err("Failed to locate ID " + to_string(ID) + "-getNode"); 
	}
	return vNode[it->second];
}
JNODE& JTREE::getParent(int ID)
{
	lock_guard<mutex> lg(m_tree);
	auto it = mapIDIndex.find(ID);
	if (it == mapIDIndex.end()) {
		err("Failed to locate ID " + to_string(ID) + "-getParent");
	}
	int parentIndex = treeSTanc[it->second].back();
	return vNode[parentIndex];
}
int JTREE::getPivot(vector<int> treeSTrow)
{
	int pivot;
	int iroot = -1;
	for (int ii = 0; ii < treeSTrow.size(); ii++)
	{
		if (treeSTrow[ii] < 0) {
			pivot = ii;
			return pivot;
		}
		else if (treeSTrow[ii] == 0) { iroot = ii; }

		if (ii == treeSTrow.size() - 1 && iroot >= 0) { return iroot; }
	}
	return -1;
}
JNODE& JTREE::getRoot()
{
	return vNode[0];
}
int JTREE::hasTwin(int ID)
{
	lock_guard<mutex> lg(m_tree);
	int twinStatus = -1;
	auto it = mapIDIndex.find(ID);
	if (it != mapIDIndex.end()) {
		twinStatus = vNode[it->second].hasTwin;
	}
	return twinStatus;
}
vector<int> JTREE::hasTwinList(int twinStatus)
{
	// Return a list of node IDs which have the given twinStatus.
	lock_guard<mutex> lg(m_tree);
	vector<int> viID;
	int numNode = (int)vNode.size();
	for (int ii = 0; ii < numNode; ii++) {
		if (vNode[ii].hasTwin == twinStatus) {
			viID.push_back(vNode[ii].ID);
		}
	}
	return viID;
}
int JTREE::getID(int index)
{
	if (index >= vNode.size()) { return -1; }
	return vNode[index].ID;
}
bool JTREE::isTopLevel(int ID)
{
	// Returns whether the node's only ancestor is the invisible root.
	lock_guard<mutex> lg(m_tree);
	auto it = mapIDIndex.find(ID);
	if (it == mapIDIndex.end()) {
		err("Failed to locate ID " + to_string(ID) + "-isTopLevel");
	}
	
	if (treeSTanc[it->second].size() == 1 && treeSTanc[it->second][0] == 0) { 
		return 1; 
	}
	return 0;
}
void JTREE::log(string message)
{
	string logMessage = "JTREE log entry:\n" + message;
	JLOG::getInstance()->log(logMessage);
}
int JTREE::maxLength(int iCol)
{
	// Return the number of chars in the longest vsData entry, for the given iCol.
	int max = 0;
	for (int ii = 1; ii < vNode.size(); ii++) {
		if (vNode[ii].vsData[iCol].size() > max) {
			max = (int)vNode[ii].vsData[iCol].size();
		}
	}
	return max;
}
void JTREE::reset()
{
	// Clear pre-existing data, and make the invisible root entry.
	lock_guard<mutex> lg(m_tree);
	vNode.resize(1);
	vNode[0].expanded = 1;
	selectedID = -1;
	mapIDIndex.clear();
	mapIDIndex.emplace(vNode[0].ID, 0);
	setBlankIndex.clear();
	treeSTanc.clear();
	treeSTanc.push_back({ -1 });
	treeSTdes.clear();
	treeSTdes.resize(1, vector<int>());
}
vector<int> JTREE::searchData(string sData)
{
	// Return a list of node IDs which contain the given sData.
	vector<int> viID;
	int numData;
	int numNode = (int)vNode.size();
	for (int ii = 1; ii < numNode; ii++) {
		numData = (int)vNode[ii].vsData.size();
		for (int jj = 0; jj < numData; jj++) {
			if (vNode[ii].vsData[jj] == sData) {
				viID.push_back(vNode[ii].ID);
				break;
			}
		}
	}
	return viID;
}
vector<int> JTREE::searchData(string sData, int iCol)
{
	// Return a list of node IDs which contain the given sData.
	// Search only the vsData column given by iCol.
	vector<int> viID;
	int numData;
	int numNode = (int)vNode.size();
	for (int ii = 1; ii < numNode; ii++) {
		if (vNode[ii].vsData[iCol] == sData) {
			viID.push_back(vNode[ii].ID);
		}
	}
	return viID;
}
void JTREE::setExpandGeneration(int numRow)
{
	// Expand as many generations as possible without exceeding "numRow"
	// nodes on display. Note that internal indices are used here, rather
	// than unique IDs.
	vector<vector<int>> vvGenerationIndex = { {0} };
	vector<int> viChildren;
	int count, numCurrent, numPrevious, index = 0;
	while (1) {
		index++;
		vvGenerationIndex.push_back(vector<int>());
		numPrevious = (int)vvGenerationIndex[index - 1].size();
		for (int ii = 0; ii < numPrevious; ii++) {
			viChildren = treeSTdes[vvGenerationIndex[index - 1][ii]];
			for (int jj = 0; jj < viChildren.size(); jj++) {
				vvGenerationIndex[index].push_back(viChildren[jj]);
			}
		}
		if (vvGenerationIndex[index].size() < 1) { return; }
		count = 0;
		for (int ii = 0; ii <= index; ii++) {
			count += (int)vvGenerationIndex[ii].size();
		}
		if (count > numRow) { return; }
		else {
			numCurrent = (int)vvGenerationIndex[index].size();
			for (int ii = 0; ii < numCurrent; ii++) {
				vNode[vvGenerationIndex[index][ii]].expanded = 1;
			}
		}
	}
}
void JTREE::setNodeColour(int ID, pair<string, string> standard, pair<string, string> selected)
{
	// Colour pairs have form [background, foreground], #AARRGGBB or #RRGGBB.
	// If ID is negative, apply the colour scheme to the root node.
	if (ID < 0) {
		vNode[0].colour = standard;
		vNode[0].colourSelected = selected;
	}
	else {
		auto it = mapIDIndex.find(ID);
		if (it != mapIDIndex.end()) {
			vNode[it->second].colour = standard;
			vNode[it->second].colourSelected = selected;
		}
	}
}
void JTREE::setNodeColour(vector<int> vID, pair<string, string> standard, pair<string, string> selected)
{
	// If vID is empty, apply the colour scheme to all nodes. 
	int numNode = (int)vID.size();
	if (numNode > 0) {
		for (int ii = 0; ii < numNode; ii++) {
			auto it = mapIDIndex.find(vID[ii]);
			if (it != mapIDIndex.end()) {
				vNode[it->second].colour = standard;
				vNode[it->second].colourSelected = selected;
			}
		}
	}
	else {
		for (int ii = 0; ii < vNode.size(); ii++) {
			if (!setBlankIndex.count(ii)) {
				vNode[ii].colour = standard;
				vNode[ii].colourSelected = selected;
			}
		}
	}
}
