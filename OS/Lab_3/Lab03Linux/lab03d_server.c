#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>

int main(int argc, char *argv[]) {
    if (argc < 5) {
        fprintf(stderr, "Usage: lab03d_server <K> <M> <L> <R>\n");
        return 1;
    }

    int K = atoi(argv[1]);
    int M = atoi(argv[2]);
    int L = atoi(argv[3]);
    int R = atoi(argv[4]);

    struct timespec t0, t1;
    clock_gettime(CLOCK_MONOTONIC, &t0);

    int total = R - L + 1;
    int chunk = total / K;
    int remainder = total % K;

    int readFds[K];
    pid_t pids[K];

    int rangeStart = L;
    for (int i = 0; i < K; i++) {
        int len = chunk + (i == K - 1 ? remainder : 0);
        int rangeEnd = rangeStart + len - 1;

        int fds[2];
        if (pipe(fds) == -1) {
            perror("pipe failed");
            readFds[i] = -1;
            rangeStart = rangeEnd + 1;
            continue;
        }

        pid_t pid = fork();
        if (pid == 0) {

            close(fds[0]);
            dup2(fds[1], STDOUT_FILENO);
            close(fds[1]);

            char mStr[16], lStr[16], rStr[16];
            sprintf(mStr, "%d", M);
            sprintf(lStr, "%d", rangeStart);
            sprintf(rStr, "%d", rangeEnd);

            execl("./lab03d_client", "lab03d_client", mStr, lStr, rStr, (char *)NULL);
            perror("execl failed");
            _exit(127);
        }

        close(fds[1]);
        readFds[i] = fds[0];
        pids[i] = pid;

        rangeStart = rangeEnd + 1;
    }

    int allNumbers[200000];
    int allCount = 0;

    for (int i = 0; i < K; i++) {
        if (readFds[i] == -1) continue;

        int count = 0;
        read(readFds[i], &count, sizeof(count));

        if (count > 0) {
            int buf[100000];
            read(readFds[i], buf, count * sizeof(int));
            for (int j = 0; j < count; j++) {
                allNumbers[allCount++] = buf[j];
            }
        }

        close(readFds[i]);
    }

    for (int i = 0; i < K; i++) {
        int status;
        waitpid(pids[i], &status, 0);
    }

    clock_gettime(CLOCK_MONOTONIC, &t1);
    double elapsedMs = (t1.tv_sec - t0.tv_sec) * 1000.0 + (t1.tv_nsec - t0.tv_nsec) / 1e6;

    printf("M = %d\n", M);
    printf("Range: [%d, %d]\n", L, R);
    printf("Processes: %d\n", K);
    printf("Total found: %d\n", allCount);
    printf("Numbers: ");
    for (int i = 0; i < allCount; i++) printf("%d ", allNumbers[i]);
    printf("\n");
    printf("Elapsed time: %.3f ms\n", elapsedMs);

    return 0;
}
