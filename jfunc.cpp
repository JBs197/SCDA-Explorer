#include "jfunc.h"

using namespace std;

vector<string> JFUNC::list_from_marker(string& input, char marker)
{
	// Split a string into a vector of strings, dividing when the marker char is encountered.
	// The first element is always the original string, even if no marker was encountered.
	
	vector<string> output;
	string temp1;
	size_t pos1 = 0;
	size_t pos2 = input.find(marker);
	
	output.push_back(input);
	while (pos2 < input.size())
	{
		temp1 = input.substr(pos1, pos2 - pos1);
		output.push_back(temp1);
		pos1 = input.find_first_not_of(marker, pos2);
		if (pos1 >= input.size()) { break; }
		pos2 = input.find(marker, pos1);
	}
	return output;
}
string JFUNC::load(string file_path)
{
	// Load a file into memory as a string.
	// Uses the first 8 bytes to guess what file encoding is being used.

	FILE* pFile;
	errno_t err = fopen_s(&pFile, file_path.c_str(), "rb");
	if (pFile == NULL) { cerr << "ERROR: fopen-load" << endl; cin.get(); }
	char enc[8];
	vector<bool> zero(8, 0);
	int encoding = 0;
	int count = 0;
	for (int ii = 0; ii < 8; ii++)
	{
		enc[ii] = fgetc(pFile);
		if (enc[ii] != 0)
		{
			zero[ii] = 1;
			count++;
		}
	}
	if (count == 4)
	{
		count = 0; 
		for (int ii = 0; ii < 7; ii++)
		{
			if (zero[ii] != zero[ii + 1])
			{
				count++;
			}
		}
		if (count == 7)
		{
			encoding = 2;  // UTF16
		}
	}
	else if (count > 6)
	{
		encoding = 1;  // UTF8
	}
	fclose(pFile);

	string output;
	wstring wtemp;
	ifstream myfile;
	wifstream mywfile;
	streampos size;
	wstreampos wsize;
	char* buffer;
	wchar_t* wbuffer;
	long file_size;
	size_t bytes_read, bullshit;
	switch (encoding)
	{
	case 0:
		myfile.open(file_path, ios::in | ios::binary | ios::ate);
		size = myfile.tellg();
		buffer = new char[size];
		myfile.seekg(0, ios::beg);
		myfile.read(buffer, size);
		output.assign(buffer, size);
		delete[] buffer;
		break;

	case 1:
		mywfile.open(file_path, ios::in | ios::ate);
		wsize = mywfile.tellg();
		wbuffer = new wchar_t[wsize];
		mywfile.seekg(0, ios::beg);
		mywfile.read(wbuffer, wsize);
		wtemp.assign(wbuffer, wsize);
		delete[] wbuffer;
		//utf8::utf16to8(wtemp.begin(), wtemp.end(), back_inserter(output));
		output = utf16to8(wtemp);
		break;

	case 2:
		mywfile.open(file_path, ios::in | ios::ate);
		wsize = mywfile.tellg();
		wbuffer = new wchar_t[wsize];
		mywfile.seekg(0, ios::beg);
		mywfile.read(wbuffer, wsize);
		wtemp.assign(wbuffer, wsize);
		delete[] wbuffer;
		output = utf16to8(wtemp);
		break;
	}
	return output;
}
string JFUNC::parent_from_marker(string& child, char marker)
{
	size_t pos1 = child.rfind(marker);
	pos1 = child.find_last_of(marker, pos1);
	string parent = child.substr(0, pos1);
	return parent;
}
int JFUNC::tree_from_indent(vector<int>& ind_list, vector<vector<int>>& tree_st)
{
	// Note that the tree structure is of the form 
	// [node_index][ancestor1, ancestor2, ... , (neg) node value, child1, child2, ...]

	// genealogy's indices are indent magnitudes, while its 
	// values are the last list index to have that indentation.
	vector<int> genealogy, vtemp;
	int delta_indent, node, parent;
	tree_st.resize(ind_list.size(), vector<int>());

	for (int ii = 0; ii < ind_list.size(); ii++)
	{
		// Update genealogy.
		delta_indent = ind_list[ii] - genealogy.size() + 1;  // New indent minus old indent.
		if (delta_indent == 0)
		{
			genealogy[genealogy.size() - 1] = ii;
		}
		else if (delta_indent > 0)
		{
			for (int jj = 0; jj < delta_indent; jj++)
			{
				genealogy.push_back(ii);
			}
		}
		else if (delta_indent < 0)
		{
			for (int jj = 0; jj > delta_indent; jj--)
			{
				genealogy.pop_back();
			}
			genealogy[genealogy.size() - 1] = ii;
		}
		else { return 1; }

		// Populate the current node with its ancestry and with itself, but no children.
		tree_st[ii] = genealogy;
		node = tree_st[ii].size() - 1;  // Genealogical position of the current node.
		tree_st[ii][node] *= -1;

		// Determine this node's parent, and add this node to its list of children.
		if (node == 0)
		{
			continue;  // This node has no parents.
		}
		parent = genealogy[node - 1];
		tree_st[parent].push_back(ii);
	}
	return 0;
}
int JFUNC::tree_from_marker(vector<vector<int>>& tree_st, vector<string>& tree_pl)
{
	// Starting from a list of strings containing separation markers, parse each line of that list into
	// segments. Then, assimilate all segments such that each segment, in each position, has mapped and  
	// unique representation in the 'sections' matrix. After that, build the tree structure integer 
	// matrix by linking child to parent to root. 

	vector<string> line;
	vector<int> vtemp;
	string sparent;
	bool orphans = 0;
	int iparent, generation, orphanage, num1;
	unordered_map<string, int> payloads;
	unordered_map<int, int> parents;
	vector<vector<int>> kids_waiting(1, vector<int>({ -1 }));
	tree_st.resize(tree_pl.size(), vector<int>());
	
	// Register all nodes.
	for (int ii = 0; ii < tree_pl.size(); ii++)
	{
		payloads.emplace(tree_pl[ii], ii);
	}

	// Make a node to catch the orphaned nodes.
	orphanage = tree_pl.size();
	tree_pl.push_back("Unknown");
	tree_st.push_back({ -1 * orphanage });
	parents.emplace(orphanage, -1);

	// Register every node's parent.
	for (int ii = 0; ii < tree_pl.size(); ii++)
	{
		line = list_from_marker(tree_pl[ii], '$');
		generation = line.size();
		if (generation > 1)
		{
			sparent = parent_from_marker(tree_pl[ii], '$');
			try
			{
				num1 = payloads.at(sparent);
				parents.emplace(ii, num1);
			}
			catch (out_of_range& oor)                                         
			{
				orphans = 1;
				parents.emplace(ii, orphanage);                           // Orphaned element.
			}
		}
		else if (generation == 1)                                               
		{ 
			parents.emplace(ii, -1);                                      // Root element.
		}
		else { return 1; }
	}

	// Make a tree structure entry for each node. Nodes add themselves to their parent's entry,
	// or go on a waiting list if the parent has yet to be processed.
	for (int ii = 0; ii < tree_st.size() - 1; ii++)
	{
		vtemp.clear();
		num1 = ii;
		do
		{
			try
			{
				iparent = parents.at(num1);
			}
			catch (out_of_range& oor) { return 2; }
			vtemp.push_back(iparent);
			num1 = iparent;
		} while (iparent >= 0);

		tree_st[ii].assign(vtemp.rbegin() + 1, vtemp.rend());       // Get chronological order, exclude root.
		tree_st[ii].push_back(-1 * ii);                             // Add self.
		
		// Check for waiting child nodes.
		for (int jj = 0; jj < kids_waiting.size(); jj++)            
		{
			if (kids_waiting[jj][0] == ii)
			{
				for (int kk = 1; kk < kids_waiting[jj].size(); kk++)
				{
					tree_st[ii].push_back(kids_waiting[jj][kk]);
				}
				kids_waiting.erase(kids_waiting.begin() + jj);
				break;
			}
		}

		// Add this node to its parent's list.
		if (vtemp.size() < 1) { continue; }           // This node is a root - cannot it add to a parent.
		iparent = vtemp[0];
		if (iparent < ii)
		{
			tree_st[iparent].push_back(ii);
		}
		else 
		{
			for (int jj = 0; jj < kids_waiting.size(); jj++)
			{
				if (kids_waiting[jj][0] == iparent)
				{
					kids_waiting[jj].push_back(ii);
					break;
				}
				else if (jj == kids_waiting.size() - 1)
				{
					kids_waiting.push_back({ iparent, ii });
				}
			}
		}
	}

	return 0;
}
string JFUNC::utf16to8(wstring input)
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
wstring JFUNC::utf8to16(string input)
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
