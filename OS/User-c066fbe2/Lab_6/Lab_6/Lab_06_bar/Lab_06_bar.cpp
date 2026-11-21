#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <process.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <random>

static int threadCount, matrixCount, matrixSize;
static std::vector<std::vector<int>> matrices;
static std::vector<LONG64> result;
static LONG64 totalSum = 0;
static SYNCHRONIZATION_BARRIER barrierPartial;
static SYNCHRONIZATION_BARRIER barrierMerged;

struct ThreadArg
{
    int id;
};

static unsigned __stdcall Worker(void* p)
{
    int id = ((ThreadArg*)p)->id;
    size_t cells = (size_t)matrixSize * matrixSize;

    std::vector<LONG64> partial(cells, 0);
    for (int m = id; m < matrixCount; m += threadCount)
    {
        for (size_t c = 0; c < cells; c++)
            partial[c] += matrices[m][c];
    }

    EnterSynchronizationBarrier(&barrierPartial, 0);

    for (size_t c = 0; c < cells; c++)
    {
        InterlockedExchangeAdd64(&result[c], partial[c]);
        InterlockedExchangeAdd64(&totalSum, partial[c]);
    }

    if (EnterSynchronizationBarrier(&barrierMerged, 0))
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
    return 0;
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

    std::vector<LONG64> expected(cells, 0);
    LONG64 expectedTotal = 0;
    for (int m = 0; m < matrixCount; m++)
        for (size_t c = 0; c < cells; c++)
        {
            expected[c] += matrices[m][c];
            expectedTotal += matrices[m][c];
        }

    InitializeSynchronizationBarrier(&barrierPartial, threadCount, -1);
    InitializeSynchronizationBarrier(&barrierMerged, threadCount, -1);

    std::vector<ThreadArg> args(threadCount);
    std::vector<HANDLE> h(threadCount);
    for (int i = 0; i < threadCount; i++)
    {
        args[i].id = i;
        h[i] = (HANDLE)_beginthreadex(NULL, 0, Worker, &args[i], 0, NULL);
    }

    WaitForMultipleObjects(threadCount, h.data(), TRUE, INFINITE);
    for (int i = 0; i < threadCount; i++) CloseHandle(h[i]);
    DeleteSynchronizationBarrier(&barrierPartial);
    DeleteSynchronizationBarrier(&barrierMerged);

    bool ok = (totalSum == expectedTotal);
    for (size_t c = 0; ok && c < cells; c++)
        if (result[c] != expected[c]) ok = false;
    printf("Check against sequential sum: %s\n", ok ? "OK" : "MISMATCH");
    return ok ? 0 : 1;
}