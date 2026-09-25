// dfcRecipe.cpp : 해당 DLL의 초기화 루틴을 정의합니다.
//

#include "pch.h"
#include "dfcRecipe.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//
//TODO: 이 DLL이 MFC DLL에 대해 동적으로 링크되어 있는 경우
//		MFC로 호출되는 이 DLL에서 내보내지는 모든 함수의
//		시작 부분에 AFX_MANAGE_STATE 매크로가
//		들어 있어야 합니다.
//
//		예:
//
//		extern "C" BOOL PASCAL EXPORT ExportedFunction()
//		{
//			AFX_MANAGE_STATE(AfxGetStaticModuleState());
//			// 일반적인 함수 본문은 여기에 옵니다.
//		}
//
//		이 매크로는 MFC로 호출하기 전에
//		각 함수에 반드시 들어 있어야 합니다.
//		즉, 매크로는 함수의 첫 번째 문이어야 하며 
//		개체 변수의 생성자가 MFC DLL로
//		호출할 수 있으므로 개체 변수가 선언되기 전에
//		나와야 합니다.
//
//		자세한 내용은
//		MFC Technical Note 33 및 58을 참조하십시오.
//

// CdfcRecipeApp

BEGIN_MESSAGE_MAP(CdfcRecipeApp, CWinApp)
END_MESSAGE_MAP()

CdfcRecipeApp::CdfcRecipeApp()
{
	OleInitialize (NULL);
	m_iniFile = C_IniFile (dfcIniFile ());
	m_strRcpFolder = m_iniFile.GetProfileString (_T("PATH"), _T("RECIPE_FOLDER"), _T("D:\\"));
	m_strRcpBackup = m_iniFile.GetProfileString (_T("PATH"), _T("BACKUP_FOLDER"), _T("D:\\"));
	m_strAlignFolder  = m_iniFile.GetProfileString (_T("PATH"), _T("ALIGN_FOLDER"), _T("D:\\")); 
	m_strRefImgFolder = m_iniFile.GetProfileString (_T("PATH"), _T("REF_IMG_FOLDER"), _T("D:\\")); 
	m_fContactLimit   = m_iniFile.GetProfileDouble (_T("SETTING"), _T("CONTACT_LIMIT"), 20.00);		// LYS 2026-09-18 Contact Limit 5->20 TEMP
	m_strPPIDPath	  = m_iniFile.GetProfileString (_T("PATH"), _T("PPID_FOLDER"), _T("D:\\"));
	m_strRcpListPath  = m_iniFile.GetProfileString (_T("PATH"), _T("RECIPE_LIST"), _T("D:\\")); 


	int nCnt = 0;
	nCnt = m_iniFile.GetProfileInt (_T("COLOR"), _T("COUNT"), 0);

	double fData1 = 0.0, fData2 = 0.0;
	for (int i = 0; i < nCnt; i++) {
		fData1 = m_iniFile.GetProfileDouble (_T("COLOR"), dfcStr(_T("ITEM_%d"), i + 1), -600.00);
		fData2 = m_iniFile.GetProfileDouble (_T("COLOR"), dfcStr(_T("ZPOS_%d"), i + 1), -600.00);
		m_arrColorItem.Add(dfcStr(_T("%.2lf"), fData1));
		m_arrColorPosZ.Add(dfcStr(_T("%.2lf"), fData2));
	}
}

CdfcRecipeApp theApp;

HANDLE			g_hSysMapFile = NULL;
VTM_RECIPE*		g_pSysMapView = NULL;
//S_RecipeMgr		g_RecipeMgr;
C_InRecipeFunc	g_inRcpInfo;

BOOL CdfcRecipeApp::InitInstance()
{
	if (!g_hSysMapFile && !g_pSysMapView) {
		g_hSysMapFile = CreateFileMapping (INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof (VTM_RECIPE), df_RECIPE_SHARE_MEM_STRING);
		
		if (!g_hSysMapFile) {
			AfxMessageBox (_T("CreateFileMapping Failed (WBS_SHARE_MEM_STRING)"));
			return FALSE;
		}

		g_pSysMapView = (VTM_RECIPE*) MapViewOfFile (g_hSysMapFile, FILE_MAP_ALL_ACCESS, 0, 0, sizeof (VTM_RECIPE));

		if (!g_pSysMapView) {
			CloseHandle (g_hSysMapFile);
			AfxMessageBox (_T("MapViewOfFile Failed (WBS_SHARE_MEM_STRING)"));
			return FALSE;
		}
	}

	return CWinApp::InitInstance();
}

int CdfcRecipeApp::ExitInstance() 
{
	OleUninitialize ();

	if (g_pSysMapView) UnmapViewOfFile (g_pSysMapView);
	if (g_hSysMapFile) CloseHandle (g_hSysMapFile);

	return CWinApp::ExitInstance();
}