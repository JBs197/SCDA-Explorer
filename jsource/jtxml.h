#pragma once
#include "jfile.h"
#include "jtree.h"

class JTXML : public JTREE
{
	std::string encoding;
	JFILE jfile;
	std::unordered_map<std::string, char> mapEntity;
	double version;
	std::string xmlFile;

	void addChild(int parentID, JNODE& jnChild);
	void err(std::string message);
	void extractDeclaration(std::string& xmlFile);
	void extractNameAttribute(std::string element, JNODE& jxe);
	void initEntity();
	size_t populateTree(std::string& xmlFile, JNODE& jxeParent);
	void removeComment(std::string& xmlText);
	void replaceEntity(std::string& sData);

public:
	JTXML(const JTXML& jtxOld) : JTREE(jtxOld) { initEntity(); }
	JTXML() { reset(); }

	JNODE& getNode(int ID);
	JNODE& getRoot();
	void loadXML(std::string filePath);
	void reset() override;
};