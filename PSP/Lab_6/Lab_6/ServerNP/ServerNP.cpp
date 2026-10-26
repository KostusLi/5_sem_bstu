#include <iostream>
#include <string>
#include <windows.h>
#include <sddl.h>
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

    SECURITY_ATTRIBUTES sa{ sizeof(sa), NULL, FALSE };
    ConvertStringSecurityDescriptorToSecurityDescriptorA(
        "D:(A;;GA;;;WD)",
        SDDL_REVISION_1, &sa.lpSecurityDescriptor, NULL);

    HANDLE hPipe;
    char rbuf[512];
    DWORD nr, nw;
    try
    {
        // Task 1
        if ((hPipe = CreateNamedPipeA("\\\\.\\pipe\\Tube", PIPE_ACCESS_DUPLEX,
            PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT, 1, 0, 0, INFINITE, &sa)) == INVALID_HANDLE_VALUE)
            throw SetPipeError("create:", GetLastError());

        // Task 1
        // if (!ConnectNamedPipe(hPipe, NULL))
        //     throw SetPipeError("connect:", GetLastError());

        // Task 4
        // if (!ReadFile(hPipe, rbuf, sizeof(rbuf), &nr, NULL))
        //     throw SetPipeError("readfile:", GetLastError());
        // rbuf[nr] = '\0';
        // cout << rbuf << endl;
        // if (!WriteFile(hPipe, rbuf, nr, &nw, NULL))
        //     throw SetPipeError("writefile:", GetLastError());

        // Task 1
        // if (!DisconnectNamedPipe(hPipe))
        //     throw SetPipeError("disconnect:", GetLastError());

        // Task 5
        while (true)
        {
            cout << "Waiting for client..." << endl;
            if (!ConnectNamedPipe(hPipe, NULL))
            {
                DWORD e = GetLastError();
                if (e != ERROR_PIPE_CONNECTED)
                    throw SetPipeError("connect:", e);
            }
            cout << "Client connected" << endl;
            while (true)
            {
                if (!ReadFile(hPipe, rbuf, sizeof(rbuf) - 1, &nr, NULL))
                {
                    DWORD e = GetLastError();
                    if (e == ERROR_BROKEN_PIPE || e == ERROR_PIPE_NOT_CONNECTED) break;
                    throw SetPipeError("readfile:", e);
                }
                if (nr == 0) break;
                rbuf[nr] = '\0';
                cout << rbuf << endl;
                if (!WriteFile(hPipe, rbuf, nr, &nw, NULL))
                {
                    DWORD e = GetLastError();
                    if (e == ERROR_BROKEN_PIPE || e == ERROR_NO_DATA) break;
                    throw SetPipeError("writefile:", e);
                }
            }
            if (!DisconnectNamedPipe(hPipe))
                throw SetPipeError("disconnect:", GetLastError());
            cout << "Client disconnected" << endl;
        }

        CloseHandle(hPipe);
    }
    catch (string e)
    {
        cout << endl << e << endl;
    }
}