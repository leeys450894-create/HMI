#if !defined(AFX_DLGALARM_H__25FDA8FE_7918_498A_9F7D_06F5ECF37D0C__INCLUDED_)
#define AFX_DLGALARM_H__25FDA8FE_7918_498A_9F7D_06F5ECF37D0C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#include "AlarmDefine.h"
#include "DlgAlarmModule.h"
#include "..\..\common\Label.h"
#include "..\..\common\MySpreadDll.h"
#endif // _MSC_VER > 1000
// DlgAlarm.h : header file
//

///////////////////////////////////////////////////////////////////////////////
// CDlgAlarm dialog

class CDlgAlarm : public CDialog
{
// Construction
public:
	CDlgAlarm(CWnd* pParent = NULL);   // standard constructor
 
	CDlgAlarmModule* m_pDlgAlarmModule;

	HBRUSH	m_hBrush;

	HFONT m_hfont;
	CMySpreadDll* m_sprdResult;

public:
	void InitControls();
	
	void CreateAlarmModule(HWND hParent);
	void ShowAlarmModule(BOOL bShow);
	
	void InsertList(ALARM_EVENT_DATA *pAlarm);
	void DeleteList(UINT id);
	BOOL FindList(UINT id);
	int GetListCount();

// Dialog Data
	//{{AFX_DATA(CDlgAlarm)
	enum { IDD = IDD_DLG_ALARM };
	CLabel		m_LabelInfo;
	CListCtrl	m_listAlarm;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgAlarm)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgAlarm)
	afx_msg void OnDestroy();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	virtual BOOL OnInitDialog();
	afx_msg void OnClickListAlarm(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
//	CString GetText(int iRow, int col);
//	BOOL LoadSpred(CString strFileName, bool bNew = false);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	CString MessageCode2Str(int nID, CString strPath);
	BOOL LoadSpred(CString strFileName, bool bNew = false);
	CString GetText(int iRow, int col);
	int UpdateLanguage(HINSTANCE hResource_Lang_Dll); //lhj 20181225 lang Report Alarm

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGALARM_H__25FDA8FE_7918_498A_9F7D_06F5ECF37D0C__INCLUDED_)
