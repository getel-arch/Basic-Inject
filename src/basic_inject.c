#include <windows.h>
#include <stdio.h>
#include <tlhelp32.h>

// Function to get the process ID by its name
DWORD GetProcessIdByName(const char* processName) {
    PROCESSENTRY32 pe32;
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (hSnapshot == INVALID_HANDLE_VALUE) {
        return 0;
    }

    pe32.dwSize = sizeof(PROCESSENTRY32);

    if (!Process32First(hSnapshot, &pe32)) {
        CloseHandle(hSnapshot);
        return 0;
    }

    do {
        if (strcmp(pe32.szExeFile, processName) == 0) {
            CloseHandle(hSnapshot);
            return pe32.th32ProcessID;
        }
    } while (Process32Next(hSnapshot, &pe32));

    CloseHandle(hSnapshot);
    return 0;
}

// Function to inject a DLL into a process
BOOL InjectDLL(DWORD processID, const char* dllPath) {
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processID);

    if (hProcess == NULL) {
        return FALSE;
    }

    void* pLibRemote = VirtualAllocEx(hProcess, NULL, strlen(dllPath) + 1, MEM_COMMIT, PAGE_READWRITE);

    if (pLibRemote == NULL) {
        CloseHandle(hProcess);
        return FALSE;
    }

    if (!WriteProcessMemory(hProcess, pLibRemote, (void*)dllPath, strlen(dllPath) + 1, NULL)) {
        VirtualFreeEx(hProcess, pLibRemote, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return FALSE;
    }

    HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandle("kernel32.dll"), "LoadLibraryA"), pLibRemote, 0, NULL);

    if (hThread == NULL) {
        VirtualFreeEx(hProcess, pLibRemote, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return FALSE;
    }

    WaitForSingleObject(hThread, INFINITE);

    VirtualFreeEx(hProcess, pLibRemote, 0, MEM_RELEASE);
    CloseHandle(hThread);
    CloseHandle(hProcess);

    return TRUE;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        printf("Usage: %s <process_name> <dll_full_path>\n", argv[0]);
        return 1;
    }

    const char* processName = argv[1];
    const char* dllPath = argv[2];

    DWORD processID = GetProcessIdByName(processName);

    if (processID == 0) {
        printf("Process not found: %s\n", processName);
        return 1;
    }

    if (InjectDLL(processID, dllPath)) {
        printf("Successfully injected DLL.\n");
    } else {
        printf("Failed to inject DLL.\n");
    }

    return 0;
}
