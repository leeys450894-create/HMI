// ResumeInfoMgr.cpp
// LYS 2026-07-03 Resume Inspection manager

#include "pch.h"
#include "ResumeInfoMgr.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

static const LPCTSTR SECTION_RESUME = _T("RESUME");

CResumeInfoMgr::CResumeInfoMgr(CString strConfigPath)
	: m_nLastStick(-1)
	, m_nTotalStick(0)
	, m_nGlsRotate(-1)
	, m_bRunning(false)
	, m_bCompleted(false)
	, m_nExpireHours(0)
{
	m_strFilePath.Format(_T("%s\\ResumeInfo.ini"), strConfigPath);
	m_strTmpPath.Format(_T("%s\\ResumeInfo.ini.tmp"), strConfigPath);

	LoadResumeInfo();

	CString strLog;
	strLog.Format(_T("ResumeInfoMgr initialized, file=%s, Running=%d, Completed=%d, LastStick=%d/%d"),
		m_strFilePath, m_bRunning ? 1 : 0, m_bCompleted ? 1 : 0, m_nLastStick, m_nTotalStick);
	CLogWrapper::GetInstance()->KMLOG(SYST, strLog);
}

CResumeInfoMgr::~CResumeInfoMgr()
{
}

// ============================================================================
// Load
// ============================================================================
bool CResumeInfoMgr::LoadResumeInfo()
{
	if (!PathFileExists(m_strFilePath))
	{
		m_strGlassID.Empty();
		m_strRecipeID.Empty();
		m_strLotID.Empty();
		m_strInnerID.Empty();
		m_nLastStick = -1;
		m_nTotalStick = 0;
		m_nGlsRotate = -1;  // LYS 2026-07-21
		m_bRunning = false;
		m_bCompleted = false;
		m_strSaveTime.Empty();
		m_strStartTime.Empty();
		m_strInspDate.Empty();
		m_strResultPath.Empty();
		return false;
	}

	TCHAR szBuf[512];

	// Identification
	GetPrivateProfileString(SECTION_RESUME, _T("GlassID"), _T(""), szBuf, 256, m_strFilePath);
	m_strGlassID = szBuf;

	GetPrivateProfileString(SECTION_RESUME, _T("RecipeID"), _T(""), szBuf, 256, m_strFilePath);
	m_strRecipeID = szBuf;

	GetPrivateProfileString(SECTION_RESUME, _T("LotID"), _T(""), szBuf, 256, m_strFilePath);
	m_strLotID = szBuf;

	GetPrivateProfileString(SECTION_RESUME, _T("InnerID"), _T(""), szBuf, 256, m_strFilePath);
	m_strInnerID = szBuf;

	// Progress
	m_nLastStick = GetPrivateProfileInt(SECTION_RESUME, _T("LastStick"), -1, m_strFilePath);
	m_nTotalStick = GetPrivateProfileInt(SECTION_RESUME, _T("TotalStick"), 0, m_strFilePath);
	m_nGlsRotate = GetPrivateProfileInt(SECTION_RESUME, _T("GlsRotate"), -1, m_strFilePath);  // LYS 2026-07-21
	m_bRunning = GetPrivateProfileInt(SECTION_RESUME, _T("Running"), 0, m_strFilePath) != 0;
	m_bCompleted = GetPrivateProfileInt(SECTION_RESUME, _T("Completed"), 0, m_strFilePath) != 0;

	// Time/Path
	GetPrivateProfileString(SECTION_RESUME, _T("SaveTime"), _T(""), szBuf, 256, m_strFilePath);
	m_strSaveTime = szBuf;

	GetPrivateProfileString(SECTION_RESUME, _T("StartTime"), _T(""), szBuf, 256, m_strFilePath);
	m_strStartTime = szBuf;

	GetPrivateProfileString(SECTION_RESUME, _T("InspDate"), _T(""), szBuf, 256, m_strFilePath);
	m_strInspDate = szBuf;

	GetPrivateProfileString(SECTION_RESUME, _T("ResultPath"), _T(""), szBuf, 512, m_strFilePath);
	m_strResultPath = szBuf;

	CString strLog;
	strLog.Format(_T("Resume Load: Glass=%s, Recipe=%s, Lot=%s, Stick=%d/%d, Running=%d, Completed=%d, Date=%s"),
		m_strGlassID, m_strRecipeID, m_strLotID, m_nLastStick, m_nTotalStick,
		m_bRunning ? 1 : 0, m_bCompleted ? 1 : 0, m_strInspDate);
	CLogWrapper::GetInstance()->KMLOG(SYST, strLog);

	return true;
}

// ============================================================================
// Save (full)
// ============================================================================
bool CResumeInfoMgr::SaveResumeInfo(CString strGlassID, CString strRecipeID, int nLastStick,
									bool bRunning, bool bCompleted,
									int nTotalStick, CString strLotID,
									CString strInnerID, CString strResultPath,
									int nGlsRotate)  // LYS 2026-07-21 glass rotation
{
	m_strGlassID = strGlassID;
	m_strRecipeID = strRecipeID;
	m_nLastStick = nLastStick;
	m_bRunning = bRunning;
	m_bCompleted = bCompleted;
	m_nTotalStick = nTotalStick;
	if (nGlsRotate >= 0) m_nGlsRotate = nGlsRotate;  // LYS 2026-07-21
	m_strSaveTime = GetCurrentTimeString();

	if (!strLotID.IsEmpty())	m_strLotID = strLotID;
	if (!strInnerID.IsEmpty())	m_strInnerID = strInnerID;
	if (!strResultPath.IsEmpty()) m_strResultPath = strResultPath;

	// Set start time and date on first save (inspection start)
	if (m_strStartTime.IsEmpty())
	{
		m_strStartTime = m_strSaveTime;
		m_strInspDate = GetCurrentDateString();
	}

	// Atomic save: write tmp first, then replace original
	bool bRet = WriteToFile(m_strTmpPath);
	if (bRet)
	{
		// Only delete original after tmp is confirmed on disk
		DeleteFile(m_strFilePath);
		if (!MoveFile(m_strTmpPath, m_strFilePath))
		{
			CopyFile(m_strTmpPath, m_strFilePath, FALSE);
			DeleteFile(m_strTmpPath);
		}
	}
	else
	{
		CLogWrapper::GetInstance()->KMLOG(WARN, _T("Resume Save: WriteToFile failed, keeping existing file"));
	}

	CString strLog;
	strLog.Format(_T("Resume Save: Glass=%s, Recipe=%s, Stick=%d/%d, Running=%d, OK=%d"),
		m_strGlassID, m_strRecipeID, m_nLastStick, m_nTotalStick, m_bRunning ? 1 : 0, bRet ? 1 : 0);
	CLogWrapper::GetInstance()->KMLOG(SYST, strLog);

	return bRet;
}

// ============================================================================
// UpdateStickProgress (lightweight - only updates stick number and save time)
// ============================================================================
bool CResumeInfoMgr::UpdateStickProgress(int nLastStick)
{
	m_nLastStick = nLastStick;
	m_strSaveTime = GetCurrentTimeString();

	bool bRet = WriteToFile(m_strTmpPath);
	if (bRet)
	{
		DeleteFile(m_strFilePath);
		if (!MoveFile(m_strTmpPath, m_strFilePath))
		{
			CopyFile(m_strTmpPath, m_strFilePath, FALSE);
			DeleteFile(m_strTmpPath);
		}
	}
	else
	{
		CLogWrapper::GetInstance()->KMLOG(WARN, _T("Resume Update: WriteToFile failed"));
	}

	CString strLog;
	strLog.Format(_T("Resume Update: Stick=%d/%d, OK=%d"), m_nLastStick, m_nTotalStick, bRet ? 1 : 0);
	CLogWrapper::GetInstance()->KMLOG(SYST, strLog);

	return bRet;
}

// ============================================================================
// WriteToFile (internal - writes all fields to specified path)
// ============================================================================
bool CResumeInfoMgr::WriteToFile(CString strPath)
{
	DeleteFile(strPath);

	CString strVal;

	// Identification
	WritePrivateProfileString(SECTION_RESUME, _T("GlassID"), m_strGlassID, strPath);
	WritePrivateProfileString(SECTION_RESUME, _T("RecipeID"), m_strRecipeID, strPath);
	WritePrivateProfileString(SECTION_RESUME, _T("LotID"), m_strLotID, strPath);
	WritePrivateProfileString(SECTION_RESUME, _T("InnerID"), m_strInnerID, strPath);

	// Progress
	strVal.Format(_T("%d"), m_nLastStick);
	WritePrivateProfileString(SECTION_RESUME, _T("LastStick"), strVal, strPath);

	strVal.Format(_T("%d"), m_nTotalStick);
	WritePrivateProfileString(SECTION_RESUME, _T("TotalStick"), strVal, strPath);

	strVal.Format(_T("%d"), m_nGlsRotate);  // LYS 2026-07-21
	WritePrivateProfileString(SECTION_RESUME, _T("GlsRotate"), strVal, strPath);

	strVal.Format(_T("%d"), m_bRunning ? 1 : 0);
	WritePrivateProfileString(SECTION_RESUME, _T("Running"), strVal, strPath);

	strVal.Format(_T("%d"), m_bCompleted ? 1 : 0);
	WritePrivateProfileString(SECTION_RESUME, _T("Completed"), strVal, strPath);

	// Time/Path
	WritePrivateProfileString(SECTION_RESUME, _T("SaveTime"), m_strSaveTime, strPath);
	WritePrivateProfileString(SECTION_RESUME, _T("StartTime"), m_strStartTime, strPath);
	WritePrivateProfileString(SECTION_RESUME, _T("InspDate"), m_strInspDate, strPath);
	WritePrivateProfileString(SECTION_RESUME, _T("ResultPath"), m_strResultPath, strPath);

	// Flush to disk
	WritePrivateProfileString(NULL, NULL, NULL, strPath);

	// Verify file was actually written
	if (!PathFileExists(strPath))
	{
		CLogWrapper::GetInstance()->KMLOG(WARN, _T("Resume Error: WriteToFile failed, file not created"));
		return false;
	}

	return true;
}

// ============================================================================
// Clear
// ============================================================================
void CResumeInfoMgr::ClearResumeInfo()
{
	m_strGlassID.Empty();
	m_strRecipeID.Empty();
	m_strLotID.Empty();
	m_strInnerID.Empty();
	m_nLastStick = -1;
	m_nTotalStick = 0;
	m_nGlsRotate = -1;  // LYS 2026-07-21
	m_bRunning = false;
	m_bCompleted = true;
	m_strSaveTime.Empty();
	m_strStartTime.Empty();
	m_strInspDate.Empty();
	m_strResultPath.Empty();

	DeleteFile(m_strFilePath);
	DeleteFile(m_strTmpPath);

	CLogWrapper::GetInstance()->KMLOG(SYST, _T("Resume Reset: info cleared"));
}

// ============================================================================
// IsResumeAvailable
// ============================================================================
bool CResumeInfoMgr::IsResumeAvailable() const
{
	if (!m_bRunning || m_bCompleted)
		return false;

	if (m_strGlassID.IsEmpty() || m_strRecipeID.IsEmpty())
		return false;

	if (m_nLastStick < 0)
		return false;

	if (IsExpired())
	{
		CLogWrapper::GetInstance()->KMLOG(WARN, _T("Resume Error: resume info expired"));
		return false;
	}

	return true;
}

// ============================================================================
// ValidateResume
// ============================================================================
bool CResumeInfoMgr::ValidateResume(CString strCurrentRecipeID, int nTotalStick, int nGlsRotate) const
{
	if (!IsResumeAvailable())
		return false;

	if (m_strRecipeID.CompareNoCase(strCurrentRecipeID) != 0)
	{
		CLogWrapper::GetInstance()->KMLOG(WARN, _T("Resume Error: RecipeID mismatch"));
		return false;
	}

	if (m_nTotalStick > 0 && m_nTotalStick != nTotalStick)
	{
		CLogWrapper::GetInstance()->KMLOG(WARN, _T("Resume Error: TotalStick count mismatch"));
		return false;
	}

	if (m_nLastStick + 1 >= nTotalStick)
	{
		CLogWrapper::GetInstance()->KMLOG(WARN, _T("Resume Error: LastStick exceeds total stick count"));
		return false;
	}

	// LYS 2026-07-21 Validate glass rotation angle
	if (nGlsRotate >= 0 && m_nGlsRotate >= 0 && m_nGlsRotate != nGlsRotate)
	{
		CString strLog;
		strLog.Format(_T("Resume Error: GlsRotate mismatch (saved=%d, current=%d)"), m_nGlsRotate, nGlsRotate);
		CLogWrapper::GetInstance()->KMLOG(WARN, strLog);
		return false;
	}

	return true;
}

// ============================================================================
// IsExpired
// ============================================================================
bool CResumeInfoMgr::IsExpired() const
{
	if (m_nExpireHours <= 0 || m_strSaveTime.IsEmpty())
		return false;

	// Parse SaveTime: "YYYYMMDD-HHMMSS"
	if (m_strSaveTime.GetLength() < 15)
		return false;

	SYSTEMTIME stSave;
	ZeroMemory(&stSave, sizeof(stSave));
	stSave.wYear   = (WORD)_ttoi(m_strSaveTime.Left(4));
	stSave.wMonth  = (WORD)_ttoi(m_strSaveTime.Mid(4, 2));
	stSave.wDay    = (WORD)_ttoi(m_strSaveTime.Mid(6, 2));
	stSave.wHour   = (WORD)_ttoi(m_strSaveTime.Mid(9, 2));
	stSave.wMinute = (WORD)_ttoi(m_strSaveTime.Mid(11, 2));
	stSave.wSecond = (WORD)_ttoi(m_strSaveTime.Mid(13, 2));

	FILETIME ftSave, ftNow;
	SystemTimeToFileTime(&stSave, &ftSave);

	SYSTEMTIME stNow;
	::GetLocalTime(&stNow);
	SystemTimeToFileTime(&stNow, &ftNow);

	ULARGE_INTEGER uliSave, uliNow;
	uliSave.LowPart = ftSave.dwLowDateTime;
	uliSave.HighPart = ftSave.dwHighDateTime;
	uliNow.LowPart = ftNow.dwLowDateTime;
	uliNow.HighPart = ftNow.dwHighDateTime;

	// 100-nanosecond intervals per hour
	ULONGLONG ullExpire = (ULONGLONG)m_nExpireHours * 36000000000ULL;
	if (uliNow.QuadPart > uliSave.QuadPart + ullExpire)
		return true;

	return false;
}

// ============================================================================
// Utility
// ============================================================================
CString CResumeInfoMgr::GetCurrentTimeString() const
{
	SYSTEMTIME st;
	::GetLocalTime(&st);

	CString str;
	str.Format(_T("%04d%02d%02d-%02d%02d%02d"),
		st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
	return str;
}

CString CResumeInfoMgr::GetCurrentDateString() const
{
	SYSTEMTIME st;
	::GetLocalTime(&st);

	CString str;
	str.Format(_T("%04d%02d%02d"), st.wYear, st.wMonth, st.wDay);
	return str;
}
