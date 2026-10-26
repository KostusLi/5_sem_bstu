#include <windows.h>
#include <iostream>

int main() {
    setlocale(LC_ALL, "ru");

    HANDLE hMailslot;
    char buffer[256];
    DWORD bytesRead;

    // Создание почтового ящика
    // Формат имени: \\.\mailslot\xxxxx для локального компьютера[cite: 3]
    hMailslot = CreateMailslotA(
        "\\\\.\\mailslot\\test_mailslot",
        0,                                 // максимальная длина сообщения (0 - любая)
        MAILSLOT_WAIT_FOREVER,             // бесконечное ожидание сообщения[cite: 3]
        NULL                               // атрибуты безопасности по умолчанию[cite: 3]
    );

    if (hMailslot == INVALID_HANDLE_VALUE) {
        std::cout << "Ошибка создания Mailslot. Код ошибки: " << GetLastError() << std::endl;
        return 1;
    }

    std::cout << "Сервер Mailslot запущен. Ожидание данных..." << std::endl;

    // Считывание данных из почтового ящика[cite: 2]
    if (ReadFile(hMailslot, buffer, sizeof(buffer) - 1, &bytesRead, NULL)) {
        buffer[bytesRead] = '\0'; // Устанавливаем конец строки
        std::cout << "Успешно получено сообщение: " << buffer << std::endl;
    }
    else {
        std::cout << "Ошибка чтения функцией ReadFile. Код ошибки: " << GetLastError() << std::endl;
    }

    // Уничтожение почтового ящика[cite: 2]
    CloseHandle(hMailslot);

    std::cout << "Работа сервера завершена." << std::endl;
    system("pause");
    return 0;
}