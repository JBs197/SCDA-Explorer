#pragma once

#include <vector>
#include <string>
#include <unordered_map>
#include <stdexcept>

using namespace std;

class JFUNC
{


public:
	explicit JFUNC() {}
	~JFUNC() {}
	vector<string> list_from_marker(string&, char);
	string parent_from_marker(string&, char);
	int tree_from_indent(vector<int>&, vector<vector<int>>&);
	int tree_from_marker(vector<vector<int>>&, vector<string>&);

};

