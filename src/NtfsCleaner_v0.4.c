#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <tlhelp32.h>
#include <stdio.h>

char operationLog[256];
int WriteFileStatus = 0;
HANDLE LogFile = INVALID_HANDLE_VALUE;
COLORREF logColor = RGB(255, 255, 255);

void FindJournal(HWND hwnd);
void UpdateLog(HWND hwnd, const char* text, COLORREF textColor);
void DeleteJournal(HWND hwnd);
void WriteLog(HWND hwnd, HANDLE FileHandle, const char* text);
void CreateFileToWriteLog(HWND hwnd);
void DeleteLogFile(HWND	hwnd);
BOOL privileges_check();


LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpstr, int nCmdShow) {

	if (!privileges_check()) {
		MessageBox(NULL, "Please Open Programm As Administrator!", "Privileges Error.", MB_ICONWARNING);
		return 1;
	}

	WNDCLASSEX wc = { 0 };
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hInstance = hInstance;
	wc.lpfnWndProc = WndProc;
	wc.lpszClassName = "NTFSCleaner";
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wc.style = CS_HREDRAW | CS_VREDRAW | WS_EX_CLIENTEDGE;
	wc.hIcon = LoadIcon(NULL, IDI_WARNING);
	wc.hIconSm = LoadIcon(NULL, IDI_WARNING);

	if (!RegisterClassEx(&wc)) {
		MessageBox(NULL, "Failed to register window class", "Error (FTR1WC)", MB_ICONERROR);
		return 1;
	}

	HWND MainWindow = CreateWindowEx(WS_EX_CLIENTEDGE | WS_EX_TOPMOST, "NTFSCleaner", "Ntfs-Cleaner (v0.4 Alpha)", WS_OVERLAPPED | WS_VISIBLE | WS_SYSMENU, CW_USEDEFAULT, CW_USEDEFAULT, 800, 500, NULL, NULL, hInstance, NULL);
	HWND ExitButton = CreateWindowEx(0, "BUTTON", "Exit", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_FLAT, 710, 10, 60, 30, MainWindow, (HMENU)1000, hInstance, NULL);
	HWND HelpButton = CreateWindowEx(0, "BUTTON", "Reference", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_FLAT, 10, 10, 100, 30, MainWindow, (HMENU)1001, hInstance, NULL);
	HWND LiteButton = CreateWindowEx(0, "BUTTON", "Lite", WS_CHILD | BS_PUSHBUTTON | BS_FLAT, 210, 200, 100, 30, MainWindow, (HMENU)1002, hInstance, NULL);
	HWND NormalButton = CreateWindowEx(0, "BUTTON", "Normal", WS_CHILD | BS_PUSHBUTTON | BS_FLAT, 315, 200, 100, 30, MainWindow, (HMENU)1003, hInstance, NULL);
	HWND ExtraButton = CreateWindowEx(0, "BUTTON", "Extra", WS_CHILD | BS_PUSHBUTTON | BS_FLAT, 420, 200, 100, 30, MainWindow, (HMENU)1004, hInstance, NULL);
	HWND MenuButton = CreateWindowEx(0, "BUTTON", "Menu", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_FLAT, 298, 155, 100, 30, MainWindow, (HMENU)1005, hInstance, NULL);
	HWND FindJournalButton = CreateWindowEx(0, "BUTTON", "Find Journal", WS_CHILD | BS_PUSHBUTTON | BS_FLAT, 210, 250, 100, 30, MainWindow, (HMENU)1006, hInstance, NULL);
	HWND EnableLogFile = CreateWindowEx(0, "BUTTON", "Enable logging", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_FLAT, 330, 10, 115, 30, MainWindow, (HMENU)1007, hInstance, NULL);
	HWND DisableLogFile = CreateWindowEx(0, "BUTTON", "Delete LogFile", WS_CHILD | BS_PUSHBUTTON | BS_FLAT, 315, 250, 100, 30, MainWindow, (HMENU)1008, hInstance, NULL);
	HWND SelfDestructProgramm = CreateWindowEx(0, "BUTTON", "Self-Destruct The Program!", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_FLAT, 120, 10, 200, 30, MainWindow, (HMENU)1009, hInstance, NULL);

	if (MainWindow == NULL) {
		MessageBox(NULL, "Failed to create Main Window", "Error (CR1MW)", MB_ICONERROR);
		return 1;
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

		case 1000:

			if (WriteFileStatus && LogFile != INVALID_HANDLE_VALUE) {
				WriteLog(hwnd, LogFile, "Success Exit to Programm!");
			}
			PostQuitMessage(0);
			break;

		case 1001:

			MessageBox(
				hwnd, 
				"V0.4, Created By Spectre2Ux.\nNew Func:\n\n- Help Button\n- Enable Log-File\n- Improved UI\n- Error handling\n- Temp Folder Cleaning on LiteMode\n- (NO WORK Disk Changer)", "Reference (V0.4)",
				MB_ICONINFORMATION);

			if (WriteFileStatus && LogFile != INVALID_HANDLE_VALUE) {
				WriteLog(hwnd, LogFile, "Status Reference Button = Success!");
			}
			break;

		case 1002:

			DeleteJournal(hwnd);
			if (WriteFileStatus && LogFile != INVALID_HANDLE_VALUE) {
				WriteLog(hwnd, LogFile, "Successfully Delete USN Journal!");
			}
			break;

		case 1003:

			UpdateLog(hwnd, "Not Added.", RGB(227, 177, 41));
			MessageBox(hwnd, "Soon v0.5-0.6", "Not Added.", MB_ICONWARNING);
			break;

		case 1004:

			UpdateLog(hwnd, "Not Added.", RGB(227, 177, 41));
			MessageBox(hwnd, "Soon.", "Not Added.", MB_ICONWARNING);
			break;

		case 1005: {
			static int VisibleStatus = 0;
			if (!VisibleStatus) {
				ShowWindow(GetDlgItem(hwnd, 1002), SW_SHOW);
				ShowWindow(GetDlgItem(hwnd, 1003), SW_SHOW);
				ShowWindow(GetDlgItem(hwnd, 1004), SW_SHOW);
				ShowWindow(GetDlgItem(hwnd, 1006), SW_SHOW);
				ShowWindow(GetDlgItem(hwnd, 1008), SW_SHOW);
				VisibleStatus = 1;
				if (WriteFileStatus && LogFile != INVALID_HANDLE_VALUE) {
					WriteLog(hwnd, LogFile, "Success Open Menu!");
				}
			}
			else {
				ShowWindow(GetDlgItem(hwnd, 1002), SW_HIDE);
				ShowWindow(GetDlgItem(hwnd, 1003), SW_HIDE);
				ShowWindow(GetDlgItem(hwnd, 1004), SW_HIDE);
				ShowWindow(GetDlgItem(hwnd, 1006), SW_HIDE);
				ShowWindow(GetDlgItem(hwnd, 1008), SW_HIDE);
				VisibleStatus = 0;
				if (WriteFileStatus && LogFile != INVALID_HANDLE_VALUE) {
					WriteLog(hwnd, LogFile, "Success Exit Menu!");
				}
			}

			break;
		}

		case 1006:
			FindJournal(hwnd);
			if (WriteFileStatus && LogFile != INVALID_HANDLE_VALUE) {
				WriteLog(hwnd, LogFile, "Successfully, USN Journal Found!");
			}
			break;
		case 1007:
			CreateFileToWriteLog(hwnd);
			break;
			
		case 1008:
			DeleteLogFile(hwnd);
			break;

		case 1009:
			UpdateLog(hwnd, "Not Added.", RGB(227, 177, 41));
			MessageBox(hwnd, "Soon v0.5 Version!", "Not Added.", MB_ICONWARNING);
			break;

		}

			
	case WM_PAINT: {

		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hwnd, &ps);
		SetBkColor(hdc, RGB(0, 0, 0));
		SetTextColor(hdc, RGB(255, 255, 255));

		SetTextColor(hdc, logColor);

		TextOut(hdc, 10, 50, "Operation log:", (int)strlen("Operation log:"));
		TextOut(hdc, 10, 70, operationLog, (int)strlen(operationLog));

		SetTextColor(hdc, RGB(255, 255, 255));

		TextOut(hdc, 255, 120, "Clearing Windows Log Mods:", (int)strlen("Clearing Windows Log Mods:"));

		SetBkColor(hdc, RGB(0, 0, 0));
		SetTextColor(hdc, RGB(255, 255, 255));

		TextOut(hdc, 260, 380, "# <_O Created By Spectre2Ux", (int)strlen("# <_O Created By Spectre2Ux"));
		TextOut(hdc, 250, 410, "V0.4 Alpha, bugs may be present.", (int)strlen("V0.4 Alpha, bugs may be present."));

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


void UpdateLog(HWND hwnd, const char* text, COLORREF textColor) {
	strcpy(operationLog, text);
	logColor = textColor;
	InvalidateRect(hwnd, NULL, TRUE);
	UpdateWindow(hwnd);
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

void CreateFileToWriteLog(HWND hwnd) {
	LogFile = CreateFile("NtfsCleaner_v0.4_Log.txt", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (LogFile == INVALID_HANDLE_VALUE) {
		UpdateLog(hwnd, "Error Creating LogFile!", RGB(145, 4, 11));
		MessageBox(hwnd, "Error Creating LogFile!", "Error (ECL1F)", MB_ICONERROR);
		return;
	}
	WriteFileStatus = 1;
	SetFilePointer(LogFile, 0, NULL, FILE_END);
	UpdateLog(hwnd, "Success! LogFile SuccessFully Created!", RGB(106, 227, 20));
	WriteLog(hwnd, LogFile, "Success Create LogFile");
	Sleep(300);
	char DirectoryPath[MAX_PATH];
	DWORD CurrentLogFileDirectory = GetCurrentDirectory(MAX_PATH, DirectoryPath);
	strcat(DirectoryPath, "\\NtfsCleaner_v0.4_Log.txt");
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
	strcat(DeleteFilePathBuffer, "\\NtfsCleaner_v0.4_Log.txt");
	if (DeleteFile(DeleteFilePathBuffer)) {
		UpdateLog(hwnd, "Success Deleting LogFile!", RGB(106, 227, 20));
	}
	else {
		UpdateLog(hwnd, "Not Found Log File.", RGB(145, 4, 11));
	}
	return;
}

void WriteLog(HWND hwnd, HANDLE FileHandle, const char* text) {
	if (WriteFileStatus == 0) {
		return;
	}
	SYSTEMTIME LocalSystemTime;
	GetLocalTime(&LocalSystemTime);
	char TextBuffer[512];
	DWORD WrittenBytesToLog;
	sprintf(TextBuffer, "[ %d || %d || %d ] - %s\n", LocalSystemTime.wHour, LocalSystemTime.wMinute, LocalSystemTime.wSecond, text);
	if (!WriteFile(FileHandle, TextBuffer, (DWORD)(strlen(TextBuffer)), &WrittenBytesToLog, NULL)) {
		UpdateLog(hwnd, "Error or EnableLog (FOW1TF)", RGB(145, 4, 11));
		MessageBox(hwnd, "Failed or WriteLog to File, (Access diened)", "Error (FOW1TF)", MB_ICONERROR);
		return;
	}
}

void DeleteJournal(HWND	hwnd) {
	HANDLE VolumeOpen_handle = CreateFile("\\\\.\\c:", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, NULL, NULL);
	if (VolumeOpen_handle == INVALID_HANDLE_VALUE) {
		if (WriteFileStatus && LogFile != INVALID_HANDLE_VALUE) {
			WriteLog(hwnd, LogFile, "Failed to open volume handle, Error (FT1OH)");
		}
		UpdateLog(hwnd, "There are not enough rights to open the volume completely.", RGB(145, 4, 11));
		MessageBox(hwnd, "Failed to open volume handle", "Error (FT1OH)", MB_ICONERROR);

	}

	UpdateLog(hwnd, "Volume Opened!", RGB(255, 255, 255));
	Sleep(200);

	USN_JOURNAL_DATA journalData = { 0 };
	DWORD ReturnedBytes = { 0 };
	if (!DeviceIoControl(VolumeOpen_handle, FSCTL_QUERY_USN_JOURNAL, NULL, 0, &journalData, sizeof(journalData), &ReturnedBytes, NULL)) {
		if (WriteFileStatus && LogFile != INVALID_HANDLE_VALUE) {
			WriteLog(hwnd, LogFile, "Mode: Lite - Error querying USN_J, [ Debugging ]");
		}
		UpdateLog(hwnd, "Error Querying USN Journal. Please Restart Lite Mode.", RGB(145, 4, 11));
		MessageBox(hwnd, "Mode: Lite - Error querying USN_J, [ Debugging ]", "Error (FT1QJ)", MB_ICONERROR);

	}

	UpdateLog(hwnd, "USN Journal Found!", RGB(255, 255, 255));
	Sleep(200);

	DELETE_USN_JOURNAL_DATA journalDelete = { 0 };
	journalDelete.DeleteFlags = USN_DELETE_FLAG_DELETE;
	journalDelete.UsnJournalID = journalData.UsnJournalID;

	if (!DeviceIoControl(VolumeOpen_handle, FSCTL_DELETE_USN_JOURNAL, &journalDelete, sizeof(journalDelete), NULL, 0, &ReturnedBytes, NULL)) {
		if (WriteFileStatus && LogFile != INVALID_HANDLE_VALUE) {
			WriteLog(hwnd, LogFile, "Mode: Lite - Error deleting USN_J, [ Debugging ]");
		}
		UpdateLog(hwnd, "Error Deleting USN Journal. Please Restart Lite Mode.", RGB(145, 4, 11));
		MessageBox(hwnd, "Mode: Lite - Error deleting USN_J, [ Debugging ]", "Error (FT1DJ)", MB_ICONERROR);

	}

	UpdateLog(hwnd, "USN Journal Deleted!", RGB(255, 255, 255));
	Sleep(2000);

	CREATE_USN_JOURNAL_DATA journalCreate = { 0 };
	journalCreate.MaximumSize = 0x10000000;
	journalCreate.AllocationDelta = 0x1000000;
	if (!DeviceIoControl(VolumeOpen_handle, FSCTL_CREATE_USN_JOURNAL, &journalCreate, sizeof(journalCreate), NULL, 0, &ReturnedBytes, NULL)) {
		if (WriteFileStatus && LogFile != INVALID_HANDLE_VALUE) {
			WriteLog(hwnd, LogFile, "Error Creating New USN Journal. Please Restart Lite Mode.");
		}
		DWORD lastError = GetLastError();
		char msg[256];
		sprintf(msg, "Error creating USN Journal. Code: %lu", lastError);
		UpdateLog(hwnd, "Error Creating New USN Journal. Please Restart Lite Mode.", RGB(145, 4, 11));
		MessageBox(hwnd, msg, "Error (FT1CJ)", MB_ICONERROR);
		return;

	}

	UpdateLog(hwnd, "USN Journal Created!", RGB(255, 255, 255));
	Sleep(300);

	UpdateLog(hwnd, "Success! Error not found.", RGB(106, 227, 20));
	CloseHandle(VolumeOpen_handle);

}

void FindJournal(HWND hwnd) {
	HANDLE VolumeOpen_handle = CreateFile("\\\\.\\c:", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, NULL, NULL);
	if (VolumeOpen_handle == INVALID_HANDLE_VALUE) {
		if (WriteFileStatus && LogFile != INVALID_HANDLE_VALUE) {
			WriteLog(hwnd, LogFile, "Failed to open volume handle, Error (FT1OH)");
		}
		UpdateLog(hwnd, "There are not enough rights to open the volume completely.", RGB(145, 4, 11));
		MessageBox(hwnd, "Failed to open volume handle", "Error (FT1OH)", MB_ICONERROR);
	}
	UpdateLog(hwnd, "Volume Opened!", RGB(255, 255, 255));
	Sleep(200);

	USN_JOURNAL_DATA journalData = { 0 };
	DWORD ReturnedBytes = { 0 };
	if (!DeviceIoControl(VolumeOpen_handle, FSCTL_QUERY_USN_JOURNAL, NULL, 0, &journalData, sizeof(journalData), &ReturnedBytes, NULL)) {
		if (WriteFileStatus && LogFile != INVALID_HANDLE_VALUE) {
			WriteLog(hwnd, LogFile, "USN Journal not found or you PC");
		}
		UpdateLog(hwnd, "USN Journal not found or you PC", RGB(145, 4, 11));
		MessageBox(hwnd, "Mode: Lite - USN Journal not found or you PC, [ Debugging ]", "Error (FT1QJ)", MB_ICONERROR);

	}

	char UsnJournalMemory[512];
	sprintf(UsnJournalMemory, "Usn ID: %I64x", journalData.UsnJournalID);
	UpdateLog(hwnd, UsnJournalMemory, RGB(106, 227, 20));
	if (WriteFileStatus && LogFile != INVALID_HANDLE_VALUE) {
		WriteLog(hwnd, LogFile, UsnJournalMemory);
	}
	memset(UsnJournalMemory, 0, sizeof(UsnJournalMemory));
	Sleep(200);
	sprintf(UsnJournalMemory, "First Usn: %I64d", journalData.FirstUsn);
	UpdateLog(hwnd, UsnJournalMemory, RGB(106, 227, 20));
	if (WriteFileStatus && LogFile != INVALID_HANDLE_VALUE) {
		WriteLog(hwnd, LogFile, UsnJournalMemory);
	}
	memset(UsnJournalMemory, 0, sizeof(UsnJournalMemory));
	Sleep(200);
	sprintf(UsnJournalMemory, "Next Usn: %I64d", journalData.NextUsn);
	UpdateLog(hwnd, UsnJournalMemory, RGB(106, 227, 20));
	if (WriteFileStatus && LogFile != INVALID_HANDLE_VALUE) {
		WriteLog(hwnd, LogFile, UsnJournalMemory);
	}
	memset(UsnJournalMemory, 0, sizeof(UsnJournalMemory));
	Sleep(200);
	sprintf(UsnJournalMemory, "Max Usn: %I64d", journalData.MaxUsn);
	UpdateLog(hwnd, UsnJournalMemory, RGB(106, 227, 20));
	if (WriteFileStatus && LogFile != INVALID_HANDLE_VALUE) {
		WriteLog(hwnd, LogFile, UsnJournalMemory);
	}
	memset(UsnJournalMemory, 0, sizeof(UsnJournalMemory));
	Sleep(300);
	UpdateLog(hwnd, "if LogFile Enabled - then all the information is recorded in it.", RGB(106, 227, 20));
	Sleep(600);
	UpdateLog(hwnd, "USN Journal Found!", RGB(106, 227, 20));

	

	CloseHandle(VolumeOpen_handle);
}