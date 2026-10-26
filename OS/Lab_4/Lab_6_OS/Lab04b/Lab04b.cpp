#define _CRT_RAND_S
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

static const unsigned char kFio[] = {
    0xD5, 0xE0, 0xF2, 0xEA, 0xE5, 0xE2, 0xE8, 0xF7, 0x20, 0xCA, 0xEE, 0xED, 0xF1, 0xF2, 0xE0, 0xED,
    0xF2, 0xE8, 0xED, 0x20, 0xC2, 0xEB, 0xE0, 0xE4, 0xE8, 0xEC, 0xE8, 0xF0, 0xEE, 0xE2, 0xE8, 0xF7
};

static char gUser[257];
static DWORD gUserLen = 0;

static BOOL InitIdentity(void)
{
    DWORD len = (DWORD)sizeof(gUser);
    if (!GetUserNameA(gUser, &len) || len < 2) {
        fprintf(stderr, "GetUserName failed: %lu\n", GetLastError());
        return FALSE;
    }
    gUserLen = len - 1;
    if (!SetConsoleOutputCP(1251)) {
        fprintf(stderr, "SetConsoleOutputCP failed: %lu\n", GetLastError());
        return FALSE;
    }
    setvbuf(stdout, NULL, _IONBF, 0);
    return TRUE;
}

static void Lab_04x_Step(DWORD iter)
{
    unsigned int r = 0;
    DWORD delay = 400;
    if (rand_s(&r) == 0) {
        delay = 350 + r % 101;
    }
    printf("%lu - %lu - %lu - %c - %c\n",
        GetCurrentProcessId(),
        GetCurrentThreadId(),
        iter,
        gUser[(iter - 1) % gUserLen],
        kFio[(iter - 1) % sizeof(kFio)]);
    Sleep(delay);
}

DWORD WINAPI Lab_04x(LPVOID param)
{
    DWORD n = (DWORD)(ULONG_PTR)param;
    for (DWORD i = 1; i <= n; i++) {
        Lab_04x_Step(i);
    }
    return 0;
}

static int StartThreads(HANDLE h[2])
{
    const DWORD counts[2] = { 55, 130 };
    int created = 0;
    for (int k = 0; k < 2; k++) {
        h[k] = CreateThread(NULL, 0, Lab_04x, (LPVOID)(ULONG_PTR)counts[k], 0, NULL);
        if (h[k] == NULL) {
            fprintf(stderr, "CreateThread failed: %lu\n", GetLastError());
            break;
        }
        created++;
    }
    return created;
}

static void WaitAndClose(HANDLE h[2], int created)
{
    for (int k = 0; k < created; k++) {
        if (h[k] == NULL) {
            continue;
        }
        if (WaitForSingleObject(h[k], INFINITE) == WAIT_FAILED) {
            fprintf(stderr, "WaitForSingleObject failed: %lu\n", GetLastError());
        }
        CloseHandle(h[k]);
        h[k] = NULL;
    }
}

int main(void)
{
    HANDLE h[2] = { NULL, NULL };
    if (!InitIdentity()) {
        return 1;
    }
    int created = StartThreads(h);
    int rc = (created == 2) ? 0 : 1;
    if (created == 2) {
        for (DWORD i = 1; i <= 100; i++) {
            Lab_04x_Step(i);
            if (i == 30) {
                if (SuspendThread(h[0]) == (DWORD)-1) {
                    fprintf(stderr, "SuspendThread failed: %lu\n", GetLastError());
                    rc = 1;
                }
                else {
                    printf("[main] thread 1 suspended\n");
                }
            }
            if (i == 40) {
                if (SuspendThread(h[1]) == (DWORD)-1) {
                    fprintf(stderr, "SuspendThread failed: %lu\n", GetLastError());
                    rc = 1;
                }
                else {
                    printf("[main] thread 2 suspended\n");
                }
            }
            if (i == 70) {
                if (ResumeThread(h[0]) == (DWORD)-1) {
                    fprintf(stderr, "ResumeThread failed: %lu\n", GetLastError());
                    rc = 1;
                }
                else {
                    printf("[main] thread 1 resumed\n");
                }
            }
        }
        if (ResumeThread(h[1]) == (DWORD)-1) {
            fprintf(stderr, "ResumeThread failed: %lu\n", GetLastError());
            rc = 1;
        }
        else {
            printf("[main] thread 2 resumed\n");
        }
    }
    WaitAndClose(h, created);
    return rc;
}