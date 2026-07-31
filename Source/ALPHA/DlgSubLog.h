#pragma once

#include "MainSubDlgWin.h"
#include "WindowEventHandler.h"


// CDlgSubLog 대화 상자입니다.
class CAuroraMgr;
class CDlgSubLogFile;
class CDlgSubLog : public CMainSubDlgWin
{
	DECLARE_DYNAMIC(CDlgSubLog)

private:
	CAuroraMgr*			m_pAuroraMgr;
	CDlgSubLogFile*		m_pMainLoggingFile[enumLogItemIDCount];
	int					m_iCurLogFileView;

	void RefreshLogFileView(BOOL bShow);

public:
	CDlgSubLog(CAuroraMgr *pAuroraMgr, CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~CDlgSubLog();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DLG_SUB_LOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnBnClickedButtonFileOpen();
	afx_msg void OnBnClickedButtonDlog();
	afx_msg void OnBnClickedButtonOlog();
	afx_msg void OnBnClickedButtonTlog();
	afx_msg void OnBnClickedButtonMeas1();
	afx_msg void OnBnClickedButtonMeas2();
	afx_msg void OnBnClickedButtonMeas3();
	afx_msg void OnBnClickedButtonMeas4();
	afx_msg void OnBnClickedButtonMeas5();
	afx_msg void OnBnClickedButtonMeas6();
	afx_msg void OnBnClickedButtonMeas7();
	afx_msg void OnBnClickedButtonMeas8();
	afx_msg void OnBnClickedButtonMeas9();
	afx_msg void OnBnClickedButtonMeas10();
	afx_msg void OnBnClickedButtonMCC();
	afx_msg void OnBnClickedButtonMotion();
	afx_msg void OnBnClickedButtonIo();
	afx_msg void OnBnClickedButtonSerial();
	afx_msg void OnBnClickedButtonMcc();
	afx_msg void OnBnClickedButtonProbe();
	afx_msg void OnBnClickedButtonCim();
};
