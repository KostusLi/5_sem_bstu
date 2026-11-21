#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <random>

static const char* UserName = "User-c066fbe2";

static pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t notFull = PTHREAD_COND_INITIALIZER;
static pthread_cond_t notEmpty = PTHREAD_COND_INITIALIZER;

static int capacity;
static int* buffer;
static int head = 0, tail = 0, count = 0;
static volatile bool running = true;

static int pMin, pMax, cMin, cMax;

static double startTime;
static long long produced = 0, consumed = 0;
static long long producedPeriod = 0, consumedPeriod = 0;
static double producerWork = 0, consumerWork = 0;

static double Mono()
{
    timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec / 1e9;
}

static double Now()
{
    return Mono() - startTime;
}

static void* Producer(void*)
{
    std::mt19937 gen(std::random_device{}());
    std::uniform_int_distribution<int> delay(pMin, pMax);
    int value = 0;
    while (running)
    {
        double t0 = Mono();
        usleep(delay(gen) * 1000);
        double spent = Mono() - t0;

        pthread_mutex_lock(&mtx);
        producerWork += spent;
        while (count == capacity && running)
        {
            printf("[%7.3f] Producer BLOCKED: buffer is full (%d/%d)\n", Now(), count, capacity);
            pthread_cond_wait(&notFull, &mtx);
            printf("[%7.3f] Producer WOKE UP\n", Now());
        }
        if (!running)
        {
            pthread_mutex_unlock(&mtx);
            break;
        }
        buffer[tail] = ++value;
        tail = (tail + 1) % capacity;
        count++;
        produced++;
        producedPeriod++;
        printf("[%7.3f] Producer ADDED item %d (buffer %d/%d)\n", Now(), value, count, capacity);
        printf("[%7.3f] Waking consumer\n", Now());
        pthread_cond_signal(&notEmpty);
        fflush(stdout);
        pthread_mutex_unlock(&mtx);
    }
    return NULL;
}

static void* Consumer(void*)
{
    std::mt19937 gen(std::random_device{}());
    std::uniform_int_distribution<int> delay(cMin, cMax);
    while (running)
    {
        pthread_mutex_lock(&mtx);
        while (count == 0 && running)
        {
            printf("[%7.3f] Consumer BLOCKED: buffer is empty (%d/%d)\n", Now(), count, capacity);
            pthread_cond_wait(&notEmpty, &mtx);
            printf("[%7.3f] Consumer WOKE UP\n", Now());
        }
        if (!running)
        {
            pthread_mutex_unlock(&mtx);
            break;
        }
        int item = buffer[head];
        head = (head + 1) % capacity;
        count--;
        consumed++;
        consumedPeriod++;
        printf("[%7.3f] Consumer TOOK item %d (buffer %d/%d)\n", Now(), item, count, capacity);
        printf("[%7.3f] Waking producer\n", Now());
        pthread_cond_signal(&notFull);
        fflush(stdout);
        pthread_mutex_unlock(&mtx);

        double t0 = Mono();
        usleep(delay(gen) * 1000);
        double spent = Mono() - t0;

        pthread_mutex_lock(&mtx);
        consumerWork += spent;
        pthread_mutex_unlock(&mtx);
    }
    return NULL;
}

static void* Stats(void*)
{
    double lastReport = 0.0;
    double nextReport = 5.0;
    while (running)
    {
        while (running && Now() < nextReport) usleep(50000);
        if (!running) break;

        pthread_mutex_lock(&mtx);
        double t = Now();
        double period = t - lastReport;
        lastReport = t;
        nextReport += 5.0;
        double fill = 100.0 * count / capacity;
        double avgProd = produced > 0 ? t * 1000.0 / produced : 0.0;
        double avgCons = consumed > 0 ? consumerWork * 1000.0 / consumed : 0.0;
        printf("\n===== STATISTICS =====\n");
        printf("Run time:                       %.1f s\n", t);
        printf("Produced total:                 %lld\n", produced);
        printf("Consumed total:                 %lld\n", consumed);
        printf("Items in buffer:                %d\n", count);
        printf("Buffer fill:                    %.1f %%\n", fill);
        printf("Produced in last period:        %lld\n", producedPeriod);
        printf("Avg time between productions:   %.1f ms\n", avgProd);
        printf("Production rate:                %.2f items/s\n", producedPeriod / period);
        printf("Producer total work time:       %.2f s\n", producerWork);
        printf("Consumed in last period:        %lld\n", consumedPeriod);
        printf("Avg item processing time:       %.1f ms\n", avgCons);
        printf("Consumption rate:               %.2f items/s\n", consumedPeriod / period);
        printf("Consumer total work time:       %.2f s\n", consumerWork);
        printf("======================\n\n");
        fflush(stdout);
        producedPeriod = 0;
        consumedPeriod = 0;
        pthread_mutex_unlock(&mtx);
    }
    return NULL;
}

int main(int argc, char* argv[])
{
    if (argc != 6)
    {
        printf("Usage: Lab-06-conds [buffer size] [producer min delay] [producer max delay] [consumer min delay] [consumer max delay]\n");
        printf("Example: Lab-06-conds 10 0 100 50 150\n");
        return 1;
    }

    capacity = atoi(argv[1]);
    pMin = atoi(argv[2]);
    pMax = atoi(argv[3]);
    cMin = atoi(argv[4]);
    cMax = atoi(argv[5]);

    if (capacity <= 0 || pMin < 0 || cMin < 0 || pMin > pMax || cMin > cMax)
    {
        printf("Invalid arguments\n");
        return 1;
    }

    buffer = (int*)malloc(sizeof(int) * capacity);

    printf("User name:           %s\n", UserName);
    printf("Current buffer size: %d\n", count);
    printf("Max buffer size:     %d\n", capacity);
    printf("Press Enter to stop\n\n");
    fflush(stdout);

    startTime = Mono();

    pthread_t p, c, s;
    pthread_create(&p, NULL, Producer, NULL);
    pthread_create(&c, NULL, Consumer, NULL);
    pthread_create(&s, NULL, Stats, NULL);

    getchar();

    pthread_mutex_lock(&mtx);
    running = false;
    pthread_cond_broadcast(&notFull);
    pthread_cond_broadcast(&notEmpty);
    pthread_mutex_unlock(&mtx);

    pthread_join(p, NULL);
    pthread_join(c, NULL);
    pthread_join(s, NULL);
    free(buffer);

    printf("Stopped\n");
    return 0;
}