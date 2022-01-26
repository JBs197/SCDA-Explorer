#include "SConline.h"

using namespace std;

string SConline::cataPID(std::string sYear, std::string sCata)
{
	// Return a catalogue's PID (product identification number). It is frequently needed
	// when navigating Stats Canada's census website.
	string page, urlTemp;
	urlYear(urlTemp, sYear);
	wf.browse(page, urlTemp);
	string temp = "title=\"Dataset " + sCata;
	size_t pos2 = page.find(temp);
	size_t pos1 = page.find("PID=", pos2) + 4;
	pos2 = page.find_first_not_of("1234567890", pos1);
	string sPID = page.substr(pos1, pos2 - pos1);
	if (sPID.size() < 1) { err("Failed to parse yearly catalogue list-cataPID"); }
	return sPID;
}
void SConline::downloadCata(SWITCHBOARD& sbgui)
{
	thread::id myid = this_thread::get_id();
	vector<int> mycomm;
	sbgui.answerCall(myid, mycomm);
	vector<string> vsPrompt;
	sbgui.getPrompt(vsPrompt);  // Form (year, cata0, cata1, ...)

	// Make the root directory for this census year. 
	int iYear, numThread, numZip;
	try { iYear = stoi(vsPrompt[0]); }
	catch (invalid_argument) { err("stoi-downloadCata"); }
	if (iYear < 1981 || iYear > 2017) { err("Invalid year-downloadCata"); }
	vector<string> vsTag = { "path", "local_storage" };
	vector<vector<string>> vvsTag = jparse.getXML(configXML, vsTag);
	string yearFolder = vvsTag[0][1] + "/" + vsPrompt[0];
	wf.makeDir(yearFolder);

	// Make a GeoTree file for this year, containing a list of all region
	// tree types used by the year's catalogues.
	string geoTreePath = yearFolder + "/GeoTree_" + vsPrompt[0] + ".txt";
	if (!jfile.fileExist(geoTreePath)) {
		makeGeoTree(yearFolder, vsPrompt[0], sbgui);
	}

	// Determine if a separate Topic file is needed for the catalogues.
	string topicFileName;
	string replace = "[cata]";
	vsTag = { "file_name", vsPrompt[0] };
	unordered_map<string, string> mapTag;
	jparse.getXML(mapTag, configXML, vsTag);
	auto it = mapTag.find("topic");
	if (it != mapTag.end()) { topicFileName = it->second; }

	// If no catalogues were specified for the given year, then download
	// all catalogues for that year. 
	int numCata = (int)vsPrompt.size() - 1;
	if (numCata < 0) { err("Invalid prompt-downloadCata"); }
	else if (numCata == 0) {
		for (int ii = 0; ii < vvsCata.size(); ii++) {
			if (vvsCata[ii][0] == vsPrompt[0]) {
				vsPrompt = vvsCata[ii];
				numCata = (int)vsPrompt.size() - 1;
				break;
			}
		}
	}
	mycomm[2] = numCata;
	sbgui.update(myid, mycomm);  // Inform the GUI thread of the task size.

	// Download the requested catalogue ZIP archives (if not already present). 
	size_t pos1;
	string cataFolder, filePath, temp, url;
	vector<string> vsZipFile, vsZipFolder;
	for (int ii = 0; ii < numCata; ii++) {
		cataFolder = yearFolder + "/" + vsPrompt[1 + ii];
		wf.makeDir(cataFolder);

		// Make a Geo file and a GeoLayer file to describe the tree 
		// structure of the regions, if not already present.
		makeGeo(cataFolder, vsPrompt[0], vsPrompt[1 + ii]);

		// If a separate topic file is required, extract it from 
		// the Stats Canada website HTML, if not already present.
		if (topicFileName.size() > 0) {
			filePath = cataFolder + "/" + topicFileName;
			pos1 = filePath.rfind(replace);
			if (pos1 < filePath.size()) {
				filePath.replace(pos1, replace.size(), vsPrompt[1 + ii]);
			}
			downloadTopic(filePath);
		}

		// Download the catalogue ZIP archive, if not already present.
		filePath = cataFolder + "/" + vsPrompt[1 + ii] + ".zip";
		if (!jfile.fileExist(filePath)) {
			urlCataDownload(url, vsPrompt[0], vsPrompt[1 + ii]);
			wf.download(url, filePath);
		}

		mycomm[1]++;
		sbgui.update(myid, mycomm);
	}

	sbgui.endCall(myid);
}
void SConline::downloadTopic(string filePath)
{
	// Niche function to make a tiny "topic" file detailing a catalogue's
	// topic (as some census years do not specify it within the meta file).
	if (jfile.fileExist(filePath)) { return; }
	string webpage, url;
	size_t pos2 = filePath.find_last_of("/\\");
	size_t pos1 = filePath.find_last_of("/\\", pos2 - 1) + 1;
	string sCata = filePath.substr(pos1, pos2 - pos1);
	pos2 = pos1 - 1;
	pos1 = filePath.find_last_of("/\\", pos2 - 1) + 1;
	string sYear = filePath.substr(pos1, pos2 - pos1);
	urlCataTopic(url, sYear, sCata);
	wf.browse(webpage, url);

	vector<string> vsTag = { "parse", "statscan_topic" };
	vector<vector<string>> vvsTag = jparse.getXML(configXML, vsTag);
	vector<vector<string>> vvsClippings = jparse.parseFromXML(webpage, vvsTag);
	if (vvsClippings.size() < 1) { err("parseFromXML-downloadTopic"); }

	wf.printer(filePath, vvsClippings[0][0]);
}
void SConline::err(string message)
{
	string errorMessage = "SConline error:\n" + message;
	JLOG::getInstance()->err(errorMessage);
}
void SConline::getCataTree(JTREE& jt)
{
	// Fetch the complete list of catalogues (organized by year), and
	// structure it as a tree. 

	vector<string> vsCata, vsYear = getListYear();
	int index, numCata, numYear = (int)vsYear.size();
	jt.reset();
	JNODE jnRoot = jt.getRoot();
	int yearID, rootID = jnRoot.ID;
	for (int ii = 0; ii < numYear; ii++) {
		JNODE jnYear;
		jnYear.vsData[0] = vsYear[ii];
		yearID = jnYear.ID;
		jt.addChild(rootID, jnYear);

		vsCata = getListCata(vsYear[ii]);
		numCata = (int)vsCata.size();

		index = (int)vvsCata.size();
		vvsCata.push_back(vector<string>(numCata + 1));
		vvsCata[index][0] = vsYear[ii];

		for (int jj = 0; jj < numCata; jj++) {
			JNODE jnCata;
			jnCata.vsData[0] = vsCata[jj];
			jt.addChild(yearID, jnCata);

			vvsCata[index][1 + jj] = vsCata[jj];
		}
	}
}
vector<string> SConline::getListCata(string sYear)
{
	string webpage, url;
	vector<string> vsCata;
	vector<string> vsTag = { "parse", "statscan_cata" };
	vector<vector<string>> vvsTag = jparse.getXML(configXML, vsTag);
	urlYear(url, sYear);
	wf.browse(webpage, url);
	vector<vector<string>> vvsClippings = jparse.parseFromXML(webpage, vvsTag);
	int numCata = (int)vvsClippings.size();
	vsCata.resize(numCata);
	for (int ii = 0; ii < numCata; ii++) {
		vsCata[ii] = vvsClippings[ii][0];
	}
	jsort.sortAlphabetically(vsCata);
	return vsCata;
}
vector<string> SConline::getListYear()
{
	// Return a list of the years for which census data is available.
	vector<string> vsYear;
	if (urlRoot.size() < 1) { return vsYear; }

	vector<string> vsTag = { "parse", "statscan_year" };
	vector<vector<string>> vvsTag = jparse.getXML(configXML, vsTag);
	string webpage;
	wf.browse(webpage, urlRoot);
	vector<vector<string>> vvsClippings = jparse.parseFromXML(webpage, vvsTag);
	int numYear = (int)vvsClippings.size();
	vsYear.resize(numYear);
	for (int ii = 0; ii < numYear; ii++) {
		vsYear[ii] = vvsClippings[ii][0];
	}
	jsort.integerList(vsYear, JSORT::Increasing);
	return vsYear;
}
void SConline::init(string& xml)
{
	if (xml.size() < 1) { err("Missing configXML-init"); }
	configXML = xml;

	vector<string> vsTag = { "url", "statscan_root" };
	urlRoot = jparse.getXML1(configXML, vsTag);

	mapGeoLayer.clear();
	char marker;
	size_t pos2, pos1 = 1;
	vsTag = { "map", "geo_layer" };
	vector<vector<string>> vvsTag = jparse.getXML(configXML, vsTag);
	for (int ii = 0; ii < vvsTag.size(); ii++) {
		marker = vvsTag[ii][1][0];
		pos2 = vvsTag[ii][1].find(marker, pos1);
		if (pos2 > vvsTag[ii][1].size()) { err("Failed to locate geo_layer marker-initMap"); }
		mapGeoLayer.emplace(vvsTag[ii][1].substr(pos1, pos2 - pos1), vvsTag[ii][1].substr(pos2 + 1));
	}

	int input, output;
	mapGeoLevel.clear();
	pos1 = 1;
	vsTag = { "map", "geo_level" };
	vvsTag = jparse.getXML(configXML, vsTag);
	for (int ii = 0; ii < vvsTag.size(); ii++) {
		marker = vvsTag[ii][1][0];
		pos2 = vvsTag[ii][1].find(marker, pos1);
		if (pos2 > vvsTag[ii][1].size()) { err("Failed to locate geo_level marker-initMap"); }
		try { output = stoi(vvsTag[ii][1].substr(pos2 + 1)); }
		catch (invalid_argument) { err("stoi-initMap"); }
		mapGeoLevel.emplace(vvsTag[ii][1].substr(pos1, pos2 - pos1), output);
	}
	mapGeoIndent.clear();
	pos1 = 1;
	vsTag = { "map", "geo_indent" };
	vvsTag = jparse.getXML(configXML, vsTag);
	for (int ii = 0; ii < vvsTag.size(); ii++) {
		marker = vvsTag[ii][1][0];
		pos2 = vvsTag[ii][1].find(marker, pos1);
		if (pos2 > vvsTag[ii][1].size()) { err("Failed to locate geo_indent marker-initMap"); }
		try { 
			input = stoi(vvsTag[ii][1].substr(pos1, pos2 - pos1));
			output = stoi(vvsTag[ii][1].substr(pos2 + 1)); 
		}
		catch (invalid_argument) { err("stoi-initMap"); }
		mapGeoIndent.emplace(input, output);
	}
}
void SConline::makeGeo(string cataFolder, string sYear, string sCata)
{
	// Go to the catalogue's geographic index page, and write a text
	// file describing the GeoLayer structure, as well as a file listing 
	// each region's GEO_CODE, Region Name, and ancestry.
	size_t pos1, pos2, posBegin, posEnd, posStart, posStop;
	string geoFile, geoLayers, page, sLayer, sMissing, sRegion, temp, url;
	unordered_map<string, string>::iterator it;
	vector<string> vsLayer, vsTag;
	vector<vector<string>> vvsTag;

	string geoLayerPath = cataFolder + "/GeoLayers_" + sCata + ".txt";
	if (!jfile.fileExist(geoLayerPath)) {
		if (page.size() < 1) {
			urlCataGeo(url, sYear, sCata);
			wf.browse(page, url);
			pos1 = page.find("<title>File Not Found");
			if (pos1 < page.size()) { return; }
		}

		vsLayer.clear();
		vsTag = { "parse", "statscan_geo_layer" };
		vvsTag = jparse.getXML(configXML, vsTag);
		geoLayers = jparse.parseFromXML1(page, vvsTag);
		pos1 = 0;
		pos2 = geoLayers.find('/');
		while (pos2 < geoLayers.size()) {
			sLayer = geoLayers.substr(pos1, pos2 - pos1);
			while (sLayer.front() == ' ') { sLayer.erase(sLayer.begin()); }
			while (sLayer.back() == ' ') { sLayer.pop_back(); }
			it = mapGeoLayer.find(sLayer);
			if (vsLayer.size() < 1 || it->second != vsLayer.back()) {
				vsLayer.push_back(it->second);
				geoFile += "$" + it->second;
			}
			pos1 = geoLayers.find_first_not_of("/ ", pos2 + 1);
			if (pos1 > geoLayers.size()) { break; }
			pos2 = geoLayers.find('/', pos1);
		}
		if (pos1 < geoLayers.size()) {
			sLayer = geoLayers.substr(pos1);
			while (sLayer.front() == ' ') { sLayer.erase(sLayer.begin()); }
			while (sLayer.back() == ' ') { sLayer.pop_back(); }
			it = mapGeoLayer.find(sLayer);
			if (vsLayer.size() < 1 || it->second != vsLayer.back()) {
				vsLayer.push_back(it->second);
				geoFile += "$" + it->second;
			}
		}
		jfile.printer(geoLayerPath, geoFile);
	}

	string geoPath = cataFolder + "/Geo_" + sCata + ".txt";
	if (!jfile.fileExist(geoPath)) {
		if (page.size() < 1) {
			urlCataGeo(url, sYear, sCata);
			wf.browse(page, url);
			pos1 = page.find("<title>File Not Found");
			if (pos1 < page.size()) { return; }
		}
		geoFile = "$GEO_CODE$Region Name$GEO_LEVEL\n";

		int geoLevel, indent;
		unsigned long long geoCode;
		vsTag = { "parse", "statscan_geo" };
		vvsTag = jparse.getXML(configXML, vsTag);
		if (vvsTag.size() < 10) { err("Missing vvsTag-makeGeo"); }

		posBegin = page.find(vvsTag[0][1]);
		posEnd = page.find(vvsTag[1][1], posBegin);
		posStart = page.find(vvsTag[2][1], posBegin);
		while (posStart < posEnd) {
			posStop = page.find(vvsTag[3][1], posStart);
			
			pos1 = page.find(vvsTag[4][1], posStart);
			if (pos1 > posStop) { err("Failed to locate begin_substr_1-makeGeo"); }
			pos1 += vvsTag[4][1].size();
			pos2 = page.find(vvsTag[5][1], pos1);
			if (pos2 > posStop) { err("Failed to locate end_substr_1-makeGeo"); }
			temp = page.substr(pos1, pos2 - pos1);
			try { 
				indent = stoi(temp); 
				geoLevel = mapGeoIndent.at(indent);
			}
			catch (invalid_argument) { err("stoi/mapGeoLevel-makeGeo"); }

			pos1 = page.find(vvsTag[6][1], pos2);
			if (pos1 > posStop) { err("Failed to locate begin_substr_2-makeGeo"); }
			pos1 += vvsTag[6][1].size();
			pos2 = page.find(vvsTag[7][1], pos1);
			if (pos2 > posStop) { err("Failed to locate end_substr_2-makeGeo"); }
			temp = page.substr(pos1, pos2 - pos1);
			try { geoCode = stoull(temp); }
			catch (invalid_argument) { 
				pos1 = page.find(vvsTag[6][1], pos2);
				if (pos1 > posStop) { err("Failed to locate begin_substr_2-makeGeo"); }
				pos1 += vvsTag[6][1].size();
				pos2 = page.find(vvsTag[7][1], pos1);
				if (pos2 > posStop) { err("Failed to locate end_substr_2-makeGeo"); }
				temp = page.substr(pos1, pos2 - pos1);
				try { geoCode = stoull(temp); }
				catch (invalid_argument) { err("geoCode stoull-makeGeo"); }
			}

			pos1 = page.find(vvsTag[8][1], pos2);
			if (pos1 > posStop) { err("Failed to locate begin_substr_3-makeGeo"); }
			pos1 += vvsTag[8][1].size();
			pos2 = page.find(vvsTag[9][1], pos1);
			if (pos2 > posStop) { err("Failed to locate end_substr_3-makeGeo"); }
			sRegion = page.substr(pos1, pos2 - pos1);

			geoFile += "$" + to_string(geoCode);
			geoFile += "$" + sRegion;
			geoFile += "$" + to_string(geoLevel) + "\n";

			posStart = page.find(vvsTag[2][1], posStop);
		}

		jfile.printer(geoPath, geoFile);
	}
}
void SConline::makeGeoTree(string yearFolder, string sYear, SWITCHBOARD& sbgui)
{
	// For the given year, obtain a list of all region tree types used, and
	// save that list within the year folder in local storage. For each type,
	// list the catalogues which could serve as a template for tree population.
	size_t pos1, pos2;
	int numType;
	string geoLayer, geoTreeFile, page, sLayer, temp, url;
	vector<vector<string>> vvsGeoLayerCata, vvsTag;
	vector<string> vsTag = { "parse", "statscan_geo_layer" };
	vector<string> vsCata = getListCata(sYear);
	int numCata = (int)vsCata.size();

	thread::id myid = this_thread::get_id();
	vector<int> mycomm = sbgui.getMyComm(myid);
	mycomm[0] = 0;
	mycomm[1] = 0;
	mycomm[2] = numCata;
	sbgui.update(myid, mycomm);

	for (int ii = 0; ii < numCata; ii++) {
		mycomm[1]++;
		sbgui.update(myid, mycomm);

		urlCataGeo(url, sYear, vsCata[ii]);
		wf.browse(page, url);

		// Disregard catalogues which do not have Geographic Index pages.
		pos1 = page.find("</head");
		pos1 = page.rfind("title>File Not Found", pos1);
		if (pos1 < page.size()) { continue; }

		// Extract the GeoLayers.
		vvsTag = jparse.getXML(configXML, vsTag);
		geoLayer = jparse.parseFromXML1(page, vvsTag);
		if (geoLayer.size() < 1) { err("Failed to extract GeoLayers-makeGeoTree"); }

		// Simplify the myriad types into consistent notation.
		pos1 = 0;
		pos2 = geoLayer.find('/');
		if (pos2 > geoLayer.size()) {
			while (geoLayer.back() == ' ') { geoLayer.pop_back(); }
			while (geoLayer.front() == ' ') { geoLayer.erase(geoLayer.begin()); }
			try { sLayer = "$" + mapGeoLayer.at(geoLayer); }
			catch (invalid_argument) { err("Failed to identify GeoLayers"); }
		}
		else {
			sLayer = "";
			while (pos2 < geoLayer.size()) {
				temp = geoLayer.substr(pos1, pos2 - pos1);
				while (temp.back() == ' ') { temp.pop_back(); }
				while (temp.front() == ' ') { temp.erase(temp.begin()); }
				try { sLayer += "$" + mapGeoLayer.at(temp); }
				catch (invalid_argument) { err("Failed to identify GeoLayers"); }

				pos1 = geoLayer.find_first_not_of("/ ", pos2);
				pos2 = geoLayer.find('/', pos1);
			}
			if (pos1 < geoLayer.size()) {
				temp = geoLayer.substr(pos1);
				while (temp.back() == ' ') { temp.pop_back(); }
				while (temp.front() == ' ') { temp.erase(temp.begin()); }
				try { sLayer += "$" + mapGeoLayer.at(temp); }
				catch (invalid_argument) { err("Failed to identify GeoLayers"); }
			}
		}

		// Categorize this catalogue as per its simplified GeoLayers.
		numType = (int)vvsGeoLayerCata.size();
		if (numType < 1) {
			vvsGeoLayerCata.resize(1, vector<string>(2));
			vvsGeoLayerCata[0][0] = sLayer;
			vvsGeoLayerCata[0][1] = vsCata[ii];
		}
		else {
			for (int jj = 0; jj < numType; jj++) {
				if (vvsGeoLayerCata[jj][0] == sLayer) {
					vvsGeoLayerCata[jj].push_back(vsCata[ii]);
					break;
				}
				else if (jj == numType - 1) {
					vvsGeoLayerCata.push_back({ sLayer, vsCata[ii] });
				}
			}
		}
	}

	numType = (int)vvsGeoLayerCata.size();
	for (int ii = 0; ii < numType; ii++) {
		geoTreeFile += vvsGeoLayerCata[ii][0] + "\n";
		for (int jj = 1; jj < vvsGeoLayerCata[ii].size(); jj++) {
			geoTreeFile += "$" + vvsGeoLayerCata[ii][jj];
		}
		geoTreeFile += "\n\n";
	}
	string geoTreePath = yearFolder + "/GeoTree_" + sYear + ".txt";
	jfile.printer(geoTreePath, geoTreeFile);
}
void SConline::urlCataDownload(string& url, string sYear, string sCata)
{
	// Return a url to download a catalogue's complete zip archive.
	string sPID = cataPID(sYear, sCata);

	vector<string> vsTag = { "url", "statscan_download", sYear };
	url = jparse.getXML1(configXML, vsTag);
	size_t pos1 = url.rfind("[pid]");
	if (pos1 > url.size()) { err("Failed to locate [pid] placeholder-urlCataDownload"); }
	url.replace(pos1, 5, sPID);
}
void SConline::urlCataGeo(string& url, string sYear, string sCata)
{
	// Return a URL to the catalogue's geographic index page.
	string sPID = cataPID(sYear, sCata);
	vector<string> vsTag = { "url", "statscan_geo", sYear };
	url = jparse.getXML1(configXML, vsTag);
	size_t pos1 = url.rfind("[pid]");
	if (pos1 > url.size()) { err("Failed to locate [pid] placeholder-urlCataGeo"); }
	url.replace(pos1, 5, sPID);
}
void SConline::urlCataTopic(string& url, string sYear, string sCata)
{
	string sPID = cataPID(sYear, sCata);

	vector<string> vsTag = { "url", "statscan_topic", sYear };
	url = jparse.getXML1(configXML, vsTag);
	size_t pos1 = url.rfind("[pid]");
	if (pos1 > url.size()) { err("Failed to locate [pid] placeholder-urlCataTopic"); }
	url.replace(pos1, 5, sPID);
}
void SConline::urlYear(string& url, string syear)
{
	url = urlRoot + "?Temporal=" + syear;
}
