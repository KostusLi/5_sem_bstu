#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#include "Winsock2.h"
#include <iostream>
#include <string>
#include <cstdlib>
#pragma comment(lib, "WS2_32.lib")
using namespace std;
#include "WsaErrors.h"

// ClientTy <ip> <count>
int main(int argc, char* argv[])
{
    const char* ip = argc > 1 ? argv[1] : "127.0.0.1";
    int count = argc > 2 ? atoi(argv[2]) : 100;
    try
    {
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 0), &wsaData) != 0)
            throw SetErrorMsgText("Startup:", WSAGetLastError());
        for (int i = 1; i <= count; i++)
        {
            SOCKET sC;
            if ((sC = socket(AF_INET, SOCK_STREAM, NULL)) == INVALID_SOCKET)
                throw SetErrorMsgText("socket:", WSAGetLastError());
            SOCKADDR_IN serv;
            serv.sin_family = AF_INET;
            serv.sin_port = htons(2000);
            serv.sin_addr.s_addr = inet_addr(ip);
            if (connect(sC, (sockaddr*)&serv, sizeof(serv)) == SOCKET_ERROR)
                throw SetErrorMsgText("connect:", WSAGetLastError());
            send(sC, "echo", 5, 0);
            Sleep(20);
            closesocket(sC);
            cout << "cycle " << i << endl;
        }
        WSACleanup();
    }
    catch (string errorMsgText)
    {
        cout << endl << errorMsgText << endl;
    }
    return 0;
}