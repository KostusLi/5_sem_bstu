#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <iostream>
#include <string>
#include <cstring>
#include "WsaErrors.h"
#include <windows.h>

using namespace std;

bool GetServer(char* call, short port, struct sockaddr* from, int* flen)
{
    SOCKET cC = socket(AF_INET, SOCK_DGRAM, 0);
    if (cC == INVALID_SOCKET)
        throw SetErrorMsgText("socket:", WSAGetLastError());

    int optval = 1;
    if (setsockopt(cC, SOL_SOCKET, SO_BROADCAST, (char*)&optval, sizeof(optval)) == SOCKET_ERROR)
        throw SetErrorMsgText("setsockopt SO_BROADCAST:", WSAGetLastError());

    DWORD timeout = 5000;
    if (setsockopt(cC, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout)) == SOCKET_ERROR)
        throw SetErrorMsgText("setsockopt SO_RCVTIMEO:", WSAGetLastError());

    SOCKADDR_IN bcast;
    memset(&bcast, 0, sizeof(bcast));
    bcast.sin_family = AF_INET;
    bcast.sin_port = htons(port);
    bcast.sin_addr.s_addr = inet_addr("172.20.10.15");

    if (sendto(cC, call, (int)strlen(call) + 1, 0, (sockaddr*)&bcast, sizeof(bcast)) == SOCKET_ERROR)
        throw SetErrorMsgText("sendto:", WSAGetLastError());

    char ibuf[50];
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

        SOCKADDR_IN serv;
        int lserv = sizeof(serv);

        cout << "Поиск сервера в локальной сети..." << endl;

        if (GetServer(callsign, port, (sockaddr*)&serv, &lserv))
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