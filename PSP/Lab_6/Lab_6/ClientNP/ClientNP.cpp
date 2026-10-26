#include <iostream>
#include <string>
#include <ctime>
#include <windows.h>
using namespace std;

string SetPipeError(string msgText, int code)
{
    char* buf = nullptr;
    FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, code, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&buf, 0, NULL);
    string result = msgText + " (" + to_string(code) + ") " + (buf ? buf : "");
    if (buf) LocalFree(buf);
    return result;
}

int main()
{
    HANDLE hPipe;
    char rbuf[512];
    DWORD nr, nw;
    try
    {
         if ((hPipe = CreateFileA("\\\\10.99.18.186\\pipe\\Tube", GENERIC_READ | GENERIC_WRITE,
             FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, NULL, NULL)) == INVALID_HANDLE_VALUE)
             throw SetPipeError("createfile:", GetLastError());

        // Task 6
        /*if ((hPipe = CreateFileA("\\\\SERVERPC\\pipe\\Tube", GENERIC_READ | GENERIC_WRITE,
            FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, NULL, NULL)) == INVALID_HANDLE_VALUE)
            throw SetPipeError("createfile:", GetLastError());*/

        // Task 5
        DWORD mode = PIPE_READMODE_MESSAGE;
        if (!SetNamedPipeHandleState(hPipe, &mode, NULL, NULL))
            throw SetPipeError("setstate:", GetLastError());

        // Task 4
        // const char wbuf[] = "Hello from ClientNP";
        // if (!WriteFile(hPipe, wbuf, sizeof(wbuf), &nw, NULL))
        //     throw SetPipeError("writefile:", GetLastError());
        // if (!ReadFile(hPipe, rbuf, sizeof(rbuf) - 1, &nr, NULL))
        //     throw SetPipeError("readfile:", GetLastError());
        // rbuf[nr] = '\0';
        // cout << rbuf << endl;

        int count;
        cout << "Messages count: ";
        cin >> count;
        string msg = "Fuck you, nigga 1";
        clock_t t1 = clock();
        for (int i = 0; i < count; i++)
        {
            if (!WriteFile(hPipe, msg.c_str(), (DWORD)msg.length(), &nw, NULL))
                throw SetPipeError("writefile:", GetLastError());
            if (!ReadFile(hPipe, rbuf, sizeof(rbuf) - 1, &nr, NULL))
                throw SetPipeError("readfile:", GetLastError());
            rbuf[nr] = '\0';
            cout << rbuf << endl;
            int x = atoi(rbuf + 16) + 1;
            msg = "Fuck you, nigga " + to_string(x);
        }
        clock_t t2 = clock();
        if (!WriteFile(hPipe, msg.c_str(), 0, &nw, NULL))
            throw SetPipeError("writefile:", GetLastError());
        cout << "Time: " << (double)(t2 - t1) / CLOCKS_PER_SEC << " s" << endl;

        CloseHandle(hPipe);
    }
    catch (string e)
    {
        cout << endl << e << endl;
    }
}