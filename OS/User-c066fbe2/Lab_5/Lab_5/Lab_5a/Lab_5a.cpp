#include "Lab05Common.h"

int main()
{
    HANDLE hp = GetCurrentProcess();
    HANDLE ht = GetCurrentThread();

    DWORD_PTR pm = 0, sm = 0;
    GetProcessAffinityMask(hp, &pm, &sm);
    int width = MaskWidth(sm);

    DWORD cls = GetPriorityClass(hp);
    int tp = GetThreadPriority(ht);

    printf("Process ID: %lu\n", GetCurrentProcessId());
    printf("Thread ID: %lu\n", GetCurrentThreadId());
    printf("Process priority class: %s (0x%lX)\n", PriorityClassName(cls), cls);
    printf("Thread priority: %s\n", ThreadPriorityName(tp));
    printf("Thread priority (numeric value): %d\n", tp);
    printf("Thread base priority level: %d\n", BasePriority(cls, tp));
    printf("Process affinity mask: %s\n", ToBinary(pm, width).c_str());
    printf("System affinity mask:  %s\n", ToBinary(sm, width).c_str());
    printf("Processors available to the process: %d\n", PopCount(pm));
    printf("Current processor: %lu\n", GetCurrentProcessorNumber());

    printf("Press Enter...");
    getchar();
    return 0;
}