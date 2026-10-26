#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <iostream>
#include <string>
#include <cstring>
#include <cstdio>
#include "WsaErrors.h"

using namespace std;

int main()
{
    SOCKET cC;
    WSADATA wsaData;

    try
    {
        if (WSAStartup(MAKEWORD(2, 0), &wsaData) != 0)
            throw SetErrorMsgText("Startup:", WSAGetLastError());

        if ((cC = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET)
            throw SetErrorMsgText("socket:", WSAGetLastError());

        string ip;
        cout << "Server IP: ";
        cin >> ip;

        SOCKADDR_IN serv;
        memset(&serv, 0, sizeof(serv));
        serv.sin_family = AF_INET;
        serv.sin_port = htons(2000);
        serv.sin_addr.s_addr = inet_addr(ip.c_str());

        const int total = 10000;
        char obuf[1000];

        for (int i = 1; i <= total; i++)
        {
            memset(obuf, 'x', sizeof(obuf));
            sprintf_s(obuf, sizeof(obuf), "%d", i);
            if (sendto(cC, obuf, sizeof(obuf), 0, (sockaddr*)&serv, sizeof(serv)) == SOCKET_ERROR)
                throw SetErrorMsgText("sendto:", WSAGetLastError());
        }

        cout << "LossClient: sent " << total << " datagrams" << endl;

        if (closesocket(cC) == SOCKET_ERROR)
            throw SetErrorMsgText("closesocket:", WSAGetLastError());

        if (WSACleanup() == SOCKET_ERROR)
            throw SetErrorMsgText("Cleanup:", WSAGetLastError());
    }
    catch (string errorMsgText)
    {
        cout << endl << "WSAGetLastError: " << errorMsgText << endl;
    }
    return 0;
}