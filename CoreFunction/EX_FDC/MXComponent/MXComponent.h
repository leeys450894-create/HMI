// MXComponent.h : main header file for the MXCOMPONENT DLL
//

#if !defined(AFX_MXCOMPONENT_H__7715EA11_B0EA_4B55_A896_84CA077AD589__INCLUDED_)
#define AFX_MXCOMPONENT_H__7715EA11_B0EA_4B55_A896_84CA077AD589__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CMXComponentApp
// See MXComponent.cpp for the implementation of this class
//

class CMXComponentApp : public CWinApp
{
public:
	CMXComponentApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMXComponentApp)
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CMXComponentApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MXCOMPONENT_H__7715EA11_B0EA_4B55_A896_84CA077AD589__INCLUDED_)
