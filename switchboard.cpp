#include "switchboard.h"

using namespace std;

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
	if (pindex != 0) { return 1; }
	phone_lines.clear();
	sprompt.clear();
	map_phone.clear();
	//signout.clear();
	return 0;
}

// Any thread can simultaneously give an update on its comm, and receive the current comm status for the job.
vector<vector<int>> SWITCHBOARD::update(thread::id id, vector<int>& comm)
{
	lock_guard<mutex> addrem(m_sb);
	int phone_index = map_phone.at(id);
	phone_lines[phone_index] = comm;
	return phone_lines;
}

// Functions related to the switchboard buffers shared between threads. 
void SWITCHBOARD::set_prompt(thread::id id, vector<string>& prompt)
{
	lock_guard<mutex> addrem(m_sb);
	int phone_index = map_phone.at(id);
	if (phone_index == 0)
	{
		sprompt = prompt;
	}
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
	}
	//qDebug() << "SB did not release " << pindex;
	return 1;
}
