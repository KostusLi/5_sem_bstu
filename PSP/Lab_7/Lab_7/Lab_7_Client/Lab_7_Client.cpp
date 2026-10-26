#include <windows.h>
#include <iostream>
#include <string>

int main() 
{
    setlocale(LC_ALL, "ru");
    HANDLE hFile;
    DWORD bytesWritten;
    std::string message = "Это твоя бибикаааа? Бимбомбом";

    // Подключение клиента к почтовому ящику[cite: 5]
    hFile = CreateFileA(
        "\\\\.\\mailslot\\test_mailslot", // локальное имя почтового ящика[cite: 3, 5]
        GENERIC_WRITE,                     // режим доступа: запись[cite: 5]
        FILE_SHARE_READ,                   // режим совместного использования[cite: 5]
        NULL,                              // атрибуты безопасности по умолчанию[cite: 5]
        OPEN_EXISTING,                     // открытие существующего ящика[cite: 5]
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if (hFile == INVALID_HANDLE_VALUE) {
        std::cout << "Ошибка подключения к Mailslot (сервер запущен, мудила, или нет?). Код ошибки: " << GetLastError() << std::endl;
        system("pause");
        return 1;
    }

    std::cout << "Связь с Mailslot установлена. Отправка данных..." << std::endl;

    // Запись данных в почтовый сервер[cite: 2]
    if (WriteFile(hFile, message.c_str(), message.length(), &bytesWritten, NULL)) {
        std::cout << "Сообщение отправлено успешно. Количество записанных байт: " << bytesWritten << std::endl;
    }
    else {
        std::cout << "Ошибка при записи функцией WriteFile. Код ошибки: " << GetLastError() << std::endl;
    }

    // Закрытие дескриптора[cite: 2]
    CloseHandle(hFile);

    system("pause");
    return 0;
}