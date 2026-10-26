#include <iostream>
#include <dlfcn.h>

typedef int(*MathFunc)(int, int);

int main() {
    std::cout << "--- Задание 04: SP_04 (Linux Dynamic Load via dlopen) ---" << std::endl;

    void* handle = dlopen("./SP_XX.so", RTLD_LAZY);
    if (!handle) {
        std::cerr << "Ошибка загрузки .so: " << dlerror() << std::endl;
        return 1;
    }

    MathFunc my_sum = (MathFunc)dlsym(handle, "sum");
    MathFunc my_sub = (MathFunc)dlsym(handle, "sub");
    MathFunc my_mul = (MathFunc)dlsym(handle, "mul");
    MathFunc my_div = (MathFunc)dlsym(handle, "my_div");

    const char* dlsym_error = dlerror();
    if (dlsym_error) {
        std::cerr << "Ошибка поиска функции: " << dlsym_error << std::endl;
        dlclose(handle);
        return 1;
    }

    std::cout << "sum(60, 12) = " << my_sum(60, 12) << std::endl;
    std::cout << "sub(60, 12) = " << my_sub(60, 12) << std::endl;
    std::cout << "mul(60, 12) = " << my_mul(60, 12) << std::endl;
    std::cout << "my_div(60, 12) = " << my_div(60, 12) << std::endl;

    dlclose(handle);

    return 0;
}