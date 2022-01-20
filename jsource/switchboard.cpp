#include "switchboard.h"

using namespace std;

// Error-related functions.
void SWITCHBOARD::err(string message)
{
	string errorMessage = "SWITCHBOARD error:\n" + message;
	JLOG::getInstance()->err(errorMessage);
}

// Manager thread creates a new job on the switchboard.
int SWITCHBOARD::startCall(thread::id id, vector<int>& comm)
{	
	// Ensure that the switchboard is available.
	lock_guard<mutex> addrem(m_sb);
	if (mapPhone.size() > 0) { return 1; }

	// Start a new task in the switchboard, and map this thread as its manager.
	if (comm.size() < 1) { return 2; }
	phoneLines.resize(1);
	phoneLines[0] = comm;
	mapPhone.emplace(id, 0);
	return 0;
}

// Worker thread assigns itself to an existing job.
int SWITCHBOARD::answerCall(thread::id id, vector<int>& comm)
{
	// This variant is for a thread which will take the first free index spot.
	lock_guard<mutex> addrem(m_sb);
	int myIndex = (int)mapPhone.size();
	if (myIndex == 0) { return 3; }

	auto feedback = mapPhone.emplace(id, myIndex);
	if (!get<1>(feedback)) { return 4; }

	int commLength = (int)phoneLines[0].size();
	phoneLines.push_back(vector<int>());
	phoneLines[myIndex].assign(commLength, 0);
	comm = phoneLines[myIndex];

	return 0;
}
int SWITCHBOARD::answerCall(thread::id id, vector<int>& comm, int myIndex)
{
	// This variant is for a thread that wants a specific index.
	lock_guard<mutex> addrem(m_sb);
	int nextIndex = (int)mapPhone.size();
	if (nextIndex == 0) { return 3; }

	auto feedback = mapPhone.emplace(id, myIndex);
	if (!get<1>(feedback)) { return 4; }

	int commLength = (int)phoneLines[0].size();
	if (phoneLines.size() <= myIndex) { phoneLines.resize(myIndex + 1); }
	phoneLines[myIndex].assign(commLength, 0);
	comm = phoneLines[myIndex];

	return 0;
}

// Erases the switchboard's entries for the calling thread's job.
int SWITCHBOARD::endCall(thread::id id)
{
	lock_guard<mutex> addrem(m_sb);
	int myIndex = -1;
	auto it = mapPhone.find(id);
	if (it == mapPhone.end()) { return 5; }
	else { myIndex = it->second; }

	if (myIndex == 0) {
		vvsPrompt.clear();
		phoneLines.clear();
		mapPhone.clear();
	}
	else {
		phoneLines[myIndex][1] = phoneLines[myIndex][2];
		phoneLines[myIndex][0] = 1;
	}

	return 0;
}

// Allows a manager thread to remove a worker thread's phone line.
int SWITCHBOARD::terminateWorker(thread::id managerID, int workerIndex)
{
	lock_guard<mutex> addrem(m_sb);
	int myIndex = -1;
	auto it = mapPhone.find(managerID);
	if (it == mapPhone.end()) { return 5; }
	else { 
		myIndex = it->second; 
		if (myIndex != 0) { return 6; }
	}

	for (auto it2 = mapPhone.begin(); it2 != mapPhone.end(); ++it2) {
		if (it2->second == workerIndex) {
			thread::id workerID = it2->first;
			mapPhone.erase(workerID);
			int commLength = (int)phoneLines[0].size();
			phoneLines[workerIndex].assign(commLength, 0);
			return 0;
		}
	}

	return 7;
}

// Allows a worker thread to remove itself from the switchboard.
int SWITCHBOARD::terminateSelf(thread::id workerID)
{
	lock_guard<mutex> addrem(m_sb);
	int myIndex = -1;
	auto it = mapPhone.find(workerID);
	if (it == mapPhone.end()) { return 5; }
	else { myIndex = it->second; }
	int commLength = (int)phoneLines[0].size();
	phoneLines[myIndex].assign(commLength, 0);
	mapPhone.erase(workerID);

	return 0;
}

// Any thread can simultaneously give an update on its comm, and receive the current comm status for the job.
vector<vector<int>> SWITCHBOARD::update(thread::id id, vector<int>& myComm)
{
	lock_guard<mutex> addrem(m_sb);
	vector<vector<int>> comm;
	int myIndex = -1;
	auto it = mapPhone.find(id);
	if (it == mapPhone.end()) { return comm; }
	else { myIndex = it->second; }
	phoneLines[myIndex] = myComm;
	return phoneLines;
}
vector<int> SWITCHBOARD::getMyComm(thread::id id)
{
	lock_guard<mutex> addrem(m_sb);
	vector<int> myComm;
	int myIndex = -1;
	auto it = mapPhone.find(id);
	if (it == mapPhone.end()) { return myComm; }
	else { myIndex = it->second; }
	return phoneLines[myIndex];
}

// Functions related to the switchboard buffers shared between threads. 
void SWITCHBOARD::setPrompt(string& prompt)
{
	lock_guard<mutex> addrem(m_sb);
	vvsPrompt.resize(1, vector<string>(1));
	vvsPrompt[0][0] = prompt;
}
void SWITCHBOARD::setPrompt(vector<string>& prompt)
{
	lock_guard<mutex> addrem(m_sb);
	vvsPrompt.resize(1);
	vvsPrompt[0] = prompt;
}
void SWITCHBOARD::setPrompt(vector<vector<string>>& prompt)
{
	lock_guard<mutex> addrem(m_sb);
	vvsPrompt = prompt;
}
void SWITCHBOARD::getPrompt(string& prompt)
{
	lock_guard<mutex> addrem(m_sb);
	if (vvsPrompt.size() < 1 || vvsPrompt[0].size() < 1) { prompt = ""; }
	else { prompt = vvsPrompt[0][0]; }
}
void SWITCHBOARD::getPrompt(vector<string>& prompt)
{
	lock_guard<mutex> addrem(m_sb);
	if (vvsPrompt.size() < 1) { prompt = { "" }; }
	else { prompt = vvsPrompt[0]; }
}
void SWITCHBOARD::getPrompt(vector<vector<string>>& prompt)
{
	lock_guard<mutex> addrem(m_sb);
	prompt = vvsPrompt;
}

// Functions related to the orderly access of a task's shared memory.
bool SWITCHBOARD::done(thread::id id)
{
	// Release ownership of a shared memory buffer.
	int myIndex = -1;
	auto it = mapPhone.find(id);
	if (it == mapPhone.end()) { return 0; }
	else { myIndex = it->second; }

	if (myIndex > 0) {
		m_calls[myIndex - 1].unlock();
	}
	else {
		m_calls[managerUse].unlock();
		managerUse = -1;
	}
	return 1;
}
int SWITCHBOARD::pullAny(thread::id id)
{
	// Secure exclusive access to the first available shared buffer.
	lock_guard<mutex> addrem(m_sb);
	int myIndex = -1;
	auto it = mapPhone.find(id);
	if (it == mapPhone.end()) { return -1; }
	else { myIndex = it->second; }
	bool success = 0;
	while (!success) {
		iRotate = (iRotate + 1) % ((int)mapPhone.size() - 1);
		success = m_calls[iRotate].try_lock();
	}
	managerUse = iRotate;
	return managerUse;
}
bool SWITCHBOARD::push(thread::id id)
{
	// Attempt to secure access to the shared buffer. If unavailable, quit.
	lock_guard<mutex> addrem(m_sb);
	int myIndex = -1;
	auto it = mapPhone.find(id);
	if (it == mapPhone.end()) { return 0; }
	else { myIndex = it->second; }
	return m_calls[myIndex - 1].try_lock();
}
bool SWITCHBOARD::pushHard(thread::id id)
{
	// Attempt to secure access to the shared buffer. If unavailable, wait.
	lock_guard<mutex> addrem(m_sb);
	int myIndex = -1;
	auto it = mapPhone.find(id);
	if (it == mapPhone.end()) { return 0; }
	else { myIndex = it->second; }
	m_calls[myIndex - 1].lock();
	return 1;
}
