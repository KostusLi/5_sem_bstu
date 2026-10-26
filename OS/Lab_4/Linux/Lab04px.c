#include <pthread.h>
#include <pwd.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/syscall.h>
#include <time.h>
#include <unistd.h>

static const char* const kFio[] = {
    "Х", "а", "т", "к", "е", "в", "и", "ч",
    " ", "К", "о", "н", "с", "т", "а", "н",
    "т", "и", "н", " ", "В", "л", "а", "д",
    "и", "м", "и", "р", "о", "в", "и", "ч"
};
static const size_t kFioLen = sizeof(kFio) / sizeof(kFio[0]);

static char gUser[256];
static size_t gUserLen = 0;

static int InitIdentity(void)
{
    struct passwd* pw = getpwuid(geteuid());
    if (pw == NULL || pw->pw_name == NULL || pw->pw_name[0] == '\0') {
        fprintf(stderr, "getpwuid failed\n");
        return 0;
    }
    strncpy(gUser, pw->pw_name, sizeof(gUser) - 1);
    gUser[sizeof(gUser) - 1] = '\0';
    gUserLen = strlen(gUser);
    setvbuf(stdout, NULL, _IONBF, 0);
    return 1;
}

static void Lab_04x_Step(unsigned long iter, unsigned int* seed)
{
    long tid = syscall(SYS_gettid);
    printf("%d - %ld - %lu - %c - %s\n",
           (int)getpid(),
           tid,
           iter,
           gUser[(iter - 1) % gUserLen],
           kFio[(iter - 1) % kFioLen]);
    struct timespec ts;
    long ms = 350 + (long)(rand_r(seed) % 101);
    ts.tv_sec = ms / 1000;
    ts.tv_nsec = (ms % 1000) * 1000000L;
    while (nanosleep(&ts, &ts) == -1) {
    }
}

void* Lab_04x(void* param)
{
    unsigned long n = (unsigned long)(uintptr_t)param;
    unsigned int seed = (unsigned int)syscall(SYS_gettid) ^ (unsigned int)time(NULL);
    for (unsigned long i = 1; i <= n; i++) {
        Lab_04x_Step(i, &seed);
    }
    return NULL;
}

int main(void)
{
    if (!InitIdentity()) {
        return 1;
    }
    const unsigned long counts[2] = { 50, 125 };
    pthread_t th[2];
    int created = 0;
    int rc = 0;
    for (int k = 0; k < 2; k++) {
        int err = pthread_create(&th[k], NULL, Lab_04x, (void*)(uintptr_t)counts[k]);
        if (err != 0) {
            fprintf(stderr, "pthread_create failed: %s\n", strerror(err));
            rc = 1;
            break;
        }
        created++;
    }
    if (rc == 0) {
        Lab_04x((void*)(uintptr_t)100);
    }
    for (int k = 0; k < created; k++) {
        int err = pthread_join(th[k], NULL);
        if (err != 0) {
            fprintf(stderr, "pthread_join failed: %s\n", strerror(err));
            rc = 1;
        }
    }
    return rc;
}