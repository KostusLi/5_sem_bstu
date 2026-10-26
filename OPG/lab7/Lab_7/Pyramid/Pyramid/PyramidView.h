#pragma once


class CPyramidView : public CView
{
protected:
	CPyramidView() noexcept;
	DECLARE_DYNCREATE(CPyramidView)

public:
	CPyramidDoc* GetDocument() const;
	CPyramid PIR;
	CRect WinRect;
	CMatrix PView;
	int Index;


public:

public:
	virtual void OnDraw(CDC* pDC);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);

public:
	virtual ~CPyramidView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

protected:
	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnPyramidCurrent();
	afx_msg void OnPyramidWithRemoval();
	afx_msg void OnPyramidWithoutRemoval();
	afx_msg void OnPyramidSetCamera();
	afx_msg void OnCameraFromFile();
	afx_msg void OnCameraFromDialog();
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnSize(UINT nType, int cx, int cy);
};

#ifndef _DEBUG
inline CPyramidDoc* CPyramidView::GetDocument() const
{
	return reinterpret_cast<CPyramidDoc*>(m_pDocument);
}
#endif