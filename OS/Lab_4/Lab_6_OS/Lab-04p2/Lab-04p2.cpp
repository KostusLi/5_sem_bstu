#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

typedef struct {
    DWORD lo;
    DWORD hi;
} Range;

static DWORD* gPrimes = NULL;
static volatile DWORD gCount = 0;

static BOOL IsPrime(DWORD n)
{
    if (n < 2) {
        return FALSE;
    }
    if (n < 4) {
        return TRUE;
    }
    if (n % 2 == 0) {
        return FALSE;
    }
    for (DWORD d = 3; (unsigned long long)d * d <= n; d += 2) {
        if (n % d == 0) {
            return FALSE;
        }
    }
    return TRUE;
}

static __declspec(thread) DWORD* tData = NULL;
static __declspec(thread) DWORD tCount = 0;
static __declspec(thread) DWORD tCap = 0;

static BOOL PushPrime(DWORD v)
{
    if (tCount == tCap) {
        DWORD ncap = tCap ? tCap * 2 : 1024;
        DWORD* nd = (DWORD*)realloc(tData, (size_t)ncap * sizeof(DWORD));
        if (nd == NULL) {
            return FALSE;
        }
        tData = nd;
        tCap = ncap;
    }
    tData[tCount++] = v;
    return TRUE;
}

DWORD WINAPI L4Primes(LPVOID param)
{
    const Range* r = (const Range*)param;
    tData = NULL;
    tCount = 0;
    tCap = 0;
    DWORD n = r->lo;
    for (;;) {
        if (IsPrime(n)) {
            if (!PushPrime(n)) {
                free(tData);
                tData = NULL;
                return 3;
            }
        }
        if (n == r->hi) {
            break;
        }
        n++;
    }
    for (DWORD i = 0; i < tCount; i++) {
        gPrimes[gCount++] = tData[i];
    }
    free(tData);
    tData = NULL;
    return 0;
}

static int CmpDword(const void* a, const void* b)
{
    DWORD x = *(const DWORD*)a;
    DWORD y = *(const DWORD*)b;
    return (x > y) - (x < y);
}

static BOOL ParseU32(const char* s, DWORD* out)
{
    char* end = NULL;
    errno = 0;
    unsigned long long v = strtoull(s, &end, 10);
    if (end == s || *end != '\0' || errno == ERANGE || v > 0xFFFFFFFFULL) {
        return FALSE;
    }
    *out = (DWORD)v;
    return TRUE;
}

static BOOL CountReference(DWORD lo, DWORD hi, DWORD* out)
{
    unsigned long long total = (unsigned long long)hi - lo + 1;
    if (total > (1ULL << 27)) {
        return FALSE;
    }
    unsigned char* seg = (unsigned char*)calloc((size_t)total, 1);
    unsigned char* smallik = (unsigned char*)calloc(65537, 1);
    if (seg == NULL || smallik == NULL) {
        free(seg);
        free(smallik);
        return FALSE;
    }
    for (unsigned long long p = 2; p * p <= 65536; p++) {
        if (!smallik[p]) {
            for (unsigned long long m = p * p; m <= 65536; m += p) {
                smallik[m] = 1;
            }
        }
    }
    for (unsigned long long p = 2; p <= 65536 && p * p <= hi; p++) {
        if (smallik[p]) {
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
    DWORD count = 0;
    for (unsigned long long n = lo; n <= hi; n++) {
        if (n >= 2 && !seg[n - lo]) {
            count++;
        }
    }
    free(seg);
    free(smallik);
    *out = count;
    return TRUE;
}

int main(int argc, char* argv[])
{
    if (argc != 4 && argc != 5) {
        fprintf(stderr, "Usage: Lab-04p2 <threads> <low> <high> [check]\n");
        return 1;
    }
    DWORD tcount = 0, lo = 0, hi = 0;
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
    if (cap > (size_t)-1 / sizeof(DWORD)) {
        fprintf(stderr, "Range is too large\n");
        return 1;
    }
    gPrimes = (DWORD*)malloc((size_t)cap * sizeof(DWORD));
    Range* ranges = (Range*)calloc(tcount, sizeof(Range));
    HANDLE* h = (HANDLE*)calloc(tcount, sizeof(HANDLE));
    if (gPrimes == NULL || ranges == NULL || h == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        free(gPrimes);
        free(ranges);
        free(h);
        return 1;
    }
    int rc = 0;
    DWORD created = 0;
    LARGE_INTEGER freq, t0, t1;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&t0);
    for (DWORD k = 0; k < tcount; k++) {
        ranges[k].lo = (DWORD)(lo + (unsigned long long)k * chunk);
        ranges[k].hi = (k == tcount - 1) ? hi : (DWORD)(ranges[k].lo + chunk - 1);
        h[k] = CreateThread(NULL, 0, L4Primes, &ranges[k], 0, NULL);
        if (h[k] == NULL) {
            fprintf(stderr, "CreateThread failed: %lu\n", GetLastError());
            rc = 1;
            break;
        }
        created++;
    }
    for (DWORD k = 0; k < created; k++) {
        DWORD code = 0;
        if (WaitForSingleObject(h[k], INFINITE) == WAIT_FAILED) {
            fprintf(stderr, "WaitForSingleObject failed: %lu\n", GetLastError());
            rc = 1;
        }
        else if (GetExitCodeThread(h[k], &code) && code != 0) {
            fprintf(stderr, "Thread %lu failed with code %lu\n", k, code);
            rc = 1;
        }
        CloseHandle(h[k]);
    }
    if (rc == 0) {
        QueryPerformanceCounter(&t1);
        fprintf(stderr, "Time: %.1f ms\n", (double)(t1.QuadPart - t0.QuadPart) * 1000.0 / (double)freq.QuadPart);
        DWORD found = gCount;
        qsort(gPrimes, found, sizeof(DWORD), CmpDword);
        for (DWORD i = 0; i < found; i++) {
            printf("%lu\n", (unsigned long)gPrimes[i]);
        }
        printf("Total: %lu\n", (unsigned long)found);
        if (check) {
            DWORD expected = 0;
            if (CountReference(lo, hi, &expected)) {
                printf("Expected: %lu, lost: %ld\n", (unsigned long)expected, (long)expected - (long)found);
            }
            else {
                printf("Reference check skipped\n");
            }
        }
    }
    free(gPrimes);
    free(ranges);
    free(h);
    return rc;
}