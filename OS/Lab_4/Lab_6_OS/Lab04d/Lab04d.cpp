#include <windows.h>
#include <tlhelp32.h>
#include <stdio.h>
#include <stdlib.h>

static DWORD WINAPI Worker(LPVOID param)
{
    (void)param;
    Sleep(20000);
    return 0;
}

static int ListThreads(DWORD pid)
{
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    if (snap == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "CreateToolhelp32Snapshot failed: %lu\n", GetLastError());
        return 1;
    }
    THREADENTRY32 te;
    te.dwSize = sizeof(te);
    if (!Thread32First(snap, &te)) {
        fprintf(stderr, "Thread32First failed: %lu\n", GetLastError());
        CloseHandle(snap);
        return 1;
    }
    int count = 0;
    do {
        if (te.th32OwnerProcessID == pid) {
            count++;
            printf("%d: TID = %lu\n", count, te.th32ThreadID);
        }
    } while (Thread32Next(snap, &te));
    printf("PID = %lu, threads: %d\n", pid, count);
    CloseHandle(snap);
    return 0;
}

int main(int argc, char* argv[])
{
    if (argc > 2) {
        fprintf(stderr, "Usage: Lab-04d [PID]\n");
        return 1;
    }
    if (argc == 2) {
        char* end = NULL;
        unsigned long pid = strtoul(argv[1], &end, 10);
        if (end == argv[1] || *end != '\0' || pid == 0) {
            fprintf(stderr, "Invalid PID: %s\n", argv[1]);
            return 1;
        }
        return ListThreads((DWORD)pid);
    }
    HANDLE h[3] = { NULL, NULL, NULL };
    int created = 0;
    for (int k = 0; k < 3; k++) {
        h[k] = CreateThread(NULL, 0, Worker, NULL, 0, NULL);
        if (h[k] == NULL) {
            fprintf(stderr, "CreateThread failed: %lu\n", GetLastError());
            break;
        }
        created++;
    }
    int rc = ListThreads(GetCurrentProcessId());
    for (int k = 0; k < created; k++) {
        WaitForSingleObject(h[k], INFINITE);
        CloseHandle(h[k]);
    }
    return rc;
}