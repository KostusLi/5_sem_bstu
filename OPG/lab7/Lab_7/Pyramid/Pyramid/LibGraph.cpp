#include "pch.h"

CRectD::CRectD()
{
	left = top = right = bottom = 0;
}

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

	M(0, 0) = kx;
	M(0, 1) = 0;
	M(0, 2) = (double)RW.left - kx * RS.left;
	M(1, 0) = 0;
	M(1, 1) = -ky;
	M(1, 2) = (double)RW.bottom + ky * RS.bottom;
	M(2, 0) = 0;
	M(2, 1) = 0;
	M(2, 2) = 1;

	return M;
}

void SetMyMode(CDC& dc, CRect& RS, CRect& RW)
{
	int dsx = RS.right - RS.left;
	int dsy = RS.top - RS.bottom;
	int xsL = RS.left;
	int ysL = RS.bottom;
	int dwx = RW.right - RW.left;
	int dwy = RW.bottom - RW.top;
	int xwL = RW.left;
	int ywH = RW.bottom;

	dc.SetMapMode(MM_ANISOTROPIC);
	dc.SetWindowExt(dsx, dsy);
	dc.SetViewportExt(dwx, -dwy);
	dc.SetWindowOrg(xsL, ysL);
	dc.SetViewportOrg(xwL, ywH);
}

CMatrix CreateTranslate2D(double dx, double dy)
{
	CMatrix TM(3, 3);
	TM(0, 0) = 1;
	TM(0, 2) = dx;
	TM(1, 1) = 1;
	TM(1, 2) = dy;
	TM(2, 2) = 1;

	return TM;
}

CMatrix CreateTranslate3D(double dx, double dy, double dz)
{
	CMatrix TM(4, 4);

	for (int i = 0; i < 4; i++)
	{
		TM(i, i) = 1;
	}

	TM(0, 3) = dx;
	TM(1, 3) = dy;
	TM(2, 3) = dz;

	return TM;
}

CMatrix CreateRotate2D(double fi)
{
	double fg = fmod(fi, 360.0);
	double ff = (fg / 180.0) * M_PI;
	CMatrix RM(3, 3);
	RM(0, 0) = cos(ff);
	RM(0, 1) = -sin(ff);
	RM(1, 0) = sin(ff);
	RM(1, 1) = cos(ff);
	RM(2, 2) = 1;

	return RM;
}

CMatrix CreateRotate3DZ(double fi)
{
	double fg = fmod(fi, 360.0);
	double ff = (fg / 180.0) * M_PI;
	CMatrix RM(4, 4);
	RM(0, 0) = cos(ff);
	RM(0, 1) = -sin(ff);
	RM(1, 0) = sin(ff);
	RM(1, 1) = cos(ff);
	RM(2, 2) = 1;
	RM(3, 3) = 1;

	return RM;
}

CMatrix CreateRotate3DX(double fi)
{
	double fg = fmod(fi, 360.0);
	double ff = (fg / 180.0) * M_PI;
	CMatrix RM(4, 4);
	RM(0, 0) = 1;
	RM(1, 1) = cos(ff);
	RM(1, 2) = -sin(ff);
	RM(2, 1) = sin(ff);
	RM(2, 2) = cos(ff);
	RM(3, 3) = 1;

	return RM;
}

CMatrix CreateRotate3DY(double fi)
{
	double fg = fmod(fi, 360.0);
	double ff = (fg / 180.0) * M_PI;
	CMatrix RM(4, 4);
	RM(0, 0) = cos(ff);
	RM(0, 2) = sin(ff);
	RM(1, 1) = 1;
	RM(2, 0) = -sin(ff);
	RM(2, 2) = cos(ff);
	RM(3, 3) = 1;

	return RM;
}

CMatrix VectorMult(CMatrix& V1, CMatrix& V2)
{
	int b1 = (V1.cols() == 1) && (V1.rows() == 3);
	int b2 = (V2.cols() == 1) && (V2.rows() == 3);
	int b = b1 && b2;

	if (!b)
	{
		TCHAR* error = _T("VectorMult: неправильные размерности векторов! ");
		MessageBox(NULL, error, _T("Ошибка"), MB_ICONSTOP);
		exit(1);
	}

	CMatrix W(3);
	W(0) = V1(1) * V2(2) - V1(2) * V2(1);
	W(1) = -(V1(0) * V2(2) - V1(2) * V2(0));
	W(2) = V1(0) * V2(1) - V1(1) * V2(0);

	return W;
}

double ScalarMult(CMatrix& V1, CMatrix& V2)
{
	int b1 = (V1.cols() == 1) && (V1.rows() == 3);
	int b2 = (V2.cols() == 1) && (V2.rows() == 3);
	int b = b1 && b2;

	if (!b)
	{
		TCHAR* error = _T("ScalarMult: неправильные размерности векторов! ");
		MessageBox(NULL, error, _T("Ошибка"), MB_ICONSTOP);
		exit(1);
	}

	double p = V1(0) * V2(0) + V1(1) * V2(1) + V1(2) * V2(2);

	return p;
}

double ModVec(CMatrix& V)
{
	int b = (V.cols() == 1) && (V.rows() == 3);

	if (!b)
	{
		TCHAR* error = _T("ModVec: неправильная размерность вектора! ");
		MessageBox(NULL, error, _T("Ошибка"), MB_ICONSTOP);
		exit(1);
	}

	double q = sqrt(V(0) * V(0) + V(1) * V(1) + V(2) * V(2));

	return q;
}

double CosV1V2(CMatrix& V1, CMatrix& V2)
{
	double modV1 = ModVec(V1);
	double modV2 = ModVec(V2);
	int b = (modV1 < 1e-7) || (modV2 < 1e-7);

	if (b)
	{
		TCHAR* error = _T("CosV1V2: модуль одного или обоих векторов < 1e-7! ");
		MessageBox(NULL, error, _T("Ошибка"), MB_ICONSTOP);
		exit(1);
	}

	int b1 = (V1.cols() == 1) && (V1.rows() == 3);
	int b2 = (V2.cols() == 1) && (V2.rows() == 3);
	b = b1 && b2;

	if (!b)
	{
		TCHAR* error = _T("CosV1V2: неправильные размерности векторов! ");
		MessageBox(NULL, error, _T("Ошибка"), MB_ICONSTOP);
		exit(1);
	}

	double cos_f = ScalarMult(V1, V2) / (modV1 * modV2);

	return cos_f;
}

double AngleV1V2(CMatrix& V1, CMatrix& V2)
{
	double modV1 = ModVec(V1);
	double modV2 = ModVec(V2);
	int b = (modV1 < 1e-7) || (modV2 < 1e-7);

	if (b)
	{
		TCHAR* error = _T("AngleV1V2: модуль одного или обоих векторов < 1e-7! ");
		MessageBox(NULL, error, _T("Ошибка"), MB_ICONSTOP);
		exit(1);
	}

	int b1 = (V1.cols() == 1) && (V1.rows() == 3);
	int b2 = (V2.cols() == 1) && (V2.rows() == 3);
	b = b1 && b2;

	if (!b)
	{
		TCHAR* error = _T("AngleV1V2: неправильные размерности векторов! ");
		MessageBox(NULL, error, _T("Ошибка"), MB_ICONSTOP);
		exit(1);
	}

	double cos_f = ScalarMult(V1, V2) / (modV1 * modV2);

	if (fabs(cos_f) > 1)
	{
		TCHAR* error = _T("AngleV1V2: модуль cos(f)>1! ");
		MessageBox(NULL, error, _T("Ошибка"), MB_ICONSTOP);
		exit(1);
	}

	double f;

	if (cos_f > 0)
	{
		f = acos(cos_f);
	}
	else
	{
		f = M_PI - acos(cos_f);
	}

	double fg = (f / M_PI) * 180;

	return fg;
}

CMatrix CreateViewCoord(double r, double fi, double q)
{
	double fg = fmod(fi, 360.0);
	double ff = (fg / 180.0) * M_PI;
	fg = fmod(q, 360.0);
	double qq = (fg / 180.0) * M_PI;
	CMatrix VM(4, 4);
	VM(0, 0) = -sin(ff);
	VM(0, 1) = cos(ff);
	VM(1, 0) = -cos(qq) * cos(ff);
	VM(1, 1) = -cos(qq) * sin(ff);
	VM(1, 2) = sin(qq);
	VM(2, 0) = -sin(qq) * cos(ff);
	VM(2, 1) = -sin(qq) * sin(ff);
	VM(2, 2) = -cos(qq);
	VM(2, 3) = r;
	VM(3, 3) = 1;

	return VM;
}

CMatrix SphereToCart(CMatrix& PView)
{
	CMatrix R(3);
	double r = PView(0);
	double fi = PView(1);
	double q = PView(2);
	double fi_rad = (fi / 180.0) * M_PI;
	double q_rad = (q / 180.0) * M_PI;
	R(0) = r * sin(q_rad) * cos(fi_rad);
	R(1) = r * sin(q_rad) * sin(fi_rad);
	R(2) = r * cos(q_rad);

	return R;
}

void GetProjection(CRectD& RS, CMatrix& Data, CMatrix& PView, CRectD& PR)
{
	double Zmax = Data.MaxElement();
	double Zmin = Data.MinElement();
	CMatrix PS(4, 4);
	PS(3, 0) = 1;
	PS(3, 1) = 1;
	PS(3, 2) = 1;
	PS(3, 3) = 1;
	CMatrix MV = CreateViewCoord(PView(0), PView(1), PView(2));
	PS(0, 0) = RS.left;
	PS(1, 0) = RS.top;
	PS(2, 0) = Zmax;
	PS(0, 1) = RS.left;
	PS(1, 1) = RS.bottom;
	PS(2, 1) = Zmax;
	PS(0, 2) = RS.right;
	PS(1, 2) = RS.top;
	PS(2, 2) = Zmax;
	PS(0, 3) = RS.right;
	PS(1, 3) = RS.bottom;
	PS(2, 3) = Zmax;
	CMatrix Q = MV * PS;
	CMatrix V = Q.GetRow(0);
	double Xmin = V.MinElement();
	double Xmax = V.MaxElement();
	V = Q.GetRow(1);
	double Ymax = V.MaxElement();
	PS(2, 0) = Zmin;
	PS(2, 1) = Zmin;
	PS(2, 2) = Zmin;
	PS(2, 3) = Zmin;
	Q = MV * PS;
	V = Q.GetRow(1);
	double Ymin = V.MinElement();
	PR.SetRectD(Xmin, Ymax, Xmax, Ymin);
}