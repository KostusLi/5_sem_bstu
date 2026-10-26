#include "stdafx.h"

CRectD::CRectD(double l, double t, double r, double b)
{
	left = l;
	top = t;
	right = r;
	bottom = b;
}

void CRectD::SetRectD(double l, double t, double r, double b)
{
	left = l;
	top = t;
	right = r;
	bottom = b;
}

CSizeD CRectD::SizeD()
{
	CSizeD cz;
	cz.cx = fabs(right - left);
	cz.cy = fabs(top - bottom);
	return cz;
}

CMatrix CreateTranslate2D(double dx, double dy)
{
	CMatrix TM(3, 3);
	TM(0, 0) = 1; TM(0, 2) = dx;
	TM(1, 1) = 1;  TM(1, 2) = dy;
	TM(2, 2) = 1;
	return TM;
}

CMatrix CreateRotate2D(double fi)
{
	double fg = fmod(fi, 360.0);
	double ff = (fg / 180.0) * pi;
	CMatrix RM(3, 3);
	RM(0, 0) = cos(ff); RM(0, 1) = -sin(ff);
	RM(1, 0) = sin(ff);  RM(1, 1) = cos(ff);
	RM(2, 2) = 1;
	return RM;
}

CMatrix SpaceToWindow(CRectD& RS, CRect& RW)
{
	CMatrix M(3, 3);
	CSize sz = RW.Size();
	int dwx = sz.cx;
	int dwy = sz.cy;
	CSizeD szd = RS.SizeD();

	double dsx = szd.cx;
	double dsy = szd.cy;

	double kx = (double)dwx / dsx;
	double ky = (double)dwy / dsy;

	M(0, 0) = kx;  M(0, 1) = 0;    M(0, 2) = (double)RW.left - kx * RS.left;
	M(1, 0) = 0;   M(1, 1) = -ky;  M(1, 2) = (double)RW.bottom + ky * RS.bottom;
	M(2, 0) = 0;   M(2, 1) = 0;    M(2, 2) = 1;
	return M;
}

void SetMyMode(CDC& dc, CRectD& RS, CRect& RW)
{
	double dsx = RS.right - RS.left;
	double dsy = RS.top - RS.bottom;
	double xsL = RS.left;
	double ysL = RS.bottom;

	int dwx = RW.right - RW.left;
	int dwy = RW.bottom - RW.top;
	int xwL = RW.left;
	int ywH = RW.bottom;

	dc.SetMapMode(MM_ANISOTROPIC);
	dc.SetWindowExt((int)dsx, (int)dsy);
	dc.SetViewportExt(dwx, -dwy);
	dc.SetWindowOrg((int)xsL, (int)ysL);
	dc.SetViewportOrg(xwL, ywH);
}

CBlade::CBlade()
{
	double rS = 30;
	double RoE = 10 * rS;
	double d = RoE;
	RS.SetRectD(-d, d, d, -d);
	RW.SetRect(0, 0, 690, 640);
	MainPoint.SetRect(-rS, rS, rS, -rS);
	FirstTop.SetRect(-5, 5, 5, -5);
	SecondTop.SetRect(-5, 5, 5, -5);
	FirstBootom.SetRect(-5, 5, 5, -5);
	SecondBootom.SetRect(-5, 5, 5, -5);
	WayRotation.SetRect(-RoE, RoE, RoE, -RoE);
	fiSB = 215;
	fiFB = 205;
	fiST = 25;
	fiFT = 35;
	fiHSB = 5;
	fiHFB = 355;
	fiHST = 175;
	fiHFT = 185;
	wPoint = -8;
	dt = 0.1;
	FTCoords.RedimMatrix(3);
	STCoords.RedimMatrix(3);
	FBCoords.RedimMatrix(3);
	SBCoords.RedimMatrix(3);
	FTHCoords.RedimMatrix(3);
	STHCoords.RedimMatrix(3);
	FBHCoords.RedimMatrix(3);
	SBHCoords.RedimMatrix(3);
	FHGCoords.RedimMatrix(3);
	SHGCoords.RedimMatrix(3);
}

void CBlade::SetNewCoords()
{
	double RoV = (WayRotation.right - WayRotation.left) / 2;

	CMatrix BasePoint(3);
	BasePoint(0) = RoV;
	BasePoint(1) = 0;
	BasePoint(2) = 1;

	fiFT += wPoint * dt;

	FTCoords = CreateRotate2D(fiFT + 10) * BasePoint;
	STCoords = CreateRotate2D(fiFT - 10) * BasePoint;

	FBCoords = CreateRotate2D(fiFT + 190) * BasePoint;
	SBCoords = CreateRotate2D(fiFT + 170) * BasePoint;

	FTHCoords = CreateRotate2D(fiFT + 100) * BasePoint;
	STHCoords = CreateRotate2D(fiFT + 80) * BasePoint;

	FBHCoords = CreateRotate2D(fiFT + 280) * BasePoint;
	SBHCoords = CreateRotate2D(fiFT + 260) * BasePoint;

	FHGCoords = CreateRotate2D(fiFT + 50) * BasePoint;
	SHGCoords = CreateRotate2D(fiFT + 70) * BasePoint;
}


void CBlade::Draw(CDC& dc)
{
	CBrush SBrush, EBrush, MBrush, VBrush, * pOldBrush;
	CRect R;

	SBrush.CreateSolidBrush(RGB(0, 0, 255));
	EBrush.CreateSolidBrush(RGB(0, 0, 255));
	MBrush.CreateSolidBrush(RGB(0, 255, 0));
	VBrush.CreateSolidBrush(RGB(0, 128, 0));


	dc.MoveTo(0, 0);
	dc.LineTo(FTCoords(0), FTCoords(1));
	DrawTriangle(FTCoords, STCoords, dc, true);
	DrawTriangle(FBCoords, SBCoords, dc, true);
	DrawTriangle(FTHCoords, STHCoords, dc, false);
	DrawTriangle(FBHCoords, SBHCoords, dc, false);
	DrawTriangle(FHGCoords, SHGCoords, dc, false);
	dc.SelectObject(&VBrush);
	dc.Ellipse(MainPoint);

	pOldBrush = dc.SelectObject(&SBrush);

	dc.SelectObject(pOldBrush);
}
void CBlade::DrawTriangle(CMatrix FTCoords, CMatrix STCoords, CDC& dc, bool color)
{

	if (color) {
		CPen Pen(PS_SOLID, 1, RGB(255, 0, 0));
		dc.SelectObject(&Pen);

		CBrush Brush(RGB(255, 0, 0));
		dc.SelectObject(&Brush);

		CPoint points[3];
		points[0] = CPoint(0, 0);
		points[1] = CPoint(FTCoords(0), FTCoords(1));
		points[2] = CPoint(STCoords(0), STCoords(1));

		dc.Polygon(points, 3);
	}
	else {
		CPen Pen(PS_SOLID, 1, RGB(0, 0, 255));
		dc.SelectObject(&Pen);

		CBrush Brush(RGB(0, 0, 255));
		dc.SelectObject(&Brush);

		CPoint points[3];
		points[0] = CPoint(0, 0);
		points[1] = CPoint(FTCoords(0), FTCoords(1));
		points[2] = CPoint(STCoords(0), STCoords(1));

		dc.Polygon(points, 3);
	}

}


void CBlade::GetRS(CRectD& RSX)
{
	RSX.left = RS.left;
	RSX.top = RS.top;
	RSX.right = RS.right;
	RSX.bottom = RS.bottom;
}