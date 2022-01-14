#include "SCdatabase.h"

void SCdatabase::err(string message)
{
	string errorMessage = "SCdatabase error:\n" + message;
	JLOG::getInstance()->err(errorMessage);
}
void SCdatabase::insertCata(SWITCHBOARD& sbgui)
{
	thread::id myid = this_thread::get_id();
	vector<int> mycomm;
	sbgui.answerCall(myid, mycomm);
	vector<string> vsPrompt; 
	sbgui.getPrompt(vsPrompt);  // Form (dirCata0, dirCata1, ...)
	int numCata = (int)vsPrompt.size();
	mycomm[2] = numCata;
	sbgui.update(myid, mycomm);

	// Perform fast operations prior to main data insertion.
	string stmt, temp, tname, sCata;
	vector<string> conditions, search, vsTag;
	vector<vector<string>> vvsGeo, vvsRow, vvsTag;
	if (mapGeoLayers.size() < 1) {
		vsTag = { "map", "geo_layers" };
		vvsTag = jf.getXML(configXML, vsTag);
		for (int ii = 0; ii < vvsTag.size(); ii++) {
			mapGeoLayers.emplace(vvsTag[ii][0], vvsTag[ii][1]);
		}
	}
	vsTag = { "table", "GeoLayers$[year]" };
	vector<vector<string>> vvsGeoLayers = jf.getXML(configXML, vsTag);
	jf.transpose(vvsGeoLayers);
	for (int ii = 0; ii < numCata; ii++) {
		// 1. Make the catalogue's entry into "GeoLayers".
		tname = "GeoLayers$" + sYear;
		sf.createTable(tname, vvsGeoLayers);
		loadGeo(vvsGeo, vvsGeoLayers, vsPrompt[ii], sYear, sCata);		
		search = { "Catalogue" };
		temp.clear();
		conditions = { "Catalogue LIKE '" + sCata + "'" };
		sf.select(search, tname, temp, conditions);
		if (temp.size() < 1) {
			if (!safeInsertRow(tname, vvsRow)) { err("Failed to expand table columns-insertCata"); }
			insertRow(tname, vvsRow);
		}

		// 2. Make and fill a Geo table for this catalogue.
		tname = "Geo$" + sYear + "$" + sCata;
		if (!sf.tableExist(tname)) {
			vsTag = { "table", "Geo$[year]$[cata]" };
			vvsTag = jf.getXML(configXML, vsTag);
			//createTable(tname, vvsTag);
		}



	}


	sbgui.endCall(myid);
}
void SCdatabase::insertRow(string tname, vector<vector<string>>& vvsRow)
{
	// vvsRow has form [col index][col title, cell value].
	// If more than one row of data is contained within vvsRow, then
	// perform the SQL insertions as a batch transaction.
	if (vvsRow.size() < 1) { err("vvsRow empty-insertRow"); }
	
	int numRow = (int)vvsRow[0].size();
	int numCol = (int)vvsRow.size();
	string stmt0 = "INSERT OR IGNORE INTO \"" + tname + "\" (";
	for (int ii = 0; ii < numCol; ii++) {
		if (ii > 0) { stmt0 += ", "; }
		stmt0 += vvsRow[ii][0];
	}
	stmt0 += ") VALUES (";
	vector<string> vStmt(numRow - 1);
	for (int ii = 1; ii < numRow; ii++) {
		vStmt[ii - 1] = stmt0;
		for (int jj = 0; jj < numCol; jj++) {
			if (jj > 0) { vStmt[ii - 1] += ", "; }
			vStmt[ii - 1] += vvsRow[jj][ii];
		}
		vStmt[ii - 1] += ");";
	}

	if (numRow > 1) { sf.insertPrepared(vStmt); }
	else { sf.executor(vStmt[0]); }	
}
void SCdatabase::loadGeo(vector<vector<string>>& vvsGeo, vector<vector<string>>& vvsGeoLayers, string dirCata, string& sYear, string& sCata)
{
	// vvsGeo form [region index][GEO_CODE, Region Name, GEO_LEVEL, Ancestor0, ...]
	// vvsGeoLayers form [col title, col value][col index].
	if (configXML.size() < 1) { err("Missing configXML-loadGeo"); }
	if (mapGeoLayers.size() < 1) { err("Missing mapGeoLayers-loadGeo"); }

	size_t pos1 = dirCata.find_last_of("/\\") + 1;
	sCata = dirCata.substr(pos1);
	size_t pos2 = pos1 - 1;
	pos1 = dirCata.find_last_of("/\\", pos2 - 1) + 1;
	sYear = dirCata.substr(pos1, pos2 - pos1);
	int count, numIndent, test;
	try { test = stoi(sYear); }
	catch (invalid_argument) { err("sYear stoi-loadGeo"); }

	string geoPath, temp;
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
	int numRegion = (int)vvsGeo.size();
	if (numRegion < 1) { err("Failed to parse geoFile-loadGeo"); }
	
	// Handle the special cases (Canada, or cross-border regions).
	unordered_map<string, int> mapSpacing;
	set<int> setSkip;
	bool Canada = 0;
	for (int ii = 0; ii < numRegion; ii++) {
		pos1 = vvsGeo[ii][1].find(" part");
		if (pos1 < vvsGeo[ii][1].size()) {
			pos2 = vvsGeo[ii][1].find("partie du");
			if (pos2 > vvsGeo[ii][1].size()) { err("Failed to determine cross-border region"); }
			if (pos1 < pos2) {
				vvsGeo[ii][1].resize(pos1 + 6);
				vvsGeo[ii][1].back() = ')';
			}
			else {
				pos2 = vvsGeo[ii][1].rfind('(', pos2);
				pos1 = vvsGeo[ii][1].find('/', pos2) + 1;
				pos1 = vvsGeo[ii][1].find_first_not_of(' ', pos1);
				vvsGeo[ii][1].replace(pos2, pos1 - pos2, "(");
			}

			temp = vvsGeo[ii - 1][1];
			pos1 = vvsGeo[ii][1].find(temp);
			if (pos1 < vvsGeo[ii][1].size()) {
				vvsGeo.erase(vvsGeo.begin() + ii);
				ii--;
			}
			setSkip.emplace(ii);
			continue;
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
	unordered_map<int, string> mapGeoLevel;
	for (int ii = 0; ii < numIndent; ii++) {
		mapGeoLevel.emplace(ii, vsLayers[ii]);
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
	for (int ii = 1; ii < numIndent + 1; ii++) {

	}


	// Refine GeoLayer name into GEO_LEVEL, and add ancestries.
	for (int ii = 0; ii < numRegion; ii++) {
		if (setSkip.count(ii)) { continue; }
		
	}
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
void SCdatabase::log(string message)
{
	string logMessage = "SCdatabase log entry:\n" + message;
	JLOG::getInstance()->log(logMessage);
}
vector<vector<string>> SCdatabase::makeGeoLayers(string sCata, vector<vector<string>>& vvsGeo)
{
	// vvsGeo has form [region index][GEO_CODE, Region Name, GeoLayer Name].
	// Return form [col index][col title, cell value].
	vector<string> vsTag;
	vector<vector<string>> vvsTag;


	vector<vector<string>> vvsRow(1, vector<string>(2));
	vvsRow[0] = { "Catalogue", sCata };
	unordered_map<string, int> mapLayerSpacing;
	vector<int> viIndent;

	int indent, numIndent, spacing;
	int indexCanada = -1;  // Canada must be separated from the provinces.
	int numRegion = (int)vvsGeo.size();
	for (int ii = 0; ii < numRegion; ii++) {
		if (vvsGeo[ii][1] == "Canada") { indexCanada = ii; }
		spacing = (int)vvsGeo[ii][1].find_first_not_of(' ');
		auto it = mapLayerSpacing.find(vvsGeo[ii][2]);
		if (it == mapLayerSpacing.end()) {
			mapLayerSpacing.emplace(vvsGeo[ii][2], spacing);
			numIndent = (int)viIndent.size();
			if (numIndent < 1) { viIndent.push_back(spacing); }
			else {
				for (int jj = 0; jj < numIndent; jj++) {
					if (spacing < viIndent[jj]) {
						viIndent.insert(viIndent.begin() + jj, spacing);
					}
					else if (spacing == viIndent[jj]) { err("Different GeoLayers sharing indentation-makeGeoLayers"); }
					else if (jj == numIndent - 1) {
						viIndent.push_back(spacing);
					}
				}
			}
		}
		else if (spacing != mapLayerSpacing.at(vvsGeo[ii][2])) {
			err("Inconsistent GeoLayer spacing-makeGeoLayers");
		}
	}

	string sLayer;
	if (indexCanada >= 0) { 
		vvsRow.resize(2 + viIndent.size()); 
		vvsRow[1] = { "Level0", "" };
		for (auto it = mapLayerSpacing.begin(); it != mapLayerSpacing.end(); ++it) {
			sLayer = mapGeoLayers.at(it->first);
			spacing = it->second;
			for (int ii = 0; ii < viIndent.size(); ii++) {
				if (viIndent[ii] == spacing) {
					vvsRow[2 + ii] = { "Level" + to_string(1 + ii), sLayer };
					break;
				}
			}
		}
	}
	else { 
		vvsRow.resize(1 + viIndent.size()); 
		for (auto it = mapLayerSpacing.begin(); it != mapLayerSpacing.end(); ++it) {
			sLayer = mapGeoLayers.at(it->first);
			spacing = it->second;
			for (int ii = 0; ii < viIndent.size(); ii++) {
				if (viIndent[ii] == spacing) {
					vvsRow[1 + ii] = { "Level" + to_string(ii), sLayer };
					break;
				}
			}
		}
	}

	return vvsRow;
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
		jnYear.vsData.push_back(yearList[ii]);
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
			jn.vsData.push_back(cataList[jj]);
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
	// have been added, return TRUE. Return FALSE otherwise.
	vector<vector<string>> vvsColTitle = sf.getColTitle(tname);
	unordered_map<string, string> mapColTitle;
	for (int ii = 0; ii < vvsColTitle.size(); ii++) {
		mapColTitle.emplace(vvsColTitle[ii][0], vvsColTitle[ii][1]);
	}

	size_t pos1;
	int iLevel;
	string title, type;
	for (int ii = 0; ii < vvsRow.size(); ii++) {
		auto it = mapColTitle.find(vvsRow[ii][0]);
		if (it == mapColTitle.end()) {  // Column title is missing from table.
			pos1 = vvsRow[ii][0].find_last_of("0123456789");
			if (pos1 != vvsRow[ii][0].size() - 1) { 
				log("Final column is not numerical-safeInsertRow"); 
				return 0;
			}
			pos1 = vvsRow[ii][0].find_last_not_of("0123456789") + 1;
			try { iLevel = stoi(vvsRow[ii][0].substr(pos1)); }
			catch (invalid_argument) { 
				log("iLevel stoi-safeInsertRow"); 
				return 0;
			}
			title = vvsRow[ii][0].substr(0, pos1) + "0";
			if (!mapColTitle.count(title)) { 
				log("Failed to locate progenitor column in existing table-safeInsertRow"); 
				return 0;
			}
			type = mapColTitle.at(title);
			title.pop_back();
			title += to_string(iLevel);
			sf.addColumn(tname, title, type);
			mapColTitle.emplace(title, type);
		}
	}
	return 1;
}
