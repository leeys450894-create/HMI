#pragma once
#include "ButtonEx.h"
#include "preDefine.h"

class C_SelectCellDlg : public C_BaseDialog
{
	DECLARE_DYNAMIC(C_SelectCellDlg)

	public:
		C_SelectCellDlg (CWnd* pParent = NULL);   // 표준 생성자입니다.
		virtual ~C_SelectCellDlg();

	protected:
		CWnd		*m_hWnd;
		C_GridCtrl	m_gridSelSpec;
		int			m_nMode;
		C_ButtonEx	m_rdMode[M_MODE_MAX];

	public:
		//VTM_RECIPE	*m_pRecipeMem;

	public:
		void InitSelectGridUI (C_GridCtrl &Grid, int nDlgItem, int nRow, int nCol, int nAng);
		void OnSelItem (NMHDR *pNMHDR, LRESULT *pResult);
		void SetViewSelectCell (/*int nMode*/);
		void UpdateSelectCell ();
		// LYS 2026-08-18 bulk select helpers. View-only: the grid colour is the working
		// state and nothing reaches the recipe until Cell Update is pressed, same as a
		// manual cell click.
		void SetAllCell (BOOL bSelect);
		BOOL ShowWindow (int nCmdShow);

	protected:
		virtual BOOL OnInitDialog ();
		virtual void DoDataExchange (CDataExchange* pDX);    // DDX/DDV 지원입니다.
		afx_msg void OnDestroy ();
		afx_msg void OnSelMode (UINT nID);
		afx_msg void OnUpdateCell ();
		afx_msg void OnSelectAllCell ();	// LYS 2026-08-18
		afx_msg void OnClearAllCell ();		// LYS 2026-08-18


	DECLARE_MESSAGE_MAP()
};
