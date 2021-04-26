#pragma once

#include <vector>
#include <mutex>
#include <unordered_map>
#include <utility>
#include <array>
#include <QDebug>

using namespace std;
//extern vector<recursive_mutex> m_calls;

class SWITCHBOARD                                           // comm protocol:
{                                                           // comm[0] = task status
	mutex m_sb;                                             // comm[1] = jobs completed
	array<recursive_mutex, 16> m_calls;                        // comm[2] = jobs max
	unordered_map<thread::id, int> map_phone;               // comm[3] = max table name parameters 
	vector<vector<int>> phone_lines;          // Form [phone index][data understood by participants].
	vector<string> sprompt;                   // Form [prompt0, prompt1, ...].	
	int workers;
	int manager_use;
	vector<vector<vector<double>>> sbDoubleData;

public:
	explicit SWITCHBOARD() {}
	~SWITCHBOARD() {}
	int start_call(thread::id, int, vector<int>&);  
	int answer_call(thread::id, vector<int>&);
	int end_call(thread::id);
	void getDoubleData(vector<vector<vector<double>>>& doubleData);
	vector<string> get_prompt();
	vector<vector<int>> update(thread::id, vector<int>&);
	void setDoubleData(vector<vector<vector<double>>>& doubleData);
	void set_prompt(thread::id, vector<string>&);
	bool done(thread::id);
	bool push(thread::id);
	int pull(thread::id, int);

};

