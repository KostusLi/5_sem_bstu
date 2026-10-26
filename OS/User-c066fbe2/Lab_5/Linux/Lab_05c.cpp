#include "Lab05Common.h"
#include <pthread.h>
#include <cstdlib>

struct Args
{
    long long total;
    int nice;
};

static pthread_barrier_t barrier;

void* ThreadFunc(void* param)
{
    Args* a = (Args*)param;
    if (setpriority(PRIO_PROCESS, GetTid(), a->nice) != 0)
        perror("setpriority (negative nice needs sudo)");
    pthread_barrier_wait(&barrier);
    RunLoop(a->total);
    return nullptr;
}

int main(int argc, char* argv[])
{
    if (argc < 4)
    {
        printf("Usage: Lab_05c <affinity mask> <nice 1> <nice 2> [iterations]\n");
        return 1;
    }

    unsigned long long mask = strtoull(argv[1], nullptr, 0);
    long long total = argc > 4 ? atoll(argv[4]) : 1000000;
    Args args[2] = { { total, atoi(argv[2]) }, { total, atoi(argv[3]) } };

    cpu_set_t set;
    MaskToSet(mask, set);
    if (sched_setaffinity(0, sizeof(set), &set) != 0)
    {
        perror("sched_setaffinity");
        return 1;
    }

    printf("P1 affinity mask: %llu (%s)\n", mask, SetToBinary(set, CpuCount()).c_str());
    printf("P2 nice of thread 1: %d\n", args[0].nice);
    printf("P3 nice of thread 2: %d\n", args[1].nice);
    printf("Iterations per thread: %lld\n", total);
    fflush(stdout);

    pthread_barrier_init(&barrier, nullptr, 2);

    pthread_t th[2];
    for (int i = 0; i < 2; ++i)
    {
        if (pthread_create(&th[i], nullptr, ThreadFunc, &args[i]) != 0)
        {
            perror("pthread_create");
            return 1;
        }
    }

    for (int i = 0; i < 2; ++i)
        pthread_join(th[i], nullptr);

    pthread_barrier_destroy(&barrier);

    printf("Process wall=%.3f s, clock=%.3f s (cpu time)\n", WallSeconds(), (double)clock() / CLOCKS_PER_SEC);
    return 0;
}