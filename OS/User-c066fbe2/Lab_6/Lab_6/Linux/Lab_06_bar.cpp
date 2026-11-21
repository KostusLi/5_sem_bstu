#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <random>

static int threadCount, matrixCount, matrixSize;
static std::vector<std::vector<int>> matrices;
static std::vector<long long> result;
static long long totalSum = 0;
static pthread_barrier_t barrierPartial;
static pthread_barrier_t barrierMerged;

struct ThreadArg
{
    int id;
};

static void* Worker(void* p)
{
    int id = ((ThreadArg*)p)->id;
    size_t cells = (size_t)matrixSize * matrixSize;

    std::vector<long long> partial(cells, 0);
    for (int m = id; m < matrixCount; m += threadCount)
    {
        for (size_t c = 0; c < cells; c++)
            partial[c] += matrices[m][c];
    }

    pthread_barrier_wait(&barrierPartial);

    for (size_t c = 0; c < cells; c++)
    {
        __atomic_fetch_add(&result[c], partial[c], __ATOMIC_SEQ_CST);
        __atomic_fetch_add(&totalSum, partial[c], __ATOMIC_SEQ_CST);
    }

    if (pthread_barrier_wait(&barrierMerged) == PTHREAD_BARRIER_SERIAL_THREAD)
    {
        printf("Result matrix (sum of %d matrices %dx%d):\n", matrixCount, matrixSize, matrixSize);
        if (matrixSize <= 10)
        {
            for (int r = 0; r < matrixSize; r++)
            {
                for (int c = 0; c < matrixSize; c++)
                    printf("%8lld", result[(size_t)r * matrixSize + c]);
                printf("\n");
            }
        }
        else
        {
            printf("(matrix is large, first 10 elements of the first row)\n");
            for (int c = 0; c < 10; c++)
                printf("%8lld", result[c]);
            printf("\n");
        }
        printf("Sum of all elements: %lld\n", totalSum);
    }
    return NULL;
}

int main(int argc, char* argv[])
{
    if (argc != 4)
    {
        printf("Usage: Lab-06-bar [threads] [matrices] [matrix size]\n");
        printf("Example: Lab-06-bar 4 100 5\n");
        return 1;
    }

    threadCount = atoi(argv[1]);
    matrixCount = atoi(argv[2]);
    matrixSize = atoi(argv[3]);
    if (threadCount <= 0 || matrixCount <= 0 || matrixSize <= 0)
    {
        printf("Invalid arguments\n");
        return 1;
    }

    size_t cells = (size_t)matrixSize * matrixSize;
    std::mt19937 gen(std::random_device{}());
    std::uniform_int_distribution<int> dist(-2000, 2000);

    matrices.assign(matrixCount, std::vector<int>(cells));
    for (int m = 0; m < matrixCount; m++)
        for (size_t c = 0; c < cells; c++)
            matrices[m][c] = dist(gen);

    result.assign(cells, 0);

    std::vector<long long> expected(cells, 0);
    long long expectedTotal = 0;
    for (int m = 0; m < matrixCount; m++)
        for (size_t c = 0; c < cells; c++)
        {
            expected[c] += matrices[m][c];
            expectedTotal += matrices[m][c];
        }

    pthread_barrier_init(&barrierPartial, NULL, threadCount);
    pthread_barrier_init(&barrierMerged, NULL, threadCount);

    std::vector<ThreadArg> args(threadCount);
    std::vector<pthread_t> h(threadCount);
    for (int i = 0; i < threadCount; i++)
    {
        args[i].id = i;
        pthread_create(&h[i], NULL, Worker, &args[i]);
    }

    for (int i = 0; i < threadCount; i++)
        pthread_join(h[i], NULL);
    pthread_barrier_destroy(&barrierPartial);
    pthread_barrier_destroy(&barrierMerged);

    bool ok = (totalSum == expectedTotal);
    for (size_t c = 0; ok && c < cells; c++)
        if (result[c] != expected[c]) ok = false;
    printf("Check against sequential sum: %s\n", ok ? "OK" : "MISMATCH");
    return ok ? 0 : 1;
}