#pragma once

#include "MainSubDlgWin.h"


// CDlgSubManual 대화 상자
class CAuroraMgr;
class CDlgSubManual_STEP;
class CDlgSubManual_Meas;
class CDlgSubManual_Ionizer;
class CDlgSubManual_AgingTest;
class CDlgSubManual_AgingSet;
class CDlgSubManual_AutoContact;
class CDlgSubManual_MultiMeas;	// LYS 2026-07-23 WAD Multi Meas
class CDlgSubManual : public CMainSubDlgWin
{
	DECLARE_DYNAMIC(CDlgSubManual)

private:
	CAuroraMgr* m_pAuroraMgr;

	CDlgSubManual_STEP*			 m_pDlgSub_Step;
	CDlgSubManual_Meas*			 m_pDlgSub_Meas;
	CDlgSubManual_Ionizer*		 m_pDlgSub_Ionizer;
	CDlgSubManual_AgingTest*	 m_pDlgSub_AgingTest;
	CDlgSubManual_AgingSet*		 m_pDlgSub_AgingSet;
	CDlgSubManual_AutoContact*	 m_pDlgSub_AutoContact;
	CDlgSubManual_MultiMeas*	 m_pDlgSub_MultiMeas;	// LYS 2026-07-23 WAD Multi Meas

public:
	CDlgSubManual(CAuroraMgr* pAuroraMgr, CWnd* pParent = nullptr);   // 표준 생성자입니다.
	virtual ~CDlgSubManual();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DLG_SUB_MANUAL };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	int m_iCurFileView;
	void RefreshFileView(BOOL bShow);
	afx_msg void OnBnClickedButtonManualStep();
	afx_msg void OnBnClickedButtonManualSacc();
	afx_msg void OnBnClickedButtonManualIonizer();
	afx_msg void OnBnClickedButtonManualAgingTest();
	afx_msg void OnBnClickedButtonManual5();
	afx_msg void OnBnClickedButtonManualAgingSet();
	afx_msg void OnBnClickedButtonManualWadMulti();	// LYS 2026-07-23 WAD Multi Meas
protected:
	afx_msg LRESULT OnContrilMsgEvent(WPARAM wParam, LPARAM lParam);
};
