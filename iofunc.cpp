#include "iofunc.h"

bool IOFUNC::askYesNo(string question)
{
	QString qMessage = QString::fromStdString(question);
	qDebug() << qMessage;
	while (1)
	{
		Sleep(50);
		if (GetAsyncKeyState(89)) { return 1; }  // 'y'
		else if (GetAsyncKeyState(78)) { return 0; }  // 'n'
		else if (GetAsyncKeyState(VK_ESCAPE)) { exit(EXIT_FAILURE); }
	}
	return 0;
}
string IOFUNC::copyText()
{
	HWND hActive = GetForegroundWindow();
	string sText = copyText(hActive);
	return sText;
}
string IOFUNC::copyText(HWND& targetWindow)
{
	// Returns a string from selected text, as if ctrl+c had been used.
	kbHoldPress(VK_CONTROL, (WORD)67, targetWindow);  // ctrl+c
	BOOL success = OpenClipboard(NULL);
	if (!success) { winerr("OpenClipboard-io.copyText"); }
	HANDLE hClip = GetClipboardData(CF_TEXT);
	if (hClip == NULL) { winerr("GetClipboardData-io.copyText"); }
	string sData = (string)static_cast<char*>(hClip);
	if (sData.size() < 1) { jf.err("Copying from clipboard to string-io.copyText"); }
	success = EmptyClipboard();
	if (!success) { winerr("EmptyClipboard-io.copyText"); }
	success = CloseClipboard();
	if (!success) { winerr("CloseClipboard-io.copyText"); }
	return sData;
}
vector<double> IOFUNC::getBinPosition(string regionName, string sParent)
{
	// Note that the region and parent names should be in ASCII !
	QString qMessage;
	string region1, region2, query;
	HWND targetWindow;
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
		bool yesno = askYesNo("(y/n): Do you want to continue using the previous set of coordinates?");
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
		qMessage = "NUM1: Position the cursor INSIDE the search box.";
		qDebug() << qMessage;
		pSearch = getCoord(VK_NUMPAD1);
		activeWindow = GetForegroundWindow();
		qMessage = "NUM2: Position the cursor over the search button.";
		qDebug() << qMessage;
		pButton = getCoord(VK_NUMPAD2);
		qMessage = "NUM3: Position the cursor over the results box.";
		qDebug() << qMessage;
		pResult = getCoord(VK_NUMPAD3);
		gpsWebpage.resize(3);
		gpsWebpage[0] = pSearch;
		gpsWebpage[1] = pButton;
		gpsWebpage[2] = pResult;
	}
	qMessage = "Working ... DO NOT TOUCH THE MOUSE OR KEYBOARD.";
	qDebug() << qMessage;

	size_t pos1 = regionName.find(" - ");
	if (pos1 < regionName.size())
	{
		region1 = regionName.substr(0, pos1);
		region2 = regionName.substr(pos1 + 3);
		vector<double> coord1 = getBinPosition(region1, sParent);
		vector<double> coord2 = getBinPosition(region2, sParent);
		vector<double> coord(2);
		coord[0] = (coord1[0] + coord2[0]) / 2.0;
		coord[1] = (coord1[1] + coord2[1]) / 2.0;
		return coord;
	}
	pos1 = sParent.find(" or ");
	if (pos1 < sParent.size())
	{
		sParent = sParent.substr(0, pos1);
	}
	pos1 = regionName.find(" or ");
	if (pos1 < regionName.size())
	{
		regionName = regionName.substr(0, pos1);
	}
	query = regionName + " " + sParent;
	jf.asciiNearestFit(query);

	mouseClickTriple(pSearch, targetWindow);
	if (targetWindow == NULL) { targetWindow = activeWindow; }
	Sleep(200);
	kbInput(VK_BACK, targetWindow);
	Sleep(50);
	kbInput(query, targetWindow);
	Sleep(50);
	kbInput(VK_RETURN, targetWindow);
	Sleep(50);
	mouseClick(pButton);
	Sleep(4000);
	mouseClick(pResult);
	Sleep(50);
	string gps = copyText(targetWindow);
	pos1 = gps.find(',');
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
	qDebug() << qMessage;
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
vector<POINT> IOFUNC::getUserTLBR(int vKey1, int vKey2)
{
	vector<POINT> TLBR;
	POINT p;
	while (1)
	{
		Sleep(50);
		if (TLBR.size() == 0 && GetAsyncKeyState(vKey1))
		{
			GetCursorPos(&p);
			TLBR.push_back(p);
			Sleep(1000);
		}
		else if (TLBR.size() == 1 && GetAsyncKeyState(vKey2))
		{
			GetCursorPos(&p);
			TLBR.push_back(p);
		}
		else if (GetAsyncKeyState(VK_ESCAPE)) { exit(EXIT_FAILURE); }
		if (TLBR.size() == 2) { break; }
	}
	return TLBR;
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
void IOFUNC::kbHoldPress(WORD holdKey, WORD pressKey)
{
	HWND hActive = GetForegroundWindow();
	kbHoldPress(holdKey, pressKey, hActive);
}
void IOFUNC::kbHoldPress(WORD holdKey, WORD pressKey, HWND& targetWindow)
{
	BOOL success = SetForegroundWindow(targetWindow);
	if (!success) { winerr("SetForegroundWindow-io.kbHoldPress"); }
	INPUT ipt[1] = {};
	ipt[0].type = INPUT_KEYBOARD;
	ipt[0].ki.wVk = holdKey;
	UINT sent = SendInput(1, ipt, sizeof(INPUT));
	if (!sent) { jf.err("SendInput-io.kbHoldPress"); }
	Sleep(50);
	kbInput(pressKey, targetWindow);
	Sleep(50);
	success = SetForegroundWindow(targetWindow);
	if (!success) { winerr("SetForegroundWindow-io.kbHoldPress"); }
	ipt[0].ki.dwFlags = KEYEVENTF_KEYUP;
	sent = SendInput(1, ipt, sizeof(INPUT));
	if (!sent) { jf.err("SendInput-io.kbHoldPress"); }
}
void IOFUNC::kbInput(WORD vKey)
{
	HWND hActive = GetForegroundWindow();
	kbInput(vKey, hActive);
}
void IOFUNC::kbInput(WORD vKey, HWND& targetWindow)
{
	//if (vKey < 0 || vKey > 255) { jf.err("Invalid vKey-io.kbInput"); }
	INPUT ipt[2] = {};
	ipt[0].type = INPUT_KEYBOARD;
	ipt[0].ki.wVk = vKey;
	ipt[0].ki.time = 50;
	ipt[1].type = INPUT_KEYBOARD;
	ipt[1].ki.wVk = vKey;
	ipt[1].ki.dwFlags = KEYEVENTF_KEYUP;
	ipt[1].ki.time = 150;
	BOOL success = SetForegroundWindow(targetWindow);
	if (!success) { winerr("SetForegroundWindow-io.kbInput"); }
	UINT sent = SendInput(2, ipt, sizeof(INPUT));
	if (!sent) { jf.err("SendInput-io.kbInput"); }
}
void IOFUNC::kbInput(string str)
{
	HWND hActive = GetForegroundWindow();
	kbInput(str, hActive);
}
void IOFUNC::kbInput(string str, HWND& targetWindow)
{
	// Untested for non-ASCII chars.
	SHORT vKey;
	for (int ii = 0; ii < str.size(); ii++)
	{
		vKey = VkKeyScanA(str[ii]);
		kbInput(vKey, targetWindow);
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
	if (bbq != 3) { jf.err("SendInput-io.mouseClick"); }
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
void IOFUNC::mouseClickTriple(POINT p1, HWND& targetWindow)
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
	ipt[1].mi.time = 550;
	ipt[2].type = INPUT_MOUSE;
	ipt[2].mi.dx = xC;
	ipt[2].mi.dy = yC;
	ipt[2].mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_VIRTUALDESK | MOUSEEVENTF_LEFTUP;
	ipt[2].mi.time = 600;
	ipt[3].type = INPUT_MOUSE;
	ipt[3].mi.dx = xC;
	ipt[3].mi.dy = yC;
	ipt[3].mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_VIRTUALDESK | MOUSEEVENTF_LEFTDOWN;
	ipt[3].mi.time = 650;
	ipt[4].type = INPUT_MOUSE;
	ipt[4].mi.dx = xC;
	ipt[4].mi.dy = yC;
	ipt[4].mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_VIRTUALDESK | MOUSEEVENTF_LEFTUP;
	ipt[4].mi.time = 700;
	ipt[5].type = INPUT_MOUSE;
	ipt[5].mi.dx = xC;
	ipt[5].mi.dy = yC;
	ipt[5].mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_VIRTUALDESK | MOUSEEVENTF_LEFTDOWN;
	ipt[5].mi.time = 750;
	ipt[6].type = INPUT_MOUSE;
	ipt[6].mi.dx = xC;
	ipt[6].mi.dy = yC;
	ipt[6].mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_VIRTUALDESK | MOUSEEVENTF_LEFTUP;
	ipt[6].mi.time = 800;

	UINT sent = SendInput(7, ipt, sizeof(INPUT));
	if (!sent) { jf.err("SendInput-io.mouseClickTriple"); }
	targetWindow = GetForegroundWindow();
}
void IOFUNC::mouseMove(POINT p1)
{
	LONG xC = p1.x * 65535 / defaultRes[0];
	LONG yC = p1.y * 65535 / defaultRes[1];
	INPUT ipt[1];
	ipt[0].type = INPUT_MOUSE;
	ipt[0].mi.dx = xC;
	ipt[0].mi.dy = yC;
	ipt[0].mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_VIRTUALDESK | MOUSEEVENTF_MOVE;

	UINT bbq = SendInput(1, ipt, sizeof(INPUT));
	if (!bbq) { jf.err("SendInput-io.mouseMove"); }
}
bool IOFUNC::signal(WORD vKey)
{
	// Wait until the chosen keystroke is registered, then return TRUE.
	while (1)
	{
		Sleep(50);
		if (GetAsyncKeyState(vKey)) { break; }
		else if (GetAsyncKeyState(VK_ESCAPE)) { exit(EXIT_FAILURE); }
	}
	return 1;
}
