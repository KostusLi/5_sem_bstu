#include <windows.h>
#include <tlhelp32.h>
#include <cstdio>

int main() {
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) {
        printf("CreateToolhelp32Snapshot FAILED, error: %lu\n", GetLastError());
        return 1;
    }

    PROCESSENTRY32 pe;
    pe.dwSize = sizeof(PROCESSENTRY32);

    if (!Process32First(snapshot, &pe)) {
        printf("Process32First FAILED, error: %lu\n", GetLastError());
        CloseHandle(snapshot);
        return 1;
    }

    printf("%-30s %-10s %-10s\n", "Name", "PID", "PPID");
    printf("--------------------------------------------------\n");

    do {
        printf("%-30ls %-10lu %-10lu\n", pe.szExeFile, pe.th32ProcessID, pe.th32ParentProcessID);
    } while (Process32Next(snapshot, &pe));

    CloseHandle(snapshot);
    return 0;
}