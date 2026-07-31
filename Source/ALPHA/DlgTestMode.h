// DlgTestMode.h
// LYS 2026-07-02 Test Mode dialog (ported from GMI IDD_TEST_MODE_DLG)
// GMI option: 2Gantry 4Head, REAR STAGE only
#pragma once

#include "resource.h"
#include "common\MFCGridControl\GridCtrl_src\GridCtrl.h"
#include "AuroraMgrPool.h"

class CAuroraMgr;

class CDlgTestMode : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgTestMode)

public:
	CDlgTestMode(CAuroraMgr* pAuroraMgr, CWnd* pParent = NULL);
	CDlgTestMode(stAuroraMgrPool* pMgrPool, CWnd* pParent = NULL);
	virtual ~CDlgTestMode();

	enum { IDD = IDD_DLG_TEST_MODE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()

private:
	CAuroraMgr* m_pAuroraMgr;
	stAuroraMgrPool* m_pMgrPool;

	// Mode: 0=Review, 1=WAD, 2=Vision, 3=Angle
	int m_nTestMode;

	// Grid
	CGridCtrl m_gridGlass;
	int m_nSelRow;
	int m_nSelCol;
	COLORREF m_crPrevBk;
	int m_nMaxStick;
	int m_nMaxCell;

	// Radio buttons (dynamic)
	CButton m_rdMode[4];
	CButton m_rdStage;

	// Action buttons (dynamic)
	CButton m_btnMove;
	CButton m_btnPreContact;
	CButton m_btnContact;
	CButton m_btnUnContact;
	CButton m_btnClose;

	// Status label
	CStatic m_stcStatus;

	// Internal accessors
	CMaintControlMgr* GetMaintCtrl();
	CRecipeEditMgr*   GetRecipeEdit();
	CPositionConfigMgr* GetPosConfig();

	void CreateControls();
	void PopulateCellGrid();
	int  ManualMoveToCell();
	int  ManualMoveStick(int iStick);

	afx_msg void OnSelMode();
	afx_msg void OnBtnMove();
	afx_msg void OnBtnPreContact();
	afx_msg void OnBtnContact();
	afx_msg void OnBtnUnContact();
	afx_msg void OnGridClick(NMHDR* pNMHDR, LRESULT* pResult);
};
