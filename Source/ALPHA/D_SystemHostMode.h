#pragma once
#include "common\Label.h"

// CD_SystemHostMode 대화 상자
class CAuroraMgr;
class CD_SystemHostMode : public CDialogEx
{
	DECLARE_DYNAMIC(CD_SystemHostMode)

public:
	CD_SystemHostMode(CAuroraMgr* pAuroraMgr, CWnd* pParent = nullptr);   // 표준 생성자입니다.
	virtual ~CD_SystemHostMode();
	CLabel			m_LabelInfo;
	CAuroraMgr* m_pAuroraMgr;

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_POPUP_SYSTEM_HOSTMODE };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedSystemHostmodeOffline();
	afx_msg void OnBnClickedSystemHostmodeOnline();
	afx_msg void OnBnClickedCtbuttonctrlShtterOpen();
	afx_msg void OnBnClickedCtbuttonctrlShtterClose();
	afx_msg void OnBnClickedOk();
	int m_nState;
	void CheckHostMode(bool bEnable);
	bool CheckValid();
};
