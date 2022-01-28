#pragma once
#include <deque>
#include "jbuffer.h"
#include "jfile.h"
#include "jtree.h"

#define NUM_BUFFER_SLOT 6
#define MIN_FILE_SIZE 1000000
#define MAX_FILE_SIZE 1000000000
// 1GB max is default and arbitrary. maxBufferSize should be specified for operations.

class JTXML : public JTREE
{
	std::string attr, encoding, tag, wild;
	JFILE jfile;
	JPARSE jparse;
	std::unordered_map<std::string, char> mapEntity;
	uintmax_t maxBufferSize = 0;
	std::set<int> setDisable;
	double version;
	std::string xmlFile;

	void err(std::string message);
	void extractDeclaration(std::string& xmlFile);
	void extractNameAttribute(std::string element, JNODE& jxe);
	void filterDisabled(std::vector<int>& vID);
	void initEntity();
	size_t populateTree(std::string& xmlFile, JNODE& jxeParent);
	size_t populateTree(std::string& xmlFragment, JNODE& jxeParent, JBUFFER<std::string, NUM_BUFFER_SLOT>& jbuf);
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
	void initValue(std::string tag, std::string attr, std::string wild, uintmax_t maxSize = 0);
	void loadXML(std::string filePath, std::vector<std::vector<std::string>> vvsTag = { {} });
	std::string nodeValue(JNODE& jn);
	void populateSubtree(JTREE*& jtsub, std::pair<int, int> parentID, std::deque<std::string> dsQuery);
	void query(std::vector<int>& vID, std::string sQuery, int mode, int startID = -1);
	void reset() override;
};