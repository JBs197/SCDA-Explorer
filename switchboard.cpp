#include "switchboard.h"

using namespace std;

void SWITCHBOARD::init()
{
	vector<mutex> dummy(128);  // ARBITRARY.
	m_calls.swap(dummy);
}

// Manager thread creates a new job on the switchboard, giving its id, comm, task name and receiving task index.
int SWITCHBOARD::start_call(thread::id id, vector<int>& comm, int& tindex, string tname)
{
	// Find the lowest unused task index number, and claim it.
	lock_guard<mutex> addrem(m_add_remove);
	int task_index;
	for (int ii = 0; ii < task_indices.size(); ii++)  
	{
		if (!task_indices[ii])
		{
			task_index = ii;
			task_indices[ii] = 1;
			if (ii == task_indices.size() - 1)  
			{
				task_indices.push_back(0);  // If this was the only unused tindex, make a new one.
			}
			break;
		}
		else if (ii == task_indices.size() - 1) { return 4; }
	}
	
	// Make a new task in the switchboard, and map this thread as its manager.
	if (task_index >= phone_lines.size())
	{
		task_names.push_back(tname);
		phone_lines.push_back(vector<vector<int>>());          // New task.
		phone_lines[phone_lines.size() - 1].push_back(comm);   // New manager.
		sprompt.push_back(vector<string>());
	}
	else
	{
		task_names[task_index] = tname;
		phone_lines[task_index].push_back(comm);
	}
	pair iresult = map_task.emplace(id, task_index);
	bool success = get<1>(iresult);
	if (!success) { return 1; }
	iresult = map_phone.emplace(id, 0);
	success = get<1>(iresult);
	if (!success) { return 2; }
	pair sresult = map_name.emplace(id, tname);
	success = get<1>(sresult);
	if (!success) { return 3; }
	tindex = task_index;
	return 0;
}

// Worker thread assigns itself to an existing job, giving its id, comm, and the task index.
int SWITCHBOARD::answer_call(thread::id id, vector<int>& comm, int& tindex)
{
	lock_guard<mutex> addrem(m_add_remove);
	int pindex = phone_lines[tindex].size();
	int comm_size = phone_lines[tindex][0].size();
	comm.assign(comm_size, 0);
	phone_lines[tindex].push_back(comm);
	pair iresult = map_task.emplace(id, tindex);
	bool success = get<1>(iresult);
	if (!success) { return -1; }
	iresult = map_phone.emplace(id, pindex);
	success = get<1>(iresult);
	if (!success) { return -2; }
	pair sresult = map_name.emplace(id, task_names[tindex]);
	success = get<1>(sresult);
	if (!success) { return -3; }
	return pindex;  // Easy way for worker threads to gain a simple ID.
}

// Erases the switchboard's entries for the calling thread's job.
int SWITCHBOARD::end_call(thread::id id)
{
	lock_guard<mutex> addrem(m_add_remove);
	int task_index = map_task.at(id);
	lock_guard<mutex> mycall(m_calls[task_index]);
	task_names[task_index] = "";
	phone_lines[task_index].clear();
	task_indices[task_index] = 0;
	size_t success = map_task.erase(id);
	if (!success) { return 1; }
	success = map_phone.erase(id);
	if (!success) { return 2; }
	success = map_name.erase(id);
	if (!success) { return 3; }
	return 0;
}

// Any thread can simultaneously give an update on its comm, and receive the current comm status for the job.
vector<vector<int>> SWITCHBOARD::update(thread::id id, vector<int>& comm)
{
	lock_guard<mutex> addrem(m_add_remove);
	int task_index = map_task.at(id);
	int phone_index = map_phone.at(id);
	phone_lines[task_index][phone_index] = comm;
	return phone_lines[task_index];
}

// Any thread can get the name of its job.
string SWITCHBOARD::get_name(thread::id id)
{
	lock_guard<mutex> addrem(m_add_remove);
	int task_index = map_task.at(id);
	if (task_names.size() > task_index)
	{
		return task_names[task_index];
	}
	return "ERROR: task index exceeds the boundary of the task name list.";
}

// Functions related to the switchboard buffers shared between threads. 
void SWITCHBOARD::set_prompt(thread::id id, vector<string>& prompt)
{
	lock_guard<mutex> locksbuf(m_sbuf);
	int task_index = map_task.at(id);
	sprompt[task_index] = prompt;
}
vector<string> SWITCHBOARD::get_prompt(thread::id id)
{
	lock_guard<mutex> locksbuf(m_sbuf);
	int task_index = map_task.at(id);
	return sprompt[task_index];
}
