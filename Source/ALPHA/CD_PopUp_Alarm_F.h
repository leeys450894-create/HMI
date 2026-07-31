#pragma once
#include "common\Label.h"
#include "common\MFCGridControl\GridCtrl_src\GridCtrl.h"

// CD_PopUp_Alarm_F 대화 상자
class CMaintControlMgr;
class CD_PopUp_Alarm_F : public CDialogEx
{
	DECLARE_DYNAMIC(CD_PopUp_Alarm_F)

	CMaintControlMgr* m_pMaintControlMgr;

	CLabel		m_LabelInfo;
	CGridCtrl   m_gridList;

public:
	CD_PopUp_Alarm_F(CMaintControlMgr* pMaintControlMgr, CWnd* pParent = nullptr);   // 표준 생성자입니다.
	virtual ~CD_PopUp_Alarm_F();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_PUPUP_ALARM_F };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	void __InitListControl();
	void __InitialListData();
	void __RefreshListData();
	afx_msg void OnDestroy();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
};
