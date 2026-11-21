#pragma once
class CMyApp : public CWinApp
{
public:
	virtual BOOL InitInstance()
	{
		m_pMainWnd = new CMainWnd();
		ASSERT(m_pMainWnd);
		m_pMainWnd->ShowWindow(m_nCmdShow);
		m_pMainWnd->UpdateWindow();
		return TRUE;
	}
};