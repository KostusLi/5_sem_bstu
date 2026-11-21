#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#include "Contact.h"
#include <iostream>
#include <string>
#include <vector>
#include <ctime>
#include <cstdio>
#include <cstdlib>
#pragma comment(lib, "WS2_32.lib")
using namespace std;
#include "WsaErrors.h"

#define AS_SQUIRT    10
#define REQUEST_WAIT 10000
#define SERVICE_MAX  (-600000000LL)

// [З3][З25]
struct StatsInfo
{
    volatile LONG total;
    volatile LONG refused;
    volatile LONG ok;
    volatile LONG active;
};
StatsInfo stats = { 0, 0, 0, 0 };

// [З3][З23]
struct TalkersCmdEntry { const char* name; TalkersCommand code; };

TalkersCmdEntry TalkersCmd[] =
{
    { "start", START }, { "stop", STOP }, { "exit", EXIT },
    { "statistics", STATISTICS }, { "wait", WAIT }, { "shutdown", SHUTDOWN }
};

const int NCMD = sizeof(TalkersCmd) / sizeof(TalkersCmd[0]);

typedef HANDLE(*TS)(char*, LPVOID);
TS ts = NULL;

ListContact contacts;
CRITICAL_SECTION scListContact;

HANDLE hAcceptServer = NULL, hDispatchServer = NULL, hGarbageCleaner = NULL,
hConsolePipe = NULL, hResponseServer = NULL;
HANDLE volatile hEvent = NULL;
SOCKET sS = INVALID_SOCKET;
volatile bool g_accepting = false;

int  g_port = 2000;
int  g_uport = 2000;
char g_dll[100] = "ServiceLibrary";
char g_pipe[100] = "Tube";
char g_ucall[50] = "Hello";

string NowStr()
{
    time_t t = time(NULL);
    tm lt;
    localtime_s(&lt, &t);
    char b[30];
    strftime(b, sizeof(b), "%d.%m.%Y %H:%M:%S", &lt);
    return b;
}

// [З19]
VOID CALLBACK ASStartMessage(ULONG_PTR p)
{
    Contact* c = (Contact*)p;
    cout << "[" << NowStr() << "] start  " << c->srvname << " "
        << inet_ntoa(c->prms.sin_addr) << ":" << ntohs(c->prms.sin_port) << endl;
}

// [З19][З20][З25]
VOID CALLBACK ASFinishMessage(ULONG_PTR p)
{
    Contact* c = (Contact*)p;
    cout << "[" << NowStr() << "] finish " << c->srvname << " "
        << inet_ntoa(c->prms.sin_addr) << ":" << ntohs(c->prms.sin_port)
        << " (" << c->msg << ")" << endl;
    if (c->htimer != NULL)
    {
        CancelWaitableTimer(c->htimer);
        CloseHandle(c->htimer);
        c->htimer = NULL;
    }
    if (c->hthread != NULL)
    {
        CloseHandle(c->hthread);
        c->hthread = NULL;
    }
    InterlockedDecrement(&stats.active);

    if (c->sthread == Contact::FINISH)
        InterlockedIncrement(&stats.ok);
    else       
        InterlockedIncrement(&stats.refused);
    c->type = Contact::EMPTY;
}

// [З20]
VOID CALLBACK ASWTimer(LPVOID p, DWORD lo, DWORD hi)
{
    Contact* c = (Contact*)p;
    if (c->type == Contact::CONTACT && c->sthread == Contact::WORK)
    {
        TerminateThread(c->hthread, 1);
        closesocket(c->s);
        c->SetST(Contact::TIMEOUT, "timeout");
        QueueUserAPC(ASFinishMessage, hAcceptServer, (ULONG_PTR)c);
    }
}

int Busy()
{
    int n = 0;
    EnterCriticalSection(&scListContact);
    for (ListContact::iterator p = contacts.begin(); p != contacts.end(); p++)
        if (p->type != Contact::EMPTY) n++;
    LeaveCriticalSection(&scListContact);
    return n;
}

// [З10][З18][З21]
bool AcceptCycle(int squirt)
{
    bool rc = false;
    Contact c(Contact::ACCEPT, "");
    while (squirt-- > 0 && rc == false)
    {
        c.lprms = sizeof(c.prms);
        if ((c.s = accept(sS, (sockaddr*)&c.prms, &c.lprms)) == INVALID_SOCKET)
        {
            if (WSAGetLastError() != WSAEWOULDBLOCK)
                throw SetErrorMsgText("accept:", WSAGetLastError());
        }
        else
        {
            rc = true;
            u_long nb = 1;
            ioctlsocket(c.s, FIONBIO, &nb);
            c.tick = GetTickCount();
            c.hAcceptServer = hAcceptServer;
            c.ASStart = ASStartMessage;
            c.ASFinish = ASFinishMessage;
            InterlockedIncrement(&stats.total);
            EnterCriticalSection(&scListContact);
            contacts.push_front(c);
            LeaveCriticalSection(&scListContact);
            SetEvent(hEvent);
        }
    }
    return rc;
}

// [З9][З24]
void CommandsCycle(volatile TalkersCommand& cmd)
{
    int squirt = 0;
    bool waitMode = false, shutMode = false;
    while (cmd != EXIT)
    {
        switch (cmd)
        {
        case START:
            cmd = GETCOMMAND;
            squirt = AS_SQUIRT;
            g_accepting = true;
            waitMode = shutMode = false;
            break;
        case STOP:
            cmd = GETCOMMAND;
            squirt = 0;
            g_accepting = false;
            break;
        case WAIT:
            cmd = GETCOMMAND;
            squirt = 0;
            g_accepting = false;
            waitMode = true;
            break;
        case SHUTDOWN:
            cmd = GETCOMMAND;
            squirt = 0;
            g_accepting = false;
            shutMode = true;
            break;
        default:
            break;
        }
        if ((waitMode || shutMode) && Busy() == 0)
        {
            if (shutMode)
            {
                cmd = EXIT;
                break;
            }
            waitMode = false;
            squirt = AS_SQUIRT;
            g_accepting = true;
        }
        // [З9] Sleep(100);
        if (!AcceptCycle(squirt)) SleepEx(0, TRUE);   // [З10][З19]
    }
}

// [З6][З8]
DWORD WINAPI AcceptServer(LPVOID pPrm)
{
    DWORD rc = 0;
    volatile TalkersCommand& cmd = *(volatile TalkersCommand*)pPrm;
    // [З6] while (true) { cout << "AcceptServer" << endl; Sleep(5000); }
    try
    {
        hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);   // [З21]
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 0), &wsaData) != 0)
            throw SetErrorMsgText("Startup:", WSAGetLastError());
        if ((sS = socket(AF_INET, SOCK_STREAM, NULL)) == INVALID_SOCKET)
            throw SetErrorMsgText("socket:", WSAGetLastError());
        SOCKADDR_IN serv;
        serv.sin_family = AF_INET;
        serv.sin_port = htons((u_short)g_port);
        serv.sin_addr.s_addr = INADDR_ANY;
        if (bind(sS, (LPSOCKADDR)&serv, sizeof(serv)) == SOCKET_ERROR)
            throw SetErrorMsgText("bind:", WSAGetLastError());
        if (listen(sS, SOMAXCONN) == SOCKET_ERROR)
            throw SetErrorMsgText("listen:", WSAGetLastError());
        u_long nonblk = 1;
        if (ioctlsocket(sS, FIONBIO, &nonblk) == SOCKET_ERROR)
            throw SetErrorMsgText("ioctlsocket:", WSAGetLastError());
        CommandsCycle(cmd);
        if (closesocket(sS) == SOCKET_ERROR)
            throw SetErrorMsgText("closesocket:", WSAGetLastError());
        if (WSACleanup() == SOCKET_ERROR)
            throw SetErrorMsgText("Cleanup:", WSAGetLastError());
    }
    catch (string errorMsgText)
    {
        cout << endl << "AcceptServer: " << errorMsgText << endl;
        cmd = EXIT;
        rc = 1;
    }
    return rc;
}

// [З16][З17][З18]
DWORD WINAPI GarbageCleaner(LPVOID pPrm)
{
    volatile TalkersCommand& cmd = *(volatile TalkersCommand*)pPrm;
    // [З16] while (true) { cout << "GarbageCleaner" << endl; Sleep(5000); }
    while (cmd != EXIT)
    {
        EnterCriticalSection(&scListContact);
        for (ListContact::iterator p = contacts.begin(); p != contacts.end(); )
        {
            if (p->type == Contact::EMPTY) p = contacts.erase(p);
            else                           p++;
        }
        LeaveCriticalSection(&scListContact);
        Sleep(200);
    }
    return 0;
}

// [З15]
void RejectContact(Contact* c, const char* text)
{
    send(c->s, text, (int)strlen(text) + 1, 0);
    closesocket(c->s);
    InterlockedIncrement(&stats.refused);
    c->type = Contact::EMPTY;
}

// [З12][З15][З20]
void DispatchOne(Contact* c)
{
    char buf[50];
    int n = recv(c->s, buf, sizeof(buf) - 1, 0);
    if (n == SOCKET_ERROR)
    {
        if (WSAGetLastError() == WSAEWOULDBLOCK)
        {
            if (GetTickCount() - c->tick > REQUEST_WAIT)
            {
                cout << "DispatchServer: request timeout " << inet_ntoa(c->prms.sin_addr) << endl;
                RejectContact(c, "TimeoutInquiry");
            }
            return;
        }
        closesocket(c->s);
        InterlockedIncrement(&stats.refused);
        c->type = Contact::EMPTY;
        return;
    }
    if (n == 0)
    {
        closesocket(c->s);
        InterlockedIncrement(&stats.refused);
        c->type = Contact::EMPTY;
        return;
    }
    buf[n] = 0;
    cout << "DispatchServer: request '" << buf << "' from " << inet_ntoa(c->prms.sin_addr) << endl;

    HANDLE h = ts(buf, c);
    if (h == NULL)
    {
        RejectContact(c, "ErrorInquiry");
        return;
    }
    u_long nb = 0;
    ioctlsocket(c->s, FIONBIO, &nb);
    c->type = Contact::CONTACT;
    c->sthread = Contact::WORK;
    c->hthread = h;
    LARGE_INTEGER due;
    due.QuadPart = SERVICE_MAX;
    c->htimer = CreateWaitableTimer(NULL, FALSE, NULL);
    SetWaitableTimer(c->htimer, &due, 0, ASWTimer, c, FALSE);
    InterlockedIncrement(&stats.active);
    ResumeThread(h);
}

// [З11][З12][З21]
DWORD WINAPI DispatchServer(LPVOID pPrm)
{
    volatile TalkersCommand& cmd = *(volatile TalkersCommand*)pPrm;
    while (hEvent == NULL && cmd != EXIT) Sleep(10);
    // [З11] while (true) { cout << "DispatchServer" << endl; Sleep(5000); }
    while (cmd != EXIT)
    {
        WaitForSingleObjectEx(hEvent, 50, TRUE);
        vector<Contact*> fresh;
        EnterCriticalSection(&scListContact);
        for (ListContact::iterator p = contacts.begin(); p != contacts.end(); p++)
            if (p->type == Contact::ACCEPT) fresh.push_back(&(*p));
        LeaveCriticalSection(&scListContact);
        for (size_t i = 0; i < fresh.size(); i++) DispatchOne(fresh[i]);
    }
    return 0;
}

// [З27]
bool GetRequestFromClient(SOCKET s, char* name, SOCKADDR_IN* from, int* flen)
{
    char ibuf[50];
    *flen = sizeof(SOCKADDR_IN);
    int lb = recvfrom(s, ibuf, sizeof(ibuf) - 1, 0, (sockaddr*)from, flen);
    if (lb == SOCKET_ERROR)
    {
        int e = WSAGetLastError();
        if (e == WSAEWOULDBLOCK || e == WSAETIMEDOUT || e == WSAECONNRESET) return false;
        throw SetErrorMsgText("recvfrom:", e);
    }
    ibuf[lb] = 0;
    return strcmp(ibuf, name) == 0;
}

// [З27]
bool PutAnswerToClient(SOCKET s, char* name, SOCKADDR_IN* to, int lto)
{
    if (sendto(s, name, (int)strlen(name) + 1, 0, (sockaddr*)to, lto) == SOCKET_ERROR)
        throw SetErrorMsgText("sendto:", WSAGetLastError());
    return true;
}

// [З28]
DWORD WINAPI ResponseServer(LPVOID pPrm)
{
    volatile TalkersCommand& cmd = *(volatile TalkersCommand*)pPrm;
    SOCKET sU = INVALID_SOCKET;
    try
    {
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 0), &wsaData) != 0)
            throw SetErrorMsgText("Startup:", WSAGetLastError());
        if ((sU = socket(AF_INET, SOCK_DGRAM, NULL)) == INVALID_SOCKET)
            throw SetErrorMsgText("socket:", WSAGetLastError());
        SOCKADDR_IN serv;
        serv.sin_family = AF_INET;
        serv.sin_port = htons((u_short)g_uport);
        serv.sin_addr.s_addr = INADDR_ANY;
        if (bind(sU, (LPSOCKADDR)&serv, sizeof(serv)) == SOCKET_ERROR)
            throw SetErrorMsgText("bind:", WSAGetLastError());
        u_long nonblk = 1;
        if (ioctlsocket(sU, FIONBIO, &nonblk) == SOCKET_ERROR)
            throw SetErrorMsgText("ioctlsocket:", WSAGetLastError());
        SOCKADDR_IN from;
        int lfrom = sizeof(from);
        while (cmd != EXIT)
        {
            bool got = GetRequestFromClient(sU, g_ucall, &from, &lfrom);
            if (got)
            {
                if (g_accepting)
                {
                    cout << "ResponseServer: call from " << inet_ntoa(from.sin_addr)
                        << ":" << ntohs(from.sin_port) << endl;
                    PutAnswerToClient(sU, g_ucall, &from, lfrom);
                }
            }
            else Sleep(10);
        }
        closesocket(sU);
        WSACleanup();
    }
    catch (string errorMsgText)
    {
        cout << endl << "ResponseServer: " << errorMsgText << endl;
    }
    return 0;
}

string SetPipeError(string msg, DWORD code)
{
    return msg + to_string((unsigned long)code);
}

// [З22][З23][З25]
DWORD WINAPI ConsolePipe(LPVOID pPrm)
{
    volatile TalkersCommand& cmd = *(volatile TalkersCommand*)pPrm;
    HANDLE hPipe = INVALID_HANDLE_VALUE;
    try
    {
        string pn = string("\\\\.\\pipe\\") + g_pipe;
        if ((hPipe = CreateNamedPipeA(pn.c_str(), PIPE_ACCESS_DUPLEX,
            PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
            1, 512, 512, 0, NULL)) == INVALID_HANDLE_VALUE)
            throw SetPipeError("CreateNamedPipe:", GetLastError());
        bool shuttingDown = false;
        while (cmd != EXIT)
        {
            if (!ConnectNamedPipe(hPipe, NULL) && GetLastError() != ERROR_PIPE_CONNECTED)
                throw SetPipeError("ConnectNamedPipe:", GetLastError());
            cout << "ConsolePipe: console connected" << endl;
            bool alive = true;
            while (alive && cmd != EXIT)
            {
                if (shuttingDown || cmd != GETCOMMAND)
                {
                    Sleep(50);
                    continue;
                }
                char rbuf[100], wbuf[128];
                DWORD rb = 0, wb = 0;
                if (!ReadFile(hPipe, rbuf, sizeof(rbuf) - 1, &rb, NULL))
                {
                    alive = false;
                    break;
                }
                rbuf[rb] = 0;
                while (rb > 0 && (rbuf[rb - 1] == '\r' || rbuf[rb - 1] == '\n' || rbuf[rb - 1] == 0))
                    rbuf[--rb] = 0;

                TalkersCommand code = START;
                bool known = false;
                for (int i = 0; i < NCMD; i++)
                    if (strcmp(rbuf, TalkersCmd[i].name) == 0)
                    {
                        code = TalkersCmd[i].code;
                        known = true;
                        break;
                    }

                if (!known)
                {
                    strcpy(wbuf, "nocmd");
                }
                else if (code == STATISTICS)
                {
                    sprintf(wbuf, "total=%ld active=%ld ok=%ld refused=%ld",
                        (long)stats.total, (long)stats.active, (long)stats.ok, (long)stats.refused);
                    cout << "ConsolePipe: " << wbuf << endl;
                }
                else
                {
                    strcpy(wbuf, rbuf);
                    cout << "ConsolePipe: command " << rbuf << endl;
                }
                WriteFile(hPipe, wbuf, (DWORD)strlen(wbuf) + 1, &wb, NULL);
                if (known && code != STATISTICS)
                {
                    if (code == SHUTDOWN) shuttingDown = true;
                    cmd = code;
                }
            }
            DisconnectNamedPipe(hPipe);
        }
        CloseHandle(hPipe);
    }
    catch (string errorMsgText)
    {
        cout << endl << "ConsolePipe: " << errorMsgText << endl;
    }
    return 0;
}

// [З5][З7][З14][З29]
int main(int argc, char* argv[])
{
    if (argc > 1) g_port = atoi(argv[1]);
    if (argc > 2) strcpy_s(g_dll, argv[2]);
    if (argc > 3) strcpy_s(g_pipe, argv[3]);
    if (argc > 4) strcpy_s(g_ucall, argv[4]);
    if (argc > 5) g_uport = atoi(argv[5]);
    cout << "ConcurrentServer: tcp port " << g_port << ", dll " << g_dll
        << ", pipe " << g_pipe << ", call " << g_ucall << ", udp port " << g_uport << endl;

    HMODULE st = LoadLibraryA(g_dll);
    if (st == NULL)
    {
        cout << "LoadLibrary error " << GetLastError() << endl;
        return 1;
    }
    ts = (TS)GetProcAddress(st, "SSS");
    if (ts == NULL)
    {
        cout << "GetProcAddress error " << GetLastError() << endl;
        return 1;
    }

    InitializeCriticalSection(&scListContact);
    volatile TalkersCommand cmd = START;

    hAcceptServer = CreateThread(NULL, 0, AcceptServer, (LPVOID)&cmd, 0, NULL);
    hDispatchServer = CreateThread(NULL, 0, DispatchServer, (LPVOID)&cmd, 0, NULL);
    hGarbageCleaner = CreateThread(NULL, 0, GarbageCleaner, (LPVOID)&cmd, 0, NULL);
    hConsolePipe = CreateThread(NULL, 0, ConsolePipe, (LPVOID)&cmd, 0, NULL);
    hResponseServer = CreateThread(NULL, 0, ResponseServer, (LPVOID)&cmd, 0, NULL);

    SetThreadPriority(hAcceptServer, THREAD_PRIORITY_HIGHEST);
    SetThreadPriority(hDispatchServer, THREAD_PRIORITY_ABOVE_NORMAL);
    SetThreadPriority(hResponseServer, THREAD_PRIORITY_ABOVE_NORMAL);
    SetThreadPriority(hConsolePipe, THREAD_PRIORITY_NORMAL);
    SetThreadPriority(hGarbageCleaner, THREAD_PRIORITY_LOWEST);

    WaitForSingleObject(hAcceptServer, INFINITE);
    CloseHandle(hAcceptServer);
    WaitForSingleObject(hDispatchServer, INFINITE);
    CloseHandle(hDispatchServer);
    WaitForSingleObject(hGarbageCleaner, INFINITE);
    CloseHandle(hGarbageCleaner);
    WaitForSingleObject(hResponseServer, INFINITE);
    CloseHandle(hResponseServer);
    if (WaitForSingleObject(hConsolePipe, 3000) == WAIT_TIMEOUT)
        TerminateThread(hConsolePipe, 0);
    CloseHandle(hConsolePipe);

    DeleteCriticalSection(&scListContact);
    FreeLibrary(st);
    cout << "ConcurrentServer: stopped" << endl;
    return 0;
}