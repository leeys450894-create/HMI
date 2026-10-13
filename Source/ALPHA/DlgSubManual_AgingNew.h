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
	CString		m_strTitleBase;		// LYS 2026-10-12 Hybrid Phase 6 rc title ("EFA Test View")
	CString		m_strTitleShown;	// title on screen now (suffix only while a hybrid EFA run measures)

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
	CString			m_strViewRcp;		// LYS 2026-09-30 recipe the cell grid was built for (empty = default grid)
	int				m_nMaskMismatch;	// LYS 2026-09-30 rows with a wrong cell count found by the last MeasCellLoad
	CComboBox		m_cmbSeg;			// LYS 2026-09-30 Hybrid 3c A13 segment selector (hybrid recipes only, hidden otherwise)
	int				m_nViewSeg;			// LYS 2026-09-30 Hybrid 3c A13 segment shown / saved (0 for 1-segment recipes), m_strViewRcp = its recipe stamp
	int				m_nSegLostTick;		// LYS 2026-09-30 Hybrid 3c A13 (S2 fix) consecutive idle timer ticks that saw a 1-segment recipe (hide on 2)

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
	BOOL MeasCellSave();				// LYS 2026-09-30 Hybrid 3c A13 FALSE = not written (stale / overwrite declined)
	void MeasCellLoad();
	BOOL IsMaskViewStale();			// LYS 2026-09-30 grid not built for the current recipe -> popup + TRUE
	BOOL ConfirmMaskOverwrite();		// LYS 2026-09-30 mismatched rows -> Yes/No before saving
	BOOL IsSegmented();				// LYS 2026-09-30 Hybrid 3c A13 recipe has more than 1 segment
	BOOL IsHybridRunBusy();			// LYS 2026-09-30 Hybrid 3c A13 engine running / cycle state (raw flags, no IsSegmented precondition)
	void UpdateSegmentCombo();		// LYS 2026-09-30 Hybrid 3c A13 fill / hide the selector, reset or clamp m_nViewSeg
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
	afx_msg void OnSelchangeSeg();		// LYS 2026-09-30 Hybrid 3c A13
	afx_msg void OnTimer(UINT_PTR nIDEvent);

	DECLARE_MESSAGE_MAP()
	DECLARE_EVENTSINK_MAP()
};
