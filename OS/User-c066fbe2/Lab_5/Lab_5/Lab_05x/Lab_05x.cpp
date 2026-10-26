#include "Lab05Common.h"
#include <cstdlib>

int main(int argc, char* argv[])
{
    long long total = 1000000;
    if (argc > 1)
        total = _atoi64(argv[1]);

    RunLoop(total);

    printf("Press Enter...");
    getchar();
    return 0;
}