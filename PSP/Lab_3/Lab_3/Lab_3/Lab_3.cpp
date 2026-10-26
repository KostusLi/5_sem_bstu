#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <iostream>
#include <string>
#include <cstring>
#include "WsaErrors.h"

using namespace std;

int main()
{
    SOCKET sS;
    WSADATA wsaData;

    try
    {
        if (WSAStartup(MAKEWORD(2, 0), &wsaData) != 0)
            throw SetErrorMsgText("Startup:", WSAGetLastError());

        if ((sS = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET)
            throw SetErrorMsgText("socket:", WSAGetLastError());

        SOCKADDR_IN serv;
        memset(&serv, 0, sizeof(serv));
        serv.sin_family = AF_INET;
        serv.sin_port = htons(2000);
        serv.sin_addr.s_addr = INADDR_ANY;

        if (bind(sS, (LPSOCKADDR)&serv, sizeof(serv)) == SOCKET_ERROR)
            throw SetErrorMsgText("bind:", WSAGetLastError());

        cout << "ServerU: socket created and bound to UDP port 2000" << endl;

        SOCKADDR_IN clnt;
        memset(&clnt, 0, sizeof(clnt));
        int lc = sizeof(clnt);
        char ibuf[50];
        int lb = 0;

        // Задание 8
        int count = 0;

        // Задание 2
        // if ((lb = recvfrom(sS, ibuf, sizeof(ibuf), 0, (sockaddr*)&clnt, &lc)) == SOCKET_ERROR)
        //     throw SetErrorMsgText("recvfrom:", WSAGetLastError());

        // Задание 4
        // cout << "ServerU: received " << lb << " bytes: " << ibuf << endl;

        // Задание 5
        // if (sendto(sS, ibuf, lb, 0, (sockaddr*)&clnt, sizeof(clnt)) == SOCKET_ERROR)
        //     throw SetErrorMsgText("sendto:", WSAGetLastError());

        // Задание 7
        while (true)
        {
            if ((lb = recvfrom(sS, ibuf, sizeof(ibuf), 0, (sockaddr*)&clnt, &lc)) == SOCKET_ERROR)
                throw SetErrorMsgText("recvfrom:", WSAGetLastError());

            if (lb == 0)
                break;

            if (sendto(sS, ibuf, lb, 0, (sockaddr*)&clnt, sizeof(clnt)) == SOCKET_ERROR)
                throw SetErrorMsgText("sendto:", WSAGetLastError());

            // Задание 8
            count++;
        }

        cout << "ServerU: messages echoed: " << count << endl;

        if (closesocket(sS) == SOCKET_ERROR)
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