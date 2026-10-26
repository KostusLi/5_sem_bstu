#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <errno.h>

typedef struct {
    double sum;
    double part;
    unsigned long long k;
    unsigned long long remaining;
} Worker;

static Worker* gWorkers = NULL;
static LPVOID* gFibers = NULL;
static LPVOID gMainFiber = NULL;
static unsigned long long gNext = 0;
static unsigned long long gPart = 2000;
static unsigned long long gSlice = 1000;
static volatile LONG gStop = 0;

static BOOL WINAPI CtrlHandler(DWORD type)
{
    if (type == CTRL_C_EVENT || type == CTRL_BREAK_EVENT) {
        InterlockedExchange(&gStop, 1);
        return TRUE;
    }
    return FALSE;
}

static VOID WINAPI FiberProc(LPVOID param)
{
    Worker* w = (Worker*)param;
    for (;;) {
        if (w->remaining == 0) {
            if (gStop) {
                SwitchToFiber(gMainFiber);
                continue;
            }
            w->k = gNext;
            w->remaining = gPart;
            w->part = 0.0;
            gNext += gPart;
        }
        unsigned long long todo = gStop ? w->remaining : (gSlice < w->remaining ? gSlice : w->remaining);
        unsigned long long k = w->k;
        double part = w->part;
        for (unsigned long long i = 0; i < todo; i++, k++) {
            double term = 1.0 / (double)(2 * k + 1);
            part += (k & 1) ? -term : term;
        }
        w->k = k;
        w->part = part;
        w->remaining -= todo;
        if (w->remaining == 0) {
            w->sum += w->part;
            w->part = 0.0;
        }
        SwitchToFiber(gMainFiber);
    }
}

static int ParseU64(const char* s, unsigned long long* out)
{
    char* end = NULL;
    errno = 0;
    unsigned long long v = strtoull(s, &end, 10);
    if (end == s || *end != '\0' || errno == ERANGE || s[0] == '-') {
        return 0;
    }
    *out = v;
    return 1;
}

static double ElapsedMs(LARGE_INTEGER t0, LARGE_INTEGER freq)
{
    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);
    return (double)(now.QuadPart - t0.QuadPart) * 1000.0 / (double)freq.QuadPart;
}

int main(int argc, char* argv[])
{
    if (argc < 2 || argc > 5) {
        fprintf(stderr, "Usage: Lab-04pi <fibers> [partSize] [slice] [seconds]\n");
        return 1;
    }
    unsigned long long fibers = 0, seconds = 0;
    if (!ParseU64(argv[1], &fibers) || fibers == 0 || fibers > 1000 ||
        (argc > 2 && (!ParseU64(argv[2], &gPart) || gPart == 0 || gPart > 1000000000ULL)) ||
        (argc > 3 && (!ParseU64(argv[3], &gSlice) || gSlice == 0)) ||
        (argc > 4 && (!ParseU64(argv[4], &seconds) || seconds > 86400))) {
        fprintf(stderr, "Invalid arguments\n");
        return 1;
    }
    gMainFiber = ConvertThreadToFiber(NULL);
    if (gMainFiber == NULL) {
        fprintf(stderr, "ConvertThreadToFiber failed: %lu\n", GetLastError());
        return 1;
    }
    if (!SetConsoleCtrlHandler(CtrlHandler, TRUE)) {
        fprintf(stderr, "SetConsoleCtrlHandler failed: %lu\n", GetLastError());
        ConvertFiberToThread();
        return 1;
    }
    gWorkers = (Worker*)calloc((size_t)fibers, sizeof(Worker));
    gFibers = (LPVOID*)calloc((size_t)fibers, sizeof(LPVOID));
    if (gWorkers == NULL || gFibers == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        free(gWorkers);
        free(gFibers);
        SetConsoleCtrlHandler(CtrlHandler, FALSE);
        ConvertFiberToThread();
        return 1;
    }
    int rc = 0;
    unsigned long long created = 0;
    for (unsigned long long i = 0; i < fibers; i++) {
        gFibers[i] = CreateFiber(0, FiberProc, &gWorkers[i]);
        if (gFibers[i] == NULL) {
            fprintf(stderr, "CreateFiber failed: %lu\n", GetLastError());
            rc = 1;
            break;
        }
        created++;
    }
    if (rc == 0) {
        LARGE_INTEGER freq, t0;
        QueryPerformanceFrequency(&freq);
        QueryPerformanceCounter(&t0);
        unsigned long long perRound = gSlice * created;
        unsigned long long checkEvery = (perRound >= 4096) ? 1 : (4096 / perRound + 1);
        unsigned long long rounds = 0;
        double nextReport = 1000.0;
        while (!gStop) {
            for (unsigned long long i = 0; i < created && !gStop; i++) {
                SwitchToFiber(gFibers[i]);
            }
            rounds++;
            if (rounds % checkEvery == 0) {
                double ms = ElapsedMs(t0, freq);
                if (seconds > 0 && ms >= (double)seconds * 1000.0) {
                    InterlockedExchange(&gStop, 1);
                }
                if (ms >= nextReport) {
                    double approx = 0.0;
                    for (unsigned long long i = 0; i < created; i++) {
                        approx += gWorkers[i].sum + gWorkers[i].part;
                    }
                    printf("[%.0f s] terms: %llu, pi ~ %.10f\n", ms / 1000.0, gNext, 4.0 * approx);
                    nextReport += 1000.0;
                }
            }
        }
        for (unsigned long long i = 0; i < created; i++) {
            SwitchToFiber(gFibers[i]);
        }
        double ms = ElapsedMs(t0, freq);
        double total = 0.0;
        for (unsigned long long i = 0; i < created; i++) {
            total += gWorkers[i].sum;
        }
        double pi = 4.0 * total;
        printf("Pi = %.12f\n", pi);
        printf("Error = %.3e\n", fabs(pi - 3.14159265358979323846));
        printf("Terms = %llu\n", gNext);
        printf("Time: %.1f ms\n", ms);
        printf("Rate: %.2f Mterms/s\n", (double)gNext / (ms / 1000.0) / 1e6);
    }
    for (unsigned long long i = 0; i < created; i++) {
        DeleteFiber(gFibers[i]);
    }
    SetConsoleCtrlHandler(CtrlHandler, FALSE);
    ConvertFiberToThread();
    free(gWorkers);
    free(gFibers);
    return rc;
}