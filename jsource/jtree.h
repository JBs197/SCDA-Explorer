#pragma once
#include <map>
#include <mutex>
#include <set>
#include "jstring.h"

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
	~JNODE() = default;

	bool empty, expanded;
	int hasTwin;
	const int ID;
	static int nextID;
	std::pair<std::string, std::string> colour;  // Form [BG, FG] #AARRGGBB#RRGGBB or #RRGGBB
	std::pair<std::string, std::string> colourSelected;  // Form [BG, FG] #AARRGGBB#RRGGBB or #RRGGBB
	std::map<std::string, std::string> mapAttribute;
	size_t posStart, posStop;
	std::string prefix;
	std::vector<std::string> vsData, vsItemRole;

	static int getNextID() { return nextID++; }
	
	JNODE operator=(JNODE jn) {
		JNODE jnCopy;
		jnCopy.vsData = jn.vsData;
		jnCopy.vsItemRole = jn.vsItemRole;
		if (jn.prefix.size() > 0) { jnCopy.prefix = jn.prefix; }
		jnCopy.colour = jn.colour;
		jnCopy.colourSelected = jn.colourSelected;
		jnCopy.mapAttribute = jn.mapAttribute;
		return jnCopy;
	}
};

class JTREE
{
	friend class JTXML;

private:
	std::mutex m_tree;
	std::unordered_map<int, int> mapIDIndex;  // Unique ID -> index
	std::set<int> setBlankIndex;
	std::vector<std::vector<int>> treeSTanc;       // Form [index][anc1, anc2, ...]
	std::vector<std::vector<int>> treeSTdes;       // Form [index][des1, des2, ...]
	std::vector<JNODE> vNode;

	void err(std::string message);
	void getChildrenAllWorker(int ID, std::vector<int>& viChildren);
	void log(std::string message);

public:
	JTREE(const JTREE& jtOld) : mapIDIndex(jtOld.mapIDIndex), 
		setBlankIndex(jtOld.setBlankIndex), treeSTanc(jtOld.treeSTanc), 
		treeSTdes(jtOld.treeSTdes), vNode(jtOld.vNode) {
		selectedID = -1;
	}
	JTREE() { reset(); }
	~JTREE() = default;

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
	std::vector<std::reference_wrapper<JNODE>> getChildren(int ID);
	std::vector<std::reference_wrapper<JNODE>> getChildrenAll(int ID);
	std::vector<int> getChildrenID(int parentID);
	std::vector<std::string> getData(int ID);
	std::vector<std::string> getDataUserRole(int ID);
	int getExpandGeneration(int numRow);
	std::vector<std::string> getGenealogy(int ID, int iCol = 0);
	std::vector<int> getGenealogyID(int nodeID);
	int getID(int index);
	JNODE& getNode(int ID);
	JNODE& getParent(int ID);
	int getPivot(std::vector<int> treeSTrow);
	JNODE& getRoot();
	int hasTwin(int ID);
	std::vector<int> hasTwinList(int twinStatus);
	bool isTopLevel(int ID);
	int maxLength(int iCol);
	int numNode() { return (int)vNode.size(); }
	virtual void reset();
	std::vector<int> searchData(std::string sData);
	std::vector<int> searchData(std::string sData, int iCol);
	void setExpandGeneration(int numRow);
	void setNodeColour(int ID, std::pair<std::string, std::string> standard, std::pair<std::string, std::string> selected);
	void setNodeColour(std::vector<int> vID, std::pair<std::string, std::string> standard, std::pair<std::string, std::string> selected);
};
