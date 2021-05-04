#include "winfunc.h"

using namespace std;
vector<wstring> WINFUNC::objects;

typedef struct {
	HWND       hWindow;
	int        nStatusList;
	HINTERNET hresource;
	char szMemo[512];
} REQUEST_CONTEXT;
void WINFUNC::call(HINTERNET hint, DWORD_PTR dw_context, DWORD dwInternetStatus, LPVOID status_info, DWORD status_info_length)
{
	REQUEST_CONTEXT* cpContext;
	cpContext = (REQUEST_CONTEXT*)status_info;
	string temp;
	wstring wtemp;
	bool yesno = 0;
	int count = 0;

	switch (dwInternetStatus)
	{
	case INTERNET_STATUS_CLOSING_CONNECTION:

		break;
	case INTERNET_STATUS_CONNECTED_TO_SERVER:

		break;
	case INTERNET_STATUS_CONNECTING_TO_SERVER:

		break;
	case INTERNET_STATUS_CONNECTION_CLOSED:

		break;
	case INTERNET_STATUS_HANDLE_CLOSING:

		break;
	case INTERNET_STATUS_HANDLE_CREATED:

		break;
	case INTERNET_STATUS_INTERMEDIATE_RESPONSE:

		break;
	case INTERNET_STATUS_NAME_RESOLVED:

		break;
	case INTERNET_STATUS_RECEIVING_RESPONSE:

		break;
	case INTERNET_STATUS_RESPONSE_RECEIVED:

		break;
	case INTERNET_STATUS_REDIRECT:
		for (int ii = 0; ii < 512; ii++)
		{
			if ((cpContext->szMemo)[ii] != '\0') { temp.push_back((cpContext->szMemo)[ii]); count = 0; }
			else { count++; }

			if (count > 2) { break; }
		}
		for (int ii = 0; ii < temp.size(); ii++)
		{
			if (temp[ii] == '/') { yesno = 1; }
			else if (temp[ii] < 0)
			{
				break;
			}
			if (yesno)
			{
				wtemp.push_back((wchar_t)temp[ii]);
			}
		}
		objects.push_back(wtemp);
		break;
	case INTERNET_STATUS_REQUEST_COMPLETE:

		break;
	case INTERNET_STATUS_REQUEST_SENT:

		break;
	case INTERNET_STATUS_RESOLVING_NAME:

		break;
	case INTERNET_STATUS_SENDING_REQUEST:

		break;
	case INTERNET_STATUS_STATE_CHANGE:

		break;
	}

}

string WINFUNC::browse(string url)
{
	string server_name, object_name, sPage;
	string oddError = "12002";
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

	if (server_name == hServer)
	{
		sPage = browseHelper(object_name, hConnect);
	}
	else
	{
		sPage = browseHelper(url);
	}

	return sPage;
}
void WINFUNC::delete_file(string path)
{
	HANDLE hfile = CreateFileA(path.c_str(), (GENERIC_READ | GENERIC_WRITE), (FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE), NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hfile == INVALID_HANDLE_VALUE) { winerr("CreateFile-wf.delete_file"); }
	if (!DeleteFileA(path.c_str())) { winerr("DeleteFile-wf.delete_file"); }
	if (!CloseHandle(hfile)) { winerr("CloseHandle-wf.delete_file"); }
}
int WINFUNC::download(string url, string filepath)
{
	string server_name;
	string object_name;
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
	string agent = "downloader";
	HINTERNET hint = NULL;
	HINTERNET hconnect = NULL;
	HINTERNET hrequest = NULL;
	DWORD bytes_available;
	DWORD bytes_read = 0;
	char* bufferA;
	unsigned char* ubufferA;
	int size1, size2;
	wstring fileW;
	string sfile;
	bool special_char = 0;
	bool saveBinary = 0;
	ofstream PRINTER;
	wofstream WPRINTER;

	size_t pos1 = filepath.rfind(".pdf");
	if (pos1 < filepath.size()) { saveBinary = 1; }

	hint = InternetOpenA(agent.c_str(), INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
	if (hint)
	{
		InternetStatusCallback = InternetSetStatusCallback(hint, (INTERNET_STATUS_CALLBACK)call);
		hconnect = InternetConnectA(hint, server_name.c_str(), INTERNET_DEFAULT_HTTP_PORT, NULL, NULL, INTERNET_SERVICE_HTTP, 0, context);
	}
	else { return 1; }
	if (hconnect)
	{
		hrequest = HttpOpenRequestA(hconnect, NULL, object_name.c_str(), NULL, NULL, NULL, 0, context);
	}
	else { return 2; }
	if (hrequest)
	{
		yesno = HttpSendRequest(hrequest, NULL, 0, NULL, 0);
	}
	else { return 3; }
	if (yesno)
	{
		if (saveBinary)
		{
			PRINTER.open(filepath.c_str(), ios::binary | ios::trunc);
			auto report = PRINTER.rdstate();
			bool Fail = PRINTER.fail();
			DWORD dwErr = GetLastError();
			if (Fail == 1 && dwErr == 1113)
			{
				PRINTER.close();
				wstring wfilepath = jfwf.utf8to16(filepath);
				WPRINTER.open(wfilepath.c_str(), wios::binary | wios::trunc);
				report = WPRINTER.rdstate();
				Fail = WPRINTER.fail();
				do
				{
					bytes_available = 0;
					InternetQueryDataAvailable(hrequest, &bytes_available, 0, 0);
					ubufferA = new unsigned char[bytes_available];
					auto bufferW = new wchar_t[bytes_available];
					if (!InternetReadFile(hrequest, ubufferA, bytes_available, &bytes_read))
					{
						return 4;
					}
					for (int ii = 0; ii < bytes_available; ii++)
					{
						bufferW[ii] = (wchar_t)ubufferA[ii];
					}
					WPRINTER.write(bufferW, bytes_available);
					WPRINTER.flush();
					delete[] ubufferA;
					delete[] bufferW;
				} while (bytes_available > 0);
				WPRINTER.close();
			}
			else
			{
				do
				{
					bytes_available = 0;
					InternetQueryDataAvailable(hrequest, &bytes_available, 0, 0);
					bufferA = new char[bytes_available];
					if (!InternetReadFile(hrequest, bufferA, bytes_available, &bytes_read))
					{
						return 4;
					}
					PRINTER.write(bufferA, bytes_available);
					PRINTER.flush();
					delete[] bufferA;
				} while (bytes_available > 0);
				PRINTER.close();
			}
		}
		else
		{
			do
			{
				bytes_available = 0;
				InternetQueryDataAvailable(hrequest, &bytes_available, 0, 0);
				ubufferA = new unsigned char[bytes_available];
				if (!InternetReadFile(hrequest, ubufferA, bytes_available, &bytes_read))
				{
					return 4;
				}
				for (int ii = 0; ii < bytes_available; ii++)
				{
					fileW.push_back((wchar_t)ubufferA[ii]);
					if (fileW.back() == L'\x00C3' && !special_char)
					{
						special_char = 1;
					}
					else if (special_char)
					{
						fileW[fileW.size() - 2] = fileW[fileW.size() - 1] + 64;
						fileW.pop_back();
						special_char = 0;
					}
				}
			} while (bytes_available > 0);
			delete[] ubufferA;
			sfile = jfwf.utf16to8(fileW);
			HANDLE hprinter = CreateFileA(filepath.c_str(), (GENERIC_READ | GENERIC_WRITE), (FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE), NULL, CREATE_ALWAYS, 0, NULL);
			if (hprinter == INVALID_HANDLE_VALUE) { return 6; }
			DWORD bytes_written;
			DWORD file_size = sfile.size();
			if (!WriteFile(hprinter, sfile.c_str(), file_size, &bytes_written, NULL)) { return 7; }
		}
	}
	else { return 5; }

	if (hrequest) { InternetCloseHandle(hrequest); }
	if (hconnect) { InternetCloseHandle(hconnect); }
	if (hint) { InternetCloseHandle(hint); }
	return 0;
}
void WINFUNC::err(string func)
{
	jfwf.err(func);
}
bool WINFUNC::file_exist(string path)
{
	DWORD attributes = GetFileAttributesA(path.c_str());
	if (attributes == INVALID_FILE_ATTRIBUTES)
	{
		return 0;
	}
	return 1;
}
string WINFUNC::get_exec_dir()
{
	string exec_path;
	LPSTR bufferA = new CHAR[500];
	DWORD slength = GetModuleFileNameA(NULL, bufferA, 500);
	if (slength == 0) { winerr("GetModuleFileName-wf.get_exec_path"); }
	exec_path.assign(bufferA, slength);
	delete[] bufferA;
	size_t pos1 = exec_path.rfind('\\');
	string exec_dir = exec_path.substr(0, pos1);
	return exec_dir;
}
string WINFUNC::get_exec_path()
{
	string exec_path;
	LPSTR bufferA = new CHAR[500];
	DWORD slength = GetModuleFileNameA(NULL, bufferA, 500);
	if (slength == 0) { winerr("GetModuleFileName-wf.get_exec_path"); }
	exec_path.assign(bufferA, slength);
	delete[] bufferA;
	return exec_path;
}
vector<string> WINFUNC::get_file_list(string folder_path, string search)
{
	// Returns a list of file names (not full paths) within a given folder, and
	// subject to the search criteria.

	vector<string> file_list;
	string folder_search = folder_path + "\\" + search;
	WIN32_FIND_DATAA info;
	DWORD attributes, GLE;
	HANDLE hfile = FindFirstFileA(folder_search.c_str(), &info);
	if (hfile == INVALID_HANDLE_VALUE) 
	{
		GLE = GetLastError();
		if (GLE == 2) { return file_list; }
		else { winerr("FindFirstFile-get_file_list"); }		 
	}
	do
	{
		attributes = info.dwFileAttributes;
		if (attributes == FILE_ATTRIBUTE_NORMAL || attributes == FILE_ATTRIBUTE_ARCHIVE)
		{
			file_list.push_back(info.cFileName);
		}
	} while (FindNextFileA(hfile, &info));
	return file_list;
}
int WINFUNC::get_file_path_number(string folder_path, string file_extension)
{
	int count = 0;
	string temp = folder_path + "\\*" + file_extension;
	wstring folder_search = jfwf.utf8to16(temp);
	WIN32_FIND_DATAW info;
	HANDLE hfile1 = FindFirstFileW(folder_search.c_str(), &info);
	//if (hfile1 == INVALID_HANDLE_VALUE) { winerr("FindFirstFile-get_file_path_number"); }
	if (hfile1 == (HANDLE)-1)
	{
		DWORD gle = GetLastError();
		if (gle == 2 || gle == 3) { return 0; }
		else { winerr("FindFirstFile-get_file_path_number"); }
	}
	do
	{
		count++;
	} while (FindNextFileW(hfile1, &info));

	if (!FindClose(hfile1)) { winerr("FindClose-get_file_path_number"); }
	return count;
}
vector<string> WINFUNC::get_folder_list(string folder_path, string search)
{
	// Returns a list of folder names (not full paths) within a given folder, and
	// subject to the search criteria. EXCLUDES the return system folders. 

	vector<string> folder_list;
	string folder_search = folder_path + "\\" + search;
	WIN32_FIND_DATAA info;
	DWORD attributes;
	string temp1;
	HANDLE hfile = FindFirstFileA(folder_search.c_str(), &info);
	if (hfile == INVALID_HANDLE_VALUE) { winerr("FindFirstFile-get_folder_list"); }
	do
	{
		attributes = info.dwFileAttributes;
		if (attributes == FILE_ATTRIBUTE_DIRECTORY)
		{
			temp1 = info.cFileName;
			if (temp1 == "." || temp1 == "..") { continue; }
			folder_list.push_back(temp1);
		}
	} while (FindNextFileA(hfile, &info));
	return folder_list;
}
void WINFUNC::makeDir(string dirPath)
{
	vector<int> vBslash;
	string spath;
	DWORD gle;
	BOOL success;
	size_t pos1 = dirPath.find('\\');
	while (pos1 < dirPath.size())
	{
		vBslash.push_back((int)pos1);
		pos1 = dirPath.find('\\', pos1 + 1);
	}
	for (int ii = 0; ii < vBslash.size(); ii++)
	{
		if (ii == vBslash.size() - 1)
		{
			spath = dirPath;
		}
		else
		{
			spath = dirPath.substr(0, vBslash[ii + 1]);
		}
		success = CreateDirectoryA(spath.c_str(), NULL);
		if (!success)
		{
			gle = GetLastError();
			if (gle != ERROR_ALREADY_EXISTS) { winerr("CreateDirectory-wf.makeDir"); }
		}
	}
}
int WINFUNC::makeTreeLocal(vector<vector<int>>& treeST, vector<string>& treePL, vector<int>& treeiPL, string rootDir, string search)
{
	// Populate the given tree structure and payload, by searching a local drive.
	// Returns the total number of tree leaves. iPL is 0 for folder, 1 for file. 

	// Add the root directory to the tree.
	treePL.resize(1);
	treePL[0] = rootDir;
	treeST.clear();
	treeST.push_back({ 0 });
	treeiPL.clear();
	treeiPL.push_back(0);

	// Recursively search a given tree element for leaves (files matching search criteria) 
	// and for branches (subfolders). For every branch, repeat. 
	int countLeaf = makeTreeLocalHelper(treeST, treePL, treeiPL, search, 0);

	return countLeaf;
}
int WINFUNC::makeTreeLocalHelper(vector<vector<int>>& treeST, vector<string>& treePL, vector<int>& treeiPL, string search, int myIndex)
{
	// Get the pivot.
	int countLeaf = 0;
	int inum = -1;
	int pivot;
	for (int ii = 0; ii < treeST[myIndex].size(); ii++)
	{
		if (treeST[myIndex][ii] < 0)
		{
			pivot = ii;
			break;
		}
		else if (treeST[myIndex][ii] == 0)
		{
			inum = ii;
		}

		if (ii == treeST[myIndex].size() - 1) 
		{
			if (inum >= 0) { pivot = inum; }
			else { err("Cannot determine pivot-wf.makeTreeLocalHelper"); }
		}
	}

	// Get the local directory.
	vector<int> iGenealogy;
	for (int ii = 0; ii <= pivot; ii++)
	{
		iGenealogy.push_back(abs(treeST[myIndex][ii]));
	}
	string myDir;
	for (int ii = 0; ii < iGenealogy.size(); ii++)
	{
		myDir += treePL[iGenealogy[ii]] + "\\";
	}
	string searchFolder = myDir + "*";
	myDir.pop_back();

	// Add this directory's subfolders to the tree.
	WIN32_FIND_DATAA info;
	DWORD attributes;
	string temp1;
	vector<int> subfolders, viTemp;
	int indexPL;
	HANDLE hfind = FindFirstFileA(searchFolder.c_str(), &info);
	if (hfind == INVALID_HANDLE_VALUE) { err("FindFirstFile-wf.makeTreeLocalHelper"); }
	do
	{
		attributes = info.dwFileAttributes;
		if (attributes == FILE_ATTRIBUTE_DIRECTORY)
		{
			temp1 = info.cFileName;
			if (temp1 == "." || temp1 == "..") { continue; }
			indexPL = treePL.size();
			treePL.push_back(temp1);
			subfolders.push_back(indexPL);
			viTemp = iGenealogy;
			viTemp.push_back(-1 * indexPL);
			treeST.push_back(viTemp);
			treeiPL.push_back(0);  // Folder.
			treeST[myIndex].push_back(indexPL);  // Add subfolder to this node's list of kids.
		}
	} while (FindNextFileA(hfind, &info));

	// Add this directory's files (matching search criteria) to the tree.
	searchFolder = myDir + "\\" + search;
	hfind = FindFirstFileA(searchFolder.c_str(), &info);
	if (hfind == INVALID_HANDLE_VALUE) 
	{
		attributes = GetLastError();
		if (attributes == 2) { goto FTL1; }
		else { err("FindFirstFile-wf.makeTreeLocalHelper"); }
	}
	do
	{
		attributes = info.dwFileAttributes;
		if (attributes == FILE_ATTRIBUTE_ARCHIVE || attributes == FILE_ATTRIBUTE_NORMAL)
		{
			temp1 = info.cFileName;
			indexPL = treePL.size();
			treePL.push_back(temp1);
			viTemp = iGenealogy;
			viTemp.push_back(-1 * indexPL);
			treeST.push_back(viTemp);
			treeiPL.push_back(1);  // File.
			treeST[myIndex].push_back(indexPL);  // Add file to this node's list of kids.
			countLeaf++;
		}
	} while (FindNextFileA(hfind, &info));

FTL1:
	// RESUME HERE. For every subfolder, recurse.

	return countLeaf;
}
void WINFUNC::make_tree_local(vector<vector<int>>& tree_st, vector<string>& tree_pl, int mode, string root_dir, int depth, string search)
{
	// Populate the given tree structure and payload, by searching a local drive.
	// Modes:  0 = file search, 1 = folder search.

	vector<vector<int>> subfolder_structure;
	vector<string> subfolder_pl_names;
	int pl_index;
	WIN32_FIND_DATAA info;
	DWORD attributes;
	string folder_search, subfolder_name, temp1;
	HANDLE hfind = INVALID_HANDLE_VALUE;
	vector<int> genealogy = { 0 };
	vector<int> new_genealogy;

	// Add the root directory to the tree.
	tree_pl.resize(1);
	tree_pl[0] = root_dir;
	pl_index = 0;
	tree_st.clear();
	tree_st.push_back({ 0 });

	if (depth == 1)
	{
		switch (mode)
		{
		case 0:
			folder_search = root_dir + "\\" + search;
			hfind = FindFirstFileA(folder_search.c_str(), &info);
			if (hfind == INVALID_HANDLE_VALUE) { winerr("FindFirstFile1-make_tree_local"); }
			do
			{
				attributes = info.dwFileAttributes;
				if (attributes == FILE_ATTRIBUTE_NORMAL || attributes == FILE_ATTRIBUTE_ARCHIVE)
				{
					temp1 = root_dir + "\\" + info.cFileName;
					pl_index = tree_pl.size();
					tree_pl.push_back(temp1);
					tree_st.push_back(vector<int>(1));
					tree_st[pl_index][0] = -1 * pl_index;
				}
			} while (FindNextFileA(hfind, &info));
			break;

		case 1:
			folder_search = root_dir + "\\" + search;
			hfind = FindFirstFileA(folder_search.c_str(), &info);
			if (hfind == INVALID_HANDLE_VALUE) { winerr("FindFirstFile2-make_tree_local"); }
			do
			{
				attributes = info.dwFileAttributes;
				if (attributes == FILE_ATTRIBUTE_DIRECTORY)
				{
					temp1 = info.cFileName;
					if (temp1 == "." || temp1 == "..") { continue; }
					subfolder_name = root_dir + "\\" + info.cFileName;
					pl_index = tree_pl.size();
					tree_pl.push_back(subfolder_name);
					tree_st.push_back(vector<int>(1));
					tree_st[pl_index][0] = -1 * pl_index;
				}
			} while (FindNextFileA(hfind, &info));
			break;
		}
	}
	else if (depth > 1)
	{
		// Determine the root directory's subfolders, and add them to the tree.
		folder_search = root_dir + "\\*";
		hfind = FindFirstFileA(folder_search.c_str(), &info);
		if (hfind == INVALID_HANDLE_VALUE) { winerr("FindFirstFile3-make_tree_local"); }
		do
		{
			attributes = info.dwFileAttributes;
			if (attributes == FILE_ATTRIBUTE_DIRECTORY)
			{
				temp1 = info.cFileName;
				if (temp1 == "." || temp1 == "..") { continue; }
				subfolder_pl_names.push_back(temp1);
				pl_index = tree_pl.size();
				tree_pl.push_back(temp1);
				tree_st.push_back({ 0, -1 * pl_index });
				tree_st[0].push_back(pl_index);
			}
		} while (FindNextFileA(hfind, &info));
		
		for (int ii = 0; ii < subfolder_pl_names.size(); ii++)
		{
			new_genealogy = genealogy;
			new_genealogy.push_back(ii + 1);
			temp1 = root_dir + "\\" + subfolder_pl_names[ii];
			make_tree_local_helper1(tree_st, tree_pl, new_genealogy, temp1, mode, 2, depth, search);
		}
	}
	else { winerr("depth error-make_tree_local"); }
}
void WINFUNC::make_tree_local_helper1(vector<vector<int>>& tree_st, vector<string>& tree_pl, vector<int> genealogy, string folder_path, int mode, int my_depth, int target_depth, string search)
{
	string folder_search, temp1;
	vector<string> subfolder_pl_names;
	vector<int> subfolder_pl_indices;
	int iparent = genealogy[genealogy.size() - 1];
	int pl_index, inum;
	size_t pos1;
	HANDLE hfind = INVALID_HANDLE_VALUE;
	WIN32_FIND_DATAA info;
	DWORD attributes;
	vector<int> new_genealogy;

	if (my_depth < target_depth)
	{
		// Determine this folder's subfolders, and add them to the tree (no children). 
		// Add this node's children to its tree_st.
		folder_search = folder_path + "\\*";
		hfind = FindFirstFileA(folder_search.c_str(), &info);
		if (hfind == INVALID_HANDLE_VALUE) { winerr("FindFirstFile3-make_tree_local_helper1"); }
		do
		{
			attributes = info.dwFileAttributes;
			if (attributes == FILE_ATTRIBUTE_DIRECTORY)
			{
				temp1 = info.cFileName;
				if (temp1 == "." || temp1 == "..") { continue; }
				pl_index = tree_pl.size();
				subfolder_pl_names.push_back(temp1);
				subfolder_pl_indices.push_back(pl_index);
				tree_pl.push_back(temp1);
				tree_st.push_back(genealogy);
				tree_st[pl_index].push_back(-1 * pl_index);				
				inum = genealogy[genealogy.size() - 1];  // Parent index.
				tree_st[inum].push_back(pl_index);
			}
		} while (FindNextFileA(hfind, &info));
		
		// Launch this function again, once for each subfolder.
		for (int ii = 0; ii < subfolder_pl_names.size(); ii++)
		{
			new_genealogy = genealogy;
			new_genealogy.push_back(subfolder_pl_indices[ii]);
			temp1 = folder_path + "\\" + subfolder_pl_names[ii];
			make_tree_local_helper1(tree_st, tree_pl, new_genealogy, temp1, mode, my_depth + 1, target_depth, search);
		}
	}
	else
	{
		switch (mode)
		{
		case 0:
			folder_search = folder_path + "\\" + search;
			hfind = FindFirstFileA(folder_search.c_str(), &info);
			if (hfind == INVALID_HANDLE_VALUE) { winerr("FindFirstFile1-make_tree_local_helper1"); }
			do
			{
				attributes = info.dwFileAttributes;
				if (attributes == FILE_ATTRIBUTE_NORMAL || attributes == FILE_ATTRIBUTE_ARCHIVE)
				{
					temp1 = info.cFileName;
					pl_index = tree_pl.size();
					tree_pl.push_back(temp1);
					tree_st.push_back(genealogy);
					tree_st[pl_index].push_back(-1 * pl_index);
					inum = tree_st[pl_index][tree_st[pl_index].size() - 2];  // Parent index.
					tree_st[inum].push_back(pl_index);  // Add this file as the parent's child.
				}
			} while (FindNextFileA(hfind, &info));
			break;

		case 1:
			folder_search = folder_path + "\\" + search;
			hfind = FindFirstFileA(folder_search.c_str(), &info);
			if (hfind == INVALID_HANDLE_VALUE) { winerr("FindFirstFile2-make_tree_local_helper1"); }
			do
			{
				attributes = info.dwFileAttributes;
				if (attributes == FILE_ATTRIBUTE_DIRECTORY)
				{
					temp1 = info.cFileName;
					if (temp1 == "." || temp1 == "..") { continue; }
					pl_index = tree_pl.size();
					tree_pl.push_back(temp1);
					tree_st.push_back(genealogy);
					tree_st[pl_index].push_back(-1 * pl_index);
					inum = tree_st[pl_index][tree_st[pl_index].size() - 2];  // Parent index.
					tree_st[inum].push_back(pl_index);  // Add this file as the parent's child.
				}
			} while (FindNextFileA(hfind, &info));
			break;
		}
	}
}
void WINFUNC::set_error_path(string errpath)
{
	error_path = errpath;
}
string WINFUNC::urlRedirect(string url)
{
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
	string agent = "urlRedirect";
	HINTERNET hint = NULL;
	HINTERNET hconnect = NULL;
	HINTERNET hrequest = NULL;
	wstring object;

	hint = InternetOpenA(agent.c_str(), INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
	if (hint)
	{
		InternetStatusCallback = InternetSetStatusCallback(hint, (INTERNET_STATUS_CALLBACK)call);
		hconnect = InternetConnectA(hint, server_name.c_str(), INTERNET_DEFAULT_HTTP_PORT, NULL, NULL, INTERNET_SERVICE_HTTP, 0, context);
	}
	else { err("internetopen-wf.urlRedirect"); }
	if (hconnect)
	{
		hrequest = HttpOpenRequestA(hconnect, NULL, object_name.c_str(), NULL, NULL, NULL, 0, context);
	}
	else { err("internetconnect-wf.urlRedirect"); }
	if (hrequest)
	{
		yesno = HttpSendRequest(hrequest, NULL, 0, NULL, 0);
	}
	else { err("httpopenrequest-wf.urlRedirect"); }
	if (yesno)
	{
		if (objects.size() > 0)
		{
			object = objects[objects.size() - 1];
			objects.clear();
		}
	}
	else { err("httpsendrequest-wf.urlRedirect"); }
	string newUrl = server_name + jfwf.utf16to8(object);
	return newUrl;
}

