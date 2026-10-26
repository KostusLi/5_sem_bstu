#include <iostream>
#include "HT.h"

// ВСТАВЬ СВОЙ ПУТЬ ДО .lib ФАЙЛА
#pragma comment(lib, "D:\\5_sem_bstu\\SP\\Lab_3\\OS11_HTAPI\\x64\\Debug\\OS11_HTAPI.lib")

using namespace std;

int main(int argc, char* argv[])
{
    if (argc != 2) {
        cout << "Usage: OS11_START <filename>\n";
        return 1;
    }

    const char* filename = argv[1];

    // Запускаем хранилище (вызов функции Open из DLL)
    HT::HTHANDLE* ht = HT::Open(filename);

    if (ht) {
        // Требуемый вывод по методичке (параметры подтянутся сами из заголовка файла)
        cout << "HT-Storage Start filename=" << filename
            << ", snapshotinterval=" << ht->SecSnapshotInterval
            << " capacity=" << ht->Capacity
            << ", maxkeylength=" << ht->MaxKeyLength
            << ", maxdatalength=" << ht->MaxPayloadLength << endl;

        cout << "\nPress any key to exit...\n";
        cin.get(); // Ждем ввода любого символа

        HT::Close(ht); // После ввода символа закрываем хранилище
    }
    else {
        cout << "Error starting HT-Storage\n";
    }
    return 0;
}