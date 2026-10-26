#include <errno.h>
#include <math.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ucontext.h>

typedef struct {
    double sum;
    double part;
    unsigned long long k;
    unsigned long long remaining;
} Worker;

static Worker* gWorkers = NULL;
static ucontext_t* gCtx = NULL;
static ucontext_t gMainCtx;
static unsigned long long gNext = 0;
static unsigned long long gPart = 2000;
static unsigned long long gSlice = 1000;
static volatile sig_atomic_t gStop = 0;
static int gSwitchError = 0;

static void OnSigint(int sig)
{
    (void)sig;
    gStop = 1;
}

static void SwitchToMain(int idx)
{
    if (swapcontext(&gCtx[idx], &gMainCtx) == -1) {
        gSwitchError = 1;
    }
}

static void FiberProc(int idx)
{
    Worker* w = &gWorkers[idx];
    for (;;) {
        if (w->remaining == 0) {
            if (gStop) {
                SwitchToMain(idx);
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
        SwitchToMain(idx);
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

static double ElapsedMs(const struct timespec* t0)
{
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    return (double)(now.tv_sec - t0->tv_sec) * 1000.0 + (double)(now.tv_nsec - t0->tv_nsec) / 1000000.0;
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
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = OnSigint;
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("sigaction");
        return 1;
    }
    const size_t stackSize = 256 * 1024;
    gWorkers = (Worker*)calloc((size_t)fibers, sizeof(Worker));
    gCtx = (ucontext_t*)calloc((size_t)fibers, sizeof(ucontext_t));
    void** stacks = (void**)calloc((size_t)fibers, sizeof(void*));
    if (gWorkers == NULL || gCtx == NULL || stacks == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        free(gWorkers);
        free(gCtx);
        free(stacks);
        return 1;
    }
    int rc = 0;
    unsigned long long created = 0;
    for (unsigned long long i = 0; i < fibers; i++) {
        stacks[i] = malloc(stackSize);
        if (stacks[i] == NULL || getcontext(&gCtx[i]) == -1) {
            fprintf(stderr, "Fiber creation failed\n");
            rc = 1;
            break;
        }
        gCtx[i].uc_stack.ss_sp = stacks[i];
        gCtx[i].uc_stack.ss_size = stackSize;
        gCtx[i].uc_link = &gMainCtx;
        makecontext(&gCtx[i], (void (*)(void))FiberProc, 1, (int)i);
        created++;
    }
    if (rc == 0) {
        struct timespec t0;
        clock_gettime(CLOCK_MONOTONIC, &t0);
        unsigned long long perRound = gSlice * created;
        unsigned long long checkEvery = (perRound >= 4096) ? 1 : (4096 / perRound + 1);
        unsigned long long rounds = 0;
        double nextReport = 1000.0;
        while (!gStop && !gSwitchError) {
            for (unsigned long long i = 0; i < created && !gStop; i++) {
                if (swapcontext(&gMainCtx, &gCtx[i]) == -1) {
                    gSwitchError = 1;
                    break;
                }
            }
            rounds++;
            if (rounds % checkEvery == 0) {
                double ms = ElapsedMs(&t0);
                if (seconds > 0 && ms >= (double)seconds * 1000.0) {
                    gStop = 1;
                }
                if (ms >= nextReport) {
                    double approx = 0.0;
                    for (unsigned long long i = 0; i < created; i++) {
                        approx += gWorkers[i].sum + gWorkers[i].part;
                    }
                    printf("[%.0f s] terms: %llu, pi ~ %.10f\n", ms / 1000.0, gNext, 4.0 * approx);
                    fflush(stdout);
                    nextReport += 1000.0;
                }
            }
        }
        gStop = 1;
        for (unsigned long long i = 0; i < created; i++) {
            if (swapcontext(&gMainCtx, &gCtx[i]) == -1) {
                gSwitchError = 1;
            }
        }
        double ms = ElapsedMs(&t0);
        if (gSwitchError) {
            fprintf(stderr, "swapcontext failed\n");
            rc = 1;
        } else {
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
    }
    for (unsigned long long i = 0; i < fibers; i++) {
        free(stacks[i]);
    }
    free(stacks);
    free(gCtx);
    free(gWorkers);
    return rc;
}