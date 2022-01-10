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
	sbgui.getPrompt(vsPrompt);  // Form (year, cata0, cata1, ...)

	int bbq = 1;
}
