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
	if (phone_lines.size() != 0) { return 2; }
	phone_lines.resize(1);
	phone_lines[0].assign(comm.size(), 0);
	pair iresult = map_phone.emplace(id, 0);
	bool success = get<1>(iresult);
	if (!success) { return 1; }
	//signout.assign(workers, -1);
	return 0;
}

// Worker thread assigns itself to an existing job.
int SWITCHBOARD::answer_call(thread::id id, vector<int>& comm)
{
	lock_guard<mutex> addrem(m_sb);
	int inum = phone_lines.size();
	int comm_length = phone_lines[0].size();
	phone_lines.push_back(vector<int>());
	phone_lines[inum].assign(comm_length, 0);
	comm.assign(comm_length, 0);
	pair iresult = map_phone.emplace(id, inum);
	bool success = get<1>(iresult);
	if (!success) { return -1; }
	return inum;  
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

// Allows the manager thread to remove a worker thread.
int SWITCHBOARD::terminateCall(thread::id id, int pindex)
{
	lock_guard<mutex> addrem(m_sb);
	int myIndex = map_phone.at(id);
	if (myIndex != 0) { return 1; }
	phone_lines.erase(phone_lines.begin() + pindex);
	return 0;
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
	int bbq = 1;
}
vector<string> SWITCHBOARD::get_prompt()
{
	lock_guard<mutex> locksbuf(m_sb);
	return sprompt;
}

// Functions related to the orderly access of a task's shared buffer.
bool SWITCHBOARD::push(thread::id id)
{
	lock_guard<mutex> addrem(m_sb);
	int pindex = map_phone.at(id);
	if (pindex < 1) { return 0; }
	return m_calls[pindex - 1].try_lock();
	//qDebug() << "SB pushed " << pindex;
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
	//qDebug() << "SB pulled " << start;
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
	//qDebug() << "SB did not release " << pindex;
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
