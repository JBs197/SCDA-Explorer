#pragma once

#include <string>
#include <vector>
#include <fstream>
#include <mutex>
#include <codecvt>
#include <windows.h>
#include <winhttp.h>
#include "jfunc.h"
#pragma comment(lib, "winhttp.lib")

using namespace std;

class WINFUNC
{
    JFUNC jf;
    vector<string> domains = { ".com", ".net", ".org", ".edu", ".ca" };
    string error_path = sroot + "\\SCDA Error Log.txt";
    static vector<wstring> objects;
    vector<int> tree_pl_int;  // List of integer payload values in the tree.
    vector<string> tree_pl_str;  // List of string payload values in the tree.
    vector<vector<int>> tree_structure;  // Form [node index][ancestor indices, node index as negative, children's indices].

public:
	WINFUNC() {}
	~WINFUNC() {}

    string browseA(string url);
	string browseS(string url);
	vector<unsigned char> browseUC(string url);
	wstring browseW(string url);
    //static void CALLBACK call(HINTERNET, DWORD_PTR, DWORD, LPVOID, DWORD);
	void delete_file(string);
    void download(string url, string filePath);
    bool file_exist(string);
    string get_exec_dir();
    string get_exec_path();
	vector<string> getFileList(string folder_path, string search);
    vector<string> get_file_list(string folder_path, string search);
    int get_file_path_number(string folder_path, string file_extension);
    vector<string> get_folder_list(string folder_path, string search);
	string load(string filePath);
    void makeDir(string);
    void make_tree_local(vector<vector<int>>&, vector<string>&, int, string, int, string);
    void make_tree_local_helper1(vector<vector<int>>&, vector<string>&, vector<int>, string, int, int, int, string);
	void printer(string path, string& file);
	void renameFile(string oldPath, string newPath);
	void set_error_path(string);
    //string urlRedirect(string url);


	// TEMPLATES
    template<typename S> void winerr(S) 
    {
        int bbq = 1;
    }
    template<> void winerr<const char*>(const char* func)
    {
        lock_guard<mutex> lock(m_err);
		ofstream ERR;
        DWORD num = GetLastError();
        string smessage = jf.timestamper() + " Windows Error #" + to_string(num) + ", from " + func + "\r\n";
        ERR.open(error_path, ofstream::app);
        ERR << smessage << endl;
        ERR.close();
        exit(EXIT_FAILURE);
    }

	template<typename ... Args> void getLayerFolder(int myIndex, vector<vector<int>>& treeST, vector<string>& treePL, Args& ... args)
	{
		// From a given root directory, adds all folders there to the STPL tree. Excludes system return folders. 
		jf.err("getTreeFolder template-wf");
	}
	template<> void getLayerFolder< >(int myIndex, vector<vector<int>>& treeST, vector<string>& treePL)
	{
		int kidIndex, pivot;
		vector<int> iGenealogy = treeST[myIndex], iKids;
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
	template<> void getLayerFolder<string>(int myIndex, vector<vector<int>>& treeST, vector<string>& treePL, string& search)
	{
		int kidIndex, pivot;
		vector<int> iGenealogy = treeST[myIndex], iKids;
		iGenealogy[iGenealogy.size() - 1] = abs(iGenealogy[iGenealogy.size() - 1]);
		WIN32_FIND_DATAA info;
		DWORD attributes;
		string folderSearch = treePL[myIndex] + "\\" + search;
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
		// Recursive function, begins with root/branch already present in the tree vectors.
		int kidIndex, pivot;
		vector<int> iGenealogy = treeST[myIndex], iKids;
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
		
		iGenealogy = treeST[myIndex];
		pivot = jf.getPivot(iGenealogy);
		if (pivot < 0) { jf.err("No pivot found-wf.getTreeFolder"); }
		else if (pivot == iGenealogy.size() - 1) { return; }
		iKids.assign(iGenealogy.begin() + pivot + 1, iGenealogy.end());
		for (int ii = 0; ii < iKids.size(); ii++)
		{
			getTreeFolder(iKids[ii], treeST, treePL);
		}
	}

};



