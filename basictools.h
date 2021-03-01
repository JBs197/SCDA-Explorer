#pragma once
#ifndef BASICTOOLS_H
#define BASICTOOLS_H

#include <windows.h>
#include <wininet.h>
#include <winuser.h>
#include <thread>
#include <locale>
#include <QtSql>

#pragma comment(lib, "wininet.lib")
#pragma comment(lib,"user32.lib")
using namespace std;

typedef struct {
    HWND       hWindow;
    int        nStatusList;
    HINTERNET hresource;
    char szMemo[512];
} REQUEST_CONTEXT;

// Text encoder conversion functions.
string utf16to8(wstring);
wstring utf8to16(string);

// Return a timestamp from the system clock.
string timestamperA();

void err_bt(string);
void winerr_bt(string);

// Determine the type of number contained within the given string. 0 = error, 1 = int, 2 = double.
int qnum_test(QString);

// Return a piece of the original vector, defined by the first and last positions.
QVector<QString> string_vector_slicer(QVector<QString>&, int, int);

// Remove problematic chars from a string.
int qclean(QString&, int);
int sclean(string&, int);

// From a given file path, writes that file's full content to the referenced (empty) string.
// Currently, can accomodate UTF8 strings, UTF16 wstrings, and QStrings.
wstring bin_memory(HANDLE&);
QString q_memory(wstring&);
wstring w_memory(wstring&);
string s_memory(wstring&);

// Contains pre-programmed responses to certain automated events during server-client communications.
void CALLBACK call(HINTERNET, DWORD_PTR, DWORD, LPVOID, DWORD);

// Given destination folder and filename, will download the file at the URL.
int download(wstring, wstring, wstring);

// Given a folder path, return a vector containing the file paths of all files within. Does not list subfolders.
vector<wstring> get_file_paths(wstring);
vector<wstring> get_file_path_endings(wstring, size_t);
int get_file_path_number(wstring, wstring);

// Given a root folder path, return a vector containing the full paths of all subfolders within.
vector<wstring> get_subfolders(wstring);
vector<vector<wstring>> get_subfolders2(wstring);

// Search a given vector for a particular value, and return the index. If not found, insert and return.
// Returns negative (error) if a new entry fails to match and is not the largest entry.
int index_card(vector<int>&, int);

// Returns a CSV's data rows.
QVector<QVector<QString>> extract_csv_data_rows(QString&, QVector<QString>, bool);


#endif
