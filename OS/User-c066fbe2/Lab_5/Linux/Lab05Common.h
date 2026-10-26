#pragma once
#include <sched.h>
#include <unistd.h>
#include <sys/resource.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <cstdio>
#include <cmath>
#include <ctime>
#include <cerrno>
#include <chrono>
#include <string>

constexpr int WORK = 150000;

inline const auto g_start = std::chrono::steady_clock::now();

inline int GetTid()
{
    return (int)syscall(SYS_gettid);
}

inline int GetNice(int tid)
{
    return getpriority(PRIO_PROCESS, tid);
}

inline const char* PolicyName(int p)
{
    switch (p & ~SCHED_RESET_ON_FORK)
    {
    case SCHED_OTHER: return "SCHED_OTHER";
    case SCHED_FIFO: return "SCHED_FIFO";
    case SCHED_RR: return "SCHED_RR";
    case SCHED_BATCH: return "SCHED_BATCH";
    case SCHED_IDLE: return "SCHED_IDLE";
    default: return "UNKNOWN";
    }
}

inline int CpuCount()
{
    return (int)sysconf(_SC_NPROCESSORS_CONF);
}

inline void MaskToSet(unsigned long long mask, cpu_set_t& set)
{
    CPU_ZERO(&set);
    for (int i = 0; i < 64; ++i)
        if ((mask >> i) & 1ULL)
            CPU_SET(i, &set);
}

inline std::string SetToBinary(const cpu_set_t& set, int width)
{
    std::string s;
    for (int i = width - 1; i >= 0; --i)
        s += CPU_ISSET(i, &set) ? '1' : '0';
    return s;
}

inline double WallSeconds()
{
    return std::chrono::duration<double>(std::chrono::steady_clock::now() - g_start).count();
}

inline void DoWork(int n)
{
    volatile double x = 0;
    for (int i = 0; i < n; ++i)
        x = x + std::sqrt((double)i);
}

inline void RunLoop(long long total)
{
    int pid = getpid();
    int tid = GetTid();
    for (long long i = 1; i <= total; ++i)
    {
        DoWork(WORK);
        if (i % 1000 == 0)
        {
            printf("iter=%lld pid=%d tid=%d nice=%d cpu=%d\n", i, pid, tid, GetNice(tid), sched_getcpu());
            fflush(stdout);
            usleep(200000);
        }
    }
    printf("tid=%d finished, wall=%.3f s, clock=%.3f s (cpu time)\n", tid, WallSeconds(), (double)clock() / CLOCKS_PER_SEC);
    fflush(stdout);
}