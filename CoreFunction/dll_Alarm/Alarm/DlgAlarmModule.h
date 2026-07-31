#if !defined(AFX_DLGALARMMODULE_H__86E937FE_8DC0_4445_B9EF_9675116B9EE9__INCLUDED_)
#define AFX_DLGALARMMODULE_H__86E937FE_8DC0_4445_B9EF_9675116B9EE9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#include "AlarmDefine.h"
#include "..\..\common\Label.h"
#endif // _MSC_VER > 1000
// DlgAlarmModule.h : header file
//

enum enumStaticID_AlarmID
{
	enumStaticID_AlarmID = 0,
	enumStaticID_AlarmModule,
	enumStaticID_AlarmType,
	enumStaticID_AlarmCommand,
	enumStaticID_AlarmText,
	enumStaticID_AlarmDesc,
	enumStaticID_AlarmCount,
};

/////////////////////////////////////////////////////////////////////////////
// CDlgAlarmModule dialog

class CDlgAlarmModule : public CDialog
{
// Construction
public:
	CDlgAlarmModule(CWnd* pParent = NULL);
	void ShowAlarm(ALARM_EVENT_DATA*pAlarm);

	CLabel				m_LabelInfo[enumStaticID_AlarmCount];
	ALARM_EVENT_DATA *m_pAlarm;
	CBrush	m_brushBK;

// Dialog Data
	//{{AFX_DATA(CDlgAlarmModule)
	enum { IDD = IDD_DLG_ALARM_MODULE };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgAlarmModule)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgAlarmModule)
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	virtual BOOL OnInitDialog();
	afx_msg void OnButtonAlarmModuleInitretry();
	afx_msg void OnButtonAlarmModuleRetry();
	afx_msg void OnButtonAlarmModuleIgnore();
	afx_msg void OnButtonAlarmModuleAbort();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnClose();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGALARMMODULE_H__86E937FE_8DC0_4445_B9EF_9675116B9EE9__INCLUDED_)
