#pragma once
#include <utility>
#include "CellIdRule.h"

// LYS 2026-10-01 Hybrid Phase 4b Cell ID A/B Auto/Manual editor (docs/HybridGlass_Porting_Plan.md 5.7, ICP C_CellIdEditDlg).
//   Edits a copy of g_inRcpInfo's S_CellIdConfig only; APPLY validates and commits it, CLOSE drops it. Never touches the recipe grids.
//   Modal only: C_BaseDialog::DoModal sets m_bModal, so OnOK / OnCancel end the dialog (a modeless Create would only hide it).
//   Include after pch.h and dfcRecipe.h (IDD_CELL_ID_EDIT_DLG comes from resource.h). Include it from .cpp files only, never from
//   RecipeMgrDlg.h: RecipeManager.cpp includes RecipeMgrDlg.h without pch.h.
//   Caller contract (C_RecipeMgrDlg::OnCellIdEdit, stage 4b-3):
//     AFX_MANAGE_STATE (AfxGetStaticModuleState ());   C_CellIdEditDlg dlg (this);   INT_PTR nRet = dlg.DoModal ();
//     nRet == -1       -> template / window creation failed (not the same as CLOSE)
//     nRet == IDOK     -> g_inRcpInfo.SetCellIdConfig () was called with a validated config (file not saved - SAVE re-validates)
//     nRet == IDCANCEL -> nothing changed (CLOSE / Esc, or APPLY refused because the recipe cells / config changed meanwhile)
class C_CellIdEditDlg : public C_BaseDialog
{
	DECLARE_DYNAMIC (C_CellIdEditDlg)
	public:
		enum { IDD = IDD_CELL_ID_EDIT_DLG };
		C_CellIdEditDlg (CWnd* pParent = NULL);
		virtual ~C_CellIdEditDlg ();
		static CString	FormatError (const S_CellIdError &e);						// Korean, used by RecipeMgrDlg::SaveFile too
		static CString	FormatErrors (const std::vector<S_CellIdError> &vec, int nMax);	// first nMax lines + "... (n more)" line
	protected:
		C_GridCtrl	m_gridCtrlHead, m_gridCtrlData, m_gridStatHead, m_gridStatData, m_gridMap,
					m_gridSelHead, m_gridSelData, m_gridBulkHead, m_gridBulkData;	// IDC_GRID_1 .. 9
		S_CellIdConfig	m_work;
		CCellIdGeo		m_geo;
		CCellIdGrid		m_auto[M_CELLID_PANEL_MAX];	// pure Auto tokens of m_geo, computed in OnInitDialog. Valid while g_inRcpInfo.GetCellIdCfgSeq() == m_nCfgSeq
		int				m_nCfgSeq;					// g_inRcpInfo.GetCellIdCfgSeq() at open: MakeCell APPLY (a modeless popup stays clickable) bumps it -> APPLY refused
		int		m_nSeg, m_nPanel, m_nSelStick, m_nSelCell;
		void	InitControlGrid ();	void InitStatusGrid ();	void InitSelectGrid ();	void InitBulkGrid ();
		void	BuildMap ();		void RefreshView ();	void RefreshSelectGrid ();	void RefreshBulkGrid ();
		void	SetPanelMode (int nMode);
		void	SelectError (const S_CellIdError &e);	// switch segment / panel combos + selection to the error (ICP X6 fix)
		BOOL	GetBulkLineCells (std::vector< std::pair<int, int> > &vecCells, CString &strLine, CString &strError);
		void	ApplyBulkPrefix ();	void ApplyBulkNumbers ();
		CString*	ManualId (int nSeg, int nStick, int nCell);	// current panel; NULL when out of shape (ICP X14 guard)
		int		StickCount (int nSeg);						// m_geo, 0 when out of range
		int		CellCount (int nSeg);
		BOOL	IsManual () { return m_work.m_nMode[m_nPanel] == M_CELLID_MODE_MANUAL; }
		virtual BOOL	OnInitDialog ();
		virtual void	DoDataExchange (CDataExchange* pDX);
		virtual void	OnOK ();	// empty: Enter must not close / apply (APPLY = IDC_BUTTON_1)
		afx_msg void	OnCtrlEndEdit (NMHDR* pNMHDR, LRESULT* pResult);	// GVN_ENDLABELEDIT IDC_GRID_2
		afx_msg void	OnMapClick (NMHDR* pNMHDR, LRESULT* pResult);		// NM_CLICK IDC_GRID_5
		afx_msg void	OnSelEndEdit (NMHDR* pNMHDR, LRESULT* pResult);		// GVN_ENDLABELEDIT IDC_GRID_7
		afx_msg void	OnBulkClick (NMHDR* pNMHDR, LRESULT* pResult);		// NM_CLICK IDC_GRID_9
		afx_msg void	OnBulkEndEdit (NMHDR* pNMHDR, LRESULT* pResult);	// GVN_ENDLABELEDIT IDC_GRID_9
		afx_msg void	OnBtnApply ();										// ON_COMMAND IDC_BUTTON_1
	DECLARE_MESSAGE_MAP ()
};
