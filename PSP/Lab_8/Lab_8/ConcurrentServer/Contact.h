#pragma once
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Winsock2.h>
#include <windows.h>
#include <string.h>
#include <list>

enum TalkersCommand { START, STOP, EXIT, STATISTICS, WAIT, SHUTDOWN, GETCOMMAND };

struct Contact
{
    enum TE { EMPTY, ACCEPT, CONTACT } type;
    enum ST { WORK, ABORT, TIMEOUT, FINISH } sthread;

    SOCKET      s;
    SOCKADDR_IN prms;
    int         lprms;
    HANDLE      hthread;
    HANDLE      htimer;
    HANDLE      hAcceptServer;
    PAPCFUNC    ASStart;
    PAPCFUNC    ASFinish;
    DWORD       tick;

    char msg[50];
    char srvname[15];

    Contact(TE t = EMPTY, const char* namesrv = "")
    {
        memset(&prms, 0, sizeof(SOCKADDR_IN));
        lprms = sizeof(SOCKADDR_IN);
        type = t;
        sthread = WORK;
        s = INVALID_SOCKET;
        hthread = NULL;
        htimer = NULL;
        hAcceptServer = NULL;
        ASStart = NULL;
        ASFinish = NULL;
        tick = GetTickCount();
        strcpy_s(srvname, sizeof(srvname), namesrv);
        msg[0] = 0;
    }

    void SetST(ST sth, const char* m = "")
    {
        sthread = sth;
        strcpy_s(msg, sizeof(msg), m);
    }
};

typedef std::list<Contact> ListContact;