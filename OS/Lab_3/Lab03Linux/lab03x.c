#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    int iterations = -1;

    if (argc > 1) {
        iterations = atoi(argv[1]);
    } else {
        char *env = getenv("ITER_NUM");
        if (env != NULL) {
            iterations = atoi(env);
        }
    }

    if (iterations <= 0) {
        _exit(1);
    }

    pid_t pid = getpid();
    printf("Iterations: %d\n", iterations);
    fflush(stdout);

    for (int i = 0; i < iterations; i++) {
        printf("PID: %d, iteration: %d\n", pid, i + 1);
        fflush(stdout);
        usleep(500 * 1000);
    }

    return 0;
}
