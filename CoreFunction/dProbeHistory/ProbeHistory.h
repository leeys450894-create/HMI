// ProbeHistory.h : main header file for the ProbeHistory DLL
//

#if !defined(AFX_ProbeHistory_H__11B50DC8_3B3C_4763_B1E3_051D7637ACAC__INCLUDED_)
#define AFX_ProbeHistory_H__11B50DC8_3B3C_4763_B1E3_051D7637ACAC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CProbeHistoryApp
// See ProbeHistory.cpp for the implementation of this class
//

class CProbeHistoryApp : public CWinApp
{
public:
	CProbeHistoryApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CProbeHistoryApp)
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CProbeHistoryApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ProbeHistory_H__11B50DC8_3B3C_4763_B1E3_051D7637ACAC__INCLUDED_)
