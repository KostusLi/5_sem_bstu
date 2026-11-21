#pragma once
double cosv1v2(CMatrix& one, CMatrix& two);

class CPyramid
{
private:
	CMatrix Vertices;
	void GetRect(CMatrix& Vert, CRectD& RectView);
	void DrawPolygonShaded(CDC& dc, CPoint* pts, int count, CMatrix& normal,
		CMatrix& center, CMatrix& viewCart, CMatrix& lightCart, COLORREF color);

public:
	CPyramid();
	void Draw(CDC& dc, CMatrix& P, CRect& RW);
	void ColorDraw(CDC& dc, CMatrix& PView, CMatrix& PLight, CRect& RW, COLORREF color);
};

CPyramid::CPyramid()
{
	Vertices.RedimMatrix(4, 6);
	Vertices(0, 0) = 3; Vertices(0, 1) = 0;  Vertices(0, 2) = 0; Vertices(0, 3) = 1; Vertices(0, 4) = 0; Vertices(0, 5) = 0;
	Vertices(1, 0) = 0; Vertices(1, 1) = 0;  Vertices(1, 2) = 3; Vertices(1, 3) = 0; Vertices(1, 4) = 0; Vertices(1, 5) = 1;
	Vertices(2, 0) = 0; Vertices(2, 1) = 0;  Vertices(2, 2) = 0; Vertices(2, 3) = 6; Vertices(2, 4) = 6; Vertices(2, 5) = 6;
	Vertices(3, 0) = 1; Vertices(3, 1) = 1;  Vertices(3, 2) = 1; Vertices(3, 3) = 1; Vertices(3, 4) = 1; Vertices(3, 5) = 1;
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

void CPyramid::DrawPolygonShaded(CDC& dc, CPoint* pts, int count, CMatrix& normal,
	CMatrix& center, CMatrix& viewCart, CMatrix& lightCart, COLORREF color)
{
	if (cosv1v2(viewCart, normal) > 0)
	{
		CMatrix toLight = lightCart - center;
		double cosLight = cosv1v2(toLight, normal);

		COLORREF faceColor = RGB(0, 0, 0);
		if (cosLight > 0)
		{
			BYTE r = (BYTE)(cosLight * GetRValue(color));
			BYTE g = (BYTE)(cosLight * GetGValue(color));
			BYTE b = (BYTE)(cosLight * GetBValue(color));
			faceColor = RGB(r, g, b);
		}

		CBrush brush(faceColor);
		CPen pen(PS_SOLID, 1, RGB(80, 80, 80));
		CBrush* pOldBrush = dc.SelectObject(&brush);
		CPen* pOldPen = dc.SelectObject(&pen);

		dc.Polygon(pts, count);

		dc.SelectObject(pOldPen);
		dc.SelectObject(pOldBrush);
	}
}

void CPyramid::ColorDraw(CDC& dc, CMatrix& PView, CMatrix& PLight, CRect& RW, COLORREF color)
{
	if (RW.Width() <= 0 || RW.Height() <= 0)
		return;

	CMatrix ViewCart = SphereToCart(PView);
	CMatrix LightCart = SphereToCart(PLight);

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

	for (int i = 0; i < 3; i++)
	{
		int k = (i == 2) ? 0 : i + 1;

		CMatrix R1 = Vertices.GetCol(i, 0, 2);
		CMatrix R2 = Vertices.GetCol(k, 0, 2);
		CMatrix VE1 = Vertices.GetCol(i + 3, 0, 2);
		CMatrix VE2 = Vertices.GetCol(k + 3, 0, 2);

		CMatrix V1 = R2 - R1;
		CMatrix V2 = VE1 - R1;
		CMatrix VN = VectorMult(V2, V1);

		CMatrix center(3);
		for (int j = 0; j < 3; j++)
			center(j) = (R1(j) + R2(j) + VE1(j) + VE2(j)) / 4.0;

		CPoint quad[4] = { MasVert[i], MasVert[k], MasVert[k + 3], MasVert[i + 3] };
		DrawPolygonShaded(dc, quad, 4, VN, center, ViewCart, LightCart, color);
	}

	{
		CMatrix A = Vertices.GetCol(0, 0, 2);
		CMatrix B = Vertices.GetCol(1, 0, 2);
		CMatrix C = Vertices.GetCol(2, 0, 2);
		CMatrix V1 = B - A;
		CMatrix V2 = C - A;
		CMatrix VN = VectorMult(V1, V2);

		CMatrix center(3);
		for (int j = 0; j < 3; j++)
			center(j) = (A(j) + B(j) + C(j)) / 3.0;

		CPoint tri[3] = { MasVert[0], MasVert[1], MasVert[2] };
		DrawPolygonShaded(dc, tri, 3, VN, center, ViewCart, LightCart, color);
	}

	{
		CMatrix A1 = Vertices.GetCol(3, 0, 2);
		CMatrix B1 = Vertices.GetCol(4, 0, 2);
		CMatrix C1 = Vertices.GetCol(5, 0, 2);
		CMatrix V1 = B1 - A1;
		CMatrix V2 = C1 - A1;
		CMatrix VN = VectorMult(V2, V1);

		CMatrix center(3);
		for (int j = 0; j < 3; j++)
			center(j) = (A1(j) + B1(j) + C1(j)) / 3.0;

		CPoint tri[3] = { MasVert[3], MasVert[4], MasVert[5] };
		DrawPolygonShaded(dc, tri, 3, VN, center, ViewCart, LightCart, color);
	}
}

double cosv1v2(CMatrix& V1, CMatrix& V2)
{
	double modV1 = ModVec(V1);
	double modV2 = ModVec(V2);
	if (modV1 < 1e-7 || modV2 < 1e-7)
		return 0.0;
	return ScalarMult(V1, V2) / (modV1 * modV2);
}