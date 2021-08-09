#include "switchboard.h"

// Error-related functions.
void SWITCHBOARD::err(string func)
{
	lock_guard<mutex> lock(m_err);
	ofstream ERR;
	string message;
	if (errorPath.size() > 0)
	{
		ERR.open(errorPath, ofstream::app);
		message = "Switchboard error within " + func;
		ERR << message << endl << endl;
		ERR.close();
	}
	exit(EXIT_FAILURE);
}
void SWITCHBOARD::setErrorPath(string errPath)
{
	errorPath = errPath;
}

// Manager thread creates a new job on the switchboard, specifying the number of workers.
int SWITCHBOARD::start_call(thread::id id, int worker_num, vector<int>& comm)
{	
	// Make a new task in the switchboard, and map this thread as its manager.
	lock_guard<mutex> addrem(m_sb);
	workers = worker_num;
	if (phone_lines.size() != 0) { return 1; }
	viParking.assign(workers + 1, 0);
	viParking[0] = 1;
	phone_lines.resize(workers + 1);
	phone_lines[0].assign(comm.size(), 0);
	map_phone.emplace(id, 0);
	return 0;
}

// Worker thread assigns itself to an existing job.
int SWITCHBOARD::answer_call(thread::id id, vector<int>& comm)
{
	// This variant is for a thread which will take the first free index spot.
	lock_guard<mutex> addrem(m_sb);
	int myIndex;
	for (int ii = 0; ii < viParking.size(); ii++)
	{
		if (!viParking[ii])
		{
			myIndex = ii;
			viParking[ii] = 1;
			break;
		}
	}	
	int comm_length = phone_lines[0].size();
	phone_lines[myIndex].assign(comm_length, 0);
	comm.assign(comm_length, 0);
	map_phone.emplace(id, myIndex);
	return myIndex;
}
void SWITCHBOARD::answerCall(thread::id id, vector<int>& comm, int myIndex)
{
	// This variant is for a thread that already knows its assigned index.
	lock_guard<mutex> addrem(m_sb);
	int comm_length = phone_lines[0].size();
	phone_lines[myIndex].assign(comm_length, 0);
	comm.assign(comm_length, 0);
	map_phone.emplace(id, myIndex);
}

// Erases the switchboard's entries for the calling thread's job.
int SWITCHBOARD::end_call(thread::id id)
{
	lock_guard<mutex> addrem(m_sb);
	int pindex = map_phone.at(id);
	if (pindex == 0) 
	{
		phone_lines.clear();
		sprompt.clear();
		map_phone.clear();
	}
	else
	{
		phone_lines[pindex] = { -1, 0 };
		map_phone.erase(id);
	}
	return 0;
}

// Allows a manager thread to remove a worker thread's integer data.
int SWITCHBOARD::terminateWorker(thread::id id, int pindex)
{
	lock_guard<mutex> addrem(m_sb);
	int myIndex = map_phone.at(id);
	if (myIndex < pindex) { phone_lines.erase(phone_lines.begin() + pindex); }
	else { return 1; }
	return 0;
}

// Allows a worker thread to remove itself from the map, but does not remove its final report.
void SWITCHBOARD::terminateSelf(thread::id id)
{
	lock_guard<mutex> addrem(m_sb);
	int inum = map_phone.at(id);
	viParking[inum] = 0;
	map_phone.erase(id);
}

// Any thread can simultaneously give an update on its comm, and receive the current comm status for the job.
vector<vector<int>> SWITCHBOARD::update(thread::id id, vector<int>& comm)
{
	lock_guard<mutex> addrem(m_sb);
	int phone_index;
	try { phone_index = map_phone.at(id); }
	catch (out_of_range) { err("map_phone-sb.update"); }
	phone_lines[phone_index] = comm;
	return phone_lines;
}
vector<int> SWITCHBOARD::getMyComm(thread::id id)
{
	lock_guard<mutex> addrem(m_sb);
	int phone_index = map_phone.at(id);
	vector<int> yourComm = phone_lines[phone_index];
	return yourComm;
}

// Functions related to the switchboard buffers shared between threads. 
void SWITCHBOARD::set_prompt(vector<string>& prompt)
{
	lock_guard<mutex> addrem(m_sb);
	sprompt = prompt;
}
vector<string> SWITCHBOARD::get_prompt()
{
	lock_guard<mutex> locksbuf(m_sb);
	return sprompt;
}
void SWITCHBOARD::setIPrompt(vector<int>& viPrompt)
{
	lock_guard<mutex> addrem(m_sb);
	iPrompt = viPrompt;
}
vector<int> SWITCHBOARD::getIPrompt()
{
	lock_guard<mutex> locksbuf(m_sb);
	return iPrompt;
}
void SWITCHBOARD::setMapSS(unordered_map<string, string> mSS)
{
	mapSS = mSS;
}
unordered_map<string, string> SWITCHBOARD::getMapSS()
{
	return mapSS;
}
vector<string> SWITCHBOARD::requestToGUI(thread::id id, vector<string> sQuery)
{
	m_sb.lock();
	sprompt = sQuery;
	int phone_index;
	try { phone_index = map_phone.at(id); }
	catch (out_of_range) { err("map_phone-sb.update"); }
	phone_lines[phone_index][0] = 3;
	onHold = 1;
	m_sb.unlock();
	while (onHold)
	{
		this_thread::sleep_for(50ms);
	}
	return sprompt;
}

// Thread parking functions.
int SWITCHBOARD::getParkingSpot()
{
	if (viParking.size() < 2) { err("Call not started properly-sb.getParkingSpot"); }
	for (int ii = 1; ii < viParking.size(); ii++)
	{
		if (viParking[ii] == 0) { return ii; }
	}
	return -1;
}

// Functions related to the orderly access of a task's shared buffer.
bool SWITCHBOARD::push(thread::id id)
{
	lock_guard<mutex> addrem(m_sb);
	int pindex = map_phone.at(id);
	if (pindex < 1) { return 0; }
	return m_calls[pindex - 1].try_lock();
}
bool SWITCHBOARD::pushHard(thread::id id)
{
	lock_guard<mutex> addrem(m_sb);
	int pindex = map_phone.at(id);
	if (pindex < 1) { return 0; }
	m_calls[pindex - 1].lock();
	return 1;
}
int SWITCHBOARD::pull(thread::id id, int start)
{
	lock_guard<mutex> addrem(m_sb);
	int pindex = map_phone.at(id);
	if (pindex != 0) { return -1; }
	bool success = 0;
	while (!success)
	{
		start = (start + 1) % workers;
		success = m_calls[start].try_lock();
	}
	manager_use = start;
	return start;
}
bool SWITCHBOARD::done(thread::id id)
{
	int pindex = map_phone.at(id);
	if (pindex > 0)
	{
		m_calls[pindex - 1].unlock();
	}
	else if (pindex == 0)
	{
		m_calls[manager_use].unlock();
		manager_use = -1;
	}
	return 1;
}

// Functions related to the transfer of binary data between callers.
void SWITCHBOARD::getDoubleData(vector<vector<vector<double>>>& doubleData)
{
	doubleData = sbDoubleData;
}
void SWITCHBOARD::setDoubleData(vector<vector<vector<double>>>& doubleData)
{
	sbDoubleData = doubleData;
}
