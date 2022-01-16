#pragma once
#include "jfunc.h"
#include "jtree.h"

using namespace std;

class JTXML : public JTREE
{
	string encoding;
	JFUNC jf;
	unordered_map<string, char> mapEntity;
	double version;
	string xmlFile;

	void addChild(int parentID, JNODE& jnChild);
	void err(string message);
	void extractDeclaration(string& xmlFile);
	void extractNameAttribute(string element, JNODE& jxe);
	void initEntity();
	size_t populateTree(string& xmlFile, JNODE& jxeParent);
	void removeComment(string& xmlText);
	void replaceEntity(string& sData);

public:
	JTXML(const JTXML& jtxOld) : JTREE(jtxOld) { initEntity(); }
	JTXML() { reset(); }
	~JTXML() {}

	JNODE& getNode(int ID);
	JNODE& getRoot();
	void loadXML(string filePath);
	void reset() override;
};