#include "SCdatabase.h"

using namespace std;

void SCdatabase::deleteTable(string tname)
{
	if (sf.tableExist(tname)) { 
		sf.dropTable(tname); 
	}
}
void SCdatabase::err(string message)
{
	string errorMessage = "SCdatabase error:\n" + message;
	JLOG::getInstance()->err(errorMessage);
}
vector<string> SCdatabase::extractUnique(vector<vector<string>>& vvsTag)
{
	// Niche function to extract unique specifiers for SQL table columns.
	vector<string> vsUnique;
	size_t pos1;
	for (int ii = 0; ii < vvsTag.size(); ii++) {
		pos1 = vvsTag[ii][0].find("unique");
		if (pos1 < vvsTag[ii][0].size()) {
			if (!sf.columnType.count(vvsTag[ii][1])) {
				vsUnique.push_back(vvsTag[ii][1]);
				vvsTag.erase(vvsTag.begin() + ii);
				ii--;
			}
		}
	}
	return vsUnique;
}
void SCdatabase::init(string& xml)
{
	if (xml.size() < 1) { err("Missing configXML-init"); }
	configXML = xml;

	mapGeoLayers.clear();
	char marker;
	size_t pos2, pos1 = 1;
	vector<string> vsTag = { "map", "geo_layers" };
	vector<vector<string>> vvsTag = jf.getXML(configXML, vsTag);
	for (int ii = 0; ii < vvsTag.size(); ii++) {
		marker = vvsTag[ii][1][0];
		pos2 = vvsTag[ii][1].find(marker, pos1);
		if (pos2 > vvsTag[ii][1].size()) { err("Failed to locate splitter marker-initMap"); }
		mapGeoLayers.emplace(vvsTag[ii][1].substr(pos1, pos2 - pos1), vvsTag[ii][1].substr(pos2 + 1));
	}

	vsTag = { "path", "database" };
	string dbPath = jf.getXML1(configXML, vsTag);
	sf.init(dbPath);
}
void SCdatabase::insertCata(SWITCHBOARD& sbgui)
{
	thread::id myid = this_thread::get_id();
	vector<int> mycomm;
	sbgui.answerCall(myid, mycomm);
	vector<string> vsPrompt; 
	sbgui.getPrompt(vsPrompt);  // Form (dirCata0, dirCata1, ...)
	int numCata = (int)vsPrompt.size();
	if (numCata < 1) { sbgui.endCall(myid); }
	else {
		size_t pos2 = vsPrompt[0].find_last_of("/\\");
		size_t pos1 = vsPrompt[0].find_last_of("/\\", pos2 - 1) + 1;
		sYear = vsPrompt[0].substr(pos1, pos2 - pos1);
		int test;
		try { test = stoi(sYear); }
		catch (invalid_argument) { err("sYear stoi-insertCata"); }
		mycomm[2] = numCata;
		sbgui.update(myid, mycomm);
	}

	// Perform fast operations prior to main data insertion.
	string metaFile, stmt, sCata, sTopic, temp;
	vector<string> conditions, search, vsTag;
	vector<vector<string>> vvsRow, vvsTag;
	insertCensus(sYear);

	vsTag = { "table", "GeoLayers$[year]" };
	vector<vector<string>> vvsGeoLayers = jf.getXML(configXML, vsTag);
	vector<string> vsUnique = extractUnique(vvsGeoLayers);
	jf.transpose(vvsGeoLayers);
	string tname = "GeoLayers$" + sYear;
	if (!sf.tableExist(tname)) {
		sf.createTable(tname, vvsGeoLayers, vsUnique);
	}
	
	vsTag = { "table", "Geo$[year]$[cata]" };
	vector<vector<string>> vvsColTitle = jf.getXML(configXML, vsTag);
	vsUnique = extractUnique(vvsColTitle);
	jf.transpose(vvsColTitle);
	vector<vector<string>> vvsGeo = vvsColTitle;

	for (int ii = 0; ii < numCata; ii++) {
		// 1. Insert the catalogue's entry into "GeoLayers".		
		loadGeo(vvsGeo, vvsGeoLayers, vsPrompt[ii], sCata);
		if (safeInsertRow(tname, vvsGeoLayers)) {
			sf.insertRow(tname, vvsGeoLayers);
		}

		// 2. Make and fill a Geo table for this catalogue.
		tname = "Geo$" + sYear + "$" + sCata;
		if (!sf.tableExist(tname)) {
			sf.createTable(tname, vvsColTitle, vsUnique);
		}
		if (safeInsertRow(tname, vvsGeo)) {
			sf.insertRow(tname, vvsGeo);
		}

		// 3. Add this catalogue to the CensusYear table.
		loadMeta(metaFile, vsPrompt[ii]);
		loadTopic(sTopic, vsPrompt[ii]);
		insertCensusYear(metaFile, sYear, sCata, sTopic);

		// 4. Add this catalogue's topic to the TopicYear table.
		insertTopicYear(sYear, sTopic);

		// 5. Make and fill this catalogue's DIM/dim tables.


	}


	sbgui.endCall(myid);
}
void SCdatabase::insertCensus(string sYear)
{
	vector<string> vsTag = { "table", "Census" };
	vector<vector<string>> vvsColTitle = jf.getXML(configXML, vsTag);
	vector<string> vsUnique = extractUnique(vvsColTitle);
	jf.transpose(vvsColTitle);
	string tname = "Census";
	if (!sf.tableExist(tname)) {
		sf.createTable(tname, vvsColTitle, vsUnique);
	}
	vvsColTitle[1][0] = sYear;
	if (safeInsertRow(tname, vvsColTitle)) {
		sf.insertRow(tname, vvsColTitle);
	}
}
void SCdatabase::insertCensusYear(string& metaFile, string sYear, string sCata, string sTopic)
{
	vector<string> vsTag = { "table", "Census$[year]" };
	vector<vector<string>> vvsColTitle = jf.getXML(configXML, vsTag);
	vector<string> vsUnique = extractUnique(vvsColTitle);
	jf.transpose(vvsColTitle);
	string tname = "Census$" + sYear;
	if (!sf.tableExist(tname)) {
		sf.createTable(tname, vvsColTitle, vsUnique);
	}

	if (sTopic.size() < 1) {
		size_t pos1 = metaFile.find("Topic:");
		if (pos1 > metaFile.size()) { err("Failed to extract topic from metaFile-insertCensusYear"); }		
		pos1 = metaFile.find_first_not_of(' ', pos1 + 6);
		size_t pos2 = metaFile.find_first_of("\r\n", pos1);
		sTopic = metaFile.substr(pos1, pos2 - pos1);
	}
	vvsColTitle[1][0] = sCata;
	vvsColTitle[1][1] = sTopic;
	
	if (safeInsertRow(tname, vvsColTitle)) {
		sf.insertRow(tname, vvsColTitle);
	}
}
void SCdatabase::insertTopicYear(string sYear, string sTopic)
{
	if (sYear.size() < 1 || sTopic.size() < 1) { err("Missing input-insertTopicYear"); }
	vector<string> vsTag = { "table", "Topic$[year]" };
	vector<vector<string>> vvsColTitle = jf.getXML(configXML, vsTag);
	vector<string> vsUnique = extractUnique(vvsColTitle);
	jf.transpose(vvsColTitle);
	string tname = "Topic$" + sYear;
	if (!sf.tableExist(tname)) {
		sf.createTable(tname, vvsColTitle, vsUnique);
	}

	int index = sf.getNumRows(tname);
	vvsColTitle[1][0] = to_string(index);
	vvsColTitle[1][1] = sTopic;
	if (safeInsertRow(tname, vvsColTitle)) {
		sf.insertRow(tname, vvsColTitle);
	}
}
void SCdatabase::loadGeo(vector<vector<string>>& vvsGeo, vector<vector<string>>& vvsGeoLayers, string dirCata, string& sCata)
{
	// vvsGeo form [region index][GEO_CODE, Region Name, GEO_LEVEL, Ancestor0, ...]
	// vvsGeoLayers form [col title, col value][col index].
	if (configXML.size() < 1) { err("Missing configXML-loadGeo"); }
	if (mapGeoLayers.size() < 1) { err("Missing mapGeoLayers-loadGeo"); }
	vector<string> vsGeoTitle;
	if (vvsGeo.size() < 1) { err("Missing vvsGeo-loadGeo"); }
	else { vsGeoTitle = vvsGeo[0]; }

	size_t pos2, pos1 = dirCata.find_last_of("/\\") + 1;
	sCata = dirCata.substr(pos1);
	int count, index, numIndent;
	string geoPath, sRegion, temp;
	vector<string> vsTag = { "file_name", sYear };
	vector<vector<string>> vvsTag = jf.getXML(configXML, vsTag);
	int numTag = (int)vvsTag.size();
	if (numTag < 1) { err("Failed to extract vvsTag-loadGeo"); }
	for (int jj = 0; jj < numTag; jj++) {
		pos1 = vvsTag[jj][0].find("geo");
		if (pos1 < vvsTag[jj][0].size()) {
			geoPath = dirCata + "/" + vvsTag[jj][1];
			pos2 = geoPath.rfind("[cata]");
			if (pos2 < geoPath.size()) {
				geoPath.replace(pos2, 6, sCata);
			}
			break;
		}
	}
	if (geoPath.size() < 1) { err("Failed to determine geoPath-insertCata"); }

	string geoFile = jf.load(geoPath);
	vsTag = { "parse", sYear, "geo" };
	vvsTag = jf.getXML(configXML, vsTag);
	vvsGeo = jf.parseFromXML(geoFile, vvsTag);
	if (vvsGeo.size() < 1) { err("Failed to parse geoFile-loadGeo"); }
	
	// Handle the special cases (Canada, or cross-border regions).
	unordered_map<string, int> mapSpacing;
	set<int> setSkip;
	bool Canada = 0;
	for (int ii = 0; ii < vvsGeo.size(); ii++) {
		pos2 = vvsGeo[ii][1].find('/');  // Indicates English/French name listings, or cross-border region.
		if (pos2 < vvsGeo[ii][1].size()) {
			pos1 = vvsGeo[ii][1].find(" part");
			if (pos1 < vvsGeo[ii][1].size()) {  // Cross-border region.
				pos2 = vvsGeo[ii][1].find("partie du");
				if (pos2 > vvsGeo[ii][1].size()) {
					pos2 = vvsGeo[ii][1].find("partie de");
					if (pos2 > vvsGeo[ii][1].size()) {
						err("Failed to determine cross-border region");
					}
				}
				if (pos1 < pos2) {  // English is listed first.
					vvsGeo[ii][1].resize(pos1 + 6);
					vvsGeo[ii][1].back() = ')';
				}
				else {  // Le français est listé premier.
					pos2 = vvsGeo[ii][1].rfind('(', pos2);
					pos1 = vvsGeo[ii][1].find('/', pos2) + 1;
					pos1 = vvsGeo[ii][1].find_first_not_of(' ', pos1);
					vvsGeo[ii][1].replace(pos2, pos1 - pos2, "(");
					pos1 = vvsGeo[ii][1].find('(', pos2 + 1);
					if (pos1 < vvsGeo[ii][1].size()) {
						pos2 = vvsGeo[ii][1].rfind(')', pos1) + 1;
						vvsGeo[ii][1].resize(pos2);
					}
				}

				temp = vvsGeo[ii - 1][1];
				pos1 = vvsGeo[ii][1].find(temp);
				if (pos1 < vvsGeo[ii][1].size()) {
					vvsGeo.erase(vvsGeo.begin() + ii - 1);
					ii--;
				}
				setSkip.emplace(ii);
				continue;
			}
			else {  // English/French name listing.
				pos1 = vvsGeo[ii][1].find_last_not_of(" /", pos2) + 1;
				sRegion = vvsGeo[ii][1].substr(0, pos1);
				temp = sRegion;
				jf.asciiOnly(temp);
				if (temp == sRegion) {  // No accented chars.
					vvsGeo[ii][1] = sRegion;
				}
				else {
					pos1 = vvsGeo[ii][1].find_first_not_of(" /", pos2);
					sRegion = vvsGeo[ii][1].substr(pos1);
					temp = sRegion;
					jf.asciiOnly(temp);
					if (temp != sRegion) { err("Failed to determine English name-loadGeo"); }
					vvsGeo[ii][1] = sRegion;
				}
			}
		}
		if (vvsGeo[ii][1] == "Canada") { 
			vvsGeo[ii][0] = "1";
			vvsGeo[ii][2] = "0"; 
			vvsGeo[ii].resize(3);
			setSkip.emplace(ii);
			Canada = 1;
			continue;
		}
		
		auto it = mapSpacing.find(vvsGeo[ii][2]);
		if (it == mapSpacing.end()) {
			count = 0;
			while (vvsGeo[ii][1][count] == ' ') { count++; }
			mapSpacing.emplace(vvsGeo[ii][2], count);
		}
	}

	// Define the GEO_LEVELs.
	vector<int> viSpacing;
	vector<string> vsLayers;
	if (Canada) {
		viSpacing.push_back(-1);
		vsLayers.push_back("Canada");
	}
	for (auto it = mapSpacing.begin(); it != mapSpacing.end(); ++it) {
		numIndent = (int)viSpacing.size();
		if (numIndent < 1) {
			viSpacing.push_back(it->second);
			vsLayers.push_back(it->first);
		}
		else {
			for (int ii = 0; ii < numIndent; ii++) {
				if (it->second < viSpacing[ii]) {
					viSpacing.insert(viSpacing.begin() + ii, it->second);
					vsLayers.insert(vsLayers.begin() + ii, it->first);
				}
				else if (it->second == viSpacing[ii]) { err("Different GeoLayers have identical spacing-loadGeo"); }
				else if (ii == numIndent - 1) {
					viSpacing.push_back(it->second);
					vsLayers.push_back(it->first);
				}
			}
		}
	}
	numIndent = (int)viSpacing.size();
	unordered_map<int, string> mapLevelGeo;
	unordered_map<string, int> mapGeoLevel;
	for (int ii = 0; ii < numIndent; ii++) {
		mapLevelGeo.emplace(ii, vsLayers[ii]);
		mapGeoLevel.emplace(vsLayers[ii], ii);
	}

	// Return the GeoLayers.
	int indexStart = (int)vvsGeoLayers[0].size();
	pos1 = vvsGeoLayers[0][indexStart - 1].find_last_not_of("0123456789") + 1;
	int iLevel;
	try { iLevel = stoi(vvsGeoLayers[0][indexStart - 1].substr(pos1)); }
	catch (invalid_argument) { err("iLevel stoi-loadGeo"); }
	string root = vvsGeoLayers[0][indexStart - 1].substr(0, pos1);
	vvsGeoLayers[0].resize(numIndent + 1);
	vvsGeoLayers[1].resize(numIndent + 1);
	for (int ii = indexStart; ii < numIndent + 1; ii++) {
		iLevel++;
		vvsGeoLayers[0][ii] = root + to_string(iLevel);
	}
	vvsGeoLayers[1][0] = sCata;
	for (int ii = 0; ii < numIndent; ii++) {
		temp = mapLevelGeo.at(ii);
		vvsGeoLayers[1][1 + ii] = mapGeoLayers.at(temp);
	}
	for (int ii = 0; ii < vvsGeoLayers[0].size(); ii++) {
		if (vvsGeoLayers[1][ii].size() < 1) {
			vvsGeoLayers[0].erase(vvsGeoLayers[0].begin() + ii);
			vvsGeoLayers[1].erase(vvsGeoLayers[1].begin() + ii);
			ii--;
		}
	}

	// Refine GeoLayer name into GEO_LEVEL, and add ancestries.
	vector<int> viIndent;
	count = 0;
	if (Canada) { viIndent.push_back(0); }
	for (int ii = 0; ii < vvsGeo.size(); ii++) {
		if (setSkip.count(ii)) { continue; }
		
		iLevel = mapGeoLevel.at(vvsGeo[ii][2]);
		if (viIndent.size() == 0) {
			if (iLevel != 0) { err("First entry is not a parent-loadGeo"); }
			viIndent.push_back(ii);
		}
		else if (iLevel < viIndent.size()) {
			viIndent[iLevel] = ii;
			viIndent.resize(iLevel + 1);
		}
		else {
			while (iLevel >= viIndent.size()) {
				viIndent.push_back(ii);
			}
		}
		vvsGeo[ii][2] = to_string(iLevel);

		vvsGeo[ii].resize(2 + viIndent.size());
		for (int jj = 3; jj < vvsGeo[ii].size(); jj++) {
			vvsGeo[ii][jj] = vvsGeo[viIndent[jj - 3]][0];
		}
		if (vvsGeo[ii].size() > count) { count = (int)vvsGeo[ii].size(); }
	}

	// Remove region name indentation spacings.
	for (int ii = 0; ii < vvsGeo.size(); ii++) {
		if (setSkip.count(ii)) { continue; }
		while (vvsGeo[ii][1][0] == ' ') {
			vvsGeo[ii][1].erase(vvsGeo[ii][1].begin());
		}
	}

	// Refine the special cases (cross-border regions).
	vector<int> viPreParent;
	for (auto it = setSkip.begin(); it != setSkip.end(); ++it) {
		while (vvsGeo[*it][1][0] == ' ') {
			vvsGeo[*it][1].erase(vvsGeo[*it][1].begin());
		}

		sRegion = vvsGeo[*it][1];
		if (sRegion == "Canada") { continue; }
		pos2 = sRegion.rfind(" part");
		pos1 = sRegion.rfind('(', pos2) + 1;
		temp = sRegion.substr(pos1, pos2 - pos1);
		for (int ii = 0; ii < vvsGeo.size(); ii++) {
			if (vvsGeo[ii][1] == temp) {
				iLevel = stoi(vvsGeo[ii][2]);
				iLevel++;
				vvsGeo[*it][2] = to_string(iLevel);
				
				for (int jj = 3; jj < vvsGeo[ii].size(); jj++) {
					vvsGeo[*it].push_back(vvsGeo[ii][jj]);
				}
				vvsGeo[*it].push_back(vvsGeo[ii][0]);
				if (vvsGeo[*it].size() > count) { count = (int)vvsGeo[*it].size(); }

				if (ii > *it) { viPreParent.push_back(*it); }
				break;
			}
		}
	}
	for (int ii = (int)viPreParent.size() - 1; ii >= 0; ii--) {
		vvsGeo.push_back(vvsGeo[viPreParent[ii]]);
		vvsGeo.erase(vvsGeo.begin() + viPreParent[ii]);
	}

	// Prepend vvsGeo's column titles.
	index = (int)vsGeoTitle.size() - 1;
	if (count > index + 1) {
		pos1 = vsGeoTitle[index].find_last_of("0123456789");
		if (pos1 != vsGeoTitle[index].size() - 1) { err("Failed to extrapolate additional column titles for vvsGeo-loadGeo"); }
		pos1 = vsGeoTitle[index].find_last_not_of("0123456789") + 1;
		temp = vsGeoTitle[index].substr(0, pos1);
		iLevel = stoi(vsGeoTitle[index].substr(pos1));
		vsGeoTitle.resize(count);
		for (int ii = index + 1; ii < count; ii++) {
			iLevel++;
			vsGeoTitle[ii] = temp + to_string(iLevel);
		}
	}
	vvsGeo.insert(vvsGeo.begin(), vsGeoTitle);
}
void SCdatabase::loadMeta(string& metaFile, string dirCata)
{
	// Load the catalogue's meta file into memory.
	if (sYear.size() != 4) { err("Missing sYear-loadMeta"); }
	size_t pos1 = dirCata.find_last_of("/\\") + 1;
	string sCata = dirCata.substr(pos1);

	vector<string> vsTag = { "file_name", sYear };
	vector<vector<string>> vvsTag = jf.getXML(configXML, vsTag);
	int numTag = (int)vvsTag.size();
	string metaPath;
	for (int ii = 0; ii < numTag; ii++) {
		pos1 = vvsTag[ii][0].find("meta");
		if (pos1 < vvsTag[ii][0].size()) {
			metaPath = dirCata + "/" + vvsTag[ii][1];
			break;
		}
	}
	if (metaPath.size() < 1) { err("Failed to read metaPath-loadMeta"); }
	pos1 = metaPath.rfind("[cata]");
	if (pos1 < metaPath.size()) {
		metaPath.replace(pos1, 6, sCata);
	}
	metaFile = jf.load(metaPath);
}
void SCdatabase::loadTable(vector<vector<string>>& vvsData, vector<vector<string>>& vvsColTitle, string tname)
{
	if (sf.tableExist(tname)) {
		vvsData.clear();
		vector<string> search = { "*" };
		sf.select(search, tname, vvsData);
		vvsColTitle = sf.getColTitle(tname);
	}
}
void SCdatabase::loadTopic(string& sTopic, string dirCata)
{
	size_t pos1 = dirCata.find_last_of("/\\") + 1;
	string temp = dirCata.substr(pos1);
	string filePath = dirCata + "/Topic_" + temp + ".txt";
	if (jf.fileExist(filePath)) {
		temp = jf.load(filePath);
		pos1 = temp.find_last_not_of("\r\n") + 1;
		sTopic = temp.substr(0, pos1);
	}
	else { sTopic = ""; }
}
void SCdatabase::log(string message)
{
	string logMessage = "SCdatabase log entry:\n" + message;
	JLOG::getInstance()->log(logMessage);
}
void SCdatabase::makeTreeCata(SWITCHBOARD& sbgui, JTREE& jt)
{
	// Obtain a tree structure representing the database's catalogues
	// organized by census year. 

	thread::id myid = this_thread::get_id();
	vector<int> mycomm;
	sbgui.answerCall(myid, mycomm);
	
	jt.reset();
	JNODE jnRoot = jt.getRoot();
	int rootID = jnRoot.ID;
	vector<string> search = { "Year" }, yearList, cataList;
	string tname = "Census";
	string orderby = "Year ASC";
	sf.selectOrderBy(search, tname, yearList, orderby);
	int numCata, numYear = (int)yearList.size();
	for (int ii = 0; ii < numYear; ii++) {
		JNODE jnYear;
		jnYear.vsData[0] = yearList[ii];
		jt.addChild(rootID, jnYear);
	}

	vector<int> viYearID(numYear);
	vector<reference_wrapper<JNODE>> vJNYear = jt.getChildren(rootID);
	for (int ii = 0; ii < numYear; ii++) {
		viYearID[ii] = vJNYear[ii].get().ID;
	}

	search = { "Catalogue" };
	for (int ii = 0; ii < numYear; ii++) {
		cataList.clear();
		tname = "Census$" + yearList[ii];
		numCata = sf.select(search, tname, cataList);
		for (int jj = 0; jj < numCata; jj++) {
			JNODE jn;
			jn.vsData[0] = cataList[jj];
			jt.addChild(viYearID[ii], jn);
		}
	}

	sbgui.endCall(myid);
}
void SCdatabase::searchTable(SWITCHBOARD& sbgui, JTREE& jt, vector<string>& vsTable)
{
	thread::id myid = this_thread::get_id();
	vector<int> mycomm;
	sbgui.answerCall(myid, mycomm);
	string sQuery;
	sbgui.getPrompt(sQuery);

	int numTable = sf.searchTableName(vsTable, sQuery);

	bool done;
	int nodeID, numChildren, numParam;
	vector<string> vsParam;
	vector<int> childrenID;
	JNODE jnRoot = jt.getRoot();
	for (int ii = 0; ii < numTable; ii++) {
		vsParam = jf.splitByMarker(vsTable[ii], marker);
		numParam = (int)vsParam.size();
		if (numParam < 1) { err("splitByMarker-searchTable"); }

		nodeID = jnRoot.ID;
		for (int jj = 0; jj < numParam; jj++) {
			done = 0;
			childrenID = jt.getChildrenID(nodeID);
			numChildren = (int)childrenID.size();
			for (int kk = 0; kk < numChildren; kk++) {
				JNODE jn = jt.getNode(childrenID[kk]);
				if (jn.vsData[0] == vsParam[jj]) {
					nodeID = jn.ID;
					done = 1;
					break;
				}
			}
			if (!done) {
				JNODE jn(1);
				jn.vsData[0] = vsParam[jj];
				jt.addChild(nodeID, jn);
				nodeID = jn.ID;
			}
		}
	}

	sbgui.endCall(myid);
}
bool SCdatabase::safeInsertRow(string tname, vector<vector<string>>& vvsRow)
{
	// If the named table already has the requisite columns, or if those columns
	// have been added, return TRUE.
	// vvsRow has form [column titles, row0, row1, ...][values]. 
	if (vvsRow.size() < 1 || vvsRow[0].size() < 1) { err("Missing vvsRow-safeInsertRow"); }
	vector<vector<string>> vvsColTitle = sf.getColTitle(tname);
	unordered_map<string, string> mapTitleType;
	for (int ii = 0; ii < vvsColTitle[0].size(); ii++) {
		mapTitleType.emplace(vvsColTitle[0][ii], vvsColTitle[1][ii]);
	}

	size_t pos1;
	int iLevel;
	string title, type;
	for (int ii = 0; ii < vvsRow[0].size(); ii++) {
		auto it = mapTitleType.find(vvsRow[0][ii]);
		if (it == mapTitleType.end()) {  // Column title is missing from table.
			pos1 = vvsRow[0][ii].find_last_of("0123456789");
			if (pos1 != vvsRow[0][ii].size() - 1) { err("Final column is not numerical-safeInsertRow"); }
			pos1 = vvsRow[0][ii].find_last_not_of("0123456789") + 1;
			try { iLevel = stoi(vvsRow[0][ii].substr(pos1)); }
			catch (invalid_argument) { err("iLevel stoi-safeInsertRow"); }
			title = vvsRow[0][ii].substr(0, pos1) + "0";
			if (!mapTitleType.count(title)) { err("Failed to locate progenitor column in existing table-safeInsertRow"); }
			type = mapTitleType.at(title);
			title.pop_back();
			title += to_string(iLevel);
			sf.addColumn(tname, title, type);
			mapTitleType.emplace(title, type);
		}
	}
	return 1;
}
