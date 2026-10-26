#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <iostream>
#include <string>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <ctime>
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

        // Задание 8
        DWORD timeout = 3000;
        if (setsockopt(cC, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout)) == SOCKET_ERROR)
            throw SetErrorMsgText("setsockopt:", WSAGetLastError());

        SOCKADDR_IN serv;
        memset(&serv, 0, sizeof(serv));
        serv.sin_family = AF_INET;
        serv.sin_port = htons(2000);

        // Задание 3
        // serv.sin_addr.s_addr = inet_addr("127.0.0.1");

        // Задание 6
        string ip;
        cout << "Server IP: ";
        cin >> ip;
        serv.sin_addr.s_addr = inet_addr(ip.c_str());

        char obuf[50] = "Hello from ClientU";
        int lobuf = 0;

        // Задание 5
        SOCKADDR_IN from;
        memset(&from, 0, sizeof(from));
        int lfrom = sizeof(from);
        char ibuf[50];
        int libuf = 0;

        // Задание 3
        // if ((lobuf = sendto(cC, obuf, (int)strlen(obuf) + 1, 0, (sockaddr*)&serv, sizeof(serv))) == SOCKET_ERROR)
        //     throw SetErrorMsgText("sendto:", WSAGetLastError());

        // Задание 3
        // cout << "ClientU: sent " << lobuf << " bytes" << endl;

        // Задание 5
        // if ((libuf = recvfrom(cC, ibuf, sizeof(ibuf), 0, (sockaddr*)&from, &lfrom)) == SOCKET_ERROR)
        //     throw SetErrorMsgText("recvfrom:", WSAGetLastError());

        // Задание 5 — закомментировано в задании 7
        // cout << "ClientU: received " << libuf << " bytes: " << ibuf << endl;

        int n = 0;
        cout << "Messages count: ";
        cin >> n;

        const char prefix[] = "Hello from Client ";
        int x = 1;

        // Задание 8
        int sent = 0;
        int received = 0;

        clock_t tStart = clock();


        sprintf_s(obuf, sizeof(obuf), "%s%d", prefix, x);

        if ((lobuf = sendto(cC, obuf, (int)strlen(obuf) + 1, 0, (sockaddr*)&serv, sizeof(serv))) == SOCKET_ERROR)
            throw SetErrorMsgText("sendto:", WSAGetLastError());

        //for (int i = 0; i < n; i++)
        //{
        //    sprintf_s(obuf, sizeof(obuf), "%s%d", prefix, x);

        //    if ((lobuf = sendto(cC, obuf, (int)strlen(obuf) + 1, 0, (sockaddr*)&serv, sizeof(serv))) == SOCKET_ERROR)
        //        throw SetErrorMsgText("sendto:", WSAGetLastError());

        //    // Задание 8
        //    sent++;

        //    // Задание 7
        //    // if ((libuf = recvfrom(cC, ibuf, sizeof(ibuf), 0, (sockaddr*)&from, &lfrom)) == SOCKET_ERROR)
        //    //     throw SetErrorMsgText("recvfrom:", WSAGetLastError());

        //    // Задание 8
        //    if ((libuf = recvfrom(cC, ibuf, sizeof(ibuf), 0, (sockaddr*)&from, &lfrom)) == SOCKET_ERROR)
        //    {
        //        // Задание 8 — закомментировано в доработке задания 8
        //        // if (WSAGetLastError() == WSAETIMEDOUT)
        //        //     break;

        //        // Задание 8 (доработка)
        //        if (WSAGetLastError() == WSAETIMEDOUT || WSAGetLastError() == WSAECONNRESET)
        //            break;
        //        throw SetErrorMsgText("recvfrom:", WSAGetLastError());
        //    }

        //    // Задание 8
        //    received++;

        //    x = atoi(ibuf + sizeof(prefix) - 1) + 1;
        //}

        clock_t tEnd = clock();

        if (sendto(cC, obuf, 0, 0, (sockaddr*)&serv, sizeof(serv)) == SOCKET_ERROR)
            throw SetErrorMsgText("sendto:", WSAGetLastError());

        cout << "ClientU: exchange time: " << (tEnd - tStart) * 1000.0 / CLOCKS_PER_SEC << " ms" << endl;

        // Задание 8
        cout << "ClientU: sent " << sent << ", received " << received << endl;

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