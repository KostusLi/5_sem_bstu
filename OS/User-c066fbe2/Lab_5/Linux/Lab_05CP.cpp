#include "Sched.h"

int main(int argc, char* argv[])
{
    std::vector<Task> tasks = DefaultTasks();
    int quantum = 0;
    bool sweep = false;
    bool low = false;

    for (int i = 1; i < argc; ++i)
    {
        if (ParseOption(argc, argv, i, tasks, &low))
            continue;
        if (strcmp(argv[i], "sweep") == 0)
            sweep = true;
        else
            quantum = atoi(argv[i]);
    }

    bool bigIsHigh = !low;

    if (sweep)
    {
        printf("Algorithm: preemptive priority, quantum sweep (%s)\n\n",
               bigIsHigh ? "larger number = higher priority" : "smaller number = higher priority");
        PrintSweepHeader();
        for (int q : QuantumList())
            PrintSweepRow(q, Compute(RunPriority(tasks, q, bigIsHigh)));
        return 0;
    }

    if (quantum <= 0)
    {
        printf("Usage: Lab-05CP <quantum> | sweep [-low] [-f tasks.txt]\n");
        return 1;
    }

    printf("Algorithm: preemptive priority, quantum = %d (%s)\n\n", quantum,
           bigIsHigh ? "larger number = higher priority" : "smaller number = higher priority");
    PrintAll(RunPriority(tasks, quantum, bigIsHigh));
    return 0;
}