#if !defined(AFX_DLGMONITORING_H__4E5C58FF_2E34_4E11_B179_31B998917EF2__INCLUDED_)
#define AFX_DLGMONITORING_H__4E5C58FF_2E34_4E11_B179_31B998917EF2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgMonitoring.h : header file
//

#include "DlgPLC.h"
#include "DlgPLCDRead.h"
#include "..\Include\API_MXComponent.h"
#include "afxwin.h"
#include "LedButton.h"
//#include "..\\Include\API_MelsoftControl.h"

#define BTN_CHECK(btn, bCheck)	{ if ((btn).GetCheck() != (bCheck))			(btn).SetCheck(bCheck); }

class CDlgPLC;
class CDlgPLCDRead;

/////////////////////////////////////////////////////////////////////////////
// CDlgMonitoring dialog

class CDlgMonitoring : public CDialogEx
{
// Construction
public:
	void GetConfig();
	void UpdateMonitoringUI();
	void SetStateMessage(CString& state, int num);  // gdh 20180324 add
	void MoveSubMonitor();							// mn.bong 20180328 add
	void Prepare(int nIndex);						//sypark 20180608 ad

public:
	CDlgMonitoring(CDlgPLC& _pDlgPLC, CDlgPLCDRead& _dlgPLCRead, CWnd* pParent = NULL);   // standard constructor

	CDlgPLC&		m_pDlgPLC;
	CDlgPLCDRead&	m_pDlgPLCRead;

	BOOL		m_bUiUse;

	int			m_nUseMonitorNum;
	int			m_nMonitorWidth;
	int			m_nMonitorHeight;
	int			m_nMonitorPosX;
	int			m_nMonitorPosY;

	char		m_szName[3][256];
	// gdh 20180322 add
	double		m_dDown[2];	
	double		m_dUp[2];	
	int			m_nDataNumber[2];	
	int			m_nFontSize;
	CString		m_strEQName;		
	CString		m_strMonitorName;	//sypark 20180608 add
	CString		m_strDataUnit[2];
	BOOL		m_bBelowIsStable[2];
	//double		m_dPrevData[2];		//sypark 20180608 change
	double		m_dPrevData[3];			// O2 %, H2O, O2 PPM 
	double		m_dNewData[3];			// O2 %, H2O, O2 PPM 

	//sypark 20180608 add
	CString		strData[7];				// 0:O2 1:H20 2:% 3:ppm 4:장비이름
	double		m_dPrevData1[2];
	double		m_dPrevData2[2];
	double		m_dPrevData3[2];
	double		m_dPrevData4[2];
	double		m_dPrevData5[2];
	double		m_dPrevData6[2];
	//

	BOOL		m_bLive;

	CFont m_font;
	CFont m_font2;
	CFont m_font3;

	int	m_nIndex;

	CBitmap			m_bmpBk;	
	BITMAP			m_bmBk;

// Dialog Data
	//{{AFX_DATA(CDlgMonitoring)
	enum { IDD = IDD_DLG_MONITORING };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgMonitoring)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgMonitoring)
	virtual BOOL OnInitDialog();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual void OnCancel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedCancel();
	void StartUIMonitor(void);
	int SetUIUpdateData(double data1, double data2, double data3, BOOL bLive);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	CLedButton m_ledLive;
	BOOL       m_ledbLive;
	CString	   m_strTime;
	afx_msg void OnDestroy();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	void DrawBackGround(CDC *pDC);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGMONITORING_H__4E5C58FF_2E34_4E11_B179_31B998917EF2__INCLUDED_)
