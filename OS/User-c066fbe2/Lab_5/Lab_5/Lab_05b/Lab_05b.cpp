#include "Lab05Common.h"
#include <cstdlib>
#include <string>

int main(int argc, char* argv[])
{
    if (argc < 4)
    {
        printf("Usage: Lab_05b <affinity mask> <class 1> <class 2> [iterations]\n");
        return 1;
    }

    DWORD_PTR mask = (DWORD_PTR)_strtoui64(argv[1], nullptr, 0);
    DWORD classes[2] = { strtoul(argv[2], nullptr, 0), strtoul(argv[3], nullptr, 0) };
    std::string iterations = argc > 4 ? argv[4] : "1000000";

    if (!ValidMask(mask))
    {
        printf("Invalid affinity mask\n");
        return 1;
    }
    if (!ValidClass(classes[0]) || !ValidClass(classes[1]))
    {
        printf("Invalid priority class value\n");
        return 1;
    }

    DWORD_PTR pm = 0, sm = 0;
    GetProcessAffinityMask(GetCurrentProcess(), &pm, &sm);

    printf("P1 affinity mask: %llu (%s)\n", (unsigned long long)mask, ToBinary(mask, MaskWidth(sm)).c_str());
    printf("P2 priority class of child 1: %lu (%s)\n", classes[0], PriorityClassName(classes[0]));
    printf("P3 priority class of child 2: %lu (%s)\n", classes[1], PriorityClassName(classes[1]));
    printf("Iterations: %s\n", iterations.c_str());

    char path[MAX_PATH];
    GetModuleFileNameA(nullptr, path, MAX_PATH);
    std::string dir(path);
    dir = dir.substr(0, dir.find_last_of("\\/") + 1);
    std::string cmd = "\"" + dir + "Lab_05x.exe\" " + iterations;

    PROCESS_INFORMATION pi[2] = {};
    char titles[2][16] = { "Child 1", "Child 2" };

    for (int i = 0; i < 2; ++i)
    {
        STARTUPINFOA si = {};
        si.cb = sizeof(si);
        si.lpTitle = titles[i];
        std::string buf = cmd;

        if (!CreateProcessA(nullptr, &buf[0], nullptr, nullptr, FALSE,
            CREATE_NEW_CONSOLE | CREATE_SUSPENDED, nullptr, nullptr, &si, &pi[i]))
        {
            printf("CreateProcess failed: %lu\n", GetLastError());
            return 1;
        }
        if (!SetProcessAffinityMask(pi[i].hProcess, mask))
            printf("SetProcessAffinityMask failed: %lu\n", GetLastError());
        if (!SetPriorityClass(pi[i].hProcess, classes[i]))
            printf("SetPriorityClass failed: %lu\n", GetLastError());

        printf("Child %d created, pid=%lu\n", i + 1, pi[i].dwProcessId);
    }

    for (int i = 0; i < 2; ++i)
        ResumeThread(pi[i].hThread);

    HANDLE h[2] = { pi[0].hProcess, pi[1].hProcess };
    WaitForMultipleObjects(2, h, TRUE, INFINITE);

    for (int i = 0; i < 2; ++i)
    {
        CloseHandle(pi[i].hThread);
        CloseHandle(pi[i].hProcess);
    }

    printf("Both children finished\n");
    return 0;
}