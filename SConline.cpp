#include "SConline.h"

void SConline::downloadCata(SWITCHBOARD& sbgui)
{
	thread::id myid = this_thread::get_id();
	vector<int> mycomm;
	sbgui.answerCall(myid, mycomm);
	vector<string> vsPrompt;
	sbgui.getPrompt(vsPrompt);  // Form (year, cata0, cata1, ...)

	// Make the root directory for this census year. 
	int iYear, numMissing, numThread, numZip;
	try { iYear = stoi(vsPrompt[0]); }
	catch (invalid_argument) { err("stoi-downloadCata"); }
	if (iYear < 1981 || iYear > 2017) { err("Invalid year-downloadCata"); }
	vector<string> vsTag = { "path", "local_storage" };
	vector<vector<string>> vvsTag = jf.getXML(configXML, vsTag);
	string yearFolder = vvsTag[0][1] + "/" + vsPrompt[0];
	wf.makeDir(yearFolder);

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
	mycomm[2] = numCata * 3;
	sbgui.update(myid, mycomm);  // Inform the GUI thread of the task size.

	// Prepare a list of file names which are needed to insert the 
	// catalogue into the database.
	vsTag = { "file_name", vsPrompt[0] };
	vvsTag = jf.getXML(configXML, vsTag);
	string fileName, replace = "[cata]";

	// Download the missing catalogue files.
	size_t pos1;
	string url, cataFolder;
	vector<string> vsZipFile, vsZipFolder;
	for (int ii = 0; ii < numCata; ii++) {
		cataFolder = yearFolder + "/" + vsPrompt[1 + ii];
		wf.makeDir(cataFolder);

		numMissing = 0;
		for (int jj = 0; jj < vvsTag.size(); jj++) {
			fileName = cataFolder + "/" + vvsTag[jj][1];
			pos1 = fileName.rfind(replace);
			if (pos1 < fileName.size()) {
				fileName.replace(pos1, replace.size(), vsPrompt[1 + ii]);
			}
			if (!wf.file_exist(fileName)) {
				numMissing++;
				break;
			}
		}
		if (numMissing == 0) { 
			mycomm[1] += 2;
			sbgui.update(myid, mycomm);
			continue; 
		}

		fileName = cataFolder + "/" + vsPrompt[1 + ii] + ".zip";
		url = urlCataDownload(vsPrompt[0], vsPrompt[1 + ii]);
		wf.download(url, fileName);

		vsZipFile.push_back(fileName);
		pos1 = fileName.rfind('/');
		vsZipFolder.push_back(fileName.substr(0, pos1));

		mycomm[1]++;
		sbgui.update(myid, mycomm);
	}

	// Unzip all necessary files, using multiple worker threads.
	vsTag = { "settings", "cpu_cores" };
	vvsTag = jf.getXML(configXML, vsTag);
	try { numThread = stoi(vvsTag[0][1]); }
	catch (invalid_argument) { err("stoi-downloadCata"); }
	numZip = (int)vsZipFile.size();
	if (numZip == 1) { jf.unzip(vsZipFile[0], vsZipFolder[0]); }
	else { jf.unzip(numThread, vsZipFile, vsZipFolder); }
	
	mycomm[1] += numZip;
	sbgui.update(myid, mycomm);

	// Delete the zip files, and note the paths of all files larger
	// than maxFileSize. 
	vector<string> vsLarge, vsName;
	long long fileSize;
	if (maxFileSize < 0) {
		vsTag = { "settings", "max_file_size" };
		vvsTag = jf.getXML(configXML, vsTag);
		try { maxFileSize = stoll(vvsTag[0][1]); }
		catch (invalid_argument) { err("stoll-downloadCata"); }
	}
	for (int ii = 0; ii < numZip; ii++) {
		wf.deleteFile(vsZipFile[ii]);
		vsName = wf.getFileList(vsZipFolder[ii], "*");
		for (int jj = 0; jj < vsName.size(); jj++) {
			fileName = vsZipFolder[ii] + "/" + vsName[jj];
			fileSize = wf.getFileSize(fileName);
			if (fileSize > maxFileSize) {
				vsLarge.push_back(fileName);
			}
		}
	}
	mycomm[1] += numCata - (int)vsLarge.size();
	sbgui.update(myid, mycomm);

	// Split all the large files, using multiple worker threads.
	jf.fileSplitter(numThread, vsLarge, maxFileSize);
	wf.deleteFile(vsLarge);
	mycomm[1] = mycomm[2];
	mycomm[0] = 1;
	sbgui.update(myid, mycomm);
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
		jnYear.vsData.push_back(vsYear[ii]);
		yearID = jnYear.ID;
		jt.addChild(rootID, jnYear);

		vsCata = getListCata(vsYear[ii]);
		numCata = (int)vsCata.size();

		index = (int)vvsCata.size();
		vvsCata.push_back(vector<string>(numCata + 1));
		vvsCata[index][0] = vsYear[ii];

		for (int jj = 0; jj < numCata; jj++) {
			JNODE jnCata;
			jnCata.vsData.push_back(vsCata[jj]);
			jt.addChild(yearID, jnCata);

			vvsCata[index][1 + jj] = vsCata[jj];
		}
	}
}
vector<string> SConline::getListCata(string sYear)
{
	vector<string> vsCata;
	vector<string> vsTag = { "parse", "statscan_cata" };
	vector<vector<string>> vvsTag = jf.getXML(configXML, vsTag);
	string url = urlYear(sYear);
	string webpage = wf.browseS(url);
	vector<vector<string>> vvsClippings = jf.parseFromXML(webpage, vvsTag);
	int numCata = (int)vvsClippings.size();
	vsCata.resize(numCata);
	for (int ii = 0; ii < numCata; ii++) {
		vsCata[ii] = vvsClippings[ii][0];
	}
	jf.sortAlphabetically(vsCata);
	return vsCata;
}
vector<string> SConline::getListYear()
{
	// Return a list of the years for which census data is available.
	vector<string> vsYear;
	if (urlRoot.size() < 1) { return vsYear; }

	vector<string> vsTag = { "parse", "statscan_year" };
	vector<vector<string>> vvsTag = jf.getXML(configXML, vsTag);
	string webpage = wf.browseS(urlRoot);
	vector<vector<string>> vvsClippings = jf.parseFromXML(webpage, vvsTag);
	int numYear = (int)vvsClippings.size();
	vsYear.resize(numYear);
	for (int ii = 0; ii < numYear; ii++) {
		vsYear[ii] = vvsClippings[ii][0];
	}
	jf.sortInteger(vsYear, JFUNC::Increasing);
	return vsYear;
}
string SConline::urlCataDownload(string sYear, string sCata)
{
	string url, urlTemp;
	int iYear;
	try { iYear = stoi(sYear); }
	catch (invalid_argument) { err("stoi-sc.urlCSVDownload"); }

	urlTemp = "www12.statcan.gc.ca/datasets/Index-eng.cfm?Temporal=" + sYear;
	wstring wPage = wf.browseW(urlTemp);
	wstring wCata = jf.utf8To16(sCata);
	wstring wTemp = L"title=\"Dataset " + wCata;
	size_t pos2 = wPage.find(wTemp);
	size_t pos1 = wPage.find(L"PID=", pos2) + 4;
	pos2 = wPage.find_first_not_of(L"1234567890", pos1);
	wTemp = wPage.substr(pos1, pos2 - pos1);
	if (iYear == 2016 || iYear == 2017) {
		url = "www12.statcan.gc.ca/census-recensement/2016/dp-pd/dt-td/";
		url += "CompDataDownload.cfm?LANG=E&PID=" + jf.utf16To8(wTemp);
		url += "&OFT=CSV";
	}
	else if (iYear == 2013) {
		url = "www12.statcan.gc.ca/nhs-enm/2011/dp-pd/dt-td/";
		url += "OpenDataDownload.cfm?PID=" + jf.utf16To8(wTemp);
	}
	else if (iYear == 2011) {
		url = "www12.statcan.gc.ca/census-recensement/2011/dp-pd/";
		url += "tbt-tt/OpenDataDownload.cfm?PID=" + jf.utf16To8(wTemp);
	}
	else { err("Invalid Year-urlCSVDownload"); }
	return url;
}
string SConline::urlYear(string syear)
{
	string url = "www12.statcan.gc.ca/datasets/index-eng.cfm?Temporal=" + syear;
	return url;
}
