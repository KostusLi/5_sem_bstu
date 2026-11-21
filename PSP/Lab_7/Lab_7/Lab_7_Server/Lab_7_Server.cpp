#include <windows.h>
#include <iostream>

int main() {
    setlocale(LC_ALL, "ru");

    HANDLE hMailslot;
    char buffer[256];
    DWORD bytesRead;

    hMailslot = CreateMailslotA(
        "\\\\.\\mailslot\\test_mailslot",
        0,
        MAILSLOT_WAIT_FOREVER,
        NULL
    );

    if (hMailslot == INVALID_HANDLE_VALUE) {
        std::cout << "Ошибка создания Mailslot. Код ошибки: " << GetLastError() << std::endl;
        return 1;
    }

    std::cout << "Сервер Mailslot запущен. Ожидание данных..." << std::endl;

    if (ReadFile(hMailslot, buffer, sizeof(buffer) - 1, &bytesRead, NULL)) {
        buffer[bytesRead] = '\0';
        std::cout << "Успешно получено сообщение: " << buffer << std::endl;
    }
    else {
        std::cout << "Ошибка чтения функцией ReadFile. Код ошибки: " << GetLastError() << std::endl;
    }

    CloseHandle(hMailslot);

    std::cout << "Работа сервера завершена." << std::endl;
    system("pause");
    return 0;
}