#pragma once
#ifndef LIBSURFACE
#define LIBSURFACE 1
using namespace std;

typedef vector<CMatrix> CVecMatrix;
typedef vector<CVecMatrix> CMasMatrix;

typedef vector<CPoint> CVecPoint;
typedef vector<CVecPoint> CMatrPoint;

double Function1(double x, double y);
double Function2(double x, double y);
double Function3(double x, double y);

class CPlot3D
{
	pfunc2 pFunc;
	CRectD SpaceRect;
	CMasMatrix MatrF;
	CMasMatrix MatrView;
	CRectD ViewRect;
	CRect WinRect;
	CMatrix ViewPoint;

	CMatrPoint MatrWindow;


public:
	CPlot3D();
	~CPlot3D() { pFunc = NULL; };
	void SetFunction(pfunc2 pF, CRectD RS, double dx, double dy);
	void SetViewPoint(double r, double fi, double q);
	CMatrix GetViewPoint();
	void SetWinRect(CRect Rect);
	void CreateMatrF(double dx, double dy);
	void SetMatrF(CMasMatrix& Matr);
	void CreateMatrView();
	void CreateMatrWindow();
	int GetNumberRegion();
	void Draw(CDC& dc);
};

#endif