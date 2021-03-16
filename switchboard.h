#pragma once

#include <vector>
#include <mutex>
#include <unordered_map>
#include <utility>

using namespace std;

class SWITCHBOARD                                           // comm protocol:
{                                                           // comm[0] = task status
	mutex m_add_remove, m_sbuf;                             // comm[1] = jobs completed
	vector<mutex> m_calls;                                  // comm[2] = jobs max
	vector<bool> task_indices = { 0 };                      // comm[3] = max table name parameters
	vector<string> task_names;  // Form [task index].
	vector<vector<vector<int>>> phone_lines;  // Form [task index][phone index][data understood by participants].
	vector<vector<string>> sprompt;  // Form [task index][prompt0, prompt1, ...].
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
	void set_prompt(thread::id, vector<string>&);
	vector<string> get_prompt(thread::id);
};

