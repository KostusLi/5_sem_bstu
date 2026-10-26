#include "Lab05Common.h"

int main()
{
    int tid = GetTid();
    int policy = sched_getscheduler(0);
    sched_param sp = {};
    sched_getparam(0, &sp);
    int nice = GetNice(tid);

    cpu_set_t set;
    CPU_ZERO(&set);
    sched_getaffinity(0, sizeof(set), &set);

    printf("Process ID: %d\n", getpid());
    printf("Thread ID: %d\n", tid);
    printf("Scheduling policy: %s (%d)\n", PolicyName(policy), policy);
    printf("Thread priority (nice): %d\n", nice);
    printf("Thread priority (numeric, PR = 20 + nice): %d\n", 20 + nice);
    printf("Static priority (sched_priority): %d\n", sp.sched_priority);
    printf("Process affinity mask: %s\n", SetToBinary(set, CpuCount()).c_str());
    printf("Processors available to the process: %d\n", CPU_COUNT(&set));
    printf("Current processor: %d\n", sched_getcpu());
    return 0;
}