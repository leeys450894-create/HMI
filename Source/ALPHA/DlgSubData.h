#pragma once

#include "MainSubDlgWin.h"
#include "common\Label.h"

// CDlgSubData 대화 상자
class CAuroraMgr;
class CDlgSubData : public CMainSubDlgWin
{
	DECLARE_DYNAMIC(CDlgSubData)

private:
	CAuroraMgr* m_pAuroraMgr;
	CLabel		m_LabelInfo;

public:
	CDlgSubData(CAuroraMgr* pAuroraMgr, CWnd* pParent = nullptr);   // 표준 생성자입니다.
	virtual ~CDlgSubData();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DLG_SUB_DATA };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedButtonExportSeApd();
	afx_msg void OnBnClickedButtonSendApdToFtp();
	afx_msg void OnBnClickedButtonSendRcplistToFtp();
};
