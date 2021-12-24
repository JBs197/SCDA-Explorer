#pragma once
#include <zip.h>
#include "jfunc.h"

using namespace std;

class ZIPFUNC
{
	JFUNC jf;
	zip_uint64_t maxBuff = 2000000000; // 2GB

public:
	explicit ZIPFUNC() {}
	~ZIPFUNC() {}
	void unzip(string& zipPath);

};

