#pragma once
#include "afxwin.h"
#include "DlgSubManual_STEP.h"
// DlgSubManual_AgingTest 대화 상자

using namespace std;

#define MAX_AGING_STICK	10
#define MAX_AGING_ITEM	7	// LYS 2026-08-06 Phase 2: +ANGLE col (OCP 7 w/ LCR)

class CAuroraMgr;	// LYS 2026-08-06 Phase 1

class CDlgSubManual_AgingTest : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgSubManual_AgingTest)
	enum { IDD = IDD_DLG_SUB_MANUAL_AGING_TEST };

private:
	CLabel		m_LabelInfo;

public:
	CDlgSubManual_AgingTest(CAuroraMgr* pAuroraMgr, CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~CDlgSubManual_AgingTest();

public:
	//VTM_SYSTEM* m_pSystem;	//To Use Shared Memory Pointer
	vector <int>			m_nStick;
	vector <S_AgingTest>	m_vecAginList;

protected:
	CMFCButton		m_chkStop;
	CMFCButton		m_chkStick[MAX_AGING_STICK];
	//CString			m_strOldRcp;
	CMySpreadDll*	m_spreadAgingList;
	CMySpreadDll*	m_spreadAgingMeas;
	CComboBox		m_cmbCycle;
	CComboBox		m_cmbSeg;			// LYS 2026-09-29 Hybrid 3b A13 segment selector (shown only for hybrid recipes)
	int				m_nViewSeg;			// segment shown in the cell grid (0 for 1-segment recipes)
	int				m_nMaskMismatch;	// rows with a wrong cell count found by the last MeasCellLoad
	CString			m_strViewSegRcp;	// recipe the combo was built for
	CString			m_strAgingRcpPath;
	CAuroraMgr*		m_pAuroraMgr;	// LYS 2026-08-06 Phase 1
	HFONT			m_hfont;
	int			m_nHlRow;	// LYS 2026-08-06 E4: last highlighted cell (grid coords)
	int			m_nHlCol;
public:
	void UpdateStatus(BOOL bEnable);
	void UpdateListNum();
	BOOL IsOfflineStatus();
	BOOL LoadAgingInfo();
	void SaveAgingInfo();
	void AgingItemClearAll();
	void AgingItemAdd();
	void UpdateAgingCell();
	int	 GetMeasUse(int iMeas, int nMeasUse);
	BOOL MeasCellSave();				// LYS 2026-09-29 Hybrid 3b A13 FALSE = not written (cancelled / recipe changed)
	void MeasCellLoad();
	BOOL IsSegmented();					// LYS 2026-09-29 Hybrid 3b A13 recipe has more than 1 segment
	BOOL IsHybridRunBusy();				// hybrid recipe and a measure run / cycle test in progress
	void UpdateSegmentCombo();
	BOOL IsMaskViewStale(CString strRcpID);	// grid built for another recipe or not the recipe's size -> popup + TRUE (LYS 2026-09-30 every recipe)
	BOOL ConfirmMaskOverwrite();			// LYS 2026-09-30 mismatched rows shown unchecked -> Yes/No before saving
	afx_msg LRESULT OnSpreadDBClick(WPARAM wParam, LPARAM lParam);

protected:
	void DblClickFpspreadCellStatus(long Col, long Row);
	void SheetChangedFsMeasArea(short OldSheet, short NewSheet);

protected:
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnStop();
	afx_msg void OnRcpOpen();
	afx_msg void OnItemClearAll();
	afx_msg void OnItemAdd();
	afx_msg void OnItemDel();
	afx_msg void OnItemSave();
	afx_msg void OnLoadAgingInfo();
	afx_msg void OnAgingProcStrart();
	afx_msg void OnSelchangeSeg();		// LYS 2026-09-29 Hybrid 3b A13
	afx_msg void OnTimer(UINT_PTR nIDEvent);	// LYS 2026-08-06 E4: aging progress highlight

	DECLARE_MESSAGE_MAP()
	DECLARE_EVENTSINK_MAP()
};
