#pragma once
#include <Windows.h>
#include <Unknwn.h>

static const GUID CLSID_OS12 =
{ 0x8a1b6f52, 0x3c4d, 0x4e7a, { 0x9b, 0x21, 0x5d, 0x0f, 0x7c, 0x3e, 0x8a, 0x61 } };

static const GUID IID_Adder =
{ 0x1f3a7b90, 0x6c2e, 0x4d58, { 0xa4, 0xb3, 0x92, 0xe7, 0xd1, 0xc0, 0x5f, 0x48 } };

static const GUID IID_Multiplier =
{ 0xc47e2d19, 0x8b5a, 0x4f63, { 0xb0, 0xd7, 0x3a, 0x91, 0xe6, 0xf2, 0xc8, 0x5d } };

struct IAdder : public IUnknown
{
    virtual HRESULT __stdcall Add(const double x, const double y, double& z) = 0;
    virtual HRESULT __stdcall Sub(const double x, const double y, double& z) = 0;
};

struct IMultiplier : public IUnknown
{
    virtual HRESULT __stdcall Mul(const double x, const double y, double& z) = 0;
    virtual HRESULT __stdcall Div(const double x, const double y, double& z) = 0;
};