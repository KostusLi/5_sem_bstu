#pragma once
#include "pch.h"

class CPyramid
{
private:
    CMatrix Vertices;
    CMatrix NormalBottom;
    CMatrix NormalTop;

    void GetRect(CMatrix& Vert, CRectD& RectView);

public:
    CPyramid();
    bool LoadFromFile(CString fileName);
    void Draw(CDC& dc, CMatrix& PView, CRect& RW);
    void Draw1(CDC& dc, CMatrix& PView, CRect& RW);
    void DrawCurrent(CDC& dc, CMatrix& PView, CRect& RW);
    void DrawCamera(CDC& dc, CMatrix& PView, CRect& RW);
};