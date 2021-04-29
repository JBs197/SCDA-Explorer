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
#include <cmath>

using namespace std;
extern mutex m_err;
extern const string sroot;
extern const string scroot;

class JFUNC
{
	ofstream ERR, LOG;
	string error_path = sroot + "\\SCDA Error Log.txt";
	string log_path = sroot + "\\SCDA Process Log.txt";
	string navigator_asset_path = sroot + "\\SCDA Navigator Asset.bin";
	vector<vector<string>> navigator_search;  // Form [search tree layer index][section start, section end, inside iteration, ...].

	void stopWatch(atomic_int&, atomic_ullong&);
	chrono::steady_clock::time_point t1;

public:
	explicit JFUNC() {}
	~JFUNC() {}

	atomic_ullong stopwatch = 0;  // Max timer ticks is 1.8e19.
	atomic_int stopwatch_control = 0;

	string bind(string&, vector<string>&);
	void err(string);
	string get_error_path();
	void isort_slist(vector<string>&);
	int is_numeric(string&);
	vector<int> get_roots(vector<vector<int>>&);
	vector<string> ivectorToSvector(vector<int>&);
	vector<string> list_from_marker(string&, char);
	string load(string);
	void log(string message);
	void logTime(string func, long long timer);
	void navigator(vector<vector<int>>&, vector<string>&, vector<string>&, string&, int);
	void navParser(string& sfile, vector<vector<string>>& search);
	string parent_from_marker(string&, char);
	void pngRead(string& pathPNG);
	void quicksort(vector<int>&, int, int);
	void set_navigator_asset_path(string&);
	vector<int> svectorToIvector(vector<string>&);
	void tclean(string&, char, string);
	vector<string> textParser(string&, vector<string>&);
	void timerStart();
	long long timerRestart();
	long long timerStop();
	string timestamper();
	int tree_from_marker(vector<vector<int>>&, vector<string>&);
	string utf16to8(wstring);
	wstring utf8to16(string);


	// TEMPLATES

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
	template<> int clean<vector<string>>(string& bbq, vector<string> dirt, vector<string> soap)
	{
		if (dirt.size() != soap.size()) { err("size mismatch-jf.clean"); }
		size_t pos1;
		for (int ii = 0; ii < dirt.size(); ii++)
		{
			pos1 = bbq.find(dirt[ii]);
			while (pos1 < bbq.size())
			{
				bbq.replace(pos1, dirt[ii].size(), soap[ii]);
				pos1 = bbq.find(dirt[ii], pos1 + soap[ii].size());
			}
		}
		return 0;
	}

	template<typename ... Args> int coordCircleClockwise(Args& ... args)
	{
		// Takes in three sets of point coordinates (originPast, origin, test).
		// Define the baseline vector to be the line formed by starting from 
		// originPast and passing through origin. Define the radius as the distance
		// between the test point and the origin point. Define the examined circle 
		// to be the set of points which have this (radius) distance between 
		// themselves and the origin. Suppose we travel along the circle, starting
		// from the point of intersection between the circle and the baseline 
		// vector (NOT originPast). The test point is considered to be "clockwise"
		// if a clockwise travel direction along the circle is shorter than a 
		// counter-clockwise travel direction. Likewise, the test point is 
		// "counter-clockwise" if that travel direction is shorter. Return values
		// can be 1 = clockwise, 0 = counterclockwise, -1 = neither
		err("coordCircleClockwise template");
	}
	template<> int coordCircleClockwise<vector<vector<double>>>(vector<vector<double>>& coords)
	{
		// NOTE: Image coordinates use a reversed y-axis (positive points down) !
		//       7
		//     2 | 3
		//   6---+---4      <--- Quadrant diagram.
		//     1 | 0
		//       5
		double baseDx = coords[1][0] - coords[0][0];
		double baseDy = coords[1][1] - coords[0][1];
		double testDx = coords[2][0] - coords[1][0];
		double testDy = coords[2][1] - coords[1][1];
		if (baseDx == 0.0 && baseDy == 0.0) { err("same coordinate-jf.coordCircleClockwise"); }
		if (testDx == 0.0 && testDy == 0.0) { err("same coordinate-jf.coordCircleClockwise"); }
		int baseQuadrant, testQuadrant;  // [0,3]
		if (baseDx > 0.0)
		{
			if (baseDy > 0.0) { baseQuadrant = 0; }
			else if (baseDy < 0.0) { baseQuadrant = 3; }
			else { baseQuadrant = 4; }
		}
		else if (baseDx < 0.0)
		{
			if (baseDy > 0.0) { baseQuadrant = 1; }
			else if (baseDy < 0.0) { baseQuadrant = 2; }
			else { baseQuadrant = 6; }
		}
		else
		{
			if (baseDy >= 0.0) { baseQuadrant = 5; }
			else { baseQuadrant = 7; }
		}
		if (testDx > 0.0)
		{
			if (testDy > 0.0) { testQuadrant = 0; }
			else if (testDy < 0.0) { testQuadrant = 3; }
			else { testQuadrant = 4; }
		}
		else if (testDx < 0.0)
		{
			if (testDy > 0.0) { testQuadrant = 1; }
			else if (testDy < 0.0) { testQuadrant = 2; }
			else { testQuadrant = 6; }
		}
		else
		{
			if (testDy >= 0.0) { testQuadrant = 5; }
			else { testQuadrant = 7; }
		}

		// Easy quadrants cases (no angle needed).
		switch (baseQuadrant)
		{
		case 0:
			if (testQuadrant == 5 || testQuadrant == 1 || testQuadrant == 6) { return 1; }
			if (testQuadrant == 4 || testQuadrant == 3 || testQuadrant == 7) { return 0; }
			break;
		case 1:
			if (testQuadrant == 6 || testQuadrant == 2 || testQuadrant == 7) { return 1; }
			if (testQuadrant == 5 || testQuadrant == 0 || testQuadrant == 4) { return 0; }
			break;
		case 2:
			if (testQuadrant == 7 || testQuadrant == 3 || testQuadrant == 4) { return 1; }
			if (testQuadrant == 6 || testQuadrant == 1 || testQuadrant == 5) { return 0; }
			break;
		case 3:
			if (testQuadrant == 4 || testQuadrant == 0 || testQuadrant == 5) { return 1; }
			if (testQuadrant == 7 || testQuadrant == 2 || testQuadrant == 6) { return 0; }
			break;
		case 4:
			if (testQuadrant == 0 || testQuadrant == 5 || testQuadrant == 1) { return 1; }
			if (testQuadrant == 3 || testQuadrant == 7 || testQuadrant == 2) { return 0; }
			err("base and test angles are parallel-jf.coordCircleClockwise");
			break;
		case 5:
			if (testQuadrant == 1 || testQuadrant == 6 || testQuadrant == 2) { return 1; }
			if (testQuadrant == 0 || testQuadrant == 4 || testQuadrant == 3) { return 0; }
			err("base and test angles are parallel-jf.coordCircleClockwise");
			break;
		case 6:
			if (testQuadrant == 2 || testQuadrant == 7 || testQuadrant == 3) { return 1; }
			if (testQuadrant == 1 || testQuadrant == 5 || testQuadrant == 0) { return 0; }
			err("base and test angles are parallel-jf.coordCircleClockwise");
			break;
		case 7:
			if (testQuadrant == 3 || testQuadrant == 4 || testQuadrant == 0) { return 1; }
			if (testQuadrant == 2 || testQuadrant == 6 || testQuadrant == 1) { return 0; }
			err("base and test angles are parallel-jf.coordCircleClockwise");
			break;
		}

		// Same/opposite quadrants cases.
		double phi = abs(atan(baseDy / baseDx));
		double theta = abs(atan(testDy / testDx));
		switch (baseQuadrant)
		{
		case 0:
			if (testQuadrant == 0)
			{
				if (theta > phi) { return 1; }
				else { return 0; }
			}
			else if (testQuadrant == 2)
			{
				if (theta > phi) { return 0; }
				else { return 1; }
			}
			else { err("wrong quadrants-jf.coordCircleClockwise"); }
			break;
		case 1:
			if (testQuadrant == 1)
			{
				if (theta > phi) { return 0; }
				else { return 1; }
			}
			else if (testQuadrant == 3)
			{
				if (theta > phi) { return 1; }
				else { return 0; }
			}
			else { err("wrong quadrants-jf.coordCircleClockwise"); }
			break;
		case 2:
			if (testQuadrant == 2)
			{
				if (theta > phi) { return 1; }
				else { return 0; }
			}
			else if (testQuadrant == 0)
			{
				if (theta > phi) { return 0; }
				else { return 1; }
			}
			else { err("wrong quadrants-jf.coordCircleClockwise"); }
			break;
		case 3:
			if (testQuadrant == 3)
			{
				if (theta > phi) { return 0; }
				else { return 1; }
			}
			else if (testQuadrant == 1)
			{
				if (theta > phi) { return 1; }
				else { return 0; }
			}
			else { err("wrong quadrants-jf.coordCircleClockwise"); }
			break;
		}
	
		return -1;
	}
	template<> int coordCircleClockwise<vector<vector<int>>>(vector<vector<int>>& icoords)
	{
		vector<vector<double>> coords;
		toDouble(icoords, coords);

		double baseDx = coords[1][0] - coords[0][0];
		double baseDy = coords[1][1] - coords[0][1];
		double testDx = coords[2][0] - coords[1][0];
		double testDy = coords[2][1] - coords[1][1];
		int baseQuadrant, testQuadrant;  // [0,3]
		if (baseDx > 0.0)
		{
			if (baseDy > 0.0) { baseQuadrant = 0; }
			else { baseQuadrant = 3; }
		}
		else
		{
			if (baseDy > 0.0) { baseQuadrant = 1; }
			else { baseQuadrant = 2; }
		}
		if (testDx > 0.0)
		{
			if (testDy > 0.0) { testQuadrant = 0; }
			else { testQuadrant = 3; }
		}
		else
		{
			if (testDy > 0.0) { testQuadrant = 1; }
			else { testQuadrant = 2; }
		}

		// Adjacent quadrants cases.
		if (testQuadrant == (baseQuadrant + 1) % 4) { return 0; }
		if (testQuadrant == (baseQuadrant - 1) % 4) { return 1; }

		// Same/opposite quadrants cases.
		double phi = atan(baseDy / baseDx);
		double theta = atan(testDy / testDx);
		if (baseQuadrant == 0 || baseQuadrant == 2)
		{
			if (theta > phi) { return 0; }
			else { return 1; }
		}
		else
		{
			if (theta > phi) { return 1; }
			else { return 0; }
		}

		return -1;
	}

	template<typename ... Args> double coordDist(Args& ... args)
	{
		err("coordDist template");
	}
	template<> double coordDist<vector<double>, vector<double>>(vector<double>& origin, vector<double>& test)
	{
		double xTemp = pow(test[0] - origin[0], 2.0);
		double yTemp = pow(test[1] - origin[1], 2.0);
		double radius = sqrt(xTemp + yTemp);
		return radius;
	}

	template<typename ... Args> string decToHex(Args& ... args) {}
	template<> string decToHex<int>(int& idec)
	{
		string shex;
		vector<int> remainders = { idec % 16 };
		vector<int> quotients = { idec / 16 };
		int index = 0;
		while (quotients[index] > 0)
		{
			remainders.push_back(quotients[index] % 16);
			quotients.push_back(quotients[index] / 16);
			index++;
		}
		for (int ii = index; ii >= 0; ii--)
		{
			if (remainders[ii] < 10)
			{
				shex.append(to_string(remainders[ii]));
			}
			else 
			{
				shex.push_back(remainders[ii] + 55);
			}
		}
		return shex;
	}
	template<> string decToHex<unsigned char>(unsigned char& ucdec)
	{
		string shex;
		vector<int> remainders = { ucdec % 16 };
		vector<int> quotients = { ucdec / 16 };
		int index = 0;
		while (quotients[index] > 0)
		{
			remainders.push_back(quotients[index] % 16);
			quotients.push_back(quotients[index] / 16);
			index++;
		}
		for (int ii = index; ii >= 0; ii--)
		{
			if (remainders[ii] < 10)
			{
				shex.append(to_string(remainders[ii]));
			}
			else
			{
				shex.push_back(remainders[ii] + 55);
			}
		}
		return shex;
	}

	template<typename ... Args> int maxNumCol(Args& ... args) {}
	template<> int maxNumCol<vector<vector<wstring>>>(vector<vector<wstring>>& task)
	{
		int count = 0;
		for (int ii = 0; ii < task.size(); ii++)
		{
			if (task[ii].size() > count) { count = task[ii].size(); }
		}
		return count;
	}

	template<typename ... Args> void printer(string path, Args& ... args)
	{
		// For a given file name and (string/wstring) content, write to file in UTF-8.
	}
	template<> void printer<string>(string path, string& sfile)
	{
		wstring wfile = utf8to16(sfile);
		wstring wpath = utf8to16(path);
		wofstream WPR;
		WPR.imbue(locale(locale::empty(), new codecvt_utf8<wchar_t, 0x10ffff, generate_header>));
		WPR.open(wpath, wofstream::binary, wofstream::trunc);
		WPR << wfile << endl;
		WPR.close();
	}
	template<> void printer<wstring>(string path, wstring& wfile)
	{
		wstring wpath = utf8to16(path);
		wofstream WPR;
		WPR.imbue(locale(locale::empty(), new codecvt_utf8<wchar_t, 0x10ffff, generate_header>));
		WPR.open(wpath, wofstream::trunc);
		WPR << wfile << endl;
		WPR.close();
	}

	template<typename ... Args> void removeBlanks(Args& ... args) {}
	template<> void removeBlanks<vector<string>>(vector<string>& task)
	{
		for (int ii = 0; ii < task.size(); ii++)
		{
			if (task[ii] == "")
			{
				task.erase(task.begin() + ii);
				ii--;
			}
		}
	}
	template<> void removeBlanks<vector<wstring>>(vector<wstring>& task)
	{
		for (int ii = 0; ii < task.size(); ii++)
		{
			if (task[ii] == L"")
			{
				task.erase(task.begin() + ii);
				ii--;
			}
		}
	}
	template<> void removeBlanks<vector<vector<wstring>>>(vector<vector<wstring>>& task)
	{
		for (int ii = 0; ii < task.size(); ii++)
		{
			for (int jj = 0; jj < task[ii].size(); jj++)
			{
				if (task[ii][jj] == L"")
				{
					task[ii].erase(task[ii].begin() + jj);
					jj--;
				}
			}
		}
	}

	template<typename ... Args> void toDouble(Args& ... args)
	{
		err("toDouble template");
	}
	template<> void toDouble<vector<int>, vector<double>>(vector<int>& input, vector<double>& output)
	{
		output.resize(input.size());
		for (int ii = 0; ii < input.size(); ii++)
		{
			output[ii] = (double)input[ii];
		}
	}
	template<> void toDouble<vector<vector<int>>, vector<vector<double>>>(vector<vector<int>>& input, vector<vector<double>>& output)
	{
		int length;
		output.clear();
		output.resize(input.size(), vector<double>());
		for (int ii = 0; ii < input.size(); ii++)
		{
			length = input[ii].size();
			output[ii].resize(length);
			for (int jj = 0; jj < length; jj++)
			{
				output[ii][jj] = (double)input[ii][jj];
			}
		}
	}

	template<typename ... Args> void toInt(Args& ... args)
	{
		err("toInt template");
	}
	template<> void toInt<vector<double>, vector<int>>(vector<double>& input, vector<int>& output)
	{
		output.resize(input.size());
		for (int ii = 0; ii < input.size(); ii++)
		{
			output[ii] = int(round(input[ii]));
		}
	}
	template<> void toInt<vector<vector<double>>, vector<vector<int>>>(vector<vector<double>>& input, vector<vector<int>>& output)
	{
		int length;
		output.clear();
		output.resize(input.size(), vector<int>());
		for (int ii = 0; ii < input.size(); ii++)
		{
			length = input[ii].size();
			output[ii].resize(length);
			for (int jj = 0; jj < length; jj++)
			{
				output[ii][jj] = int(round(input[ii][jj]));
			}
		}
	}

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


};

