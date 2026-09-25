// DlgTestMode.h
// LYS 2026-07-02 Test Mode 다이얼로그(GMI IDD_TEST_MODE_DLG 이식).
// dfcRecipe DLL 안의 모델리스 다이얼로그 - C_RecipeMgrDlg::OnRecipeAlign 패턴.
#pragma once

#include "BaseDialog.h"
#include "RecipeShareMem.h"

class CDlgTestMode : public C_BaseDialog
{
	DECLARE_DYNAMIC(CDlgTestMode)

public:
	CDlgTestMode(CWnd* pParent = NULL);
	virtual ~CDlgTestMode();

	BOOL ShowWindow(int nCmdShow);

	VTM_RECIPE* m_pRecipeMem;

	void InitSelectGridUI(C_GridCtrl& Grid, int nDlgItem, int nRow, int nCol);
	void SetViewSelectCell();
	void GetSelectCell(int* pRow, int* pCol);

protected:
	CWnd* m_hWnd;
	int m_nTestMode;    // 0=Review, 1=WAD, 2=Vision, 3=Angle

	C_GridCtrl  m_gridGlass;
	C_ButtonEx  m_rdMode[4];
	C_ButtonEx  m_rdStage;
	C_ButtonEx  m_btnMove;
	C_ButtonEx  m_btnPreContact;
	C_ButtonEx  m_btnContact;
	C_ButtonEx  m_btnUnContact;

	int m_nBfRowGlass;
	int m_nBColGlass;
	COLORREF m_nBfcrBkColour;

protected:
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);
	afx_msg void OnDestroy();
	afx_msg void OnSelMode(UINT nID);
	afx_msg void OnBtnMove();
	afx_msg void OnBtnPreContact();
	afx_msg void OnBtnContact();
	afx_msg void OnBtnUnContact();
	afx_msg void OnSelItem(NMHDR* pNMHDR, LRESULT* pResult);

	DECLARE_MESSAGE_MAP()
};
