#include <Windows.h>
#include <objbase.h>
#include <cmath>
#include <mutex>
#include <new>
#include <set>
#include "OS12.h"
#include "OS12_IFACE.h"

#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "uuid.lib")

namespace OS12
{
    namespace
    {
        struct Handle
        {
            IUnknown* pUnknown;
            bool uninit;
        };

        std::mutex g_mtx;
        std::set<Handle*> g_handles;

        Handle* Check(OS12HANDEL h)
        {
            std::lock_guard<std::mutex> lk(g_mtx);
            if (h == nullptr || g_handles.find(static_cast<Handle*>(h)) == g_handles.end())
                throw (int)E_HANDLE;
            return static_cast<Handle*>(h);
        }

        void CheckArgs(double x, double y)
        {
            if (!std::isfinite(x) || !std::isfinite(y)) throw (int)E_INVALIDARG;
        }

        template<class I>
        I* Query(OS12HANDEL h, REFIID iid)
        {
            Handle* hd = Check(h);
            I* p = nullptr;
            HRESULT hr = hd->pUnknown->QueryInterface(iid, reinterpret_cast<void**>(&p));
            if (FAILED(hr)) throw (int)hr;
            if (p == nullptr) throw (int)E_POINTER;
            return p;
        }
    }

    OS12HANDEL Init()
    {
        HRESULT hr = CoInitialize(NULL);
        bool uninit = SUCCEEDED(hr);
        if (FAILED(hr) && hr != RPC_E_CHANGED_MODE) throw (int)hr;

        IUnknown* p = nullptr;
        hr = CoCreateInstance(CLSID_OS12, NULL, CLSCTX_INPROC_SERVER, IID_IUnknown, reinterpret_cast<void**>(&p));
        if (FAILED(hr))
        {
            if (uninit) CoUninitialize();
            throw (int)hr;
        }

        Handle* h = new (std::nothrow) Handle{ p, uninit };
        if (h == nullptr)
        {
            p->Release();
            if (uninit) CoUninitialize();
            throw (int)E_OUTOFMEMORY;
        }

        std::lock_guard<std::mutex> lk(g_mtx);
        g_handles.insert(h);
        return h;
    }

    namespace Adder
    {
        double Add(OS12HANDEL h, double x, double y)
        {
            CheckArgs(x, y);
            IAdder* p = Query<IAdder>(h, IID_Adder);
            double z = 0.0;
            HRESULT hr = p->Add(x, y, z);
            p->Release();
            if (FAILED(hr)) throw (int)hr;
            return z;
        }

        double Sub(OS12HANDEL h, double x, double y)
        {
            CheckArgs(x, y);
            IAdder* p = Query<IAdder>(h, IID_Adder);
            double z = 0.0;
            HRESULT hr = p->Sub(x, y, z);
            p->Release();
            if (FAILED(hr)) throw (int)hr;
            return z;
        }
    }

    namespace Multiplier
    {
        double Mul(OS12HANDEL h, double x, double y)
        {
            CheckArgs(x, y);
            IMultiplier* p = Query<IMultiplier>(h, IID_Multiplier);
            double z = 0.0;
            HRESULT hr = p->Mul(x, y, z);
            p->Release();
            if (FAILED(hr)) throw (int)hr;
            return z;
        }

        double Div(OS12HANDEL h, double x, double y)
        {
            CheckArgs(x, y);
            IMultiplier* p = Query<IMultiplier>(h, IID_Multiplier);
            double z = 0.0;
            HRESULT hr = p->Div(x, y, z);
            p->Release();
            if (FAILED(hr)) throw (int)hr;
            return z;
        }
    }

    void Dispose(OS12HANDEL h)
    {
        Handle* hd = nullptr;
        {
            std::lock_guard<std::mutex> lk(g_mtx);
            auto it = g_handles.find(static_cast<Handle*>(h));
            if (h == nullptr || it == g_handles.end()) throw (int)E_HANDLE;
            hd = *it;
            g_handles.erase(it);
        }
        hd->pUnknown->Release();
        bool uninit = hd->uninit;
        delete hd;
        if (uninit) CoUninitialize();
    }
}