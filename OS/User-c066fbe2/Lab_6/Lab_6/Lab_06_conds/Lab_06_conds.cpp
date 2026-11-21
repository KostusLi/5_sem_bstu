#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <process.h>
#include <stdio.h>
#include <stdlib.h>
#include <random>

static const char* UserName = "User-c066fbe2";

static CRITICAL_SECTION cs;
static CONDITION_VARIABLE notFull;
static CONDITION_VARIABLE notEmpty;

static int capacity;
static int* buffer;
static int head = 0, tail = 0, count = 0;
static volatile bool running = true;

static int pMin, pMax, cMin, cMax;

static ULONGLONG startTick;
static long long produced = 0, consumed = 0;
static long long producedPeriod = 0, consumedPeriod = 0;
static double producerWork = 0, consumerWork = 0;

static double Now()
{
    return (double)(GetTickCount64() - startTick) / 1000.0;
}

static unsigned __stdcall Producer(void*)
{
    std::mt19937 gen(GetCurrentThreadId() ^ (unsigned)GetTickCount64());
    std::uniform_int_distribution<int> delay(pMin, pMax);
    int value = 0;
    while (running)
    {
        ULONGLONG t0 = GetTickCount64();
        Sleep(delay(gen));
        double spent = (double)(GetTickCount64() - t0) / 1000.0;

        EnterCriticalSection(&cs);
        producerWork += spent;
        while (count == capacity && running)
        {
            printf("[%7.3f] Producer BLOCKED: buffer is full (%d/%d)\n", Now(), count, capacity);
            SleepConditionVariableCS(&notFull, &cs, INFINITE);
            printf("[%7.3f] Producer WOKE UP\n", Now());
        }
        if (!running)
        {
            LeaveCriticalSection(&cs);
            break;
        }
        buffer[tail] = ++value;
        tail = (tail + 1) % capacity;
        count++;
        produced++;
        producedPeriod++;
        printf("[%7.3f] Producer ADDED item %d (buffer %d/%d)\n", Now(), value, count, capacity);
        printf("[%7.3f] Waking consumer\n", Now());
        WakeConditionVariable(&notEmpty);
        LeaveCriticalSection(&cs);
    }
    return 0;
}

static unsigned __stdcall Consumer(void*)
{
    std::mt19937 gen(GetCurrentThreadId() ^ (unsigned)GetTickCount64());
    std::uniform_int_distribution<int> delay(cMin, cMax);
    while (running)
    {
        EnterCriticalSection(&cs);
        while (count == 0 && running)
        {
            printf("[%7.3f] Consumer BLOCKED: buffer is empty (%d/%d)\n", Now(), count, capacity);
            SleepConditionVariableCS(&notEmpty, &cs, INFINITE);
            printf("[%7.3f] Consumer WOKE UP\n", Now());
        }
        if (!running)
        {
            LeaveCriticalSection(&cs);
            break;
        }
        int item = buffer[head];
        head = (head + 1) % capacity;
        count--;
        consumed++;
        consumedPeriod++;
        printf("[%7.3f] Consumer TOOK item %d (buffer %d/%d)\n", Now(), item, count, capacity);
        printf("[%7.3f] Waking producer\n", Now());
        WakeConditionVariable(&notFull);
        LeaveCriticalSection(&cs);

        ULONGLONG t0 = GetTickCount64();
        Sleep(delay(gen));
        double spent = (double)(GetTickCount64() - t0) / 1000.0;

        EnterCriticalSection(&cs);
        consumerWork += spent;
        LeaveCriticalSection(&cs);
    }
    return 0;
}

static unsigned __stdcall Stats(void*)
{
    double lastReport = 0.0;
    double nextReport = 5.0;
    while (running)
    {
        while (running && Now() < nextReport) Sleep(50);
        if (!running) break;

        EnterCriticalSection(&cs);
        double t = Now();
        double period = t - lastReport;
        lastReport = t;
        nextReport += 5.0;
        double fill = 100.0 * count / capacity;
        double avgProd = produced > 0 ? t * 1000.0 / produced : 0.0;
        double avgCons = consumed > 0 ? consumerWork * 1000.0 / consumed : 0.0;
        printf("\n===== STATISTICS =====\n");
        printf("Run time:                       %.1f s\n", t);
        printf("Produced total:                 %lld\n", produced);
        printf("Consumed total:                 %lld\n", consumed);
        printf("Items in buffer:                %d\n", count);
        printf("Buffer fill:                    %.1f %%\n", fill);
        printf("Produced in last period:        %lld\n", producedPeriod);
        printf("Avg time between productions:   %.1f ms\n", avgProd);
        printf("Production rate:                %.2f items/s\n", producedPeriod / period);
        printf("Producer total work time:       %.2f s\n", producerWork);
        printf("Consumed in last period:        %lld\n", consumedPeriod);
        printf("Avg item processing time:       %.1f ms\n", avgCons);
        printf("Consumption rate:               %.2f items/s\n", consumedPeriod / period);
        printf("Consumer total work time:       %.2f s\n", consumerWork);
        printf("======================\n\n");
        producedPeriod = 0;
        consumedPeriod = 0;
        LeaveCriticalSection(&cs);
    }
    return 0;
}

int main(int argc, char* argv[])
{
    if (argc != 6)
    {
        printf("Usage: Lab-06-conds [buffer size] [producer min delay] [producer max delay] [consumer min delay] [consumer max delay]\n");
        printf("Example: Lab-06-conds 10 0 100 50 150\n");
        return 1;
    }

    capacity = atoi(argv[1]);
    pMin = atoi(argv[2]);
    pMax = atoi(argv[3]);
    cMin = atoi(argv[4]);
    cMax = atoi(argv[5]);

    if (capacity <= 0 || pMin < 0 || cMin < 0 || pMin > pMax || cMin > cMax)
    {
        printf("Invalid arguments\n");
        return 1;
    }

    buffer = (int*)malloc(sizeof(int) * capacity);
    InitializeCriticalSection(&cs);
    InitializeConditionVariable(&notFull);
    InitializeConditionVariable(&notEmpty);

    printf("User name:           %s\n", UserName);
    printf("Current buffer size: %d\n", count);
    printf("Max buffer size:     %d\n", capacity);
    printf("Press Enter to stop\n\n");

    startTick = GetTickCount64();

    HANDLE h[3];
    h[0] = (HANDLE)_beginthreadex(NULL, 0, Producer, NULL, 0, NULL);
    h[1] = (HANDLE)_beginthreadex(NULL, 0, Consumer, NULL, 0, NULL);
    h[2] = (HANDLE)_beginthreadex(NULL, 0, Stats, NULL, 0, NULL);

    getchar();

    EnterCriticalSection(&cs);
    running = false;
    WakeAllConditionVariable(&notFull);
    WakeAllConditionVariable(&notEmpty);
    LeaveCriticalSection(&cs);

    WaitForMultipleObjects(3, h, TRUE, INFINITE);
    for (int i = 0; i < 3; i++) CloseHandle(h[i]);
    DeleteCriticalSection(&cs);
    free(buffer);

    printf("Stopped\n");
    return 0;
}