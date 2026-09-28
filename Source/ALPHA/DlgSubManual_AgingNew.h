// DlgSubManual_AgingNew.h
// LYS 2026-09-28 EFA: EFA (2-pass aging) test view.
// Port of OCP_EFA A3_ICP CDlgNaviManualAgingNew (IDD_DLG_NAVI_MANUAL_AGING_NEW, "EFA TEST").
// Structure copied from CDlgSubManual_AgingTest - see docs/AgingNew_EFA_Porting_Plan.md.
// ASCII only on purpose: compiled as CP949 without /utf-8, so user messages stay English.
#pragma once
#include "afxwin.h"
#include "DlgSubManual_STEP.h"

using namespace std;

#define MAX_EFA_STICK			10
#define MAX_EFA_ITEM			7	// same list grid as AgingTest: No | Aging | PTN | SEC | WAD | VISION | ANGLE
#define EFA_FIXED_ITEM_COUNT	2	// OCP_EFA: "EFA list is fixed to 2 rows" (T0 = before aging, T1 = after aging)

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
	CString			m_strEfaRcpPath;	// registry DIR\EFA_RCP (separate from AgingTest DIR\AGING_RCP - OCP shares the key by mistake)
	CAuroraMgr*		m_pAuroraMgr;
	HFONT			m_hfont;
	int				m_nHlRow;			// last highlighted list cell (grid coords)
	int				m_nHlCol;

public:
	static CString GetEfaRcpPath();		// "{DIR\EFA_RCP}\" with trailing backslash

	void UpdateStatus(BOOL bEnable);
	void UpdateListNum();
	BOOL IsOfflineStatus();
	BOOL LoadAgingInfo();
	void SaveAgingInfo();
	void AgingItemClearAll(BOOL bFixed2 = FALSE);	// TRUE: rebuild the 2 fixed rows (OCP AgingItemClearAll(TRUE))
	void AgingItemAdd();
	void UpdateAgingCell();
	int	 GetMeasUse(int iMeas, int nMeasUse);
	void MeasCellSave();
	void MeasCellLoad();
	afx_msg LRESULT OnSpreadDBClick(WPARAM wParam, LPARAM lParam);

protected:
	CString GetRecipeID();
	CString GetEfaRcpFile();			// "{path}{RecipeID}.rcp", empty if no recipe
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
