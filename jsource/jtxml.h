#pragma once
#include <deque>
#include "jfile.h"
#include "jtree.h"

class JTXML : public JTREE
{
	std::string attr, encoding, tag, wild;
	JFILE jfile;
	JPARSE jparse;
	std::unordered_map<std::string, char> mapEntity;
	std::set<int> setDisable;
	double version;
	std::string xmlFile;

	void err(std::string message);
	void extractDeclaration(std::string& xmlFile);
	void extractNameAttribute(std::string element, JNODE& jxe);
	void filterDisabled(std::vector<int>& vID);
	void initEntity();
	size_t populateTree(std::string& xmlFile, JNODE& jxeParent);
	void query(std::vector<int>& vID, std::string sQuery, int mode, int startID = -1);
	void removeComment(std::string& xmlText);
	void replaceEntity(std::string& sData);

public:
	JTXML(const JTXML& jtxOld) : JTREE(jtxOld) { initEntity(); }
	JTXML() { reset(); }

	enum branch{ Disable, Enable };
	enum filter{ Off, On };

	void branchIgnore(std::string sQuery, int mode);
	JNODE& getNode(int ID);
	JNODE& getRoot();
	void getValue(std::string& sValue, std::string sQuery);
	void getValue(std::vector<std::string>& vsValue, std::string sQuery);
	void initMarker(std::string tag, std::string attr, std::string wild);
	void loadXML(std::string filePath);
	std::string nodeValue(JNODE& jn);
	void populateSubtree(JTREE*& jtsub, int parentID, std::deque<std::string> dsQuery);
	void reset() override;
};