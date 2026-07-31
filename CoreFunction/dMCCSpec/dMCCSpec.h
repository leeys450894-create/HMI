// dMCCSpec.h : main header file for the DMCCSPEC DLL
//

#if !defined(AFX_DMCCSPEC_H__A2103634_EB95_430E_A57C_A46ADCAC5CD8__INCLUDED_)
#define AFX_DMCCSPEC_H__A2103634_EB95_430E_A57C_A46ADCAC5CD8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CDMCCSpecApp
// See dMCCSpec.cpp for the implementation of this class
//

class CDMCCSpecApp : public CWinApp
{
public:
	CDMCCSpecApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDMCCSpecApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CDMCCSpecApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DMCCSPEC_H__A2103634_EB95_430E_A57C_A46ADCAC5CD8__INCLUDED_)
