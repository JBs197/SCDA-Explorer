#include "jtxml.h"

void JTXML::addChild(int parentID, JNODE& jnChild)
{
	lock_guard<mutex> lg(m_tree);
	auto itParent = mapIDIndex.find(parentID);
	if (itParent == mapIDIndex.end()) {
		err("Failed to locate parentID " + to_string(parentID) + "-addChild");
	}

	vector<int> viAncestry;
	if (itParent->second == 0) { viAncestry = { 0 }; }
	else {
		viAncestry = treeSTanc[itParent->second];
		viAncestry.push_back(itParent->second);
	}

	jnChild.colour = vNode[viAncestry.back()].colour;
	jnChild.colourSelected = vNode[viAncestry.back()].colourSelected;

	int index;
	if (setBlankIndex.size() < 1) {
		index = (int)vNode.size();
		vNode.push_back(move(jnChild));
		treeSTanc.push_back(viAncestry);
		treeSTdes.push_back(vector<int>());
	}
	else {
		auto itChild = setBlankIndex.rbegin();
		index = *itChild;
		vNode[index] = move(jnChild);
		setBlankIndex.erase(index);
		treeSTanc[index] = viAncestry;
		treeSTdes[index] = vector<int>();
	}
	treeSTdes[viAncestry.back()].push_back(index);
	mapIDIndex.emplace(vNode[index].ID, index);
}
void JTXML::err(string message)
{
	string errorMessage = "JTXML error:\n" + message;
	JLOG::getInstance()->err(errorMessage);
}
void JTXML::extractDeclaration(string& xmlFile)
{
	// Only pulls attributes from the first declaration found.
	size_t posStart = xmlFile.find("<?");
	if (posStart > xmlFile.size()) { return; }
	size_t posStop = xmlFile.find("?>", posStart + 2);
	if (posStop > xmlFile.size()) { err("Asymmetric declaration brackets-extractDeclaration"); }
	string declaration = xmlFile.substr(posStart + 2, posStop - posStart - 2);
	
	size_t pos2, pos1 = declaration.find("version=\"");
	if (pos1 < declaration.size()) {
		pos1 += 9;
		pos2 = declaration.find('"', pos1);
		if (pos2 > declaration.size()) { err("Asymmetric attribute-extractDeclaration"); }
		try { version = stod(declaration.substr(pos1, pos2 - pos1)); }
		catch (invalid_argument) { err("version stod-extractDeclaration"); }
	}
	pos1 = declaration.find("encoding=\"");
	if (pos1 < declaration.size()) {
		pos1 += 10;
		pos2 = declaration.find('"', pos1);
		if (pos2 > declaration.size()) { err("Asymmetric attribute-extractDeclaration"); }
		encoding = declaration.substr(pos1, pos2 - pos1);
	}

	xmlFile.erase(posStart, posStop - posStart + 2);
}
void JTXML::extractNameAttribute(string element, JNODE& jn)
{
	string attribute0, attribute1;
	size_t pos1, pos2, pos3, pos4;
	size_t posEqual = element.rfind('=');
	while (posEqual < element.size()) {
		pos3 = element.find_first_of("'\"", posEqual + 1);
		pos4 = element.find(element[pos3], pos3 + 1);  // Accomodate single or double quotes.
		attribute1 = element.substr(pos3 + 1, pos4 - pos3 - 1);
		replaceEntity(attribute1);

		pos2 = element.find_last_not_of(' ', posEqual - 1);
		pos2++;
		pos1 = element.find_last_of(' ', pos2) + 1;
		attribute0 = element.substr(pos1, pos2 - pos1);
		replaceEntity(attribute0);

		jn.mapAttribute.emplace(attribute0, attribute1);
		
		element.resize(pos1 - 1);
		posEqual = element.rfind('=');
	}

	pos1 = element.find(':');
	if (pos1 < element.size()) {
		jn.vsData[0] = element.substr(pos1 + 1);
		replaceEntity(jn.vsData[0]);
		jn.prefix = element.substr(0, pos1);
		replaceEntity(jn.prefix);
	}
	else {
		replaceEntity(element);
		jn.vsData[0] = element;
	}	
}
JNODE& JTXML::getNode(int ID)
{
	lock_guard<mutex> lg(m_tree);
	auto it = mapIDIndex.find(ID);
	if (it == mapIDIndex.end()) {
		err("Failed to locate ID " + to_string(ID) + "-getNode");
	}
	return vNode[it->second];
}
JNODE& JTXML::getRoot()
{
	return vNode[0];
}
void JTXML::initEntity()
{
	mapEntity.emplace("&lt;", '<');
	mapEntity.emplace("&gt;", '>');
	mapEntity.emplace("&amp;", '&');
	mapEntity.emplace("&apos;", '\'');
	mapEntity.emplace("&quot;", '"');
}
void JTXML::loadXML(string filePath)
{
	xmlFile = jf.load(filePath);
	extractDeclaration(xmlFile);
	removeComment(xmlFile);
	size_t posStart = xmlFile.find('<');
	if (posStart > xmlFile.size()) { err("File has no element tags-loadXML"); }
	posStart++;
	size_t posStop = xmlFile.find('>', posStart);
	if (posStop > xmlFile.size()) { err("Asymmetric root-loadXML"); }
	JNODE jnXMLRoot;
	jnXMLRoot.posStart = posStop + 1;  // Start of tag's interior.
	string sElement = xmlFile.substr(posStart, posStop - posStart);
	extractNameAttribute(sElement, jnXMLRoot);
	addChild(vNode[0].ID, jnXMLRoot);

	populateTree(xmlFile, jnXMLRoot);
}
size_t JTXML::populateTree(string& xmlFile, JNODE& jnParent)
{
	// Recursive function, returns the position in the xmlFile from
	// which the parent should continue parsing.
	string element, name;
	vector<int> childrenID;
	size_t pos2, pos3, pos1 = xmlFile.find('<', jnParent.posStart);
	while (xmlFile[pos1 + 1] == '!' || xmlFile[pos1 + 1] == '?') {
		pos1 = xmlFile.find('<', pos1 + 2);
	}	
	while (pos1 < xmlFile.size() - 1) {
		if (xmlFile[pos1 + 1] == '/') {  // Closing tag.
			pos2 = xmlFile.find('>', pos1 + 2);
			name = xmlFile.substr(pos1 + 2, pos2 - pos1 - 2);
			pos3 = name.rfind(':');
			if (pos3 < name.size()) { name = name.substr(pos3 + 1); }
			replaceEntity(name);
			if (name != jnParent.vsData[0]) {
				err("Tag opens [" + jnParent.vsData[0] + "] but closes [" + name + "]-populateTree");
			}

			jnParent.posStop = pos1;
			childrenID = getChildrenID(jnParent.ID);
			if (childrenID.size() < 1) {
				if (pos1 > jnParent.posStart) {  // Value only, not a tag.
					JNODE jnChild;
					jnChild.posStart = 0;  // Indicates the element has no children.
					jnChild.posStop = pos1;
					jnChild.vsData[0] = xmlFile.substr(jnParent.posStart, pos1 - jnParent.posStart);
					replaceEntity(jnChild.vsData[0]);
					addChild(jnParent.ID, jnChild);
				}				
			}
			return pos2;
		}
		else {  // New tag.
			JNODE jnChild;
			pos2 = xmlFile.find('>', pos1 + 1);
			if (pos2 > xmlFile.size()) { err("Asymmetric tag-populateTree"); }
			jnChild.posStart = pos2 + 1;			
			if (xmlFile[pos2 - 1] == '/') {  // Self-closing tag.
				jnChild.posStop = jnChild.posStart;
				pos2 = xmlFile.find_last_not_of("/ ", pos2 - 1) + 1;
				element = xmlFile.substr(pos1 + 1, pos2 - pos1 - 1);
				extractNameAttribute(element, jnChild);
				addChild(jnParent.ID, jnChild);
				return jnChild.posStop;
			}
			element = xmlFile.substr(pos1 + 1, pos2 - pos1 - 1);
			extractNameAttribute(element, jnChild);
			addChild(jnParent.ID, jnChild);

			pos2 = populateTree(xmlFile, jnChild);
		}
		pos1 = xmlFile.find('<', pos2);
	}
}
void JTXML::removeComment(string& xmlText)
{
	size_t pos2, pos1 = xmlText.find("<!--");
	while (pos1 < xmlText.size()) {
		pos2 = xmlText.find("-->", pos1 + 4);
		if (pos2 > xmlText.size()) { err("Asymmetric comment brackets-removeComment"); }
		pos2 += 3;
		xmlText.erase(pos1, pos2 - pos1);
		pos1 = xmlText.find("<!--", pos1);
	}
}
void JTXML::replaceEntity(string& sData)
{
	string sEntity;
	unordered_map<string, char>::iterator it;
	size_t pos2, pos1 = sData.find('&');
	while (pos1 < sData.size()) {
		pos2 = sData.find(';', pos1 + 1);
		if (pos2 > sData.size()) { return; }
		pos2++;
		sEntity = sData.substr(pos1, pos2 - pos1);
		it = mapEntity.find(sEntity);
		if (it != mapEntity.end()) { 
			sData.replace(pos1, pos2 - pos1, 1, it->second);
		}
		pos1 = sData.find('&', pos1 + 1);
	}
}
void JTXML::reset()
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

	initEntity();
}
