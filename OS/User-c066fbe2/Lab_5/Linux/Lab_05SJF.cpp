#include "Sched.h"

int main(int argc, char* argv[])
{
    std::vector<Task> tasks = DefaultTasks();
    for (int i = 1; i < argc; ++i)
        ParseOption(argc, argv, i, tasks, nullptr);

    printf("Algorithm: Shortest Job First (non-preemptive)\n\n");
    PrintAll(RunSJF(tasks));
    return 0;
}