#pragma once

#include <vector>
#include <mutex>
#include <unordered_map>

using namespace std;

class SWITCHBOARD                                           // comm protocol:
{                                                           // comm[0] = task status
	mutex m_add_remove;                                     // comm[1] = jobs completed
	vector<mutex> m_calls;                                  // comm[2] = jobs max
	vector<thread::id> ids;  // Form [thread index].
	vector<vector<int>> directory;  // Form [thread index][task index, phone line position].
	vector<string> task_names;  // Form [task index].
	vector<vector<vector<int>>> phone_lines;  // Form [task index][manager status, worker1 status, worker2 status...][data understood by participants].
	unordered_map<thread::id, int> map_task;
	unordered_map<thread::id, int> map_phone;

	void init();

public:
	explicit SWITCHBOARD() { init(); }
	~SWITCHBOARD() {}
	void start_call(thread::id id, vector<int>&, string);  // Data lines, task index, task name.
	void answer_call(vector<int>&, int&);
};

