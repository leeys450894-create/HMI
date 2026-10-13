// ResumeInfoMgr.cpp
// LYS 2026-07-03 Resume Inspection 관리자 - 비정상 종료 후 검사 재개용 상태 INI 관리.

#include "pch.h"
#include "ResumeInfoMgr.h"
#include "ResumeJournal.h"		// LYS 2026-10-08 Hybrid Phase 5 (S_Resume* types, SetFolder, DeleteFiles, ParseSegShape)

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

static const LPCTSTR SECTION_RESUME = _T("RESUME");

// LYS 2026-10-08 Hybrid Phase 5 file-local helpers
namespace
{
	// m_cs scope lock - every return path leaves the lock (CRITICAL_SECTION is recursive: GetBookmark -> GetResumePoint)
	class CResumeInfoLock
	{
	public:
		explicit CResumeInfoLock(CRITICAL_SECTION* p) : m_p(p)	{ ::EnterCriticalSection(m_p); }
		~CResumeInfoLock()										{ ::LeaveCriticalSection(m_p); }
	private:
		CResumeInfoLock(const CResumeInfoLock&);
		CResumeInfoLock& operator=(const CResumeInfoLock&);
		CRITICAL_SECTION* m_p;
	};

	// sticks of segment nSeg in SegShape (0 = unreadable / out of range) - log text only
	int RiSegSticks(const CString& strSegShape, int nSeg)
	{
		std::vector<int> vecSticks;
		if (CResumeJournal::ParseSegShape(strSegShape, vecSticks) < 0 || nSeg < 0 || nSeg >= (int)vecSticks.size()) return 0;
		return vecSticks[nSeg];
	}

	// fingerprint "{rcp}:{aging}:{efa}:{wadmp}" -> 4 parts (false = another shape)
	bool RiSplitFingerprint(const CString& strFp, CString* pParts)
	{
		int nStart = 0;
		for (int i = 0; i < 4; i++)
		{
			int nColon = strFp.Find(_T(':'), nStart);
			if (i < 3)
			{
				if (nColon < 0) return false;
				pParts[i] = strFp.Mid(nStart, nColon - nStart);
				nStart = nColon + 1;
			}
			else
			{
				if (nColon >= 0) return false;
				pParts[i] = strFp.Mid(nStart);
			}
		}
		return true;
	}
}

CResumeInfoMgr::CResumeInfoMgr(CString strConfigPath)
	: m_nLastStick(-1)
	, m_nTotalStick(0)
	, m_nGlsRotate(-1)
	, m_bRunning(false)
	, m_bCompleted(false)
	, m_nExpireHours(0)
{
	::InitializeCriticalSection(&m_cs);		// LYS 2026-10-08 Hybrid Phase 5
	ResetTupleMembers();
	m_strFilePath.Format(_T("%s\\ResumeInfo.ini"), strConfigPath);
	m_strTmpPath.Format(_T("%s\\ResumeInfo.ini.tmp"), strConfigPath);
	CResumeJournal::GetInstance()->SetFolder(strConfigPath);	// LYS 2026-10-08 Hybrid Phase 5 journal + cell status ring next to ResumeInfo.ini (logs only when a debug key is set)

	LoadResumeInfo();

	CString strLog;
	strLog.Format(_T("ResumeInfoMgr initialized, file=%s, Running=%d, Completed=%d, LastStick=%d/%d"),
		m_strFilePath, m_bRunning ? 1 : 0, m_bCompleted ? 1 : 0, m_nLastStick + 1, m_nTotalStick);
	CLogWrapper::GetInstance()->KMLOG(SYST, strLog);

	// LYS 2026-10-08 Hybrid Phase 5 tuple bookmark state at boot (Format=2 files only - a legacy file adds no line)
	if (m_nFormat >= 2)
	{
		strLog.Format(_T("Resume tuple: Format %d, T%d seg %d/%d stick %d, pass %d (EFA %d), boundary %d, result saved %d, attempt %d, journal ok %d lines %d"),
			m_nFormat, m_nLastPass, m_nLastSegment + 1, m_nSegmentCount, m_nLastStick + 1, m_nPassCount, m_nEfaMode,
			m_nPassBoundaryDone, m_nResultSaved, m_nAttempt, m_nJournalOK, m_nJournalLines);
		CLogWrapper::GetInstance()->KMLOG(SYST, strLog);
	}
}

CResumeInfoMgr::~CResumeInfoMgr()
{
	::DeleteCriticalSection(&m_cs);		// LYS 2026-10-08 Hybrid Phase 5
}

// ============================================================================
// Load
// ============================================================================
bool CResumeInfoMgr::LoadResumeInfo()
{
	CResumeInfoLock lock(&m_cs);		// LYS 2026-10-08 Hybrid Phase 5
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
		ResetTupleMembers();		// LYS 2026-10-08 Hybrid Phase 5 tuple members too (review C14)
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

	// LYS 2026-10-08 Hybrid Phase 5 tuple bookmark keys (Format=2). A legacy file has no Format key -> 1 and every tuple member keeps its default.
	//   A Format=2 file without LastPass / LastSegment resumes as (0, 0, LastStick)
	m_nFormat = (int)GetPrivateProfileInt(SECTION_RESUME, _T("Format"), 1, m_strFilePath);
	if (m_nFormat >= 2)
	{
		m_nLastPass			= (int)GetPrivateProfileInt(SECTION_RESUME, _T("LastPass"), 0, m_strFilePath);
		m_nLastSegment		= (int)GetPrivateProfileInt(SECTION_RESUME, _T("LastSegment"), 0, m_strFilePath);
		m_nSegmentCount		= (int)GetPrivateProfileInt(SECTION_RESUME, _T("SegmentCount"), 1, m_strFilePath);
		GetPrivateProfileString(SECTION_RESUME, _T("SegShape"), _T(""), szBuf, 512, m_strFilePath);
		m_strSegShape		= szBuf;
		m_nGlassRows		= (int)GetPrivateProfileInt(SECTION_RESUME, _T("GlassRows"), 0, m_strFilePath);
		m_nPassCount		= (int)GetPrivateProfileInt(SECTION_RESUME, _T("PassCount"), 1, m_strFilePath);
		m_nEfaMode			= (int)GetPrivateProfileInt(SECTION_RESUME, _T("EfaMode"), 0, m_strFilePath);
		m_nEfaFlag			= (int)GetPrivateProfileInt(SECTION_RESUME, _T("EfaFlag"), 0, m_strFilePath);
		m_nPassBoundaryDone	= (int)GetPrivateProfileInt(SECTION_RESUME, _T("PassBoundaryDone"), 0, m_strFilePath);
		m_nResultSaved		= (int)GetPrivateProfileInt(SECTION_RESUME, _T("ResultSaved"), 0, m_strFilePath);
		m_nAttempt			= (int)GetPrivateProfileInt(SECTION_RESUME, _T("Attempt"), 0, m_strFilePath);
		m_nJournalOK		= (int)GetPrivateProfileInt(SECTION_RESUME, _T("JournalOK"), 1, m_strFilePath);
		m_nJournalLines		= (int)GetPrivateProfileInt(SECTION_RESUME, _T("JournalLines"), 0, m_strFilePath);
		GetPrivateProfileString(SECTION_RESUME, _T("Fingerprint"), _T(""), szBuf, 256, m_strFilePath);
		m_strFingerprint	= szBuf;
		if (m_nSegmentCount < 1)	m_nSegmentCount = 1;
		if (m_nPassCount < 1)		m_nPassCount = 1;
	}
	else
		ResetTupleMembers();

	CString strLog;
	strLog.Format(_T("Resume Load: Glass=%s, Recipe=%s, Lot=%s, Stick=%d/%d, Running=%d, Completed=%d, Date=%s"),
		m_strGlassID, m_strRecipeID, m_strLotID, m_nLastStick + 1, m_nTotalStick,
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
	CResumeInfoLock lock(&m_cs);		// LYS 2026-10-08 Hybrid Phase 5
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

	// LYS 2026-10-08 Hybrid Phase 5 this full save starts a legacy (Format 1) bookmark of a new glass (only caller: engine start, LastStick -1):
	//   drop the tuple state and the journal / cell status ring of an unfinished earlier glass (silent when there is none, so a 1-segment
	//   run keeps its log and INI bytes). A journal glass turns it into Format 2 at measure start (BeginTupleGlass)
	ResetTupleMembers();
	CResumeJournal::GetInstance()->DeleteFiles();

	// Atomic save: write tmp first, then replace original
	bool bRet = WriteToFile(m_strTmpPath);
	if (bRet)
	{
		// Only delete original after tmp is confirmed on disk
		ReplaceIniFromTmp();		// LYS 2026-10-08 Hybrid Phase 5 (D5-18 a) MoveFileEx replace - the delete + move / copy that stood here is its fallback
	}
	else
	{
		CLogWrapper::GetInstance()->KMLOG(WARN, _T("Resume Save: WriteToFile failed, keeping existing file"));
	}

	CString strLog;
	strLog.Format(_T("Resume Save: Glass=%s, Recipe=%s, Stick=%d/%d, Running=%d, OK=%d"),
		m_strGlassID, m_strRecipeID, m_nLastStick + 1, m_nTotalStick, m_bRunning ? 1 : 0, bRet ? 1 : 0);
	CLogWrapper::GetInstance()->KMLOG(SYST, strLog);

	return bRet;
}

// ============================================================================
// UpdateStickProgress (lightweight - only updates stick number and save time)
// ============================================================================
bool CResumeInfoMgr::UpdateStickProgress(int nLastStick)
{
	CResumeInfoLock lock(&m_cs);		// LYS 2026-10-08 Hybrid Phase 5
	m_nLastStick = nLastStick;
	m_strSaveTime = GetCurrentTimeString();

	bool bRet = WriteToFile(m_strTmpPath);
	if (bRet)
	{
		ReplaceIniFromTmp();		// LYS 2026-10-08 Hybrid Phase 5 (D5-18 a) MoveFileEx replace - the delete + move / copy that stood here is its fallback
	}
	else
	{
		CLogWrapper::GetInstance()->KMLOG(WARN, _T("Resume Update: WriteToFile failed"));
	}

	CString strLog;
	strLog.Format(_T("Resume Update: Stick=%d/%d, OK=%d"), m_nLastStick + 1, m_nTotalStick, bRet ? 1 : 0);
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

	// LYS 2026-10-08 Hybrid Phase 5 tuple bookmark keys after the legacy ones - only for a Format 2 bookmark, so a legacy file keeps its bytes
	if (m_nFormat >= 2)
	{
		strVal.Format(_T("%d"), m_nFormat);
		WritePrivateProfileString(SECTION_RESUME, _T("Format"), strVal, strPath);
		strVal.Format(_T("%d"), m_nLastPass);
		WritePrivateProfileString(SECTION_RESUME, _T("LastPass"), strVal, strPath);
		strVal.Format(_T("%d"), m_nLastSegment);
		WritePrivateProfileString(SECTION_RESUME, _T("LastSegment"), strVal, strPath);
		strVal.Format(_T("%d"), m_nSegmentCount);
		WritePrivateProfileString(SECTION_RESUME, _T("SegmentCount"), strVal, strPath);
		WritePrivateProfileString(SECTION_RESUME, _T("SegShape"), m_strSegShape, strPath);
		strVal.Format(_T("%d"), m_nGlassRows);
		WritePrivateProfileString(SECTION_RESUME, _T("GlassRows"), strVal, strPath);
		strVal.Format(_T("%d"), m_nPassCount);
		WritePrivateProfileString(SECTION_RESUME, _T("PassCount"), strVal, strPath);
		strVal.Format(_T("%d"), m_nEfaMode);
		WritePrivateProfileString(SECTION_RESUME, _T("EfaMode"), strVal, strPath);
		strVal.Format(_T("%d"), m_nEfaFlag);
		WritePrivateProfileString(SECTION_RESUME, _T("EfaFlag"), strVal, strPath);
		strVal.Format(_T("%d"), m_nPassBoundaryDone);
		WritePrivateProfileString(SECTION_RESUME, _T("PassBoundaryDone"), strVal, strPath);
		strVal.Format(_T("%d"), m_nResultSaved);
		WritePrivateProfileString(SECTION_RESUME, _T("ResultSaved"), strVal, strPath);
		strVal.Format(_T("%d"), m_nAttempt);
		WritePrivateProfileString(SECTION_RESUME, _T("Attempt"), strVal, strPath);
		strVal.Format(_T("%d"), m_nJournalOK);
		WritePrivateProfileString(SECTION_RESUME, _T("JournalOK"), strVal, strPath);
		strVal.Format(_T("%d"), m_nJournalLines);
		WritePrivateProfileString(SECTION_RESUME, _T("JournalLines"), strVal, strPath);
		WritePrivateProfileString(SECTION_RESUME, _T("Fingerprint"), m_strFingerprint, strPath);
	}

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
	CResumeInfoLock lock(&m_cs);		// LYS 2026-10-08 Hybrid Phase 5
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
	ResetTupleMembers();							// LYS 2026-10-08 Hybrid Phase 5 tuple state (ICP "First" defect not ported: everything goes)
	CResumeJournal::GetInstance()->DeleteFiles();	// journal + cell status ring + tmp, silent when absent (lock order m_cs -> journal)

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

// ============================================================================
// LYS 2026-10-08 Hybrid Phase 5 tuple bookmark (Format=2) - journal glasses only (CResumeJournal::IsTupleGlass), scratchpad p5_plan.md (1) (6-2)
//   Logs: "Resume Begin / Boundary / ResultSaved" and the tuple form of "Resume Update" only for N > 1. A 1-segment journal glass
//   (RESUME_TUPLE_ALL=1) logs the legacy "Resume Update: Stick=" line and nothing new in a normal run (D5-2 b).
//   Lock order m_cs -> journal lock; nothing here runs under the journal lock.
// ============================================================================
void CResumeInfoMgr::ResetTupleMembers()
{
	m_nFormat			= 1;
	m_nLastPass			= 0;
	m_nLastSegment		= 0;
	m_nSegmentCount		= 1;
	m_strSegShape.Empty();
	m_nGlassRows		= 0;
	m_nPassCount		= 1;
	m_nEfaMode			= 0;
	m_nEfaFlag			= 0;
	m_nPassBoundaryDone	= 0;
	m_nResultSaved		= 0;
	m_nAttempt			= 0;
	m_nJournalOK		= 1;
	m_nJournalLines		= 0;
	m_strFingerprint.Empty();
}

void CResumeInfoMgr::ReplaceIniFromTmp()
{
	// D5-18 (a): one-step replace, so there is no moment without the INI. The legacy delete + move / copy stays as the fallback
	if (MoveFileEx(m_strTmpPath, m_strFilePath, MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
		return;
	DeleteFile(m_strFilePath);
	if (!MoveFile(m_strTmpPath, m_strFilePath))
	{
		CopyFile(m_strTmpPath, m_strFilePath, FALSE);
		DeleteFile(m_strTmpPath);
	}
}

bool CResumeInfoMgr::SaveTupleLocked()
{
	m_strSaveTime = GetCurrentTimeString();
	bool bRet = WriteToFile(m_strTmpPath);
	if (bRet) ReplaceIniFromTmp();
	return bRet;
}

// ============================================================================
// BeginTupleGlass - measure start of a journal glass (new glass, cycle index > 0 included)
// ============================================================================
bool CResumeInfoMgr::BeginTupleGlass(CString strGlassID, CString strRecipeID, CString strLotID, CString strInnerID, CString strResultPath,
									 int nTotalStick0, int nGlsRotate0, const S_ResumeShape& shape, int nPassCount, int nEfaMode, int nEfaFlag, bool bLog)
{
	CResumeInfoLock lock(&m_cs);

	// identification of THIS glass (a cycle glass with index > 0 too - D11) and a new start time / date (D12)
	m_strGlassID		= strGlassID;
	m_strRecipeID		= strRecipeID;
	m_strLotID			= strLotID;
	m_strInnerID		= strInnerID;
	m_strResultPath		= strResultPath;
	m_nLastStick		= -1;
	m_nTotalStick		= nTotalStick0;		// segment 0 values, as the legacy keys always had them
	m_nGlsRotate		= nGlsRotate0;
	m_bRunning			= true;
	m_bCompleted		= false;

	ResetTupleMembers();
	m_nFormat			= 2;
	m_nSegmentCount		= (shape.nSegCount > 0) ? shape.nSegCount : 1;
	m_strSegShape		= shape.strSegShape;
	m_nGlassRows		= shape.nGlassRows;
	m_nPassCount		= (nPassCount > 0) ? nPassCount : 1;
	m_nEfaMode			= (nEfaMode != 0) ? 1 : 0;
	m_nEfaFlag			= (nEfaFlag != 0) ? 1 : 0;
	m_strFingerprint	= shape.strFingerprint;

	m_strSaveTime		= GetCurrentTimeString();
	m_strStartTime		= m_strSaveTime;
	m_strInspDate		= m_strSaveTime.Left(8);	// YYYYMMDD, same form as GetCurrentDateString
	bool bRet = WriteToFile(m_strTmpPath);
	if (bRet) ReplaceIniFromTmp();
	else CLogWrapper::GetInstance()->KMLOG(WARN, _T("Resume Begin: WriteToFile failed"));

	if (bLog)
	{
		CString strLog;
		strLog.Format(_T("Resume Begin: Glass=%s, Recipe=%s, segments %d, pass %d, G %d, OK=%d"),
			m_strGlassID.GetString(), m_strRecipeID.GetString(), m_nSegmentCount, m_nPassCount, m_nGlassRows, bRet ? 1 : 0);
		CLogWrapper::GetInstance()->KMLOG(SYST, strLog);
	}
	return bRet;
}

// ============================================================================
// UpdateTupleProgress - stick done (journal Commit -> cell status snapshot -> this INI write)
// ============================================================================
bool CResumeInfoMgr::UpdateTupleProgress(int nPass, int nSeg, int nStick, bool bJournalOK, int nJournalLines)
{
	CResumeInfoLock lock(&m_cs);
	if (m_nFormat < 2)
	{
		CLogWrapper::GetInstance()->KMLOG(WARN, _T("Resume Update: not a journal bookmark - tuple progress not saved"));
		return false;
	}

	bool bWasOK = (m_nJournalOK != 0);
	m_nLastPass			= nPass;
	m_nLastSegment		= nSeg;
	m_nLastStick		= nStick;
	if (!bJournalOK) m_nJournalOK = 0;		// sticky (review K7): one failed journal write makes this glass non-resumable
	m_nJournalLines		= nJournalLines;

	bool bRet = SaveTupleLocked();
	if (!bRet) CLogWrapper::GetInstance()->KMLOG(WARN, _T("Resume Update: WriteToFile failed"));

	CString strLog;
	if (m_nSegmentCount > 1)
	{
		strLog.Format(_T("Resume Update: T%d seg %d/%d stick %d/%d, OK=%d"),
			nPass, nSeg + 1, m_nSegmentCount, nStick + 1, RiSegSticks(m_strSegShape, nSeg), bRet ? 1 : 0);
		CLogWrapper::GetInstance()->KMLOG(SYST, strLog);
		if (bWasOK && m_nJournalOK == 0)
			CLogWrapper::GetInstance()->KMLOG(WARN, _T("Resume Update: journal write failed earlier - this glass will not be resumable"));
	}
	else
	{	// 1-segment journal glass: the legacy UpdateStickProgress line, byte for byte
		strLog.Format(_T("Resume Update: Stick=%d/%d, OK=%d"), m_nLastStick + 1, m_nTotalStick, bRet ? 1 : 0);
		CLogWrapper::GetInstance()->KMLOG(SYST, strLog);
	}
	return bRet;
}

// ============================================================================
// SetPassBoundaryDone - inside EfaPassBoundary once its side effects are done (T0 flush / EFA^0$ sent)
// ============================================================================
bool CResumeInfoMgr::SetPassBoundaryDone(int nPass)
{
	CResumeInfoLock lock(&m_cs);
	if (m_nFormat < 2) return false;		// legacy bookmark: no flag, the boundary always runs again
	if (nPass > m_nPassBoundaryDone) m_nPassBoundaryDone = nPass;	// never lowers

	bool bRet = SaveTupleLocked();
	if (m_nSegmentCount > 1)
	{
		CString strLog;
		strLog.Format(_T("Resume Boundary: pass %d boundary done, OK=%d"), nPass, bRet ? 1 : 0);
		CLogWrapper::GetInstance()->KMLOG(SYST, strLog);
	}
	else if (!bRet)
		CLogWrapper::GetInstance()->KMLOG(WARN, _T("Resume Boundary: WriteToFile failed"));
	return bRet;
}

// ============================================================================
// MarkResultSaved - 1 = Result item done (SaveResult export started), 2 = result handled (Unload SaveResultDone / save gate 0)
// ============================================================================
bool CResumeInfoMgr::MarkResultSaved(int nLevel)
{
	CResumeInfoLock lock(&m_cs);
	if (m_nFormat < 2) return false;			// legacy bookmark: nothing to mark
	if (nLevel > 2) nLevel = 2;
	if (nLevel <= m_nResultSaved) return true;	// never lowers (only the pull-back of ApplyCompaction resets it)
	m_nResultSaved = nLevel;

	bool bRet = SaveTupleLocked();
	if (m_nSegmentCount > 1)
	{
		CString strLog;
		strLog.Format(_T("Resume ResultSaved: %d, OK=%d"), m_nResultSaved, bRet ? 1 : 0);
		CLogWrapper::GetInstance()->KMLOG(SYST, strLog);
	}
	else if (!bRet)
		CLogWrapper::GetInstance()->KMLOG(WARN, _T("Resume ResultSaved: WriteToFile failed"));
	return bRet;
}

// ============================================================================
// ApplyCompaction - resume measure entry, between the journal tmp write and the journal swap (INI first - review C1/K1)
//   A failed INI write restores the members, so memory = file and the caller must not swap the journal
// ============================================================================
bool CResumeInfoMgr::ApplyCompaction(bool bRewind, const S_ResumeTuple& last, int nKeptLines)
{
	CResumeInfoLock lock(&m_cs);
	if (m_nFormat < 2) return false;

	int nAttempt0 = m_nAttempt, nLines0 = m_nJournalLines, nPass0 = m_nLastPass, nSeg0 = m_nLastSegment, nStick0 = m_nLastStick, nSaved0 = m_nResultSaved;
	CString strSaveTime0 = m_strSaveTime;

	m_nAttempt++;
	m_nJournalLines = (nKeptLines > 0) ? nKeptLines : 0;
	if (bRewind)
	{	// VISION pull-back (D5-12): Last* = pred(Tp), so a stop before Tp finishes resumes at Tp again
		m_nLastPass		= last.nPass;
		m_nLastSegment	= last.nSeg;
		m_nLastStick	= last.nStick;
		m_nResultSaved	= 0;
	}

	bool bRet = SaveTupleLocked();

	CString strLog;
	if (bRet)
	{
		strLog.Format(_T("Resume Attempt: %d, journal lines %d"), m_nAttempt, m_nJournalLines);
		CLogWrapper::GetInstance()->KMLOG(SYST, strLog);
		if (bRewind)
		{
			strLog.Format(_T("Resume Rewind: T%d seg %d/%d stick %d/%d (VISION pending), OK=%d"),
				last.nPass, last.nSeg + 1, m_nSegmentCount, last.nStick + 1, RiSegSticks(m_strSegShape, last.nSeg), bRet ? 1 : 0);
			CLogWrapper::GetInstance()->KMLOG(SYST, strLog);
		}
	}
	else
	{
		strLog.Format(_T("Resume Attempt: WriteToFile failed (attempt %d, journal lines %d, rewind %d) - bookmark unchanged, the journal must not be swapped"),
			m_nAttempt, m_nJournalLines, bRewind ? 1 : 0);
		m_nAttempt = nAttempt0; m_nJournalLines = nLines0; m_nLastPass = nPass0; m_nLastSegment = nSeg0; m_nLastStick = nStick0; m_nResultSaved = nSaved0;
		m_strSaveTime = strSaveTime0;
		CLogWrapper::GetInstance()->KMLOG(WARN, strLog);
	}
	return bRet;
}

// ============================================================================
// GetResumePoint - plan (1-2): (LastPass, LastSegment, LastStick + 1) normalized over SegShape
// ============================================================================
int CResumeInfoMgr::GetResumePoint(int& rPass, int& rSeg, int& rStick) const
{
	CResumeInfoLock lock(&m_cs);
	rPass = rSeg = rStick = 0;
	if (m_nFormat < 2) return -1;

	std::vector<int> vecSticks;
	if (CResumeJournal::ParseSegShape(m_strSegShape, vecSticks) != m_nSegmentCount) return -1;
	S_ResumeTuple last, out;
	last.nPass	= m_nLastPass;
	last.nSeg	= m_nLastSegment;
	last.nStick	= m_nLastStick;
	int nRet = CResumeJournal::ResumePointFromLast(last, m_nSegmentCount, &vecSticks[0], m_nPassCount, out);
	if (nRet >= 0)
	{
		rPass	= out.nPass;
		rSeg	= out.nSeg;
		rStick	= out.nStick;
	}
	return nRet;
}

// ============================================================================
// ValidateTupleShape - popup checks #2 RecipeID, #3 segment shape, #4 file fingerprint (plan (2-2)); no log (the popup logs the refusal)
// ============================================================================
bool CResumeInfoMgr::ValidateTupleShape(const CString& strRecipeID, const S_ResumeShape& cur, CString& strWhyEn, int* pnFail) const
{
	CResumeInfoLock lock(&m_cs);
	int nFail = RESUME_SHAPE_OK;
	strWhyEn.Empty();

	if (m_nFormat < 2)
	{
		nFail = RESUME_SHAPE_SEGMENTS;
		strWhyEn = _T("not a journal bookmark");
	}
	else if (m_strRecipeID.CompareNoCase(strRecipeID) != 0)
	{
		nFail = RESUME_SHAPE_RECIPE;
		strWhyEn = _T("RecipeID mismatch");
	}
	else if (m_nSegmentCount != cur.nSegCount || m_strSegShape.Compare(cur.strSegShape) != 0 || m_nGlassRows != cur.nGlassRows)
	{
		nFail = RESUME_SHAPE_SEGMENTS;
		strWhyEn.Format(_T("segment shape mismatch (saved %s, now %s)"), m_strSegShape.GetString(), cur.strSegShape.GetString());
	}
	else if (m_strFingerprint.Compare(cur.strFingerprint) != 0)
	{
		static const LPCTSTR s_szPart[4] = { _T("rcp"), _T("aging"), _T("efa"), _T("wadmp") };
		CString arrSaved[4], arrNow[4], strList;
		if (RiSplitFingerprint(m_strFingerprint, arrSaved) && RiSplitFingerprint(cur.strFingerprint, arrNow))
		{
			for (int i = 0; i < 4; i++)
			{
				if (arrSaved[i] == arrNow[i]) continue;
				if (!strList.IsEmpty()) strList += _T(", ");
				strList += s_szPart[i];
				if (arrNow[i] == _T("?")) strList += _T(" unreadable");
			}
		}
		if (strList.IsEmpty()) strList = _T("fingerprint");
		nFail = RESUME_SHAPE_FILES;
		strWhyEn.Format(_T("files changed (%s)"), strList.GetString());
	}

	if (pnFail != NULL) *pnFail = nFail;
	return (nFail == RESUME_SHAPE_OK);
}

// ============================================================================
// GetBookmark - one consistent copy for CResumeJournal::Analyze / WriteCompacted and the popup
// ============================================================================
void CResumeInfoMgr::GetBookmark(S_ResumeBookmark& out) const
{
	CResumeInfoLock lock(&m_cs);
	out = S_ResumeBookmark();
	out.strGlassID			= m_strGlassID;
	out.strRecipeID			= m_strRecipeID;
	out.strSegShape			= m_strSegShape;
	out.nSegCount			= m_nSegmentCount;
	out.nPassCount			= m_nPassCount;
	out.nEfaMode			= m_nEfaMode;
	out.nGlassRows			= m_nGlassRows;
	out.nPassBoundaryDone	= m_nPassBoundaryDone;
	out.nJournalOK			= m_nJournalOK;
	out.nJournalLines		= m_nJournalLines;
	out.nAttempt			= m_nAttempt;
	out.last.nPass			= m_nLastPass;
	out.last.nSeg			= m_nLastSegment;
	out.last.nStick			= m_nLastStick;
	out.nPoint				= GetResumePoint(out.T.nPass, out.T.nSeg, out.T.nStick);	// legacy bookmark: -1, T (0,0,0)
}

CString CResumeInfoMgr::GetSegShape() const
{
	CResumeInfoLock lock(&m_cs);
	return m_strSegShape;
}

CString CResumeInfoMgr::GetFingerprint() const
{
	CResumeInfoLock lock(&m_cs);
	return m_strFingerprint;
}
