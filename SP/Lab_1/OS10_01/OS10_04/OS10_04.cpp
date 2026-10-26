#include <iostream>
#include "HT.h"

#pragma comment(lib, "D:\\5_sem_bstu\\SP\\Lab_1\\OS10_01\\x64\\Debug\\OS10_HTAPI.lib")

using namespace std;

void RunTest(const char* testName, bool condition) {
    cout << "[TEST] " << testName << ": " << (condition ? "PASS" : "FAIL") << "\n";
}

int main()
{
    setlocale(LC_ALL, "Russian");
    cout << "ЗАПУСК ТЕСТОВ HT API\n\n";

    HT::HTHANDLE* ht = HT::Create(3, 5, 10, 20, "test.ht");
    RunTest("Создание хранилища", ht != nullptr);

    HT::Element* el1 = new HT::Element("Key1", 5, "Val1", 5);
    RunTest("Вставка элемента", HT::Insert(ht, el1) == TRUE);
    RunTest("Чтение элемента", HT::Get(ht, new HT::Element("Key1", 5)) != nullptr);
    RunTest("Обновление элемента", HT::Update(ht, el1, "NewV", 5) == TRUE);

    HT::Element* badEl = new HT::Element("TooLongKeyThisIs", 20, "Val", 4);
    RunTest("Ошибка: ключ больше MaxKeyLength", HT::Insert(ht, badEl) == FALSE);

    RunTest("Ошибка: чтение несуществующего", HT::Get(ht, new HT::Element("NoKey", 6)) == nullptr);

    HT::Insert(ht, new HT::Element("Key2", 5, "Val2", 5));
    HT::Insert(ht, new HT::Element("Key3", 5, "Val3", 5));
    RunTest("Ошибка: переполнение хранилища", HT::Insert(ht, new HT::Element("Key4", 5, "Val4", 5)) == FALSE);

    RunTest("Закрытие хранилища", HT::Close(ht) == TRUE);

    cout << "\nТЕСТИРОВАНИЕ ЗАВЕРШЕНО\n";
    return 0;
}