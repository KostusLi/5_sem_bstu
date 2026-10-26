#include <iostream>
#include <Windows.h>
#include "HT.h"

#pragma comment(lib, "D:\\5_sem_bstu\\SP\\Lab_3\\OS11_HTAPI\\x64\\Debug\\OS11_HTAPI.lib")

using namespace std;

int main(int argc, char* argv[])
{
    const char* filename = (argc > 1) ? argv[1] : "D:\\5_sem_bstu\\SP\\Lab_3\\OS11_HTAPI\\OS11_CREATE\\file.ht";

    HT::HTHANDLE* ht = HT::Open(filename);
    if (!ht) {
        cout << "Error: Cannot open HT-Storage. Is OS11_START running?\n";
        return 1;
    }

    cout << "OS11_03 (DELETE) STARTED. Press Ctrl+C to stop.\n\n";

    while (true) {
        int keyNum = rand() % 50;

        // Для удаления данные не нужны, только ключ
        HT::Element* el = new HT::Element(&keyNum, 4);

        if (HT::Delete(ht, el)) {
            cout << "[DELETE] Key: " << keyNum << " -> SUCCESS\n";
        }
        else {
            cout << "[DELETE] Key: " << keyNum << " -> FAILED (Not found)\n";
        }

        delete el;
        Sleep(1000);
    }

    return 0;
}