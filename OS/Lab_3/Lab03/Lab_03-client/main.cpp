#include <windows.h>
#include <cstdio>
#include <cstdlib>
#include <vector>

int gcd(int a, int b) {
    while (b != 0) {
        int t = b;
        b = a % b;
        a = t;
    }
    return a;
}

int main(int argc, char* argv[]) {
    if (argc < 4) {
        fprintf(stderr, "Usage: Lab-03d-client.exe <M> <L> <R>\n");
        return 1;
    }

    int M = atoi(argv[1]);
    int L = atoi(argv[2]);
    int R = atoi(argv[3]);

    HANDLE hWrite = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hWrite == NULL || hWrite == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "No valid write handle\n");
        return 1;
    }

    Sleep(10000);

    std::vector<int> found;
    for (int n = L; n <= R; n++) {
        if (gcd(n, M) == 1) {
            found.push_back(n);
        }
    }

    DWORD written;
    int count = (int)found.size();
    WriteFile(hWrite, &count, sizeof(count), &written, NULL);
    if (count > 0) {
        WriteFile(hWrite, found.data(), count * sizeof(int), &written, NULL);
    }

    CloseHandle(hWrite);
    return 0;
}