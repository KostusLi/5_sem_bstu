#include "pch.h"
#include <objbase.h>
#include <new>
#include "OS12_IFACE.h"

#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "advapi32.lib")

HRESULT RegisterServer(HMODULE hModule, const CLSID& clsid, const WCHAR* szFriendlyName, const WCHAR* szVerIndProgID, const WCHAR* szProgID);
HRESULT UnregisterServer(const CLSID& clsid, const WCHAR* szVerIndProgID, const WCHAR* szProgID);

HMODULE g_hModule = nullptr;
volatile LONG g_Components = 0;
volatile LONG g_ServerLocks = 0;

class OS12 : public IAdder, public IMultiplier
{
public:
    OS12() : m_ref(1) { InterlockedIncrement(&g_Components); }
    ~OS12() { InterlockedDecrement(&g_Components); }

    HRESULT __stdcall QueryInterface(REFIID riid, void** ppv) override
    {
        if (ppv == nullptr) return E_POINTER;
        if (riid == IID_IUnknown) *ppv = static_cast<IAdder*>(this);
        else if (riid == IID_Adder) *ppv = static_cast<IAdder*>(this);
        else if (riid == IID_Multiplier) *ppv = static_cast<IMultiplier*>(this);
        else { *ppv = nullptr; return E_NOINTERFACE; }
        static_cast<IUnknown*>(*ppv)->AddRef();
        return S_OK;
    }

    ULONG __stdcall AddRef() override { return InterlockedIncrement(&m_ref); }

    ULONG __stdcall Release() override
    {
        LONG n = InterlockedDecrement(&m_ref);
        if (n == 0) delete this;
        return n;
    }

    HRESULT __stdcall Add(const double x, const double y, double& z) override { z = x + y; return S_OK; }
    HRESULT __stdcall Sub(const double x, const double y, double& z) override { z = x - y; return S_OK; }
    HRESULT __stdcall Mul(const double x, const double y, double& z) override { z = x * y; return S_OK; }
    HRESULT __stdcall Div(const double x, const double y, double& z) override
    {
        if (y == 0.0) return E_INVALIDARG;
        z = x / y;
        return S_OK;
    }

private:
    volatile LONG m_ref;
};

class OS12Factory : public IClassFactory
{
public:
    OS12Factory() : m_ref(1) {}

    HRESULT __stdcall QueryInterface(REFIID riid, void** ppv) override
    {
        if (ppv == nullptr) return E_POINTER;
        if (riid == IID_IUnknown || riid == IID_IClassFactory) *ppv = static_cast<IClassFactory*>(this);
        else { *ppv = nullptr; return E_NOINTERFACE; }
        static_cast<IUnknown*>(*ppv)->AddRef();
        return S_OK;
    }

    ULONG __stdcall AddRef() override { return InterlockedIncrement(&m_ref); }

    ULONG __stdcall Release() override
    {
        LONG n = InterlockedDecrement(&m_ref);
        if (n == 0) delete this;
        return n;
    }

    HRESULT __stdcall CreateInstance(IUnknown* pOuter, REFIID riid, void** ppv) override
    {
        if (ppv == nullptr) return E_POINTER;
        *ppv = nullptr;
        if (pOuter != nullptr) return CLASS_E_NOAGGREGATION;
        OS12* p = new (std::nothrow) OS12();
        if (p == nullptr) return E_OUTOFMEMORY;
        HRESULT hr = p->QueryInterface(riid, ppv);
        p->Release();
        return hr;
    }

    HRESULT __stdcall LockServer(BOOL bLock) override
    {
        if (bLock) InterlockedIncrement(&g_ServerLocks);
        else InterlockedDecrement(&g_ServerLocks);
        return S_OK;
    }

private:
    volatile LONG m_ref;
};

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
    if (ppv == nullptr) return E_POINTER;
    *ppv = nullptr;
    if (rclsid != CLSID_OS12) return CLASS_E_CLASSNOTAVAILABLE;
    OS12Factory* f = new (std::nothrow) OS12Factory();
    if (f == nullptr) return E_OUTOFMEMORY;
    HRESULT hr = f->QueryInterface(riid, ppv);
    f->Release();
    return hr;
}

STDAPI DllCanUnloadNow()
{
    return (g_Components == 0 && g_ServerLocks == 0) ? S_OK : S_FALSE;
}

STDAPI DllRegisterServer()
{
    return RegisterServer(g_hModule, CLSID_OS12, L"OS12 COM Component", L"OS12.Component", L"OS12.Component.1");
}

STDAPI DllUnregisterServer()
{
    return UnregisterServer(CLSID_OS12, L"OS12.Component", L"OS12.Component.1");
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID)
{
    if (reason == DLL_PROCESS_ATTACH)
    {
        g_hModule = hModule;
        DisableThreadLibraryCalls(hModule);
    }
    return TRUE;
}