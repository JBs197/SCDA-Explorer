#include "jtxml.h"

using namespace std;

/*
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
*/

void JTXML::branchIgnore(string sQuery, int mode)
{
	// All nodes satisfying sQuery are added to or removed from to the set of 
	// disabled IDs (which are ignored when searching).
	vector<int> vID;
	query(vID, sQuery, filter::Off);
	switch (mode) {
	case branch::Disable:
		for (int ID : vID) {
			setDisable.emplace(ID);
		}
		break;
	case branch::Enable:
		for (int ID : vID) {
			setDisable.erase(ID);
		}
		break;
	}
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
void JTXML::filterDisabled(std::vector<int>& vID)
{
	// Nodes listed in setDisable are removed from the list of IDs.
	for (int ii = 0; ii < vID.size(); ii++) {
		if (setDisable.count(vID[ii])) {
			vID.erase(vID.begin() + ii);
			ii--;
		}
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
void JTXML::getValue(string& sValue, string sQuery)
{
	// This variant returns the first value found within the XML tree matching sQuery.
	// Note: disabled nodes are ignored.
	vector<int> vID, viTemp;
	query(vID, sQuery, filter::On);
	if (vID.size() < 1) { return; }
	
	JNODE& jn = getNode(vID[0]);
	sValue = nodeValue(jn);
}
void JTXML::getValue(vector<string>& vsValue, string sQuery)
{
	// Returns all values found within the XML tree matching sQuery.
	// Note: disabled nodes are ignored.
	vector<int> vID, viTemp;
	query(vID, sQuery, filter::On);

	for (int ii = 0; ii < vID.size(); ii++) {
		JNODE& jn = getNode(vID[ii]);
		vsValue.emplace_back(nodeValue(jn));
	}
}
void JTXML::initEntity()
{
	mapEntity.emplace("&lt;", '<');
	mapEntity.emplace("&gt;", '>');
	mapEntity.emplace("&amp;", '&');
	mapEntity.emplace("&apos;", '\'');
	mapEntity.emplace("&quot;", '"');
}
void JTXML::initValue(string tagChar, string attrChar, string wildChar, uintmax_t maxSize)
{
	// Each marker indicates the beginning of the variable within a string.
	tag = tagChar;
	attr = attrChar;
	wild = wildChar;
	maxBufferSize = maxSize;
}
void JTXML::loadXML(string filePath, vector<vector<string>> vvsTag)
{
	// If vvsTag is specified, then only those elements (name, value) will be loaded into
	// this tree. Otherwise, all elements are loaded. 
	// If a file exceeds MINFILESIZE, then a worker thread is used to pre-load the next
	// chunk of data while the main thread parses it. 
	// If maxBufferSize has not been specified, then MAXFILESIZE is used instead.

	uintmax_t fileSize = jfile.fileSize(filePath);
	if (fileSize < MIN_FILE_SIZE) { 
		jfile.load(xmlFile, filePath);
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
	else {
		JBUFFER<string, NUM_BUFFER_SLOT> jbuf;
		if (maxBufferSize > 0) { jbuf.slotSize = min(fileSize, maxBufferSize) / (NUM_BUFFER_SLOT * 262144); }
		else { jbuf.slotSize = min(fileSize, (uintmax_t)MAX_FILE_SIZE) / (NUM_BUFFER_SLOT * 262144); }  // 2^18
		
		std::jthread xmlLoad([&jbuf, filePath](std::stop_token stopToken) {
			string sData; 
			sData.reserve(jbuf.slotSize);
			ifstream input(filePath, ifstream::binary);
			while (input.is_open()) {
				if (stopToken.stop_requested()) { input.close(); }
				input.read(&sData[0], jbuf.slotSize);
				jbuf.push(sData);
				if (input.gcount() < jbuf.slotSize) { input.close(); }
			}
		});

		string sFile = jbuf.pull();
		size_t posStart = sFile.find('<');
		if (posStart > sFile.size()) { err("File has no element tags-loadXML"); }
		while (sFile[posStart + 1] == '?' || sFile[posStart + 1] == '!') {
			posStart = sFile.find('<', posStart + 1);
		}
		size_t posStop = sFile.find('>', posStart);
		if (posStop > sFile.size()) { err("Asymmetric root-loadXML"); }
		JNODE jnXMLRoot;
		jnXMLRoot.posStart = posStop + 1;  // Start of tag's interior.
		string sElement = sFile.substr(posStart + 1, posStop - posStart - 1);
		extractNameAttribute(sElement, jnXMLRoot);
		addChild(vNode[0].ID, jnXMLRoot);
		populateTree(sFile, jnXMLRoot);

	}

}
string JTXML::nodeValue(JNODE& jn)
{
	// When evaluating an XML node, returns a value using the priority hierarchy
	// (value-only single child) > (single-attribute value) > (element name)
	vector<int> childrenID = getChildrenID(jn.ID);
	if (childrenID.size() == 1) {
		JNODE& jnChild = getNode(childrenID[0]);
		if (jnChild.posStart == 0) {  // Value only, not an element.
			return jnChild.vsData[0];
		}
	}
	if (jn.mapAttribute.size() == 1) {
		auto it = jn.mapAttribute.begin();
		return it->second;
	}
	return jn.vsData[0];
}
void JTXML::populateSubtree(JTREE*& jtsub, pair<int, int> parentID, deque<string> dsQuery)
{
	// Every query within dsQuery serves as a branch splitter, with each branch 
	// gaining its own matching values as well as gaining subbranches, and so forth. 
	// With each recursion, the top layer of dsQuery is used to populate the next
	// generation of child nodes. The first position within parentID refers to the 
	// main tree within this object, while the second position refers to the subtree.
	// Note: disabled nodes are ignored.
	pair<int, int> childID;
	vector<int> vID;
	query(vID, dsQuery[0], filter::On, get<0>(parentID));
	dsQuery.pop_front();
	size_t querySize = dsQuery.size();
	for (int ii = 0; ii < vID.size(); ii++) {
		JNODE& myChild = getNode(vID[ii]);
		JNODE subChild = myChild;
		jtsub->addChild(get<1>(parentID), subChild);
		if (querySize > 0) {
			childID = make_pair(myChild.ID, subChild.ID);
			populateSubtree(jtsub, childID, dsQuery);
		}
	}
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
size_t JTXML::populateTree(string& xmlFragment, JNODE& jnParent, JBUFFER<string, NUM_BUFFER_SLOT>& jbuf)
{
	// Recursive function, returns the position in the xmlFile from which the parent should
	// continue parsing. This variant is used when xmlFile is only a fragment of the file.
	string element, name;
	vector<int> childrenID;
	size_t length = xmlFragment.size();
	size_t pos2, pos3, pos1 = xmlFragment.find('<', jnParent.posStart);
	if (pos1 > length) {
		// HANDLE SPLIT STRING CASE
	}
	while (xmlFragment[pos1 + 1] == '!' || xmlFragment[pos1 + 1] == '?') {
		pos1 = xmlFragment.find('<', pos1 + 2);
	}
	while (pos1 < xmlFragment.size()) {
		if (xmlFragment[pos1 + 1] == '/') {  // Closing tag.
			pos2 = xmlFragment.find('>', pos1 + 2);
			name = xmlFragment.substr(pos1 + 2, pos2 - pos1 - 2);
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
					jnChild.vsData[0] = xmlFragment.substr(jnParent.posStart, pos1 - jnParent.posStart);
					replaceEntity(jnChild.vsData[0]);
					addChild(jnParent.ID, jnChild);
				}
			}
			return pos2;
		}
		else {  // New tag.
			JNODE jnChild;
			pos2 = xmlFragment.find('>', pos1 + 1);
			if (pos2 > xmlFragment.size()) { err("Asymmetric tag-populateTree"); }
			jnChild.posStart = pos2 + 1;
			if (xmlFragment[pos2 - 1] == '/') {  // Self-closing tag.
				jnChild.posStop = jnChild.posStart;
				pos2 = xmlFragment.find_last_not_of("/ ", pos2 - 1) + 1;
				element = xmlFragment.substr(pos1 + 1, pos2 - pos1 - 1);
				extractNameAttribute(element, jnChild);
				addChild(jnParent.ID, jnChild);
				return jnChild.posStop;
			}
			element = xmlFragment.substr(pos1 + 1, pos2 - pos1 - 1);
			extractNameAttribute(element, jnChild);
			addChild(jnParent.ID, jnChild);

			pos2 = populateTree(xmlFragment, jnChild);
		}
		pos1 = xmlFragment.find('<', pos2);
	}
}
void JTXML::query(vector<int>& vID, string sQuery, int mode, int startID)
{
	// Determine which nodes satisfy sQuery, and append their IDs to the list. 
	// "mode" determines whether the "disable" filter is applied or not. 
	// "startID" (default is root) specifies which node from which to begin searching - 
	// note that sQuery begins where startID's own vsBranch trail ends !
	if (tag.size() < 1 || attr.size() < 1 || wild.size() < 1) { err("Missing initMarker-query"); }
	size_t pos1, pos2, pos3, length;
	bool match;
	string temp;
	vector<int> viTemp;
	vector<pair<string, string>> vAttr;
	vector<string> vsBranch;
	jparse.splitByMarker(vsBranch, sQuery, tag[0]);
	int numBranch = (int)vsBranch.size();
	vector<vector<int>> vvID(numBranch + 1, vector<int>());  // Form [branch index][candidateID]
	if (startID < 0) { vvID[0] = { vNode[0].ID }; }
	else { vvID[0] = { startID }; }

	for (int ii = 0; ii < numBranch; ii++) {
		length = vsBranch[ii].size();
		if (length == 0) {
			for (int ID : vvID[ii]) {
				appendChildrenID(vvID[1 + ii], ID);
			}
			continue;
		}

		// Extract attribute pairs (name, value), leaving the branch as only a tag name.
		vAttr.clear();
		pos1 = vsBranch[ii].rfind(attr[0]);
		while (pos1 < length) {
			pos2 = vsBranch[ii].find("=\"", pos1);
			if (pos2 < length) {
				pos3 = vsBranch[ii].find('"', pos2 + 2);
				if (pos3 < length) {
					vAttr.emplace_back(make_pair(vsBranch[ii].substr(pos1 + 1, pos2 - pos1 - 1), vsBranch[ii].substr(pos2 + 2, pos3 - pos2 - 2)));
				}
			}
			vsBranch[ii].resize(pos1);
			pos1 = vsBranch[ii].rfind(attr[0]);
		}

		// Add the next row of candidate IDs.
		for (int jj = 0; jj < vvID[ii].size(); jj++) {
			viTemp = getChildrenID(vvID[ii][jj]);
			if (mode) { filterDisabled(viTemp); }
			for (int kk = 0; kk < viTemp.size(); kk++) {
				JNODE& jn = getNode(viTemp[kk]);
				if (jn.vsData[0] == vsBranch[ii] || vsBranch[ii] == wild) {
					if (vAttr.size() < 1) { vvID[1 + ii].push_back(jn.ID); }
					for (int ll = 0; ll < vAttr.size(); ll++) {
						// Try to disqualify this candidate node.
						if (vAttr[ll].first == wild) {
							match = 0;
							for (auto it = jn.mapAttribute.begin(); it != jn.mapAttribute.end(); ++it) {
								if (it->second == vAttr[ll].second) {
									match = 1;
									break;
								}
							}
							if (!match) { break; }
						}
						else {
							auto it = jn.mapAttribute.find(vAttr[ll].first);
							if (it == jn.mapAttribute.end()) { break; }
							if (vAttr[ll].second != wild) {
								if (vAttr[ll].second != it->second) { break; }
							}
						}
						if (ll == vAttr.size() - 1) {  // Candidate passes all tests.
							vvID[1 + ii].push_back(jn.ID);
						}
					}
				}
			}
		}
	}
	for (int ii = 0; ii < vvID.back().size(); ii++) {
		vID.emplace_back(vvID.back()[ii]);
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
