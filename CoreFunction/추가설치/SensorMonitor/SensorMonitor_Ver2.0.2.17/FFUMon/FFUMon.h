// FFUMon.h : main header file for the FFUMON application
//

#if !defined(AFX_FFUMON_H__2145F663_2BCC_4E19_8794_DBC45C51C72C__INCLUDED_)
#define AFX_FFUMON_H__2145F663_2BCC_4E19_8794_DBC45C51C72C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CFFUMonApp:
// See FFUMon.cpp for the implementation of this class
//

class CFFUMonApp : public CWinApp
{
public:
	CFFUMonApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFFUMonApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CFFUMonApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FFUMON_H__2145F663_2BCC_4E19_8794_DBC45C51C72C__INCLUDED_)
