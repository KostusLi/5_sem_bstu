#include <iostream>
#include <Windows.h>
#include "HT.h"

// НЕ ЗАБУДЬ ПОМЕНЯТЬ ПУТЬ ДО .lib!
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

    cout << "OS11_02 (INSERT) STARTED. Press Ctrl+C to stop.\n\n";

    while (true) {
        // Генерируем ключ (от 0 до 49) и данные (всегда 0)
        int keyNum = rand() % 50;
        int payload = 0;

        HT::Element* el = new HT::Element(&keyNum, 4, &payload, 4);

        if (HT::Insert(ht, el)) {
            cout << "[INSERT] Key: " << keyNum << " | Payload: " << payload << " -> SUCCESS\n";
        }
        else {
            cout << "[INSERT] Key: " << keyNum << " -> FAILED (Already exists)\n";
        }

        delete el;
        Sleep(1000); // Спим 1 секунду по заданию
    }

    return 0;
}