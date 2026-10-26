#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
   
    pid_t pid1 = fork();
    if (pid1 == 0) {
        execl("./lab03x", "lab03x", "6", (char *)NULL);
        perror("execl child1 failed");
        _exit(127);
    }

	setenv("ITER_NUM", "8", 1);

    pid_t pid2 = fork();
    if (pid2 == 0) {
        execl("./lab03x", "lab03x", (char *)NULL);
        perror("execl child2 failed");
        _exit(127);
    }

    printf("Parent PID: %d, child1 PID: %d, child2 PID: %d\n", getpid(), pid1, pid2);

    int status1, status2;
    waitpid(pid1, &status1, 0);
    waitpid(pid2, &status2, 0);

    printf("Both children finished.\n");
    return 0;
}
