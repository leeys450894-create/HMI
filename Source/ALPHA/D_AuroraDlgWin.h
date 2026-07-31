/*
* =====================================================================================
*
*       Filename:  CD_AuroraDlgWin.h
*
*    Description:
*
*        Version:  1.0
*        Created:  2012-01-09 오후 7:31:04
*       Revision:  none
*       Compiler:  gcc
*
*         Author:  YOUR NAME (),
*        Company:
*
* =====================================================================================
*/
#pragma once

#include "resource.h"
#include "common\MFCGridControl\GridCtrl_src\GridCtrl.h"
#include "common\MFCGridControl\NewCellTypes\GridCellCheck.h"
#include "common\Label.h"

// CD_AuroraDlgWin 대화 상자입니다.

class CD_AuroraDlgWin : public CDialogEx
{
	DECLARE_DYNAMIC(CD_AuroraDlgWin);

private:
	CWnd	*m_pParent;
	UINT	m_idTemplate;
	CSize	m_szMinimum;

public:
	BOOL Create(UINT id, BOOL bShow = TRUE);
	virtual void ControlButtonEnable(BOOL bEnable);

public:
	CD_AuroraDlgWin(UINT idTemplate, CWnd *pParent = 0);
	virtual ~CD_AuroraDlgWin();

protected:
	virtual void PostNcDestroy(void);
	virtual BOOL PreTranslateMessage(MSG *pMsg);
	DECLARE_MESSAGE_MAP()

	afx_msg LRESULT OnWinMgr(WPARAM wp, LPARAM lp);
	afx_msg void OnDestroy();
};

