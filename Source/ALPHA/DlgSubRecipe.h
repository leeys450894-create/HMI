#pragma once

#include "MainSubDlgWin.h"
#include "WindowEventHandler.h"

// CDlgSubRecipe 대화 상자입니다.
class CDlgSubRcpFile;
class CRecipeEditMgr;
class CDlgSubRecipe : public CMainSubDlgWin
{
	DECLARE_DYNAMIC(CDlgSubRecipe)

private:
	CDlgSubRcpFile*		m_pSubRcpFile;
	CAuroraMgr*			m_pAuroraMgr;
	CRecipeEditMgr		*m_pRecipeEditMgr;

	CLabel		m_LabelInfo;

public:
	CDlgSubRecipe(CAuroraMgr *pAuroraMgr, CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~CDlgSubRecipe();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DLG_SUB_RECIPE };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedRcpBtnNew();
	afx_msg void OnBnClickedRcpBtnOpen();
	afx_msg void OnBnClickedRcpBtnSaveas();
	afx_msg void OnBnClickedRcpBtnSave();
	afx_msg void OnBnClickedRcpBtnDelete();
	void UpdateRecipeInformation();
};
