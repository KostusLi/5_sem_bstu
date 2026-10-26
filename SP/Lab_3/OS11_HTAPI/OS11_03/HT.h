#pragma once
#include <Windows.h>
#include <ctime>

#ifdef OS11_HTAPI_EXPORTS
#define HT_API __declspec(dllexport)
#else
#define HT_API __declspec(dllimport)
#endif

namespace HT
{
    struct HT_API HTHANDLE
    {
        HTHANDLE();
        HTHANDLE(int Capacity, int SecSnapshotInterval, int MaxKeyLength, int MaxPayloadLength, const char FileName[512]);
        int     Capacity;
        int     SecSnapshotInterval;
        int     MaxKeyLength;
        int     MaxPayloadLength;
        char    FileName[512];
        HANDLE  File;
        HANDLE  FileMapping;
        LPVOID  Addr;
        char    LastErrorMessage[512];
        time_t  lastsnaptime;
        HANDLE  Mutex;
    };

    struct HT_API Element
    {
        Element();
        Element(const void* key, int keylength);
        Element(const void* key, int keylength, const void* payload, int payloadlength);
        Element(Element* oldelement, const void* newpayload, int newpayloadlength);
        const void* key;
        int         keylength;
        const void* payload;
        int         payloadlength;
    };

    HT_API HTHANDLE* Create(int Capacity, int SecSnapshotInterval, int MaxKeyLength, int MaxPayloadLength, const char FileName[512]);
    HT_API HTHANDLE* Open(const char FileName[512]);
    HT_API BOOL Snap(const HTHANDLE* hthandle);
    HT_API BOOL Close(const HTHANDLE* hthandle);
    HT_API BOOL Insert(const HTHANDLE* hthandle, const Element* element);
    HT_API BOOL Delete(const HTHANDLE* hthandle, const Element* element);
    HT_API Element* Get(const HTHANDLE* hthandle, const Element* element);
    HT_API BOOL Update(const HTHANDLE* hthandle, const Element* oldelement, const void* newpayload, int newpayloadlength);
    HT_API char* GetLastError(HTHANDLE* ht);
    HT_API void print(const Element* element);
};