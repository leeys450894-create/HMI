#pragma once

#define DF_INFO_PARM_COL			2
#define DF_INFO_PARM_ITEM_MAX		4
									
#define DF_STATUS_PARM_COL			2
#define DF_STATUS_PARM_ITEM_MAX		3

#define DF_MODE_ONLY_QUALITY		0
#define DF_MODE_ONLY_COLOR			1
#define DF_MODE_SYNC				2

class CPGControlDlg : public C_BaseDialog
{
	enum { IDD = IDD_PGCONTROL_DIALOG };

	public:
		CPGControlDlg(CWnd* pParent = nullptr);	// 표준 생성자입니다.
		virtual ~CPGControlDlg();

	protected:
		HICON		m_hIcon;
		CListBox	m_lstBox;
		C_GridCtrl	m_gridInfo;
		C_GridCtrl	m_gridStatus;
		CMFCButton	m_btnRunProc;
		C_ButtonEx	m_rdProcMode[3];

	public:
		int			m_nAutoMode;
		bool		m_bQualityRun;
		bool		m_bColorPGOn;

	public:
		void		WriteLog(CString strMsg);
		void		InitGridInfo();
		void		InitGridStatus();
		void		DisplayConnectState();
		
	protected:
		virtual	void		OnCancel();
		virtual void		DoDataExchange(CDataExchange* pDX);	// DDX/DDV 지원입니다.
		virtual BOOL		OnInitDialog();
		afx_msg void		OnDestroy();
		afx_msg void		OnPaint();
		afx_msg HCURSOR		OnQueryDragIcon();
		afx_msg void		OnSize(UINT nType, int cx, int cy);
		afx_msg void		OnTimer(UINT_PTR nIDEvent);

		afx_msg void		OnBtnBeginPG();
		afx_msg void		OnSelProcMode(UINT nID);

	DECLARE_MESSAGE_MAP()
};

extern C_WaitDlg* theWaitDlg;