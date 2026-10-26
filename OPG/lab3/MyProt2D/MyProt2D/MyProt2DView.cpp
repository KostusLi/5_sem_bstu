#include "pch.h"
#include "framework.h"
#include "MyProt2D.h"
#include "MyProt2DDoc.h"
#include "MyProt2DView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(CMyProt2DView, CView)

BEGIN_MESSAGE_MAP(CMyProt2DView, CView)
	ON_COMMAND(ID_TESTS_F1, &CMyProt2DView::OnTestsF1)
	ON_COMMAND(ID_TESTS_F2, &CMyProt2DView::OnTestsF2)
	ON_COMMAND(ID_TESTS_F3, &CMyProt2DView::OnTestsF3)
END_MESSAGE_MAP()

CMyProt2DView::CMyProt2DView() noexcept
{

}

CMyProt2DView::~CMyProt2DView()
{
}

BOOL CMyProt2DView::PreCreateWindow(CREATESTRUCT& cs)
{
	if (!CWnd::PreCreateWindow(cs))
		return FALSE;

	cs.dwExStyle |= WS_EX_CLIENTEDGE;
	cs.style &= ~WS_BORDER;
	cs.lpszClass = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS,
		::LoadCursor(nullptr, IDC_ARROW), reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1), nullptr);

	return TRUE;
}

#pragma region математика
double CMyProt2DView::MyF1(double x)
{
	if (x == 0) return 1;
	return sin(x) / x;
}

double CMyProt2DView::MyF2(double x)
{
	return sqrt(fabs(x)) * sin(x);
}
#pragma endregion

void CMyProt2DView::OnDraw(CDC* pDC)
{
	if (m_ShowShapes)
	{
		Graph.Draw(*pDC, 1, 0);

		int x_left, y_top, x_right, y_bottom;

		Graph.GetWindowCoords(-10.0, 10.0, x_left, y_top);
		Graph.GetWindowCoords(10.0, -10.0, x_right, y_bottom);

		CPen penCircle(PS_SOLID, 2, RGB(0, 0, 255));
		CPen* pOldPen = pDC->SelectObject(&penCircle);
		HGDIOBJ oldBrush = pDC->SelectStockObject(NULL_BRUSH);

		pDC->Ellipse(x_left, y_top, x_right, y_bottom);

		pDC->SelectObject(pOldPen);
		pDC->SelectObject(oldBrush);
		return;
	}

	Graph.Draw(*pDC, 1, 1);
}

void CMyProt2DView::OnTestsF1()
{
	m_ShowShapes = false;
	double xL = -3 * pi;
	double xH = -xL;
	double dx = pi / 36;
	int N = (xH - xL) / dx;
	X.RedimMatrix(N + 1);
	Y.RedimMatrix(N + 1);
	for (int i = 0; i <= N; i++)
	{
		X(i) = xL + i * dx;
		Y(i) = MyF1(X(i));
	}
	PenLine.Set(PS_SOLID, 1, RGB(255, 0, 0));
	PenAxis.Set(PS_SOLID, 2, RGB(0, 0, 255));
	RW.SetRect(500, 150, 900, 550);
	Graph.SetParams(X, Y, RW);
	Graph.SetPenLine(PenLine);
	Graph.SetPenAxis(PenAxis);
	this->Invalidate();
}

void CMyProt2DView::OnTestsF2()
{
	m_ShowShapes = false;
	double xL = -4 * pi;
	double xH = -xL;
	double dx = pi / 36;
	int N = (xH - xL) / dx;
	X.RedimMatrix(N + 1);
	Y.RedimMatrix(N + 1);
	for (int i = 0; i <= N; i++)
	{
		X(i) = xL + i * dx;
		Y(i) = MyF2(X(i));
	}
	PenLine.Set(PS_DASHDOT, 1, RGB(255, 0, 0));
	PenAxis.Set(PS_SOLID, 2, RGB(0, 0, 0));
	RW.SetRect(500, 150, 900, 550);
	Graph.SetParams(X, Y, RW);
	Graph.SetPenLine(PenLine);
	Graph.SetPenAxis(PenAxis);
	this->Invalidate();
}

void CMyProt2DView::OnTestsF3()
{
	m_ShowShapes = true;
	X.RedimMatrix(9);
	Y.RedimMatrix(9);

	double R = 10.0;
	double a = 0.0;

	for (int i = 0; i < 8; ++i) {
		X(i) = R * cos(a);
		Y(i) = R * sin(a);
		a += pi / 4;
	}
	X(8) = X(0);
	Y(8) = Y(0);

	PenLine.Set(PS_SOLID, 3, RGB(255, 0, 0));
	PenAxis.Set(PS_SOLID, 2, RGB(0, 0, 255));

	RW.SetRect(400, 100, 800, 500);

	Graph.SetParams(X, Y, RW);
	Graph.SetPenLine(PenLine);
	Graph.SetPenAxis(PenAxis);

	this->Invalidate();
}

#ifdef _DEBUG
void CMyProt2DView::AssertValid() const
{
	CView::AssertValid();
}

void CMyProt2DView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}
#endif