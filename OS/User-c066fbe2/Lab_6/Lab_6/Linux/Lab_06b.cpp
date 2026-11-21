#include <semaphore.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

static const char* UserName = "User-c066fbe2";
static const char* SemName = "/Lab06b_Sem";

static void Loop(const char* who, sem_t* sem)
{
    int len = (int)strlen(UserName);
    for (int i = 1; i <= 100; i++)
    {
        if (i == 30) sem_wait(sem);
        printf("%s: iteration %d, symbol %c\n", who, i, UserName[(i - 1) % len]);
        fflush(stdout);
        usleep(100000);
        if (i == 60) sem_post(sem);
    }
}

int main(int argc, char* argv[])
{
    setvbuf(stdout, NULL, _IOLBF, 0);

    sem_unlink(SemName);
    sem_t* sem = sem_open(SemName, O_CREAT | O_EXCL, 0644, 1);
    if (sem == SEM_FAILED)
    {
        perror("sem_open");
        return 1;
    }

    const char* names[2] = { "A", "B" };
    pid_t pids[2];
    for (int k = 0; k < 2; k++)
    {
        pids[k] = fork();
        if (pids[k] < 0)
        {
            perror("fork");
            return 1;
        }
        if (pids[k] == 0)
        {
            if (argc > 1 + k)
            {
                if (freopen(argv[1 + k], "w", stdout) == NULL)
                {
                    perror("freopen");
                    _exit(1);
                }
                setvbuf(stdout, NULL, _IOLBF, 0);
            }
            Loop(names[k], sem);
            printf("Done\n");
            sem_close(sem);
            _exit(0);
        }
    }

    Loop("main", sem);

    waitpid(pids[0], NULL, 0);
    waitpid(pids[1], NULL, 0);
    sem_close(sem);
    sem_unlink(SemName);

    printf("Done\n");
    return 0;
}