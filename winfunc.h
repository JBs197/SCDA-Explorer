#pragma once

#include <string>
#include <vector>
#include <fstream>
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
	HINTERNET hConnect;
	string hServer;
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
    vector<string> get_file_list(string folder_path, string search);
    int get_file_path_number(string, string);
    vector<string> get_folder_list(string, string);
    void makeDir(string);
    void make_tree_local(vector<vector<int>>&, vector<string>&, int, string, int, string);
    void make_tree_local_helper1(vector<vector<int>>&, vector<string>&, vector<int>, string, int, int, int, string);
	void set_error_path(string);
    string urlRedirect(string url);


	// TEMPLATES
    template<typename S> void winerr(S) 
    {
        int bbq = 1;
    }
    template<> void winerr<const char*>(const char* func)
    {
        lock_guard<mutex> lock(m_err);
        DWORD num = GetLastError();
        string smessage = jfwf.timestamper() + " Windows Error #" + to_string(num) + ", from " + func + "\r\n";
        ERR.open(error_path, ofstream::app);
        ERR << smessage << endl;
        ERR.close();
        exit(EXIT_FAILURE);
    }

    template<typename ... Args> string browseHelper(string, Args& ... args) {}
    template<> string browseHelper(string url)
	{
		setlocale(LC_ALL, "en_US.utf8");
		string server_name, object_name;
		size_t cut_here;
		for (int ii = 0; ii < domains.size(); ii++)
		{
			cut_here = url.rfind(domains[ii]);
			if (cut_here < url.size())
			{
				server_name = url.substr(0, cut_here + domains[ii].size());
				object_name = url.substr(cut_here + domains[ii].size(), url.size() - cut_here - domains[ii].size());
				break;
			}
		}
		INTERNET_STATUS_CALLBACK InternetStatusCallback;
		DWORD context = 1;
		BOOL yesno = 0;
		string agent = "browser";
		HINTERNET hint = NULL;
		HINTERNET hconnect = NULL;
		HINTERNET hrequest = NULL;
		DWORD bytes_available;
		DWORD bytes_read = 0;
		unsigned char* ubuffer;
		wchar_t* bufferW;
		int size1, size2, num_chars;
		wstring fileW;
		string fileA;
		vector<unsigned char> utf;

		hint = InternetOpenA(agent.c_str(), INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
		if (hint)
		{
			InternetStatusCallback = InternetSetStatusCallback(hint, (INTERNET_STATUS_CALLBACK)call);
			hconnect = InternetConnectA(hint, server_name.c_str(), INTERNET_DEFAULT_HTTP_PORT, NULL, NULL, INTERNET_SERVICE_HTTP, 0, context);
		}
		else { winerr("internetopen-wf.browse"); }
		if (hconnect)
		{
			hConnect = hconnect;
			hServer = server_name;
			hrequest = HttpOpenRequestA(hconnect, NULL, object_name.c_str(), NULL, NULL, NULL, 0, context);
		}
		else { winerr("internetconnect-wf.browse"); }
		if (hrequest)
		{
			yesno = HttpSendRequest(hrequest, NULL, 0, NULL, 0);
		}
		else { winerr("httpopenrequest-wf.browse"); }
		if (yesno)
		{
			do
			{
				bytes_available = 0;
				InternetQueryDataAvailable(hrequest, &bytes_available, 0, 0);
				ubuffer = new unsigned char[bytes_available];
				if (!InternetReadFile(hrequest, ubuffer, bytes_available, &bytes_read))
				{
					err("internetreadfile-wf.browse");
				}
				for (int ii = 0; ii < bytes_available; ii++)
				{
					fileW.push_back((wchar_t)ubuffer[ii]);
					if (fileW.back() == L'\x00C3' && utf.size() == 0)
					{
						utf.push_back(fileW.back());
					}
					else if (utf.size() > 0)
					{
						utf.push_back(fileW.back());
						if (utf.size() == 4)
						{
							if (utf[0] == 195 && utf[2] == 194)
							{
								fileW[fileW.size() - 4] = utf[3] + 64;
								fileW.erase(fileW.size() - 3, 3);
								utf.clear();
							}
							else
							{
								fileW[fileW.size() - 4] = utf[1] + 64;
								fileW.erase(fileW.size() - 3, 1);
								utf.clear();
							}
						}
					}
				}
				delete[] ubuffer;
			} while (bytes_available > 0);
		}
		else
		{
			winerr("httpsendrequest-wf.browse");
		}

		fileA = jfwf.utf16to8(fileW);
		if (fileA.size() < 1) { winerr("blank return-wf.browse"); }
		if (fileA == "") { winerr("blank return-wf.browse"); }
		return fileA;
	}
	template<> string browseHelper<HINTERNET>(string object_name, HINTERNET& hConnect)
	{
		setlocale(LC_ALL, "en_US.utf8");
		INTERNET_STATUS_CALLBACK InternetStatusCallback;
		DWORD context = 1;
		BOOL yesno = 0;
		DWORD bytes_available;
		DWORD bytes_read = 0;
		DWORD headerIndex = 0;
		DWORD bufQsize = 1000;
		size_t pos1;
		char bufferQuery[1000];
		unsigned char* ubuffer;
		wchar_t* bufferW;
		int size1, size2, num_chars;
		wstring fileW;
		string fileA;
		vector<unsigned char> utf;

		HINTERNET hrequest = HttpOpenRequestA(hConnect, NULL, object_name.c_str(), NULL, NULL, NULL, 0, context);
		InternetStatusCallback = InternetSetStatusCallback(hrequest, (INTERNET_STATUS_CALLBACK)call);
		if (hrequest)
		{
			yesno = HttpSendRequestA(hrequest, NULL, 0, NULL, 0);
		}
		else { winerr("httpopenrequest-wf.browse"); }
		if (yesno)
		{
			yesno = HttpQueryInfoA(hrequest, HTTP_QUERY_CONTENT_LENGTH, bufferQuery, &bufQsize, &headerIndex);
			do
			{
				bytes_available = 0;
				InternetQueryDataAvailable(hrequest, &bytes_available, 0, 0);
				ubuffer = new unsigned char[bytes_available];
				if (!InternetReadFile(hrequest, ubuffer, bytes_available, &bytes_read))
				{
					err("internetreadfile-wf.browse");
				}
				for (int ii = 0; ii < bytes_available; ii++)
				{
					fileW.push_back((wchar_t)ubuffer[ii]);
					if (fileW.back() == L'\x00C3' && utf.size() == 0)
					{
						utf.push_back(fileW.back());
					}
					else if (utf.size() > 0)
					{
						utf.push_back(fileW.back());
						if (utf.size() == 4)
						{
							if (utf[0] == 195 && utf[2] == 194)
							{
								fileW[fileW.size() - 4] = utf[3] + 64;
								fileW.erase(fileW.size() - 3, 3);
								utf.clear();
							}
							else
							{
								fileW[fileW.size() - 4] = utf[1] + 64;
								fileW.erase(fileW.size() - 3, 1);
								if (utf[2] == 195)
								{
									utf.erase(utf.begin(), utf.begin() + 2);
								}
								else if (utf[3] == 195)
								{
									utf.erase(utf.begin(), utf.begin() + 3);
								}
								else { utf.clear(); }
							}
						}
					}
				}
				delete[] ubuffer;
			} while (bytes_available > 0);
		}
		else
		{
			return to_string(GetLastError());
			winerr("httpsendrequest-wf.browse");
		}

		fileA = jfwf.utf16to8(fileW);
		if (fileA.size() < 1) { winerr("blank return-wf.browse"); }
		if (fileA == "") { winerr("blank return-wf.browse"); }
		return fileA;
	}

};



