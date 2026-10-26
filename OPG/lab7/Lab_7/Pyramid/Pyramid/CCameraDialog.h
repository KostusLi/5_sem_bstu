#pragma once
#include "afxdialogex.h"


class CCameraDialog : public CDialogEx
{
	DECLARE_DYNAMIC(CCameraDialog)

public:
	CCameraDialog(CWnd* pParent = nullptr);
	virtual ~CCameraDialog();

#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_CAMERA_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);

	DECLARE_MESSAGE_MAP()

public:
	double m_r;
	double m_fi;
	double m_q;

	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedOk();
};