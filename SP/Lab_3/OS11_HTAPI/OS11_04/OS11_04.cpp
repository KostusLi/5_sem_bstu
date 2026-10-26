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

    cout << "OS11_04 (UPDATE) STARTED. Press Ctrl+C to stop.\n\n";

    while (true) {
        int keyNum = rand() % 50;
        HT::Element* searchEl = new HT::Element(&keyNum, 4);

        // Сначала читаем
        HT::Element* readEl = HT::Get(ht, searchEl);

        if (readEl) {
            // Берем текущее значение и увеличиваем на 1
            int currentPayload = *(int*)readEl->payload;
            int newPayload = currentPayload + 1;

            if (HT::Update(ht, readEl, &newPayload, 4)) {
                cout << "[UPDATE] Key: " << keyNum << " | " << currentPayload << " -> " << newPayload << " SUCCESS\n";
            }

            // Очищаем память, которую выделил Get
            delete[](char*)readEl->key;
            delete[](char*)readEl->payload;
            delete readEl;
        }
        else {
            cout << "[UPDATE] Key: " << keyNum << " -> FAILED (Not found)\n";
        }

        delete searchEl;
        Sleep(1000);
    }

    return 0;
}