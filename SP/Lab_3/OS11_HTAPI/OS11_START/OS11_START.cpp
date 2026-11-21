#include <iostream>
#include "HT.h"

#pragma comment(lib, "D:\\5_sem_bstu\\SP\\Lab_3\\OS11_HTAPI\\x64\\Debug\\OS11_HTAPI.lib")

using namespace std;

int main(int argc, char* argv[])
{
    if (argc != 2) {
        cout << "Usage: OS11_START <filename>\n";
        return 1;
    }

    const char* filename = argv[1];

    HT::HTHANDLE* ht = HT::Open(filename);

    if (ht) {
        cout << "HT-Storage Start filename=" << filename
            << ", snapshotinterval=" << ht->SecSnapshotInterval
            << " capacity=" << ht->Capacity
            << ", maxkeylength=" << ht->MaxKeyLength
            << ", maxdatalength=" << ht->MaxPayloadLength << endl;

        cout << "\nPress any key to exit...\n";
        cin.get();

        HT::Close(ht);
    }
    else {
        cout << "Error starting HT-Storage\n";
    }
    return 0;
}