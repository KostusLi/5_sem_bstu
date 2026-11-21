#include <windows.h>
#include <iostream>
#include <string>
#include <cstring>
using namespace std;

// ClientNP [имя_компьютера]   (по умолчанию "." - локальный)
int main(int argc, char* argv[])
{
    string host = argc > 1 ? argv[1] : ".";
    string pn = "\\\\" + host + "\\pipe\\Tube";
    HANDLE hPipe = CreateFileA(pn.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
    if (hPipe == INVALID_HANDLE_VALUE)
    {
        cout << "CreateFile error " << GetLastError() << endl;
        return 1;
    }
    char cmd[100], ans[128];
    DWORD wb, rb;
    while (true)
    {
        cout << "> ";
        cin >> cmd;
        if (!WriteFile(hPipe, cmd, (DWORD)strlen(cmd) + 1, &wb, NULL)) break;
        if (!ReadFile(hPipe, ans, sizeof(ans), &rb, NULL)) break;
        cout << ans << endl;
        if (strcmp(cmd, "exit") == 0 || strcmp(cmd, "shutdown") == 0) break;
    }
    CloseHandle(hPipe);
    return 0;
}