#include "jstring.h"

string JSTRING::asciiToUTF8(string& input)
{
	int sizeInput = (int)input.size(), offset = 0;
	unordered_map<char, pair<char, char>>::iterator it2;
	unordered_map<char, tuple<char, char, char>>::iterator it3;
	string output;
	output.resize(sizeInput);
	for (int ii = 0; ii < sizeInput; ii++)
	{
		if (input[ii] < 0) {
			it2 = mapAsciiUTF8.find(input[ii]);
			if (it2 != mapAsciiUTF8.end()) {
				offset++;
				output.resize(sizeInput + offset);
				output[ii + offset] = get<0>(it2->second);
				output[ii + offset + 1] = get<1>(it2->second);
			}
			else {
				it3 = mapAsciiUTF8ext.find(input[ii]);
				if (it3 != mapAsciiUTF8ext.end()) {
					offset += 2;
					output.resize(sizeInput + offset);
					output[ii + offset] = get<0>(it3->second);
					output[ii + offset + 1] = get<1>(it3->second);
					output[ii + offset + 2] = get<2>(it3->second);
				}
				else {
					output[ii + offset] = input[ii];
				}
			}

		}
		else {
			output[ii + offset] = input[ii];
		}
	}
	return output;
}
void JSTRING::capsAll(string& text)
{
	// Replaces all lower-case chars with upper-case chars.
	int len = (int)text.size();
	for (int ii = 0; ii < len; ii++) {
		if (text[ii] >= 97 && text[ii] <= 122) {
			text[ii] -= 32;
		}
	}
}
void JSTRING::capsNone(string& text)
{
	// Replaces all upper-case chars with lower-case chars.
	int len = (int)text.size();
	for (int ii = 0; ii < len; ii++) {
		if (text[ii] >= 65 && text[ii] <= 90) {
			text[ii] += 32;
		}
	}
}
void JSTRING::clean(string& text, vector<string>& dirt, vector<string>& soap)
{
	int numTask = (int)min(dirt.size(), soap.size());
	size_t pos1;
	for (int ii = 0; ii < numTask; ii++)
	{
		pos1 = text.find(dirt[ii]);
		while (pos1 < text.size())
		{
			text.replace(pos1, dirt[ii].size(), soap[ii]);
			pos1 = text.find(dirt[ii], pos1 + soap[ii].size());
		}
	}
}
void JSTRING::init()
{
	initASCII();
}
void JSTRING::initASCII()
{
	mapAsciiUTF8.emplace(-128, make_pair(-61, -121));
	mapAsciiUTF8.emplace(-127, make_pair(-61, -68));
	mapAsciiUTF8.emplace(-126, make_pair(-61, -87));
	mapAsciiUTF8.emplace(-125, make_pair(-61, -94));
	mapAsciiUTF8.emplace(-124, make_pair(-61, -92));
	mapAsciiUTF8.emplace(-123, make_pair(-61, -96));
	mapAsciiUTF8.emplace(-122, make_pair(-61, -91));
	mapAsciiUTF8.emplace(-121, make_pair(-61, -89));
	mapAsciiUTF8.emplace(-120, make_pair(-61, -86));
	mapAsciiUTF8.emplace(-119, make_pair(-61, -85));
	mapAsciiUTF8.emplace(-118, make_pair(-61, -88));
	mapAsciiUTF8.emplace(-117, make_pair(-61, -81));
	mapAsciiUTF8.emplace(-116, make_pair(-61, -82));
	mapAsciiUTF8.emplace(-115, make_pair(-61, -84));
	mapAsciiUTF8.emplace(-114, make_pair(-61, -124));
	mapAsciiUTF8.emplace(-113, make_pair(-61, -123));
	mapAsciiUTF8.emplace(-112, make_pair(-61, -119));
	mapAsciiUTF8.emplace(-111, make_pair(-61, -90));
	mapAsciiUTF8.emplace(-110, make_pair(-61, -122));
	mapAsciiUTF8.emplace(-109, make_pair(-61, -76));
	mapAsciiUTF8.emplace(-108, make_pair(-61, -74));
	mapAsciiUTF8.emplace(-107, make_pair(-61, -78));
	mapAsciiUTF8.emplace(-106, make_pair(-61, -69));
	mapAsciiUTF8.emplace(-105, make_pair(-61, -71));
	mapAsciiUTF8.emplace(-104, make_pair(-61, -65));
	mapAsciiUTF8.emplace(-103, make_pair(-61, -106));
	mapAsciiUTF8.emplace(-102, make_pair(-61, -100));
	mapAsciiUTF8.emplace(-101, make_pair(-62, -94));
	mapAsciiUTF8.emplace(-100, make_pair(-62, -93));
	mapAsciiUTF8.emplace(-99, make_pair(-62, -91));
	mapAsciiUTF8ext.emplace(-98, make_tuple(-30, -126, -89));
	mapAsciiUTF8.emplace(-97, make_pair(152, 70));
	mapAsciiUTF8.emplace(-96, make_pair(-61, -95));
	mapAsciiUTF8.emplace(-95, make_pair(-61, -83));
	mapAsciiUTF8.emplace(-94, make_pair(-61, -77));
	mapAsciiUTF8.emplace(-93, make_pair(-61, -70));
	mapAsciiUTF8.emplace(-92, make_pair(-61, -79));
	mapAsciiUTF8.emplace(-91, make_pair(-61, -111));
	mapAsciiUTF8.emplace(-90, make_pair(-62, -86));
	mapAsciiUTF8.emplace(-89, make_pair(-62, -70));
	mapAsciiUTF8.emplace(-88, make_pair(-62, -65));
	mapAsciiUTF8ext.emplace(-87, make_tuple(-30, -116, -112));
	mapAsciiUTF8.emplace(-86, make_pair(-62, -84));
	mapAsciiUTF8.emplace(-85, make_pair(-62, -67));
	mapAsciiUTF8.emplace(-84, make_pair(-62, -68));
	mapAsciiUTF8.emplace(-83, make_pair(-62, -95));
	mapAsciiUTF8.emplace(-82, make_pair(-62, -85));
	mapAsciiUTF8.emplace(-81, make_pair(-62, -69));
	mapAsciiUTF8ext.emplace(-80, make_tuple(-30, -106, -111));
	mapAsciiUTF8ext.emplace(-79, make_tuple(-30, -106, -110));
	mapAsciiUTF8ext.emplace(-78, make_tuple(-30, -106, -109));
	mapAsciiUTF8ext.emplace(-77, make_tuple(-30, -108, -126));
	mapAsciiUTF8ext.emplace(-76, make_tuple(-30, -108, -92));
	mapAsciiUTF8ext.emplace(-75, make_tuple(-30, -107, -95));
	mapAsciiUTF8ext.emplace(-74, make_tuple(-30, -107, -94));
	mapAsciiUTF8ext.emplace(-73, make_tuple(-30, -107, -106));
	mapAsciiUTF8ext.emplace(-72, make_tuple(-30, -107, -107));
	mapAsciiUTF8ext.emplace(-71, make_tuple(-30, -107, -93));
	mapAsciiUTF8ext.emplace(-70, make_tuple(-30, -107, -111));
	mapAsciiUTF8ext.emplace(-69, make_tuple(-30, -107, -105));
	mapAsciiUTF8ext.emplace(-68, make_tuple(-30, -107, -99));
	mapAsciiUTF8ext.emplace(-67, make_tuple(-30, -107, -100));
	mapAsciiUTF8ext.emplace(-66, make_tuple(-30, -107, -101));
	mapAsciiUTF8ext.emplace(-65, make_tuple(-30, -108, -112));
	mapAsciiUTF8ext.emplace(-64, make_tuple(-30, -108, -108));
	mapAsciiUTF8ext.emplace(-63, make_tuple(-30, -108, -76));
	mapAsciiUTF8ext.emplace(-62, make_tuple(-30, -108, -84));
	mapAsciiUTF8ext.emplace(-61, make_tuple(-30, -108, -100));
	mapAsciiUTF8ext.emplace(-60, make_tuple(-30, -108, -128));
	mapAsciiUTF8ext.emplace(-59, make_tuple(-30, -108, -68));
	mapAsciiUTF8ext.emplace(-58, make_tuple(-30, -107, -98));
	mapAsciiUTF8ext.emplace(-57, make_tuple(-30, -107, -97));
	mapAsciiUTF8ext.emplace(-56, make_tuple(-30, -107, -102));
	mapAsciiUTF8ext.emplace(-55, make_tuple(-30, -107, -108));
	mapAsciiUTF8ext.emplace(-54, make_tuple(-30, -107, -87));
	mapAsciiUTF8ext.emplace(-53, make_tuple(-30, -107, -90));
	mapAsciiUTF8ext.emplace(-52, make_tuple(-30, -107, -96));
	mapAsciiUTF8ext.emplace(-51, make_tuple(-30, -107, -112));
	mapAsciiUTF8ext.emplace(-50, make_tuple(-30, -107, -84));
	mapAsciiUTF8ext.emplace(-49, make_tuple(-30, -107, -89));
	mapAsciiUTF8ext.emplace(-48, make_tuple(-30, -107, -88));
	mapAsciiUTF8ext.emplace(-47, make_tuple(-30, -107, -92));
	mapAsciiUTF8ext.emplace(-46, make_tuple(-30, -107, -91));
	mapAsciiUTF8ext.emplace(-45, make_tuple(-30, -107, -103));
	mapAsciiUTF8ext.emplace(-44, make_tuple(-30, -107, -104));
	mapAsciiUTF8ext.emplace(-43, make_tuple(-30, -107, -110));
	mapAsciiUTF8ext.emplace(-42, make_tuple(-30, -107, -109));
	mapAsciiUTF8ext.emplace(-41, make_tuple(-30, -107, -85));
	mapAsciiUTF8ext.emplace(-40, make_tuple(-30, -107, -86));
	mapAsciiUTF8ext.emplace(-39, make_tuple(-30, -108, -104));
	mapAsciiUTF8ext.emplace(-38, make_tuple(-30, -108, -116));
	mapAsciiUTF8ext.emplace(-37, make_tuple(-30, -106, -120));
	mapAsciiUTF8ext.emplace(-36, make_tuple(-30, -106, -124));
	mapAsciiUTF8ext.emplace(-35, make_tuple(-30, -106, -116));
	mapAsciiUTF8ext.emplace(-34, make_tuple(-30, -106, -112));
	mapAsciiUTF8ext.emplace(-33, make_tuple(-30, -106, -128));
	mapAsciiUTF8.emplace(-32, make_pair(-50, -79));
	mapAsciiUTF8.emplace(-31, make_pair(-61, -97));
	mapAsciiUTF8.emplace(-30, make_pair(-50, -109));
	mapAsciiUTF8.emplace(-29, make_pair(-49, -128));
	mapAsciiUTF8.emplace(-28, make_pair(-50, -93));
	mapAsciiUTF8.emplace(-27, make_pair(-49, -125));
	mapAsciiUTF8.emplace(-26, make_pair(-62, -75));
	mapAsciiUTF8.emplace(-25, make_pair(-50, -92));
	mapAsciiUTF8.emplace(-24, make_pair(-50, -90));
	mapAsciiUTF8.emplace(-23, make_pair(-50, -104));
	mapAsciiUTF8.emplace(-22, make_pair(-50, -87));
	mapAsciiUTF8.emplace(-21, make_pair(-50, -76));
	mapAsciiUTF8ext.emplace(-20, make_tuple(-30, -120, -98));
	mapAsciiUTF8.emplace(-19, make_pair(-49, -122));
	mapAsciiUTF8.emplace(-18, make_pair(-50, -75));
	mapAsciiUTF8ext.emplace(-17, make_tuple(-30, -120, -87));
	mapAsciiUTF8ext.emplace(-16, make_tuple(-30, -119, -95));
	mapAsciiUTF8.emplace(-15, make_pair(-62, -79));
	mapAsciiUTF8ext.emplace(-14, make_tuple(-30, -119, -91));
	mapAsciiUTF8ext.emplace(-13, make_tuple(-30, -119, -92));
	mapAsciiUTF8ext.emplace(-12, make_tuple(-30, -116, -96));
	mapAsciiUTF8ext.emplace(-11, make_tuple(-30, -116, -95));
	mapAsciiUTF8.emplace(-10, make_pair(-61, -73));
	mapAsciiUTF8ext.emplace(-9, make_tuple(-30, -119, -120));
	mapAsciiUTF8.emplace(-8, make_pair(-62, -80));
	mapAsciiUTF8ext.emplace(-7, make_tuple(-30, -120, -103));
	mapAsciiUTF8.emplace(-6, make_pair(-62, -73));
	mapAsciiUTF8ext.emplace(-5, make_tuple(-30, -120, -102));
	mapAsciiUTF8ext.emplace(-4, make_tuple(-30, -127, -65));
	mapAsciiUTF8.emplace(-3, make_pair(-62, -78));
	mapAsciiUTF8ext.emplace(-2, make_tuple(-30, -106, -96));
	mapAsciiUTF8.emplace(-1, make_pair(-62, -96));
}
