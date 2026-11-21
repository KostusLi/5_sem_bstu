#pragma once
#include "resource.h"
#include "CMatrix.h"
#include "LibGraph.h"
#include "LibPyramid.h"
#include <fstream>

class CMainWnd : public CFrameWnd
{
private:
	CPyramid PIR;
	CRect WinRect;
	CMatrix PView;
	CMatrix PLight;
	int Index;
	CMenu menu;
	COLORREF color;
	DECLARE_MESSAGE_MAP()

public:
	CMainWnd()
	{
		Create(NULL, L"Lab09", WS_OVERLAPPEDWINDOW, CRect(10, 10, 700, 800), NULL, NULL);
		Index = 0;

		PView.RedimMatrix(3);
		PView(0) = 10; PView(1) = 45; PView(2) = 60;

		PLight.RedimMatrix(3);
		PLight(0) = 100; PLight(1) = 45; PLight(2) = 45;
	}

	int OnCreate(LPCREATESTRUCT lpCreateStruct);
	void OnPaint();
	void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	void OnSize(UINT nType, int cx, int cy);
	void OnPyramid();
	void Exit();
	void Cam();
	void Light();
	void ColRed();
	void ColBlue();
	void ColGreen();
	void ColOrange();
	void ColPink();
	void ColR();
	void ColG();
	void ColB();
};

BEGIN_MESSAGE_MAP(CMainWnd, CFrameWnd)
	ON_WM_CREATE()
	ON_WM_PAINT()
	ON_WM_KEYDOWN()
	ON_WM_SIZE()
	ON_COMMAND(ID_30002, OnPyramid)
	ON_COMMAND(ID_EXIT, Exit)
	ON_COMMAND(ID_30005, Cam)
	ON_COMMAND(ID_30006, Light)
	ON_COMMAND(ID_30008, ColRed)
	ON_COMMAND(ID_30009, ColGreen)
	ON_COMMAND(ID_30010, ColBlue)
	ON_COMMAND(ID_30012, ColOrange)
	ON_COMMAND(ID_30013, ColPink)
END_MESSAGE_MAP()

int CMainWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	menu.LoadMenu(IDR_MENU1);
	SetMenu(&menu);
	color = RGB(255, 0, 0);

	CRect rc;
	GetClientRect(&rc);
	WinRect.SetRect(100, 100, rc.Width() - 100, rc.Height() - 100);
	return 0;
}

void CMainWnd::OnPaint()
{
	CPaintDC dc(this);
	if (Index == 1)
	{
		PIR.ColorDraw(dc, PView, PLight, WinRect, color);
		CString ss;
		ss.Format(L"Наблюдатель: r=%.1f, fi=%.1f, th=%.1f", PView(0), PView(1), PView(2));
		dc.TextOutW(10, 10, ss);
		ss.Format(L"Источник света: r=%.1f, fi=%.1f, th=%.1f", PLight(0), PLight(1), PLight(2));
		dc.TextOutW(10, 35, ss);
	}
}

void CMainWnd::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if (Index == 1)
	{
		switch (nChar)
		{
		case VK_DOWN:
			if (PView(2) - 3 >= 0) PView(2) -= 3;
			break;
		case VK_UP:
			if (PView(2) + 3 <= 180) PView(2) += 3;
			break;
		case VK_RIGHT:
			PView(1) = (PView(1) - 3 >= -180) ? PView(1) - 3 : PView(1) - 3 + 360;
			break;
		case VK_LEFT:
			PView(1) = (PView(1) + 3 <= 180) ? PView(1) + 3 : PView(1) + 3 - 360;
			break;
		case 'S':
			if (PLight(2) - 3 >= 0) PLight(2) -= 3;
			break;
		case 'W':
			if (PLight(2) + 3 <= 180) PLight(2) += 3;
			break;
		case 'D':
			PLight(1) = (PLight(1) - 3 >= -180) ? PLight(1) - 3 : PLight(1) - 3 + 360;
			break;
		case 'A':
			PLight(1) = (PLight(1) + 3 <= 180) ? PLight(1) + 3 : PLight(1) + 3 - 360;
			break;
		case VK_NUMPAD1: ColRed(); return;
		case VK_NUMPAD2: ColGreen(); return;
		case VK_NUMPAD3: ColBlue(); return;
		case VK_NUMPAD4: ColOrange(); return;
		case VK_NUMPAD5: ColPink(); return;
		case VK_NUMPAD7: ColR(); return;
		case VK_NUMPAD8: ColG(); return;
		case VK_NUMPAD9: ColB(); return;
		}
		Invalidate();
	}
	CWnd::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CMainWnd::OnSize(UINT nType, int cx, int cy)
{
	CFrameWnd::OnSize(nType, cx, cy);
	if (cx > 220 && cy > 220)
		WinRect.SetRect(100, 100, cx - 100, cy - 100);
}

void CMainWnd::OnPyramid()
{
	Index = 1;
	Invalidate();
}

void CMainWnd::Exit()
{
	DestroyWindow();
}

void CMainWnd::Cam()
{
	std::ifstream fin("dataCam.txt");
	if (fin.is_open())
	{
		fin >> PView(1) >> PView(2);
		Invalidate();
	}
	else
	{
		MessageBox(L"Файл dataCam.txt не найден!", L"Ошибка", MB_ICONWARNING);
	}
}

void CMainWnd::Light()
{
	std::ifstream fin("dataLight.txt");
	if (fin.is_open())
	{
		fin >> PLight(1) >> PLight(2);
		Invalidate();
	}
	else
	{
		MessageBox(L"Файл dataLight.txt не найден!", L"Ошибка", MB_ICONWARNING);
	}
}

void CMainWnd::ColRed() { color = RGB(255, 0, 0);   Invalidate(); }
void CMainWnd::ColGreen() { color = RGB(0, 255, 0);   Invalidate(); }
void CMainWnd::ColBlue() { color = RGB(0, 0, 255);   Invalidate(); }
void CMainWnd::ColOrange() { color = RGB(255, 128, 0); Invalidate(); }
void CMainWnd::ColPink() { color = RGB(255, 0, 127); Invalidate(); }

void CMainWnd::ColR()
{
	color = RGB((GetRValue(color) + 10) % 256, GetGValue(color), GetBValue(color));
	Invalidate();
}

void CMainWnd::ColG()
{
	color = RGB(GetRValue(color), (GetGValue(color) + 10) % 256, GetBValue(color));
	Invalidate();
}

void CMainWnd::ColB()
{
	color = RGB(GetRValue(color), GetGValue(color), (GetBValue(color) + 10) % 256);
	Invalidate();
}