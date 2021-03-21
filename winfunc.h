#pragma once

#include <string>
#include <vector>
#include <mutex>
#include <codecvt>
#include <windows.h>
#include "jfunc.h"

using namespace std;

class WINFUNC
{
    JFUNC jfwin;
    ofstream ERR;
    string error_path = root + "\\SCDA Error Log.txt";
    vector<int> tree_pl_int;  // List of integer payload values in the tree.
    vector<string> tree_pl_str;  // List of string payload values in the tree.
    vector<vector<int>> tree_structure;  // Form [node index][ancestor indices, node index as negative, children's indices].

    void err(string);

public:
	explicit WINFUNC() {}
	~WINFUNC() {}

    string get_exec_dir();
    string get_exec_path();
    vector<string> get_file_list(string, string);
    vector<string> get_folder_list(string, string);
    void make_tree_local(vector<vector<int>>&, vector<string>&, int, string, int, string);
    void make_tree_local_helper1(vector<vector<int>>&, vector<string>&, vector<int>, string, int, int, int, string);
    void set_error_path(string);
    wstring utf8to16(string);
    string utf16to8(wstring);
    string timestamper();

	// TEMPLATES
    template<typename S> void winerr(S) {}
    template<> void winerr<string>(string func)
    {
        lock_guard<mutex> lock(m_err);
        DWORD num = GetLastError();
        string smessage = timestamper() + " Windows Error #" + to_string(num) + ", from " + func + "\r\n";
        ERR.open(error_path, ofstream::app);
        ERR << smessage << endl;
        ERR.close();
        exit(EXIT_FAILURE);
    }

};



