#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>

const char* CLIENT_PATH = "D:\\5_sem_bstu\\OS\\Lab_3\\Lab03\\x64\\Debug\\Lab_03-client.exe";

int main(int argc, char* argv[]) {
    if (argc < 5) {
        fprintf(stderr, "Usage: Lab_03d-server.exe <K> <M> <L> <R>\n");
        return 1;
    }

    int K = atoi(argv[1]);
    int M = atoi(argv[2]);
    int L = atoi(argv[3]);
    int R = atoi(argv[4]);

    if (K <= 0 || M <= 0 || L < 1 || R < L) {
        fprintf(stderr, "Invalid arguments\n");
        return 1;
    }

    LARGE_INTEGER freq, t0, t1;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&t0);

    int total = R - L + 1;
    int chunk = total / K;
    int remainder = total % K;

    std::vector<HANDLE> readHandles(K, NULL);
    std::vector<PROCESS_INFORMATION> pis(K);
    for (auto& pi : pis) pi.hProcess = NULL;

    int rangeStart = L;
    for (int i = 0; i < K; i++) {
        int len = chunk + (i == K - 1 ? remainder : 0);
        int rangeEnd = rangeStart + len - 1;

        SECURITY_ATTRIBUTES sa;
        sa.nLength = sizeof(sa);
        sa.bInheritHandle = TRUE;
        sa.lpSecurityDescriptor = NULL;

        HANDLE hRead, hWrite;
        if (!CreatePipe(&hRead, &hWrite, &sa, 0)) {
            fprintf(stderr, "CreatePipe failed, error: %lu\n", GetLastError());
            rangeStart = rangeEnd + 1;
            continue;
        }

        SetHandleInformation(hRead, HANDLE_FLAG_INHERIT, 0);

        STARTUPINFOA si = { sizeof(si) };
        si.dwFlags = STARTF_USESTDHANDLES;
        si.hStdOutput = hWrite;
        si.hStdError = hWrite;
        si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);

        char cmdLine[256];
        sprintf(cmdLine, "\"%s\" %d %d %d", CLIENT_PATH, M, rangeStart, rangeEnd);

        PROCESS_INFORMATION pi = { 0 };
        BOOL ok = CreateProcessA(NULL, cmdLine, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi);

        CloseHandle(hWrite);

        if (!ok) {
            fprintf(stderr, "CreateProcess FAILED for chunk [%d, %d], error: %lu\n",
                rangeStart, rangeEnd, GetLastError());
            CloseHandle(hRead);
        }
        else {
            readHandles[i] = hRead;
            pis[i] = pi;
        }

        rangeStart = rangeEnd + 1;
    }

    std::vector<int> allNumbers;

    for (int i = 0; i < K; i++) {
        if (readHandles[i] == NULL) continue;

        int count = 0;
        DWORD bytesRead;
        BOOL ok = ReadFile(readHandles[i], &count, sizeof(count), &bytesRead, NULL);

        if (ok && bytesRead == sizeof(count) && count > 0) {
            std::vector<int> buf(count);
            ReadFile(readHandles[i], buf.data(), count * sizeof(int), &bytesRead, NULL);
            for (int n : buf) allNumbers.push_back(n);
        }

        CloseHandle(readHandles[i]);
    }

    for (int i = 0; i < K; i++) {
        if (pis[i].hProcess != NULL) {
            WaitForSingleObject(pis[i].hProcess, INFINITE);
            CloseHandle(pis[i].hProcess);
            CloseHandle(pis[i].hThread);
        }
    }

    QueryPerformanceCounter(&t1);
    double elapsedMs = (double)(t1.QuadPart - t0.QuadPart) * 1000.0 / freq.QuadPart;

    printf("M = %d\n", M);
    printf("Range: [%d, %d]\n", L, R);
    printf("Processes: %d\n", K);
    printf("Total found: %d\n", (int)allNumbers.size());
    printf("Numbers: ");
    for (int n : allNumbers) printf("%d ", n);
    printf("\n");
    printf("Elapsed time: %.3f ms\n", elapsedMs);

    return 0;
}