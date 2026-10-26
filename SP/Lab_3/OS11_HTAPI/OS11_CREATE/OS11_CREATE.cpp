#include <iostream>
#include <cstdlib>
#include "HT.h"

// ВСТАВЬ СВОЙ ПУТЬ ДО .lib ФАЙЛА (обрати внимание на двойные слэши \\)
#pragma comment(lib, "D:\\5_sem_bstu\\SP\\Lab_3\\OS11_HTAPI\\x64\\Debug\\OS11_HTAPI.lib")

using namespace std;

int main(int argc, char* argv[])
{
    // Программа ждет параметры из командной строки
    if (argc != 6) {
        cout << "Usage: OS11_CREATE <capacity> <snapshotinterval> <maxkeylength> <maxdatalength> <filename>\n";
        return 1;
    }

    // Считываем параметры из аргументов
    int capacity = atoi(argv[1]);
    int snapInterval = atoi(argv[2]);
    int maxKeyLen = atoi(argv[3]);
    int maxDataLen = atoi(argv[4]);
    const char* filename = argv[5];

    // Создаем хранилище (вызов функции из нашей DLL)
    HT::HTHANDLE* ht = HT::Create(capacity, snapInterval, maxKeyLen, maxDataLen, filename);

    if (ht) {
        // Требуемый вывод по методичке
        cout << "HT-Storage Created filename=" << filename
            << ", snapshotinterval=" << snapInterval
            << " capacity=" << capacity
            << ", maxkeylength=" << maxKeyLen
            << ", maxdatalength=" << maxDataLen << endl;

        HT::Close(ht); // Создали, разметили файл и закрыли
    }
    else {
        cout << "Error creating HT-Storage\n";
    }
    return 0;
}