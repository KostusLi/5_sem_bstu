#include "Functions.h";

#pragma comment(lib, "WS2_32.lib")

using namespace std;

namespace bbb
{

    string GetErrorMsgText(int code)
    {
        string msgText;
        switch (code)
        {
        case WSAEINTR:msgText = "WSAEINTR (работа функции прервана)"; break;
        case WSAEACCES:              msgText = "WSAEACCES (разрешение отвергнуто)"; break;
        case WSAEFAULT:              msgText = "WSAEFAULT (ошибочный адрес)"; break;
        case WSAEINVAL:              msgText = "WSAEINVAL (ошибка в аргументе)"; break;
        case WSAEMFILE:              msgText = "WSAEMFILE (слишком много файлов открыто)"; break;
        case WSAEWOULDBLOCK:         msgText = "WSAEWOULDBLOCK (ресурс временно недоступен)"; break;
        case WSAEINPROGRESS:         msgText = "WSAEINPROGRESS (операция в процессе развития)"; break;
        case WSAEALREADY:            msgText = "WSAEALREADY (операция уже выполняется)"; break;
        case WSAENOTSOCK:            msgText = "WSAENOTSOCK (сокет задан неправильно)"; break;
        case WSAEDESTADDRREQ:        msgText = "WSAEDESTADDRREQ (требуется адрес расположения)"; break;
        case WSAEMSGSIZE:            msgText = "WSAEMSGSIZE (сообщение слишком длинное)"; break;
        case WSAEPROTOTYPE:          msgText = "WSAEPROTOTYPE (неправильный тип протокола для сокета)"; break;
        case WSAENOPROTOOPT:         msgText = "WSAENOPROTOOPT (ошибка в опции протокола)"; break;
        case WSAEPROTONOSUPPORT:     msgText = "WSAEPROTONOSUPPORT (протокол не поддерживается)"; break;
        case WSAESOCKTNOSUPPORT:     msgText = "WSAESOCKTNOSUPPORT (тип сокета не поддерживается)"; break;
        case WSAEOPNOTSUPP:          msgText = "WSAEOPNOTSUPP (операция не поддерживается)"; break;
        case WSAEPFNOSUPPORT:        msgText = "WSAEPFNOSUPPORT (тип протоколов не поддерживается)"; break;
        case WSAEAFNOSUPPORT:        msgText = "WSAEAFNOSUPPORT (тип адресов не поддерживается протоколом)"; break;
        case WSAEADDRINUSE:          msgText = "WSAEADDRINUSE (адрес уже используется)"; break;
        case WSAEADDRNOTAVAIL:       msgText = "WSAEADDRNOTAVAIL (запрошенный адрес не может быть использован)"; break;
        case WSAENETDOWN:            msgText = "WSAENETDOWN (сеть отключена)"; break;
        case WSAENETUNREACH:         msgText = "WSAENETUNREACH (сеть не достижима)"; break;
        case WSAENETRESET:           msgText = "WSAENETRESET (сеть разорвала соединение)"; break;
        case WSAECONNABORTED:        msgText = "WSAECONNABORTED (программный отказ связи)"; break;
        case WSAECONNRESET:          msgText = "WSAECONNRESET (связь восстановлена)"; break;
        case WSAENOBUFS:             msgText = "WSAENOBUFS (не хватает памяти для буферов)"; break;
        case WSAEISCONN:             msgText = "WSAEISCONN (сокет уже подключен)"; break;
        case WSAENOTCONN:            msgText = "WSAENOTCONN (сокет не подключен)"; break;
        case WSAESHUTDOWN:           msgText = "WSAESHUTDOWN (нельзя выполнить send: сокет завершил работу)"; break;
        case WSAETIMEDOUT:           msgText = "WSAETIMEDOUT (закончился отведенный интервал времени)"; break;
        case WSAECONNREFUSED:        msgText = "WSAECONNREFUSED (соединение отклонено)"; break;
        case WSAEHOSTDOWN:           msgText = "WSAEHOSTDOWN (хост в неработоспособном состоянии)"; break;
        case WSAEHOSTUNREACH:        msgText = "WSAEHOSTUNREACH (нет маршрута для хоста)"; break;
        case WSAEPROCLIM:            msgText = "WSAEPROCLIM (слишком много процессов)"; break;
        case WSASYSNOTREADY:         msgText = "WSASYSNOTREADY (сеть не доступна)"; break;
        case WSAVERNOTSUPPORTED:     msgText = "WSAVERNOTSUPPORTED (данная версия недоступна)"; break;
        case WSANOTINITIALISED:      msgText = "WSANOTINITIALISED (не выполнена инициализация WS2_32.DLL)"; break;
        case WSAEDISCON:             msgText = "WSAEDISCON (выполняется отключение)"; break;
        case WSATYPE_NOT_FOUND:      msgText = "WSATYPE_NOT_FOUND (класс не найден)"; break;
        case WSAHOST_NOT_FOUND:      msgText = "WSAHOST_NOT_FOUND (хост не найден)"; break;
        case WSATRY_AGAIN:           msgText = "WSATRY_AGAIN (неавторизированный хост не найден)"; break;
        case WSANO_RECOVERY:         msgText = "WSANO_RECOVERY (неопределенная ошибка)"; break;
        case WSANO_DATA:             msgText = "WSANO_DATA (нет записи запрошенного типа)"; break;
        case WSA_INVALID_HANDLE:     msgText = "WSA_INVALID_HANDLE (указанный дескриптор события с ошибкой)"; break;
        case WSA_INVALID_PARAMETER:  msgText = "WSA_INVALID_PARAMETER (один или более параметров с ошибкой)"; break;
        case WSA_IO_INCOMPLETE:      msgText = "WSA_IO_INCOMPLETE (объект ввода-вывода не в сигнальном состоянии)"; break;
        case WSA_IO_PENDING:         msgText = "WSA_IO_PENDING (операция завершится позже)"; break;
        case WSA_NOT_ENOUGH_MEMORY:  msgText = "WSA_NOT_ENOUGH_MEMORY (недостаточно памяти)"; break;
        case WSA_OPERATION_ABORTED:  msgText = "WSA_OPERATION_ABORTED (операция отвергнута)"; break;
        case WSASYSCALLFAILURE:      msgText = "WSASYSCALLFAILURE (аварийное завершение системного вызова)"; break;
        default:                     msgText = "***НЕИЗВЕСТНЫЙ КОД ОШИБКИ***"; break;
        };
        return msgText;
    }

    string SetErrorMsgText(string prefix, int code)
    {
        return prefix + GetErrorMsgText(code);
    }
}
