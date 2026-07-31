// dSharedMemory.h : main header file for the DSHAREDMEMORY DLL
//

#if !defined(AFX_DSHAREDMEMORY_H__2AA89818_DF1C_491F_9365_E164103BAD30__INCLUDED_)
#define AFX_DSHAREDMEMORY_H__2AA89818_DF1C_491F_9365_E164103BAD30__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols
#include "SharedMemoryDefine.h"

/////////////////////////////////////////////////////////////////////////////
// CDSharedMemoryApp
// See dSharedMemory.cpp for the implementation of this class
//
extern HANDLE			g_hSysMapFile[5];
extern SHARED_MEMORTY*		g_pMapView[5];

class CdSharedMemoryApp : public CWinApp
{
public:
	CdSharedMemoryApp();

	BOOL SetSharedMemory();
	
	virtual BOOL InitInstance();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDSharedMemoryApp)
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CDSharedMemoryApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	virtual int ExitInstance();
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DSHAREDMEMORY_H__2AA89818_DF1C_491F_9365_E164103BAD30__INCLUDED_)
