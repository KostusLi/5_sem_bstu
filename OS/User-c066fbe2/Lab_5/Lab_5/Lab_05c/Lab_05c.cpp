#include "Lab05Common.h"
#include <cstdlib>

DWORD WINAPI ThreadFunc(LPVOID param)
{
    RunLoop(*(long long*)param);
    return 0;
}

int main(int argc, char* argv[])
{
    if (argc < 5)
    {
        printf("Usage: Lab_05c <affinity mask> <process class> <thread 1 priority> <thread 2 priority> [iterations]\n");
        return 1;
    }

    DWORD_PTR mask = (DWORD_PTR)_strtoui64(argv[1], nullptr, 0);
    DWORD cls = strtoul(argv[2], nullptr, 0);
    int prio[2] = { atoi(argv[3]), atoi(argv[4]) };
    long long total = argc > 5 ? _atoi64(argv[5]) : 1000000;

    if (!ValidMask(mask))
    {
        printf("Invalid affinity mask\n");
        return 1;
    }
    if (!ValidClass(cls))
    {
        printf("Invalid process priority class value\n");
        return 1;
    }
    if (!ValidThreadPriority(prio[0]) || !ValidThreadPriority(prio[1]))
    {
        printf("Invalid thread priority value\n");
        return 1;
    }

    if (!SetProcessAffinityMask(GetCurrentProcess(), mask))
        printf("SetProcessAffinityMask failed: %lu\n", GetLastError());
    if (!SetPriorityClass(GetCurrentProcess(), cls))
        printf("SetPriorityClass failed: %lu\n", GetLastError());

    DWORD_PTR pm = 0, sm = 0;
    GetProcessAffinityMask(GetCurrentProcess(), &pm, &sm);

    printf("P1 affinity mask: %llu (%s)\n", (unsigned long long)mask, ToBinary(pm, MaskWidth(sm)).c_str());
    printf("P2 process priority class: %lu (%s)\n", cls, PriorityClassName(cls));
    printf("P3 priority of thread 1: %d (%s)\n", prio[0], ThreadPriorityName(prio[0]));
    printf("P4 priority of thread 2: %d (%s)\n", prio[1], ThreadPriorityName(prio[1]));
    printf("Iterations per thread: %lld\n", total);

    HANDLE h[2];
    for (int i = 0; i < 2; ++i)
    {
        h[i] = CreateThread(nullptr, 0, ThreadFunc, &total, CREATE_SUSPENDED, nullptr);
        if (h[i] == nullptr)
        {
            printf("CreateThread failed: %lu\n", GetLastError());
            return 1;
        }
        if (!SetThreadPriority(h[i], prio[i]))
            printf("SetThreadPriority failed: %lu\n", GetLastError());
    }

    for (int i = 0; i < 2; ++i)
        ResumeThread(h[i]);

    WaitForMultipleObjects(2, h, TRUE, INFINITE);

    for (int i = 0; i < 2; ++i)
        CloseHandle(h[i]);

    printf("Process clock: %.3f s\n", (double)clock() / CLOCKS_PER_SEC);
    printf("Press Enter...");
    getchar();
    return 0;
}