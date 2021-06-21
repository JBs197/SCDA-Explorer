#pragma once
#include <zip.h>
#include "jfunc.h"

using namespace std;

class ZIPFUNC
{
	JFUNC jf;

public:
	explicit ZIPFUNC() {}
	~ZIPFUNC() {}
	void unzip(string& zipPath);

};

