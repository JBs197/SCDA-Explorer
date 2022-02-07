#pragma once
#include <array>
#include <mutex>
#include <queue>
#include <unordered_map>
#include <utility>
#include "jstring.h"

class SWITCHBOARD                                       // comm protocol:
{                                                       // comm[0] = task status
	int iRotate = -1, managerUse = -1;                  // comm[1] = tasks completed
	std::mutex m_sb, m_queue;                           // comm[2] = maximum tasks
	std::array<std::recursive_mutex, 16> m_calls;       
	std::unordered_map<std::thread::id, int> mapPhone;
	std::vector<std::vector<int>> phoneLines;           // Form [phone index][data understood by participants].
	std::queue<std::string> qWork;
	std::vector<std::vector<std::string>> vvsPrompt;

	void err(std::string message);

public:
	SWITCHBOARD() {}                                                    // Task status definitions:
	~SWITCHBOARD() = default;                                           // 0 = running normally
																		// 1 = completed task
	int answerCall(std::thread::id workerID, std::vector<int>& myComm); // 2 = cancelled task
	int answerCall(std::thread::id workerID, std::vector<int>& myComm, int myIndex);
	int endCall(std::thread::id managerID);								// 3 = paused task
	std::vector<int> getMyComm(std::thread::id id);
	void getPrompt(std::string& sPrompt);
	void getPrompt(std::vector<std::string>& vsPrompt);
	void getPrompt(std::vector<std::vector<std::string>>& vvsPrompt);
	int pullAny(std::thread::id id);
	int pullWork(std::string& work);
	int pullWork(std::vector<std::string>& vsWork, int numRequest = 1);
	bool push(std::thread::id id);
	bool pushHard(std::thread::id id);
	void pushWork(std::string& work);
	void pushWork(std::vector<std::string>& vsWork);
	void setPrompt(std::string& sPrompt);
	void setPrompt(std::vector<std::string>& vsPrompt);
	void setPrompt(std::vector<std::vector<std::string>>& vvsPrompt);
	int startCall(std::thread::id managerID, std::vector<int>& myComm);  
	int startCall(std::thread::id managerID, int commLength);
	int terminateSelf(std::thread::id workerID);
	int terminateWorker(std::thread::id managerID, int workerIndex);
	std::vector<std::vector<int>> update(std::thread::id id, std::vector<int>& myComm);

	bool done(std::thread::id);

};

