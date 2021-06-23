#include "iofunc.h"

bool IOFUNC::askYesNo(QPlainTextEdit*& box, string question)
{
	QString qMessage = QString::fromStdString(question);
	box->setPlainText(qMessage);
	QCoreApplication::processEvents();
	while (1)
	{
		Sleep(50);
		if (GetAsyncKeyState(89)) { return 1; }  // 'y'
		else if (GetAsyncKeyState(78)) { return 0; }  // 'n'
		else if (GetAsyncKeyState(VK_ESCAPE)) { exit(EXIT_FAILURE); }
		QCoreApplication::processEvents();
	}
	return 0;
}
string IOFUNC::copyText()
{
	// Returns a string from selected text, as if ctrl+c had been used.
	kbHoldPress(VK_CONTROL, 67);  // ctrl+c
	BOOL success = OpenClipboard(NULL);
	if (!success) { winerr("OpenClipboard-io.copyText"); }
	HANDLE hClip = GetClipboardData(CF_TEXT);
	if (hClip == NULL) { winerr("GetClipboardData-io.copyText"); }
	//size_t bufSize = GlobalSize(hClip);
	//char* buffer = (char*)GlobalLock(hClip);
	string sData = (string)static_cast<char*>(hClip);
	if (sData.size() < 1) { jf.err("Copying from clipboard to string-io.copyText"); }
	success = EmptyClipboard();
	if (!success) { winerr("EmptyClipboard-io.copyText"); }
	success = CloseClipboard();
	if (!success) { winerr("CloseClipboard-io.copyText"); }
	return sData;
}
vector<double> IOFUNC::getBinPosition(QPlainTextEdit*& box, string regionName, string sParent)
{
	// Note that the region and parent names should be in ASCII !
	QString qMessage;
	POINT pSearch, pButton, pResult;
	pSearch.x = -1;
	int numCoord = gpsWebpage.size();
	if (numCoord == 3 && reuse == 1)
	{
		pSearch = gpsWebpage[0];
		pButton = gpsWebpage[1];
		pResult = gpsWebpage[2];
	}
	else if (numCoord == 3 && reuse == 0)
	{
		bool yesno = askYesNo(box, "(y/n): Do you want to continue using the previous set of coordinates?");
		if (yesno)
		{
			pSearch = gpsWebpage[0];
			pButton = gpsWebpage[1];
			pResult = gpsWebpage[2];
			reuse = 1;
		}
		else { reuse = -1; }
	}
	if (pSearch.x < 0)
	{
		qMessage = "NUM1: Position the cursor inside the search box.";
		box->setPlainText(qMessage);
		QCoreApplication::processEvents();
		pSearch = getCoord(VK_NUMPAD1);
		qMessage = "NUM2: Position the cursor over the search button.";
		box->setPlainText(qMessage);
		QCoreApplication::processEvents();
		pButton = getCoord(VK_NUMPAD2);
		qMessage = "NUM3: Position the cursor over the results box.";
		box->setPlainText(qMessage);
		QCoreApplication::processEvents();
		pResult = getCoord(VK_NUMPAD3);
		gpsWebpage.resize(3);
		gpsWebpage[0] = pSearch;
		gpsWebpage[1] = pButton;
		gpsWebpage[2] = pResult;
	}
	qMessage = "Working ... DO NOT TOUCH THE MOUSE OR KEYBOARD.";
	box->setPlainText(qMessage);
	QCoreApplication::processEvents();

	string query = regionName + " " + sParent;
	mouseClickTriple(pSearch);
	Sleep(50);
	kbInput(VK_BACK);
	Sleep(50);
	kbInput(query);
	Sleep(50);
	mouseClick(pButton);
	Sleep(4000);
	mouseClickTriple(pResult);
	Sleep(50);
	string gps = copyText();
	size_t pos1 = gps.find(',');
	string latitude = gps.substr(0, pos1);
	string longitude = gps.substr(pos1 + 1);
	vector<double> coord(2);
	try 
	{
		coord[0] = stod(latitude);
		coord[1] = stod(longitude);
	}
	catch (invalid_argument) { jf.err("stod-io.getBinPosition"); }
	qMessage = "Finished using the keyboard and mouse.";
	box->setPlainText(qMessage);
	QCoreApplication::processEvents();
	return coord;
}
POINT IOFUNC::getCoord(int vKey)
{
	// Wait until vKey is pressed, then return the absolute mouse coords.
	POINT p;
	while (1)
	{
		Sleep(50);
		if (GetAsyncKeyState(vKey))
		{
			GetCursorPos(&p);
			return p;
		}
		else if (GetAsyncKeyState(VK_ESCAPE)) { exit(EXIT_FAILURE); }
	}
}
QString IOFUNC::getQSCoord(POINT& p0)
{
	string sCoord = getSCoord(p0);
	QString qsCoord = QString::fromStdString(sCoord);
	return qsCoord;
}
string IOFUNC::getSCoord(POINT& p0)
{
	// Returns a string in the form   (x,y)
	string sCoord = "(" + to_string(p0.x) + "," + to_string(p0.y) + ")";
	return sCoord;
}
HWND IOFUNC::getWindow(int vKey)
{
	// Wait until vKey is pressed, then return a handle to the foreground window.
	HWND hWindow;
	while (1)
	{
		Sleep(50);
		if (GetAsyncKeyState(vKey))
		{
			hWindow = GetForegroundWindow();
			return hWindow;
		}
		else if (GetAsyncKeyState(VK_ESCAPE)) { exit(EXIT_FAILURE); }
	}
}
void IOFUNC::kbHoldPress(int holdKey, int pressKey)
{
	INPUT ipt[1];
	ipt[0].type = INPUT_KEYBOARD;
	ipt[0].ki.wVk = holdKey;
	UINT sent = SendInput(1, ipt, sizeof(INPUT));
	if (!sent) { jf.err("SendInput-io.kbHoldPress"); }
	Sleep(50);
	kbInput(pressKey);
	Sleep(50);
	ipt[1].ki.dwFlags = KEYEVENTF_KEYUP;
	sent = SendInput(1, ipt, sizeof(INPUT));
	if (!sent) { jf.err("SendInput-io.kbHoldPress"); }
}
void IOFUNC::kbInput(int vKey)
{
	if (vKey < 0 || vKey > 255) { jf.err("Invalid vKey-io.kbInput"); }
	INPUT ipt[2];
	ipt[0].type = INPUT_KEYBOARD;
	ipt[0].ki.wVk = vKey;
	ipt[1].type = INPUT_KEYBOARD;
	ipt[1].ki.wVk = vKey;
	ipt[1].ki.dwFlags = KEYEVENTF_KEYUP;
	UINT sent = SendInput(2, ipt, sizeof(INPUT));
	if (!sent) { jf.err("SendInput-io.kbInput"); }
}
void IOFUNC::kbInput(char ch)
{
	int vKey = VkKeyScanA(ch);
	kbInput(vKey);
}
void IOFUNC::kbInput(string str)
{
	// Untested for non-ASCII chars.
	for (int ii = 0; ii < str.size(); ii++)
	{
		kbInput(str[ii]);
		Sleep(20);
	}
}
void IOFUNC::mouseClick(POINT p1)
{
	LONG xC = p1.x * 65535 / defaultRes[0];
	LONG yC = p1.y * 65535 / defaultRes[1];
	INPUT ipt[3];
	ipt[0].type = INPUT_MOUSE;
	ipt[0].mi.dx = xC;
	ipt[0].mi.dy = yC;
	ipt[0].mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_VIRTUALDESK | MOUSEEVENTF_MOVE;
	ipt[1].type = INPUT_MOUSE;
	ipt[1].mi.dx = xC;
	ipt[1].mi.dy = yC;
	ipt[1].mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_VIRTUALDESK | MOUSEEVENTF_LEFTDOWN;
	ipt[2].type = INPUT_MOUSE;
	ipt[2].mi.dx = xC;
	ipt[2].mi.dy = yC;
	ipt[2].mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_VIRTUALDESK | MOUSEEVENTF_LEFTUP;

	UINT bbq = SendInput(3, ipt, sizeof(INPUT));
	if (!bbq) { jf.err("SendInput-io.mouseClick"); }
}
void IOFUNC::mouseClickKey(int vKey, POINT p0)
{
	// Wait until vKey is pressed, then click on the given point (activeWindow).
	while (1)
	{
		Sleep(50);
		if (GetAsyncKeyState(vKey))
		{
			mouseClick(p0);
			return;
		}
		else if (GetAsyncKeyState(VK_ESCAPE)) { exit(EXIT_FAILURE); }
	}
}
void IOFUNC::mouseClickTriple(POINT p1)
{
	LONG xC = p1.x * 65535 / defaultRes[0];
	LONG yC = p1.y * 65535 / defaultRes[1];
	INPUT ipt[7];
	ipt[0].type = INPUT_MOUSE;
	ipt[0].mi.dx = xC;
	ipt[0].mi.dy = yC;
	ipt[0].mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_VIRTUALDESK | MOUSEEVENTF_MOVE;
	ipt[0].mi.time = 500;
	ipt[1].type = INPUT_MOUSE;
	ipt[1].mi.dx = xC;
	ipt[1].mi.dy = yC;
	ipt[1].mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_VIRTUALDESK | MOUSEEVENTF_LEFTDOWN;
	ipt[2].type = INPUT_MOUSE;
	ipt[2].mi.dx = xC;
	ipt[2].mi.dy = yC;
	ipt[2].mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_VIRTUALDESK | MOUSEEVENTF_LEFTUP;
	ipt[3].type = INPUT_MOUSE;
	ipt[3].mi.dx = xC;
	ipt[3].mi.dy = yC;
	ipt[3].mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_VIRTUALDESK | MOUSEEVENTF_LEFTDOWN;
	ipt[4].type = INPUT_MOUSE;
	ipt[4].mi.dx = xC;
	ipt[4].mi.dy = yC;
	ipt[4].mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_VIRTUALDESK | MOUSEEVENTF_LEFTUP;
	ipt[5].type = INPUT_MOUSE;
	ipt[5].mi.dx = xC;
	ipt[5].mi.dy = yC;
	ipt[5].mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_VIRTUALDESK | MOUSEEVENTF_LEFTDOWN;
	ipt[6].type = INPUT_MOUSE;
	ipt[6].mi.dx = xC;
	ipt[6].mi.dy = yC;
	ipt[6].mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_VIRTUALDESK | MOUSEEVENTF_LEFTUP;

	UINT sent = SendInput(7, ipt, sizeof(INPUT));
	if (!sent) { jf.err("SendInput-io.mouseClickTriple"); }
}
