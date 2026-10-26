#include <iostream>
#include "SP_XX.h"

#pragma comment(lib, "SP_XX.lib") 

int main() {
    std::cout << "--- Задание 01: SP_01 (Static Load) ---" << std::endl;
    std::cout << "sum(10, 5) = " << sum(10, 5) << std::endl;
    std::cout << "sub(10, 5) = " << sub(10, 5) << std::endl;
    std::cout << "mul(10, 5) = " << mul(10, 5) << std::endl;
    std::cout << "div(10, 5) = " << my_div(10, 5) << std::endl;

    std::cin.get();
    return 0;
}