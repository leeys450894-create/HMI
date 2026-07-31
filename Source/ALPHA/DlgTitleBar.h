#pragma once

#include "D_AuroraDlgWin.h"

enum
{
	ID_SYSTEM,
	ID_LOGIN,
	ID_CONNECT,
	ID_LOTID,
	ID_GLASSID,
	ID_RECIPE,
	ID_DATETIME,
	ID_UNKNOWN
};

const long TIMERID_PROGRAM_TIME = 100;
const long TIMERID_IO_STATE_TIME = 200;
const long TIMERID_CONNECT_STATE = 300;

// CDlgTitleBar 대화 상자입니다.
class CAuroraMgr;
class CDlgTitleBar : public CD_AuroraDlgWin
{
	DECLARE_DYNAMIC(CDlgTitleBar)

public:
	CDlgTitleBar(CAuroraMgr *pMgr, CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~CDlgTitleBar();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DLG_MAIN_TITLE };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()

private:
	CAuroraMgr* m_pAuroraMgr;
	CLabel		m_statusInfo[ID_UNKNOWN][20];			// Array of buttons for status
	CGridCtrl   m_gridListIOnfo;

public:
	virtual BOOL OnInitDialog();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
private:
	void SetConnectStatus();
	void SetIOStatus();
public:
	void UpdateTime();
	void RefreshStatus_RecipeID();
	void RefreshStatus_LoginInfo();
	void RefreshStatus_GlassID();
	void RefreshStatus_LotID();

	LRESULT	OnUpdateRecipeInformation(WPARAM wp, LPARAM lp);
	LRESULT OnUpdateLogInState(WPARAM wp, LPARAM lp);
	LRESULT OnUpdateLoginInfo(WPARAM wp, LPARAM lp);
	LRESULT OnUpdateChangeNotInitialize(WPARAM wp, LPARAM lp);
	afx_msg void OnDestroy();
	void __InitListControl();
	void __InitialListData();
	void __RefreshListData();
};
