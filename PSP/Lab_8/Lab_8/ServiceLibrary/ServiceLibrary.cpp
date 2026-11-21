#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#include "Contact.h"
#include <ctime>
#include <cstdlib>
#include <cstdio>
#include "DefineTableService.h"
#pragma comment(lib, "WS2_32.lib")

// [З26] API обслуживающего потока
Contact* SvcBegin(LPVOID prm, const char* name)
{
    Contact* c = (Contact*)prm;
    strcpy_s(c->srvname, sizeof(c->srvname), name);
    srand((unsigned)(GetTickCount() ^ GetCurrentThreadId()));
    QueueUserAPC(c->ASStart, c->hAcceptServer, (ULONG_PTR)c);
    return c;
}

int SvcRecv(Contact* c, char* buf, int len)
{
    int n = recv(c->s, buf, len - 1, 0);
    if (n > 0) buf[n] = 0;
    return n;
}

bool SvcSend(Contact* c, const char* buf, int len)
{
    return send(c->s, buf, len, 0) != SOCKET_ERROR;
}

void SvcEnd(Contact* c, Contact::ST st, const char* msg)
{
    closesocket(c->s);
    c->SetST(st, msg);
    QueueUserAPC(c->ASFinish, c->hAcceptServer, (ULONG_PTR)c);
}

// [З13][З26] echo
DWORD WINAPI EchoServer(LPVOID prm)
{
    Contact* c = SvcBegin(prm, "EchoServer");
    char buf[256];
    int n;
    while ((n = SvcRecv(c, buf, sizeof(buf))) > 0)
    {
        if (!SvcSend(c, buf, n))
        {
            SvcEnd(c, Contact::ABORT, "send error");
            return 1;
        }
    }
    if (n == 0) SvcEnd(c, Contact::FINISH, "ok");
    else        SvcEnd(c, Contact::ABORT, "recv error");
    return 0;
}

// [З26] time
DWORD WINAPI TimeServer(LPVOID prm)
{
    Contact* c = SvcBegin(prm, "TimeServer");
    char buf[256], out[40];
    int n;
    while ((n = SvcRecv(c, buf, sizeof(buf))) > 0)
    {
        if (strcmp(buf, "time") != 0)
        {
            SvcEnd(c, Contact::FINISH, "other request");
            return 0;
        }
        time_t t = time(NULL);
        tm lt;
        localtime_s(&lt, &t);
        strftime(out, sizeof(out), "%d.%m.%Y/%H:%M:%S", &lt);
        if (!SvcSend(c, out, (int)strlen(out) + 1))
        {
            SvcEnd(c, Contact::ABORT, "send error");
            return 1;
        }
    }
    if (n == 0) SvcEnd(c, Contact::FINISH, "ok");
    else        SvcEnd(c, Contact::ABORT, "recv error");
    return 0;
}

// [З26] rand
DWORD WINAPI RandServer(LPVOID prm)
{
    Contact* c = SvcBegin(prm, "RandServer");
    char buf[256];
    int n;
    while ((n = SvcRecv(c, buf, sizeof(buf))) > 0)
    {
        if (strcmp(buf, "rand") != 0)
        {
            SvcEnd(c, Contact::FINISH, "other request");
            return 0;
        }
        int r = (int)(((unsigned)rand() << 17) ^ ((unsigned)rand() << 2) ^ (unsigned)rand());
        if (!SvcSend(c, (const char*)&r, sizeof(r)))
        {
            SvcEnd(c, Contact::ABORT, "send error");
            return 1;
        }
    }
    if (n == 0) SvcEnd(c, Contact::FINISH, "ok");
    else        SvcEnd(c, Contact::ABORT, "recv error");
    return 0;
}

// [З13][З26] TableService
BEGIN_TABLESERVICE
ENTRYSERVICE("echo", EchoServer),
ENTRYSERVICE("time", TimeServer),
ENTRYSERVICE("rand", RandServer)
END_TABLESERVICE;

// [З13]
extern "C" __declspec(dllexport) HANDLE SSS(char* id, LPVOID prm)
{
    HANDLE rc = NULL;
    size_t i = 0;
    while (i < SIZETS && strcmp(TABLESERVICE_ID(i), id) != 0) i++;
    if (i < SIZETS)
    {
        rc = CreateThread(NULL, 0, TABLESERVICE_FN(i), prm, CREATE_SUSPENDED, NULL);
        if (rc != NULL) SetThreadPriority(rc, THREAD_PRIORITY_NORMAL);   // [З29]
    }
    return rc;
}

BOOL APIENTRY DllMain(HMODULE hinst, DWORD rcall, LPVOID wres)
{
    return TRUE;
}