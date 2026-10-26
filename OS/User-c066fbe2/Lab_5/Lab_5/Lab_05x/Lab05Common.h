#pragma once
#include <windows.h>
#include <cstdio>
#include <cmath>
#include <ctime>
#include <string>

constexpr int WORK = 30000;

inline const char* PriorityClassName(DWORD c)
{
    switch (c)
    {
    case IDLE_PRIORITY_CLASS: return "IDLE_PRIORITY_CLASS";
    case BELOW_NORMAL_PRIORITY_CLASS: return "BELOW_NORMAL_PRIORITY_CLASS";
    case NORMAL_PRIORITY_CLASS: return "NORMAL_PRIORITY_CLASS";
    case ABOVE_NORMAL_PRIORITY_CLASS: return "ABOVE_NORMAL_PRIORITY_CLASS";
    case HIGH_PRIORITY_CLASS: return "HIGH_PRIORITY_CLASS";
    case REALTIME_PRIORITY_CLASS: return "REALTIME_PRIORITY_CLASS";
    default: return "UNKNOWN";
    }
}

inline const char* ThreadPriorityName(int p)
{
    switch (p)
    {
    case THREAD_PRIORITY_IDLE: return "THREAD_PRIORITY_IDLE";
    case THREAD_PRIORITY_LOWEST: return "THREAD_PRIORITY_LOWEST";
    case THREAD_PRIORITY_BELOW_NORMAL: return "THREAD_PRIORITY_BELOW_NORMAL";
    case THREAD_PRIORITY_NORMAL: return "THREAD_PRIORITY_NORMAL";
    case THREAD_PRIORITY_ABOVE_NORMAL: return "THREAD_PRIORITY_ABOVE_NORMAL";
    case THREAD_PRIORITY_HIGHEST: return "THREAD_PRIORITY_HIGHEST";
    case THREAD_PRIORITY_TIME_CRITICAL: return "THREAD_PRIORITY_TIME_CRITICAL";
    default: return "UNKNOWN";
    }
}

inline int BasePriority(DWORD c, int tp)
{
    int b;
    switch (c)
    {
    case IDLE_PRIORITY_CLASS: b = 4; break;
    case BELOW_NORMAL_PRIORITY_CLASS: b = 6; break;
    case NORMAL_PRIORITY_CLASS: b = 8; break;
    case ABOVE_NORMAL_PRIORITY_CLASS: b = 10; break;
    case HIGH_PRIORITY_CLASS: b = 13; break;
    case REALTIME_PRIORITY_CLASS: b = 24; break;
    default: return -1;
    }
    if (tp == THREAD_PRIORITY_IDLE) return c == REALTIME_PRIORITY_CLASS ? 16 : 1;
    if (tp == THREAD_PRIORITY_TIME_CRITICAL) return c == REALTIME_PRIORITY_CLASS ? 31 : 15;
    return b + tp;
}

inline bool ValidClass(DWORD c)
{
    return c == IDLE_PRIORITY_CLASS || c == BELOW_NORMAL_PRIORITY_CLASS || c == NORMAL_PRIORITY_CLASS ||
        c == ABOVE_NORMAL_PRIORITY_CLASS || c == HIGH_PRIORITY_CLASS || c == REALTIME_PRIORITY_CLASS;
}

inline bool ValidThreadPriority(int p)
{
    return p == THREAD_PRIORITY_IDLE || p == THREAD_PRIORITY_LOWEST || p == THREAD_PRIORITY_BELOW_NORMAL ||
        p == THREAD_PRIORITY_NORMAL || p == THREAD_PRIORITY_ABOVE_NORMAL || p == THREAD_PRIORITY_HIGHEST ||
        p == THREAD_PRIORITY_TIME_CRITICAL;
}

inline int MaskWidth(DWORD_PTR m)
{
    int w = 0;
    while (m) { ++w; m >>= 1; }
    return w;
}

inline int PopCount(DWORD_PTR m)
{
    int c = 0;
    while (m) { c += (int)(m & 1); m >>= 1; }
    return c;
}

inline std::string ToBinary(DWORD_PTR mask, int width)
{
    std::string s;
    for (int i = width - 1; i >= 0; --i)
        s += ((mask >> i) & 1) ? '1' : '0';
    return s;
}

inline bool ValidMask(DWORD_PTR mask)
{
    DWORD_PTR pm = 0, sm = 0;
    GetProcessAffinityMask(GetCurrentProcess(), &pm, &sm);
    return mask != 0 && (mask & ~sm) == 0;
}

inline void DoWork(int n)
{
    volatile double x = 0;
    for (int i = 0; i < n; ++i)
        x = x + std::sqrt((double)i);
}

inline void RunLoop(long long total)
{
    DWORD pid = GetCurrentProcessId();
    DWORD tid = GetCurrentThreadId();
    for (long long i = 1; i <= total; ++i)
    {
        DoWork(WORK);
        if (i % 1000 == 0)
        {
            DWORD cls = GetPriorityClass(GetCurrentProcess());
            int tp = GetThreadPriority(GetCurrentThread());
            printf("iter=%lld pid=%lu tid=%lu class=%s prio=%s(%d) base=%d cpu=%lu\n",
                i, pid, tid, PriorityClassName(cls), ThreadPriorityName(tp), tp,
                BasePriority(cls, tp), GetCurrentProcessorNumber());
            Sleep(200);
        }
    }
    printf("tid=%lu finished, clock=%.3f s\n", tid, (double)clock() / CLOCKS_PER_SEC);
}