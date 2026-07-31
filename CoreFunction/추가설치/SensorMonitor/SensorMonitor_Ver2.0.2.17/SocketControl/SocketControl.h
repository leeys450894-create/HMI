// SocketControl.h : main header file for the SOCKETCONTROL DLL
//

#if !defined(AFX_SOCKETCONTROL_H__86AF6F02_3818_41D8_A0A5_2EE53A9A2601__INCLUDED_)
#define AFX_SOCKETCONTROL_H__86AF6F02_3818_41D8_A0A5_2EE53A9A2601__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols



/////////////////////////////////////////////////////////////////////////////
// CSocketControlApp
// See SocketControl.cpp for the implementation of this class
//

class CSocketControlApp : public CWinApp
{
public:
	CSocketControlApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSocketControlApp)
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CSocketControlApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SOCKETCONTROL_H__86AF6F02_3818_41D8_A0A5_2EE53A9A2601__INCLUDED_)
