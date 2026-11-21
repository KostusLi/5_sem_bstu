#include <stdio.h>

static const int N = 5;
static const int M = 3;

static int total[M] = { 10, 5, 7 };
static int alloc[N][M] = {
    { 0, 1, 0 },
    { 2, 0, 0 },
    { 3, 0, 2 },
    { 2, 1, 1 },
    { 0, 0, 2 }
};
static int maxNeed[N][M] = {
    { 7, 5, 3 },
    { 3, 2, 2 },
    { 9, 0, 2 },
    { 2, 2, 2 },
    { 4, 3, 3 }
};
static int avail[M] = { 3, 3, 2 };

static int need[N][M];

static void CalcNeed()
{
    for (int i = 0; i < N; i++)
        for (int j = 0; j < M; j++)
            need[i][j] = maxNeed[i][j] - alloc[i][j];
}

static void PrintState()
{
    printf("Process | Allocation | Max     | Need    | Available\n");
    printf("        | A B C      | A B C   | A B C   | A B C\n");
    for (int i = 0; i < N; i++)
    {
        printf("   %d    | %d %d %d      | %d %d %d   | %d %d %d   |", i,
            alloc[i][0], alloc[i][1], alloc[i][2],
            maxNeed[i][0], maxNeed[i][1], maxNeed[i][2],
            need[i][0], need[i][1], need[i][2]);
        if (i == 0) printf(" %d %d %d", avail[0], avail[1], avail[2]);
        printf("\n");
    }
}

static bool IsSafe(bool print)
{
    int work[M];
    bool finish[N] = { false, false, false, false, false };
    int sequence[N];
    int count = 0;
    for (int j = 0; j < M; j++) work[j] = avail[j];

    bool found = true;
    while (found)
    {
        found = false;
        for (int i = 0; i < N; i++)
        {
            if (finish[i]) continue;
            bool fits = true;
            for (int j = 0; j < M; j++)
                if (need[i][j] > work[j]) fits = false;
            if (fits)
            {
                for (int j = 0; j < M; j++) work[j] += alloc[i][j];
                finish[i] = true;
                sequence[count++] = i;
                if (print)
                    printf("  P%d can finish, work = (%d %d %d)\n", i, work[0], work[1], work[2]);
                found = true;
            }
        }
    }

    if (count == N)
    {
        if (print)
        {
            printf("State is SAFE. Safe sequence:");
            for (int k = 0; k < N; k++) printf(" P%d%s", sequence[k], k + 1 < N ? "," : "");
            printf("\n");
        }
        return true;
    }
    if (print) printf("State is UNSAFE.\n");
    return false;
}

static bool Request(int p, const int* req)
{
    printf("\nRequest: P%d asks for (%d %d %d)\n", p, req[0], req[1], req[2]);

    for (int j = 0; j < M; j++)
    {
        if (req[j] > need[p][j])
        {
            printf("  Denied: request exceeds the declared maximum claim (need = %d %d %d)\n", need[p][0], need[p][1], need[p][2]);
            return false;
        }
    }
    for (int j = 0; j < M; j++)
    {
        if (req[j] > avail[j])
        {
            printf("  P%d must wait: not enough available resources (available = %d %d %d)\n", p, avail[0], avail[1], avail[2]);
            return false;
        }
    }

    for (int j = 0; j < M; j++)
    {
        avail[j] -= req[j];
        alloc[p][j] += req[j];
        need[p][j] -= req[j];
    }
    printf("  Resources tentatively allocated, checking safety:\n");

    if (IsSafe(true))
    {
        printf("  Request GRANTED\n");
        return true;
    }

    for (int j = 0; j < M; j++)
    {
        avail[j] += req[j];
        alloc[p][j] -= req[j];
        need[p][j] += req[j];
    }
    printf("  Request DENIED: the allocation would leave the system in an unsafe state, rolled back\n");
    return false;
}

int main()
{
    CalcNeed();

    printf("Total resources: A=%d B=%d C=%d\n\n", total[0], total[1], total[2]);
    PrintState();

    bool consistent = true;
    for (int j = 0; j < M; j++)
    {
        int sum = avail[j];
        for (int i = 0; i < N; i++) sum += alloc[i][j];
        if (sum != total[j]) consistent = false;
    }
    printf("\nCheck: allocated + available equals total: %s\n\n", consistent ? "yes" : "NO");

    printf("Part 1. Safety check of the initial state:\n");
    IsSafe(true);

    printf("\nPart 2. Resource allocation requests:\n");
    int r1[M] = { 1, 0, 2 };
    Request(1, r1);
    int r2[M] = { 3, 3, 0 };
    Request(4, r2);
    int r3[M] = { 0, 2, 0 };
    Request(0, r3);

    printf("\nState after the requests:\n");
    PrintState();

    return 0;
}