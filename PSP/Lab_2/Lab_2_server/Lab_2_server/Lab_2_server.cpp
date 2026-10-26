#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "Winsock2.h"
#pragma comment(lib, "WS2_32.lib")
#include <iostream>
#include <ctime>
#include "Functions.h"

using namespace std;
using namespace bbb;


int RecvLine(SOCKET s, char* buf, int bufSize)
{
    int total = 0;
    while (total < bufSize - 1)
    {
        int n = recv(s, buf + total, 1, NULL);
        if (n == SOCKET_ERROR)
            throw SetErrorMsgText("recv:", WSAGetLastError());
        if (n == 0)
            return 0;

        total += n;
        if (buf[total - 1] == '\0')
            break;
    }
    return total;
}

int main()
{
    setlocale(LC_ALL, "ru");
    WSADATA wsaData;
    SOCKET sS;

    try
    {
        if (WSAStartup(MAKEWORD(2, 0), &wsaData) != 0)
            throw SetErrorMsgText("Startup:", WSAGetLastError());

        if ((sS = socket(AF_INET, SOCK_STREAM, NULL)) == INVALID_SOCKET)
            throw SetErrorMsgText("socket:", WSAGetLastError());

        SOCKADDR_IN serv;
        serv.sin_family = AF_INET;
        serv.sin_port = htons(2000);
        serv.sin_addr.s_addr = INADDR_ANY;

        if (bind(sS, (LPSOCKADDR)&serv, sizeof(serv)) == SOCKET_ERROR)
            throw SetErrorMsgText("bind:", WSAGetLastError());

        if (listen(sS, SOMAXCONN) == SOCKET_ERROR)
            throw SetErrorMsgText("listen:", WSAGetLastError());

        cout << "Сервер слушает порт 2000..." << endl;

        // 15
        while (true)
        {

            SOCKET cS;
            SOCKADDR_IN clnt;
            memset(&clnt, 0, sizeof(clnt));
            int lclnt = sizeof(clnt);

            cout << "Ожидание подключения клиента..." << endl;

            if ((cS = accept(sS, (sockaddr*)&clnt, &lclnt)) == INVALID_SOCKET)
                throw SetErrorMsgText("accept:", WSAGetLastError());

            // 10
            cout << "Клиент подключился!" << endl;
            cout << "IP-адрес клиента: " << inet_ntoa(clnt.sin_addr) << endl;
            cout << "Порт клиента: " << ntohs(clnt.sin_port) << endl;



            // 11
           /* char ibuf[50];
            int libuf = 0;

            if ((libuf = recv(cS, ibuf, sizeof(ibuf), NULL)) == SOCKET_ERROR)
                throw SetErrorMsgText("recv:", WSAGetLastError());

            cout << "Принято байт: " << libuf << endl;
            cout << "Сообщение от клиента: " << ibuf << endl;*/

            // 13
             
            char ibuf[50];
            int libuf = 0;

            for (int i = 0; i < 1000; i++)
            {
                if ((libuf = recv(cS, ibuf, sizeof(ibuf), NULL)) == SOCKET_ERROR)
                    throw SetErrorMsgText("recv:", WSAGetLastError());

                cout << ibuf << endl;
            }

            cout << "Принято 1000 сообщений" << endl;
            


            // 14
            /*
            char buf[50];
            int lbuf = 0;

            while (true)
            {
                if ((lbuf = RecvLine(cS, buf, sizeof(buf))) == SOCKET_ERROR)
                    throw SetErrorMsgText("recv:", WSAGetLastError());

                if (lbuf == 0)
                {
                    cout << "Получено сообщение нулевой длины. Завершение." << endl;
                    break;
                }

                cout << "Сервер принял: " << buf << endl;

                if (send(cS, buf, lbuf, NULL) == SOCKET_ERROR)
                    throw SetErrorMsgText("send:", WSAGetLastError());
            }
            */


            if (closesocket(cS) == SOCKET_ERROR)
                throw SetErrorMsgText("closesocket(cS):", WSAGetLastError());

            cout << "Клиент отключился." << endl;

        }

        if (closesocket(sS) == SOCKET_ERROR)
            throw SetErrorMsgText("closesocket(sS):", WSAGetLastError());

        if (WSACleanup() == SOCKET_ERROR)
            throw SetErrorMsgText("Cleanup:", WSAGetLastError());
    }
    catch (string errorMsgText)
    {
        cout << endl << "Ошибка: " << errorMsgText << endl;
    }

    system("pause");
    return 0;
}