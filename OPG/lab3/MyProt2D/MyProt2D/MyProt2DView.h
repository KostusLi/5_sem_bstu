#pragma once
enum class ViewMode { None, GraphF1, GraphF3, Shapes };

class CMyProt2DView : public CView
{
protected:
	CMyProt2DView() noexcept;
	DECLARE_DYNCREATE(CMyProt2DView)

public:
	int Index;
	int N_Bezier;
	CMatrix X, Y;
	CRect RW;
	CRectD RS;
	CPlot2D Graph;
	CMyPen PenLine,
		PenAxis;
	bool m_ShowShapes = false;

public:
	double MyF1(double x);
	double MyF2(double x);

public:
	virtual void OnDraw(CDC* pDC);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

public:
	virtual ~CMyProt2DView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	void OnTestsF1();
	void OnTestsF2();
	void OnTestsF3();

	DECLARE_MESSAGE_MAP()
};