#pragma once

#include "MainSubDlgWin.h"
#include "common\BtnST.h"
#include "SystemParamShare.h"
// CDlgNaviBar 대화 상자입니다.
class CAuroraMgr;
class CDlgNaviBar : public CMainSubDlgWin
{
	DECLARE_DYNAMIC(CDlgNaviBar)

private:
	CAuroraMgr* m_pAuroraMgr;
	CButtonST	m_btnImage[enumNavigateIDCount];
	int m_iCurrentView;
	void UpdateInitialState(BOOL bEnable);

public:
	CDlgNaviBar(CAuroraMgr *pAuroraMgr, CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~CDlgNaviBar();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DLG_MAIN_NAVI };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnBnClickedButtonLogin();
	afx_msg void OnBnClickedButtonOperate();
	afx_msg void OnBnClickedButtonResult();
	afx_msg void OnBnClickedButtonMaint();
	afx_msg void OnBnClickedButtonRecipe();
	afx_msg void OnBnClickedButtonLog();
	afx_msg void OnBnClickedButtonConfig();
	afx_msg void OnBnClickedButtonAlarm();
	afx_msg void OnBnClickedButtonBuzzerOff();
	afx_msg void OnBnClickedButtonPmMode();
	virtual BOOL OnInitDialog();
	void ChangeCheck(int index);
	afx_msg void OnBnClickedCheckManual();
};
