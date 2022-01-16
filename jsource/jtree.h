#pragma once
#include <map>
#include <mutex>
#include <set>
#include "jstring.h"

using namespace std;

struct JNODE
{
	JNODE(int numCol) : ID(getNextID()) {
		empty = 0;
		expanded = 0;
		hasTwin = 0;
		prefix = "";
		vsData.resize(numCol);
	}
	JNODE() : ID(getNextID()) { 
		empty = 0;
		expanded = 0;
		hasTwin = 0;
		prefix = "";
		vsData.resize(1);
	}
	~JNODE() {}

	bool empty, expanded;
	int hasTwin;
	const int ID;
	static int nextID;
	pair<string, string> colour;  // Form [BG, FG] #AARRGGBB#RRGGBB or #RRGGBB
	pair<string, string> colourSelected;  // Form [BG, FG] #AARRGGBB#RRGGBB or #RRGGBB
	map<string, string> mapAttribute;
	size_t posStart, posStop;
	string prefix;
	vector<string> vsData, vsItemRole;

	static int getNextID() { return nextID++; }
	
	JNODE operator=(JNODE jn) {
		JNODE jnCopy;
		jnCopy.vsData = jn.vsData;
		return jnCopy;
	}
};

class JTREE
{
	friend class JTXML;

private:
	mutex m_tree;
	unordered_map<int, int> mapIDIndex;  // Unique ID -> index
	set<int> setBlankIndex;
	vector<vector<int>> treeSTanc;       // Form [index][anc1, anc2, ...]
	vector<vector<int>> treeSTdes;       // Form [index][des1, des2, ...]
	vector<JNODE> vNode;

	void err(string message);
	void getChildrenAllWorker(int ID, vector<int>& viChildren);
	void log(string message);

public:
	JTREE(const JTREE& jtOld) : mapIDIndex(jtOld.mapIDIndex), 
		setBlankIndex(jtOld.setBlankIndex), treeSTanc(jtOld.treeSTanc), 
		treeSTdes(jtOld.treeSTdes), vNode(jtOld.vNode) {
		selectedID = -1;
	}
	JTREE() { reset(); }
	~JTREE() {}

	JTREE& operator=(const JTREE& jtOld) {
		mapIDIndex = jtOld.mapIDIndex;
		setBlankIndex = jtOld.setBlankIndex;
		treeSTanc = jtOld.treeSTanc;
		treeSTdes = jtOld.treeSTdes;
		vNode = jtOld.vNode;
		return *this;
	}

	int selectedID;

	void addChild(int parentID, JNODE& jnChild);
	void compare(JTREE& jtOther);
	void deleteChildren(int ID);
	void deleteNode(int ID);
	vector<reference_wrapper<JNODE>> getChildren(int ID);
	vector<reference_wrapper<JNODE>> getChildrenAll(int ID);
	vector<int> getChildrenID(int parentID);
	vector<string> getData(int ID);
	vector<string> getDataUserRole(int ID);
	int getExpandGeneration(int numRow);
	vector<string> getGenealogy(int ID, int iCol = 0);
	vector<int> getGenealogyID(int nodeID);
	int getID(int index);
	JNODE& getNode(int ID);
	JNODE& getParent(int ID);
	int getPivot(vector<int> treeSTrow);
	JNODE& getRoot();
	int hasTwin(int ID);
	vector<int> hasTwinList(int twinStatus);
	bool isTopLevel(int ID);
	int maxLength(int iCol);
	int numNode() { return (int)vNode.size(); }
	virtual void reset();
	vector<int> searchData(string sData);
	vector<int> searchData(string sData, int iCol);
	void setExpandGeneration(int numRow);
	void setNodeColour(int ID, pair<string, string> standard, pair<string, string> selected);
	void setNodeColour(vector<int> vID, pair<string, string> standard, pair<string, string> selected);
};
