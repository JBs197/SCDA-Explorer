#include "winfunc.h"

using namespace std;

void WINFUNC::err(string func)
{
	jfwin.err(func);
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
	DWORD attributes;
	HANDLE hfile = FindFirstFileA(folder_search.c_str(), &info);
	if (hfile == INVALID_HANDLE_VALUE) { winerr("FindFirstFile-get_file_list"); }
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
string WINFUNC::timestamper()
{
	char buffer[26];
	string timestamp;
	chrono::system_clock::time_point today = chrono::system_clock::now();
	time_t tt = chrono::system_clock::to_time_t(today);
	ctime_s(buffer, 26, &tt);
	for (int ii = 0; ii < 26; ii++)
	{
		if (buffer[ii] == '\0') { break; }
		else { timestamp.push_back(buffer[ii]); }
	}
	return timestamp;
}
string WINFUNC::utf16to8(wstring input)
{
	auto& f = use_facet<codecvt<wchar_t, char, mbstate_t>>(locale());
	mbstate_t mb{};
	string output(input.size() * f.max_length(), '\0');
	const wchar_t* past;
	char* future;
	f.out(mb, &input[0], &input[input.size()], past, &output[0], &output[output.size()], future);
	output.resize(future - &output[0]);
	return output;
}
wstring WINFUNC::utf8to16(string input)
{
	auto& f = use_facet<codecvt<wchar_t, char, mbstate_t>>(locale());
	mbstate_t mb{};
	wstring output(input.size() * f.max_length(), L'\0');
	const char* past;
	wchar_t* future;
	f.in(mb, &input[0], &input[input.size()], past, &output[0], &output[output.size()], future);
	output.resize(future - &output[0]);
	return output;
}


