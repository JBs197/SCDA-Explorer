#include "SConline.h"

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
	int numCata, numYear = vsYear.size();
	jt.reset();
	JNODE jnRoot = jt.getRoot();
	int yearID, rootID = jnRoot.ID;
	for (int ii = 0; ii < numYear; ii++) {
		JNODE jnYear;
		jnYear.vsData.push_back(vsYear[ii]);
		yearID = jnYear.ID;
		jt.addChild(rootID, jnYear);

		vsCata = getListCata(vsYear[ii]);
		numCata = vsCata.size();
		for (int jj = 0; jj < numCata; jj++) {
			JNODE jnCata;
			jnCata.vsData.push_back(vsCata[jj]);
			jt.addChild(yearID, jnCata);
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
	int numCata = vvsClippings.size();
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
	int numYear = vvsClippings.size();
	vsYear.resize(numYear);
	for (int ii = 0; ii < numYear; ii++) {
		vsYear[ii] = vvsClippings[ii][0];
	}
	jf.sortInteger(vsYear, JFUNC::Increasing);
	return vsYear;
}
string SConline::urlYear(string syear)
{
	string url = "www12.statcan.gc.ca/datasets/index-eng.cfm?Temporal=" + syear;
	return url;
}
