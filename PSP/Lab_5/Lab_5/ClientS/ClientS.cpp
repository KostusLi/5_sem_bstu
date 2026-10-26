#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <iostream>
#include <string>
#include <cstring>
#include "WsaErrors.h"
#include <windows.h>

using namespace std;

bool GetServerByName(char* name, char* call, struct sockaddr* from, int* flen)
{
    hostent* host = gethostbyname(name);
    if (host == NULL)
        throw SetErrorMsgText("gethostbyname:", WSAGetLastError());

    if (host->h_addrtype != AF_INET || host->h_addr_list[0] == NULL)
        throw string("gethostbyname: адрес IPv4 не найден");

    SOCKADDR_IN target;
    memset(&target, 0, sizeof(target));
    target.sin_family = AF_INET;
    target.sin_port = ((SOCKADDR_IN*)from)->sin_port;
    memcpy(&target.sin_addr, host->h_addr_list[0], host->h_length);

    SOCKET cC = socket(AF_INET, SOCK_DGRAM, 0);
    if (cC == INVALID_SOCKET)
        throw SetErrorMsgText("socket:", WSAGetLastError());

    DWORD timeout = 20000;
    if (setsockopt(cC, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout)) == SOCKET_ERROR)
    {
        int err = WSAGetLastError();
        closesocket(cC);
        throw SetErrorMsgText("setsockopt SO_RCVTIMEO:", err);
    }

    if (sendto(cC, call, (int)strlen(call) + 1, 0, (sockaddr*)&target, sizeof(target)) == SOCKET_ERROR)
    {
        int err = WSAGetLastError();
        closesocket(cC);
        throw SetErrorMsgText("sendto:", err);
    }

    char ibuf[50];
    *flen = sizeof(SOCKADDR_IN);
    int lb = recvfrom(cC, ibuf, sizeof(ibuf) - 1, 0, from, flen);
    if (lb == SOCKET_ERROR)
    {
        int err = WSAGetLastError();
        closesocket(cC);
        if (err == WSAETIMEDOUT)
            return false;
        throw SetErrorMsgText("recvfrom:", err);
    }

    ibuf[lb] = '\0';
    bool isMatch = (strcmp(ibuf, call) == 0);

    closesocket(cC);
    return isMatch;
}

int main()
{
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);
    setlocale(LC_ALL, "ru");
    WSADATA wsaData;
    try
    {
        if (WSAStartup(MAKEWORD(2, 0), &wsaData) != 0)
            throw SetErrorMsgText("Startup:", WSAGetLastError());

        char callsign[] = "Hello";
        short port = 2000;

        char compName[256];
        cout << "Введите имя компьютера (или IP, или localhost): ";
        cin >> compName;

        SOCKADDR_IN serv;
        memset(&serv, 0, sizeof(serv));
        serv.sin_family = AF_INET;
        serv.sin_port = htons(port);
        int lserv = sizeof(serv);

        cout << "Поиск сервера на компьютере " << compName << "..." << endl;

        if (GetServerByName(compName, callsign, (sockaddr*)&serv, &lserv))
        {
            cout << "\nСервер успешно найден и ответил на позывной!" << endl;
            cout << "IP-адрес сервера: " << inet_ntoa(serv.sin_addr) << endl;
            cout << "Порт сервера: " << ntohs(serv.sin_port) << endl;
        }
        else
        {
            cout << "\nСервер не найден или неверный отклик (таймаут)." << endl;
        }

        if (WSACleanup() == SOCKET_ERROR)
            throw SetErrorMsgText("Cleanup:", WSAGetLastError());
    }
    catch (string errorMsgText)
    {
        cout << endl << "Fatal Error: " << errorMsgText << endl;
    }
    return 0;
}