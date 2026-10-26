#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#include "Winsock2.h"
#pragma comment(lib, "WS2_32.lib")
#include <iostream>
#include <ctime>
#include "Functions.h"

using namespace std;
using namespace bbb;

int main()
{
    setlocale(LC_ALL, "ru");
    WSADATA wsaData;
    SOCKET cC;

    try
    {
        if (WSAStartup(MAKEWORD(2, 0), &wsaData) != 0)
            throw SetErrorMsgText("Startup:", WSAGetLastError());

        if ((cC = socket(AF_INET, SOCK_STREAM, NULL)) == INVALID_SOCKET)
            throw SetErrorMsgText("socket:", WSAGetLastError());

        SOCKADDR_IN serv;
        serv.sin_family = AF_INET;
        serv.sin_port = htons(2000);
        serv.sin_addr.s_addr = inet_addr("127.0.0.1");

        if (connect(cC, (sockaddr*)&serv, sizeof(serv)) == SOCKET_ERROR)
            throw SetErrorMsgText("connect:", WSAGetLastError());

        cout << "Подключение к серверу успешно!" << endl;



        //11
        //char obuf[] = "Hello from Client";
        //int lobuf = 0;

        //if ((lobuf = send(cC, obuf, strlen(obuf) + 1, NULL)) == SOCKET_ERROR)
        //    throw SetErrorMsgText("send:", WSAGetLastError());

        //cout << "Отправлено байт: " << lobuf << endl;

        // 13
        
        int lobuf = 0;

        for (int i = 0; i < 1000; i++)
        {
            char obuf[50];
            sprintf(obuf, "Hello from Client %d", i);

            if ((lobuf = send(cC, obuf, strlen(obuf) + 1, NULL)) == SOCKET_ERROR)
                throw SetErrorMsgText("send:", WSAGetLastError());
        }

        cout << "Отправлено 1000 сообщений" << endl;
        


        // 14
        /*
        int msgCount;
        cout << "Введите количество сообщений: ";
        cin >> msgCount;

        char obuf[50] = "Hello from Client 0";
        int lobuf = strlen(obuf) + 1;

        clock_t start = clock();

        for (int i = 0; i < msgCount; i++)
        {
            if (send(cC, obuf, lobuf, NULL) == SOCKET_ERROR)
                throw SetErrorMsgText("send:", WSAGetLastError());

            char ibuf[50];
            int libuf = 0;

            if ((libuf = RecvLine(cC, ibuf, sizeof(ibuf))) == SOCKET_ERROR)
                throw SetErrorMsgText("recv:", WSAGetLastError());

            int counter = 0;
            sscanf(ibuf, "Hello from Client %d", &counter);
            counter++;

            sprintf(obuf, "Hello from Client %d", counter);
            lobuf = strlen(obuf) + 1;
        }

        clock_t end = clock();
        double elapsedSec = double(end - start) / CLOCKS_PER_SEC;
        cout << "Обмен " << msgCount << " сообщениями занял: " << elapsedSec << " сек." << endl;

        // сигнал серверу об окончании - сообщение нулевой длины
        if (send(cC, "", 0, NULL) == SOCKET_ERROR)
            throw SetErrorMsgText("send (завершение):", WSAGetLastError());
        */


        if (closesocket(cC) == SOCKET_ERROR)
            throw SetErrorMsgText("closesocket:", WSAGetLastError());

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