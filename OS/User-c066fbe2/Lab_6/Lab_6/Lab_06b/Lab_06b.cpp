#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <stdio.h>
#include <string.h>

static const char* UserName = "User-c066fbe2";
static const char* SyncName = "Lab06b_Mutex";

static void Loop(const char* who, HANDLE mutex)
{
    int len = (int)strlen(UserName);
    for (int i = 1; i <= 100; i++)
    {
        if (i == 30) WaitForSingleObject(mutex, INFINITE);
        printf("%s: iteration %d, symbol %c\n", who, i, UserName[(i - 1) % len]);
        Sleep(100);
        if (i == 60) ReleaseMutex(mutex);
    }
}

int main(int argc, char* argv[])
{
    if (argc > 1)
    {
        SetConsoleTitleA(argv[1]);
        HANDLE mutex = OpenMutexA(MUTEX_ALL_ACCESS, FALSE, SyncName);
        if (mutex == NULL)
        {
            printf("OpenMutex error: %lu\n", GetLastError());
            getchar();
            return 1;
        }
        Loop(argv[1], mutex);
        CloseHandle(mutex);
        printf("Done\n");
        getchar();
        return 0;
    }

    SetConsoleTitleA("main");
    HANDLE mutex = CreateMutexA(NULL, FALSE, SyncName);

    char path[MAX_PATH];
    GetModuleFileNameA(NULL, path, MAX_PATH);

    const char* names[2] = { "A", "B" };
    HANDLE procs[2];
    for (int k = 0; k < 2; k++)
    {
        char cmd[MAX_PATH + 16];
        sprintf(cmd, "\"%s\" %s", path, names[k]);
        STARTUPINFOA si = { sizeof(si) };
        PROCESS_INFORMATION pi = {};
        if (!CreateProcessA(NULL, cmd, NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi))
        {
            printf("CreateProcess error: %lu\n", GetLastError());
            return 1;
        }
        CloseHandle(pi.hThread);
        procs[k] = pi.hProcess;
    }

    Loop("main", mutex);

    WaitForMultipleObjects(2, procs, TRUE, INFINITE);
    CloseHandle(procs[0]);
    CloseHandle(procs[1]);
    CloseHandle(mutex);

    printf("Done\n");
    getchar();
    return 0;
}