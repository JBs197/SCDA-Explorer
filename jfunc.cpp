#include "jfunc.h"

void JFUNC::asciiNearestFit(string& input)
{
	// For a given ASCII or UTF8 string, replace all negative chars with [0, 127] equivalent.
	for (int ii = 0; ii < input.size(); ii++)
	{
		if (input[ii] < 0)
		{
			if (ii < input.size() - 1)  // If UTF8, make ASCII.
			{
				if (input[ii] == -61 && input[ii + 1] < 0)  
				{
					input[ii] = input[ii + 1] + 64;
					input.erase(input.begin() + ii + 1);
				}
			}
		
			if (input[ii] >= -64 && input[ii] <= -59) { input[ii] = 'A'; } 
			else if (input[ii] == -57) { input[ii] = 'C'; }
			else if (input[ii] >= -56 && input[ii] <= -53) { input[ii] = 'E'; }
			else if (input[ii] >= -52 && input[ii] <= -49) { input[ii] = 'I'; }
			else if (input[ii] >= -46 && input[ii] <= -42) { input[ii] = 'O'; }
			else if (input[ii] >= -39 && input[ii] <= -36) { input[ii] = 'U'; }
			else if (input[ii] >= -32 && input[ii] <= -27) { input[ii] = 'a'; }
			else if (input[ii] == -25) { input[ii] = 'c'; }
			else if (input[ii] >= -24 && input[ii] <= -21) { input[ii] = 'e'; }
			else if (input[ii] >= -20 && input[ii] <= -17) { input[ii] = 'i'; }
			else if (input[ii] >= -14 && input[ii] <= -10) { input[ii] = 'o'; }
			else if (input[ii] >= -7 && input[ii] <= -4) { input[ii] = 'u'; }
			else { err("Failed to locate accent-jf.asciiNearestFit"); }
		}
	}
}
string JFUNC::asciiOnly(string& input)
{
	string ascii;
	ascii.resize(input.size());
	int index = 0;
	for (int ii = 0; ii < input.size(); ii++)
	{
		if (input[ii] >= 0) 
		{
			ascii[index] = input[ii];
			index++;
		}
	}
	ascii.resize(index);
	return ascii;
}
string JFUNC::asciiToUTF8(string input)
{
	string output;
	int sizeInput = input.size(), offset = 0;
	output.assign(sizeInput, 0);
	for (int ii = 0; ii < sizeInput; ii++)
	{
		if (input[ii] < 0)
		{
			output[ii + offset] = -61;
			output[ii + offset + 1] = input[ii] - 64;
			offset++;
			output.resize(sizeInput + offset);
		}
		else
		{
			output[ii + offset] = input[ii];
		}
	}
	return output;
}
wstring JFUNC::asciiToUTF16(string input)
{
	mbstate_t mb{};
	auto& f = use_facet<codecvt<wchar_t, char, mbstate_t>>(locale());
	wstring output(input.size() * 2, L'\0');
	const char* past;
	wchar_t* future;
	f.in(mb, &input[0], &input[input.size()], past, &output[0], &output[output.size()], future);
	output.resize(future - &output[0]);
	UTF16clean(output);
	return output;
}
vector<unsigned char> JFUNC::baseConvert(vector<unsigned char>& source, int x, int y)
{
	// Converts binary data from base 'x' to base 'y'.
	if (x < 2 || y < 2 || x > 256 || y > 256) { err("Invalid base-jf.baseConvert"); }
	vector<unsigned char> converted;
	unsigned char carry;
	int digit, numDigit = 0;
	if (x > y)
	{
		digit = x;
		do
		{
			numDigit++;
			digit /= y;
		} while (digit > 0);
		converted.resize(numDigit * source.size());
		for (int ii = 0; ii < source.size(); ii++)
		{
			carry = source[ii];
			for (int jj = numDigit - 1; jj >= 0; jj--)
			{
				converted[(ii * numDigit) + jj] = carry % y;
				carry /= y;
			}
		}
	}
	else
	{
		carry = y;
		do
		{
			numDigit++;
			carry /= x;
		} while (carry > 0);
		digit = source.size() % numDigit;
		if (digit != 0) { err("Blank spaces detected-jfunc.baseConvert"); }
		converted.resize(source.size() / numDigit);
		for (int ii = 0; ii < converted.size(); ii++)
		{
			digit = 0;
			for (int jj = 0; jj < numDigit; jj++)
			{
				digit += (source[(ii * numDigit) + (numDigit - 1 - jj)] * pow(x, jj));
			}
			converted[ii] = digit;
		}
	}
	return converted;
}
string JFUNC::bind(string& stmt0, vector<string>& params)
{
	// Replaces placeholders ('?') with parameter strings. Automatically adds single quotes.
	string temp;
	size_t pos1 = 0;
	int count = 0;
	while (pos1 < stmt0.size())
	{
		pos1 = stmt0.find('?', pos1 + 1);
		if (pos1 < stmt0.size())
		{
			count++;
		}
	}
	if (count != (int)params.size())
	{
		err("ERROR: parameter count mismatch-bind");
	}

	pos1 = 0;
	vector<string> dirt = { "[]" };
	string to_double = "'";
	for (int ii = 0; ii < count; ii++)
	{
		clean(params[ii], dirt, to_double);
		pos1 = stmt0.find('?', pos1);
		temp = "'" + params[ii] + "'";
		stmt0.replace(pos1, 1, temp);
	}

	return stmt0;
}
bool JFUNC::checkPercent(string& sNum)
{
	// Returns TRUE if sNum can be read as a number within [0, 100].
	double dTemp;
	try { dTemp = stod(sNum); }
	catch (invalid_argument) { return 0; }
	if (dTemp < 0.0 || dTemp > 100.0) { return 0; }
	return 1;
}
bool JFUNC::checkPercent(string& sNum, double tolerance)
{
	// Returns TRUE if sNum can be read as a number within [0 - tolerance, 100 + tolerance].
	double dTemp;
	try { dTemp = stod(sNum); }
	catch (invalid_argument) { return 0; }
	if (dTemp < 0.0 - tolerance || dTemp > 100.0 + tolerance) { return 0; }
	return 1;
}
bool JFUNC::checkPercent(vector<string>& list)
{
	// Returns TRUE if every element in the list can be read as a number within [0, 100].
	double dTemp;
	for (int ii = 0; ii < list.size(); ii++)
	{
		try { dTemp = stod(list[ii]); }
		catch (invalid_argument) { return 0; }
		if (dTemp < 0.0 || dTemp > 100.0) { return 0; }
	}
	return 1;
}
bool JFUNC::checkPercent(vector<string>& list, double tolerance)
{
	// Returns TRUE if every element in the list can be read as a number within [0 - tolerance, 100 + tolerance].
	double dTemp;
	for (int ii = 0; ii < list.size(); ii++)
	{
		try { dTemp = stod(list[ii]); }
		catch (invalid_argument) { return 0; }
		if (dTemp < 0.0 - tolerance || dTemp > 100.0 + tolerance) { return 0; }
	}
	return 1;
}
int JFUNC::clean(string& bbq, vector<string> dirt)
{
	int count = 0;
	size_t pos1, len;
	for (int ii = 0; ii < dirt.size(); ii++)
	{
		len = dirt[ii].size();
		pos1 = bbq.find(dirt[ii]);
		while (pos1 < bbq.size())
		{
			bbq.erase(pos1, len);
			pos1 = bbq.find(dirt[ii], pos1);
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
int JFUNC::clean(string& bbq, vector<string> dirt, string twins)
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
int JFUNC::clean(string& bbq, vector<string> dirt, vector<string> soap)
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
vector<vector<string>> JFUNC::compareList(vector<string>& list0, vector<string>& list1)
{
	vector<vector<string>> difference(2, vector<string>());
	vector<bool> checked;
	if (list0.size() < 1)
	{
		difference[1] = list1;
		return difference;
	}
	else if (list1.size() < 1)
	{
		difference[0] = list0;
		return difference;
	}
	checked.assign(list1.size(), 0);
	for (int ii = 0; ii < list0.size(); ii++)
	{
		for (int jj = 0; jj < list1.size(); jj++)
		{
			if (list0[ii] == list1[jj])
			{
				checked[jj] = 1;
				break;
			}
			else if (jj == list1.size() - 1)
			{
				difference[0].push_back(list0[ii]);
			}
		}
	}
	for (int ii = 0; ii < checked.size(); ii++)
	{
		if (!checked[ii])
		{
			difference[1].push_back(list1[ii]);
		}
	}
	return difference;
}
vector<vector<string>> JFUNC::compareList(vector<vector<string>>& list0, vector<vector<string>>& list1, vector<int>& activeColumn)
{
	vector<vector<string>> difference(2, vector<string>());
	vector<bool> checked;
	if (list0.size() < 1)
	{
		difference[1].resize(list1.size());
		for (int ii = 0; ii < list1.size(); ii++)
		{
			difference[1][ii] = list1[ii][activeColumn[1]];
		}
		return difference;
	}
	else if (list1.size() < 1)
	{
		difference[0].resize(list0.size());
		for (int ii = 0; ii < list0.size(); ii++)
		{
			difference[0][ii] = list0[ii][activeColumn[0]];
		}
		return difference;
	}
	checked.assign(list1.size(), 0);
	for (int ii = 0; ii < list0.size(); ii++)
	{
		for (int jj = 0; jj < list1.size(); jj++)
		{
			if (list0[ii][activeColumn[0]] == list1[jj][activeColumn[1]])
			{
				checked[jj] = 1;
				break;
			}
			else if (jj == list1.size() - 1)
			{
				difference[0].push_back(list0[ii][activeColumn[0]]);
			}
		}
	}
	for (int ii = 0; ii < checked.size(); ii++)
	{
		if (!checked[ii])
		{
			difference[1].push_back(list1[ii][activeColumn[1]]);
		}
	}
	return difference;
}
int JFUNC::countChar(string& bbq, char target)
{
	// Returns the number of times the target char is found within the string.
	int count = 0;
	size_t pos1 = bbq.find(target);
	while (pos1 < bbq.size())
	{
		count++;
		pos1 = bbq.find(target, pos1 + 1);
	}
	return count;
}
string JFUNC::decToHex(int idec)
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
	if (shex.size() == 1) { shex.insert(shex.begin(), '0'); }
	return shex;
}
string JFUNC::decToHex(unsigned char ucdec)
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
	if (shex.size() == 1) { shex.insert(shex.begin(), '0'); }
	return shex;
}
vector<int> JFUNC::destringifyCoord(string& sCoord)
{
	vector<int> coord;
	size_t pos1 = sCoord.find(',');
	if (pos1 > sCoord.size()) { err("No comma-jf.destringifyCoord"); }
	string temp; 
	size_t pos2 = 0;
	while (pos1 < sCoord.size())
	{
		temp = sCoord.substr(pos2, pos1 - pos2);
		try { coord.push_back(stoi(temp)); }
		catch (invalid_argument& ia) { err("stoi-jf.destringifyCoord"); }
		pos2 = pos1 + 1;
		pos1 = sCoord.find(',', pos2);
	}
	temp = sCoord.substr(pos2);
	try { coord.push_back(stoi(temp)); }
	catch (invalid_argument& ia) { err("stoi-jf.destringifyCoord"); }
	return coord;
}
vector<double> JFUNC::destringifyCoordD(string& sCoord)
{
	vector<double> coord;
	size_t pos1 = sCoord.find(','), pos2 = 0;
	if (pos1 > sCoord.size()) { err("No comma-jf.destringifyCoordD"); }
	string temp;
	while (pos1 < sCoord.size())
	{
		temp = sCoord.substr(pos2, pos1 - pos2);
		try { coord.push_back(stod(temp)); }
		catch (invalid_argument) { err("stod-jf.destringifyCoordD"); }
		pos2 = pos1 + 1;
		pos1 = sCoord.find(',', pos2);
	}
	temp = sCoord.substr(pos2);
	try { coord.push_back(stod(temp)); }
	catch (invalid_argument) { err("stod-jf.destringifyCoordD"); }
	return coord;
}
string JFUNC::doubleToCommaString(double dNum)
{
	// Every third digit left of the decimal point is separated by a comma. Uses the 
	// default number of decimal digits after the decimal point. 
	string sNum = doubleToCommaString(dNum, defaultDecimalPlaces);
	return sNum;
}
string JFUNC::doubleToCommaString(double dNum, int decimalPlaces)
{
	// Every third digit left of the decimal point is separated by a comma.
	string temp;
	string sNum = to_string(dNum);
	int sNumSize = sNum.size();
	int numDecPlaces, iNum;
	size_t posDot = sNum.find('.');
	if (posDot > sNumSize) { numDecPlaces = 0; }
	else { numDecPlaces = sNumSize - posDot - 1; }

	// Do rounding or padding, as necessary.
	if (numDecPlaces > decimalPlaces)  
	{
		temp = sNum[posDot + decimalPlaces + 1];
		iNum = stoi(temp);
		if (iNum >= 5)
		{
			temp = sNum[posDot + decimalPlaces];
			iNum = stoi(temp);
			iNum++;
		}
		if (decimalPlaces > 0) { sNum.resize(posDot + decimalPlaces + 1); }
		else { sNum.resize(posDot + decimalPlaces); }
		numDecPlaces = decimalPlaces;
	}
	while (numDecPlaces < decimalPlaces)
	{
		if (posDot > sNum.size()) 
		{ 
			posDot = sNum.size();
			sNum.push_back('.'); 
		}
		sNum.push_back('0');
		numDecPlaces++;
	}

	// Add commas to every third digit left of the decimal point.
	temp = ",";
	int posComma = (int)posDot - 3;
	while (posComma > 0)
	{
		sNum.insert(posComma, temp);
		posComma -= 3;
	}

	return sNum;
}
vector<string> JFUNC::doubleToCommaString(vector<double> vdNum, int decimalPlaces)
{
	// Every third digit left of the decimal point is separated by a comma.
	vector<string> vsNum(vdNum.size());
	string temp, sNum;
	size_t posDot;
	int sNumSize, numDecPlaces, iNum, posDigit;

	for (int ii = 0; ii < vdNum.size(); ii++)
	{
		sNum = to_string(vdNum[ii]);
		sNumSize = sNum.size();
		posDot = sNum.find('.');
		if (posDot > sNumSize) { numDecPlaces = 0; }
		else { numDecPlaces = sNumSize - posDot - 1; }

		// Do rounding, if necessary.
		if (numDecPlaces > decimalPlaces)
		{
			temp = sNum[posDot + decimalPlaces + 1];
			iNum = stoi(temp);
			if (iNum >= 5)
			{
				if (decimalPlaces == 0) { posDigit = posDot - 1; }
				else { posDigit = posDot + decimalPlaces; }
				temp = sNum[posDigit];
				iNum = stoi(temp);
				iNum++;
				while (iNum == 10)
				{
					sNum[posDigit] = '0';
					if (sNum[posDigit - 1] == '.') { posDigit -= 2; }
					else { posDigit -= 1; }

					if (posDigit < 0) 
					{ 
						sNum.insert(0, "1"); 
						break;
					}
					else
					{
						temp = sNum[posDigit];
						iNum = stoi(temp);
						iNum++;
					}
				}
				if (posDigit >= 0) { sNum[posDigit] = 48 + iNum; }
			}
			sNum.resize(posDot + decimalPlaces + 1);
			numDecPlaces = decimalPlaces;
		}

		// Add commas to every third digit left of the decimal point.
		temp = ",";
		posDot = sNum.rfind('.');
		int posComma = (int)posDot - 3;
		while (posComma > 0)
		{
			sNum.insert(posComma, temp);
			posComma -= 3;
		}
		
		if (sNum.back() == '.') { sNum.pop_back(); }
		vsNum[ii] = sNum;
	}

	// Remove unnecessary trailing zeroes.
	bool letmeout = 0;
	while (!letmeout)
	{
		for (int ii = 0; ii < vsNum.size(); ii++)
		{
			if (vsNum[ii].back() != '0') 
			{ 
				letmeout = 1;
				if (vsNum[ii].back() == '.') { vsNum[ii].pop_back(); }
			}
		}
		if (!letmeout)
		{
			for (int ii = 0; ii < vsNum.size(); ii++)
			{
				vsNum[ii].pop_back();
			}
		}
	}
	return vsNum;
}
void JFUNC::err(string func)
{
	lock_guard<mutex> lock(m_err);
	ofstream ERR;
	ERR.open(error_path, ofstream::app);
	string message = timestamper() + " General error from " + func;
	ERR << message << endl << endl;
	ERR.close();
 	exit(EXIT_FAILURE);
}
string JFUNC::get_error_path()
{
	return error_path;
}
string JFUNC::getExtension(string& spath)
{
	size_t pos1 = spath.rfind('.');
	if (pos1 >= spath.size()) { err("No dot found-jf.getExtension"); }
	string ext = spath.substr(pos1);
	return ext;
}
int JFUNC::getPivot(vector<int>& treeSTrow)
{
	int pivot; 
	int iroot = -1;
	for (int ii = 0; ii < treeSTrow.size(); ii++)
	{
		if (treeSTrow[ii] < 0)
		{
			pivot = ii;
			return pivot;
		}
		else if (treeSTrow[ii] == 0) { iroot = ii; }
		
		if (ii == treeSTrow.size() - 1 && iroot >= 0) { return iroot; }
	}
	return -1;
}
vector<int> JFUNC::get_roots(vector<vector<int>>& tree_st)
{
	vector<int> roots;
	int pivot;
	for (int ii = 0; ii < tree_st.size(); ii++)
	{
		for (int jj = 0; jj < tree_st[ii].size(); jj++)
		{
			if (tree_st[ii][jj] < 0)
			{
				pivot = jj;
				break;
			}
			else if (jj == tree_st[ii].size() - 1)
			{
				pivot = 0;
			}
		}
		if (pivot == 0)
		{
			roots.push_back(ii);
		}
	}
	return roots;
}
vector<string> JFUNC::horizontalCentering(vector<string> vsList)
{
	// Given a list of strings, add enough spaces at the beginning of each string
	// so that the list appears horizontally-centered.
	int spaces, maxLength = 0;
	vector<int> viLength(vsList.size());
	for (int ii = 0; ii < vsList.size(); ii++)
	{
		viLength[ii] = vsList[ii].size();
		if (viLength[ii] > maxLength) { maxLength = viLength[ii]; }
	}
	for (int ii = 0; ii < vsList.size(); ii++)
	{
		spaces = (maxLength - viLength[ii]) / 2;
		vsList[ii].insert(0, spaces, ' ');
	}
	return vsList;
}
string JFUNC::intToCommaString(int iNum)
{
	// Every third digit from the end is separated by a comma. 
	string sNum = to_string(iNum);
	size_t len = sNum.size();
	if (len < 4) { return sNum; }
	int pos = len - 3;
	while (pos > 0)
	{
		sNum.insert(pos, ",");
		pos -= 3;
	}
	return sNum;
}
void JFUNC::isort_ilist(vector<int>& ilist, int type)
{
	int isize = ilist.size(), index;
	vector<int> viTemp;
	switch (type)
	{
	case 0:
	{
		quicksort(ilist, 0, isize - 1);
		break;
	}
	case 1:
	{
		viTemp = ilist;
		quicksort(viTemp, 0, isize - 1);
		for (int ii = 0; ii < isize; ii++)
		{
			index = isize - 1 - ii;
			ilist[index] = viTemp[ii];
		}
		break;
	}
	}
}
void JFUNC::isort_ilist(vector<string>& slist, int type)
{
	// type:  0 = ascending, 1 = descending
	int isize = slist.size(), index;
	vector<int> ilist(isize);
	for (int ii = 0; ii < isize; ii++)
	{
		try { ilist[ii] = stoi(slist[ii]); }
		catch (invalid_argument) { err("stoi-jf.isort_slist"); }
	}
	quicksort(ilist, 0, isize - 1);
	switch (type)
	{
	case 0:
	{
		for (int ii = 0; ii < isize; ii++)
		{
			slist[ii] = to_string(ilist[ii]);
		}
		break;
	}
	case 1:
	{
		for (int ii = 0; ii < isize; ii++)
		{
			index = isize - 1 - ii;
			slist[index] = to_string(ilist[ii]);
		}
		break;
	}
	}
}
int JFUNC::is_numeric(string& candidate)
{
	// Return 0 = not a number, 1 = integer, 2 = decimal

	int inum;
	double dnum;
	try
	{
		inum = stoi(candidate);
		return 1;
	}
	catch (invalid_argument& ia) {}
	try
	{
		dnum = stod(candidate);
		return 2;
	}
	catch (invalid_argument& ia) {}
	return 0;
}
vector<string> JFUNC::ivectorToSvector(vector<int>& ivec)
{
	vector<string> svec(ivec.size());
	for (int ii = 0; ii < ivec.size(); ii++)
	{
		svec[ii] = to_string(ivec[ii]);
	}
	return svec;
}
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
	temp1 = input.substr(pos1);
	output.push_back(temp1);
	return output;
}
string JFUNC::load(string file_path)
{
	// Load a file into memory as a string.

	// NOTE: Function fails to load non-ANSI names. 
	FILE* pFile = fopen(file_path.c_str(), "rb");
	if (pFile == NULL) { err("fopen-jf.load"); }
	fseek(pFile, 0, SEEK_END);
	int sizeFile = ftell(pFile);
	if (sizeFile < 0) { err("ftell-jf.load"); }
	fseek(pFile, 0, SEEK_SET);
	unsigned char* buffer = new unsigned char[sizeFile];
	size_t numChar = fread(buffer, 1, sizeFile, pFile);
	if (numChar != sizeFile) { err("fread-jf.load"); }
	fclose(pFile);

	int inum = 0;
	for (int ii = 0; ii < 8; ii++)
	{
		if (buffer[ii] == 0) { inum++; }
	}

	string output;
	if (buffer[0] + buffer[1] == 509)
	{
		// UTF-16
	}
	else if (inum == 4)
	{
		// UTF-16
	}
	else  // UTF-8 or ASCII
	{
		output.resize(sizeFile);
		for (int ii = 0; ii < sizeFile; ii++)
		{
			output[ii] = (char)buffer[ii];
		}
	}
	delete[] buffer;
	return output;
}
vector<unsigned char> JFUNC::loadBin(string filePath)
{
	// Load a file into memory as binary data. 

	// NOTE: Function fails to load non-ANSI names. 
	FILE* pFile = fopen(filePath.c_str(), "rb");
	if (pFile == NULL) { err("fopen-jf.loadBin"); }
	fseek(pFile, 0, SEEK_END);
	int sizeFile = ftell(pFile);
	if (sizeFile < 0) { err("ftell-jf.loadBin"); }
	fseek(pFile, 0, SEEK_SET);
	vector<unsigned char> buffer(sizeFile);
	size_t numChar = fread(&buffer[0], 1, sizeFile, pFile);
	if (numChar != sizeFile) { err("fread-jf.loadBin"); }
	fclose(pFile);
	return buffer;
}
void JFUNC::log(string message)
{
	lock_guard<mutex> lock(m_err);
	ofstream LOG;
	LOG.open(log_path, ofstream::app);
	string output = timestamper() + message;
	LOG << output << endl << endl;
	LOG.close();
}
void JFUNC::logTime(string func, long long timer)
{
	lock_guard<mutex> lock(m_err);
	ofstream LOG;
	LOG.open(log_path, ofstream::app);
	string output = timestamper() + func + " completed in ";
	output += to_string(timer) + "ms.";
	LOG << output << endl << endl;
	LOG.close();
}
int JFUNC::maxNumCol(vector<vector<wstring>>& task)
{
	int count = 0;
	for (int ii = 0; ii < task.size(); ii++)
	{
		if (task[ii].size() > count) { count = task[ii].size(); }
	}
	return count;
}
vector<int> JFUNC::minMax(vector<double>& dList)
{
	vector<int> result = { 0, 0 };
	double min = dList[0];
	double max = dList[0];
	for (int ii = 1; ii < dList.size(); ii++)
	{
		if (dList[ii] < min)
		{
			min = dList[ii];
			result[0] = ii;
		}
		else if (dList[ii] > max)
		{
			max = dList[ii];
			result[1] = ii;
		}
	}
	return result;
}
vector<int> JFUNC::minMax(vector<double>* dList)
{
	vector<int> result = { 0, 0 };
	double dVal;
	double min = dList->at(0);
	double max = dList->at(0);
	for (int ii = 1; ii < dList->size(); ii++)
	{
		dVal = dList->at(ii);
		if (dVal < min)
		{
			min = dVal;
			result[0] = ii;
		}
		else if (dVal > max)
		{
			max = dVal;
			result[1] = ii;
		}
	}
	return result;
}
vector<int> JFUNC::minMax(vector<int>& iList)
{
	vector<int> result = { 0, 0 };
	int min = iList[0];
	int max = iList[0];
	for (int ii = 1; ii < iList.size(); ii++)
	{
		if (iList[ii] < min)
		{
			min = iList[ii];
			result[0] = ii;
		}
		else if (iList[ii] > max)
		{
			max = iList[ii];
			result[1] = ii;
		}
	}
	return result;
}
string JFUNC::nameFromPath(string& path)
{
	size_t pos1 = path.rfind('\\');
	if (pos1 > path.size()) { err("No backslash-jf.nameFromPath"); }
	string name = path.substr(pos1 + 1);
	return name;
}
void JFUNC::navigator(vector<vector<int>>& tree_st, vector<string>& tree_pl, vector<string>& tree_url, string& webpage, int id)
{
	// Recursive function used to make a tree of web URLs. Requires outside support to provide
	// complete webpages in string form. From the starting page, it will search through its 
	// layered search criteria and extract the objective data, or the required URLs to proceed
	// to the next tree layer and try again. Every generation of navigator will analyze a 
	// webpage given by its predecessor.
	// tree_st has form [node index][ancestors, ..., node, children, ...].
	// tree_pl has form [node index][extracted string 1, ...].

	// Ensure that the search criteria is loaded into object memory. 
	int inum, num_intervals;
	string line, temp;
	size_t pos1, pos2, pos_start, pos_stop;
	if (navigator_search.size() < 1)
	{
		line = load(navigator_asset_path);
		pos1 = line.find('$') + 1;
		pos_start = line.rfind('\n', pos1);
		pos_stop = line.find('\n', pos1);
		do
		{
			inum = navigator_search.size();
			navigator_search.push_back(vector<string>());
			pos1 = line.find('$', pos_start) + 1;
			if (pos1 > line.size()) { break; }
			pos2 = line.find('$', pos1);
			while (pos2 < pos_stop)
			{
				temp = line.substr(pos1, pos2 - pos1);
				navigator_search[inum].push_back(temp);
				pos1 = pos2 + 1;
				pos2 = line.find('$', pos1);
			}
			pos_start = pos_stop;
			pos_stop = line.find('\n', pos_start + 1);

		} while (pos_stop < line.size());
	}

	// Go through the list of search criteria, from leaf->trunk, until a match is found.
	for (int ii = 0; ii < navigator_search.size(); ii++)
	{
		pos1 = webpage.find(navigator_search[ii][0]);  
		if (pos1 < webpage.size())  
		{
			num_intervals = 1;  // Determine how many intervals we must scan from this webpage.
			for (int jj = ii + 1; jj < navigator_search.size(); jj++)
			{
				if (navigator_search[jj][0] == navigator_search[ii][0])
				{
					num_intervals++;
				}
				else
				{
					break;
				}
			}
			
			if (ii == 0)  // Leaf webpage.
			{
				for (int jj = ii; jj < ii + num_intervals; jj++)
				{
					pos_start = webpage.find(navigator_search[jj][1]);
					pos_stop = webpage.find(navigator_search[jj][2], pos_start);
					pos1 = webpage.find(navigator_search[jj][3], pos_start);
					while (pos1 < pos_stop)
					{
						pos2 = webpage.find(navigator_search[jj][4], pos1 + 1);
						temp = webpage.substr(pos1, pos2 - pos1);
						//tree_pl[id].push_back(temp);
					}
				}

			}
			else  // Branch webpage.
			{

			}
			break;
		}
		else if (ii == navigator_search.size() - 1) { err("Failed to match unique page criteria-jf.navigator"); }
	}

}
void JFUNC::navParser(string& sfile, vector<vector<string>>& search)
{
	size_t pos1, pos2, posNL1, posNL2;
	string sval;
	posNL1 = sfile.find('$') - 1;
	posNL2 = sfile.find('\n', posNL1 + 1);
	while (posNL2 < sfile.size())  // For every line in the file...
	{
		search.push_back(vector<string>());
		pos1 = sfile.find('$', posNL1);
		if (pos1 > sfile.size()) { break; }
		pos2 = sfile.find('$', pos1 + 1);
		while (pos2 < posNL2)  // For every entry on this line...
		{
			sval = sfile.substr(pos1 + 1, pos2 - pos1 - 1);
			search[search.size() - 1].push_back(sval);
			pos1 = pos2;
			pos2 = sfile.find('$', pos1 + 1);
		}
		posNL1 = posNL2;
		posNL2 = sfile.find('\n', posNL1 + 1);
	}
}
string JFUNC::numericToCommaString(string sNumeric)
{
	// sNumeric must be a string representing an int or double. 
	string sNum;
	int iNum;
	double dNum;
	size_t pos1 = sNumeric.find('.');
	if (pos1 < sNumeric.size())
	{
		try { dNum = stod(sNumeric); }
		catch (invalid_argument) { err("stod-jf.numericToCommaString"); }
		sNum = doubleToCommaString(dNum);
	}
	else
	{
		try { iNum = stoi(sNumeric); }
		catch (invalid_argument) { err("stoi-jf.numericToCommaString"); }
		sNum = intToCommaString(iNum);
	}
	return sNum;
}
string JFUNC::numericToCommaString(string sNumeric, int mode)
{
	// sNumeric must be a string representing an int or double. 
	// This variant allows for a specific conversion type:
	// mode 0 = int, mode 1 = double
	string sNum;
	int iNum;
	double dNum;
	switch (mode)
	{
	case 0:
		try { iNum = stoi(sNumeric); }
		catch (invalid_argument) { err("stoi-jf.numericToCommaString"); }
		sNum = intToCommaString(iNum);
		break;
	case 1:
		try { dNum = stod(sNumeric); }
		catch (invalid_argument) { err("stod-jf.numericToCommaString"); }
		sNum = doubleToCommaString(dNum);
		break;
	}
	return sNum;
}
string JFUNC::parent_from_marker(string& child, char marker)
{
	size_t pos1 = child.rfind(marker);
	pos1 = child.find_last_of(marker, pos1);
	string parent = child.substr(0, pos1);
	return parent;
}
void JFUNC::pngRead(string& pathPNG)
{
	int bbq = 1;
}
void JFUNC::printer(string path, string& sfile)
{
	ofstream PR;
	locale utf8 = locale("en_US.UTF8");
	PR.imbue(utf8);
	PR.open(path, ios_base::binary | ios_base::trunc);
	PR << sfile << endl;
	PR.close();
}
void JFUNC::printer(string path, wstring& wfile)
{
	string sfile = utf16to8(wfile);
	printer(path, sfile);
}
void JFUNC::printer(string path, vector<unsigned char>& binFile)
{
	size_t count = binFile.size();
	FILE* pFile = fopen(path.c_str(), "wb");
	if (!pFile) { err("fopen-jf.printer"); }
	size_t numBytes = fwrite(&binFile[0], 1, count, pFile);
	if (numBytes != count) { err("fwrite-jf.printer"); }
	fclose(pFile);
}
void JFUNC::quicksort(vector<int>& v1, int low, int high)
{
	auto partition = [](vector<int>& v1, int low, int high)
	{
		int midpoint = ((high - low) / 2) + low;
		unsigned long long pivot = v1[midpoint];
		int ii = low - 1;
		unsigned long long tempnum = v1[high];
		v1[high] = v1[midpoint];
		v1[midpoint] = tempnum;
		for (int jj = low; jj <= high - 1; jj++)
		{
			if (v1[jj] < pivot)
			{
				ii++;
				tempnum = v1[jj];
				v1[jj] = v1[ii];
				v1[ii] = tempnum;
			}
		}
		tempnum = v1[high];
		v1[high] = v1[ii + 1];
		v1[ii + 1] = tempnum;
		return ii + 1;
	};

	if (low < high)
	{
		int pivotindex = partition(v1, low, high);
		quicksort(v1, low, pivotindex - 1);
		quicksort(v1, pivotindex + 1, high);
	}
}
void JFUNC::removeBlanks(vector<string>& task)
{
	// Only removes the blank entries at the end of the vector.
	while (task.back() == "")
	{
		task.pop_back();
	}
}
void JFUNC::removeBlanks(vector<vector<string>>& task)
{
	// Only removes the blank entries at the end of each row vector.
	for (int ii = 0; ii < task.size(); ii++)
	{
		while (task[ii].back() == "")
		{
			task[ii].pop_back();
		}
	}
}
void JFUNC::removeBlanks(vector<vector<wstring>>& task)
{
	// Only removes the blank entries at the end of each row vector.
	for (int ii = 0; ii < task.size(); ii++)
	{
		while (task[ii].back() == L"")
		{
			task[ii].pop_back();
		}
	}
}
void JFUNC::removeBlanks(vector<vector<vector<string>>>& task)
{
	// Only removes the blank entries at the end of each row vector.
	for (int ii = 0; ii < task.size(); ii++)
	{
		for (int jj = 0; jj < task[ii].size(); jj++)
		{
			while (task[ii][jj].back() == "")
			{
				task[ii][jj].pop_back();
			}
		}
	}
}
vector<double> JFUNC::rgbxToDouble(vector<unsigned char>& vRGBX)
{
	vector<double> vdRGBX(vRGBX.size());
	for (int ii = 0; ii < vdRGBX.size(); ii++)
	{
		vdRGBX[ii] = (double)vRGBX[ii] / 255.0;
	}
	return vdRGBX;
}
vector<double> JFUNC::rgbxToDouble(vector<int>& vRGBX)
{
	vector<double> vdRGBX(vRGBX.size());
	for (int ii = 0; ii < vdRGBX.size(); ii++)
	{
		vdRGBX[ii] = (double)vRGBX[ii] / 255.0;
	}
	return vdRGBX;
}
void JFUNC::set_navigator_asset_path(string& path)
{
	navigator_asset_path = path;
}
void JFUNC::sleep(int ms)
{
	this_thread::sleep_for(chrono::milliseconds(ms));
}
void JFUNC::sortAlphabetically(vector<string>& vsList)
{
	int count = 1, compare;
	while (count > 0)
	{
		count = 0;
		for (int ii = 0; ii < vsList.size() - 1; ii++)
		{
			compare = vsList[ii].compare(vsList[ii + 1]);
			if (compare > 0)
			{
				vsList[ii].swap(vsList[ii + 1]);
				count++;
			}
		}
	}
}
void JFUNC::sortAlphabetically(vector<vector<string>>& vvsList, int iCol)
{
	// Use the given column to sort the entire 2D vector, keeping rows intact.
	int count = 1, compare;
	while (count > 0)
	{
		count = 0;
		for (int ii = 0; ii < vvsList.size() - 1; ii++)
		{
			compare = vvsList[ii][iCol].compare(vvsList[ii + 1][iCol]);
			if (compare > 0)
			{
				vvsList[ii].swap(vvsList[ii + 1]);
				count++;
			}
		}
	}
}
vector<string> JFUNC::splitByMarker(string& text, char marker)
{
	vector<string> vsText;
	int index;
	size_t pos1 = 0;
	size_t pos2 = text.find(marker);
	while (pos2 < text.size())
	{
		index = vsText.size();
		vsText.push_back(text.substr(pos1, pos2 - pos1));
		while (vsText[index].back() == ' ') { vsText[index].pop_back(); }
		pos1 = text.find_first_not_of(' ', pos2 + 1);
		if (pos1 > text.size()) { break; }
		pos2 = text.find(marker, pos2 + 1);
	}
	if (pos2 > pos1)
	{
		index = vsText.size();
		vsText.push_back(text.substr(pos1));
		while (vsText[index].back() == ' ') { vsText[index].pop_back(); }
	}
	return vsText;
}
vector<vector<string>> JFUNC::splitByMarker(vector<string>& vsText, char marker)
{
	vector<vector<string>> vvsText(vsText.size(), vector<string>());
	for (int ii = 0; ii < vsText.size(); ii++)
	{
		vvsText[ii] = splitByMarker(vsText[ii], marker);
	}
	return vvsText;
}
string JFUNC::stringifyCoord(vector<int>& coord)
{
	if (coord.size() < 2) { err("coord format-jf.stringifyCoord"); }
	string sCoord;
	for (int ii = 0; ii < coord.size(); ii++)
	{
		sCoord += to_string(coord[ii]) + ",";
	}
	sCoord.pop_back();
	return sCoord;
}
string JFUNC::stringifyCoord(vector<unsigned char>& coord)
{
	if (coord.size() < 2) { err("coord format-jf.stringifyCoord"); }
	string sCoord;
	for (int ii = 0; ii < coord.size(); ii++)
	{
		sCoord += to_string(coord[ii]) + ",";
	}
	sCoord.pop_back();
	return sCoord;
}
vector<int> JFUNC::svectorToIvector(vector<string>& svec)
{
	vector<int> ivec(svec.size());
	for (int ii = 0; ii < svec.size(); ii++)
	{
		try { ivec[ii] = stoi(svec[ii]); }
		catch (out_of_range& oor) { err("stoi-jfunc.svectorToIvector"); }
	}
	return ivec;
}
void JFUNC::tclean(string& bbq, char marker, string preferred)
{
	// Function to replace characters from the start of a string (table indents).

	size_t pos1 = 0;
	size_t psize = preferred.size();
	while (bbq[pos1] == marker)
	{
		bbq.replace(pos1, 1, preferred);
		pos1 += psize;
	}
}
bool JFUNC::testInt(string sNum)
{
	// Returns TRUE if the string can be converted into an integer. 
	int iNum;
	try { iNum = stoi(sNum); }
	catch (invalid_argument) { return 0; }
	return 1;
}
vector<string> JFUNC::textParser(string& sfile, vector<string>& search)
{
	// search[0] is a sanity check for the page
	// search[1] is the start boundary for all extractions
	// search[2] is the end boundary for all extractions
	// search[3] is the start point for an individual extraction
	// search[4] is the end point for an individual extraction
	vector<string> gold;
	string temp;
	size_t pos1, pos2, pos_start, pos_stop;
	pos1 = sfile.find(search[0]);
	if (pos1 > sfile.size()) { err("unique marker-jfunc.textParser"); }
	pos_start = sfile.find(search[1]);
	pos_stop = sfile.find(search[2], pos_start);
	pos1 = sfile.find(search[3], pos_start);
	while (pos1 < pos_stop)
	{
		pos2 = sfile.find(search[4], pos1);
		if (pos2 > pos_stop) { break; }  // Safety.
		temp = sfile.substr(pos1 + search[3].size(), pos2 - pos1 - search[3].size());
		gold.push_back(temp);
		pos1 = sfile.find(search[3], pos2);
	}
	if (gold.size() < 1) { err("zero results-jfunc.textParser"); }
	return gold;
}
void JFUNC::timerStart()
{
	t1 = chrono::high_resolution_clock::now();
}
long long JFUNC::timerReport()
{
	t2 = chrono::high_resolution_clock::now();  // steady_clock uses ns, but can only update every 100ns.
	chrono::milliseconds d1 = chrono::duration_cast<chrono::milliseconds>(t2 - t1);
	long long timer = d1.count();
	return timer;
}
long long JFUNC::timerRestart()
{
	t2 = chrono::high_resolution_clock::now();  // steady_clock uses ns, but can only update every 100ns.
	chrono::milliseconds d1 = chrono::duration_cast<chrono::milliseconds>(t2 - t1);
	long long timer = d1.count();
	t1 = chrono::high_resolution_clock::now();
	return timer;
}
long long JFUNC::timerStop()
{
	t2 = chrono::high_resolution_clock::now();  // steady_clock uses ns, but can only update every 100ns.
	chrono::milliseconds d1 = chrono::duration_cast<chrono::milliseconds>(t2 - t1);
	long long timer = d1.count();
	return timer;
}
string JFUNC::timestamper()
{
	string timestamp;
	chrono::system_clock::time_point today = chrono::system_clock::now();
	time_t tt = chrono::system_clock::to_time_t(today);
	char* buffer = ctime(&tt);
	for (int ii = 0; ii < 26; ii++)
	{
		if (buffer[ii] == '\0') { break; }
		else { timestamp.push_back(buffer[ii]); }
	}
	return timestamp;
}
void JFUNC::toDouble(vector<int>& input, vector<double>& output)
{
	output.resize(input.size());
	for (int ii = 0; ii < input.size(); ii++)
	{
		output[ii] = (double)input[ii];
	}
}
void JFUNC::toDouble(vector<vector<int>>& input, vector<vector<double>>& output)
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
void JFUNC::toInt(vector<double>& input, vector<int>& output)
{
	output.resize(input.size());
	for (int ii = 0; ii < input.size(); ii++)
	{
		output[ii] = int(round(input[ii]));
	}
}
void JFUNC::toInt(vector<vector<double>>& input, vector<vector<int>>& output)
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
void JFUNC::uptick(vector<int>& viCounter, vector<int> viMax)
{
	// Increases viCounter by its smallest increment, and uses viMax for rollover.
	if (viCounter.size() != viMax.size()) { err("Size mismatch-jf.uptick"); }
	viCounter[viCounter.size() - 1]++;
	for (int ii = viCounter.size() - 1; ii >= 0; ii--)
	{
		if (viCounter[ii] > viMax[ii]) { err("Beyond maximum-jf.uptick"); }
		else if (viCounter[ii] == viMax[ii])
		{
			viCounter[ii] = 0;
			if (ii > 0) { viCounter[ii - 1]++; }
		}
	}
}
string JFUNC::utf16to8(wstring input)
{
	UTF16clean(input);
	mbstate_t mb{};
	locale utf8 = locale("en_US.UTF8");
	auto& f = use_facet<codecvt<wchar_t, char, mbstate_t>>(utf8);
	size_t len = input.size();
	string output;
	output.assign(len * 2, 0);
	const wchar_t* past;
	char* future;
	f.out(mb, &input[0], &input[input.size()], past, &output[0], &output[output.size()], future);
	output.resize(future - &output[0]);
	return output;
}
wstring JFUNC::utf8to16(string input)
{
	mbstate_t mb{};
	locale utf8 = locale("en_US.UTF8");
	auto& f = use_facet<codecvt<wchar_t, char, mbstate_t>>(utf8);
	wstring output(input.size() * 2, L'\0');
	const char* past;
	wchar_t* future;
	f.in(mb, &input[0], &input[input.size()], past, &output[0], &output[output.size()], future);
	output.resize(future - &output[0]);
	return output;
}
string JFUNC::utf8ToAscii(string input)
{
	size_t pos = input.find(-61);
	if (pos > input.size()) { return input; }
	while (pos < input.size())
	{
		if (pos == input.size() - 1) { err("Scrambled UTF8-jf.utf8ToAscii"); }
		if (input[pos + 1] >= 0)
		{
			if (input[pos + 1] == 63)
			{
				input[pos] = -1 * input[pos + 1];
				input.erase(input.begin() + pos + 1);
			}
			else { err("Scrambled UTF8-jf.utf8ToAscii"); }
		}
		else
		{
			input[pos] = input[pos + 1] + 64;
			input.erase(input.begin() + pos + 1);
		}
		pos = input.find(-61, pos + 1);
	}
	return input;
}
void JFUNC::UTF16clean(wstring& ws)
{
	wchar_t wChar;
	bool fixme = 0;
	size_t len = 2, sample = 4;
	wstring wTemp;
	for (size_t ii = 0; ii < ws.size() - 1; ii++)
	{
		if (ws[ii] == 195)
		{
			if (ii + sample >= ws.size()) 
			{
				sample = ws.size() - ii - 1; 
				fixme = 1;
			}
			wTemp.assign(ws.begin() + ii, ws.begin() + ii + sample);
			if (fixme == 0 && wTemp[2] == 194)
			{
				ws[ii] = wTemp[3] + 64;
				ws.erase(ws.begin() + ii + 1, ws.begin() + ii + sample);
			}
			else
			{
				ws[ii] = ws[ii + 1] + 64;
				ws.erase(ws.begin() + ii + 1);
			}
			if (fixme) { sample = 4; fixme = 0; }
		}
	}
}
int JFUNC::xDom(double angle)
{
	// NOTE: Angle is measured in degrees, starting from north, 
	// and travelling clockwise. Returns 1 = xCoord dominant, 
	// 0 = yCoord dominant, -1 = neither is dominant.
	if (angle < 0.0 || angle >= 360.0) { err("angle out of bounds-jf.xDom"); }

	if (angle < 45.0) { return 0; }
	else if (angle > 45.0 && angle < 135.0) { return 1; }
	else if (angle > 135.0 && angle < 225.0) { return 0; }
	else if (angle > 225.0 && angle < 315.0) { return 1; }
	else if (angle > 315.0) { return 0; }

	return -1;
}
