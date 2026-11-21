#include <iostream>
#include <cstdlib>
#include "HT.h"

#pragma comment(lib, "D:\\5_sem_bstu\\SP\\Lab_3\\OS11_HTAPI\\x64\\Debug\\OS11_HTAPI.lib")

using namespace std;

int main(int argc, char* argv[])
{
    if (argc != 6) {
        cout << "Usage: OS11_CREATE <capacity> <snapshotinterval> <maxkeylength> <maxdatalength> <filename>\n";
        return 1;
    }

    int capacity = atoi(argv[1]);
    int snapInterval = atoi(argv[2]);
    int maxKeyLen = atoi(argv[3]);
    int maxDataLen = atoi(argv[4]);
    const char* filename = argv[5];

    HT::HTHANDLE* ht = HT::Create(capacity, snapInterval, maxKeyLen, maxDataLen, filename);

    if (ht) {
        cout << "HT-Storage Created filename=" << filename
            << ", snapshotinterval=" << snapInterval
            << " capacity=" << capacity
            << ", maxkeylength=" << maxKeyLen
            << ", maxdatalength=" << maxDataLen << endl;

        HT::Close(ht);
    }
    else {
        cout << "Error creating HT-Storage\n";
    }
    return 0;
}