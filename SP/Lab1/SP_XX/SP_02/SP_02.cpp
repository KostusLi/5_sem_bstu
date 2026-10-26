#include <iostream>
#include <windows.h>

typedef int(*MathFunc)(int, int);

int main() {
    std::setlocale(LC_ALL, "ru");

    std::cout << "--- Задание 02: SP_02 (Dynamic Load via LoadLibrary) ---" << std::endl;


    HMODULE hLib = LoadLibraryA("SP_XX.dll");

    if (hLib == NULL) {
        std::cout << "Ошибка! Не удалось загрузить SP_XX.dll" << std::endl;
        return 1;
    }

    MathFunc my_sum = (MathFunc)GetProcAddress(hLib, "sum");
    MathFunc my_sub = (MathFunc)GetProcAddress(hLib, "sub");
    MathFunc my_mul = (MathFunc)GetProcAddress(hLib, "mul");
    MathFunc my_div = (MathFunc)GetProcAddress(hLib, "my_div");

    if (my_sum && my_sub && my_mul && my_div) {
        std::cout << "sum(20, 4) = " << my_sum(20, 4) << std::endl;
        std::cout << "sub(20, 4) = " << my_sub(20, 4) << std::endl;
        std::cout << "mul(20, 4) = " << my_mul(20, 4) << std::endl;
        std::cout << "my_div(20, 4) = " << my_div(20, 4) << std::endl;
    }
    else {
        std::cout << "Ошибка! Не удалось найти одну из функций в DLL." << std::endl;
    }

    FreeLibrary(hLib);

    std::cin.get();
    return 0;
}