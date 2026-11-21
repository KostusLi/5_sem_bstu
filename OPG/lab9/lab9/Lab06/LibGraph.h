#pragma once
#include <math.h>
const double pi = 3.14159265358979323846;

struct CSizeD
{
	double cx;
	double cy;
};

struct CRectD
{
	double left;
	double top;
	double right;
	double bottom;
	CRectD() { left = top = right = bottom = 0; };
	CRectD(double l, double t, double r, double b) : left(l), top(t), right(r), bottom(b) {}
	void SetRectD(double l, double t, double r, double b)
	{
		left = l; top = t; right = r; bottom = b;
	}
	CSizeD SizeD()
	{
		CSizeD cz;
		cz.cx = fabs(right - left);
		cz.cy = fabs(top - bottom);
		return cz;
	}
};

CMatrix SphereToCart(CMatrix& PView)
{
	CMatrix R(3);
	double r = PView(0);
	double fi_rad = (PView(1) / 180.0) * pi;
	double q_rad = (PView(2) / 180.0) * pi;

	R(0) = r * sin(q_rad) * cos(fi_rad);
	R(1) = r * sin(q_rad) * sin(fi_rad);
	R(2) = r * cos(q_rad);

	return R;
}

CMatrix CreateViewCoord(double r, double fi, double q)
{
	double fg = fmod(fi, 360.0);
	double ff = (fg / 180.0) * pi;
	fg = fmod(q, 360.0);
	double qq = (fg / 180.0) * pi;

	CMatrix VM(4, 4);
	VM(0, 0) = -sin(ff);            VM(0, 1) = cos(ff);             VM(0, 2) = 0;        VM(0, 3) = 0;
	VM(1, 0) = -cos(qq) * cos(ff);  VM(1, 1) = -cos(qq) * sin(ff);  VM(1, 2) = sin(qq);  VM(1, 3) = 0;
	VM(2, 0) = -sin(qq) * cos(ff);  VM(2, 1) = -sin(qq) * sin(ff);  VM(2, 2) = -cos(qq); VM(2, 3) = r;
	VM(3, 0) = 0;                   VM(3, 1) = 0;                   VM(3, 2) = 0;        VM(3, 3) = 1;
	return VM;
}

CMatrix SpaceToWindow(CRectD& RS, CRect& RW)
{
	CMatrix M(3, 3);
	CSize sz = RW.Size();
	int dwx = sz.cx;
	int dwy = sz.cy;
	CSizeD szd = RS.SizeD();

	double dsx = (szd.cx > 1e-6) ? szd.cx : 1.0;
	double dsy = (szd.cy > 1e-6) ? szd.cy : 1.0;

	double kx = (double)dwx / dsx;
	double ky = (double)dwy / dsy;
	double k = (kx < ky) ? kx : ky;

	double cxW = (RW.left + RW.right) / 2.0;
	double cyW = (RW.top + RW.bottom) / 2.0;
	double cxS = (RS.left + RS.right) / 2.0;
	double cyS = (RS.top + RS.bottom) / 2.0;

	M(0, 0) = k;  M(0, 1) = 0;   M(0, 2) = cxW - k * cxS;
	M(1, 0) = 0;  M(1, 1) = -k;  M(1, 2) = cyW + k * cyS;
	M(2, 0) = 0;  M(2, 1) = 0;   M(2, 2) = 1;

	return M;
}

CMatrix VectorMult(CMatrix& V1, CMatrix& V2)
{
	CMatrix temp(3);
	temp(0) = (V1(1) * V2(2)) - (V1(2) * V2(1));
	temp(1) = (V1(2) * V2(0)) - (V1(0) * V2(2));
	temp(2) = (V1(0) * V2(1)) - (V1(1) * V2(0));
	return temp;
}

double ScalarMult(CMatrix& V1, CMatrix& V2)
{
	return V1(0) * V2(0) + V1(1) * V2(1) + V1(2) * V2(2);
}

double ModVec(CMatrix& V)
{
	return sqrt(V(0) * V(0) + V(1) * V(1) + V(2) * V(2));
}