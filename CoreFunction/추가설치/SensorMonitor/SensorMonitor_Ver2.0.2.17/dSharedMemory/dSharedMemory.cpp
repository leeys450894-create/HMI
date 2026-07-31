// dSharedMemory.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include "dSharedMemory.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//
//	Note!
//
//		If this DLL is dynamically linked against the MFC
//		DLLs, any functions exported from this DLL which
//		call into MFC must have the AFX_MANAGE_STATE macro
//		added at the very beginning of the function.
//
//		For example:
//
//		extern "C" BOOL PASCAL EXPORT ExportedFunction()
//		{
//			AFX_MANAGE_STATE(AfxGetStaticModuleState());
//			// normal function body here
//		}
//
//		It is very important that this macro appear in each
//		function, prior to any calls into MFC.  This means that
//		it must appear as the first statement within the 
//		function, even before any object variable declarations
//		as their constructors may generate calls into the MFC
//		DLL.
//
//		Please see MFC Technical Notes 33 and 58 for additional
//		details.
//

/////////////////////////////////////////////////////////////////////////////
// CDSharedMemoryApp

BEGIN_MESSAGE_MAP(CdSharedMemoryApp, CWinApp)
	//{{AFX_MSG_MAP(CDSharedMemoryApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDSharedMemoryApp construction

CdSharedMemoryApp::CdSharedMemoryApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CDSharedMemoryApp object

CdSharedMemoryApp theApp;



HANDLE			g_hSysMapFile[5];
SHARED_MEMORTY*		g_pMapView[5];		//20181207 edhan change [5]

BOOL CdSharedMemoryApp::InitInstance()
{
	CWinApp::InitInstance();
	
	SetSharedMemory();
	
	return TRUE;
}

BOOL CdSharedMemoryApp::SetSharedMemory()
{
	CString strShredMemString;
	for(int i = 0; i < 5; i++)
	{
		if(!g_hSysMapFile[i] && !g_pMapView[i])
		{
			strShredMemString.Format("%s_%d","KMAC SHARED MEMORY",i+1);

			g_hSysMapFile[i] = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0,
				sizeof(SHARED_MEMORTY), strShredMemString);
			
			if(!g_hSysMapFile[i])
			{
				AfxMessageBox(_T("CreateFileMapping Failed (strShredMemString)"));
				return FALSE;
			}
			
			g_pMapView[i] = (SHARED_MEMORTY*) MapViewOfFile(g_hSysMapFile[i], FILE_MAP_ALL_ACCESS, 0, 0,
				sizeof(SHARED_MEMORTY));
			
			if(!g_pMapView[i])
			{
				CloseHandle(g_hSysMapFile[i]);
				AfxMessageBox(_T("MapViewOfFile Failed (strShredMemString"));
				return FALSE;
			}
		}
	}


// 	if(!g_hSysMapFile && !g_pMapView)
// 	{
// 		g_hSysMapFile = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0,
// 			sizeof(SHARED_MEMORTY), MONITORING_SHARED_MEMORY_STRING);
// 		
// 		if(!g_hSysMapFile)
// 		{
// 			AfxMessageBox(_T("CreateFileMapping Failed (MONITORING_SHARED_MEMORY_STRING)"));
// 			return FALSE;
// 		}
// 		
// 		g_pMapView = (SHARED_MEMORTY*) MapViewOfFile(g_hSysMapFile, FILE_MAP_ALL_ACCESS, 0, 0,
// 			sizeof(SHARED_MEMORTY));
// 		
// 		if(!g_pMapView)
// 		{
// 			CloseHandle(g_hSysMapFile);
// 			AfxMessageBox(_T("MapViewOfFile Failed (MONITORING_SHARED_MEMORY_STRING"));
// 			return FALSE;
// 		}
// 	}
	
	return TRUE;
}

int CdSharedMemoryApp::ExitInstance()
{
	// TODO: Add your specialized code here and/or call the base class
	for(int i = 0; i < 5; i++)
	{
		if(g_pMapView[i])
			UnmapViewOfFile(g_pMapView[i]);
		
		if(g_hSysMapFile[i])
		CloseHandle(g_hSysMapFile[i]);
	}
	
	return CWinApp::ExitInstance();
}
