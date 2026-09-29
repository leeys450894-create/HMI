// DlgSubManual_AgingNew.h
// LYS 2026-09-28 EFA: EFA(2-pass 에이징) 검사 화면.
// OCP_EFA A3_ICP CDlgNaviManualAgingNew(IDD_DLG_NAVI_MANUAL_AGING_NEW, "EFA TEST") 이식.
// 구조는 CDlgSubManual_AgingTest에서 복사 - docs/AgingNew_EFA_Porting_Plan.md 참조.
// /utf-8 없이 CP949로 컴파일되므로 사용자 메시지는 영어로 둔다.
#pragma once
#include "afxwin.h"
#include "DlgSubManual_STEP.h"

using namespace std;

#define MAX_EFA_STICK			10
#define MAX_EFA_ITEM			7	// AgingTest와 같은 리스트 그리드: No | Aging | PTN | SEC | WAD | VISION | ANGLE
#define EFA_FIXED_ITEM_COUNT	2	// OCP_EFA: "EFA 리스트는 2행 고정" (T0 = 에이징 전, T1 = 에이징 후)

class CAuroraMgr;

class CDlgSubManual_AgingNew : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgSubManual_AgingNew)
	enum { IDD = IDD_DLG_SUB_MANUAL_AGING_NEW };

private:
	CLabel		m_LabelInfo;

public:
	CDlgSubManual_AgingNew(CAuroraMgr* pAuroraMgr, CWnd* pParent = NULL);
	virtual ~CDlgSubManual_AgingNew();

public:
	vector <int>			m_nStick;
	vector <S_AgingTest>	m_vecAginList;

protected:
	CMFCButton		m_chkStop;
	CMFCButton		m_chkStick[MAX_EFA_STICK];
	CMySpreadDll*	m_spreadAgingList;
	CMySpreadDll*	m_spreadAgingMeas;
	CComboBox		m_cmbCycle;
	CString			m_strEfaRcpPath;	// 레지스트리 DIR\EFA_RCP (AgingTest의 DIR\AGING_RCP와 분리 - OCP는 실수로 키를 같이 쓴다)
	CAuroraMgr*		m_pAuroraMgr;
	HFONT			m_hfont;
	int				m_nHlRow;			// 마지막으로 하이라이트한 리스트 셀 (그리드 좌표)
	int				m_nHlCol;

public:
	static CString GetEfaRcpPath();		// 끝에 백슬래시가 붙은 "{DIR\EFA_RCP}\"

	void UpdateStatus(BOOL bEnable);
	void UpdateListNum();
	BOOL IsOfflineStatus();
	BOOL LoadAgingInfo();
	void SaveAgingInfo();
	void AgingItemClearAll(BOOL bFixed2 = FALSE);	// TRUE: 고정 2행 재구성 (OCP AgingItemClearAll(TRUE))
	void AgingItemAdd();
	void UpdateAgingCell();
	int	 GetMeasUse(int iMeas, int nMeasUse);
	void MeasCellSave();
	void MeasCellLoad();
	afx_msg LRESULT OnSpreadDBClick(WPARAM wParam, LPARAM lParam);

protected:
	CString GetRecipeID();
	CString GetEfaRcpFile();			// "{path}{RecipeID}.rcp", 레시피가 없으면 빈 문자열
	void DblClickFpspreadCellStatus(long Col, long Row);
	void SheetChangedFsMeasArea(short OldSheet, short NewSheet);

protected:
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnStop();
	afx_msg void OnRcpOpen();
	afx_msg void OnItemClearAll();
	afx_msg void OnItemAdd();
	afx_msg void OnItemDel();
	afx_msg void OnItemSave();
	afx_msg void OnLoadAgingInfo();
	afx_msg void OnAgingProcStrart();
	afx_msg void OnTimer(UINT_PTR nIDEvent);

	DECLARE_MESSAGE_MAP()
	DECLARE_EVENTSINK_MAP()
};
