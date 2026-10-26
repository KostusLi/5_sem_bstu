#include "Lab05Common.h"
#include <cstdlib>

int main(int argc, char* argv[])
{
    long long total = 1000000;
    if (argc > 1)
        total = atoll(argv[1]);

    RunLoop(total);
    return 0;
}