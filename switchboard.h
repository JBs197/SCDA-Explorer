#pragma once

#include <vector>
#include <mutex>
#include <unordered_map>
#include <utility>

using namespace std;

class SWITCHBOARD                                           // comm protocol:
{                                                           // comm[0] = task status
	mutex m_add_remove;                                     // comm[1] = jobs completed
	vector<mutex> m_calls;                                  // comm[2] = jobs max
	vector<bool> task_indices = { 0 };
	vector<string> task_names;  // Form [task index].
	vector<vector<vector<int>>> phone_lines;  // Form [task index][phone index][data understood by participants].
	unordered_map<thread::id, int> map_task;
	unordered_map<thread::id, int> map_phone;
	unordered_map<thread::id, string> map_name;

	void init();

public:
	explicit SWITCHBOARD() { init(); }
	~SWITCHBOARD() {}
	int start_call(thread::id, vector<int>&, int&, string);  // Data lines, task index, task name.
	int answer_call(thread::id, vector<int>&, int&);
	int end_call(thread::id);
	vector<vector<int>> update(thread::id, vector<int>&);
	string get_name(thread::id);
};

