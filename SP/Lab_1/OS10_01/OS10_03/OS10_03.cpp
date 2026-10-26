#include <iostream>
#include "HT.h"

#pragma comment(lib, "D:\\5_sem_bstu\\SP\\Lab_1\\OS10_01\\x64\\Debug\\OS10_HTAPI.lib")

using namespace std;

int main()
{
    setlocale(LC_ALL, "Russian");
    HT::HTHANDLE* ht1 = nullptr;
    HT::HTHANDLE* ht2 = nullptr;
    try
    {
        cout << "--- Открываем два хранилища ---\n";
        ht1 = HT::Create(100, 2, 10, 50, "storage1.ht");
        ht2 = HT::Create(100, 2, 10, 50, "storage2.ht");

        if (ht1 && ht2) cout << "Оба хранилища созданы!\n";

        HT::Element* el1 = new HT::Element("HT1_KEY", 8, "Data1", 6);
        HT::Element* el2 = new HT::Element("HT2_KEY", 8, "Data2", 6);

        HT::Insert(ht1, el1);
        HT::Insert(ht2, el2);
        cout << "Данные вставлены в оба хранилища.\n";

        cout << "Читаем из первого: ";
        HT::print(HT::Get(ht1, new HT::Element("HT1_KEY", 8)));

        cout << "Читаем из второго: ";
        HT::print(HT::Get(ht2, new HT::Element("HT2_KEY", 8)));

        HT::Close(ht1);
        HT::Close(ht2);
        cout << "Оба хранилища закрыты.\n";
    }
    catch (const char* msg)
    {
        cout << "Ошибка: " << msg << "\n";
    }
    return 0;
}