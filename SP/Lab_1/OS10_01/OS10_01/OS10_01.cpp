#include <iostream>
#include "HT.h"

using namespace std;

int main()
{
    setlocale(LC_ALL, "ru");
    HT::HTHANDLE* ht = nullptr;
    try
    {
        ht = HT::Create(1000, 3, 10, 256, "HTspace.ht");
        if (ht) cout << "-- Create: success\n";
        else throw "-- Create: error";

        HT::Element* el1 = new HT::Element("key222", 7, "payload", 8);
        if (HT::Insert(ht, el1)) cout << "-- Insert: success\n";
        else throw "-- Insert: error";

        HT::Element* searchEl = new HT::Element("key222", 7);
        HT::Element* hte = HT::Get(ht, searchEl);
        if (hte) cout << "-- Get: success\n";
        else throw "-- Get: error";

        HT::print(hte);

        if (HT::Snap(ht)) cout << "-- Snap: success\n";
        else throw "-- Snap: error";

        if (HT::Update(ht, hte, "newpayload", 11)) cout << "-- Update: success\n";
        else throw "-- Update: error";

        HT::Element* hte1 = HT::Get(ht, searchEl);
        if (hte1) cout << "-- Get (after update): success\n";
        else throw "-- Get: error";

        HT::print(hte1);

        if (HT::Delete(ht, hte1)) cout << "-- Delete: success\n";
        else throw "-- Delete: error";

        if (HT::Close(ht)) cout << "-- Close: success\n";
        else throw "-- Close: error";
    }
    catch (const char* msg)
    {
        cout << msg << "\n";
        if (ht != nullptr) cout << HT::GetLastError(ht) << "\n";
    }
    return 0;
}