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
    JFUNC jf;
    vector<string> domains = { ".com", ".net", ".org", ".edu", ".ca" };
    ofstream ERR;
    string error_path = sroot + "\\SCDA Error Log.txt";
	HINTERNET hInt, hConnect, WHOpen;
	string hServer;
	vector<int> numPasses = { 0, 0 };
    static vector<wstring> objects;
    vector<int> tree_pl_int;  // List of integer payload values in the tree.
    vector<string> tree_pl_str;  // List of string payload values in the tree.
    vector<vector<int>> tree_structure;  // Form [node index][ancestor indices, node index as negative, children's indices].

public:
	explicit WINFUNC() {}
	~WINFUNC() {}

    string browse(string);
    static void CALLBACK call(HINTERNET, DWORD_PTR, DWORD, LPVOID, DWORD);
	void clearCache();
	void delete_file(string);
    void download(string url, string filePath);
	void downloadBin(string url, string filePath);
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
        string smessage = jf.timestamper() + " Windows Error #" + to_string(num) + ", from " + func + "\r\n";
        ERR.open(error_path, ofstream::app);
        ERR << smessage << endl;
        ERR.close();
        exit(EXIT_FAILURE);
    }

    template<typename ... Args> string browseHelper(string, Args& ... args) {}
    template<> string browseHelper< >(string url)
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
		HINTERNET hrequest = NULL;
		DWORD bytes_available;
		DWORD bytes_read = 0;
		unsigned char* ubuffer;
		wchar_t* bufferW;
		int size1, size2, num_chars;
		wstring fileW;
		string fileA;
		vector<unsigned char> utf;

		InternetCloseHandle(hConnect);
		InternetCloseHandle(hInt);

		hInt = InternetOpenA(agent.c_str(), INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
		if (hInt)
		{
			InternetStatusCallback = InternetSetStatusCallback(hInt, (INTERNET_STATUS_CALLBACK)call);
			hConnect = InternetConnectA(hInt, server_name.c_str(), INTERNET_DEFAULT_HTTP_PORT, NULL, NULL, INTERNET_SERVICE_HTTP, 0, context);
		}
		else { winerr("internetopen-wf.browse"); }
		if (hConnect)
		{
			hrequest = HttpOpenRequestA(hConnect, NULL, object_name.c_str(), NULL, NULL, NULL, 0, context);
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
					jf.err("internetreadfile-wf.browse");
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

		fileA = jf.utf16to8(fileW);
		if (fileA.size() < 1) { winerr("blank return-wf.browse"); }
		if (fileA == "") { winerr("blank return-wf.browse"); }
		if (!InternetCloseHandle(hrequest)) { winerr("InternetCloseHandle-wf.browseHelper"); }
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
					jf.err("internetreadfile-wf.browse");
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

		fileA = jf.utf16to8(fileW);
		if (fileA.size() < 1) { winerr("blank return-wf.browse"); }
		if (fileA == "") { winerr("blank return-wf.browse"); }
		if (!InternetCloseHandle(hrequest)) { winerr("InternetCloseHandle-wf.browseHelper"); }
		return fileA;
	}

	template<typename ... Args> void getTreeFile(string rootPath, vector<vector<int>>& treeST, vector<string>& treePL, Args& ... args)
	{
		// Recursive function that makes an STPL tree of folder/file paths, starting 
		// from a given root directory. Excludes system return folders and nonfile folders. 
		jf.err("getTreeFile template-wf");
	}
	template<> void getTreeFile<string>(string rootPath, vector<vector<int>>& treeST, vector<string>& treePL, string& search)
	{
		// Add all the folders to the tree.
		treeST.clear();
		treePL.clear();
		treeST.push_back({ 0 });
		treePL.push_back(rootPath);
		getTreeFolder(0, treeST, treePL);
		int numFolder = treeST.size();

		// Add all the searched-for files to the tree. 
		WIN32_FIND_DATAA info;
		DWORD attributes;
		HANDLE hfile = INVALID_HANDLE_VALUE;
		string folderSearch, name, temp; 
		vector<int> iGenealogy;
		int kidIndex;
		for (int ii = 0; ii < numFolder; ii++)
		{
			folderSearch = treePL[ii] + "\\" + search;
			hfile = FindFirstFileA(folderSearch.c_str(), &info);
			if (hfile == INVALID_HANDLE_VALUE) 
			{
				attributes = GetLastError();
				if (attributes == 2) { continue; }
				else { winerr("FindFirstFile-getTreeFolder"); }
			}
			iGenealogy = treeST[ii];
			iGenealogy[iGenealogy.size() - 1] = abs(iGenealogy[iGenealogy.size() - 1]);
			do
			{
				attributes = info.dwFileAttributes;
				if (attributes == FILE_ATTRIBUTE_NORMAL || attributes == FILE_ATTRIBUTE_ARCHIVE)
				{
					kidIndex = treeST.size();
					temp = info.cFileName;
					name = treePL[ii] + "\\" + temp;
					treePL.push_back(name);
					iGenealogy.push_back(-1 * kidIndex);
					treeST.push_back(iGenealogy);
					iGenealogy.pop_back();
					treeST[ii].push_back(kidIndex);
				}
			} while (FindNextFileA(hfile, &info));
		}
		
		// Remove all leafless folders from the tree's child lists.
		int pivot, iParent;
		int count = 1;
		while (count > 0)
		{
			count = 0;
			for (int ii = 0; ii < numFolder; ii++)
			{
				pivot = jf.getPivot(treeST[ii]);
				if (pivot >= treeST[ii].size() - 1)
				{
					if (pivot == 0) { jf.err("Root has no children-wf.getTreeFile"); }
					iParent = treeST[ii][pivot - 1];
					for (int jj = treeST[iParent].size() - 1; jj >= 0; jj--)
					{
						if (treeST[iParent][jj] == ii)
						{
							treeST[iParent].erase(treeST[iParent].begin() + jj);
							count++;
							break;
						}
					}
				}
			}
		}

	}
	template<> void getTreeFile<string, int>(string rootPath, vector<vector<int>>& treeST, vector<string>& treePL, string& search, int& numFolder)
	{
		// Add all the folders to the tree.
		treeST.clear();
		treePL.clear();
		treeST.push_back({ 0 });
		treePL.push_back(rootPath);
		getTreeFolder(0, treeST, treePL);
		numFolder = treeST.size();

		// Add all the searched-for files to the tree. 
		WIN32_FIND_DATAA info;
		DWORD attributes;
		HANDLE hfile = INVALID_HANDLE_VALUE;
		string folderSearch, name, temp;
		vector<int> iGenealogy;
		int kidIndex;
		for (int ii = 0; ii < numFolder; ii++)
		{
			folderSearch = treePL[ii] + "\\" + search;
			hfile = FindFirstFileA(folderSearch.c_str(), &info);
			if (hfile == INVALID_HANDLE_VALUE)
			{
				attributes = GetLastError();
				if (attributes == 2) { continue; }
				else { winerr("FindFirstFile-getTreeFolder"); }
			}
			iGenealogy = treeST[ii];
			iGenealogy[iGenealogy.size() - 1] = abs(iGenealogy[iGenealogy.size() - 1]);
			do
			{
				attributes = info.dwFileAttributes;
				if (attributes == FILE_ATTRIBUTE_NORMAL || attributes == FILE_ATTRIBUTE_ARCHIVE)
				{
					kidIndex = treeST.size();
					temp = info.cFileName;
					name = treePL[ii] + "\\" + temp;
					treePL.push_back(name);
					iGenealogy.push_back(-1 * kidIndex);
					treeST.push_back(iGenealogy);
					iGenealogy.pop_back();
					treeST[ii].push_back(kidIndex);
				}
			} while (FindNextFileA(hfile, &info));
		}

		// Remove all leafless folders from the tree's child lists.
		int pivot, iParent;
		int count = 1;
		while (count > 0)
		{
			count = 0;
			for (int ii = 0; ii < numFolder; ii++)
			{
				pivot = jf.getPivot(treeST[ii]);
				if (pivot >= treeST[ii].size() - 1)
				{
					if (pivot == 0) { jf.err("Root has no children-wf.getTreeFile"); }
					iParent = treeST[ii][pivot - 1];
					for (int jj = treeST[iParent].size() - 1; jj >= 0; jj--)
					{
						if (treeST[iParent][jj] == ii)
						{
							treeST[iParent].erase(treeST[iParent].begin() + jj);
							count++;
							break;
						}
					}
				}
			}
		}

	}

	template<typename ... Args> void getTreeFolder(int myIndex, vector<vector<int>>& treeST, vector<string>& treePL, Args& ... args)
	{
		// Recursive function that makes an STPL tree of folder paths, starting 
		// from a given root directory. Excludes system return folders. 
		jf.err("getTreeFolder template-wf");
	}
	template<> void getTreeFolder< >(int myIndex, vector<vector<int>>& treeST, vector<string>& treePL)
	{
		int kidIndex, pivot;
		vector<int> iGenealogy = treeST[myIndex];
		iGenealogy[iGenealogy.size() - 1] = abs(iGenealogy[iGenealogy.size() - 1]);
		WIN32_FIND_DATAA info;
		DWORD attributes;
		string folderSearch = treePL[myIndex] + "\\*";
		string temp, nameFolder;
		HANDLE hfile = FindFirstFileA(folderSearch.c_str(), &info);
		if (hfile == INVALID_HANDLE_VALUE) { winerr("FindFirstFile-getTreeFolder"); }
		do
		{
			attributes = info.dwFileAttributes;
			if (attributes == FILE_ATTRIBUTE_DIRECTORY)
			{
				nameFolder = info.cFileName;
				if (nameFolder == "." || nameFolder == "..") { continue; }
				kidIndex = treeST.size();
				temp = treePL[myIndex] + "\\" + nameFolder;
				treePL.push_back(temp);
				iGenealogy.push_back(-1 * kidIndex);
				treeST.push_back(iGenealogy);
				iGenealogy.pop_back();
				treeST[myIndex].push_back(kidIndex);
			}
		} while (FindNextFileA(hfile, &info));
	}


};



