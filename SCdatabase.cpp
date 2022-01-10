#include "SCdatabase.h"

void SCdatabase::err(string message)
{
	string errorMessage = "SCdatabase error:\n" + message;
	JLOG::getInstance()->err(errorMessage);
}
void SCdatabase::getCataTree(JTREE& jt)
{
	// Obtain a tree structure representing the database's catalogues
	// organized by census year. 

	jt.reset();

}
void SCdatabase::insertCata(SWITCHBOARD& sbgui)
{
	thread::id myid = this_thread::get_id();
	vector<int> mycomm;
	sbgui.answerCall(myid, mycomm);
	vector<string> vsPrompt; 
	sbgui.getPrompt(vsPrompt);  // Form (pathCata0, pathCata1, ...)



	int bbq = 1;
}
void SCdatabase::log(string message)
{
	string logMessage = "SCdatabase log entry:\n" + message;
	JLOG::getInstance()->log(logMessage);
}
