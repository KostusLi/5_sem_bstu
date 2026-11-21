#pragma once
#include <afxwin.h>
#include <afxdlgs.h>
#include <cmath>
#include <vector>

#include "resource.h"
#include "CMatrix.h"
#include "LibGraph.h"
#include "LibSurface.h"

// Вспомогательный билдер шаблона диалога в памяти (не требует правки .rc)
class CDlgTemplateBuilder
{
	std::vector<BYTE> m_buf;

	void AlignDword()
	{
		while (m_buf.size() % 4 != 0)
			m_buf.push_back(0);
	}

	void WriteBytes(const void* data, size_t size)
	{
		const BYTE* p = static_cast<const BYTE*>(data);
		m_buf.insert(m_buf.end(), p, p + size);
	}

	void WriteWord(WORD w) { WriteBytes(&w, sizeof(w)); }

	void WriteString(LPCWSTR str)
	{
		WriteBytes(str, (wcslen(str) + 1) * sizeof(WCHAR));
	}

public:
	void BeginDialog(LPCWSTR title, short cx, short cy, WORD itemCount)
	{
		m_buf.clear();
		DLGTEMPLATE dt = {};
		dt.style = WS_POPUP | WS_BORDER | WS_SYSMENU | WS_CAPTION | DS_MODALFRAME | DS_SETFONT | DS_CENTER;
		dt.cdit = itemCount;
		dt.cx = cx;
		dt.cy = cy;
		WriteBytes(&dt, sizeof(dt));
		WriteWord(0); // без меню
		WriteWord(0); // стандартный класс диалога
		WriteString(title);
		WriteWord(9); // размер шрифта
		WriteString(L"Segoe UI");
	}

	void AddControl(WORD classAtom, DWORD style, short x, short y, short cx, short cy, WORD id, LPCWSTR text)
	{
		AlignDword();
		DLGITEMTEMPLATE item = {};
		item.style = style | WS_CHILD | WS_VISIBLE;
		item.x = x;
		item.y = y;
		item.cx = cx;
		item.cy = cy;
		item.id = id;
		WriteBytes(&item, sizeof(item));
		WriteWord(0xFFFF);
		WriteWord(classAtom); // 0x0080 Button, 0x0081 Edit, 0x0082 Static
		WriteString(text);
		WriteWord(0);
	}

	LPCDLGTEMPLATE GetTemplate() const
	{
		return reinterpret_cast<LPCDLGTEMPLATE>(m_buf.data());
	}
};

// Модальный диалог ввода сферических координат (r, phi, theta)
class CSphericalDlg : public CDialog
{
	enum
	{
		ID_EDIT_R = 1001,
		ID_EDIT_FI = 1002,
		ID_EDIT_Q = 1003
	};

	CDlgTemplateBuilder m_builder;
	double m_minR;

	static bool ParseDouble(const CString& s, double& out)
	{
		CString trimmed = s;
		trimmed.Trim();
		trimmed.Replace(L',', L'.');
		if (trimmed.IsEmpty())
			return false;
		wchar_t* endPtr = nullptr;
		out = wcstod(trimmed, &endPtr);
		return endPtr != nullptr && *endPtr == L'\0' && std::isfinite(out);
	}

public:
	double m_r;
	double m_fi;
	double m_q;

	CSphericalDlg(LPCWSTR title, double r, double fi, double q, double minR, CWnd* pParent = nullptr)
		: m_minR(minR), m_r(r), m_fi(fi), m_q(q)
	{
		m_builder.BeginDialog(title, 175, 108, 8);
		m_builder.AddControl(0x0082, SS_LEFT, 12, 14, 62, 10, 2001, L"r:");
		m_builder.AddControl(0x0081, ES_AUTOHSCROLL | WS_BORDER | WS_TABSTOP, 78, 12, 84, 14, ID_EDIT_R, L"");

		m_builder.AddControl(0x0082, SS_LEFT, 12, 36, 62, 10, 2002, L"\u03C6 (\u0433\u0440\u0430\u0434.):");
		m_builder.AddControl(0x0081, ES_AUTOHSCROLL | WS_BORDER | WS_TABSTOP, 78, 34, 84, 14, ID_EDIT_FI, L"");

		m_builder.AddControl(0x0082, SS_LEFT, 12, 58, 62, 10, 2003, L"\u03B8 (\u0433\u0440\u0430\u0434.):");
		m_builder.AddControl(0x0081, ES_AUTOHSCROLL | WS_BORDER | WS_TABSTOP, 78, 56, 84, 14, ID_EDIT_Q, L"");

		m_builder.AddControl(0x0080, BS_DEFPUSHBUTTON | WS_TABSTOP, 36, 82, 48, 16, IDOK, L"OK");
		m_builder.AddControl(0x0080, BS_PUSHBUTTON | WS_TABSTOP, 92, 82, 48, 16, IDCANCEL, L"\u041E\u0442\u043C\u0435\u043D\u0430");

		InitModalIndirect(m_builder.GetTemplate(), pParent);
	}

protected:
	BOOL OnInitDialog() override
	{
		CDialog::OnInitDialog();
		CString s;
		s.Format(L"%g", m_r);  SetDlgItemTextW(ID_EDIT_R, s);
		s.Format(L"%g", m_fi); SetDlgItemTextW(ID_EDIT_FI, s);
		s.Format(L"%g", m_q);  SetDlgItemTextW(ID_EDIT_Q, s);
		return TRUE;
	}

	void OnOK() override
	{
		CString sR, sFi, sQ;
		GetDlgItemTextW(ID_EDIT_R, sR);
		GetDlgItemTextW(ID_EDIT_FI, sFi);
		GetDlgItemTextW(ID_EDIT_Q, sQ);

		double r = 0.0, fi = 0.0, q = 0.0;
		if (!ParseDouble(sR, r) || !ParseDouble(sFi, fi) || !ParseDouble(sQ, q))
		{
			AfxMessageBox(L"Введите корректные числовые значения.", MB_ICONWARNING);
			return;
		}
		if (r <= m_minR)
		{
			CString msg;
			msg.Format(L"Значение r должно быть больше радиуса шара (r > %g).", m_minR);
			AfxMessageBox(msg, MB_ICONWARNING);
			return;
		}

		m_r = r;
		m_fi = fi;
		m_q = q;
		CDialog::OnOK();
	}
};

class CMainWnd : public CFrameWnd
{
private:
	CRect WinRect;        // Квадратная область отрисовки шара
	int Index;            // 0 - пустое окно, 1 - диффузная, 2 - зеркальная
	double SphereRadius;  // Радиус шара R = 2
	CMatrix PView;        // Камера (наблюдатель): (r, phi, theta)
	CMatrix PSourceLight; // Источник света: (r, phi, theta)
	COLORREF LightColor;  // Цвет источника света
	CMenu menu;
	double delta;
	CFont m_font;

	static double WrapAngle360(double a)
	{
		double res = fmod(a, 360.0);
		return (res < 0.0) ? (res + 360.0) : res;
	}

	static double ClampTheta(double q)
	{
		if (q < 0.0) return 0.0;
		if (q > 180.0) return 180.0;
		return q;
	}

	DECLARE_MESSAGE_MAP()

public:
	CMainWnd()
	{
		Index = 0;
		SphereRadius = 2.0;
		delta = 10.0;
		LightColor = RGB(255, 255, 0); // Желтый по умолчанию

		PView.RedimMatrix(3);
		PView(0) = 15.0; PView(1) = 30.0; PView(2) = 60.0; // Наблюдатель по умолчанию

		PSourceLight.RedimMatrix(3);
		PSourceLight(0) = 10.0; PSourceLight(1) = 45.0; PSourceLight(2) = 45.0; // Свет по умолчанию

		Create(NULL, L"Лабораторная работа №10", WS_OVERLAPPEDWINDOW, CRect(50, 50, 750, 750), NULL, NULL);
	}

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);

	afx_msg void OnDiffuseModel();
	afx_msg void OnMirrorModel();
	afx_msg void OnCamera();
	afx_msg void OnLight();
	afx_msg void OnLightColor();
	afx_msg void Exit();
};

BEGIN_MESSAGE_MAP(CMainWnd, CFrameWnd)
	ON_WM_CREATE()
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	ON_WM_KEYDOWN()
	ON_COMMAND(DIFFUSION, OnDiffuseModel)
	ON_COMMAND(MIRROR, OnMirrorModel)
	ON_COMMAND(ID_CAMERA, OnCamera)
	ON_COMMAND(ID_LIGHT, OnLight)
	ON_COMMAND(ID_LIGHT_COLOR, OnLightColor)
	ON_COMMAND(ID_EXIT, Exit)
END_MESSAGE_MAP()

inline int CMainWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	m_font.CreatePointFont(100, L"Segoe UI");
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	menu.LoadMenu(IDR_MENU1);
	SetMenu(&menu);
	return 0;
}

inline BOOL CMainWnd::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}

inline void CMainWnd::OnSize(UINT nType, int cx, int cy)
{
	CFrameWnd::OnSize(nType, cx, cy);

	const int topMargin = 55;
	const int margin = 20;
	int availW = cx - 2 * margin;
	int availH = cy - topMargin - margin;
	int side = (availW < availH) ? availW : availH;
	if (side < 50) side = 50;
	if (side > 380) side = 380;

	int left = (cx - side) / 2;
	int top = topMargin + (availH - side) / 2;
	if (top < topMargin) top = topMargin;

	WinRect.SetRect(left, top, left + side, top + side);
	Invalidate(FALSE);
}

inline void CMainWnd::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	bool changed = true;
	switch (nChar)
	{
	case VK_LEFT:  PView(1) = WrapAngle360(PView(1) - delta); break;
	case VK_RIGHT: PView(1) = WrapAngle360(PView(1) + delta); break;
	case VK_UP:    PView(2) = ClampTheta(PView(2) - delta);   break;
	case VK_DOWN:  PView(2) = ClampTheta(PView(2) + delta);   break;

	case 'A': PSourceLight(1) = WrapAngle360(PSourceLight(1) - delta); break;
	case 'D': PSourceLight(1) = WrapAngle360(PSourceLight(1) + delta); break;
	case 'W': PSourceLight(2) = ClampTheta(PSourceLight(2) - delta);   break;
	case 'S': PSourceLight(2) = ClampTheta(PSourceLight(2) + delta);   break;

	default:
		changed = false;
		break;
	}

	if (changed && Index != 0)
		Invalidate(FALSE);

	CFrameWnd::OnKeyDown(nChar, nRepCnt, nFlags);
}

inline void CMainWnd::OnPaint()
{
	CPaintDC dc(this);
	CRect clientRect;
	GetClientRect(&clientRect);
	if (clientRect.Width() <= 0 || clientRect.Height() <= 0)
		return;

	CDC memDC;
	memDC.CreateCompatibleDC(&dc);
	CBitmap memBmp;
	memBmp.CreateCompatibleBitmap(&dc, clientRect.Width(), clientRect.Height());
	CBitmap* pOldBmp = memDC.SelectObject(&memBmp);

	memDC.FillSolidRect(&clientRect, RGB(255, 255, 255));

	if (Index == 1 || Index == 2)
	{
		int modelIndex = (Index == 1) ? 0 : 1;
		DrawLightSphere(memDC, SphereRadius, PView, PSourceLight, WinRect, LightColor, modelIndex);

		CFont* pOldFont = memDC.SelectObject(&m_font);
		memDC.SetBkMode(TRANSPARENT);
		memDC.SetTextColor(RGB(0, 0, 0));

		CString sCam, sLight;
		sCam.Format(L"Камера (r, \u03C6, \u03B8): (%.1f, %.1f\u00B0, %.1f\u00B0)",
			PView(0), PView(1), PView(2));
		sLight.Format(L"Источник света (r, \u03C6, \u03B8): (%.1f, %.1f\u00B0, %.1f\u00B0)",
			PSourceLight(0), PSourceLight(1), PSourceLight(2));

		memDC.TextOutW(10, 8, sCam);
		memDC.TextOutW(10, 28, sLight);
		memDC.SelectObject(pOldFont);
	}

	dc.BitBlt(0, 0, clientRect.Width(), clientRect.Height(), &memDC, 0, 0, SRCCOPY);
	memDC.SelectObject(pOldBmp);
}

inline void CMainWnd::OnDiffuseModel()
{
	Index = 1;
	Invalidate(FALSE);
}

inline void CMainWnd::OnMirrorModel()
{
	Index = 2;
	Invalidate(FALSE);
}

inline void CMainWnd::OnCamera()
{
	CSphericalDlg dlg(L"Камера (r, \u03C6, \u03B8)", PView(0), PView(1), PView(2), SphereRadius, this);
	if (dlg.DoModal() == IDOK)
	{
		PView(0) = dlg.m_r;
		PView(1) = dlg.m_fi;
		PView(2) = dlg.m_q;
		Invalidate(FALSE);
	}
}

inline void CMainWnd::OnLight()
{
	CSphericalDlg dlg(L"Источник света (r, \u03C6, \u03B8)", PSourceLight(0), PSourceLight(1), PSourceLight(2), SphereRadius, this);
	if (dlg.DoModal() == IDOK)
	{
		PSourceLight(0) = dlg.m_r;
		PSourceLight(1) = dlg.m_fi;
		PSourceLight(2) = dlg.m_q;
		Invalidate(FALSE);
	}
}

inline void CMainWnd::OnLightColor()
{
	CColorDialog dlg(LightColor, CC_FULLOPEN, this);
	if (dlg.DoModal() == IDOK)
	{
		LightColor = dlg.GetColor();
		Invalidate(FALSE);
	}
}

inline void CMainWnd::Exit()
{
	DestroyWindow();
}