#include "Lab05Common.h"
#include <sys/wait.h>
#include <cstdlib>
#include <string>

int main(int argc, char* argv[])
{
    if (argc < 4)
    {
        printf("Usage: Lab_05b <affinity mask> <nice 1> <nice 2> [iterations]\n");
        return 1;
    }

    unsigned long long mask = strtoull(argv[1], nullptr, 0);
    int nice[2] = { atoi(argv[2]), atoi(argv[3]) };
    std::string iterations = argc > 4 ? argv[4] : "1000000";

    cpu_set_t set;
    MaskToSet(mask, set);
    if (sched_setaffinity(0, sizeof(set), &set) != 0)
    {
        perror("sched_setaffinity");
        return 1;
    }

    printf("P1 affinity mask: %llu (%s)\n", mask, SetToBinary(set, CpuCount()).c_str());
    printf("P2 nice of child 1: %d\n", nice[0]);
    printf("P3 nice of child 2: %d\n", nice[1]);
    printf("Iterations: %s\n", iterations.c_str());

    char path[4096];
    ssize_t n = readlink("/proc/self/exe", path, sizeof(path) - 1);
    if (n < 0)
    {
        perror("readlink");
        return 1;
    }
    path[n] = 0;
    std::string dir(path);
    dir = dir.substr(0, dir.find_last_of('/') + 1);
    std::string child = dir + "Lab_05x";

    pid_t pids[2];
    for (int i = 0; i < 2; ++i)
    {
        fflush(stdout);
        pids[i] = fork();
        if (pids[i] < 0)
        {
            perror("fork");
            return 1;
        }
        if (pids[i] == 0)
        {
            if (setpriority(PRIO_PROCESS, 0, nice[i]) != 0)
            {
                perror("setpriority (negative nice needs sudo)");
                _exit(1);
            }
            execl(child.c_str(), child.c_str(), iterations.c_str(), (char*)nullptr);
            perror("execl");
            _exit(1);
        }
        printf("Child %d created, pid=%d\n", i + 1, (int)pids[i]);
    }

    for (int done = 0; done < 2; ++done)
    {
        int status = 0;
        pid_t p = wait(&status);
        printf("Child pid=%d exited, status=%d\n", (int)p, WEXITSTATUS(status));
    }
    return 0;
}