#include <errno.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef struct {
    uint32_t lo;
    uint32_t hi;
} Range;

static uint32_t* gPrimes = NULL;
static volatile uint32_t gCount = 0;

static int IsPrime(uint32_t n)
{
    if (n < 2) {
        return 0;
    }
    if (n < 4) {
        return 1;
    }
    if (n % 2 == 0) {
        return 0;
    }
    for (uint32_t d = 3; (unsigned long long)d * d <= n; d += 2) {
        if (n % d == 0) {
            return 0;
        }
    }
    return 1;
}

static __thread uint32_t* tData = NULL;
static __thread uint32_t tCount = 0;
static __thread uint32_t tCap = 0;

static int PushPrime(uint32_t v)
{
    if (tCount == tCap) {
        uint32_t ncap = tCap ? tCap * 2 : 1024;
        uint32_t* nd = (uint32_t*)realloc(tData, (size_t)ncap * sizeof(uint32_t));
        if (nd == NULL) {
            return 0;
        }
        tData = nd;
        tCap = ncap;
    }
    tData[tCount++] = v;
    return 1;
}

void* L4Primes(void* param)
{
    const Range* r = (const Range*)param;
    tData = NULL;
    tCount = 0;
    tCap = 0;
    uint32_t n = r->lo;
    for (;;) {
        if (IsPrime(n)) {
            if (!PushPrime(n)) {
                free(tData);
                tData = NULL;
                return (void*)(intptr_t)4;
            }
        }
        if (n == r->hi) {
            break;
        }
        n++;
    }
    for (uint32_t i = 0; i < tCount; i++) {
        gPrimes[gCount++] = tData[i];
    }
    free(tData);
    tData = NULL;
    return NULL;
}

static int CmpU32(const void* a, const void* b)
{
    uint32_t x = *(const uint32_t*)a;
    uint32_t y = *(const uint32_t*)b;
    return (x > y) - (x < y);
}

static int ParseU32(const char* s, uint32_t* out)
{
    char* end = NULL;
    errno = 0;
    unsigned long long v = strtoull(s, &end, 10);
    if (end == s || *end != '\0' || errno == ERANGE || v > 0xFFFFFFFFULL) {
        return 0;
    }
    *out = (uint32_t)v;
    return 1;
}

static int CountReference(uint32_t lo, uint32_t hi, uint32_t* out)
{
    unsigned long long total = (unsigned long long)hi - lo + 1;
    if (total > (1ULL << 27)) {
        return 0;
    }
    unsigned char* seg = (unsigned char*)calloc((size_t)total, 1);
    unsigned char* small = (unsigned char*)calloc(65537, 1);
    if (seg == NULL || small == NULL) {
        free(seg);
        free(small);
        return 0;
    }
    for (unsigned long long p = 2; p * p <= 65536; p++) {
        if (!small[p]) {
            for (unsigned long long m = p * p; m <= 65536; m += p) {
                small[m] = 1;
            }
        }
    }
    for (unsigned long long p = 2; p <= 65536 && p * p <= hi; p++) {
        if (small[p]) {
            continue;
        }
        unsigned long long first = ((unsigned long long)lo + p - 1) / p * p;
        if (first < p * p) {
            first = p * p;
        }
        for (unsigned long long m = first; m <= hi; m += p) {
            seg[m - lo] = 1;
        }
    }
    uint32_t count = 0;
    for (unsigned long long n = lo; n <= hi; n++) {
        if (n >= 2 && !seg[n - lo]) {
            count++;
        }
    }
    free(seg);
    free(small);
    *out = count;
    return 1;
}

int main(int argc, char* argv[])
{
    if (argc != 4 && argc != 5) {
        fprintf(stderr, "Usage: Lab-04p2 <threads> <low> <high> [check]\n");
        return 1;
    }
    uint32_t tcount = 0, lo = 0, hi = 0;
    if (!ParseU32(argv[1], &tcount) || !ParseU32(argv[2], &lo) || !ParseU32(argv[3], &hi) ||
        tcount == 0 || tcount > 1024 || lo > hi) {
        fprintf(stderr, "Invalid arguments\n");
        return 1;
    }
    int check = 0;
    if (argc == 5) {
        if (strcmp(argv[4], "check") != 0) {
            fprintf(stderr, "Invalid arguments\n");
            return 1;
        }
        check = 1;
    }
    unsigned long long total = (unsigned long long)hi - lo + 1;
    if (total < tcount) {
        fprintf(stderr, "Range is smaller than the number of threads\n");
        return 1;
    }
    unsigned long long chunk = total / tcount;
    unsigned long long cap = total / 2 + 2;
    if (cap > (size_t)-1 / sizeof(uint32_t)) {
        fprintf(stderr, "Range is too large\n");
        return 1;
    }
    gPrimes = (uint32_t*)malloc((size_t)cap * sizeof(uint32_t));
    Range* ranges = (Range*)calloc(tcount, sizeof(Range));
    pthread_t* th = (pthread_t*)calloc(tcount, sizeof(pthread_t));
    if (gPrimes == NULL || ranges == NULL || th == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        free(gPrimes);
        free(ranges);
        free(th);
        return 1;
    }
    int rc = 0;
    uint32_t created = 0;
    struct timespec t0, t1;
    clock_gettime(CLOCK_MONOTONIC, &t0);
    for (uint32_t k = 0; k < tcount; k++) {
        ranges[k].lo = (uint32_t)(lo + (unsigned long long)k * chunk);
        ranges[k].hi = (k == tcount - 1) ? hi : (uint32_t)(ranges[k].lo + chunk - 1);
        int err = pthread_create(&th[k], NULL, L4Primes, &ranges[k]);
        if (err != 0) {
            fprintf(stderr, "pthread_create failed: %s\n", strerror(err));
            rc = 1;
            break;
        }
        created++;
    }
    for (uint32_t k = 0; k < created; k++) {
        void* ret = NULL;
        int err = pthread_join(th[k], &ret);
        if (err != 0) {
            fprintf(stderr, "pthread_join failed: %s\n", strerror(err));
            rc = 1;
        } else if (ret != NULL) {
            fprintf(stderr, "Thread %u failed with code %ld\n", (unsigned)k, (long)(intptr_t)ret);
            rc = 1;
        }
    }
    if (rc == 0) {
        clock_gettime(CLOCK_MONOTONIC, &t1);
        fprintf(stderr, "Time: %.1f ms\n", (double)(t1.tv_sec - t0.tv_sec) * 1000.0 + (double)(t1.tv_nsec - t0.tv_nsec) / 1000000.0);
        uint32_t found = gCount;
        qsort(gPrimes, found, sizeof(uint32_t), CmpU32);
        for (uint32_t i = 0; i < found; i++) {
            printf("%u\n", (unsigned)gPrimes[i]);
        }
        printf("Total: %u\n", (unsigned)found);
        if (check) {
            uint32_t expected = 0;
            if (CountReference(lo, hi, &expected)) {
                printf("Expected: %u, lost: %ld\n", (unsigned)expected, (long)expected - (long)found);
            } else {
                printf("Reference check skipped\n");
            }
        }
    }
    free(gPrimes);
    free(ranges);
    free(th);
    return rc;
}