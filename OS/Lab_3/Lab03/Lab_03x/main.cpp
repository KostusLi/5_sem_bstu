#include <windows.h>
#include <cstdio>
#include <cstdlib>

int main(int argc, char* argv[]) {
    int iterations = -1;

    if (argc > 1) {
        iterations = atoi(argv[1]);
    }
    else {
        char buf[32];
        DWORD len = GetEnvironmentVariableA("ITER_NUM", buf, sizeof(buf));
        if (len > 0 && len < sizeof(buf)) {
            iterations = atoi(buf);
        }
    }

    if (iterations <= 0) {
        ExitProcess(0xC0000005);
    }

    DWORD pid = GetCurrentProcessId();
    printf("Iterations: %d\n", iterations);

    for (int i = 0; i < iterations; i++) {
        printf("PID: %lu, iteration: %d\n", pid, i + 1);
        Sleep(500);
    }

    return 0;
}