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
bool SCdatabase::hasGeoGap(vector<string>& vsGeoLayer, vector<vector<string>>& vvsGeoLevel, vector<int>& viGeoLevel)
{
	// Returns TRUE or FALSE as to whether this catalogue requires preliminary geo entries from 
	// a template, before inserting the catalogue's own geo region entries. If a template layer
	// is needed, it will be inserted into vsGeoLayer with a preceeding '!' to mark it.
	bool hasGap = 0;
	size_t pos1, pos2;
	string sLayer;
	for (int ii = 0; ii < vvsGeoLevel.size(); ii++) {
		if (vvsGeoLevel[ii][0].starts_with("necessary")) {
			if (vsGeoLayer[viGeoLevel[ii]] != vvsGeoLevel[ii][1]) {
				hasGap = 1;
				vsGeoLayer.insert(vsGeoLayer.begin() + viGeoLevel[ii], "!");
				vsGeoLayer[viGeoLevel[ii]].append(vvsGeoLevel[ii][1]);
			}
		}
		else { break; }
	}
	return hasGap;
}
void SCdatabase::init(string& xml)
{
	if (xml.size() < 1) { err("Missing configXML-init"); }
	configXML = xml;

	mapGeoLayers.clear();
	char marker;
	size_t pos2, pos1 = 1;
	vector<string> vsTag = { "map", "geo_layers" };
	vector<vector<string>> vvsTag = jparse.getXML(configXML, vsTag);
	for (int ii = 0; ii < vvsTag.size(); ii++) {
		marker = vvsTag[ii][1][0];
		pos2 = vvsTag[ii][1].find(marker, pos1);
		if (pos2 > vvsTag[ii][1].size()) { err("Failed to locate splitter marker-initMap"); }
		mapGeoLayers.emplace(vvsTag[ii][1].substr(pos1, pos2 - pos1), vvsTag[ii][1].substr(pos2 + 1));
	}

	vsTag = { "path", "database" };
	string dbPath = jparse.getXML1(configXML, vsTag);
	sf.init(dbPath);
}
void SCdatabase::insertCata(SWITCHBOARD& sbgui)
{
	thread::id myid = this_thread::get_id();
	vector<int> mycomm;
	sbgui.answerCall(myid, mycomm);
	
	// Determine the census year for the catalogue(s).
	int iYear, numFile;
	string cataDir, filePath, sCata, sMetaFile, sTopic, zipPath;
	vector<string> vsFileName, vsFilePath, vsGeoLayer;
	sbgui.pullWork(cataDir);
	if (cataDir.size() < 1) { return; }
	size_t pos2 = cataDir.find_last_of("/\\");
	if (pos2 > cataDir.size()) { err("Invalid sYear-insertCata"); }
	size_t pos1 = cataDir.find_last_of("/\\", pos2 - 1);
	if (pos1 > cataDir.size()) { err("Invalid sYear-insertCata"); }
	string sYear = cataDir.substr(pos1 + 1, pos2 - pos1 - 1);

	// Prepare a few shared values between catalogue(s) for this census year.
	unordered_map<string, string> mapTag;
	vector<string> vsTag{ "file_name", sYear };
	jparse.getXML(mapTag, configXML, vsTag);
	vsTag = { "settings", "max_file_size" };
	string temp = jparse.getXML1(configXML, vsTag);
	long long maxFileSize;
	try { 
		iYear = stoi(sYear);
		maxFileSize = stoll(temp); 
	}
	catch (invalid_argument) { err("iYear/maxFileSize stoi-insertCata"); }

	// Load the GeoLevel list, to determine if a subsequent catalogue needs assistance
	// from a region tree template.
	vsTag = { "map", "geo_level" };
	vector<vector<string>> vvsGeoLevel = jparse.getXML(configXML, vsTag);
	vector<int> viGeoLevel(vvsGeoLevel.size());
	for (int ii = 0; ii < vvsGeoLevel.size(); ii++) {
		pos1 = vvsGeoLevel[ii][1].rfind(marker);
		if (pos1 > vvsGeoLevel[ii][1].size()) { err("Invalid geo_level entry-insertCata"); }
		try { viGeoLevel[ii] = stoi(vvsGeoLevel[ii][1].substr(pos1 + 1)); }
		catch (invalid_argument) { err("geo_level stoi-insertCata"); }
		
		pos1 = vvsGeoLevel[ii][1].find_first_not_of(marker);
		pos2 = vvsGeoLevel[ii][1].find(marker, pos1);
		vvsGeoLevel[ii][1].resize(pos2);
		vvsGeoLevel[ii][1].erase(vvsGeoLevel[ii][1].begin(), vvsGeoLevel[ii][1].begin() + pos1);
	}

	// So long as the queue contains undone catalogues, read local files and insert the
	// data into the various database tables.
	JTXML* jtxml = nullptr;
	unordered_map<string, string> mapMeta;
	while (cataDir.size() > 0) {
		pos2 = cataDir.find_last_of("/\\");
		if (pos2 > cataDir.size()) { err("Invalid sCata-insertCata"); }
		sCata = cataDir.substr(pos2 + 1);

		// The catalogue's local files are unzipped and broken into manageable pieces. 
		prepareLocal(cataDir, sCata, maxFileSize);

		// Build and insert catalogue topic-related tables.
		loadTopic(sTopic, cataDir);
		insertTopicYear(sYear, sTopic);
		insertCensusYear(sYear, sCata, sTopic);

		// Build and insert all the catalogue's geographic region tables.
		vsGeoLayer = insertGeoLayer(cataDir, sYear, sCata);
		if (hasGeoGap(vsGeoLayer, vvsGeoLevel, viGeoLevel)) {
			insertGeo(vsGeoLayer, cataDir, sYear, sCata);  // Catalogue has GeoLayer gaps.
		}
		else { insertGeo(cataDir, sYear, sCata); }
		
		// Build and insert all the catalogue's parameter/dimension tables.
		mapMeta.clear();
		loadMeta(jtxml, mapMeta, cataDir, sYear, sCata);
		insertForWhom(jtxml, mapMeta, sYear, sCata);

	}

	sbgui.endCall(myid);
}
void SCdatabase::insertCensus(string sYear)
{
	vector<string> vsUnique;
	vector<vector<string>> vvsColTitle;
	vector<string> vsTag = { "table", "Census" };
	vector<vector<string>> vvsTag = jparse.getXML(configXML, vsTag);
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
void SCdatabase::insertCensusYear(string sYear, string sCata, string sTopic)
{
	if (sYear.size() < 1 || sCata.size() < 1 || sTopic.size() < 1) { err("Missing input-insertCensusYear"); }
	vector<string> vsUnique;
	vector<vector<string>> vvsColTitle;
	vector<string> vsTag = { "table", "Census_year" };
	vector<vector<string>> vvsTag = jparse.getXML(configXML, vsTag);
	xmlToColTitle(vvsColTitle, vsUnique, vvsTag);

	string tname = "Census" + marker + sYear;
	if (!sf.tableExist(tname)) {
		sf.createTable(tname, vvsColTitle, vsUnique);
	}

	vvsColTitle[1][0] = sCata;
	vvsColTitle[1][1] = sTopic;
	
	if (safeInsertRow(tname, vvsColTitle)) {
		sf.insertRow(tname, vvsColTitle);
	}
}
void SCdatabase::insertForWhom(JTXML*& jtxml, unordered_map<string, string>& mapMeta, string sYear, string sCata)
{
	// "ForWhom" tables are rarely-used differentiators, called when multiple catalogues
	// measure the same variables, over the same regions, during the same census year.
	if (jtxml == nullptr) { err("Missing jtxml-insertForWhom"); }
	else if (jtxml->numNode() < 2) { err("Empty jtxml-insertForWhom"); }
	
	vector<string> vsUnique;
	vector<vector<string>> vvsRow;
	vector<string> vsTag = { "table", "ForWhom_year" };
	vector<vector<string>> vvsTag = jparse.getXML(configXML, vsTag);
	xmlToColTitle(vvsRow, vsUnique, vvsTag);
	string tname = "ForWhom" + marker + sYear;
	if (!sf.tableExist(tname)) { sf.createTable(tname, vvsRow, vsUnique); }
	vvsRow[1][0] = sCata;

	string sDescription;
	auto it = mapMeta.find("for_whom");
	if (it == mapMeta.end()) { err("sQuery not found in mapMeta-insertForWhom"); }
	jtxml->getValue(sDescription, it->second);
	size_t pos1 = sDescription.rfind(" for ");
	if (pos1 > sDescription.size()) { err("Failed to determine ForWhom within description-insertForWhom"); }
	size_t pos2 = sDescription.find(" of ", pos1);
	if (pos2 < sDescription.size()) {
		vvsRow[1][1] = "F" + sDescription.substr(pos1 + 2, pos2 - pos1 - 2);
	}
	else { vvsRow[1][1] = "F" + sDescription.substr(pos1 + 2); }
	
	sf.insertRow(tname, vvsRow);
}
void SCdatabase::insertGeo(string cataDir, string sYear, string sCata)
{
	// Insert this catalogue's geographic region tree, as a SQL table. If the catalogue
	// has missing regions which are also parents to other (present) regions, then the
	// appropriate GeoTreeTemplate table will be used to fill in the gaps. The missing
	// region will not have any data attached to it, but it can prevent the tree from
	// having missing links. 
	int geoCode, geoLevel, index, inum;
	string geoFile, geoLayer, myGeoCode, parentGeoCode, tnameTemplate;
	vector<string> conditions, search, vsResult, vsTag, vsUnique;
	vector<vector<string>> vvsColTitle, vvsRow, vvsTag;

	string tname = "Geo" + marker + sYear + marker + sCata;
	if (!sf.tableExist(tname)) {
		vsTag = { "table", "Geo_year_cata" };
		vvsTag = jparse.getXML(configXML, vsTag);
		xmlToColTitle(vvsColTitle, vsUnique, vvsTag);
		sf.createTable(tname, vvsColTitle, vsUnique);
	}

	string filePath = cataDir + "/Geo_" + sCata + ".txt";
	jfile.load(geoFile, filePath);
	string sMarker(1, geoFile[0]);
	size_t pos1 = 1, pos2;
	size_t pos3 = geoFile.find('\n');
	while (pos1 < geoFile.size()) {
		index = (int)vvsRow.size();
		vvsRow.push_back(vector<string>());

		pos2 = geoFile.find(marker, pos1);
		while (pos2 < pos3) {
			vvsRow[index].emplace_back(geoFile.substr(pos1, pos2 - pos1));
			pos1 = pos2 + 1;
			pos2 = geoFile.find(sMarker, pos1);
		}
		vvsRow[index].emplace_back(geoFile.substr(pos1, pos3 - pos1));

		pos1 = geoFile.find_first_not_of(sMarker + "\n", pos3 + 1);
		pos3 = geoFile.find('\n', pos3 + 1);
	}

	// Identify the indices of the GEO_CODE, Region Name, and GEO_LEVEL columns.
	int indexCode = -1, indexLevel = -1, indexRegion = -1, myIndex;
	for (int jj = 0; jj < vvsRow[0].size(); jj++) {
		pos1 = vvsRow[0][jj].find("GEO_CODE");
		if (pos1 < vvsRow[0][jj].size()) { indexCode = jj; }

		pos1 = vvsRow[0][jj].find("GEO_LEVEL");
		if (pos1 < vvsRow[0][jj].size()) { indexLevel = jj; }

		pos1 = vvsRow[0][jj].find("Region");
		if (pos1 < vvsRow[0][jj].size()) { indexRegion = jj; }
	}
	if (indexCode < 0 || indexLevel < 0 || indexRegion < 0) { err("Failed to determine a geo column index-insertGeoTreeTemplate"); }

	// Ensure that "Canada" has GEO_LEVEL 0, and that split regions are given their
	// proper GEO_LEVEL (relative to individual parent).
	for (int jj = 0; jj < vvsRow.size(); jj++) {
		if (vvsRow[jj][indexRegion] == "Canada") {
			vvsRow[jj][indexLevel] = "0";
			continue;
		}

		// Je m'excuse ici - c'est pour la simplicité...
		pos1 = vvsRow[jj][indexRegion].find(" part");
		if (pos1 < vvsRow[jj][indexRegion].size()) {
			pos2 = vvsRow[jj][indexRegion].find('/');
			if (pos1 < pos2) {  // English listed first.
				vvsRow[jj][indexRegion].resize(pos1 + 6);
				vvsRow[jj][indexRegion].back() = ')';
			}
			else {  // French listed first.
				pos1 = vvsRow[jj][indexRegion].rfind(' ', pos1 - 1) + 1;
				pos2 = vvsRow[jj][indexRegion].rfind('(', pos2);
				vvsRow[jj][indexRegion].replace(pos2, pos1 - pos2, "(");
			}
			try { geoLevel = stoi(vvsRow[jj][indexLevel]); }
			catch (invalid_argument) { err("geoLevel stoi-insertGeoTreeTemplate"); }
			geoLevel--;
			vvsRow[jj][indexLevel] = to_string(geoLevel);
		}
		pos1 = vvsRow[jj][indexRegion].find(" / ");
		if (pos1 < vvsRow[jj][indexRegion].size()) {
			vvsRow[jj][indexRegion].resize(pos1);
		}
	}

	// Add ancestry columns to the geo data.
	vector<int> viAncestry = { -1 };
	size_t maxAncestor = 0;
	for (int jj = 1; jj < vvsRow.size(); jj++) {
		try {
			geoCode = stoi(vvsRow[jj][indexCode]);
			geoLevel = stoi(vvsRow[jj][indexLevel]);
		}
		catch (invalid_argument) { err("geoCode/geoLevel stoi-insertGeo"); }

		if (geoLevel > viAncestry.size()) { err("GeoLayer gap found-insertGeo"); }
		else if (geoLevel == viAncestry.size()) { 
			viAncestry.push_back(geoCode); 
			for (int kk = 0; kk < viAncestry.size() - 1; kk++) {
				vvsRow[jj].emplace_back(to_string(viAncestry[kk]));
			}
		}
		else {
			while (geoLevel < viAncestry.size() - 1) { viAncestry.pop_back(); }
			viAncestry.back() = geoCode;
			for (int kk = 0; kk < viAncestry.size() - 1; kk++) {
				vvsRow[jj].emplace_back(to_string(viAncestry[kk]));
			}
		}

		if (viAncestry.size() - 1 > maxAncestor) { maxAncestor = viAncestry.size() - 1; }
	}
	for (int jj = 0; jj < maxAncestor; jj++) {
		vvsRow[0].emplace_back("Ancestor" + to_string(jj));
	}

	// Insert the geographic data into the database, as a transaction.
	if (safeInsertRow(tname, vvsRow)) {
		sf.insertRow(tname, vvsRow);
	}
}
void SCdatabase::insertGeo(vector<string>& vsGeoLayer, string cataDir, string sYear, string sCata)
{
	// This variant uses the catalogue's local Geo file as well as the catalogue's template GeoTree
	// to insert a complete region tree into the database. If a GeoLayer exists in the original
	// catalogue AND the template, the values from the original catalogue are used.
	size_t pos1, pos2, pos3;
	string geoFile, geoTemplateFile, sGeoLayer, sGeoLevel, sTemplateCata, temp, tnameTemplate;
	vector<string> conditions, vsRow, vsTag, vsUnique;
	vector<vector<string>> vvsGeo, vvsGeoTemplate, vvsResult, vvsRow, vvsTag;

	// Load the table's column titles and create the table in the database, if necessary.
	int error, indexCode = -1, indexLevel = -1, indexRegion = -1;
	vector<string> search = { "*" };
	string tname = "Geo" + marker + sYear + marker + sCata;
	vsTag = { "table", "Geo_year_cata" };
	vvsTag = jparse.getXML(configXML, vsTag);
	xmlToColTitle(vvsRow, vsUnique, vvsTag);
	if (!sf.tableExist(tname)) {
		sf.createTable(tname, vvsRow, vsUnique);
	}
	for (int ii = 0; ii < vvsRow[0].size(); ii++) {
		if (vvsRow[0][ii] == "GEO_CODE") {
			indexCode = ii;
			continue;
		}
		else if (vvsRow[0][ii] == "Region Name") {
			indexRegion = ii;
			continue;
		}
		else if (vvsRow[0][ii] == "GEO_LEVEL") {
			indexLevel = ii;
			continue;
		}
	}
	if (indexCode < 0 || indexLevel < 0 || indexRegion < 0) { err("Failed to determine column indices-fillGeoGap"); }
	vvsRow.pop_back();

	// Prepare template helper's database geo table.
	for (int ii = 0; ii < vsGeoLayer.size(); ii++) {
		if (vsGeoLayer[ii][0] != '!') { sGeoLayer += marker + vsGeoLayer[ii]; }
	}
	tnameTemplate = "GeoTreeTemplate" + marker + sYear + sGeoLayer;
	if (!sf.tableExist(tnameTemplate)) { err("Missing template table-fillGeoGap"); }
	//sf.select(search, tnameTemplate, vvsGeoTemplate);

	// Load the local Geo file for the catalogue. If it is useful before the GeoLayer gap, 
	// then prepend it and adjust the template ancestral GEO_CODEs accordlingly. In either case,
	// add the local geo row ancestries such that they bind to the templated parent regions.
	bool templateInserted = 0;
	int geoCode, geoLevel;
	vector<int> viAncestry = { -1 };
	string geoPath = cataDir + "/Geo_" + sCata + ".txt";
	jfile.load(geoFile, geoPath);
	size_t length = geoFile.size();
	for (int ii = 0; ii < vsGeoLayer.size(); ii++) {
		sGeoLayer = to_string(ii);
		if (vsGeoLayer[ii][0] == '!') {  // Append from template table.
			viAncestry.clear(); 
			vvsResult.clear();
			conditions = { "GEO_LEVEL = " + sGeoLayer };
			error = sf.select(search, tnameTemplate, vvsResult, conditions);
			if (error == 0) { err("No GEO_LEVEL matches within Geo template table-insertGeo"); }
			
			if (ii == 0) { vvsRow.emplace_back(vvsResult[0]); }
			else {
				for (int jj = 0; jj < vvsResult.size(); jj++) {
					conditions = { "GEO_CODE = " + vvsResult[jj].back() };
					vsRow.clear();
					error = sf.select(search, tnameTemplate, vsRow, conditions);
					if (error == 0) { err("Failed to locate parent region in GeoTree-insertGeo"); }

					for (int kk = vvsRow.size() - 1; kk >= 0; kk--) {
						if (vvsRow[kk][indexRegion] == vsRow[indexRegion]) {
							// Parent region exists within the region tree.
							vvsResult[jj].resize(3);
							for (int ll = 3; ll < vvsRow[kk].size(); ll++) {
								vvsResult[jj].emplace_back(vvsRow[kk][ll]);
							}
							if (kk == vvsRow.size() - 1) { vvsRow.emplace_back(vvsResult[jj]); }
							else { vvsRow.emplace(vvsRow.begin() + kk + 1, vvsResult[jj]); }
							break;
						}
						else if (kk == 0) { err("Failed to locate parent row within vvsRow-insertGeo"); }
					}
				}
			}
		}
		else {  // Append from local file.
			// Rebuild viAncestry if necessary.
			if (viAncestry.size() < 1) {
				viAncestry = { -1 };
				for (int jj = 0; jj < vvsRow.size(); jj++) {
					try {
						geoCode = stoi(vvsRow[jj][indexCode]);
						geoLevel = stoi(vvsRow[jj][indexLevel]);
					}
					catch (invalid_argument) { err("geoCode/geoLevel stoi-insertGeo"); }
					
					if (geoLevel > viAncestry.size()) { err("GeoLayer gap found-insertGeo"); }
					else if (geoLevel == viAncestry.size()) { viAncestry.push_back(geoCode); }
					else {
						while (geoLevel < viAncestry.size() - 1) { viAncestry.pop_back(); }
						viAncestry.back() = geoCode;
					}
				}
				templateInserted = 1;
			}
			
			// Parse the file, line by line.
			pos1 = geoFile.find('\n');
			pos1 = geoFile.find(marker, pos1);  // Skip the title row - it is already within vvsRow.
			while (pos1 < length) {
				pos2 = geoFile.find('\n', pos1);
				temp = geoFile.substr(pos1, pos2 - pos1);
				vsRow.clear();
				jparse.splitByMarker(vsRow, temp);
				if (ii == 0) {
					if (vsRow[indexRegion] != "Canada") { err("Failed to locate root region from file-insertGeo"); }
					vsRow[indexLevel] = "0";
					vvsRow.emplace_back(vsRow);
					break;
				}
				
				// This region must be added to vvsRow.
				if (templateInserted || vsRow[indexLevel] == sGeoLayer) {  
					// Je m'excuse ici - c'est pour la simplicité...
					pos1 = vsRow[indexRegion].find(" part");
					if (pos1 < vsRow[indexRegion].size()) {
						pos2 = vsRow[indexRegion].find('/');
						if (pos1 < pos2) {  // English listed first.
							vsRow[indexRegion].resize(pos1 + 6);
							vsRow[indexRegion].back() = ')';
						}
						else {  // French listed first.
							pos1 = vsRow[indexRegion].rfind(' ', pos1 - 1) + 1;
							pos2 = vsRow[indexRegion].rfind('(', pos2);
							vsRow[indexRegion].replace(pos2, pos1 - pos2, "(");
						}
						try { geoLevel = stoi(vsRow[indexLevel]); }
						catch (invalid_argument) { err("geoLevel stoi-insertGeo"); }
						geoLevel--;
						vsRow[indexLevel] = to_string(geoLevel);
					}
					pos1 = vsRow[indexRegion].find(" / ");
					if (pos1 < vsRow[indexRegion].size()) {
						vsRow[indexRegion].resize(pos1);
					}

					// Update viAncestry.
					try {
						geoCode = stoi(vsRow[indexCode]);
						geoLevel = stoi(vsRow[indexLevel]);
					}
					catch (invalid_argument) { err("geoCode/geoLevel stoi-insertGeo"); }
					if (geoLevel > viAncestry.size()) { err("GeoLayer gap found-insertGeo"); }
					else if (geoLevel == viAncestry.size()) { viAncestry.push_back(geoCode); }
					else {
						while (geoLevel < viAncestry.size() - 1) { viAncestry.pop_back(); }
						viAncestry.back() = geoCode;
					}

					// Append the row.
					for (int jj = 0; jj < viAncestry.size() - 1; jj++) {
						vsRow.emplace_back(to_string(viAncestry[jj]));
					}
					vvsRow.emplace_back(vsRow);
				}
				pos1 = geoFile.find(marker, pos2);
			}
			if (templateInserted) { break; }
		}
	}
	
	// Expand the column titles as necessary, and fix the split regions as necessary.
	bool orphan;
	string sParent, sParentGeoCode;
	size_t maxAncestor = 0;
	for (int ii = 1; ii < vvsRow.size(); ii++) {
		if (vvsRow[ii].size() - 3 > maxAncestor) { maxAncestor = vvsRow[ii].size() - 3; }
		
		pos2 = vvsRow[ii][indexRegion].find(" part");
		if (pos2 < vvsRow[ii][indexRegion].size()) {
			orphan = 1;

			// Eliminate the merged region from this split-region pair.
			pos1 = vvsRow[ii][indexRegion].find(vvsRow[ii - 1][indexRegion]);
			if (pos1 < vvsRow[ii][indexRegion].size()) {
				vvsRow.erase(vvsRow.begin() + ii - 1);
				ii--;
			}

			// Determine the parent region, and relocate this region if necessary.
			pos1 = vvsRow[ii][indexRegion].rfind('(', pos2);
			sParent = vvsRow[ii][indexRegion].substr(pos1 + 1, pos2 - pos1 - 1);
			sParentGeoCode = vvsRow[ii].back();
			for (int jj = ii - 1; jj >= 0; jj++) {
				if (vvsRow[jj][indexCode] == sParentGeoCode) {
					if (vvsRow[jj][indexRegion] == sParent) { orphan = 0; }
					break;
				}
			}
			if (orphan) {
				for (int jj = 0; jj < vvsRow.size(); jj++) {
					if (vvsRow[jj][indexRegion] == sParent) {
						vvsRow[ii].resize(vvsRow[jj].size() + 1);
						for (int kk = 3; kk < vvsRow[jj].size(); kk++) {
							vvsRow[ii][kk] = vvsRow[jj][kk];
						}
						vvsRow[ii].back() = vvsRow[jj][indexCode];
						vvsRow.insert(vvsRow.begin() + jj + 1, vvsRow[ii]);
						vvsRow.erase(vvsRow.begin() + ii);
						ii--;
						break;
					}
					else if (jj == vvsRow.size() - 1) { err("Failed to relocate split region-insertGeo"); }
				}
			}
		}
	}
	vvsRow[0].resize(3 + maxAncestor);
	for (int ii = 0; ii < maxAncestor; ii++) {
		vvsRow[0][3 + ii] = "Ancestor" + to_string(ii);
	}

	// Insert the geographic data into the database, as a transaction.
	if (safeInsertRow(tname, vvsRow)) {
		sf.insertRow(tname, vvsRow);
	}
}
vector<string> SCdatabase::insertGeoLayer(string cataDir, string sYear, string sCata) 
{
	// Insert this catalogue's entry into the GeoLayer table. The GeoLayers shown there
	// will represent the layers which contain data - map representation may contain
	// additional layers (without data) inserted for continuity between parent/child regions.
	vector<string> vsTag, vsUnique;
	vector<vector<string>> vvsColTitle, vvsRow, vvsTag;

	string tname = "GeoLayer" + marker + sYear;
	if (!sf.tableExist(tname)) {
		vsTag = { "table", "GeoLayer_year" };
		vvsTag = jparse.getXML(configXML, vsTag);
		xmlToColTitle(vvsColTitle, vsUnique, vvsTag);
		sf.createTable(tname, vvsColTitle, vsUnique);
	}

	string filePath = cataDir + "/GeoLayer_" + sCata + ".txt";
	string geoLayerFile;
	jfile.load(geoLayerFile, filePath);
	while (geoLayerFile.back() == '\n') { geoLayerFile.pop_back(); }
	vvsRow.resize(2, vector<string>());
	jparse.splitByMarker(vvsRow[1], geoLayerFile);
	if (vvsRow[1].size() > vvsColTitle[0].size()) { 
		vvsRow[0] = vvsColTitle[0];
		safeInsertRow(tname, vvsRow);
	}
	else if (vvsRow[1].size() < vvsColTitle[0].size()) {
		for (int ii = 0; ii < vvsRow[1].size(); ii++) {
			vvsRow[0][ii] = vvsColTitle[0][ii];
		}
	}
	sf.insertRow(tname, vvsRow);

	// Return the GeoLayers contained within the original catalogue, to determine if the
	// catalogue contains parent/child gaps.
	return vvsRow[1];
}
void SCdatabase::insertGeoTreeTemplate(string yearDir)
{
	// GeoTreeTemplates are tree structures representing geographic region families. 
	// The templates are used to fill in the gaps present in some catalogues (otherwise
	// missing some regions). Templates for different catalogue types must be specified 
	// by the user prior to inserting catalogues from a given year. Template trees 
	// should NEVER have a gap between a parent region and a child region. 
	size_t pos1 = yearDir.find_last_of("/\\") + 1;
	string sYear = yearDir.substr(pos1);
	vector<string> vsTag = { "file_name", sYear, "geo_tree_template" };
	string filePath = yearDir + "/" + jparse.getXML1(configXML, vsTag);
	string geoTreeTemplate;
	jfile.load(geoTreeTemplate, filePath);
	
	vector<string> vsCata, vsGeoLayer, vsUnique;
	size_t maxAncestor, pos2, pos3;
	pos1 = 0;
	pos3 = geoTreeTemplate.find("\n\n");
	while (pos1 < geoTreeTemplate.size()) {
		pos2 = geoTreeTemplate.find('\n', pos1);
		if (pos2 == pos3) { err("Missing values-insertGeoTreeTemplate"); }
		vsGeoLayer.emplace_back(geoTreeTemplate.substr(pos1, pos2 - pos1));
		vsCata.emplace_back(geoTreeTemplate.substr(pos2 + 1, pos3 - pos2 - 1));

		pos1 = geoTreeTemplate.find_first_not_of('\n', pos3 + 2);
		pos3 = geoTreeTemplate.find("\n\n", pos3 + 2);
	}

	vsTag = { "table", "Geo_year_cata" };
	vector<vector<string>> vvsTag = jparse.getXML(configXML, vsTag);
	vector<vector<string>> vvsColTitle, vvsRow, vvsSplit; 
	xmlToColTitle(vvsColTitle, vsUnique, vvsTag);

	string geoFile, geoLayerFile, geoPath, sGeoLevel, sParent, tname;
	int geoCode, geoLevel, index, indexCode, indexLevel, indexRegion;
	vector<int> viAncestry;
	int numCata = (int)vsCata.size();
	for (int ii = 0; ii < numCata; ii++) {
		geoPath = yearDir + "/" + vsCata[ii] + "/GeoLayer_" + vsCata[ii] + ".txt";
		jfile.load(geoLayerFile, geoPath);
		while (geoLayerFile.back() == '\n') { geoLayerFile.pop_back(); }
		tname = "GeoTreeTemplate" + marker + sYear + geoLayerFile;
		if (!sf.tableExist(tname)) {
			sf.createTable(tname, vvsColTitle, vsUnique);
		}

		vvsRow.clear();
		vvsSplit.clear();
		geoPath = yearDir + "/" + vsCata[ii] + "/Geo_" + vsCata[ii] + ".txt";
		jfile.load(geoFile, geoPath);
		pos1 = 1;
		pos3 = geoFile.find('\n');
		while (pos1 < geoFile.size()) {
			index = (int)vvsRow.size();
			vvsRow.push_back(vector<string>());
			
			pos2 = geoFile.find(marker, pos1);
			while (pos2 < pos3) {
				vvsRow[index].emplace_back(geoFile.substr(pos1, pos2 - pos1));
				pos1 = pos2 + 1;
				pos2 = geoFile.find(marker, pos1);
			}
			vvsRow[index].emplace_back(geoFile.substr(pos1, pos3 - pos1));

			pos1 = geoFile.find_first_not_of(marker + "\n", pos3 + 1);
			pos3 = geoFile.find('\n', pos3 + 1);
		}

		// Ensure that "Canada" has GEO_LEVEL 0, and that split regions are given their
		// proper GEO_LEVEL (relative to individual parent).
		indexCode = -1, indexLevel = -1, indexRegion = -1;
		for (int jj = 0; jj < vvsRow[0].size(); jj++) {
			pos1 = vvsRow[0][jj].find("GEO_CODE");
			if (pos1 < vvsRow[0][jj].size()) { indexCode = jj; }

			pos1 = vvsRow[0][jj].find("GEO_LEVEL");
			if (pos1 < vvsRow[0][jj].size()) { indexLevel = jj; }
			
			pos1 = vvsRow[0][jj].find("Region");
			if (pos1 < vvsRow[0][jj].size()) { indexRegion = jj; }
		}		
		if (indexCode < 0 || indexLevel < 0 || indexRegion < 0) { err("Failed to determine a geo column index-insertGeoTreeTemplate"); }
		for (int jj = 0; jj < vvsRow.size(); jj++) {
			if (vvsRow[jj][indexRegion] == "Canada") {
				vvsRow[jj][indexLevel] = "0";
				continue;
			}
			
			// Je m'excuse ici - c'est pour la simplicité...
			pos1 = vvsRow[jj][indexRegion].find(" part");
			if (pos1 < vvsRow[jj][indexRegion].size()) {
				pos2 = vvsRow[jj][indexRegion].find('/');
				if (pos1 < pos2) {  // English listed first.
					vvsRow[jj][indexRegion].resize(pos1 + 6);
					vvsRow[jj][indexRegion].back() = ')';
				}
				else {  // French listed first.
					pos1 = vvsRow[jj][indexRegion].rfind(' ', pos1 - 1) + 1;
					pos2 = vvsRow[jj][indexRegion].rfind('(', pos2);
					vvsRow[jj][indexRegion].replace(pos2, pos1 - pos2, "(");
				}
				try { geoLevel = stoi(vvsRow[jj][indexLevel]); }
				catch (invalid_argument) { err("geoLevel stoi-insertGeoTreeTemplate"); }
				geoLevel--;
				vvsRow[jj][indexLevel] = to_string(geoLevel);

				// Remove the union region of two split regions.
				pos1 = vvsRow[jj][indexRegion].find(vvsRow[jj - 1][indexRegion]);
				if (pos1 == 0) {
					vvsRow.erase(vvsRow.begin() + jj - 1);
					jj--;
				}

				// One split region will be correctly positioned after its parent. 
				// The other split region needs to be moved elsewhere.
				pos2 = vvsRow[jj][indexRegion].find(" part");
				pos1 = vvsRow[jj][indexRegion].rfind('(', pos2) + 1;
				sParent = vvsRow[jj][indexRegion].substr(pos1, pos2 - pos1);
				sGeoLevel = to_string(geoLevel - 1);
				for (int kk = jj - 1; kk >= 0; kk--) {
					if (vvsRow[kk][indexLevel] == sGeoLevel) {
						if (vvsRow[kk][indexRegion] != sParent) {
							vvsSplit.emplace_back(vvsRow[jj]);
							vvsRow.erase(vvsRow.begin() + jj);
							jj--;
						}
						break;
					}
					else if (kk == 0) { err("Failed to locate any possible parent region-insertGeo"); }
				}
				continue;
			}
			pos1 = vvsRow[jj][indexRegion].find(" / ");
			if (pos1 < vvsRow[jj][indexRegion].size()) {
				vvsRow[jj][indexRegion].resize(pos1);
			}
		}
		for (int jj = 0; jj < vvsSplit.size(); jj++) {
			pos2 = vvsSplit[jj][indexRegion].rfind(" part");
			pos1 = vvsSplit[jj][indexRegion].rfind('(', pos2) + 1;
			sParent = vvsSplit[jj][indexRegion].substr(pos1, pos2 - pos1);
			for (int kk = 0; kk < vvsRow.size(); kk++) {
				if (vvsRow[kk][indexRegion] == sParent) {
					vvsRow.insert(vvsRow.begin() + kk + 1, vvsSplit[jj]);
					break;
				}
				else if (kk == vvsRow.size() - 1) { err("Failed to locate correct parent for split region-insertGeo"); }
			}
		}

		// Add ancestry columns to the geo data.
		viAncestry = { -1 };
		maxAncestor = 0;
		for (int jj = 1; jj < vvsRow.size(); jj++) {
			try {
				geoCode = stoi(vvsRow[jj][indexCode]);
				geoLevel = stoi(vvsRow[jj][indexLevel]);
			}
			catch (invalid_argument) { err("geoCode/geoLevel stoi-insertGeoTreeTemplate"); }
			
			if (geoLevel > viAncestry.size()) { err("Skipped a region-insertGeoTreeTemplate"); }
			else if (geoLevel == viAncestry.size()) { viAncestry.push_back(geoCode); }
			else {
				while (geoLevel < viAncestry.size() - 1) { viAncestry.pop_back(); }
				viAncestry.back() = geoCode;
			}

			for (int kk = 0; kk < viAncestry.size() - 1; kk++) {
				vvsRow[jj].emplace_back(to_string(viAncestry[kk]));
			}
			if (viAncestry.size() - 1 > maxAncestor) { maxAncestor = viAncestry.size() - 1; }
		}
		for (int jj = 0; jj < maxAncestor; jj++) {
			vvsRow[0].emplace_back("Ancestor" + to_string(jj));
		}

		// Insert the geographic data into the database, as a transaction.
		if (safeInsertRow(tname, vvsRow)) {
			sf.insertRow(tname, vvsRow);
		}
	}
}
void SCdatabase::insertTopicYear(string sYear, string sTopic)
{
	if (sYear.size() < 1 || sTopic.size() < 1) { err("Missing input-insertTopicYear"); }
	vector<string> vsUnique;
	vector<vector<string>> vvsColTitle;
	vector<string> vsTag = { "table", "Topic_year" };
	vector<vector<string>> vvsTag = jparse.getXML(configXML, vsTag);
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
void SCdatabase::loadMeta(JTXML*& jtxml, unordered_map<string, string>& mapMeta, string cataDir, string sYear, string sCata)
{
	vector<string> vsTag = { "file_name", sYear, "meta" };
	string metaPath = cataDir + "/" + jparse.getXML1(configXML, vsTag);
	size_t pos1 = metaPath.rfind("[cata]");
	if (pos1 < metaPath.size()) { metaPath.replace(pos1, 6, sCata); }
	if (!jfile.fileExist(metaPath)) { err("Missing catalogue's meta file-loadMeta"); }
	
	vsTag = { "parse", "xml_marker", "tag" };
	string xmlTag = jparse.getXML1(configXML, vsTag);
	vsTag[2] = "attribute";
	string xmlAttribute = jparse.getXML1(configXML, vsTag);
	vsTag[2] = "wildcard";
	string xmlWildcard = jparse.getXML1(configXML, vsTag);

	vsTag = { "parse", sYear, "meta" };
	jparse.getXML(mapMeta, configXML, vsTag);
	
	if (jtxml != nullptr) { delete jtxml; }
	jtxml = new JTXML;
	jtxml->initMarker(xmlTag, xmlAttribute, xmlWildcard);
	jtxml->loadXML(metaPath);
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
void SCdatabase::loadTopic(string& sTopic, string cataDir)
{
	size_t pos1 = cataDir.find_last_of("/\\") + 1;
	string sCata = cataDir.substr(pos1);
	size_t pos2 = pos1 - 1;
	pos1 = cataDir.find_last_of("/\\", pos2 - 1) + 1;
	string sYear = cataDir.substr(pos1, pos2 - pos1);

	vector<string> vsTag = { "file_name", sYear, "topic" };
	string fileName = jparse.getXML1(configXML, vsTag);
	pos1 = fileName.rfind("[cata]");
	if (pos1 < fileName.size()) {
		fileName.replace(pos1, 6, sCata);
	}

	string filePath = cataDir + "/" + fileName;
	sTopic.clear();
	jfile.load(sTopic, filePath);
	while (sTopic.back() == '\n') { sTopic.pop_back(); }
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
		if (sf.tableExist(tname)) {
			numCata = sf.select(search, tname, cataList);
			for (int jj = 0; jj < numCata; jj++) {
				JNODE jn;
				jn.vsData[0] = cataList[jj];
				jt.addChild(viYearID[ii], jn);
			}
		}
	}

	sbgui.endCall(myid);
}
void SCdatabase::prepareLocal(string cataDir, string sCata, long long maxFileSize)
{
	string zipPath = cataDir + "/" + sCata + ".zip";
	if (!jfile.fileExist(zipPath)) { err("Missing catalogue zip file-prepareLocal"); }

	// Unzip all files within the archive (not already unzipped).
	size_t pos1;
	string filePath;
	vector<string> vsFilePath, vsFileName;
	jfile.zipFileList(vsFileName, zipPath);
	int numFile = (int)vsFileName.size();
	vsFilePath.resize(numFile);
	for (int ii = 0; ii < numFile; ii++) {
		vsFilePath[ii] = cataDir + "/" + vsFileName[ii];
		filePath = vsFilePath[ii];
		pos1 = filePath.rfind('.');
		filePath.insert(pos1, 1, '*');  // Wildcard inserted in case the file has been split (renamed).
		if (!jfile.fileExist(filePath)) {
			jfile.unzipFile(vsFileName[ii], zipPath, cataDir);
		}
	}

	// Large extracted files are split into pieces, to prevent excessive memory usage.
	uintmax_t fileSize;
	for (int ii = 0; ii < numFile; ii++) {
		if (jfile.fileExist(vsFilePath[ii])) {
			fileSize = jfile.fileSize(vsFilePath[ii]);
			if (fileSize > maxFileSize) {
				jfile.fileSplitter(vsFilePath[ii], maxFileSize);
			}
		}
	}
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
		jparse.splitByMarker(vsParam, vsTable[ii], marker[0]);
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
