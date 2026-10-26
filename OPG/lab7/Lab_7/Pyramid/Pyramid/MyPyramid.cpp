#include "pch.h"
#include <fstream>


CPyramid::CPyramid()
{
    Vertices.RedimMatrix(4, 6);
    NormalBottom.RedimMatrix(3);
    NormalTop.RedimMatrix(3);

    Vertices(0, 0) = 15;
    Vertices(1, 0) = 0;
    Vertices(2, 0) = 0;

    Vertices(0, 1) = -7.5;
    Vertices(1, 1) = 13;
    Vertices(2, 1) = 0;

    Vertices(0, 2) = -7.5;
    Vertices(1, 2) = -13;
    Vertices(2, 2) = 0;

    Vertices(0, 3) = 7.5;
    Vertices(1, 3) = 0;
    Vertices(2, 3) = 20;

    Vertices(0, 4) = -3.75;
    Vertices(1, 4) = 6.5;
    Vertices(2, 4) = 20;

    Vertices(0, 5) = -3.75;
    Vertices(1, 5) = -6.5;
    Vertices(2, 5) = 20;

    for (int i = 0; i < 6; i++)
        Vertices(3, i) = 1;

    NormalBottom(0) = 0;  NormalBottom(1) = 0;  NormalBottom(2) = -1;
    NormalTop(0) = 0;     NormalTop(1) = 0;     NormalTop(2) = 1;
}

void CPyramid::Draw(CDC& dc, CMatrix& PView, CRect& RW)
{
    CMatrix ViewCart = SphereToCart(PView);
    CMatrix MV = CreateViewCoord(PView(0), PView(1), PView(2));
    CMatrix ViewVert = MV * Vertices;

    CRectD RectView;
    GetRect(ViewVert, RectView);
    CMatrix MW = SpaceToWindow(RectView, RW);

    CPoint MasVert[6];
    CMatrix V(3);
    V(2) = 1;

    for (int i = 0; i < 6; i++)
    {
        V(0) = ViewVert(0, i);
        V(1) = ViewVert(1, i);
        V = MW * V;
        MasVert[i].x = (int)V(0);
        MasVert[i].y = (int)V(1);
    }

    double smBottom = ScalarMult(NormalBottom, ViewCart);
    double smTop = ScalarMult(NormalTop, ViewCart);

    bool sideVisible[3];
    for (int i = 0; i < 3; i++)
    {
        int next = (i + 1) % 3;

        CMatrix V1 = Vertices.GetCol(i, 0, 2);
        CMatrix V2 = Vertices.GetCol(next, 0, 2);
        CMatrix V3 = Vertices.GetCol(i + 3, 0, 2);

        CMatrix Edge1 = V2 - V1;
        CMatrix Edge2 = V3 - V1;
        CMatrix Normal = VectorMult(Edge1, Edge2);

        double sm = ScalarMult(Normal, ViewCart);
        sideVisible[i] = (sm > 0);
    }

    CPen PenSolid(PS_SOLID, 2, RGB(0, 0, 0));
    CPen PenDashed(PS_DASH, 1, RGB(128, 128, 128));
    CPen* pOldPen;

    for (int i = 0; i < 3; i++)
    {
        int next = (i + 1) % 3;

        bool edgeVisible = (smBottom > 0) || sideVisible[i];

        pOldPen = dc.SelectObject(edgeVisible ? &PenSolid : &PenDashed);
        dc.MoveTo(MasVert[i]);
        dc.LineTo(MasVert[next]);
        dc.SelectObject(pOldPen);
    }

    for (int i = 0; i < 3; i++)
    {
        int next = (i + 1) % 3;

        bool edgeVisible = (smTop > 0) || sideVisible[i];

        pOldPen = dc.SelectObject(edgeVisible ? &PenSolid : &PenDashed);
        dc.MoveTo(MasVert[i + 3]);
        dc.LineTo(MasVert[next + 3]);
        dc.SelectObject(pOldPen);
    }

    for (int i = 0; i < 3; i++)
    {
        int prev = (i + 2) % 3;
        bool edgeVisible = sideVisible[i] || sideVisible[prev];

        pOldPen = dc.SelectObject(edgeVisible ? &PenSolid : &PenDashed);
        dc.MoveTo(MasVert[i]);
        dc.LineTo(MasVert[i + 3]);
        dc.SelectObject(pOldPen);
    }
}

void CPyramid::Draw1(CDC& dc, CMatrix& PView, CRect& RW)
{
    CMatrix ViewCart = SphereToCart(PView);
    CMatrix MV = CreateViewCoord(PView(0), PView(1), PView(2));
    CMatrix ViewVert = MV * Vertices;

    CRectD RectView;
    GetRect(ViewVert, RectView);
    CMatrix MW = SpaceToWindow(RectView, RW);

    CPoint MasVert[6];
    CMatrix V(3);
    V(2) = 1;

    for (int i = 0; i < 6; i++)
    {
        V(0) = ViewVert(0, i);
        V(1) = ViewVert(1, i);
        V = MW * V;
        MasVert[i].x = (int)V(0);
        MasVert[i].y = (int)V(1);
    }

    struct FaceInfo {
        int index;
        double depth;
        bool visible;
    };

    FaceInfo faces[3];

    for (int i = 0; i < 3; i++)
    {
        int next = (i + 1) % 3;

        CMatrix V1 = Vertices.GetCol(i, 0, 2);
        CMatrix V2 = Vertices.GetCol(next, 0, 2);
        CMatrix V3 = Vertices.GetCol(i + 3, 0, 2);

        CMatrix Edge1 = V2 - V1;
        CMatrix Edge2 = V3 - V1;
        CMatrix Normal = VectorMult(Edge1, Edge2);

        double sm = ScalarMult(Normal, ViewCart);

        double avgZ = (ViewVert(2, i) + ViewVert(2, next) +
            ViewVert(2, next + 3) + ViewVert(2, i + 3)) / 4.0;

        faces[i].index = i;
        faces[i].depth = avgZ;
        faces[i].visible = (sm > 0);
    }

    for (int i = 0; i < 2; i++) {
        for (int j = i + 1; j < 3; j++) {
            if (faces[i].depth > faces[j].depth) {
                FaceInfo temp = faces[i];
                faces[i] = faces[j];
                faces[j] = temp;
            }
        }
    }

    double smBottom = ScalarMult(NormalBottom, ViewCart);
    double smTop = ScalarMult(NormalTop, ViewCart);

    CPen Pen(PS_SOLID, 2, RGB(0, 0, 0));
    CPen* pOldPen = dc.SelectObject(&Pen);

    if (smBottom > 0)
    {
        CBrush BrushBottom(RGB(90, 110, 130));
        CBrush* pOldBrush = dc.SelectObject(&BrushBottom);
        CPoint BottomVert[3] = { MasVert[0], MasVert[1], MasVert[2] };
        dc.Polygon(BottomVert, 3);
        dc.SelectObject(pOldBrush);
    }

    if (smTop > 0)
    {
        CBrush BrushTop(RGB(180, 220, 180));
        CBrush* pOldBrush = dc.SelectObject(&BrushTop);
        CPoint TopVert[3] = { MasVert[3], MasVert[4], MasVert[5] };
        dc.Polygon(TopVert, 3);
        dc.SelectObject(pOldBrush);
    }

    for (int f = 0; f < 3; f++)
    {
        int i = faces[f].index;

        if (faces[f].visible)
        {
            int next = (i + 1) % 3;

            CBrush Brush(RGB(160, 140, 190));
            CBrush* pOldBrush = dc.SelectObject(&Brush);

            CPoint FaceVert[4];
            FaceVert[0] = MasVert[i];
            FaceVert[1] = MasVert[next];
            FaceVert[2] = MasVert[next + 3];
            FaceVert[3] = MasVert[i + 3];

            dc.Polygon(FaceVert, 4);
            dc.SelectObject(pOldBrush);
        }
    }

    dc.SelectObject(pOldPen);
}

void CPyramid::DrawCurrent(CDC& dc, CMatrix& PView, CRect& RW)
{
    Draw1(dc, PView, RW);
}

void CPyramid::DrawCamera(CDC& dc, CMatrix& PView, CRect& RW)
{
    Draw1(dc, PView, RW);
}

void CPyramid::GetRect(CMatrix& Vert, CRectD& RectView)
{
    CMatrix V = Vert.GetRow(0);
    double xMin = V.MinElement();
    double xMax = V.MaxElement();

    V = Vert.GetRow(1);
    double yMin = V.MinElement();
    double yMax = V.MaxElement();

    RectView.SetRectD(xMin, yMax, xMax, yMin);
}


bool CPyramid::LoadFromFile(CString fileName)
{
    std::ifstream file(fileName);
    if (!file.is_open())
        return false;

    for (int i = 0; i < 6; i++)
    {
        file >> Vertices(0, i) >> Vertices(1, i) >> Vertices(2, i);
        Vertices(3, i) = 1;
    }

    file.close();
    return true;
}