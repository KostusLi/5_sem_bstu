#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#include "Winsock2.h"
#include <iostream>
#include <string>
#include <cstdio>
#include <cstdlib>
#pragma comment(lib, "WS2_32.lib")
using namespace std;
#include "WsaErrors.h"

// ClientT <ip> <echo|time|rand|любой_другой|none> <count> <delay_ms>
int main(int argc, char* argv[])
{
    const char* ip = argc > 1 ? argv[1] : "127.0.0.1";
    const char* req = argc > 2 ? argv[2] : "echo";
    int count = argc > 3 ? atoi(argv[3]) : 5;
    int delay = argc > 4 ? atoi(argv[4]) : 0;
    SOCKET sC = INVALID_SOCKET;
    try
    {
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 0), &wsaData) != 0)
            throw SetErrorMsgText("Startup:", WSAGetLastError());
        if ((sC = socket(AF_INET, SOCK_STREAM, NULL)) == INVALID_SOCKET)
            throw SetErrorMsgText("socket:", WSAGetLastError());
        SOCKADDR_IN serv;
        serv.sin_family = AF_INET;
        serv.sin_port = htons(2000);
        serv.sin_addr.s_addr = inet_addr(ip);
        if (connect(sC, (sockaddr*)&serv, sizeof(serv)) == SOCKET_ERROR)
            throw SetErrorMsgText("connect:", WSAGetLastError());

        char buf[100];
        int n;
        if (strcmp(req, "none") != 0)
        {
            if (send(sC, req, (int)strlen(req) + 1, 0) == SOCKET_ERROR)
                throw SetErrorMsgText("send:", WSAGetLastError());
            Sleep(200);
        }
        else Sleep(15000);

        if (strcmp(req, "echo") == 0)
        {
            for (int i = 1; i <= count; i++)
            {
                sprintf(buf, "Hello from Client %d", i);
                if (send(sC, buf, (int)strlen(buf) + 1, 0) == SOCKET_ERROR)
                    throw SetErrorMsgText("send:", WSAGetLastError());
                if ((n = recv(sC, buf, sizeof(buf), 0)) == SOCKET_ERROR)
                    throw SetErrorMsgText("recv:", WSAGetLastError());
                cout << buf << endl;
                if (delay > 0) Sleep(delay);
            }
        }
        else if (strcmp(req, "time") == 0)
        {
            for (int i = 0; i < count; i++)
            {
                if (send(sC, "time", 5, 0) == SOCKET_ERROR)
                    throw SetErrorMsgText("send:", WSAGetLastError());
                if ((n = recv(sC, buf, sizeof(buf), 0)) == SOCKET_ERROR)
                    throw SetErrorMsgText("recv:", WSAGetLastError());
                cout << buf << endl;
                if (delay > 0) Sleep(delay);
            }
        }
        else if (strcmp(req, "rand") == 0)
        {
            for (int i = 0; i < count; i++)
            {
                int r = 0;
                if (send(sC, "rand", 5, 0) == SOCKET_ERROR)
                    throw SetErrorMsgText("send:", WSAGetLastError());
                if ((n = recv(sC, (char*)&r, sizeof(r), 0)) == SOCKET_ERROR)
                    throw SetErrorMsgText("recv:", WSAGetLastError());
                cout << r << endl;
                if (delay > 0) Sleep(delay);
            }
        }
        else
        {
            if ((n = recv(sC, buf, sizeof(buf), 0)) == SOCKET_ERROR)
                throw SetErrorMsgText("recv:", WSAGetLastError());
            if (n > 0) cout << "server: " << buf << endl;
            else       cout << "server closed connection" << endl;
        }
        closesocket(sC);
        WSACleanup();
    }
    catch (string errorMsgText)
    {
        cout << endl << errorMsgText << endl;
    }
    return 0;
}