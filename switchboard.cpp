#include "switchboard.h"

using namespace std;

void SWITCHBOARD::init()
{
	vector<mutex> dummy(128);  // ARBITRARY.
	m_calls.swap(dummy);
}

void SWITCHBOARD::start_call(thread::id id, vector<int>& comm, string tname)
{
	lock_guard<mutex> addrem(m_add_remove);
	int task_index = phone_lines.size();
	task_names.push_back(tname);
	phone_lines.push_back(vector<vector<int>>());          // New task.
	phone_lines[phone_lines.size() - 1].push_back(comm);   // New manager.
	ids.push_back(id);
	directory.push_back({ task_index, 0 });
}

void SWITCHBOARD::answer_call(vector<int>& comm, int& tindex)
{
	lock_guard<mutex> addrem(m_add_remove);
	phone_lines[tindex].push_back()

}