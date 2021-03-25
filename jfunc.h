#pragma once

#include <cstdio>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <unordered_map>
#include <stdexcept>
#include <locale>
#include <codecvt>
#include <algorithm>
#include <chrono>
#include <mutex>

using namespace std;
extern mutex m_err;
extern const string root;
extern const string scroot;

class JFUNC
{
	ofstream ERR, LOG;
	string error_path = root + "\\SCDA Error Log.txt";
	string log_path = root + "\\SCDA Process Log.txt";

public:
	explicit JFUNC() {}
	~JFUNC() {}
	string bind(string&, vector<string>&);
	void err(string);
	string get_error_path();
	void isort_slist(vector<string>&);
	int is_numeric(string&);
	vector<string> list_from_marker(string&, char);
	string load(string);
	void log(string);
	string parent_from_marker(string&, char);
	void quicksort(vector<int>&, int, int);
	void tclean(string&, char, string);
	string timestamper();
	int tree_from_marker(vector<vector<int>>&, vector<string>&);
	string utf16to8(wstring);
	wstring utf8to16(string);


	// TEMPLATES

	template<typename ... Args> void tree_from_indent(vector<vector<int>>&, Args& ... args)
	{
		// Populates a tree structure from the '+' indentations within the row list entries.
		// The template will accept a 1D vector of row titles, or use the first column of a
		// 2D vector provided. 
	}
	template<> void tree_from_indent<vector<string>>(vector<vector<int>>& tree_st, vector<string>& row_list)
	{
		// Note that the tree structure is of the form 
		// [node_index][ancestor1, ancestor2, ... , (neg) node value, child1, child2, ...]

		// Genealogy's indices are indent magnitudes, while its values are the last list index
		// to have that indentation.

		vector<int> genealogy, vtemp;
		int indent, delta_indent, node, parent;
		tree_st.resize(row_list.size(), vector<int>());

		for (int ii = 0; ii < row_list.size(); ii++)
		{
			// Determine this row title's indentation.
			indent = 0;
			while (row_list[ii][indent] == '+')
			{
				indent++;
			}

			// Update genealogy.
			delta_indent = indent - genealogy.size() + 1;  // New indent minus old indent.
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
	}
	template<> void tree_from_indent<vector<vector<string>>>(vector<vector<int>>& tree_st, vector<vector<string>>& rows)
	{
		// Note that the tree structure is of the form 
		// [node_index][ancestor1, ancestor2, ... , (neg) node value, child1, child2, ...]

		// Genealogy's indices are indent magnitudes, while its values are the last list index
		// to have that indentation.

		vector<string> row_list(rows.size());
		for (int ii = 0; ii < rows.size(); ii++)
		{
			row_list[ii] = rows[ii][0];
		}

		vector<int> genealogy, vtemp;
		int indent, delta_indent, node, parent;
		tree_st.resize(row_list.size(), vector<int>());

		for (int ii = 0; ii < row_list.size(); ii++)
		{
			// Determine this row title's indentation.
			indent = 0;
			while (row_list[ii][indent] == '+')
			{
				indent++;
			}

			// Update genealogy.
			delta_indent = indent - genealogy.size() + 1;  // New indent minus old indent.
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

	}

	template<typename ... Args> int clean(string&, vector<string>, Args ... args)
	{
		// First parameter is a reference to the string needing cleaning.
		// Second parameter is a list of chars to be removed. If a string here has a length of 
		// one, then each instance of that char is simply removed. If a string here has a length
		// of two, then everything between the first char and the second char (limits inclusive!)
		// will be removed. 
		// Third parameter (optional) is a string wherein each char, whenever found in the string
		// being cleaned, will be doubled. 
		// Additionally, all empty spaces at the front/back of the cleaned string will be removed.
		// The returned integer is the number of spaces removed from the front. 

		return 0;
	}
	template<> int clean(string& bbq, vector<string> dirt)
	{
		int count = 0;
		size_t pos1, pos2;
		for (int ii = 0; ii < dirt.size(); ii++)
		{
			if (dirt[ii].size() == 1)
			{
				pos1 = bbq.find(dirt[ii][0]);
				while (pos1 < bbq.size())
				{
					bbq.erase(pos1, 1);
					pos1 = bbq.find(dirt[ii][0], pos1);
				}
			}
			else if (dirt[ii].size() == 2)
			{
				pos1 = bbq.find(dirt[ii][0]);
				while (pos1 < bbq.size())
				{
					pos2 = bbq.find(dirt[ii][1], pos1 + 1);
					if (pos2 > bbq.size())
					{
						cerr << "ERROR: no closing parameter to delete interval-clean: " << dirt[ii] << endl;
						cin.get();
					}
					bbq.erase(pos1, pos2 - pos1 + 1);
					pos1 = bbq.find(dirt[ii][0], pos1);
				}
			}
		}

		while (1)
		{
			if (bbq.front() == ' ') { bbq.erase(0, 1); count++; }
			else { break; }
		}
		while (1)
		{
			if (bbq.back() == ' ') { bbq.erase(bbq.size() - 1, 1); }
			else { break; }
		}
		return count;
	}
	template<> int clean<string>(string& bbq, vector<string> dirt, string twins)
	{
		int count = 0;
		size_t pos1, pos2;
		for (int ii = 0; ii < dirt.size(); ii++)
		{
			if (dirt[ii].size() == 1)
			{
				pos1 = bbq.find(dirt[ii][0]);
				while (pos1 < bbq.size())
				{
					bbq.erase(pos1, 1);
					pos1 = bbq.find(dirt[ii][0], pos1);
				}
			}
			else if (dirt[ii].size() == 2)
			{
				pos1 = bbq.find(dirt[ii][0]);
				while (pos1 < bbq.size())
				{
					pos2 = bbq.find(dirt[ii][1], pos1 + 1);
					if (pos2 > bbq.size())
					{
						err("find second dirt-jf.clean");
					}
					bbq.erase(pos1, pos2 - pos1 + 1);
					pos1 = bbq.find(dirt[ii][0], pos1);
				}
			}
		}

		string temp;
		for (int ii = 0; ii < twins.length(); ii++)
		{
			temp.assign(2, twins[ii]);
			pos1 = bbq.find(twins[ii]);
			while (pos1 < bbq.size())
			{
				bbq.replace(pos1, 1, temp);
				pos1 = bbq.find(twins[ii], pos1 + 2);
			}
		}

		while (1)
		{
			if (bbq.front() == ' ') { bbq.erase(0, 1); count++; }
			else { break; }
		}
		while (1)
		{
			if (bbq.back() == ' ') { bbq.erase(bbq.size() - 1, 1); }
			else { break; }
		}
		return count;
	}
};

