#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <process.h>
#include <stdio.h>
#include <string.h>

static const char* UserName = "User-c066fbe2";
static CRITICAL_SECTION cs;

static void Loop(const char* who)
{
    int len = (int)strlen(UserName);
    for (int i = 1; i <= 100; i++)
    {
        if (i == 30) EnterCriticalSection(&cs);
        printf("%s: iteration %d, symbol %c\n", who, i, UserName[(i - 1) % len]);
        Sleep(100);
        if (i == 60) LeaveCriticalSection(&cs);
    }
}

static unsigned __stdcall Worker(void* arg)
{
    Loop((const char*)arg);
    return 0;
}

int main()
{
    InitializeCriticalSection(&cs);

    HANDLE h[2];
    h[0] = (HANDLE)_beginthreadex(NULL, 0, Worker, (void*)"A", 0, NULL);
    h[1] = (HANDLE)_beginthreadex(NULL, 0, Worker, (void*)"B", 0, NULL);

    Loop("main");

    WaitForMultipleObjects(2, h, TRUE, INFINITE);
    CloseHandle(h[0]);
    CloseHandle(h[1]);
    DeleteCriticalSection(&cs);

    printf("Done\n");
    getchar();
    return 0;
}