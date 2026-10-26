#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int gcd(int a, int b) {
    while (b != 0) {
        int t = b;
        b = a % b;
        a = t;
    }
    return a;
}

int main(int argc, char *argv[]) {
    if (argc < 4) {
        fprintf(stderr, "Usage: lab03d_client <M> <L> <R>\n");
        return 1;
    }

    int M = atoi(argv[1]);
    int L = atoi(argv[2]);
    int R = atoi(argv[3]);

    int buf[100000];
    int count = 0;

	//sleep(25);

    for (int n = L; n <= R; n++) {
        if (gcd(n, M) == 1) {
            buf[count++] = n;
        }
    }


    write(1, &count, sizeof(count));
    if (count > 0) {
        write(1, buf, count * sizeof(int));
    }

    return 0;
}
