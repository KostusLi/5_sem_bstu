#include <windows.h>
#include <iostream>
#include <string>

int main() {
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);

    HANDLE hFile;
    DWORD bytesWritten;
    std::string computerName;
    std::string message;

    std::cout << "Введите имя компьютера-сервера (или точку '.' для тестирования на одном ПК): ";
    std::getline(std::cin, computerName);

    std::string mailslotPath = "\\\\" + computerName + "\\mailslot\\test_mailslot";

    std::cout << "Попытка подключения к: " << mailslotPath << std::endl;

    hFile = CreateFileA(
        mailslotPath.c_str(),
        GENERIC_WRITE,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if (hFile == INVALID_HANDLE_VALUE) {
        std::cout << "Ошибка подключения к Mailslot. Код ошибки: " << GetLastError() << std::endl;
        system("pause");
        return 1;
    }

    std::cout << "Связь установлена! Введите сообщение: ";
    std::getline(std::cin, message);

    if (WriteFile(hFile, message.c_str(), message.length(), &bytesWritten, NULL)) {
        std::cout << "Сообщение отправлено успешно. Записано байт: " << bytesWritten << std::endl;
    }
    else {
        std::cout << "Ошибка при отправке функцией WriteFile. Код ошибки: " << GetLastError() << std::endl;
    }

    CloseHandle(hFile);

    system("pause");
    return 0;
}