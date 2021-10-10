#pragma once

#include "jfunc.h"

using namespace std;

class JTREE
{
	int count = 0;
	int invisRoot = 0;
	JFUNC jf;
	bool removePath = 0;
	vector<vector<vector<int>>> vvviGenealogy;  // Form [cloneIndex][candidate][treeIndex, parentIndex, grandparentIndex, ...]

public:
	JTREE() {}                        // Reserved inames: -1(root), -2(folder)
	~JTREE() {}

	int expandGeneration = 0;         // < 0 -> all, else is number of generations removed from root to expand.
	int fontRootSize = -1;            // < 0 -> all nodes have same size. Else, children decrease by one. 
	vector<int> generationCount;      // Index here represents number of ancestors, value is the count of nodes at that index.
	unordered_map<string, string> mapRegion;  // sRegion -> sID
	unordered_map<string, int> mapS;  // sName -> index
	unordered_map<int, int> mapI;     // iName -> index
	int maxLength = 0;
	string pathRoot;                       // Path to root folder. 
	string nameRoot;                       // Name given to the root tree item.
	int selectedIndex = -1;
	vector<vector<int>> treeSTanc;         // Form [index][anc1, anc2, ...]
	vector<vector<int>> treeSTdes;         // Form [index][des1, des2, ...]
	vector<string> treePL;                 // snames by index.
	vector<int> treePLi;                   // inames by index.

	void addBranchSTPL(vector<vector<int>>& treeST, vector<string>& treePL, vector<int>& treePLi, string sTrunk);
	int addChild(string sname, int iname, string sparent);
	int addChild(string sname, int iname, int iparent);
	void addChildWorker(string sname, int iname, int indexParent);
	void addChildren(vector<string>& snames, vector<int>& inames, string sparent);
	void addChildren(vector<string>& snames, vector<int>& inames, int iparent);
	void addClone(int cloneIndex, int treeIndex);
	bool alreadyExist(string sname, int iname, int parentIndex);
	int beginClone(string sName);
	void clear();
	void deleteChildren(string& sParent);
	void deleteLeaves(string& sParent);
	void deleteNodeHelper(int index);
	bool isExpanded(string sName);
	bool isExpanded(int treeIndex);
	void init(string nameRoot, int iRoot);
	void init(string nameRoot, string pathRoot);
	void inputTreeDB(vector<vector<string>>& vvsGeo);
	void inputTreeSTPL(vector<vector<int>>& tree_st, vector<string>& tree_pl, vector<int>& tree_ipl);
	int getCount() { return count; }
	int getGeneration(string sName);
	int getIName(string sName);
	int getIndex(int iName);
	int getIndex(string sNode);
	int getIndex(vector<string>& vsGenealogy);
	string getParent(string sName);
	bool getRemovePath();
	string getRootName();
	int listChildren(int& iParent, vector<int>& iKids, vector<string>& sKids);
	int listChildren(string& sparent, vector<string>& skids);
	int listChildren(string& sparent, vector<int>& ikids, vector<string>& skids);
	int listChildren(vector<string>& vsGenealogy, vector<string>& vsKids);
	void listChildrenAll(string& sparent, vector<string>& skids);
	void setExpandGeneration(int maxItem);
	void setRemovePath(int yesno);

};
