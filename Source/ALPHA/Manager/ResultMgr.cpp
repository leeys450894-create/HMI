#include "pch.h"
#include "ResultMgr.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif
//----------------------------------------------------------------------------------------------------
// Singleton
//----------------------------------------------------------------------------------------------------
CResultMgr* CResultMgr::m_pInstance = nullptr;
CResultMgr* CResultMgr::GetInstance()
{
	return m_pInstance ? m_pInstance : (m_pInstance = new CResultMgr);
}

void CResultMgr::DelInstance()
{
	if (m_pInstance)
	{
		delete m_pInstance;
		m_pInstance = nullptr;
	}
}

void CResultMgr::Initialize(HWND hParentWnd, CRect rtROI)
{
	::API_Result_Initialize(hParentWnd, rtROI);

	::API_Result_SetSearchMode(1);
}

CWnd* CResultMgr::GetReportmWnd()
{
	return ::API_Result_GetReportmWnd();
}

CResultMgr::CResultMgr(void)
{
}

CResultMgr::~CResultMgr(void)
{
	Finalize();
}

void CResultMgr::Finalize()
{
	::API_Result_FinalizeReport();
}

void CResultMgr::Updata_Data(PRSRITEMDATA result)
{
	::API_Result_Upload(result);
}

bool CResultMgr::SaveResult(bool bSaveFile, bool bRemote, CString strStart)
{
	return ::API_Result_SaveResult(bSaveFile, bRemote, strStart);
}

bool CResultMgr::SaveResultDone()
{
	return API_Result_SaveResultDone();
}

void CResultMgr::SetConfigPath(CString strPath)
{
	::API_Result_SetConfigPath(strPath);
}

void CResultMgr::ClearData()
{
	::API_Result_DataClear();
}