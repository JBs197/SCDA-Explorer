#include "jtree.h"

void JTREE::addBranchSTPL(vector<vector<int>>& treeST, vector<string>& treePL, vector<int>& treePLi, string sTrunk)
{
	// Add a STPL tree as a branch to the chosen (existing) parent, sTrunk.
	if (treeST.size() != treePL.size() || treePL.size() != treePLi.size())
	{
		jf.err("Input mismatch-jt.addBranchSTPL");
	}
	vector<vector<int>> genLayers;
	int pivot, myIndex, parentIndex;
	string sParent;
	for (int ii = 0; ii < treeST.size(); ii++)
	{
		pivot = jf.getPivot(treeST[ii]);
		while (pivot >= genLayers.size()) { genLayers.push_back(vector<int>()); }
		genLayers[pivot].push_back(ii);
	}
	for (int ii = 0; ii < genLayers.size(); ii++)
	{
		for (int jj = 0; jj < genLayers[ii].size(); jj++)
		{
			myIndex = genLayers[ii][jj];
			if (ii == 0) { sParent = sTrunk; }
			else
			{
				pivot = jf.getPivot(treeST[myIndex]);
				parentIndex = treeST[myIndex][pivot - 1];
				sParent = treePL[parentIndex];
			}
			addChild(treePL[myIndex], treePLi[myIndex], sParent);
		}
	}
}
int JTREE::addChild(string sname, int iname, string sparent)
{
	// Returns TRUE if parent is a clone. 
	int indexParent;
	if (mapS.count(sparent))
	{
		indexParent = mapS.at(sparent);
		if (indexParent < 0) { return 1; }
	}
	else { jf.err("mapS-jtree.addChild"); }
	addChildWorker(sname, iname, indexParent);
	return 0;
}
int JTREE::addChild(string sname, int iname, int iparent)
{
	// Returns TRUE if parent is a clone. 
	int indexParent;
	if (mapI.count(iparent))
	{
		indexParent = mapI.at(iparent);
		if (indexParent < 0) { return 1; }
	}
	else { jf.err("mapI-jtree.addChild"); }
	addChildWorker(sname, iname, indexParent);
	return 0;
}
void JTREE::addChildWorker(string sname, int iname, int indexParent)
{
	int cloneIndex = 1, iGen, iName;
	if (iname < -1) { iName = -1 * (count + 1); }  // Unique dummy values.
	else { iName = iname; }
	if (alreadyExist(sname, iName, indexParent)) { return; }

	if (mapS.count(sname))
	{
		cloneIndex = mapS.at(sname);
		if (cloneIndex >= 0)
		{
			cloneIndex = beginClone(sname);
		}
	}
	else { mapS.emplace(sname, count); }

	treeSTdes[indexParent].push_back(count);
	mapI.emplace(iName, count);
	treePL.push_back(sname);
	treePLi.push_back(iName);
	treeSTdes.push_back(vector<int>());
	if (treeSTanc[indexParent][0] >= 0)  // If parent is not root...
	{
		treeSTanc.push_back(treeSTanc[indexParent]);
		treeSTanc[count].push_back(indexParent);
	}
	else { treeSTanc.push_back({ 0 }); }

	if (cloneIndex < 0) { addClone(cloneIndex, count); }

	iGen = treeSTanc[count].size() - invisRoot;
	if (iGen >= generationCount.size())
	{
		generationCount.push_back(1);
	}
	else { generationCount[iGen]++; }

	int length = sname.size();
	if (length > maxLength) { maxLength = length; }
	count++;
}
void JTREE::addChildren(vector<string>& snames, vector<int>& inames, string sparent)
{
	if (snames.size() != inames.size()) { jf.err("Size mismatch-jt.addChildren"); }
	int indexParent;
	try { indexParent = mapS.at(sparent); }
	catch (out_of_range) { jf.err("mapS-jtree.addChildren"); }
	for (int ii = 0; ii < snames.size(); ii++)
	{
		addChildWorker(snames[ii], inames[ii], indexParent);
	}
}
void JTREE::addChildren(vector<string>& snames, vector<int>& inames, int iparent)
{
	if (snames.size() != inames.size()) { jf.err("Size mismatch-jt.addChildren"); }
	int indexParent;
	try { indexParent = mapI.at(iparent); }
	catch (out_of_range) { jf.err("mapI-jtree.addChildren"); }
	for (int ii = 0; ii < snames.size(); ii++)
	{
		addChildWorker(snames[ii], inames[ii], indexParent);
	}
}
void JTREE::addClone(int cloneIndex, int treeIndex)
{
	vector<int> viAncestry = treeSTanc[treeIndex];
	int tempIndex = vvviGenealogy[-1 * cloneIndex].size();
	vvviGenealogy[-1 * cloneIndex].push_back({ treeIndex });
	for (int ii = viAncestry.size() - 1; ii >= 0; ii--)
	{
		vvviGenealogy[-1 * cloneIndex][tempIndex].push_back(viAncestry[ii]);
	}
}
bool JTREE::alreadyExist(string sname, int iname, int parentIndex)
{
	int myIndex1, myIndex2;
	try
	{
		myIndex1 = mapS.at(sname);
		myIndex2 = mapI.at(iname);
	}
	catch (out_of_range& oor) { return 0; }
	for (int ii = 0; ii < treeSTdes[parentIndex].size(); ii++)
	{
		if (treeSTdes[parentIndex][ii] == myIndex1 && myIndex1 == myIndex2)
		{
			return 1;
		}
	}
	return 0;
}
int JTREE::beginClone(string sName)
{
	// In response to a new child having the same sName as a previous node, begin
	// a genealogy line for the existing node, and change mapS to give a negative
	// index for future incidents.
	if (!mapS.count(sName)) { jf.err("sName not found-jtree.beginClone"); }
	int oldIndex = mapS.at(sName);
	vector<int> viAncestry = treeSTanc[oldIndex];

	int cloneIndex = vvviGenealogy.size();
	vvviGenealogy.push_back({ {oldIndex} });
	for (int ii = viAncestry.size() - 1; ii >= 0; ii--)
	{
		vvviGenealogy[cloneIndex][0].push_back(viAncestry[ii]);
	}

	cloneIndex *= -1;
	mapS.erase(sName);
	mapS.emplace(sName, cloneIndex);
	return cloneIndex;
}
void JTREE::clear()
{
	nameRoot.clear();
	pathRoot.clear();
	treePL.clear();
	treePLi.clear();
	treeSTanc.clear();
	treeSTdes.clear();
	mapI.clear();
	mapS.clear();
	mapRegion.clear();
	vvviGenealogy.clear();
	generationCount.clear();
	selectedIndex = -1;
	maxLength = 0;
	count = 0;
}
void JTREE::deleteChildren(string& sParent)
{
	int indexNode;
	try { indexNode = mapS.at(sParent); }
	catch (out_of_range& oor) { return; }
	vector<int> vKids = treeSTdes[indexNode];
	for (int ii = vKids.size() - 1; ii >= 0; ii--)
	{
		deleteNodeHelper(vKids[ii]);
		treeSTdes[indexNode].erase(treeSTdes[indexNode].begin() + ii);
	}
}
void JTREE::deleteLeaves(string& sParent)
{
	int indexNode, numGrandKids;
	try { indexNode = mapS.at(sParent); }
	catch (out_of_range& oor) { return; }
	vector<int> vKids = treeSTdes[indexNode], vGKids;
	for (int ii = vKids.size() - 1; ii >= 0; ii--)
	{
		vGKids = treeSTdes[vKids[ii]];
		numGrandKids = vGKids.size();
		if (numGrandKids == 0)
		{
			mapS.erase(treePL[vKids[ii]]);
			mapI.erase(treePLi[vKids[ii]]);
			treePL[vKids[ii]].clear();
			treePLi[vKids[ii]] = -2;
			treeSTanc[vKids[ii]] = { -2 };
			treeSTdes[indexNode].erase(treeSTdes[indexNode].begin() + ii);
		}
	}
}
void JTREE::deleteNodeHelper(int index)
{
	vector<int> vKids = treeSTdes[index];
	if (vKids.size() == 0)
	{
		mapS.erase(treePL[index]);
		mapI.erase(treePLi[index]);
		treePL[index].clear();
		treePLi[index] = -2;
		treeSTanc[index] = { -2 };
	}
	else
	{
		for (int ii = vKids.size() - 1; ii >= 0; ii--)
		{
			deleteNodeHelper(vKids[ii]);
			treeSTdes[index].erase(treeSTdes[index].begin() + ii);
		}
	}
}
int JTREE::getGeneration(string sName)
{
	// Returns the number of generations removed from the root. 
	int index = getIndex(sName);
	int iGen = treeSTanc[index].size() - invisRoot;
	if (iGen == 1 && treeSTanc[index][0] < 0) { return 0; }
	return iGen;
}
bool JTREE::isExpanded(string sName)
{
	if (expandGeneration < 0) { return 1; }
	int treeIndex = getIndex(sName);
	if (treeIndex < 0) { return 0; }
	bool expanded = isExpanded(treeIndex);
	return expanded;
}
bool JTREE::isExpanded(int treeIndex)
{
	if (expandGeneration < 0) { return 1; }
	else if (expandGeneration == 0) 
	{ 
		if (treeIndex == 0) { return 1; }  // Always expand the root.
		else { return 0; }
	}

	int generation = treeSTanc[treeIndex].size();
	if (generation <= expandGeneration) { return 1; }
	return 0;
}
void JTREE::init(string nR, int iRoot)
{
	// This variant is made primarily for database tables. 
	nameRoot = nR;
	pathRoot.clear();
	treePL.clear();
	treePLi.clear();
	treeSTanc.clear();
	treeSTdes.clear();
	mapI.clear();
	mapS.clear();
	mapRegion.clear();
	vvviGenealogy.clear();
	treePL.push_back(nameRoot);
	treePLi.push_back(iRoot);
	treeSTanc.push_back({ -1 });
	treeSTdes.push_back(vector<int>());
	mapI.emplace(iRoot, 0);
	mapS.emplace(nameRoot, 0);
	vvviGenealogy.resize(1, vector<vector<int>>());
	selectedIndex = -1;
	maxLength = 0;
	count = 1;
	if (nameRoot == "") { invisRoot = 1; }
	if (invisRoot) { generationCount = { 0 }; }
	else { generationCount = { 1 }; }
}
void JTREE::init(string nR, string pR)
{
	// This variant is made primarily for local files. 
	nameRoot = nR;
	pathRoot = pR;
	treePL.clear();
	treePLi.clear();
	treeSTanc.clear();
	treeSTdes.clear();
	mapI.clear();
	mapS.clear();
	mapRegion.clear();
	vvviGenealogy.clear();
	treePL.push_back(nameRoot);
	treePLi.push_back(-1);
	treeSTanc.push_back({ -1 });
	treeSTdes.push_back(vector<int>());
	mapI.emplace(-1, 0);
	mapS.emplace(nameRoot, 0);
	vvviGenealogy.resize(1, vector<vector<int>>());
	selectedIndex = -1;
	maxLength = 0;
	count = 1;
	if (nameRoot == "") { invisRoot = 1; }
	if (invisRoot) { generationCount = { 0 }; }
	else { generationCount = { 1 }; }
}
void JTREE::inputTreeDB(vector<vector<string>>& vvsGeo)
{
	// Function assumes the geo table has columns (GEO_CODE, Region Name, GEO_LEVEL, 
	// Ancestor0, Ancestor1, ...). Also assumes sorted by increasing GEO_LEVEL.
	int index, geoCode, geoCodeParent;
	if (vvsGeo[0][1] != nameRoot) // Not yet initialized.
	{ 
		try { geoCode = stoi(vvsGeo[0][0]); }
		catch (invalid_argument) { jf.err("stoi-jt.inputTreeDB"); }
		init(vvsGeo[0][1], geoCode);
	}
	for (int ii = 0; ii < vvsGeo.size(); ii++)
	{
		if (vvsGeo[ii][2] == "0") { index = ii; }
		else { break; }
	}
	if (index != 0) { jf.err("Multiple roots-jt.inputTreeDB"); }

	for (int ii = 1; ii < vvsGeo.size(); ii++)
	{
		try
		{
			geoCode = stoi(vvsGeo[ii][0]);
			geoCodeParent = stoi(vvsGeo[ii].back());
		}
		catch (invalid_argument) { jf.err("stoi-jt.inputTreeDB"); }
		addChild(vvsGeo[ii][1], geoCode, geoCodeParent);
	}
}
void JTREE::inputTreeSTPL(vector<vector<int>>& tree_st, vector<string>& tree_pl, vector<int>& tree_ipl)
{
	// NOTE: This function will not insert the STPL tree's first entry, as it is presumed
	// to be the tree root which the JTREE builds itself during initialization.
	if (tree_st[0][0] != 0) { jf.err("Bad root-jt.inputTreeSTPL"); }
	int iRoot = -1;
	int indexGen, indexTree, pivot, indexParent;
	vector<int> kids;
	vector<vector<int>> genLayers;
	genLayers.push_back(tree_st[0]);
	genLayers[0].erase(genLayers[0].begin());
	int numKids = genLayers[0].size();
	for (int ii = 0; ii < numKids; ii++)
	{
		addChild(tree_pl[genLayers[0][ii]], tree_ipl[genLayers[0][ii]], iRoot);  // Root always has index -1.
	}
	while (numKids > 0)
	{
		indexGen = genLayers.size();
		genLayers.push_back(vector<int>());
		numKids = 0;
		for (int ii = 0; ii < genLayers[indexGen - 1].size(); ii++)  // For every parent...
		{
			indexTree = genLayers[indexGen - 1][ii];
			pivot = jf.getPivot(tree_st[indexTree]);
			if (pivot >= tree_st[indexTree].size() - 1) { continue; }
			kids.clear();
			kids.insert(kids.begin(), tree_st[indexTree].begin() + pivot + 1, tree_st[indexTree].end());
			for (int jj = 0; jj < kids.size(); jj++)  // For every child...
			{
				genLayers[indexGen].push_back(kids[jj]);
				numKids++;
				addChild(tree_pl[kids[jj]], tree_ipl[kids[jj]], tree_pl[indexTree]);
			}
		}
	}
}
int JTREE::getIName(string sName)
{
	int index;
	try { index = mapS.at(sName); }
	catch (out_of_range) { jf.err("mapS-jt.getIName"); }
	return treePLi[index];
}
int JTREE::getIndex(int iName)
{
	int index;
	try { index = mapI.at(iName); }
	catch (out_of_range) { jf.err("mapI-jt.getIndex"); }
	return index;
}
int JTREE::getIndex(string sNode)
{
	int index;
	if (mapS.count(sNode))
	{
		index = mapS.at(sNode);
	}
	else { jf.err("mapS-jt.getIndex"); }
	return index;
}
int JTREE::getIndex(vector<string>& vsGenealogy)
{
	// This variant is made for clones. vsGenealogy has form [sName, sParent, sGrandparent, ...]
	int ancestorIndex;
	int cloneIndex = mapS.at(vsGenealogy[0]);
	if (cloneIndex >= 0) { return cloneIndex; }
	vector<vector<int>> vviCandidate = vvviGenealogy[-1 * cloneIndex];
	for (int ii = 1; ii < vsGenealogy.size(); ii++)
	{
		ancestorIndex = mapS.at(vsGenealogy[ii]);
		if (ancestorIndex < 0)
		{
			vector<string> vsGenealogyParent;
			vsGenealogyParent.assign(vsGenealogy.begin() + ii, vsGenealogy.end());
			ancestorIndex = getIndex(vsGenealogyParent);
		}
		for (int jj = vviCandidate.size() - 1; jj >= 0; jj--)
		{
			if (vviCandidate[jj][ii] != ancestorIndex)
			{
				vviCandidate.erase(vviCandidate.begin() + jj);
			}
		}

		if (vviCandidate.size() < 1) { jf.err("Eliminated all clone candidates-jtree.listChildrenForClones"); }
		else if (vviCandidate.size() == 1)
		{
			return vviCandidate[0][0];
		}
	}
	return -1;
}
string JTREE::getParent(string sName)
{
	int index = getIndex(sName);
	int indexParent = treeSTanc[index].back();
	if (indexParent < 0) { return ""; }
	return treePL[indexParent];
}
bool JTREE::getRemovePath()
{
	return removePath;
}
string JTREE::getRootName()
{
	string sRoot = nameRoot;
	return sRoot;
}
int JTREE::listChildren(int& iParent, vector<int>& iKids, vector<string>& sKids)
{
	// Populates a list of iParent's immediate children.
	int pIndex;
	if (mapI.count(iParent))
	{
		pIndex = mapI.at(iParent);
		if (pIndex < 0) { return 1; }
	}
	else { jf.err("mapI-jtree.listChildren"); }

	vector<int> viTemp = treeSTdes[pIndex];
	sKids.resize(viTemp.size());
	iKids.resize(viTemp.size());
	for (int ii = 0; ii < viTemp.size(); ii++)
	{
		sKids[ii] = treePL[viTemp[ii]];
		iKids[ii] = treePLi[viTemp[ii]];
	}
	return 0;
}
int JTREE::listChildren(string& sparent, vector<string>& skids)
{
	// Populates a list of sparent's immediate children.
	vector<int> viDummy;
	int clone = listChildren(sparent, viDummy, skids);
	return clone;
}
int JTREE::listChildren(string& sparent, vector<int>& ikids, vector<string>& skids)
{
	// Populates a list of sparent's immediate children.
	int pIndex;
	if (mapS.count(sparent))
	{
		pIndex = mapS.at(sparent);
		if (pIndex < 0) { return 1; }
	}
	else { jf.err("mapS-jtree.listChildren"); }

	vector<int> viTemp = treeSTdes[pIndex];
	skids.resize(viTemp.size());
	ikids.resize(viTemp.size());
	for (int ii = 0; ii < viTemp.size(); ii++)
	{
		skids[ii] = treePL[viTemp[ii]];
		ikids[ii] = treePLi[viTemp[ii]];
	}
	return 0;
}
int JTREE::listChildren(vector<string>& vsGenealogy, vector<string>& vsKids)
{
	// This variant is made for clones. vsGenealogy has form [sName, sParent, sGrandparent, ...]
	int treeIndex = getIndex(vsGenealogy);
	if (treeIndex < 0) { jf.err("Failed to get clone's index-jtree.listChildren"); }

	vector<int> viKids = treeSTdes[treeIndex];
	vsKids.resize(viKids.size());
	for (int jj = 0; jj < viKids.size(); jj++)
	{
		vsKids[jj] = treePL[viKids[jj]];
	}
	return treeIndex;
}
void JTREE::listChildrenAll(string& sparent, vector<string>& vsKids)
{
	// Recursively populates a list of all sparent's descendants.
	// Note that vsKids should be cleared beforehand. 
	int pIndex;
	try { pIndex = mapS.at(sparent); }
	catch (out_of_range& oor) { jf.err("mapS-jtree.listChildrenAll"); }

	vector<int> viTemp = treeSTdes[pIndex];
	for (int ii = 0; ii < viTemp.size(); ii++)
	{
		vsKids.push_back(treePL[viTemp[ii]]);
		listChildrenAll(vsKids[vsKids.size() - 1], vsKids);
	}
}
void JTREE::setExpandGeneration(int maxItem)
{
	// For a given maximum number of items to tolerate, set the highest
	// possible value for expandGeneration.
	int numItem = 0;
	for (int ii = 0; ii < generationCount.size(); ii++)
	{
		numItem += generationCount[ii];
		if (numItem > maxItem)
		{
			expandGeneration = ii;
			return;
		}
	}
	expandGeneration = -1;
}
void JTREE::setRemovePath(int yesno)
{
	if (yesno > 0) { removePath = 1; }
	else { removePath = 0; }
}
