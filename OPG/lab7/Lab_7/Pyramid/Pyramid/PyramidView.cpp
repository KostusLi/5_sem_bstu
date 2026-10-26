#include "pch.h"
#include "framework.h"
#ifndef SHARED_HANDLERS
#include "Pyramid.h"
#endif

#include "PyramidDoc.h"
#include "PyramidView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(CPyramidView, CView)

BEGIN_MESSAGE_MAP(CPyramidView, CView)
    ON_WM_PAINT()
    ON_COMMAND(ID_PYRAMID_CURRENT, &CPyramidView::OnPyramidCurrent)
    ON_COMMAND(ID_PYRAMID_WITH_REMOVAL, &CPyramidView::OnPyramidWithRemoval)
    ON_COMMAND(ID_PYRAMID_WITHOUT_REMOVAL, &CPyramidView::OnPyramidWithoutRemoval)
    ON_COMMAND(ID_PYRAMID_SET_CAMERA, &CPyramidView::OnPyramidSetCamera)
    ON_COMMAND(ID_CAMERA_FROM_FILE, &CPyramidView::OnCameraFromFile)
    ON_COMMAND(ID_CAMERA_FROM_DIALOG, &CPyramidView::OnCameraFromDialog)
    ON_WM_KEYDOWN()
    ON_WM_SIZE()
END_MESSAGE_MAP()

CPyramidView::CPyramidView() noexcept
{
    Index = 0;
    PView.RedimMatrix(3);
    PView(0) = 10;
    PView(1) = 315;
    PView(2) = 45;
}

CPyramidView::~CPyramidView()
{
}

BOOL CPyramidView::PreCreateWindow(CREATESTRUCT& cs)
{
    if (!CWnd::PreCreateWindow(cs))
        return FALSE;

    cs.dwExStyle |= WS_EX_CLIENTEDGE;
    cs.style &= ~WS_BORDER;
    cs.lpszClass = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS,
        ::LoadCursor(NULL, IDC_ARROW), reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1), NULL);

    return TRUE;
}

void CPyramidView::OnDraw(CDC* pDC)
{
    CPyramidDoc* pDoc = GetDocument();
    ASSERT_VALID(pDoc);
    if (!pDoc)
        return;

    CString coordText;
    coordText.Format(_T("r=%.1f, φ=%.1f°, θ=%.1f°"),
        PView(0), PView(1), PView(2));
    (*pDC).SetBkMode(TRANSPARENT);
    (*pDC).SetTextColor(RGB(0, 0, 0));
    CFont font;
    font.CreatePointFont(100, _T("Arial"));
    CFont* pOldFont = (*pDC).SelectObject(&font);
    (*pDC).TextOutW(10, 10, coordText);
    (*pDC).SelectObject(pOldFont);

    if (Index == 1)
        PIR.DrawCurrent(*pDC, PView, WinRect);
    else if (Index == 2)
        PIR.Draw1(*pDC, PView, WinRect);
    else if (Index == 3)
        PIR.Draw(*pDC, PView, WinRect);
    else if (Index == 4)
        PIR.DrawCamera(*pDC, PView, WinRect);
}

void CPyramidView::OnPyramidCurrent()
{
    CString ss = _T("Усечённая пирамида - текущее положение камеры");
    CFrameWnd* pWnd = GetParentFrame();
    pWnd->SetWindowText(ss);
    Index = 1;
    Invalidate();
}

void CPyramidView::OnPyramidWithRemoval()
{
    CString ss = _T("Усечённая пирамида - с удалением невидимых граней (Рис. 1б)");
    CFrameWnd* pWnd = GetParentFrame();
    pWnd->SetWindowText(ss);
    Index = 2;
    Invalidate();
}

void CPyramidView::OnPyramidWithoutRemoval()
{
    CString ss = _T("Усечённая пирамида - без удаления невидимых граней (Рис. 1а)");
    CFrameWnd* pWnd = GetParentFrame();
    pWnd->SetWindowText(ss);
    Index = 3;
    Invalidate();
}

void CPyramidView::OnPyramidSetCamera()
{
    CString ss = _T("Усечённая пирамида - положение камеры по умолчанию");
    CFrameWnd* pWnd = GetParentFrame();
    pWnd->SetWindowText(ss);

    PView(0) = 10;
    PView(1) = 315;
    PView(2) = 45;

    Index = 4;
    Invalidate();
}

void CPyramidView::OnCameraFromFile()
{
    CFileDialog dlg(TRUE, _T("txt"), _T("camera.txt"),
        OFN_FILEMUSTEXIST | OFN_HIDEREADONLY,
        _T("Text Files (*.txt)|*.txt|All Files (*.*)|*.*||"));

    if (dlg.DoModal() == IDOK)
    {
        CString fileName = dlg.GetPathName();
        std::ifstream file(fileName);

        if (file.is_open())
        {
            double r, fi, q;

            if (!(file >> r >> fi >> q))
            {
                file.close();
                MessageBox(_T("Ошибка чтения данных из файла!\nФормат: r fi q (через пробел)"),
                    _T("Ошибка"), MB_OK | MB_ICONERROR);
                return;
            }
            file.close();

            CString errorMsg;
            bool valid = true;

            if (r < 10 || r > 200)
            {
                errorMsg += _T("• Радиус r должен быть от 10 до 200\n");
                valid = false;
            }
            if (fi < -180 || fi > 180)
            {
                errorMsg += _T("• Азимут φ (fi) должен быть от -180 до 180\n");
                valid = false;
            }
            if (q < 0 || q > 180)
            {
                errorMsg += _T("• Угол θ (q) должен быть от 0 до 180\n");
                valid = false;
            }

            if (!valid)
            {
                errorMsg = _T("Некорректные значения в файле:\n\n") + errorMsg +
                    _T("\nТекущие значения:\n") +
                    _T("r=") + CString(std::to_wstring(r).c_str()) +
                    _T(", φ=") + CString(std::to_wstring(fi).c_str()) +
                    _T(", θ=") + CString(std::to_wstring(q).c_str());
                MessageBox(errorMsg, _T("Ошибка валидации"), MB_OK | MB_ICONERROR);
                return;
            }

            PView(0) = r;
            PView(1) = fi;
            PView(2) = q;

            if (Index > 0)
                Invalidate();

            CString successMsg;
            successMsg.Format(_T("Координаты камеры загружены:\nr=%.1f, φ=%.1f°, θ=%.1f°"), r, fi, q);
            MessageBox(successMsg, _T("Успех"), MB_OK | MB_ICONINFORMATION);
        }
        else
        {
            MessageBox(_T("Не удалось открыть файл!"), _T("Ошибка"), MB_OK | MB_ICONERROR);
        }
    }
}

void CPyramidView::OnCameraFromDialog()
{
    CCameraDialog dlg;
    dlg.m_r = PView(0);
    dlg.m_fi = PView(1);
    dlg.m_q = PView(2);

    if (dlg.DoModal() == IDOK)
    {
        PView(0) = dlg.m_r;
        PView(1) = dlg.m_fi;
        PView(2) = dlg.m_q;

        if (Index > 0)
            Invalidate();
    }
}

void CPyramidView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    if (nChar == 'L')
    {
        CFileDialog dlg(TRUE, _T("txt"), _T("vertices.txt"),
            OFN_FILEMUSTEXIST | OFN_HIDEREADONLY,
            _T("Text Files (*.txt)|*.txt|All Files (*.*)|*.*||"));

        if (dlg.DoModal() == IDOK)
        {
            if (PIR.LoadFromFile(dlg.GetPathName())) {
                MessageBox(_T("Вершины успешно загружены!"), _T("Успех"), MB_OK);
                Invalidate();
            }
            else {
                MessageBox(_T("Ошибка чтения файла!"), _T("Ошибка"), MB_ICONERROR);
            }
        }
        return;
    }

    if (Index >= 1 && Index < 4)
    {
        switch (nChar)
        {
        case VK_UP:
        {
            double d = PView(2) + 5;
            if (d <= 180)
                PView(2) = d;
            break;
        }
        case VK_DOWN:
        {
            double d = PView(2) - 5;
            if (d >= 0)
                PView(2) = d;
            break;
        }
        case VK_LEFT:
        {
            double d = PView(1) - 10;
            if (d < -180)
                d += 360;
            PView(1) = d;
            break;
        }
        case VK_RIGHT:
        {
            double d = PView(1) + 10;
            if (d > 180)
                d -= 360;
            PView(1) = d;
            break;
        }
        case VK_PRIOR:
        {
            double d = PView(0) - 5;
            if (d >= 10)
                PView(0) = d;
            break;
        }
        case VK_NEXT:
        {
            double d = PView(0) + 5;
            if (d <= 200)
                PView(0) = d;
            break;
        }
        }
        Invalidate();
    }
    CWnd::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CPyramidView::OnSize(UINT nType, int cx, int cy)
{
    CWnd::OnSize(nType, cx, cy);
    WinRect.SetRect(50, 50, cx - 50, cy - 50);
}

BOOL CPyramidView::OnPreparePrinting(CPrintInfo* pInfo)
{
    return DoPreparePrinting(pInfo);
}

void CPyramidView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
}

void CPyramidView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
}

#ifdef _DEBUG
void CPyramidView::AssertValid() const
{
    CView::AssertValid();
}

void CPyramidView::Dump(CDumpContext& dc) const
{
    CView::Dump(dc);
}

CPyramidDoc* CPyramidView::GetDocument() const
{
    ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CPyramidDoc)));
    return (CPyramidDoc*)m_pDocument;
}
#endif