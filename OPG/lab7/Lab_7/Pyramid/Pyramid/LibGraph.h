#pragma once
#include "pch.h"

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

	CRectD();
	CRectD(double l, double t, double r, double b);
	void SetRectD(double l, double t, double r, double b);
	CSizeD SizeD();
};

CMatrix SpaceToWindow(CRectD& rs, CRect& rw);

void SetMyMode(CDC& dc, CRect& RS, CRect& RW);

CMatrix CreateTranslate2D(double dx, double dy);

CMatrix CreateTranslate3D(double dx, double dy, double dz);

CMatrix CreateRotate2D(double fi);

CMatrix CreateRotate3DZ(double fi);

CMatrix CreateRotate3DX(double fi);

CMatrix CreateRotate3DY(double fi);

CMatrix CreateViewCoord(double r, double fi, double q);

CMatrix VectorMult(CMatrix& V1, CMatrix& V2);

double ScalarMult(CMatrix& V1, CMatrix& V2);

double ModVec(CMatrix& V);

double CosV1V2(CMatrix& V1, CMatrix& V2);

double AngleV1V2(CMatrix& V1, CMatrix& V2);

CMatrix SphereToCart(CMatrix& PView);

void GetProjection(CRectD& RS, CMatrix& Data, CMatrix& PView, CRectD& PR);