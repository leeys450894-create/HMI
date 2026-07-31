#pragma once

#include "resource.h"
#include "common\MFCGridControl\GridCtrl_src\GridCtrl.h"
#include "common\MFCGridControl\NewCellTypes\GridCellCheck.h"
#include "common\MFCGridControl\NewCellTypes\GridCellCombo.h"
#include "common\MFCGridControl\NewCellTypes\GridCellButton.h"
#include "common\Label.h"
#include "common\XButtonXP\XButtonXP.h"

// CMainSubDlgWin 대화 상자입니다.

class CMainSubDlgWin : public CDialogEx
{
	DECLARE_DYNAMIC(CMainSubDlgWin);

private:
	CWnd	*m_pParent;
	UINT	m_idTemplate;
	CSize	m_szMinimum;

public:
	BOOL Create(UINT id, CWnd *pParent, BOOL bShow = TRUE);

public:
	virtual void UpdateOperateStatus(CString str);
	virtual void ShowReportAlarm();
	virtual void UpdateSeqItemElapsedTime();
	virtual void UpdateMeasureElapsedTime(bool bStart);
	virtual void UpdateCycleInspectionStart();
	virtual void UpdateCycleInspectionResult();
	virtual void UpdateCycleInspectionComplete();
	virtual void UpdateChangeInitialize();
	virtual void UpdateRecipeInformation();
	virtual void UpdateInformation();
	virtual void UpdateDataLampTime(int iLamp, bool bOn);
	virtual void UpdateWriteLampTime(int iLamp, bool bOn);
	virtual void RefreshListData();
	virtual void UpdateCIMState(int iState);
	virtual void UpdateControlMode(int iState);
	virtual void UpdateAgingMode(bool bStart, int nTime);
	virtual void UpdateSACCMode(bool bStart);
	virtual void UpdateMeasColor();
	virtual void UpdateCIMAlarm(int iAlarm);

public:
	CMainSubDlgWin(UINT idTemplate, CWnd *pParent = NULL);
	virtual ~CMainSubDlgWin();

protected:
	virtual BOOL PreTranslateMessage(MSG *pMsg);
	DECLARE_MESSAGE_MAP()

	afx_msg LRESULT OnWinMgr(WPARAM wp, LPARAM lp);
};

