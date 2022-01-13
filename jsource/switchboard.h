#pragma once
#include <mutex>
#include <unordered_map>
#include <utility>
#include <array>
#include "jstring.h"

using namespace std;

class SWITCHBOARD                                   // comm protocol:
{                                                   // comm[0] = task status
	int iRotate = -1, managerUse;
	mutex m_sb;                                     // comm[1] = tasks completed
	array<recursive_mutex, 16> m_calls;             // comm[2] = maximum tasks
	unordered_map<thread::id, int> mapPhone;        // comm[3] = max table name parameters 
	vector<vector<int>> phoneLines;       // Form [phone index][data understood by participants].
	vector<vector<string>> vvsPrompt;

	void err(string message);

public:
	SWITCHBOARD() {}                                // Task status definitions:
	~SWITCHBOARD() {}                               // 0 = running normally
	
	int answerCall(thread::id workerID, vector<int>& myComm);      // 2 = cancelled task
	int answerCall(thread::id workerID, vector<int>& myComm, int myIndex);
	int endCall(thread::id managerID);                       // 3 = paused task
	vector<int> getMyComm(thread::id id);
	void getPrompt(string& sPrompt);
	void getPrompt(vector<string>& vsPrompt);
	void getPrompt(vector<vector<string>>& vvsPrompt);
	int pullAny(thread::id id);
	bool push(thread::id id);
	bool pushHard(thread::id id);
	void setPrompt(string& sPrompt);
	void setPrompt(vector<string>& vsPrompt);
	void setPrompt(vector<vector<string>>& vvsPrompt);
	int startCall(thread::id managerID, vector<int>& myComm);  // 1 = completed task
	int terminateSelf(thread::id workerID);
	int terminateWorker(thread::id managerID, int workerIndex);
	vector<vector<int>> update(thread::id id, vector<int>& myComm);

	bool done(thread::id);






};

