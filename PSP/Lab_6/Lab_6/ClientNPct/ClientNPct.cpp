#include <iostream>
#include <string>
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
    setlocale(LC_ALL, "ru");
    char rbuf[800];
    DWORD nr;
    try
    {
        char wbuf[] = "Vuidi";
        if (!CallNamedPipeA("\\\\10.99.18.186\\pipe\\Tube", (LPVOID)wbuf, sizeof(wbuf) - 1, rbuf, sizeof(rbuf) - 1, &nr, NMPWAIT_WAIT_FOREVER))
            throw SetPipeError("callnamedpipe:", GetLastError());
        rbuf[nr] = '\0';
        cout << rbuf << endl;
    }
    catch (string e)
    {
        cout << endl << e << endl;
    }
}