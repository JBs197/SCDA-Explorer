#include "winfunc.h"

using namespace std;
vector<wstring> WINFUNC::objects;

typedef struct {
	HWND       hWindow;
	int        nStatusList;
	HINTERNET hresource;
	char szMemo[512];
} REQUEST_CONTEXT;

string WINFUNC::browseA(string url)
{
	// Return the URL's contents as an ASCII string.
	unsigned char* buffer;
	string aPage, server_name, object_name, temp;
	BOOL yesno;
	DWORD size, bytesRead;
	temp = url;
	size_t cut_here = temp.find("www"), aIndex = 0;
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
	wstring wServerName = jf.utf8to16(server_name);
	wstring wObjectName = jf.utf8to16(object_name);

	HINTERNET hSession = WinHttpOpen(L"wfWH.browse", WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
	if (hSession == NULL) { winerr("WinHttpOpen-wf.browse"); }
	HINTERNET hConnect = WinHttpConnect(hSession, wServerName.c_str(), INTERNET_DEFAULT_PORT, 0);
	if (hConnect == NULL) { winerr("WinHttpConnect-wf.browse"); }
	HINTERNET hRequest = WinHttpOpenRequest(hConnect, NULL, wObjectName.c_str(), NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, 0);
	if (hRequest == NULL) { winerr("WinHttpRequest-wf.browse"); }
	yesno = WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, -1L, WINHTTP_NO_REQUEST_DATA, 0, 0, 0);
	if (!yesno) { winerr("WinHttpSendRequest-wf.browse"); }
	yesno = WinHttpReceiveResponse(hRequest, NULL);
	if (!yesno) { winerr("WinHttpReceiveResponse-wf.browse"); }
	while (1)
	{
		size = 0;
		yesno = WinHttpQueryDataAvailable(hRequest, &size);
		if (!yesno) { winerr("WinHttpQueryDataAvailable-wf.browse"); }
		if (size == 0) { break; }
		buffer = new unsigned char[size];
		yesno = WinHttpReadData(hRequest, buffer, size, &bytesRead);
		if (!yesno) { winerr("WinHttpReadData-wf.browse"); }
		aPage.resize(aPage.size() + size);
		for (int ii = 0; ii < (int)size; ii++)
		{
			aPage[aIndex] = (char)buffer[ii];
			aIndex++;
		}
		delete[] buffer;
	}

	yesno = WinHttpCloseHandle(hRequest);
	if (!yesno) { winerr("WinHttpCloseHandle-wf.browse"); }
	yesno = WinHttpCloseHandle(hConnect);
	if (!yesno) { winerr("WinHttpCloseHandle-wf.browse"); }
	yesno = WinHttpCloseHandle(hSession);
	if (!yesno) { winerr("WinHttpCloseHandle-wf.browse"); }
	return aPage;
}
string WINFUNC::browseS(string url)
{
	// Return the URL's contents as a UTF-8 string.
	wstring wPage = browseW(url);
	string sPage = jf.utf16to8(wPage);
	return sPage;
}
vector<unsigned char> WINFUNC::browseUC(string url)
{
	// Return the URL's contents as a vector of uchars.
	vector<unsigned char> ucPage;
	unsigned char* buffer;
	string server_name, object_name, temp;
	BOOL yesno;
	DWORD size, bytesRead;
	temp = url;
	size_t cut_here = temp.find("www"), ucIndex = 0;
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
	wstring wServerName = jf.utf8to16(server_name);
	wstring wObjectName = jf.utf8to16(object_name);

	HINTERNET hSession = WinHttpOpen(L"wfWH.browse", WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
	if (hSession == NULL) { winerr("WinHttpOpen-wf.browse"); }
	HINTERNET hConnect = WinHttpConnect(hSession, wServerName.c_str(), INTERNET_DEFAULT_PORT, 0);
	if (hConnect == NULL) { winerr("WinHttpConnect-wf.browse"); }
	HINTERNET hRequest = WinHttpOpenRequest(hConnect, NULL, wObjectName.c_str(), NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, 0);
	if (hRequest == NULL) { winerr("WinHttpRequest-wf.browse"); }
	yesno = WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, -1L, WINHTTP_NO_REQUEST_DATA, 0, 0, 0);
	if (!yesno) { winerr("WinHttpSendRequest-wf.browse"); }
	yesno = WinHttpReceiveResponse(hRequest, NULL);
	if (!yesno) { winerr("WinHttpReceiveResponse-wf.browse"); }
	while (1)
	{
		size = 0;
		yesno = WinHttpQueryDataAvailable(hRequest, &size);
		if (!yesno) { winerr("WinHttpQueryDataAvailable-wf.browse"); }
		if (size == 0) { break; }
		buffer = new unsigned char[size];
		yesno = WinHttpReadData(hRequest, buffer, size, &bytesRead);
		if (!yesno) { winerr("WinHttpReadData-wf.browse"); }
		ucPage.resize(ucPage.size() + size);
		for (int ii = 0; ii < (int)size; ii++)
		{
			ucPage[ucIndex] = buffer[ii];
			ucIndex++;
		}
		delete[] buffer;
	}

	yesno = WinHttpCloseHandle(hRequest);
	if (!yesno) { winerr("WinHttpCloseHandle-wf.browse"); }
	yesno = WinHttpCloseHandle(hConnect);
	if (!yesno) { winerr("WinHttpCloseHandle-wf.browse"); }
	yesno = WinHttpCloseHandle(hSession);
	if (!yesno) { winerr("WinHttpCloseHandle-wf.browse"); }
	return ucPage;
}
wstring WINFUNC::browseW(string url)
{
	wstring wPage;
	unsigned char* buffer;
	string server_name, object_name, temp;
	BOOL yesno;
	DWORD size, bytesRead;
	temp = url;
	size_t cut_here = temp.find("www"), wIndex = 0;
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
	wstring wServerName = jf.utf8to16(server_name);
	wstring wObjectName = jf.utf8to16(object_name);

	HINTERNET hSession = WinHttpOpen(L"wfWH.browse", WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
	if (hSession == NULL) { winerr("WinHttpOpen-wf.browse"); }
	HINTERNET hConnect = WinHttpConnect(hSession, wServerName.c_str(), INTERNET_DEFAULT_PORT, 0);
	if (hConnect == NULL) { winerr("WinHttpConnect-wf.browse"); }
	HINTERNET hRequest = WinHttpOpenRequest(hConnect, NULL, wObjectName.c_str(), NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, 0);
	if (hRequest == NULL) { winerr("WinHttpRequest-wf.browse"); }
	yesno = WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, -1L, WINHTTP_NO_REQUEST_DATA, 0, 0, 0);
	if (!yesno) { winerr("WinHttpSendRequest-wf.browse"); }
	yesno = WinHttpReceiveResponse(hRequest, NULL);
	if (!yesno) { winerr("WinHttpReceiveResponse-wf.browse"); }
	while (1)
	{
		size = 0;
		yesno = WinHttpQueryDataAvailable(hRequest, &size);
		if (!yesno) { winerr("WinHttpQueryDataAvailable-wf.browse"); }
		if (size == 0) { break; }
		buffer = new unsigned char[size];
		yesno = WinHttpReadData(hRequest, buffer, size, &bytesRead);
		if (!yesno) { winerr("WinHttpReadData-wf.browse"); }
		wPage.resize(wPage.size() + size);
		for (int ii = 0; ii < (int)size; ii++)
		{
			wPage[wIndex] = (wchar_t)buffer[ii];
			wIndex++;
		}
		delete[] buffer;
	}
	jf.UTF16clean(wPage);

	yesno = WinHttpCloseHandle(hRequest);
	if (!yesno) { winerr("WinHttpCloseHandle-wf.browse"); }
	yesno = WinHttpCloseHandle(hConnect);
	if (!yesno) { winerr("WinHttpCloseHandle-wf.browse"); }
	yesno = WinHttpCloseHandle(hSession);
	if (!yesno) { winerr("WinHttpCloseHandle-wf.browse"); }
	return wPage;
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
	wstring wFile = browseW(url);
	jf.printer(filepath, wFile);
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
vector<string> WINFUNC::getFileList(string folder_path, string search)
{
	// Returns a list of UTF-8 file names (not full paths) within a 
	// given folder, and subject to the search criteria.

	vector<string> file_list;
	wstring wTemp;
	wstring folder_search = jf.utf8to16(folder_path) + L"\\" + jf.utf8to16(search);
	WIN32_FIND_DATAW info;
	DWORD attributes, GLE;
	HANDLE hfile = FindFirstFileW(folder_search.c_str(), &info);
	if (hfile == INVALID_HANDLE_VALUE)
	{
		GLE = GetLastError();
		if (GLE == 2) { return file_list; }
		else { winerr("FindFirstFileW-getFileList"); }
	}
	do
	{
		attributes = info.dwFileAttributes;
		if (attributes == FILE_ATTRIBUTE_NORMAL || attributes == FILE_ATTRIBUTE_ARCHIVE)
		{
			wTemp = info.cFileName;
			jf.UTF16clean(wTemp);
			file_list.push_back(jf.utf16to8(wTemp));
		}
	} while (FindNextFileW(hfile, &info));
	return file_list;
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
string WINFUNC::load(string filePath)
{
	string sFile;
	wstring wPath, wTemp;
	DWORD bytesRead, fileSize, fileSizeHigh = 0;
	unsigned char* bufferU;
	char* bufferC;
	size_t pos1 = filePath.find(-61), index;
	if (pos1 < filePath.size()) { wPath = jf.utf8to16(filePath); }
	else { wPath = jf.asciiToUTF16(filePath); }
	HANDLE hFile = CreateFileW(wPath.c_str(), (GENERIC_READ | GENERIC_WRITE), (FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE), NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE) { winerr("CreateFileW-wf.load"); }
	bufferU = new unsigned char[8];
	BOOL success = ReadFile(hFile, bufferU, 8, &bytesRead, NULL);
	if (!success) { winerr("ReadFile-wf.load"); }
	if (bufferU[0] == 239 && bufferU[1] == 187 && bufferU[2] == 191)  
	{
		// UTF8
		fileSize = GetFileSize(hFile, &fileSizeHigh);
		if (fileSize == INVALID_FILE_SIZE) { winerr("GetFileSize-wf.load"); }
		if (fileSizeHigh > 0) { jf.err("Enormous file size-wf.load"); }
		delete[] bufferU;
		bufferC = new char[fileSize];
		SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
		success = ReadFile(hFile, bufferC, fileSize, &bytesRead, NULL);
		if (!success) { winerr("ReadFile-wf.load"); }
		sFile.assign(bufferC, fileSize);
		delete[] bufferC;
	}
	else if ( (bufferU[0] == 255 && bufferU[1] == 254) || (bufferU[0] == 254 && bufferU[1] == 255)) 
	{
		// UTF16
		fileSize = GetFileSize(hFile, &fileSizeHigh);
		if (fileSize == INVALID_FILE_SIZE) { winerr("GetFileSize-wf.load"); }
		if (fileSizeHigh > 0) { jf.err("Enormous file size-wf.load"); }
		delete[] bufferU;
		bufferU = new unsigned char[fileSize + 1];
		SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
		success = ReadFile(hFile, bufferU, fileSize, &bytesRead, NULL);
		if (!success) { winerr("ReadFile-wf.load"); }
		wTemp.resize((fileSize / 2) + 1);
		index = 0;
		for (int ii = 2; ii < fileSize; ii++)
		{
			if (bufferU[ii] != 0)
			{
				wTemp[index] = (wchar_t)bufferU[ii];
				index++;
			}
		}
		while (wTemp.back() == 0) { wTemp.pop_back(); }
		delete[] bufferU;
		sFile = jf.utf16to8(wTemp);
	}
	else
	{
		// Binary data.
		fileSize = GetFileSize(hFile, &fileSizeHigh);
		if (fileSize == INVALID_FILE_SIZE) { winerr("GetFileSize-wf.load"); }
		if (fileSizeHigh > 0) { jf.err("Enormous file size-wf.load"); }
		delete[] bufferU;
		bufferC = new char[fileSize];
		SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
		success = ReadFile(hFile, bufferC, fileSize, &bytesRead, NULL);
		if (!success) { winerr("ReadFile-wf.load"); }
		sFile.assign(bufferC, fileSize);
		delete[] bufferC;
	}
	
	CloseHandle(hFile);
	if (sFile.size() < 1) { jf.err("File failed to load-wf.load"); }
	return sFile;
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
void WINFUNC::renameFile(string oldPath, string newPath)
{
	DWORD GLE;
	BOOL yesno = MoveFileA(oldPath.c_str(), newPath.c_str());
	if (!yesno) 
	{ 
		GLE = GetLastError(); 
		winerr("MoveFileA-wf.renameFile");
	}
}
void WINFUNC::set_error_path(string errpath)
{
	error_path = errpath;
}

/*
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
*/
