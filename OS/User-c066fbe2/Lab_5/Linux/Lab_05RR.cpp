#include "Sched.h"

int main(int argc, char* argv[])
{
    std::vector<Task> tasks = DefaultTasks();
    int quantum = 0;
    bool sweep = false;

    for (int i = 1; i < argc; ++i)
    {
        if (ParseOption(argc, argv, i, tasks, nullptr))
            continue;
        if (strcmp(argv[i], "sweep") == 0)
            sweep = true;
        else
            quantum = atoi(argv[i]);
    }

    if (sweep)
    {
        printf("Algorithm: Round Robin, quantum sweep\n\n");
        PrintSweepHeader();
        for (int q : QuantumList())
            PrintSweepRow(q, Compute(RunRR(tasks, q)));
        return 0;
    }

    if (quantum <= 0)
    {
        printf("Usage: Lab-05RR <quantum> | sweep [-f tasks.txt]\n");
        return 1;
    }

    printf("Algorithm: Round Robin, quantum = %d\n\n", quantum);
    PrintAll(RunRR(tasks, quantum));
    return 0;
}