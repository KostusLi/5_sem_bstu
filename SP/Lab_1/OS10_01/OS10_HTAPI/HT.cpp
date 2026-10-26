#include "HT.h"
#include <windows.h>
#include <iostream>
#include <string>

using namespace std;

namespace HT
{
    struct StoredItem {
        bool isOccupied;
        bool isDeleted;
        int keyLength;
        int payloadLength;
    };

    int GetItemSize(const HTHANDLE* ht) {
        return sizeof(StoredItem) + ht->MaxKeyLength + ht->MaxPayloadLength;
    }

    StoredItem* GetItem(const HTHANDLE* ht, int index) {
        char* baseAddr = (char*)ht->Addr + sizeof(HTHANDLE);
        return (StoredItem*)(baseAddr + index * GetItemSize(ht));
    }

    int Hash(const void* key, int keylength, int capacity) {
        const char* p = (const char*)key;
        unsigned int hash = 0;
        for (int i = 0; i < keylength; i++) {
            hash += (unsigned char)p[i];
        }
        return hash % capacity;
    }

    DWORD WINAPI SnapshotThread(LPVOID lpParam) {
        HTHANDLE* ht = (HTHANDLE*)lpParam;
        while (true) {
            Sleep(ht->SecSnapshotInterval * 1000);

            if (ht->FileMapping == NULL) break;

            Snap(ht);
        }
        return 0;
    }

    HTHANDLE::HTHANDLE() {
        Capacity = 0; SecSnapshotInterval = 0; MaxKeyLength = 0; MaxPayloadLength = 0;
        File = NULL; FileMapping = NULL; Addr = NULL; lastsnaptime = 0;
        memset(FileName, 0, sizeof(FileName));
        memset(LastErrorMessage, 0, sizeof(LastErrorMessage));
    }

    HTHANDLE::HTHANDLE(int c, int s, int mk, int mp, const char fn[512]) {
        Capacity = c; SecSnapshotInterval = s; MaxKeyLength = mk; MaxPayloadLength = mp;
        File = NULL; FileMapping = NULL; Addr = NULL; lastsnaptime = time(NULL);
        strcpy_s(FileName, fn);
        memset(LastErrorMessage, 0, sizeof(LastErrorMessage));
    }

    Element::Element() {
        key = nullptr; keylength = 0; payload = nullptr; payloadlength = 0;
    }

    Element::Element(const void* k, int kl) {
        key = k; keylength = kl; payload = nullptr; payloadlength = 0;
    }

    Element::Element(const void* k, int kl, const void* p, int pl) {
        key = k; keylength = kl; payload = p; payloadlength = pl;
    }

    HTHANDLE* Create(int Capacity, int SecSnapshotInterval, int MaxKeyLength, int MaxPayloadLength, const char FileName[512])
    {
        HTHANDLE* ht = new HTHANDLE(Capacity, SecSnapshotInterval, MaxKeyLength, MaxPayloadLength, FileName);

        ht->File = CreateFileA(FileName, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
            NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

        if (ht->File == INVALID_HANDLE_VALUE) return nullptr;

        int fileSize = sizeof(HTHANDLE) + (Capacity * GetItemSize(ht));

        ht->FileMapping = CreateFileMappingA(ht->File, NULL, PAGE_READWRITE, 0, fileSize, NULL);
        if (!ht->FileMapping) return nullptr;

        ht->Addr = MapViewOfFile(ht->FileMapping, FILE_MAP_ALL_ACCESS, 0, 0, fileSize);
        if (!ht->Addr) return nullptr;

        memcpy(ht->Addr, ht, sizeof(HTHANDLE));

        for (int i = 0; i < Capacity; i++) {
            StoredItem* item = GetItem(ht, i);
            item->isOccupied = false;
            item->isDeleted = false;
        }

        CreateThread(NULL, 0, SnapshotThread, ht, 0, NULL);

        return ht;
    }

    HTHANDLE* Open(const char FileName[512])
    {
        HANDLE hFile = CreateFileA(FileName, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
            NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile == INVALID_HANDLE_VALUE) return nullptr;

        HTHANDLE tempHt;
        DWORD bytesRead;
        ReadFile(hFile, &tempHt, sizeof(HTHANDLE), &bytesRead, NULL);

        int fileSize = sizeof(HTHANDLE) + (tempHt.Capacity * GetItemSize(&tempHt));
        HANDLE hMapping = CreateFileMappingA(hFile, NULL, PAGE_READWRITE, 0, fileSize, NULL);

        LPVOID addr = MapViewOfFile(hMapping, FILE_MAP_ALL_ACCESS, 0, 0, fileSize);

        HTHANDLE* ht = new HTHANDLE();
        memcpy(ht, addr, sizeof(HTHANDLE));

        ht->File = hFile;
        ht->FileMapping = hMapping;
        ht->Addr = addr;

        CreateThread(NULL, 0, SnapshotThread, ht, 0, NULL);

        return ht;
    }

    BOOL Insert(const HTHANDLE* hthandle, const Element* element)
    {
        if (element->keylength > hthandle->MaxKeyLength || element->payloadlength > hthandle->MaxPayloadLength) {
            return FALSE;
        }

        int index = Hash(element->key, element->keylength, hthandle->Capacity);
        int startIndex = index;

        while (true) {
            StoredItem* item = GetItem(hthandle, index);

            if (!item->isOccupied || item->isDeleted) {
                item->isOccupied = true;
                item->isDeleted = false;
                item->keyLength = element->keylength;
                item->payloadLength = element->payloadlength;

                char* keyDest = (char*)item + sizeof(StoredItem);
                char* payloadDest = keyDest + hthandle->MaxKeyLength;

                memcpy(keyDest, element->key, element->keylength);
                memcpy(payloadDest, element->payload, element->payloadlength);

                return TRUE;
            }

            index = (index + 1) % hthandle->Capacity;
            if (index == startIndex) break;
        }
        return FALSE;
    }

    Element* Get(const HTHANDLE* hthandle, const Element* element)
    {
        int index = Hash(element->key, element->keylength, hthandle->Capacity);
        int startIndex = index;

        while (true) {
            StoredItem* item = GetItem(hthandle, index);

            if (!item->isOccupied && !item->isDeleted) {
                return nullptr;
            }

            if (item->isOccupied) {
                char* itemKey = (char*)item + sizeof(StoredItem);

                if (item->keyLength == element->keylength && memcmp(itemKey, element->key, element->keylength) == 0) {
                    Element* result = new Element();
                    char* payloadSrc = itemKey + hthandle->MaxKeyLength;

                    void* retKey = new char[item->keyLength];
                    void* retPayload = new char[item->payloadLength];

                    memcpy(retKey, itemKey, item->keyLength);
                    memcpy(retPayload, payloadSrc, item->payloadLength);

                    result->key = retKey;
                    result->keylength = item->keyLength;
                    result->payload = retPayload;
                    result->payloadlength = item->payloadLength;

                    return result;
                }
            }

            index = (index + 1) % hthandle->Capacity;
            if (index == startIndex) break;
        }
        return nullptr;
    }

    BOOL Snap(const HTHANDLE* hthandle)
    {
        if (hthandle->Addr != NULL) {
            HTHANDLE* memHt = (HTHANDLE*)hthandle->Addr;
            memHt->lastsnaptime = time(NULL);

            FlushViewOfFile(hthandle->Addr, 0);
            return TRUE;
        }
        return FALSE;
    }

    BOOL Close(const HTHANDLE* hthandle)
    {
        Snap(hthandle);

        HTHANDLE* mutableHt = const_cast<HTHANDLE*>(hthandle);
        HANDLE tempMap = mutableHt->FileMapping;
        mutableHt->FileMapping = NULL;

        Sleep(50);

        UnmapViewOfFile(hthandle->Addr);
        CloseHandle(tempMap);
        CloseHandle(hthandle->File);

        delete hthandle;
        return TRUE;
    }

    BOOL Delete(const HTHANDLE* hthandle, const Element* element)
    {
        int index = Hash(element->key, element->keylength, hthandle->Capacity);
        int startIndex = index;

        while (true) {
            StoredItem* item = GetItem(hthandle, index);

            if (!item->isOccupied && !item->isDeleted) return FALSE;

            if (item->isOccupied) {
                char* itemKey = (char*)item + sizeof(StoredItem);
                if (item->keyLength == element->keylength && memcmp(itemKey, element->key, element->keylength) == 0) {
                    item->isOccupied = false;
                    item->isDeleted = true;
                    return TRUE;
                }
            }

            index = (index + 1) % hthandle->Capacity;
            if (index == startIndex) break;
        }
        return FALSE;
    }

    BOOL Update(const HTHANDLE* hthandle, const Element* oldelement, const void* newpayload, int newpayloadlength)
    {
        if (newpayloadlength > hthandle->MaxPayloadLength) return FALSE;

        int index = Hash(oldelement->key, oldelement->keylength, hthandle->Capacity);
        int startIndex = index;

        while (true) {
            StoredItem* item = GetItem(hthandle, index);

            if (!item->isOccupied && !item->isDeleted) return FALSE;

            if (item->isOccupied) {
                char* itemKey = (char*)item + sizeof(StoredItem);
                if (item->keyLength == oldelement->keylength && memcmp(itemKey, oldelement->key, oldelement->keylength) == 0) {
                    char* payloadDest = itemKey + hthandle->MaxKeyLength;
                    item->payloadLength = newpayloadlength;
                    memcpy(payloadDest, newpayload, newpayloadlength);
                    return TRUE;
                }
            }

            index = (index + 1) % hthandle->Capacity;
            if (index == startIndex) break;
        }
        return FALSE;
    }

    char* GetLastError(HTHANDLE* ht)
    {
        return ht->LastErrorMessage;
    }

    void print(const Element* element)
    {
        if (element == nullptr) {
            cout << "Element is NULL" << endl;
            return;
        }

        string keyStr((const char*)element->key, element->keylength);
        string payloadStr((const char*)element->payload, element->payloadlength);

        cout << "Key: " << keyStr << " | Payload: " << payloadStr << endl;
    }
}