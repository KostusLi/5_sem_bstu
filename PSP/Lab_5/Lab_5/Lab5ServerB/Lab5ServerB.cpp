#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <iostream>
#include <string>
#include <cstring>
#include "WsaErrors.h"
#include <windows.h>

using namespace std;

bool GetRequestFromClient(char* name, short port, struct sockaddr* from, int* flen)
{
    SOCKET s = socket(AF_INET, SOCK_DGRAM, 0);
    if (s == INVALID_SOCKET)
        throw SetErrorMsgText("socket:", WSAGetLastError());

    SOCKADDR_IN serv;
    memset(&serv, 0, sizeof(serv));
    serv.sin_family = AF_INET;
    serv.sin_port = htons(port);
    serv.sin_addr.s_addr = INADDR_ANY;

    if (bind(s, (LPSOCKADDR)&serv, sizeof(serv)) == SOCKET_ERROR)
        throw SetErrorMsgText("bind:", WSAGetLastError());

    DWORD timeout = 10000;
    if (setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout)) == SOCKET_ERROR)
        throw SetErrorMsgText("setsockopt:", WSAGetLastError());

    char ibuf[50];
    while (true)
    {
        int lb = recvfrom(s, ibuf, sizeof(ibuf) - 1, 0, from, flen);
        if (lb == SOCKET_ERROR)
        {
            int err = WSAGetLastError();
            closesocket(s);
            if (err == WSAETIMEDOUT)
                return false;
            throw SetErrorMsgText("recvfrom:", err);
        }

        ibuf[lb] = '\0';
        if (strcmp(ibuf, name) == 0)
        {
            closesocket(s);
            return true;
        }
    }
}

bool PutAnswerToClient(char* name, struct sockaddr* to, int* lto)
{
    SOCKET s = socket(AF_INET, SOCK_DGRAM, 0);
    if (s == INVALID_SOCKET)
        throw SetErrorMsgText("socket:", WSAGetLastError());

    if (sendto(s, name, (int)strlen(name) + 1, 0, to, *lto) == SOCKET_ERROR)
    {
        closesocket(s);
        throw SetErrorMsgText("sendto:", WSAGetLastError());
    }
    closesocket(s);
    return true;
}

void CheckForOtherServers(char* callsign, short port)
{
    SOCKET cC = socket(AF_INET, SOCK_DGRAM, 0);
    int optval = 1;
    setsockopt(cC, SOL_SOCKET, SO_BROADCAST, (char*)&optval, sizeof(optval));

    DWORD timeout = 2000;
    setsockopt(cC, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));

    SOCKADDR_IN bcast;
    memset(&bcast, 0, sizeof(bcast));
    bcast.sin_family = AF_INET;
    bcast.sin_port = htons(port);
    bcast.sin_addr.s_addr = INADDR_BROADCAST;

    sendto(cC, callsign, (int)strlen(callsign) + 1, 0, (sockaddr*)&bcast, sizeof(bcast));

    char ibuf[50];
    SOCKADDR_IN from;
    int lfrom = sizeof(from);
    int count = 0;

    while (true)
    {
        int lb = recvfrom(cC, ibuf, sizeof(ibuf) - 1, 0, (sockaddr*)&from, &lfrom);
        if (lb == SOCKET_ERROR && WSAGetLastError() == WSAETIMEDOUT) break;

        if (lb != SOCKET_ERROR)
        {
            ibuf[lb] = '\0';
            if (strcmp(ibuf, callsign) == 0)
            {
                count++;
                cout << "Внимание: Найден другой сервер (дубликат) с IP: " << inet_ntoa(from.sin_addr) << endl;
            }
        }
    }
    if (count > 0)
        cout << "Всего дубликатов серверов найдено: " << count << endl << endl;

    closesocket(cC);
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

        char hostName[256];
        if (gethostname(hostName, sizeof(hostName)) == SOCKET_ERROR)
            throw SetErrorMsgText("gethostname:", WSAGetLastError());
        cout << "Имя компьютера сервера: " << hostName << endl;

        CheckForOtherServers(callsign, port);

        SOCKADDR_IN clnt;
        int lc = sizeof(clnt);

        cout << "ServerB запущен..." << endl;

        while (true)
        {
            if (GetRequestFromClient(callsign, port, (sockaddr*)&clnt, &lc))
            {
                cout << "\nПолучен правильный позывной!" << endl;
                cout << "IP-адрес клиента: " << inet_ntoa(clnt.sin_addr) << endl;
                cout << "Порт клиента: " << ntohs(clnt.sin_port) << endl;

                if (PutAnswerToClient(callsign, (sockaddr*)&clnt, &lc))
                {
                    cout << "Подтверждение (позывной) отправлено клиенту." << endl;
                }

                hostent* clntHost = gethostbyaddr((char*)&clnt.sin_addr, sizeof(clnt.sin_addr), AF_INET);
                if (clntHost != NULL)
                    cout << "Имя компьютера клиента: " << clntHost->h_name << endl;
                else
                    cout << "Имя компьютера клиента не определено: " << GetErrorMsgText(WSAGetLastError()) << endl;
            }
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