#pragma once
#include <unordered_map>
#include <string>
#include <vector>

using namespace std;

class JSTRING
{
	unordered_map<char, pair<char, char>> mapAsciiUTF8;
	unordered_map<char, tuple<char, char, char>> mapAsciiUTF8ext;

	void init();
	void initASCII();

public:
	JSTRING() { init(); }
	~JSTRING() {}

	string asciiToUTF8(string& input);
	void capsAll(string& text);
	void capsNone(string& text);
	void clean(string& text, vector<string>& dirt, vector<string>& soap);
};