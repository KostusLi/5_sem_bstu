#include <iostream>
#include "HT.h"

#pragma comment(lib, "D:\\5_sem_bstu\\SP\\Lab_1\\OS10_01\\x64\\Debug\\OS10_HTAPI.lib")

using namespace std;

int main()
{
    setlocale(LC_ALL, "Russian");
    HT::HTHANDLE* ht = nullptr;
    try
    {
        ht = HT::Open("HTspace.ht");
        if (ht) cout << "-- Open: success\n";
        else throw "-- Open: error";

        HT::Element* el = new HT::Element("key333", 7, "payload", 8);
        if (HT::Insert(ht, el)) cout << "-- Insert: success\n";
        else throw "-- Insert: error";

        if (HT::Close(ht)) cout << "-- Close: success\n";
        else throw "-- Close: error";
    }
    catch (const char* msg)
    {
        cout << msg << "\n";
    }
    return 0;
}