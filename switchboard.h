#pragma once

#include <vector>
#include <fstream>
#include <mutex>
#include <unordered_map>
#include <utility>
#include <array>

using namespace std;

class SWITCHBOARD                                   // comm protocol:
{                                                   // comm[0] = task status
	mutex m_sb, m_err;                              // comm[1] = jobs completed
	array<recursive_mutex, 16> m_calls;             // comm[2] = jobs max
	unordered_map<thread::id, int> map_phone;       // comm[3] = max table name parameters 
	vector<vector<int>> phone_lines;          // Form [phone index][data understood by participants].
	vector<string> sprompt;                   // Form [prompt0, prompt1, ...].	
	vector<int> iPrompt, viParking;
	int workers, onHold = 0;
	int manager_use;
	vector<vector<vector<double>>> sbDoubleData;
	string errorPath;
	vector<vector<string>> buffer2;

public:
	explicit SWITCHBOARD() {}                       // Task status definitions:
	~SWITCHBOARD() {}                               // 0 = running normally
	int start_call(thread::id, int, vector<int>&);  // 1 = completed task
	int answer_call(thread::id, vector<int>&);      // 2 = cancelled task
	int end_call(thread::id);                       // 3 = paused task
	void err(string func);
	void answerCall(thread::id id, vector<int>& comm, int myIndex);
	bool done(thread::id);
	void getDoubleData(vector<vector<vector<double>>>& doubleData);
	vector<int> getIPrompt();
	int getParkingSpot();
	vector<string> get_prompt();
	vector<int> getMyComm(thread::id);
	bool push(thread::id);
	bool pushHard(thread::id);
	int pull(thread::id, int);
	vector<string> requestToGUI(thread::id id, vector<string> sQuery);
	void reserveBuffer2(thread::id id);
	void setDoubleData(vector<vector<vector<double>>>& doubleData);
	void setErrorPath(string errPath);
	void setIPrompt(vector<int>& viPrompt);
	void set_prompt(vector<string>&);
	void terminateSelf(thread::id);
	int terminateWorker(thread::id id, int pindex);
	vector<vector<int>> update(thread::id, vector<int>&);
};

