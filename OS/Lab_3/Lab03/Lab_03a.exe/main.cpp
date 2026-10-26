#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <cstdio>
#include <cstring>

const char* EXE_PATH = "D:\\5_sem_bstu\\OS\\Lab_3\\Lab03\\x64\\Debug\\Lab_03x.exe";

BOOL runProcess(LPSTR appName, LPSTR cmdLine, PROCESS_INFORMATION* pi) {
    STARTUPINFOA si = { sizeof(si) };
    ZeroMemory(pi, sizeof(*pi));

    BOOL ok = CreateProcessA(appName, cmdLine, NULL, NULL, FALSE, 0, NULL, NULL, &si, pi);
    if (!ok) {
        printf("CreateProcess FAILED (app=[%s], cmd=[%s]), error code: %lu\n",
            appName ? appName : "(null)", cmdLine ? cmdLine : "(null)", GetLastError());
    }
    else {
        printf("Process created, PID: %lu\n", pi->dwProcessId);
    }
    return ok;
}

int main() {
    PROCESS_INFORMATION pi1, pi2, pi3;
    char buf1[256], buf2[256], buf3[256], cmd3[16];

    sprintf(buf1, "%s 50", EXE_PATH);
    BOOL ok1 = runProcess(buf1, NULL, &pi1);

    sprintf(buf2, "%s 50", EXE_PATH);
    BOOL ok2 = runProcess(NULL, buf2, &pi2);

    strcpy(buf3, EXE_PATH);
    strcpy(cmd3, " 50");
    BOOL ok3 = runProcess(buf3, cmd3, &pi3);

    HANDLE handles[3];
    int count = 0;
    if (ok1) handles[count++] = pi1.hProcess;
    if (ok2) handles[count++] = pi2.hProcess;
    if (ok3) handles[count++] = pi3.hProcess;

    if (count > 0) {
        WaitForMultipleObjects(count, handles, TRUE, INFINITE);
    }
    printf("All children finished.\n");

    if (ok1) { CloseHandle(pi1.hProcess); CloseHandle(pi1.hThread); }
    if (ok2) { CloseHandle(pi2.hProcess); CloseHandle(pi2.hThread); }
    if (ok3) { CloseHandle(pi3.hProcess); CloseHandle(pi3.hThread); }

    return 0;
}