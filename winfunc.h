#pragma once

#include <string>
#include <vector>
#include <mutex>
#include <codecvt>
#include <windows.h>

using namespace std;

class WINFUNC
{

public:
	string exec_path;  // Full path.
    mutex m_win;
	vector<int> tree_pl_int;  // List of integer payload values in the tree.
	vector<string> tree_pl_str;  // List of string payload values in the tree.
	vector<vector<int>> tree_structure;  // Form [node index][ancestor indices, node index as negative, children's indices].

	void init();
	void make_tree_local(vector<vector<int>>&, vector<string>&, int, string, int, string);
    void make_tree_local_helper1(vector<vector<int>>&, vector<string>&, vector<int>, string, int, int, int, string);

public:
	explicit WINFUNC() { init(); }
	~WINFUNC() {}

    vector<string> sroots = { "F:", "D:" };
    int location = 0;

    vector<string> get_file_list(string, string);
    vector<string> get_folder_list(string, string);
    wstring utf8to16(string);
    string utf16to8(wstring);


	// TEMPLATES
    template<typename S> void winerr(S) {}
    template<> void winerr<string>(string message)
    {
        DWORD num = GetLastError();
        string spath = sroots[location] + "\\WINFUNC Error Log.txt";
        string smessage = " Windows Error #" + to_string(num) + ", from " + message + "\r\n\r\n";
        //lock_guard lock(m_win);
        HANDLE hprinter = CreateFileA(spath.c_str(), (GENERIC_READ | GENERIC_WRITE), (FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE), NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        SetFilePointer(hprinter, NULL, NULL, FILE_END);
        DWORD bytes;
        DWORD fsize = (DWORD)smessage.size();
        WriteFile(hprinter, smessage.c_str(), fsize, &bytes, NULL);
        if (hprinter)
        {
            CloseHandle(hprinter);
        }
        exit(EXIT_FAILURE);
    }
    /*
    template<> void winerr<wstring>(wstring message)
    {
        DWORD num = GetLastError();
        string spath = sroots[location] + "\\WINFUNC Error Log.txt";
        string smessage = timestamperA() + " Windows Error #" + to_string(num) + ", from " + utf16to8(message) + "\r\n\r\n";
        lock_guard lock(m_win);
        HANDLE hprinter = CreateFileA(spath.c_str(), (GENERIC_READ | GENERIC_WRITE), (FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE), NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        SetFilePointer(hprinter, NULL, NULL, FILE_END);
        DWORD bytes;
        DWORD fsize = (DWORD)smessage.size();
        WriteFile(hprinter, smessage.c_str(), fsize, &bytes, NULL);
        if (hprinter)
        {
            CloseHandle(hprinter);
        }
        exit(EXIT_FAILURE);
    }
    */


};



