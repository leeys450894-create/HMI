#pragma once

#include "resource.h"
#include "common\Label.h"

#define      MAIN_OPERATE_SUB_SYSTEM_INITIALIZE_TIMER_ID			5001
#define      MAIN_OPERATE_SUB_SYSTEM_SACC_TIMER_ID					5011
#define      MAIN_OPERATE_SUB_SYSTEM_AGING_TIMER_ID					5012

// CD_MainOperateInitializing 대화 상자입니다.

class CD_MainOperateInitializing : public CDialogEx
{
	DECLARE_DYNAMIC(CD_MainOperateInitializing)

private:
	CLabel			m_stclabelTitle;
	CProgressCtrl	m_prgInitState;
	CStatic			m_stcPercent;
	bool			m_bEngineBreak;
	int				m_nAgingTime;
	int				m_nTimeCount;
	bool			m_nInitShow;

public:
	void InitStart(int iType = 0);
	void InitStop();
	void SetAlignTime(int nTime) { m_nAgingTime = nTime;  }

public:
	CD_MainOperateInitializing(CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~CD_MainOperateInitializing();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_BOX_MAIN_OPERATE_SYSTEM_INITIALIZE };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnDestroy();
};
