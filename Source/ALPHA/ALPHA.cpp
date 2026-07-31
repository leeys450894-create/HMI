
// ALPHA.cpp: 애플리케이션에 대한 클래스 동작을 정의합니다.
//

#include "pch.h"
#include "framework.h"
#include "ALPHA.h"
#include "DlgALPHA.h"

#include <tlhelp32.h>
#include "AuroraFuncDefine.h"
#include "AuroraMgr.h"
#include "SystemParamShare.h"
#include "Manager\SystemConfigMgr.h"
#include "Manager\ShareMemoryMgr.h"
#include "common/CommonFunc.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CALPHAApp

BEGIN_MESSAGE_MAP(CALPHAApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CALPHAApp 생성

CALPHAApp::CALPHAApp()
{
	// TODO: 여기에 생성 코드를 추가합니다.
	// InitInstance에 모든 중요한 초기화 작업을 배치합니다.
}


// 유일한 CALPHAApp 개체입니다.

CALPHAApp theApp;


// CALPHAApp 초기화

BOOL CALPHAApp::InitInstance()
{
	//--------------------------------------------------------------------------------
	// 동일한 프로그램이 2개 이상 수행되지 못하도록 하고 싶다.
	//--------------------------------------------------------------------------------
	CString strExeName(m_pszExeName);
	CString strTopAppName = strExeName + _T(".exe");
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	if ((hSnapshot == INVALID_HANDLE_VALUE) || (hSnapshot == NULL))
		return FALSE;
	PROCESSENTRY32 sEntry32;
	sEntry32.dwSize = sizeof(PROCESSENTRY32);
	int iProcess = 0;
	BOOL bRemain = Process32First(hSnapshot, &sEntry32);
	while (bRemain)
	{
		CString szExeName(sEntry32.szExeFile);
		if (szExeName.CompareNoCase(strTopAppName) == 0)
		{
			if(sEntry32.cntThreads > 0) // 2023.04.06 add
				iProcess++;
		}
		bRemain = Process32Next(hSnapshot, &sEntry32);
	}
	CloseHandle(hSnapshot);
	if (1 < iProcess)
	{
		AfxMessageBox(strExeName + _T(" Is Already Running!!!"));
		return FALSE;
	}
	//--------------------------------------------------------------------------------

	// 응용 프로그램 매니페스트가 ComCtl32.dll 버전 6 이상을 사용하여 비주얼 스타일을
	// 사용하도록 지정하는 경우, Windows XP 상에서 반드시 InitCommonControlsEx()가 필요합니다.
	// InitCommonControlsEx()를 사용하지 않으면 창을 만들 수 없습니다.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// 응용 프로그램에서 사용할 모든 공용 컨트롤 클래스를 포함하도록
	// 이 항목을 설정하십시오.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();

	if (!AfxSocketInit())
	{
		AfxMessageBox(IDP_SOCKETS_INIT_FAILED);
		return FALSE;
	}

	AfxEnableControlContainer();

	// 대화 상자에 셸 트리 뷰 또는
	// 셸 목록 뷰 컨트롤이 포함되어 있는 경우 셸 관리자를 만듭니다.
	CShellManager *pShellManager = new CShellManager;

	// MFC 컨트롤의 테마를 사용하기 위해 "Windows 원형" 비주얼 관리자 활성화
	CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));

	// 표준 초기화
	// 이들 기능을 사용하지 않고 최종 실행 파일의 크기를 줄이려면
	// 아래에서 필요 없는 특정 초기화
	// 루틴을 제거해야 합니다.
	// 해당 설정이 저장된 레지스트리 키를 변경하십시오.
	// TODO: 이 문자열을 회사 또는 조직의 이름과 같은
	// 적절한 내용으로 수정해야 합니다.
	SetRegistryKey(_T("로컬 애플리케이션 마법사에서 생성된 애플리케이션"));


	

	// jsyoon 20210712 add
//--------------------------------------------------------------------------------
// APP FullPath
//--------------------------------------------------------------------------------
	TCHAR szFullPath[MAX_PATH] = { 0, };
	TCHAR szDrive[_MAX_DRIVE] = { 0, };
	TCHAR szDir[_MAX_DIR] = { 0, };

	// Get application's full path
	::GetModuleFileName(NULL, szFullPath, MAX_PATH);

	// Break full path into separate components
	_tsplitpath_s(szFullPath, szDrive, _MAX_DRIVE, szDir, _MAX_DIR, NULL, NULL, NULL, NULL);

	CString strAppDir;
	strAppDir.Format(_T("%s%s"), szDrive, szDir);
	CSystemParamShare::GetInstance()->SetAppFolderPath(strAppDir);

	//--------------------------------------------------------------------------------
	// Make APP Path
	//--------------------------------------------------------------------------------
	// config
	CString strConfigPath = strAppDir + _T("Config\\");
	CreateDirectory(strConfigPath, NULL);
	CSystemParamShare::GetInstance()->SetConfigFolderPath(strConfigPath);

	// recipe	
	CString strDefaultRecipePath = strAppDir + _T("DefaultRcp\\");
	CreateDirectory(strDefaultRecipePath, NULL);
	DWORD attr = GetFileAttributes(strDefaultRecipePath);	// - DefaultRecipe, hidden option
	SetFileAttributes(strDefaultRecipePath, attr | FILE_ATTRIBUTE_HIDDEN);
	CSystemParamShare::GetInstance()->SetDefaultRecipeFolderPath(strDefaultRecipePath);

	//// result
	//CString strResultPath = strAppDir + _T("Result\\");
	//CreateDirectory(strResultPath, NULL);
	//CSystemParamShare::GetInstance()->SetResultFolderPath(strResultPath);

	// alarm
	CString strAlarmPath = strAppDir + _T("Config\\CoreFunction\\AlarmConfig\\");
	CreateDirectory(strAlarmPath, NULL);
	CSystemParamShare::GetInstance()->SetAlarmFolderPath(strAlarmPath);

	// SYSTEM NAME
	CString strSysName;
	//strSysName = _T("HMI"); // 장비 이름
	strSysName = _T("OCP"); // 장비 이름		// LYS 2026-06-26 HMI -> OCP Change

	CString strLogPath = strAppDir + _T("Logs\\");
	CString strRecipePath = strAppDir + _T("Recipe\\");

	//--------------------------------------------------------------------
	// To create signal/slot manager
	//--------------------------------------------------------------------
	m_pAuroraMgr = new CAuroraMgr(strSysName, strAppDir, strConfigPath, strRecipePath, strLogPath);

	m_pAuroraMgr->GetShareMemoryMgrPtr()->GetSharedMemory();
	m_pAuroraMgr->GetShareMemoryMgrPtr()->SetSys_MachineMode(df_HOST_OFFLINE);
	m_pAuroraMgr->GetShareMemoryMgrPtr()->SetSys_MachineType(CUR_EQU_TYPE);

	// jsyoon 2022.07.06 add xCrash report <start>// Base address. /////////////////
	HANDLE hSnapshot2 = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, 0);
	MODULEENTRY32 modEntry;
	modEntry.dwSize = sizeof(MODULEENTRY32);
	if (Module32First(hSnapshot2, &modEntry))
	{
		CString strLog;
		strLog.Format(_T("0x%p, %10d, %s %s"), modEntry.modBaseAddr, modEntry.modBaseSize,
			modEntry.szModule, modEntry.szExePath);
		CLogWrapper::GetInstance()->KMLOG(SYST, strLog);

		for (; Module32Next(hSnapshot2, &modEntry);)
		{
			strLog.Format(_T("0x%p, %10d, %s %s"), modEntry.modBaseAddr, modEntry.modBaseSize, modEntry.szModule, modEntry.szExePath);
				CLogWrapper::GetInstance()->KMLOG(SYST, strLog);
		}
	}
	CloseHandle(hSnapshot2);

	//// logs
	//strLogPath.Format(_T("%s\\Logs\\"), m_pAuroraMgr->GetSystemConfigMgrPtr()->GetLogFilePath());
	//MakeDir(strLogPath);
	////	CreateDirectory(strLogPath, NULL);
	//CSystemParamShare::GetInstance()->SetLoggerFolderPath(strLogPath);

	strRecipePath.Format(_T("%s\\Recipe\\"), m_pAuroraMgr->GetSystemConfigMgrPtr()->GetRecipePath());
	MakeDir(strRecipePath);
	//	CreateDirectory(strRecipePath, NULL);
	CSystemParamShare::GetInstance()->SetRecipeFolderPath(strRecipePath);

	//------------------------------------------------------------------------------
	// 화면 표시할 언어를 선택한다.
	//------------------------------------------------------------------------------
	const int iLanguageIndex = m_pAuroraMgr->GetSystemConfigMgrPtr()->GetLanguageIndex();

	LCID   locale1 = MAKELCID(MAKELANGID(LANG_KOREAN, SUBLANG_KOREAN), SORT_DEFAULT);
	LANGID locale2 = MAKELANGID(LANG_KOREAN, SUBLANG_KOREAN);

	if (iLanguageIndex == 0) // korean
	{
		locale1 = MAKELCID(MAKELANGID(LANG_KOREAN, SUBLANG_KOREAN), SORT_DEFAULT);
		locale2 = MAKELANGID(LANG_KOREAN, SUBLANG_KOREAN);
	}
	else if (iLanguageIndex == 1) // english
	{
		locale1 = MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT);
		locale2 = MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US);
	}
	else if (iLanguageIndex == 2) // chinese	
	{
		locale1 = MAKELCID(MAKELANGID(LANG_CHINESE, SUBLANG_NEUTRAL), SORT_DEFAULT);
		locale2 = MAKELANGID(LANG_CHINESE, SUBLANG_NEUTRAL);
	}
	::ConvertDefaultLocale(locale1);
	::SetThreadLocale(locale1);
	::SetThreadUILanguage(locale2);

	//------------------------------------------------------------------------------
	// 시작
	//------------------------------------------------------------------------------
	CDlgALPHA dlg(m_pAuroraMgr);
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: 여기에 [확인]을 클릭하여 대화 상자가 없어질 때 처리할
		//  코드를 배치합니다.
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: 여기에 [취소]를 클릭하여 대화 상자가 없어질 때 처리할
		//  코드를 배치합니다.
	}
	else if (nResponse == -1)
	{
		TRACE(traceAppMsg, 0, "경고: 대화 상자를 만들지 못했으므로 애플리케이션이 예기치 않게 종료됩니다.\n");
		TRACE(traceAppMsg, 0, "경고: 대화 상자에서 MFC 컨트롤을 사용하는 경우 #define _AFX_NO_MFC_CONTROLS_IN_DIALOGS를 수행할 수 없습니다.\n");
	}

	// 위에서 만든 셸 관리자를 삭제합니다.
	if (pShellManager != nullptr)
	{
		delete pShellManager;
	}

#if !defined(_AFXDLL) && !defined(_AFX_NO_MFC_CONTROLS_IN_DIALOGS)
	ControlBarCleanUp();
#endif

	// 대화 상자가 닫혔으므로 응용 프로그램의 메시지 펌프를 시작하지 않고 응용 프로그램을 끝낼 수 있도록 FALSE를
	// 반환합니다.
	return FALSE;
}

