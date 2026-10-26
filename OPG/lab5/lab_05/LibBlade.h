#ifndef LIBPLANETS
#define LIBPLANETS 1
const double pi = 3.14159;


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
	CRectD(double l, double t, double r, double b);
	void SetRectD(double l, double t, double r, double b);
	CSizeD SizeD();
};

CMatrix CreateTranslate2D(double dx, double dy);
CMatrix CreateRotate2D(double fi);
CMatrix SpaceToWindow(CRectD& rs, CRect& rw);
void SetMyMode(CDC& dc, CRectD& RS, CRect& RW);



class CBlade
{
	CRect MainPoint;
	CRect FirstTop;
	CRect SecondTop;
	CRect FirstBootom;
	CRect SecondBootom;
	CRect WayRotation;
	CMatrix FTCoords;
	CMatrix STCoords;
	CMatrix FBCoords;
	CMatrix SBCoords;
	CMatrix FTHCoords;
	CMatrix STHCoords;
	CMatrix FBHCoords;
	CMatrix SBHCoords;
	CMatrix FHGCoords;
	CMatrix SHGCoords;
	CRect RW;
	CRectD RS;
	double wPoint;
	double fiSB;
	double fiFB;
	double fiST;
	double fiFT;
	double fiHSB;
	double fiHFB;
	double fiHST;
	double fiHFT;

	double dt;
public:
	CBlade();
	void DrawTriangle(CMatrix FTCoords, CMatrix STCoords, CDC& dc, bool color);
	void SetDT(double dtx) { dt = dtx; };
	void SetNewCoords();
	void GetRS(CRectD& RSX);
	CRect GetRW() { return RW; };
	void Draw(CDC& dc);
};


#endif