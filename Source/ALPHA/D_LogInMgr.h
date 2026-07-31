#pragma once

#include "common\Label.h"
#include "common\XStringHelper.h"
#include "common\MFCGridControl\GridCtrl_src\GridCtrl.h"
#include "common\MFCGridControl\GridCtrl_src\GridCell.h"

// CD_LogInMgr 대화 상자입니다.

class CLogInControlMgr;
class CD_LogInMgr : public CDialog
{
	DECLARE_DYNAMIC(CD_LogInMgr)

	CXStingHelper	m_strHelper;
	CLogInControlMgr		*m_pLoginMgr;
	CFont			m_font;
	CLabel			m_labelLoginTitle;
	CGridCtrl		m_gridctrl;
	CString			m_strId;
	CString			m_strPW;
	int				m_nUserLv;
	long			m_nCurSel;

protected:

	void InitLoginTitle();

	void InitListControl();
	void InitListLogInfo();
	void UpdateLoginInfo();

	void CheckLogin();

	void OnLogin();
	void OnLogout();

	void EnableControl(BOOL);
	void EnableClose();

public:
	bool IsLoginStatus();

public:
	CD_LogInMgr(CLogInControlMgr *login, CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~CD_LogInMgr();

// 대화 상자 데이터입니다.
	enum { IDD = IDD_POPUP_LOGIN };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	afx_msg void OnBnClickedLoginControl();
	afx_msg void OnGridItemActivateList(NMHDR* pNotifyStruct, LRESULT* result);
	DECLARE_MESSAGE_MAP()
public:
	int m_nMaxUser;
	bool m_bLoginCheck;
};
