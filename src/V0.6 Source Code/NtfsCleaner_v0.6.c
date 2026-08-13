#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <tlhelp32.h>
#include <stdio.h>

char operationLog[256];
int ByInjectPid = 0;
int WriteFileStatus = 0;
int SafeModeTrigger = 0;
HANDLE LogFile = INVALID_HANDLE_VALUE;
COLORREF logColor = RGB(255, 255, 255);

void FindJournal(HWND hwnd);
void UpdateLog(HWND hwnd, const char* text, COLORREF textColor);
void DeleteJournal(HWND hwnd);
void WriteLog(HWND hwnd, const char* text);
void CreateFileToWriteLog(HWND hwnd);
void DeleteLogFile(HWND	hwnd);
void DestructByProcessInject(HWND hwnd);
void OpenProcessByNtfsToken(HWND hwnd);
BOOL SafeModeCheck(HWND hwnd);
BOOL CleanPrefetch(HWND hwnd);
void LoadDllForCleanLog(HWND hwnd);
BOOL SafeModeOff(HWND hwnd);
BOOL SafeModeOn(HWND hwnd);
BOOL privileges_check();

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpstr, int nCmdShow) {

	if (!privileges_check()) {
		MessageBox(NULL, "Please Open Programm As Administrator!", "Privileges Error.", MB_ICONWARNING);
		return 1;
	}

	int SafeModeRequest = MessageBox(NULL, "Open the program in Safe Mode?", "Safe Mode", MB_OKCANCEL);
	if (SafeModeRequest == IDOK) {
		SafeModeTrigger = 1;
	}
	else {
		SafeModeTrigger = 0;
	}

	WNDCLASSEX wc = { 0 };
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hInstance = hInstance;
	wc.lpfnWndProc = WndProc;
	wc.lpszClassName = "NTFSCleaner";
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.hIcon = LoadIcon(NULL, IDI_WARNING);
	wc.hIconSm = LoadIcon(NULL, IDI_WARNING);

	if (!RegisterClassEx(&wc)) {
		MessageBox(NULL, "Failed to register window class", "Error (FTR1WC)", MB_ICONERROR);
		return 1;
	}

	HWND MainWindow = CreateWindowEx(WS_EX_CLIENTEDGE | WS_EX_TOPMOST, "NTFSCleaner", "Ntfs-Cleaner (v0.7 Alpha)", WS_OVERLAPPED | WS_VISIBLE | WS_SYSMENU | WS_CAPTION, CW_USEDEFAULT, CW_USEDEFAULT, 800, 500, NULL, NULL, hInstance, NULL);
	HWND SafeModeMenu = CreateWindowEx(0, "BUTTON", "SafeMode", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_FLAT, 645, 10, 130, 30, MainWindow, (HMENU)1000, hInstance, NULL);
	HWND HelpButton = CreateWindowEx(0, "BUTTON", "Reference", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_FLAT, 10, 10, 100, 30, MainWindow, (HMENU)1001, hInstance, NULL);
	HWND LiteButton = CreateWindowEx(0, "BUTTON", "Lite", WS_CHILD | BS_PUSHBUTTON | BS_FLAT, 230, 200, 100, 30, MainWindow, (HMENU)1002, hInstance, NULL);
	HWND NormalButton = CreateWindowEx(0, "BUTTON", "Normal", WS_CHILD | BS_PUSHBUTTON | BS_FLAT, 335, 200, 100, 30, MainWindow, (HMENU)1003, hInstance, NULL);
	HWND ExtraButton = CreateWindowEx(0, "BUTTON", "Extra", WS_CHILD | BS_PUSHBUTTON | BS_FLAT, 440, 200, 100, 30, MainWindow, (HMENU)1004, hInstance, NULL);
	HWND MenuButton = CreateWindowEx(0, "BUTTON", "Menu", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_FLAT, 320, 155, 100, 30, MainWindow, (HMENU)1005, hInstance, NULL);
	HWND FindJournalButton = CreateWindowEx(0, "BUTTON", "Find Journal", WS_CHILD | BS_PUSHBUTTON | BS_FLAT, 230, 250, 100, 30, MainWindow, (HMENU)1006, hInstance, NULL);
	HWND EnableLogFile = CreateWindowEx(0, "BUTTON", "Enable LogFile", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_FLAT, 330, 10, 115, 30, MainWindow, (HMENU)1007, hInstance, NULL);
	HWND DisableLogFile = CreateWindowEx(0, "BUTTON", "Delete LogFile", WS_CHILD | BS_PUSHBUTTON | BS_FLAT, 335, 250, 100, 30, MainWindow, (HMENU)1008, hInstance, NULL);
	HWND SelfDestructProgramm = CreateWindowEx(0, "BUTTON", "Self-Destruct The Program!", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_FLAT, 120, 10, 200, 30, MainWindow, (HMENU)1009, hInstance, NULL);
	HWND SelfDestructMettodInject = CreateWindowEx(0, "BUTTON", "Delete By Process Inject", WS_CHILD | BS_PUSHBUTTON | BS_FLAT, 120, 50, 200, 30, MainWindow, (HMENU)1010, hInstance, NULL);
	HWND SelfDestructMettodReboot = CreateWindowEx(0, "BUTTON", "Delete By Reboot", WS_CHILD | BS_PUSHBUTTON | BS_FLAT, 120, 85, 150, 30, MainWindow, (HMENU)1011, hInstance, NULL);
	HWND RollbackLogged = CreateWindowEx(0, "BUTTON", "Rollback Windows Logging", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_FLAT, 450, 10, 190, 30, MainWindow, (HMENU)1012, hInstance, NULL);
	HWND SafeModeOffButton = CreateWindowEx(0, "BUTTON", "SafeMode Off", WS_CHILD | BS_PUSHBUTTON | BS_FLAT, 645, 50, 130, 30, MainWindow, (HMENU)1013, hInstance, NULL);
	HWND SafeModeOnButton = CreateWindowEx(0, "BUTTON", "SafeMode On", WS_CHILD | BS_PUSHBUTTON | BS_FLAT, 645, 85, 130, 30, MainWindow, (HMENU)1014, hInstance, NULL);

	if (MainWindow == NULL) {
		MessageBox(NULL, "Failed to create Main Window", "Error (CR1MW)", MB_ICONERROR);
		return 1;
	}

	if (SafeModeRequest == IDOK) {
		SafeModeTrigger = 1;
		SafeModeOn(MainWindow);
	}
	else {
		SafeModeTrigger = 0;
	}

	UpdateLog(MainWindow, "Logs will appear here.", RGB(255, 255, 255));
	if (SafeModeTrigger == 1) {
		UpdateLog(MainWindow, "SafeMode Enable.", RGB(106, 227, 20));
	}
	if (WriteFileStatus && LogFile != INVALID_HANDLE_VALUE) {
		WriteLog(MainWindow, "Start NtfsCleaner!");
	}

	ShowWindow(MainWindow, nCmdShow);
	UpdateWindow(MainWindow);

	MSG msg = { 0 };
	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;

}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {

	switch (msg) {
	case WM_COMMAND:

		switch (LOWORD(wParam)) {

		case 1000: {
			static int SafeModeMenuVisibleStatus = 0;
			if (!SafeModeMenuVisibleStatus) {
				ShowWindow(GetDlgItem(hwnd, 1013), SW_SHOW);
				ShowWindow(GetDlgItem(hwnd, 1014), SW_SHOW);
				SafeModeMenuVisibleStatus = 1;
			}
			else {
				ShowWindow(GetDlgItem(hwnd, 1013), SW_HIDE);
				ShowWindow(GetDlgItem(hwnd, 1014), SW_HIDE);
				SafeModeMenuVisibleStatus = 0;
			}
			break;
		}

		case 1001:

			MessageBox(
				hwnd,
				"V0.7, Created By Spectre2Ux.\nNew Func:\n- Now every step is logged.\n- NEW MENU!\n- Improved UI\n- Error handling\n- SafeMode Screenshot Access\n- New SafeMode System\n- Cleaning Prefetch", "Reference (V0.7)",
				MB_ICONINFORMATION);

			WriteLog(hwnd, "Status Reference Button = Success!");
			break;

		case 1002:
			CleanPrefetch(hwnd);
			DeleteJournal(hwnd);

			break;

		case 1003:

			UpdateLog(hwnd, "Not Added.", RGB(227, 177, 41));
			MessageBox(hwnd, "Soon v0.8-0.9", "Not Added.", MB_ICONWARNING);
			break;

		case 1004:

			UpdateLog(hwnd, "Not Added.", RGB(227, 177, 41));
			MessageBox(hwnd, "Soon.", "Not Added.", MB_ICONWARNING);
			break;

		case 1005: {
			static int MenuVisibleStatus = 0;
			if (!MenuVisibleStatus) {
				ShowWindow(GetDlgItem(hwnd, 1002), SW_SHOW);
				ShowWindow(GetDlgItem(hwnd, 1003), SW_SHOW);
				ShowWindow(GetDlgItem(hwnd, 1004), SW_SHOW);
				ShowWindow(GetDlgItem(hwnd, 1006), SW_SHOW);
				ShowWindow(GetDlgItem(hwnd, 1008), SW_SHOW);
				MenuVisibleStatus = 1;
				WriteLog(hwnd, "Success Open Menu!");
			}
			else {
				ShowWindow(GetDlgItem(hwnd, 1002), SW_HIDE);
				ShowWindow(GetDlgItem(hwnd, 1003), SW_HIDE);
				ShowWindow(GetDlgItem(hwnd, 1004), SW_HIDE);
				ShowWindow(GetDlgItem(hwnd, 1006), SW_HIDE);
				ShowWindow(GetDlgItem(hwnd, 1008), SW_HIDE);
				MenuVisibleStatus = 0;
				WriteLog(hwnd, "Success Exit Menu!");
			}

			break;
		}

		case 1006:
			FindJournal(hwnd);
			WriteLog(hwnd, "Successfully, USN Journal Found!");
			break;
		case 1007:
			CreateFileToWriteLog(hwnd);
			break;

		case 1008:
			DeleteLogFile(hwnd);
			break;

		case 1009: {
			static int DestructFuncVisibleStatus = 0;
			if (!DestructFuncVisibleStatus) {
				ShowWindow(GetDlgItem(hwnd, 1010), SW_SHOW);
				ShowWindow(GetDlgItem(hwnd, 1011), SW_SHOW);
				DestructFuncVisibleStatus = 1;
			}
			else {
				ShowWindow(GetDlgItem(hwnd, 1010), SW_HIDE);
				ShowWindow(GetDlgItem(hwnd, 1011), SW_HIDE);
				DestructFuncVisibleStatus = 0;
			}

			break;
		}
		case 1010:
			OpenProcessByNtfsToken(hwnd);
			Sleep(2000);
			DestructByProcessInject(hwnd);
			Sleep(2000);
			break;

		case 1013:
			SafeModeOff(hwnd);
			break;

		case 1014:
			SafeModeOn(hwnd);
			break;
		}
	case WM_PAINT: {

		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hwnd, &ps);
		SetBkColor(hdc, RGB(0, 0, 0));
		SetTextColor(hdc, RGB(255, 255, 255));

		SetTextColor(hdc, logColor);

		TextOut(hdc, 10, 400, "Operation log:", (int)strlen("Operation log:"));
		TextOut(hdc, 10, 420, operationLog, (int)strlen(operationLog));

		SetTextColor(hdc, RGB(255, 255, 255));

		TextOut(hdc, 280, 120, "Clearing Windows Log Mods:", (int)strlen("Clearing Windows Log Mods:"));

		SetBkColor(hdc, RGB(0, 0, 0));
		SetTextColor(hdc, RGB(255, 255, 255));

		TextOut(hdc, 270, 320, "# <_O Created By Spectre2Ux", (int)strlen("# <_O Created By Spectre2Ux"));
		TextOut(hdc, 260, 350, "V0.7 Alpha, bugs may be present.", (int)strlen("V0.7 Alpha, bugs may be present."));

		EndPaint(hwnd, &ps);
		break;
	}
	case WM_DESTROY:
		CloseHandle(LogFile);
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}

	return 0;
}



// -----------------
// Security Function:

BOOL SafeModeCheck(HWND hwnd) {
	if (SafeModeTrigger == 1) {
		UpdateLog(hwnd, "SafeMode Triggered, Func Stopped.", RGB(227, 177, 41));
		MessageBox(hwnd, "You do not have permission to perform this function\nbecause Safe Mode is enabled.", "SafeMode Warning.", MB_ICONWARNING);
		WriteLog(hwnd, "SafeMode Triggered, You do not have permission to perform this function because Safe Mode is enabled.");
		return FALSE;
	}
	return TRUE;
}


BOOL SafeModeOn(HWND hwnd) {
	SetWindowDisplayAffinity(hwnd, WDA_EXCLUDEFROMCAPTURE);
	UpdateLog(hwnd, "SafeMode Successfully On!", RGB(106, 227, 20));
	WriteLog(hwnd, "SafeMode Successfully On!");
	return TRUE;
}

BOOL SafeModeOff(HWND hwnd) {
	if (SafeModeTrigger == 1) {
		SafeModeTrigger = 0;
		SetWindowDisplayAffinity(hwnd, WDA_NONE);
		UpdateLog(hwnd, "SafeMode Successfully Off!", RGB(106, 227, 20));
		WriteLog(hwnd, "SafeMode Successfully Off!");
		return TRUE;
	}
	else {
		UpdateLog(hwnd, "SafeMode Not Enabled.", RGB(227, 177, 41));
		return FALSE;
	}
	return FALSE;
}

BOOL privileges_check() {
	BOOL elevated = FALSE;
	HANDLE tokenHandle;
	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &tokenHandle)) {
		MessageBox(NULL, "Failed to GetInformation on privileges. (FT1OPT)", "Error (FT1OPT)", MB_ICONERROR);
		return FALSE;
	}

	TOKEN_ELEVATION elev_check = { 0 };
	DWORD BytesReturned = { 0 };
	if (GetTokenInformation(tokenHandle, TokenElevation, &elev_check, sizeof(elev_check), &BytesReturned)) {
		elevated = elev_check.TokenIsElevated;
	}

	CloseHandle(tokenHandle);
	return elev_check.TokenIsElevated;
}


void EscalationSystemAccess() {

}

// ---------------------------
// Log Function && WriteAccess


void UpdateLog(HWND hwnd, const char* text, COLORREF textColor) {
	strcpy(operationLog, text);
	logColor = textColor;
	InvalidateRect(hwnd, NULL, TRUE);
	UpdateWindow(hwnd);
}

void CreateFileToWriteLog(HWND hwnd) {
	LogFile = CreateFile("NtfsCleaner_v0.7_Log.txt", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (LogFile == INVALID_HANDLE_VALUE) {
		UpdateLog(hwnd, "Error Creating LogFile!", RGB(145, 4, 11));
		MessageBox(hwnd, "Error Creating LogFile!", "Error (ECL1F)", MB_ICONERROR);
		return;
	}
	WriteFileStatus = 1;
	SetFilePointer(LogFile, 0, NULL, FILE_END);
	UpdateLog(hwnd, "Success! LogFile SuccessFully Created!", RGB(106, 227, 20));
	WriteLog(hwnd, "Success Create LogFile");

	if (SafeModeTrigger == 1) {
		WriteLog(hwnd, "SafeMode Enable.");
	}
	else {
		WriteLog(hwnd, "SafeMode Disable.");
	}

	Sleep(300);
	char DirectoryPath[MAX_PATH];
	DWORD CurrentLogFileDirectory = GetCurrentDirectory(MAX_PATH, DirectoryPath);
	strcat(DirectoryPath, "\\NtfsCleaner_v0.7_Log.txt");
	char LogTextBuffer[510];
	sprintf(LogTextBuffer, "LogFile Directory: %s", DirectoryPath);
	UpdateLog(hwnd, LogTextBuffer, RGB(106, 227, 20));
}

void DeleteLogFile(HWND hwnd) {
	if (LogFile != INVALID_HANDLE_VALUE) {
		CloseHandle(LogFile);
		LogFile = INVALID_HANDLE_VALUE;
	}
	WriteFileStatus = 0;
	char DeleteFilePathBuffer[512];
	DWORD LogFilePath = GetCurrentDirectory(MAX_PATH, DeleteFilePathBuffer);
	strcat(DeleteFilePathBuffer, "\\NtfsCleaner_v0.7_Log.txt");
	if (DeleteFile(DeleteFilePathBuffer)) {
		UpdateLog(hwnd, "Success Deleting LogFile!", RGB(106, 227, 20));
	}
	else {
		UpdateLog(hwnd, "Not Found Log File.", RGB(145, 4, 11));
	}
	return;
}

void WriteLog(HWND hwnd, const char* LogText) {
	if (WriteFileStatus == 0 || LogFile == INVALID_HANDLE_VALUE) {
		return;
	}
	SYSTEMTIME st;
	GetLocalTime(&st);
	char LogBuffer[512];
	sprintf(LogBuffer, "[ %d:%d:%d ] - %s\n", st.wHour, st.wMinute, st.wSecond, LogText);
	DWORD BytesWritten;
	WriteFile(LogFile, LogBuffer, (DWORD)strlen(LogBuffer), &BytesWritten, NULL);
}


// ------------------
// Cleaning functions


BOOL CleanPrefetch(HWND hwnd) {

	UpdateLog(hwnd, "Start PrefetchClean Access...", RGB(255, 255, 255));
	WriteLog(hwnd, "Start PrefetchClean Access...");

	char PrefetchFiles[MAX_PATH];
	memset(PrefetchFiles, 0, sizeof(PrefetchFiles));
	strcpy(PrefetchFiles, "C:\\Windows\\Prefetch\\*.pf");
	WIN32_FIND_DATA FilesData;
	HANDLE FindFirstFileOnPrefetch = FindFirstFile(PrefetchFiles, &FilesData);
	if (FindFirstFileOnPrefetch != INVALID_HANDLE_VALUE) {
		do {
			char FilePath[MAX_PATH];
			sprintf(FilePath, "C:\\Windows\\Prefetch\\%s", FilesData.cFileName);			
			DeleteFile(FilePath);
			Sleep(50);
			UpdateLog(hwnd, FilePath, RGB(255, 255, 255));
			WriteLog(hwnd, FilePath);

		} while (FindNextFile(FindFirstFileOnPrefetch, &FilesData));
		FindClose(FindFirstFileOnPrefetch);
		CloseHandle(FindFirstFileOnPrefetch);

		UpdateLog(hwnd, "Success Prefetch Clean!.", RGB(106, 227, 20));

		return TRUE;
	}

	else {
		UpdateLog(hwnd, "Error On Prefetch Clean.", RGB(145, 4, 11));
		WriteLog(hwnd, "Error On Prefetch Clean.");
		return FALSE;
	}

	
	return TRUE;
	

}

void PrefetchBlockWrite() {

}

void RollbackWidowsLogging() {

}


void DeleteJournal(HWND	hwnd) {
	HANDLE VolumeOpen_handle = CreateFile("\\\\.\\c:", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, NULL, NULL);
	if (VolumeOpen_handle == INVALID_HANDLE_VALUE) {
		WriteLog(hwnd, "Failed to open volume handle, Error (FT1OH)");
		UpdateLog(hwnd, "There are not enough rights to open the volume completely.", RGB(145, 4, 11));
		MessageBox(hwnd, "Failed to open volume handle", "Error (FT1OH)", MB_ICONERROR);

	}

	UpdateLog(hwnd, "Volume Opened!", RGB(255, 255, 255));
	WriteLog(hwnd, "Volume Opened!");
	Sleep(200);

	USN_JOURNAL_DATA journalData = { 0 };
	DWORD ReturnedBytes = { 0 };
	if (!DeviceIoControl(VolumeOpen_handle, FSCTL_QUERY_USN_JOURNAL, NULL, 0, &journalData, sizeof(journalData), &ReturnedBytes, NULL)) {
		WriteLog(hwnd, "Mode: Lite - Error querying USN_J, [ Debugging ]");
		UpdateLog(hwnd, "Error Querying USN Journal. Please Restart Lite Mode.", RGB(145, 4, 11));
		MessageBox(hwnd, "Mode: Lite - Error querying USN_J, [ Debugging ]", "Error (FT1QJ)", MB_ICONERROR);

	}

	UpdateLog(hwnd, "USN Journal Found!", RGB(255, 255, 255));
	WriteLog(hwnd, "USN Journal Found!");
	Sleep(200);

	DELETE_USN_JOURNAL_DATA journalDelete = { 0 };
	journalDelete.DeleteFlags = USN_DELETE_FLAG_DELETE;
	journalDelete.UsnJournalID = journalData.UsnJournalID;

	if (!DeviceIoControl(VolumeOpen_handle, FSCTL_DELETE_USN_JOURNAL, &journalDelete, sizeof(journalDelete), NULL, 0, &ReturnedBytes, NULL)) {
		WriteLog(hwnd, "Mode: Lite - Error deleting USN_J, [ Debugging ]");
		UpdateLog(hwnd, "Error Deleting USN Journal. Please Restart Lite Mode.", RGB(145, 4, 11));
		MessageBox(hwnd, "Mode: Lite - Error deleting USN_J, [ Debugging ]", "Error (FT1DJ)", MB_ICONERROR);

	}

	UpdateLog(hwnd, "USN Journal Deleted!", RGB(255, 255, 255));
	WriteLog(hwnd, "USN Journal Deleted!");
	Sleep(2000);

	CREATE_USN_JOURNAL_DATA journalCreate = { 0 };
	journalCreate.MaximumSize = 0x10000000;
	journalCreate.AllocationDelta = 0x1000000;
	if (!DeviceIoControl(VolumeOpen_handle, FSCTL_CREATE_USN_JOURNAL, &journalCreate, sizeof(journalCreate), NULL, 0, &ReturnedBytes, NULL)) {
		WriteLog(hwnd, "Error Creating New USN Journal. Please Restart Lite Mode.");
		DWORD lastError = GetLastError();
		char msg[256];
		sprintf(msg, "Error creating USN Journal. Code: %lu", lastError);
		UpdateLog(hwnd, "Error Creating New USN Journal. Please Restart Lite Mode.", RGB(145, 4, 11));
		WriteLog(hwnd, "Error Creating New USN Journal. Please Restart Lite Mode.");
		MessageBox(hwnd, msg, "Error (FT1CJ)", MB_ICONERROR);
		return;

	}

	UpdateLog(hwnd, "USN Journal Created!", RGB(255, 255, 255));
	WriteLog(hwnd, "USN Journal Created!");
	Sleep(300);

	UpdateLog(hwnd, "Success! Error not found.", RGB(106, 227, 20));
	WriteLog(hwnd, "Success! Error not found.");

	CloseHandle(VolumeOpen_handle);

}

void FindJournal(HWND hwnd) {
	HANDLE VolumeOpen_handle = CreateFile("\\\\.\\c:", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, NULL, NULL);
	if (VolumeOpen_handle == INVALID_HANDLE_VALUE) {
		WriteLog(hwnd, "Failed to open volume handle, Error (FT1OH)");
		UpdateLog(hwnd, "There are not enough rights to open the volume completely.", RGB(145, 4, 11));
		MessageBox(hwnd, "Failed to open volume handle", "Error (FT1OH)", MB_ICONERROR);
	}
	UpdateLog(hwnd, "Volume Opened!", RGB(255, 255, 255));
	Sleep(200);

	USN_JOURNAL_DATA journalData = { 0 };
	DWORD ReturnedBytes = { 0 };
	if (!DeviceIoControl(VolumeOpen_handle, FSCTL_QUERY_USN_JOURNAL, NULL, 0, &journalData, sizeof(journalData), &ReturnedBytes, NULL)) {
		WriteLog(hwnd, "USN Journal not found or you PC");
		UpdateLog(hwnd, "USN Journal not found or you PC", RGB(145, 4, 11));
		MessageBox(hwnd, "Mode: Lite - USN Journal not found or you PC, [ Debugging ]", "Error (FT1QJ)", MB_ICONERROR);

	}

	char UsnJournalMemory[512];
	sprintf(UsnJournalMemory, "Usn ID: %I64x", journalData.UsnJournalID);
	UpdateLog(hwnd, UsnJournalMemory, RGB(106, 227, 20));
	WriteLog(hwnd, UsnJournalMemory);
	memset(UsnJournalMemory, 0, sizeof(UsnJournalMemory));
	Sleep(200);
	sprintf(UsnJournalMemory, "First Usn: %I64d", journalData.FirstUsn);
	UpdateLog(hwnd, UsnJournalMemory, RGB(106, 227, 20));
	WriteLog(hwnd, UsnJournalMemory);
	memset(UsnJournalMemory, 0, sizeof(UsnJournalMemory));
	Sleep(200);
	sprintf(UsnJournalMemory, "Next Usn: %I64d", journalData.NextUsn);
	UpdateLog(hwnd, UsnJournalMemory, RGB(106, 227, 20));
	WriteLog(hwnd, UsnJournalMemory);
	memset(UsnJournalMemory, 0, sizeof(UsnJournalMemory));
	Sleep(200);
	sprintf(UsnJournalMemory, "Max Usn: %I64d", journalData.MaxUsn);
	UpdateLog(hwnd, UsnJournalMemory, RGB(106, 227, 20));
	WriteLog(hwnd, UsnJournalMemory);
	memset(UsnJournalMemory, 0, sizeof(UsnJournalMemory));
	Sleep(300);
	UpdateLog(hwnd, "if LogFile Enabled - then all the information is recorded in it.", RGB(106, 227, 20));
	Sleep(600);
	UpdateLog(hwnd, "USN Journal Found!", RGB(106, 227, 20));

}

// --------------------
// Destruct Functions

void OpenProcessByNtfsToken(HWND hwnd) {

	UpdateLog(hwnd, "Start Open Test App...", RGB(255, 255, 255));
	WriteLog(hwnd, "Start Open Test App...");

	Sleep(200);

	UpdateLog(hwnd, "Search App Directory...", RGB(255, 255, 255));
	WriteLog(hwnd, "Search App Directory...");

	char TestPath[MAX_PATH];
	DWORD TestGetPath = GetCurrentDirectory(MAX_PATH, TestPath);
	strcat(TestPath, "\\TestInjectTarget.exe");
	STARTUPINFO TestParameter = { sizeof(TestParameter) };
	TestParameter.dwFlags = STARTF_USESHOWWINDOW;
	TestParameter.wShowWindow = SW_HIDE;

	sprintf(operationLog, "Path: %s", TestPath);

	UpdateLog(hwnd, operationLog, RGB(255, 255, 255));
	WriteLog(hwnd, operationLog);

	PROCESS_INFORMATION TestInformation;

	if (!CreateProcess(TestPath, NULL, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &TestParameter, &TestInformation)) {
		UpdateLog(hwnd, "Error, CreateProcess Func.", RGB(145, 4, 11));
		WriteLog(hwnd, "Error, CreateProcess Func.");

	}
	CloseHandle(TestInformation.hProcess);
	CloseHandle(TestInformation.hThread);
	return;
}


void DestructByProcessInject(HWND hwnd) {

	if (SafeModeCheck(hwnd) == FALSE) {
		return;
	}

	WriteLog(hwnd, "Start Access To DestructByProcessInject");

	char LogTextOnParameter[256];

	memset(LogTextOnParameter, 0, sizeof(LogTextOnParameter));

	UpdateLog(hwnd, "Start Find Target Process...", RGB(255, 255, 255));
	strcat(LogTextOnParameter, "Start Find Target Process...");

	WriteLog(hwnd, "Start Find Target Process...");

	HANDLE ProcessList = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (ProcessList == INVALID_HANDLE_VALUE) {
		sprintf(LogTextOnParameter, "Error, Listing Process (Debugging) Error: %d", GetLastError());
		UpdateLog(hwnd, LogTextOnParameter, RGB(145, 4, 11));
		WriteLog(hwnd, LogTextOnParameter);
		return;
	}

	UpdateLog(hwnd, LogTextOnParameter, RGB(255, 255, 255));
	Sleep(200);
	UpdateLog(hwnd, "Found Target Process.", RGB(255, 255, 255));
	WriteLog(hwnd, "Found Target Process.");

	Sleep(200);
	UpdateLog(hwnd, "Search Memory To Target Process.", RGB(255, 255, 255));
	WriteLog(hwnd, "Search Memory To Tagret Process.");


	PROCESSENTRY32 ProcessData;
	ProcessData.dwSize = sizeof(PROCESSENTRY32);
	Process32First(ProcessList, &ProcessData);
	do {
		if (strcmp(ProcessData.szExeFile, "TestInjectTarget.exe") == 0) {
			sprintf(LogTextOnParameter, "Process On Inject Found! Name: %s ProcessID: %d Threads: %d", ProcessData.szExeFile, ProcessData.th32ProcessID, ProcessData.cntThreads);
			UpdateLog(hwnd, LogTextOnParameter, RGB(106, 227, 20));
			ByInjectPid = ProcessData.th32ProcessID;
		}
	} while (Process32Next(ProcessList, &ProcessData));

	WriteLog(hwnd, LogTextOnParameter);
	
	WriteLog(hwnd, "Attempt or Open Target Process.");

	Sleep(200);
	UpdateLog(hwnd, "Attempt or Open Target Process.", RGB(255, 255, 255));

	HANDLE ProcessKey = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_VM_OPERATION | PROCESS_CREATE_THREAD, FALSE, ByInjectPid);
	if (ProcessKey == NULL) {
		sprintf(LogTextOnParameter, "Error, OpenProcess Func (Debugging) Error: %d", GetLastError());
		UpdateLog(hwnd, LogTextOnParameter, RGB(145, 4, 11));
		WriteLog(hwnd, LogTextOnParameter);
		CloseHandle(ProcessList);
		return;
	}

	Sleep(200);
	UpdateLog(hwnd, "SuccessFully Open Target Process!", RGB(255, 255, 255));
	WriteLog(hwnd, "SuccessFully Open Target Process!");

	Sleep(200);
	UpdateLog(hwnd, "Create Path To Dll...", RGB(255, 255, 255));
	WriteLog(hwnd, "Create Path To Dll...");

	char DllPath[512];
	DWORD CurrentDirectoryOrDll = GetCurrentDirectory(MAX_PATH, DllPath);
	strcat(DllPath, "\\SelfDestructDll.dll");
	size_t DllPathSize = strlen(DllPath) + 1;

	char PathLog[256];
	sprintf(PathLog, "Path: %s Size: %d", DllPath, DllPathSize);

	UpdateLog(hwnd, PathLog, RGB(255, 255, 255));
	WriteLog(hwnd, PathLog);

	Sleep(400);

	memset(LogTextOnParameter, 0, strlen(LogTextOnParameter));

	Sleep(200);
	UpdateLog(hwnd, "Start Memory Allocation...", RGB(255, 255, 255));
	WriteLog(hwnd, "Start Memory Allocation...");

	LPVOID MemAllocation = VirtualAllocEx(ProcessKey, NULL, DllPathSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	if (MemAllocation == NULL) {
		sprintf(LogTextOnParameter, "Error, VirtualAlloc Func (Debugging) Error: %d", GetLastError());
		UpdateLog(hwnd, LogTextOnParameter, RGB(145, 4, 11));
		WriteLog(hwnd, LogTextOnParameter);
		CloseHandle(ProcessList);
		CloseHandle(ProcessKey);
		return;
	}

	Sleep(200);
	UpdateLog(hwnd, "Successfully Memory Allocation!", RGB(255, 255, 255));
	WriteLog(hwnd, "Successfully Memory Allocation!");

	memset(LogTextOnParameter, 0, strlen(LogTextOnParameter));

	Sleep(200);
	UpdateLog(hwnd, "Start Memory Writing...", RGB(255, 255, 255));
	WriteLog(hwnd, "Start Memory Writing...");

	BOOL MemWrite = WriteProcessMemory(ProcessKey, MemAllocation, DllPath, DllPathSize, NULL);
	if (!MemWrite) {
		sprintf(LogTextOnParameter, "Error, MemoryWrite Func (Debugging) Error: %d", GetLastError());
		UpdateLog(hwnd, LogTextOnParameter, RGB(145, 4, 11));
		WriteLog(hwnd, LogTextOnParameter);
		CloseHandle(ProcessList);
		CloseHandle(ProcessKey);
		return;
	}

	Sleep(200);
	UpdateLog(hwnd, "Successfully! Memory Writed.", RGB(255, 255, 255));
	WriteLog(hwnd, "Successfully! Memory Writed.");

	memset(LogTextOnParameter, 0, strlen(LogTextOnParameter));

	Sleep(200);
	UpdateLog(hwnd, "Create Handle To Load Library...", RGB(255, 255, 255));
	WriteLog(hwnd, "Create Handle To Load Library...");

	LPTHREAD_START_ROUTINE LoadedLibrary = (LPTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandle("kernel32.dll"), "LoadLibraryA");
	if (!LoadedLibrary) {
		sprintf(LogTextOnParameter, "Error, LoadLibrary (Debugging) Error: %d", GetLastError());
		UpdateLog(hwnd, LogTextOnParameter, RGB(145, 4, 11));
		WriteLog(hwnd, LogTextOnParameter);
		CloseHandle(ProcessList);
		CloseHandle(ProcessKey);
		return;
	}

	Sleep(200);
	UpdateLog(hwnd, "Handle To Load Library Successfully Created!", RGB(255, 255, 255));
	WriteLog(hwnd, "Handle To Load Library Successfully Created!");

	memset(LogTextOnParameter, 0, strlen(LogTextOnParameter));

	Sleep(200);
	UpdateLog(hwnd, "Start Remote Inject...", RGB(255, 255, 255));
	WriteLog(hwnd, "Start Remote Inject...");

	HANDLE RemoteInject = CreateRemoteThread(ProcessKey, NULL, 0, LoadedLibrary, MemAllocation, 0, NULL);
	if (RemoteInject == NULL) {
		sprintf(LogTextOnParameter, "Error, Inject Func (Debugging) Error: %d", GetLastError());
		UpdateLog(hwnd, LogTextOnParameter, RGB(145, 4, 11));
		WriteLog(hwnd, LogTextOnParameter);
		CloseHandle(ProcessList);
		CloseHandle(ProcessKey);
		VirtualFreeEx(ProcessKey, MemAllocation, 0, MEM_RELEASE);
		return;
	}

	WaitForSingleObject(RemoteInject, INFINITE);

	Sleep(200);
	UpdateLog(hwnd, "Successfully Injected.", RGB(106, 227, 20));
	WriteLog(hwnd, "Successfully Injected.");

	CloseHandle(ProcessList);
	CloseHandle(RemoteInject);
	VirtualFreeEx(ProcessKey, MemAllocation, 0, MEM_RELEASE);
	CloseHandle(ProcessKey);
	return;
}