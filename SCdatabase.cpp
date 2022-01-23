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
	
	int iYear, numFile;
	uintmax_t fileSize;
	string cataDir, filePath, sCata, zipPath;
	vector<string> vsFileName, vsFilePath;
	string sMarker(1, marker);

	sbgui.pullWork(cataDir);
	if (cataDir.size() < 1) { return; }
	size_t pos2 = cataDir.rfind(marker);
	if (pos2 > cataDir.size()) { err("Invalid sYear-insertCata"); }
	size_t pos1 = cataDir.rfind(marker, pos2 - 1);
	if (pos1 > cataDir.size()) { err("Invalid sYear-insertCata"); }
	string sYear = cataDir.substr(pos1 + 1, pos2 - pos1 - 1);

	unordered_map<string, string> mapTag;
	vector<string> vsTag{ "file_name", sYear };
	jf.getXML(mapTag, configXML, vsTag);
	vsTag = { "settings", "max_file_size" };
	string temp = jf.getXML1(configXML, vsTag);
	long long maxFileSize;
	try { 
		iYear = stoi(sYear);
		maxFileSize = stoll(temp); 
	}
	catch (invalid_argument) { err("iYear/maxFileSize stoi-insertCata"); }

	while (cataDir.size() > 0) {
		pos2 = cataDir.rfind(marker);
		if (pos2 > cataDir.size()) { err("Invalid sCata-insertCata"); }
		sCata = cataDir.substr(pos2 + 1);

		// Establish the state of the catalogue's local files. 
		// Required files still in the archive are unzipped.
		zipPath = cataDir + sMarker + sCata + ".zip";
		if (!jf.fileExist(zipPath)) { err("Missing catalogue zip file-insertCata"); }
		vsFileName.clear();
		jf.zipFileList(vsFileName, zipPath);
		numFile = (int)vsFileName.size();
		vsFilePath.resize(numFile);
		for (int ii = 0; ii < numFile; ii++) {
			vsFilePath[ii] = cataDir + sMarker + vsFileName[ii];
			filePath = vsFilePath[ii];
			pos1 = filePath.rfind('.');
			filePath.insert(pos1, 1, '*');  // Wildcard inserted in case the file has been split (renamed).
			if (!jf.fileExist(filePath)) { 
				jf.unzipFile(vsFileName[ii], zipPath, cataDir);
			}			
		}
		
		// Large files are split into pieces.
		for (int ii = 0; ii < numFile; ii++) {
			if (jf.fileExist(vsFilePath[ii])) {
				fileSize = jf.fileSize(vsFilePath[ii]);
				if (fileSize > maxFileSize) {
					jf.fileSplitter(vsFilePath[ii], maxFileSize);
				}
			}
		}

		//

	}




	sbgui.endCall(myid);
}
void SCdatabase::insertCensus(string sYear)
{
	vector<string> vsUnique;
	vector<vector<string>> vvsColTitle;
	vector<string> vsTag = { "table", "Census" };
	vector<vector<string>> vvsTag = jf.getXML(configXML, vsTag);
	xmlToColTitle(vvsColTitle, vsUnique, vvsTag);

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
	vector<string> vsUnique;
	vector<vector<string>> vvsColTitle;
	vector<string> vsTag = { "table", "Census_year" };
	vector<vector<string>> vvsTag = jf.getXML(configXML, vsTag);
	xmlToColTitle(vvsColTitle, vsUnique, vvsTag);

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
	vector<string> vsUnique;
	vector<vector<string>> vvsColTitle;
	vector<string> vsTag = { "table", "Topic_year" };
	vector<vector<string>> vvsTag = jf.getXML(configXML, vsTag);
	xmlToColTitle(vvsColTitle, vsUnique, vvsTag);

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
	string sCata = dirCata.substr(pos1);
	string filePath = dirCata + "/Topic_" + sCata + ".txt";
	sTopic.clear();
	if (jf.fileExist(filePath)) {
		jf.load(sTopic, filePath);
		pos1 = sTopic.find_last_not_of("\r\n") + 1;
		sTopic.resize(pos1);
	}
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
		jf.splitByMarker(vsParam, vsTable[ii], marker);
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
	// have now been added, return TRUE.
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
void SCdatabase::xmlToColTitle(std::vector<std::vector<std::string>>& vvsColTitle, std::vector<std::string>& vsUnique, std::vector<std::vector<std::string>>& vvsTag)
{
	// Convert extracted XML tags into the SQL-friendly form [col title, col type][value].
	// If the table has unique columns, those are placed into vsUnique.
	vvsColTitle.resize(2, vector<string>());
	vsUnique.clear();
	int numTag = (int)vvsTag.size();
	for (int ii = 0; ii < numTag; ii++) {
		if (vvsTag[ii][0].starts_with("title")) {
			vvsColTitle[0].emplace_back(vvsTag[ii][1]);
		}
		else if (vvsTag[ii][0].starts_with("type")) {
			vvsColTitle[1].emplace_back(vvsTag[ii][1]);
		}
		else if (vvsTag[ii][0].starts_with("unique")) {
			vsUnique.emplace_back(vvsTag[ii][1]);
		}
	}
}
