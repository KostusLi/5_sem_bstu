#include <stdio.h>

static const int N = 4;
static const int M = 5;

static void PrintVector(const char* title, const int* v)
{
    printf("%s (", title);
    for (int j = 0; j < M; j++) printf("%d%s", v[j], j + 1 < M ? " " : "");
    printf(")\n");
}

int main()
{
    int C[N][M] = {
        { 0, 1, 1, 1, 2 },
        { 0, 1, 0, 1, 0 },
        { 0, 0, 0, 0, 1 },
        { 2, 1, 0, 0, 0 }
    };
    int R[N][M] = {
        { 1, 1, 0, 2, 1 },
        { 0, 1, 0, 2, 1 },
        { 0, 2, 0, 3, 1 },
        { 0, 2, 1, 1, 0 }
    };
    int E[M] = { 2, 4, 1, 4, 4 };
    int A[M] = { 0, 1, 0, 2, 1 };

    printf("Current allocation matrix C:\n");
    for (int i = 0; i < N; i++)
    {
        printf("  P%d:", i + 1);
        for (int j = 0; j < M; j++) printf(" %d", C[i][j]);
        printf("\n");
    }
    printf("Request matrix R:\n");
    for (int i = 0; i < N; i++)
    {
        printf("  P%d:", i + 1);
        for (int j = 0; j < M; j++) printf(" %d", R[i][j]);
        printf("\n");
    }
    PrintVector("Existing resources E =", E);
    PrintVector("Available resources A =", A);

    bool consistent = true;
    for (int j = 0; j < M; j++)
    {
        int sum = A[j];
        for (int i = 0; i < N; i++) sum += C[i][j];
        if (sum != E[j]) consistent = false;
    }
    printf("Check: sum of columns of C plus A equals E: %s\n\n", consistent ? "yes" : "NO");

    bool marked[N] = { false, false, false, false };
    int work[M];
    for (int j = 0; j < M; j++) work[j] = A[j];

    int step = 0;
    bool found = true;
    while (found)
    {
        found = false;
        for (int i = 0; i < N; i++)
        {
            if (marked[i]) continue;

            bool fits = true;
            for (int j = 0; j < M; j++)
                if (R[i][j] > work[j]) fits = false;

            if (fits)
            {
                step++;
                printf("Step %d: P%d can run (its row of R <= A). It finishes and returns its resources.\n", step, i + 1);
                for (int j = 0; j < M; j++) work[j] += C[i][j];
                marked[i] = true;
                PrintVector("  A =", work);
                found = true;
                break;
            }
            else
            {
                printf("        P%d cannot run: its row of R does not fit into A\n", i + 1);
            }
        }
    }

    printf("\n");
    bool deadlock = false;
    for (int i = 0; i < N; i++)
        if (!marked[i]) deadlock = true;

    if (!deadlock)
    {
        printf("No deadlock: all processes can finish.\n");
    }
    else
    {
        printf("DEADLOCK detected. Deadlocked processes:");
        for (int i = 0; i < N; i++)
            if (!marked[i]) printf(" P%d", i + 1);
        printf("\n");
    }
    return 0;
}