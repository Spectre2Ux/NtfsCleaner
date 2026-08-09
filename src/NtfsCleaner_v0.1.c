#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <tlhelp32.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

BOOL privileges_check() {
	BOOL elevated = FALSE;
	HANDLE tokenHandle;
	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &tokenHandle)) {
		MessageBox(NULL, "Failed to GetToken (FT1OPT)", "Error (FT1OPT)", MB_ICONERROR);
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

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpstr, int nCmdShow) {

	if (!privileges_check()) {
		MessageBox(NULL, "Please Open Programm As Administrator!", "Privileges Error.", MB_ICONERROR);
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

	HWND MainWindow = CreateWindowEx(WS_EX_CLIENTEDGE, "NTFSCleaner", "Ntfs-Cleaner (v0.1 alpha)", WS_OVERLAPPED | WS_VISIBLE | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX, CW_USEDEFAULT, CW_USEDEFAULT, 500, 500, NULL, NULL, hInstance, NULL);
	HWND ExitButton = CreateWindowEx(0, "BUTTON", "Exit", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_FLAT, 410, 10, 60, 30, MainWindow, (HMENU)1000, hInstance, NULL);
	HWND HelpButton = CreateWindowEx(0, "BUTTON", "Reference", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_FLAT, 10, 10, 100, 30, MainWindow, (HMENU)1001, hInstance, NULL);
	HWND LiteButton = CreateWindowEx(0, "BUTTON", "Lite", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_FLAT, 80, 300, 100, 30, MainWindow, (HMENU)1002, hInstance, NULL);
	HWND NormalButton = CreateWindowEx(0, "BUTTON", "Normal", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_FLAT, 190, 300, 100, 30, MainWindow, (HMENU)1003, hInstance, NULL);
	HWND ExtraButton = CreateWindowEx(0, "BUTTON", "Extra", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_FLAT, 300, 300, 100, 30, MainWindow, (HMENU)1004, hInstance, NULL);


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

}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
	case WM_COMMAND:
		if (LOWORD(wParam) == 1000) {
			PostQuitMessage(0);
			break;
		}
		else if (LOWORD(wParam) == 1002) {

			HANDLE VolumeOpen_handle = CreateFile("\\\\.\\c:", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);
			if (VolumeOpen_handle == INVALID_HANDLE_VALUE) {
				MessageBox(hwnd, "Failed to open volume handle", "Error (FT1OH)", MB_ICONERROR);
				break;
			}

			USN_JOURNAL_DATA journalData = { 0 };
			DWORD ReturnedBytes = { 0 };
			if (!DeviceIoControl(VolumeOpen_handle, FSCTL_QUERY_USN_JOURNAL, NULL, 0, &journalData, sizeof(journalData), &ReturnedBytes, NULL)) {
				MessageBox(hwnd, "Mode: Lite - Error querying USN_J, [ Debugging ]", "Error (FT1QJ)", MB_ICONERROR);
				break;
			}

			DELETE_USN_JOURNAL_DATA journalDelete = { 0 };
			journalDelete.DeleteFlags = USN_DELETE_FLAG_DELETE;
			journalDelete.UsnJournalID = journalData.UsnJournalID;

			if (!DeviceIoControl(VolumeOpen_handle, FSCTL_DELETE_USN_JOURNAL, &journalDelete, sizeof(journalDelete), NULL, 0, &ReturnedBytes, NULL)) {
				MessageBox(hwnd, "Mode: Lite - Error deleting USN_J, [ Debugging ]", "Error (FT1DJ)", MB_ICONERROR);
				break;
			}

			Sleep(2000);

			CREATE_USN_JOURNAL_DATA journalCreate = { 0 };
			journalCreate.MaximumSize = 0x10000000;
			journalCreate.AllocationDelta = 0x1000000;
			if (!DeviceIoControl(VolumeOpen_handle, FSCTL_CREATE_USN_JOURNAL, &journalCreate, sizeof(journalCreate), NULL, 0, &ReturnedBytes, NULL)) {
				DWORD lastError = GetLastError();
				char msg[256];
				sprintf(msg, "Error creating USN Journal. Code: %lu", lastError);
				MessageBox(hwnd, msg, "Error (FT1CJ)", MB_ICONERROR);

				break;
			}

			CloseHandle(VolumeOpen_handle);
			break;

		}

	case WM_PAINT: {

		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hwnd, &ps);
		SetBkColor(hdc, RGB(0, 0, 0));
		SetTextColor(hdc, RGB(255, 255, 255));


		RECT CreatorRect = { 500, 350, 0, 500 };
		RECT ConsoleRectTop = { 500, 50, 0, 55 };
		RECT ConsoleRectBottom = { 500, 240, 0, 245 };

		HBRUSH ClientColorBrush = CreateSolidBrush(RGB(255, 255, 255));

		FillRect(hdc, &CreatorRect, ClientColorBrush);
		FillRect(hdc, &ConsoleRectTop, ClientColorBrush);
		FillRect(hdc, &ConsoleRectBottom, ClientColorBrush);
		TextOut(hdc, 10, 70, "Operation log:", strlen("Operation log:"));
		TextOut(hdc, 140, 260, "Clearing Windows Log Mods:", strlen("Clearing Windows Log Mods:"));

		SetBkColor(hdc, RGB(255, 255, 255));
		SetTextColor(hdc, RGB(0, 0, 0));

		TextOut(hdc, 130, 380, "# <_O Created By Spectre2Ux", strlen("# <_O Created By Spectre2Ux"));
		TextOut(hdc, 120, 410, "V0.1 Alpha, bugs may be present.", strlen("V0.1 Alpha, bugs may be present."));

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
