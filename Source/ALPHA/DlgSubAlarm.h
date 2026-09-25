#pragma once

#include "MainSubDlgWin.h"

// CDlgSubAlarm 대화 상자입니다.
class CAuroraMgr;
class CDlgSubAlarm : public CMainSubDlgWin
{
	DECLARE_DYNAMIC(CDlgSubAlarm)

private:
	CAuroraMgr					*m_pAuroraMgr;

public:
	virtual void ShowReportAlarm() override;

public:
	CDlgSubAlarm(CAuroraMgr *pAuroraMgr, CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~CDlgSubAlarm();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DLG_SUB_ALARM };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	LRESULT	OnAlarmOccured(WPARAM wp, LPARAM lp);
	LRESULT	OnAlarmCleared(WPARAM wp, LPARAM lp);
	// LYS 2026-09-22 경(Light) 알람 - SetLightAlarm 은 WM_WARNING_ACCURED/CLEARED 를 쓰는데
	//                받는 핸들러가 없어 부저가 안 울렸다(Heavy 만 WM_ALARM_ACCURED 로 부저를 켠다).
	LRESULT	OnWarningOccured(WPARAM wp, LPARAM lp);
	LRESULT	OnWarningCleared(WPARAM wp, LPARAM lp);
	afx_msg void OnBnClickedButtonReload();
	afx_msg void OnBnClickedButtonAlarmSearch();
	afx_msg void OnBnClickedButtonAlarm();
	CString BoolToString(BOOL bItem);
	void CIMAlarmReportInfo(int nAlarmID);
};
