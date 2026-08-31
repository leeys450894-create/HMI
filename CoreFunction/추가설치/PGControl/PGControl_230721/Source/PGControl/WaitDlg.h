#pragma once

typedef	BOOL	(*WaitFunc) ();

class	C_WaitDlg : public CDialogEx
{
	DECLARE_DYNAMIC (C_WaitDlg)

	protected:
		CMFCButton	m_btnCancel;
	
	public:
		WaitFunc	m_pWaitFunc;
	
	public:
		CWinThread* m_pProcThread;

	public:
		C_WaitDlg (CWnd *pParent = NULL);
		void	SetWaitFunc (WaitFunc pWaitFunc);
		virtual ~C_WaitDlg ();

	public:
		BOOL	m_bUserStop;

	protected:
		virtual void	DoDataExchange (CDataExchange *pDX);    // DDX/DDV 지원입니다.
		virtual BOOL	OnInitDialog ();
		virtual	void	OnOK () {};
		virtual	void	OnCancel ();
		afx_msg void	OnDestroy ();
		afx_msg void	OnSize (UINT nType, int cx, int cy);
		afx_msg	LRESULT	OnRun (WPARAM wParam, LPARAM lParam);

		afx_msg LRESULT	OnWaitCancel (WPARAM wParam, LPARAM lParam);
		afx_msg LRESULT	OnWaitOK (WPARAM wParam, LPARAM lParam);

		DECLARE_MESSAGE_MAP ()
};

