#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

static const char* UserName = "User-c066fbe2";
static pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;

static void Loop(const char* who)
{
    int len = (int)strlen(UserName);
    for (int i = 1; i <= 100; i++)
    {
        if (i == 30) pthread_mutex_lock(&mtx);
        printf("%s: iteration %d, symbol %c\n", who, i, UserName[(i - 1) % len]);
        fflush(stdout);
        usleep(100000);
        if (i == 60) pthread_mutex_unlock(&mtx);
    }
}

static void* Worker(void* arg)
{
    Loop((const char*)arg);
    return NULL;
}

int main()
{
    pthread_t a, b;
    pthread_create(&a, NULL, Worker, (void*)"A");
    pthread_create(&b, NULL, Worker, (void*)"B");

    Loop("main");

    pthread_join(a, NULL);
    pthread_join(b, NULL);
    pthread_mutex_destroy(&mtx);

    printf("Done\n");
    return 0;
}