#pragma once

#include <string>
#include <vector>
#include <mutex>
#include <codecvt>
#include <windows.h>
#include <wininet.h>
#include "jfunc.h"
#pragma comment(lib, "wininet.lib")

using namespace std;


class WINFUNC
{
    JFUNC jfwf;
    vector<string> domains = { ".com", ".net", ".org", ".edu", ".ca" };
    ofstream ERR;
    string error_path = sroot + "\\SCDA Error Log.txt";
    static vector<wstring> objects;
    vector<int> tree_pl_int;  // List of integer payload values in the tree.
    vector<string> tree_pl_str;  // List of string payload values in the tree.
    vector<vector<int>> tree_structure;  // Form [node index][ancestor indices, node index as negative, children's indices].

    void err(string);

public:
	explicit WINFUNC() {}
	~WINFUNC() {}

    string browse(string);
    static void CALLBACK call(HINTERNET, DWORD_PTR, DWORD, LPVOID, DWORD);
    void delete_file(string);
    int download(string, string);
    bool file_exist(string);
    string get_exec_dir();
    string get_exec_path();
    vector<string> get_file_list(string, string);
    int get_file_path_number(string, string);
    vector<string> get_folder_list(string, string);
    void make_tree_local(vector<vector<int>>&, vector<string>&, int, string, int, string);
    void make_tree_local_helper1(vector<vector<int>>&, vector<string>&, vector<int>, string, int, int, int, string);
    void set_error_path(string);


	// TEMPLATES
    template<typename S> void winerr(S) {}
    template<> void winerr<string>(string func)
    {
        lock_guard<mutex> lock(m_err);
        DWORD num = GetLastError();
        string smessage = jfwf.timestamper() + " Windows Error #" + to_string(num) + ", from " + func + "\r\n";
        ERR.open(error_path, ofstream::app);
        ERR << smessage << endl;
        ERR.close();
        exit(EXIT_FAILURE);
    }

};



