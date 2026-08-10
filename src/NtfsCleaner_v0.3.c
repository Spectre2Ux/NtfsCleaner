#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <tlhelp32.h>
#include <stdio.h>

char operationLog[256];
COLORREF logColor = RGB(255, 255, 255);

void UpdateLog(HWND hwnd, const char* text, COLORREF textColor);
void DeleteJournal(HWND hwnd);
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

	HWND MainWindow = CreateWindowEx(WS_EX_CLIENTEDGE, "NTFSCleaner", "Ntfs-Cleaner (v0.3 alpha)", WS_OVERLAPPED | WS_VISIBLE | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX, CW_USEDEFAULT, CW_USEDEFAULT, 800, 500, NULL, NULL, hInstance, NULL);
	HWND ExitButton = CreateWindowEx(0, "BUTTON", "Exit", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_FLAT, 710, 10, 60, 30, MainWindow, (HMENU)1000, hInstance, NULL);
	HWND HelpButton = CreateWindowEx(0, "BUTTON", "Reference", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_FLAT, 10, 10, 100, 30, MainWindow, (HMENU)1001, hInstance, NULL);
	HWND LiteButton = CreateWindowEx(0, "BUTTON", "Lite", WS_CHILD | BS_PUSHBUTTON | BS_FLAT, 215, 150, 100, 30, MainWindow, (HMENU)1002, hInstance, NULL);
	HWND NormalButton = CreateWindowEx(0, "BUTTON", "Normal", WS_CHILD | BS_PUSHBUTTON | BS_FLAT, 320, 150, 100, 30, MainWindow, (HMENU)1003, hInstance, NULL);
	HWND ExtraButton = CreateWindowEx(0, "BUTTON", "Extra", WS_CHILD | BS_PUSHBUTTON | BS_FLAT, 425, 150, 100, 30, MainWindow, (HMENU)1004, hInstance, NULL);
	HWND MenuButton = CreateWindowEx(0, "BUTTON", "Menu", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_FLAT, 315, 70, 100, 30, MainWindow, (HMENU)1005, hInstance, NULL);
	HWND FindJournalButton = CreateWindowEx(0, "BUTTON", "Find Journal", WS_CHILD | BS_PUSHBUTTON | BS_FLAT, 320, 190, 100, 30, MainWindow, (HMENU)1006, hInstance, NULL);
	
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

		if (LOWORD(wParam) == 1000) {
			PostQuitMessage(0);
			break;
		}
		else if (LOWORD(wParam) == 1001) {
			MessageBox(hwnd, "V0.3, Created By Spectre2Ux.\nNew Func:\n\n- Find USN Journal Func\n- Improved UI\n- Error handling", "Reference", MB_ICONINFORMATION);
			break;
		}

		else if (LOWORD(wParam) == 1002) {
			DeleteJournal(hwnd);
			break;
		}

		else if (LOWORD(wParam) == 1003) {
			MessageBox(hwnd, "Soon.", "Not Added.", MB_ICONWARNING);
			break;
		}
		else if (LOWORD(wParam) == 1004) {
			MessageBox(hwnd, "Soon.", "Not Added.", MB_ICONWARNING);
			break;
		}
		else if (LOWORD(wParam) == 1005) {
			static int VisibleStatus = 0;
			if (!VisibleStatus) {
				ShowWindow(GetDlgItem(hwnd, 1002), SW_SHOW);
				ShowWindow(GetDlgItem(hwnd, 1003), SW_SHOW);
				ShowWindow(GetDlgItem(hwnd, 1004), SW_SHOW);
				ShowWindow(GetDlgItem(hwnd, 1006), SW_SHOW);
				VisibleStatus = 1;
			}

			else {
				ShowWindow(GetDlgItem(hwnd, 1002), SW_HIDE);
				ShowWindow(GetDlgItem(hwnd, 1003), SW_HIDE);
				ShowWindow(GetDlgItem(hwnd, 1004), SW_HIDE);
				ShowWindow(GetDlgItem(hwnd, 1006), SW_HIDE);
				VisibleStatus = 0;
			}

			break;
		}

		else if (LOWORD(wParam) == 1006) {
			HANDLE VolumeOpen_handle = CreateFile("\\\\.\\c:", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, NULL, NULL);
			if (VolumeOpen_handle == INVALID_HANDLE_VALUE) {
				UpdateLog(hwnd, "There are not enough rights to open the volume completely.", RGB(145, 4, 11));
				MessageBox(hwnd, "Failed to open volume handle", "Error (FT1OH)", MB_ICONERROR);

			}

			UpdateLog(hwnd, "Volume Opened!", RGB(255, 255, 255));
			Sleep(200);

			USN_JOURNAL_DATA journalData = { 0 };
			DWORD ReturnedBytes = { 0 };
			if (!DeviceIoControl(VolumeOpen_handle, FSCTL_QUERY_USN_JOURNAL, NULL, 0, &journalData, sizeof(journalData), &ReturnedBytes, NULL)) {
				UpdateLog(hwnd, "USN Journal not found or you PC", RGB(145, 4, 11));
				MessageBox(hwnd, "Mode: Lite - USN Journal not found or you PC, [ Debugging ]", "Error (FT1QJ)", MB_ICONERROR);

			}

			UpdateLog(hwnd, "USN Journal Found!", RGB(106, 227, 20));
			CloseHandle(VolumeOpen_handle);
			break;
		}

	case WM_PAINT: {

		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hwnd, &ps);
		SetBkColor(hdc, RGB(0, 0, 0));
		SetTextColor(hdc, RGB(255, 255, 255));

		RECT CreatorRect = { 800, 350, 0, 500 };
		RECT ConsoleRectTop = { 800, 50, 0, 55 };
		HBRUSH ClientColorBrush = CreateSolidBrush(RGB(255, 255, 255));

		FillRect(hdc, &CreatorRect, ClientColorBrush);
		FillRect(hdc, &ConsoleRectTop, ClientColorBrush);
		SetTextColor(hdc, logColor);

		TextOut(hdc, 10, 70, "Operation log:", (int)strlen("Operation log:"));
		TextOut(hdc, 10, 90, operationLog, (int)strlen(operationLog));

		SetTextColor(hdc, RGB(255, 255, 255));
		
		TextOut(hdc, 270, 120, "Clearing Windows Log Mods:", (int)strlen("Clearing Windows Log Mods:"));

		SetBkColor(hdc, RGB(255, 255, 255));
		SetTextColor(hdc, RGB(0, 0, 0));

		TextOut(hdc, 260, 380, "# <_O Created By Spectre2Ux", (int)strlen("# <_O Created By Spectre2Ux"));
		TextOut(hdc, 250, 410, "V0.3 Alpha, bugs may be present.", (int)strlen("V0.3 Alpha, bugs may be present."));

		EndPaint(hwnd, &ps);
		break;
	}
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
	return 0;
}



void DeleteJournal(HWND	hwnd) {
	HANDLE VolumeOpen_handle = CreateFile("\\\\.\\c:", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, NULL, NULL);
	if (VolumeOpen_handle == INVALID_HANDLE_VALUE) {
		UpdateLog(hwnd, "There are not enough rights to open the volume completely.", RGB(145, 4, 11));
		MessageBox(hwnd, "Failed to open volume handle", "Error (FT1OH)", MB_ICONERROR);

	}

	UpdateLog(hwnd, "Volume Opened!", RGB(255, 255, 255));
	Sleep(200);

	USN_JOURNAL_DATA journalData = { 0 };
	DWORD ReturnedBytes = { 0 };
	if (!DeviceIoControl(VolumeOpen_handle, FSCTL_QUERY_USN_JOURNAL, NULL, 0, &journalData, sizeof(journalData), &ReturnedBytes, NULL)) {
		UpdateLog(hwnd, "Error Querying USN Journal. Please Restart Lite Mode.", RGB(145, 4, 11));
		MessageBox(hwnd, "Mode: Lite - Error querying USN_J, [ Debugging ]", "Error (FT1QJ)", MB_ICONERROR);

	}

	UpdateLog(hwnd, "USN Journal Found!", RGB(255, 255, 255));
	Sleep(200);

	DELETE_USN_JOURNAL_DATA journalDelete = { 0 };
	journalDelete.DeleteFlags = USN_DELETE_FLAG_DELETE;
	journalDelete.UsnJournalID = journalData.UsnJournalID;

	if (!DeviceIoControl(VolumeOpen_handle, FSCTL_DELETE_USN_JOURNAL, &journalDelete, sizeof(journalDelete), NULL, 0, &ReturnedBytes, NULL)) {
		UpdateLog(hwnd, "Error Deleting USN Journal. Please Restart Lite Mode.", RGB(145, 4, 11));
		MessageBox(hwnd, "Mode: Lite - Error deleting USN_J, [ Debugging ]", "Error (FT1DJ)", MB_ICONERROR);

	}

	UpdateLog(hwnd, "USN Journal Deleted!", RGB(255, 255, 255));
	Sleep(2000);

	CREATE_USN_JOURNAL_DATA journalCreate = { 0 };
	journalCreate.MaximumSize = 0x10000000;
	journalCreate.AllocationDelta = 0x1000000;
	if (!DeviceIoControl(VolumeOpen_handle, FSCTL_CREATE_USN_JOURNAL, &journalCreate, sizeof(journalCreate), NULL, 0, &ReturnedBytes, NULL)) {
		DWORD lastError = GetLastError();
		char msg[256];
		sprintf(msg, "Error creating USN Journal. Code: %lu", lastError);
		UpdateLog(hwnd, "Error Creating New USN Journal. Please Restart Lite Mode.", RGB(145, 4, 11));
		MessageBox(hwnd, msg, "Error (FT1CJ)", MB_ICONERROR);

	}

	UpdateLog(hwnd, "USN Journal Created!", RGB(255, 255, 255));
	Sleep(300);

	UpdateLog(hwnd, "Success! Error not found.", RGB(106, 227, 20));
	CloseHandle(VolumeOpen_handle);

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
