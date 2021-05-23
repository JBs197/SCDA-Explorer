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
void WINFUNC::clearCache()
{
	INTERNET_CACHE_ENTRY_INFOA infoCache;
	DWORD sizeCache;
	FindFirstUrlCacheEntryA(NULL, &infoCache, &sizeCache);
	int bbq = 1;
}
string WINFUNC::browse(string url)
{
	string server_name, object_name, sPage, temp;
	string oddError = "12002";
	size_t cut_here;
	temp = url;
	cut_here = temp.find("www");
	if (cut_here > 0) { url = temp.substr(cut_here); }
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
		numPasses[0]++;
		if (sPage == "12002") 
		{
			//clearCache();
			jf.err("12002-wf.browse"); 
		}
	}
	else
	{
		hServer = server_name;
		sPage = browseHelper(url);
		numPasses[1]++;
		if (sPage == "12002") { jf.err("12002-wf.browse"); }
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
void WINFUNC::download(string url, string filepath)
{
	// UTF-8 encoding is used to write the downloaded file.

	INTERNET_STATUS_CALLBACK InternetStatusCallback;
	DWORD context = 1, bytes_available, bytes_read;
	string server_name, object_name;
	wstring wfile;
	size_t cut_here, indexWfile = 0;
	unsigned char* ubuffer;

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

	if (!hInt)
	{
		hInt = InternetOpenA("wf.download", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
		if (!hInt) { winerr("InternetOpenA-wf.download"); }
		InternetStatusCallback = InternetSetStatusCallback(hInt, (INTERNET_STATUS_CALLBACK)call);
		hConnect = InternetConnectA(hInt, server_name.c_str(), INTERNET_DEFAULT_HTTP_PORT, NULL, NULL, INTERNET_SERVICE_HTTP, 0, context);
		if (!hConnect) { winerr("InternetConnectA-wf.download"); }
	}
	HINTERNET hRequest = HttpOpenRequestA(hConnect, NULL, object_name.c_str(), NULL, NULL, NULL, INTERNET_FLAG_NO_CACHE_WRITE, context);
	if (!hRequest) { winerr("HttpOpenRequestA-wf.download"); }
	BOOL yesno = HttpSendRequestA(hRequest, NULL, 0, NULL, 0);
	if (!yesno) { winerr("HttpSendRequestA-wf.download"); }
	do
	{
		bytes_available = 0;
		InternetQueryDataAvailable(hRequest, &bytes_available, 0, 0);
		ubuffer = new unsigned char[bytes_available];
		yesno = InternetReadFile(hRequest, ubuffer, bytes_available, &bytes_read);
		if (!yesno) { winerr("InternetReadFile-wf.download"); }
		wfile.resize(wfile.size() + bytes_available);
		for (int ii = 0; ii < bytes_available; ii++)
		{
			wfile[indexWfile] = (wchar_t)ubuffer[ii];
			indexWfile++;
		}
		delete[] ubuffer;
	} while (bytes_available > 0);
	if (hRequest) { InternetCloseHandle(hRequest); }
	jf.printer(filepath, wfile);
}
void WINFUNC::downloadBin(string url, string filepath)
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
	string agent = "downloadBin";
	HINTERNET hint = NULL;
	HINTERNET hconnect = NULL;
	HINTERNET hrequest = NULL;
	DWORD bytes_available;
	DWORD bytes_read = 0;
	unsigned char* ubufferA;
	int size1, size2;
	wstring fileW, filePathW;
	string sfile;
	bool special_char = 0;
	basic_ofstream<unsigned char, char_traits<unsigned char>> BPRINTER;

	hint = InternetOpenA(agent.c_str(), INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
	if (hint)
	{
		InternetStatusCallback = InternetSetStatusCallback(hint, (INTERNET_STATUS_CALLBACK)call);
		hconnect = InternetConnectA(hint, server_name.c_str(), INTERNET_DEFAULT_HTTP_PORT, NULL, NULL, INTERNET_SERVICE_HTTP, 0, context);
	}
	else { winerr("InternetOpenA-wf.downloadBin"); }
	if (hconnect)
	{
		hrequest = HttpOpenRequestA(hconnect, NULL, object_name.c_str(), NULL, NULL, NULL, INTERNET_FLAG_NO_CACHE_WRITE, context);
	}
	else { winerr("InternetConnectA-wf.downloadBin"); }
	if (hrequest)
	{
		yesno = HttpSendRequest(hrequest, NULL, 0, NULL, 0);
	}
	else { winerr("HttpOpenRequestA-wf.downloadBin"); }
	if (yesno)
	{
		BPRINTER.open(filepath.c_str(), ios::binary | ios::trunc);
		auto report = BPRINTER.rdstate();
		bool Fail = BPRINTER.fail();
		DWORD dwErr = GetLastError();
		if (Fail == 1 && dwErr == 1113)
		{
			// Use wofstream.
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
					winerr("InternetReadFile-wf.downloadBin");
				}
				BPRINTER.write(ubufferA, bytes_available);
				BPRINTER.flush();
				delete[] ubufferA;
			} while (bytes_available > 0);
			BPRINTER.close();
		}
	}
	else { winerr("HttpSendRequest-wf.downloadBin"); }

	if (hrequest) { InternetCloseHandle(hrequest); }
	if (hconnect) { InternetCloseHandle(hconnect); }
	if (hint) { InternetCloseHandle(hint); }
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
	wstring folder_search = jf.utf8to16(temp);
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
	else { jf.err("internetopen-wf.urlRedirect"); }
	if (hconnect)
	{
		hrequest = HttpOpenRequestA(hconnect, NULL, object_name.c_str(), NULL, NULL, NULL, 0, context);
	}
	else { jf.err("internetconnect-wf.urlRedirect"); }
	if (hrequest)
	{
		yesno = HttpSendRequest(hrequest, NULL, 0, NULL, 0);
	}
	else { jf.err("httpopenrequest-wf.urlRedirect"); }
	if (yesno)
	{
		if (objects.size() > 0)
		{
			object = objects[objects.size() - 1];
			objects.clear();
		}
	}
	else { jf.err("httpsendrequest-wf.urlRedirect"); }
	string newUrl = server_name + jf.utf16to8(object);
	return newUrl;
}

