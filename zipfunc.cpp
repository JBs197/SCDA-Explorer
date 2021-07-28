#include "stdafx.h"
#include "zipfunc.h"

void ZIPFUNC::unzip(string& zipPath)
{
	int error;
	unsigned char* buffer;
	size_t pos1 = zipPath.rfind('\\');
	string folderPath = zipPath.substr(0, pos1);
	string zName, uzPath;
	FILE* myFile;
	zip_file_t* zipFile;
	zip_stat_t zipStats;
	zip_stat_init(&zipStats);
	zip_t* zipArchive = zip_open(zipPath.c_str(), NULL, &error);
	zip_uint64_t numFiles = zip_get_num_entries(zipArchive, NULL);
	zip_uint64_t sizeUnCompressed, bytesRead, bufSize;
	for (zip_uint64_t ii = 0; ii < numFiles; ii++)
	{
		error = zip_stat_index(zipArchive, ii, NULL, &zipStats);
		if (error < 0) { jf.err("zip_stat_index-zf.unzip"); }
		zName = zipStats.name;
		uzPath = folderPath + "\\" + zName;
		zipFile = zip_fopen_index(zipArchive, ii, NULL);
		sizeUnCompressed = zipStats.size;
		if (sizeUnCompressed <= maxBuff)
		{
			buffer = new unsigned char[sizeUnCompressed];
			bytesRead = zip_fread(zipFile, buffer, sizeUnCompressed);
			if (bytesRead < 0) { jf.err("zip_fread-zf.unzip"); }
			myFile = fopen(uzPath.c_str(), "wb");
			if (!myFile) { jf.err("fopen-zf.unzip"); }
			pos1 = fwrite(buffer, 1, sizeUnCompressed, myFile);
		}
		else
		{
			buffer = new unsigned char[maxBuff];
			myFile = fopen(uzPath.c_str(), "wb");
			if (!myFile) { jf.err("fopen-zf.unzip"); }
			bytesRead = 1;
			while (bytesRead > 0)
			{
				bytesRead = zip_fread(zipFile, buffer, maxBuff);
				if (bytesRead < 0) { jf.err("zip_fread-zf.unzip"); }
				pos1 = fwrite(buffer, 1, bytesRead, myFile);
			}
		}
		fclose(myFile);
		delete[] buffer;
	}
}
