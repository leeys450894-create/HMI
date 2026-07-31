#include "pch.h"
#include "RecipeMgr.h"

// ----------------------------------------------------------------------------------------------------
// Singleton
//----------------------------------------------------------------------------------------------------
CRecipeMgr * CRecipeMgr::m_pInstance = nullptr;
CRecipeMgr* CRecipeMgr::GetInstance()
{
	return m_pInstance ? m_pInstance : (m_pInstance = new CRecipeMgr);
}

void CRecipeMgr::DelInstance()
{
	if (m_pInstance)
	{
		delete m_pInstance;
		m_pInstance = nullptr;
	}
}

CRecipeMgr::CRecipeMgr()
{
	m_pSharedRecipe = nullptr;
}

CRecipeMgr::~CRecipeMgr()
{
	Finalize();
}

CWnd* CRecipeMgr::GetReportmWnd()
{
	return ::Recipe_GetRecipehWnd();
}

void CRecipeMgr::Initialize(HWND hParentWnd, CRect rtROI, CString strAppName, CString strAppPath, CString strConfigPath, CString strLogPath)
{
	const CString strFilePath = strConfigPath + _T("\\CoreFunction\\CoreFunction.ini");


	::Recipe_Initialize(hParentWnd, rtROI);

//	CWnd* pWnd = nullptr;
//	pWnd = GetReportmWnd();
//	pWnd->ShowWindow(SW_HIDE);

	//::Recipe_SetMsg(strAppName);

	//CString strRecipePath, strMCCLogPath;

	//strRecipePath.Format(_T("%s"), strLogPath);
	//strMCCLogPath.Format(_T("%s%s"), strRecipePath, _T("MCC"));
	//::API_MCC_SetLogPath(strMCCLogPath);
	//::API_MCC_SetConfigPath(strConfigPath);

	// For Recipe
	m_pSharedRecipe = Recipe_GetSharedMemory();
}

void CRecipeMgr::Finalize()
{
	//----------------------------------------------------------------------
	// 데이터 삭제
	//----------------------------------------------------------------------
	::Recipe_Finalize();
}