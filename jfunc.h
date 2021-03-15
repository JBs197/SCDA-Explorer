#pragma once

#include <vector>
#include <string>
#include <unordered_map>

using namespace std;

class JFUNC
{


public:
	explicit JFUNC() {}
	~JFUNC() {}
	int tree_from_indent(vector<int>&, vector<vector<int>>&);
	int tree_from_marker(vector<vector<int>>&, vector<string>&);

};

