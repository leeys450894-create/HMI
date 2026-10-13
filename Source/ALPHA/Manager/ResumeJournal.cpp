// ResumeJournal.cpp
// LYS 2026-10-06 Hybrid Phase 5 resume journal - see ResumeJournal.h. Design: scratchpad p5_plan.md (4) (5) (6-1), decisions D5-3/4/5/12/17.
//   Line format (ASCII/UTF-8, CRLF, ',' separated, '%' ',' CR LF TAB inside a field escaped as %25 %2C %0D %0A %09, last field '$'):
//   #RJ1,<GlassID>,<RecipeID>,<SegCount>,<PassCount>,<G>,<BeginTime>,<Attempt>
//   R,<seq>,<att>,<pass>,<seg>,<stick>,<gstick>,<kind>,<nMeasMode>,<iMeasArea>,<m_iProbe>,<iCurArea>,<iCurStep>,<iPanelNo>,<nUseResultFile>,
//     <m_iTValue>,<m_nMeasType>,<m_nMeasAreaCnt>,<m_nMaxMeasureRcpFlag>,<m_iRetryCnt>,<m_iRetryReason>,<nMultiPoint>,<dOffsetRX>,<dOffsetGX>,<dOffsetBY>,
//     <nCntData>,{<szName>,<szData>,<nReason>}*nCntData,<szCellData>,<szLotID>,<szGlassID>,<szRecipeID>,<szResultPath>,<szResultFile>,<szImageFile>,$
//   P,<seq>,<att>,<pass>,<seg>,<stick>,<gstick>,VISION,<head>,<iMeasArea>,<InnerID>,$
//   X,<seq>,<att>,<pass>,<seg>,<stick>,<gstick>,VISION,<head>,<iMeasArea>,<InnerID>,TIMEOUT,$
//   S,<seq>,<att>,<pass>,<seg>,<stick>,<gstick>,<kind>,<nMeasMode>,<iMeasArea>,<CellID>,$
#include "pch.h"
#include "ResumeJournal.h"
#include "ResultMgr.h"
#include "Recipe\RecipeEditMgr.h"
#include "DoWork\AgingCycleInfo.h"		// GetAgingRcpPath / GetEfaRcpPath, common\CommonFunc.h (PGM_MODE_*)
#include <map>
#include <deque>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define RJ_CELLSTATE_IDLE	0						// df_CELLSTATE_ILDE (ShareMem.h)
#define RJ_MAX_FILE			(64 * 1024 * 1024)		// journal read limit (plan upper bound 15,000 lines ~ 6 MB)
#define RJ_SETTLE_POLLS		110						// replay (a): dResult idle this many 10 ms polls before the second ClearData (a row in its 1 s display wait is 100 polls)
#define RJ_SIM_FAIL_WRITE	5						// RESUME_JOURNAL_OOS_SIM 4: this data line write fails on purpose

// QualityWaitThread override (SetThreadCtx)
static thread_local S_ResumeCtx t_ctxOverride = { 0, 0, 0, 0, 0, 0 };
static thread_local bool t_bOverride = false;

// cell status ring slot file
#pragma pack(push, 1)
struct S_RcsFile
{
	char	szMagic[4];							// "RCS1"
	int		nVersion;							// 1
	WCHAR	szGlassID[32];
	int		nAttempt;
	int		nSerial;							// monotonic within the glass, slot = serial % 3
	int		nPass, nSeg, nStick;
	int		nStatus[RESUME_CELL_X][RESUME_CELL_Y];
	DWORD	dwFnv;								// FNV-1a of every byte before it
};
#pragma pack(pop)

// parsed journal data line
struct S_JLine
{
	CStringA		strRaw;		// without CRLF
	char			cType;		// 'R' 'P' 'X' 'S'
	int				nSeq;
	S_ResumeTuple	t;
	int				nMode;		// R / S nMeasMode
	int				nArea;
	int				nHead;		// P / X
	CString			strInner;	// P / X InnerID, R: result file name without extension
	bool			bRawOK;		// R: szResultPath + szResultFile exists
	bool			bClosed;	// P: closed by X / R VISION (raw present) / S VISION
	bool			bKeep;
	S_JLine() : cType(0), nSeq(0), nMode(-1), nArea(-1), nHead(-1), bRawOK(false), bClosed(false), bKeep(false) { t.nPass = t.nSeg = t.nStick = 0; }
};

//----------------------------------------------------------------------------------------------------
// string / file helpers
//----------------------------------------------------------------------------------------------------
static CStringA RjU8(const WCHAR* sz, size_t nMax)
{
	CStringA s;
	if (sz == NULL || nMax == 0) return s;
	size_t n = wcsnlen(sz, nMax);
	if (n == 0) return s;
	int nb = ::WideCharToMultiByte(CP_UTF8, 0, sz, (int)n, NULL, 0, NULL, NULL);
	if (nb <= 0) return s;
	char* p = s.GetBuffer(nb);
	::WideCharToMultiByte(CP_UTF8, 0, sz, (int)n, p, nb, NULL, NULL);
	s.ReleaseBuffer(nb);
	return s;
}

static CStringA RjU8(const CString& s)
{
	return RjU8(s.GetString(), (size_t)s.GetLength());
}

static CString RjFromU8(const CStringA& s)
{
	CString w;
	if (s.IsEmpty()) return w;
	int n = ::MultiByteToWideChar(CP_UTF8, 0, s.GetString(), s.GetLength(), NULL, 0);
	if (n <= 0) return w;
	WCHAR* p = w.GetBuffer(n);
	::MultiByteToWideChar(CP_UTF8, 0, s.GetString(), s.GetLength(), p, n);
	w.ReleaseBuffer(n);
	return w;
}

static CStringA RjEsc(const CStringA& s)
{
	CStringA o;
	for (int i = 0; i < s.GetLength(); i++)
	{
		char c = s[i];
		if (c == '%')		o += "%25";
		else if (c == ',')	o += "%2C";
		else if (c == '\r')	o += "%0D";
		else if (c == '\n')	o += "%0A";
		else if (c == '\t')	o += "%09";
		else				o.AppendChar(c);
	}
	return o;
}

static CStringA RjEscW(const WCHAR* sz, size_t nMax)	{ return RjEsc(RjU8(sz, nMax)); }
static CStringA RjEscS(const CString& s)				{ return RjEsc(RjU8(s)); }

static int RjHex(char c)
{
	if (c >= '0' && c <= '9') return c - '0';
	if (c >= 'A' && c <= 'F') return c - 'A' + 10;
	if (c >= 'a' && c <= 'f') return c - 'a' + 10;
	return -1;
}

static CStringA RjUnesc(const CStringA& s)
{
	CStringA o;
	int n = s.GetLength();
	for (int i = 0; i < n; i++)
	{
		char c = s[i];
		if (c == '%' && i + 2 < n)
		{
			int h = RjHex(s[i + 1]), l = RjHex(s[i + 2]);
			if (h >= 0 && l >= 0) { o.AppendChar((char)(h * 16 + l)); i += 2; continue; }
		}
		o.AppendChar(c);
	}
	return o;
}

static CString RjField(const CStringA& f)	{ return RjFromU8(RjUnesc(f)); }

static void RjSplit(const CStringA& line, std::vector<CStringA>& f)
{
	f.clear();
	int nStart = 0;
	for (;;)
	{
		int n = line.Find(',', nStart);
		if (n < 0) { f.push_back(line.Mid(nStart)); break; }
		f.push_back(line.Mid(nStart, n - nStart));
		nStart = n + 1;
	}
}

static bool RjToInt(const CStringA& s, int& v)
{
	if (s.IsEmpty()) return false;
	char* e = NULL;
	long l = strtol(s.GetString(), &e, 10);
	if (e == NULL || *e != 0) return false;
	v = (int)l;
	return true;
}

static int RjInt(const CStringA& s, int nDef)	{ int v = 0; return RjToInt(s, v) ? v : nDef; }

static void RjPut(WCHAR* dst, size_t n, const CString& s)	{ wcsncpy_s(dst, n, s.GetString(), _TRUNCATE); }

static const char* RjKindOfMode(int nMode)
{
	switch (nMode)
	{
	case PGM_MODE_LUMIN:	return "WAD";
	case PGM_MODE_VISION:	return "VISION";
	case PGM_MODE_ANGLE:	return "ANGLE";
	case PGM_MODE_COLOR:	return "COLOR";
	default:				break;
	}
	return "OTHER";
}

static int RjModeOfKind(const TCHAR* szKind)
{
	CString s(szKind);
	if (s.CompareNoCase(_T("VISION")) == 0)	return PGM_MODE_VISION;
	if (s.CompareNoCase(_T("ANGLE")) == 0)	return PGM_MODE_ANGLE;
	if (s.CompareNoCase(_T("COLOR")) == 0)	return PGM_MODE_COLOR;
	return PGM_MODE_LUMIN;		// WAD, WAD-MP
}

static bool RjFileExists(const CString& s)
{
	if (s.IsEmpty()) return false;
	DWORD a = ::GetFileAttributes(s);
	return (a != INVALID_FILE_ATTRIBUTES) && ((a & FILE_ATTRIBUTE_DIRECTORY) == 0);
}

// whole file from llOffset; 0 = OK, else a Win32 error (ERROR_FILE_NOT_FOUND / ERROR_PATH_NOT_FOUND = missing)
static DWORD RjReadAll(const CString& strPath, std::vector<char>& buf, LONGLONG llOffset, LONGLONG* pllSize)
{
	buf.clear();
	if (pllSize) *pllSize = 0;
	if (strPath.IsEmpty()) return ERROR_PATH_NOT_FOUND;
	HANDLE h = ::CreateFile(strPath, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (h == INVALID_HANDLE_VALUE) { DWORD e = ::GetLastError(); return (e != 0) ? e : ERROR_GEN_FAILURE; }
	DWORD dwRet = 0;
	LARGE_INTEGER li;
	li.QuadPart = 0;
	if (!::GetFileSizeEx(h, &li)) dwRet = ::GetLastError();
	else
	{
		if (pllSize) *pllSize = li.QuadPart;
		LONGLONG llLen = li.QuadPart - llOffset;
		if (llLen > RJ_MAX_FILE) dwRet = ERROR_FILE_TOO_LARGE;
		else if (llLen > 0)
		{
			LARGE_INTEGER liPos;
			liPos.QuadPart = llOffset;
			if (llOffset > 0 && !::SetFilePointerEx(h, liPos, NULL, FILE_BEGIN)) dwRet = ::GetLastError();
			else
			{
				buf.resize((size_t)llLen);
				size_t nDone = 0;
				while (nDone < buf.size())
				{
					DWORD dwGot = 0;
					if (!::ReadFile(h, &buf[nDone], (DWORD)(buf.size() - nDone), &dwGot, NULL)) { dwRet = ::GetLastError(); break; }
					if (dwGot == 0) break;
					nDone += dwGot;
				}
				buf.resize(nDone);
			}
		}
	}
	::CloseHandle(h);
	if (dwRet != 0) buf.clear();
	return dwRet;
}

// split into lines; bLastComplete = the buffer ends with '\n'
static void RjSplitLines(const std::vector<char>& buf, std::vector<CStringA>& vec, bool& bLastComplete)
{
	vec.clear();
	bLastComplete = true;
	size_t n = buf.size(), s = 0;
	while (s < n)
	{
		size_t e = s;
		while (e < n && buf[e] != '\n') e++;
		size_t len = e - s;
		if (e < n && len > 0 && buf[e - 1] == '\r') len--;
		vec.push_back(CStringA(&buf[s], (int)len));
		if (e >= n) { bLastComplete = false; break; }
		s = e + 1;
	}
}

static DWORD RjWriteAll(HANDLE h, const char* p, DWORD n)
{
	DWORD dwDone = 0;
	while (dwDone < n)
	{
		DWORD w = 0;
		if (!::WriteFile(h, p + dwDone, n - dwDone, &w, NULL)) { DWORD e = ::GetLastError(); return (e != 0) ? e : ERROR_WRITE_FAULT; }
		if (w == 0) return ERROR_WRITE_FAULT;
		dwDone += w;
	}
	return 0;
}

static void RjLog(enumLogLevel lv, const CString& s)
{
	if (!s.IsEmpty()) CLogWrapper::GetInstance()->KMLOG(lv, s);
}

static S_ResumeTuple RjTup(int p, int s, int k)	{ S_ResumeTuple t; t.nPass = p; t.nSeg = s; t.nStick = k; return t; }
static bool RjIsZero(const S_ResumeTuple& t)	{ return t.nPass == 0 && t.nSeg == 0 && t.nStick == 0; }

// R line -> RSRITEMDATA (f = SplitCsv of the line); false = malformed
static bool RjParseR(const std::vector<CStringA>& f, RSRITEMDATA* rd)
{
	if (f.size() < 34 || f[0] != "R") return false;
	int nCnt = 0;
	if (!RjToInt(f[25], nCnt) || nCnt < 0 || nCnt > 100) return false;
	if (f.size() != (size_t)(34 + 3 * nCnt) || f.back() != "$") return false;
	if (rd == NULL) return true;
	ZeroMemory(rd, sizeof(RSRITEMDATA));
	rd->nMeasMode			= RjInt(f[8], 0);
	rd->iMeasArea			= RjInt(f[9], 0);
	rd->m_iProbe			= RjInt(f[10], 0);
	rd->iCurArea			= RjInt(f[11], 0);
	rd->iCurStep			= RjInt(f[12], 0);
	rd->iPanelNo			= RjInt(f[13], 0);
	rd->nUseResultFile		= RjInt(f[14], 0);
	rd->m_iTValue			= RjInt(f[15], 0);
	rd->m_nMeasType			= RjInt(f[16], 0);
	rd->m_nMeasAreaCnt		= RjInt(f[17], 0);
	rd->m_nMaxMeasureRcpFlag = RjInt(f[18], 0);
	rd->m_iRetryCnt			= RjInt(f[19], 0);
	rd->m_iRetryReason		= RjInt(f[20], 0);
	rd->nMultiPoint			= RjInt(f[21], 0);
	rd->dOffsetRX			= strtod(f[22].GetString(), NULL);
	rd->dOffsetGX			= strtod(f[23].GetString(), NULL);
	rd->dOffsetBY			= strtod(f[24].GetString(), NULL);
	rd->nCntData			= nCnt;
	size_t i = 26;
	for (int k = 0; k < nCnt; k++, i += 3)
	{
		RjPut(rd->stData[k].szName, _countof(rd->stData[k].szName), RjField(f[i]));
		RjPut(rd->stData[k].szData, _countof(rd->stData[k].szData), RjField(f[i + 1]));
		rd->stData[k].nReason = RjInt(f[i + 2], 0);
	}
	RjPut(rd->szCellData,	_countof(rd->szCellData),	RjField(f[i]));
	RjPut(rd->szLotID,		_countof(rd->szLotID),		RjField(f[i + 1]));
	RjPut(rd->szGlassID,		_countof(rd->szGlassID),	RjField(f[i + 2]));
	RjPut(rd->szRecipeID,	_countof(rd->szRecipeID),	RjField(f[i + 3]));
	RjPut(rd->szResultPath,	_countof(rd->szResultPath),	RjField(f[i + 4]));
	RjPut(rd->szResultFile,	_countof(rd->szResultFile),	RjField(f[i + 5]));
	RjPut(rd->szImageFile,	_countof(rd->szImageFile),	RjField(f[i + 6]));
	return true;
}

// structure check of one data line; bCheckRaw: R lines test the raw file
static bool RjParseLine(const CStringA& raw, S_JLine& L, bool bCheckRaw)
{
	std::vector<CStringA> f;
	RjSplit(raw, f);
	if (f.size() < 12 || f.back() != "$" || f[0].GetLength() != 1) return false;
	L = S_JLine();
	L.strRaw = raw;
	L.cType = f[0][0];
	int nAtt = 0, nG = 0;
	if (!RjToInt(f[1], L.nSeq) || !RjToInt(f[2], nAtt) || !RjToInt(f[3], L.t.nPass) || !RjToInt(f[4], L.t.nSeg) || !RjToInt(f[5], L.t.nStick) || !RjToInt(f[6], nG)) return false;
	if (L.t.nPass < 0 || L.t.nSeg < 0 || L.t.nStick < 0) return false;
	switch (L.cType)
	{
	case 'R':
		{
			if (!RjParseR(f, NULL)) return false;
			if (!RjToInt(f[8], L.nMode) || !RjToInt(f[9], L.nArea)) return false;
			size_t i = 26 + 3 * (size_t)RjInt(f[25], 0);
			CString strPath = RjField(f[i + 4]), strFile = RjField(f[i + 5]);
			int nDot = strFile.ReverseFind(_T('.'));
			L.strInner = (nDot > 0) ? strFile.Left(nDot) : strFile;
			L.bRawOK = bCheckRaw && !strFile.IsEmpty() && RjFileExists(strPath + strFile);
		}
		return true;
	case 'P':
		if (f.size() != 12 || f[7] != "VISION") return false;
		if (!RjToInt(f[8], L.nHead) || !RjToInt(f[9], L.nArea)) return false;
		L.strInner = RjField(f[10]);
		return true;
	case 'X':
		if (f.size() != 13 || f[7] != "VISION" || f[11] != "TIMEOUT") return false;
		if (!RjToInt(f[8], L.nHead) || !RjToInt(f[9], L.nArea)) return false;
		L.strInner = RjField(f[10]);
		return true;
	case 'S':
		if (f.size() != 12) return false;
		if (!RjToInt(f[8], L.nMode) || !RjToInt(f[9], L.nArea)) return false;
		return true;
	default:
		break;
	}
	return false;
}

// read + parse the journal; returns enumResumeWhy (MISSING / UNREADABLE / HEADER) or OK
static int RjLoadJournal(const CString& strPath, bool bCheckRaw, std::vector<CStringA>& vecHdr, std::vector<S_JLine>& vec,
					   int& nTorn, int& nMaxSeq, LONGLONG& llSize, DWORD& dwErr)
{
	vecHdr.clear();
	vec.clear();
	nTorn = 0;
	nMaxSeq = 0;
	llSize = 0;
	std::vector<char> buf;
	dwErr = RjReadAll(strPath, buf, 0, &llSize);
	if (dwErr == ERROR_FILE_NOT_FOUND || dwErr == ERROR_PATH_NOT_FOUND) return RESUME_WHY_MISSING;
	if (dwErr == ERROR_FILE_TOO_LARGE) return RESUME_WHY_HEADER;	// LYS 2026-10-08 over the read limit = broken ("unreadable" would cancel every START and keep the files)
	if (dwErr != 0) return RESUME_WHY_UNREADABLE;
	llSize = (LONGLONG)buf.size();

	std::vector<CStringA> lines;
	bool bLastComplete = true;
	RjSplitLines(buf, lines, bLastComplete);
	if (lines.empty()) return RESUME_WHY_HEADER;
	if (lines.size() == 1 && !bLastComplete) return RESUME_WHY_HEADER;
	RjSplit(lines[0], vecHdr);
	if (vecHdr.size() != 8 || vecHdr[0] != "#RJ1") return RESUME_WHY_HEADER;

	for (size_t i = 1; i < lines.size(); i++)
	{
		S_JLine L;
		bool bLast = (i + 1 == lines.size());
		if (RjParseLine(lines[i], L, bCheckRaw))
		{
			vec.push_back(L);
			if (L.nSeq > nMaxSeq) nMaxSeq = L.nSeq;
			continue;
		}
		if (bLast) { nTorn = 1; break; }			// torn last line (cut write) - dropped
		return RESUME_WHY_HEADER;					// a broken line before the last one
	}
	return RESUME_WHY_OK;
}

static int RjReadSnapSlot(const CString& strPath, const CString& strGlassID, S_RcsFile* pOut)	// 1 valid for the glass, 0 not
{
	std::vector<char> buf;
	LONGLONG llSize = 0;
	if (RjReadAll(strPath, buf, 0, &llSize) != 0) return 0;
	if (buf.size() != sizeof(S_RcsFile)) return 0;
	memcpy(pOut, &buf[0], sizeof(S_RcsFile));
	if (memcmp(pOut->szMagic, "RCS1", 4) != 0 || pOut->nVersion != 1) return 0;
	if (CResumeJournal::Fnv1a(pOut, offsetof(S_RcsFile, dwFnv)) != pOut->dwFnv) return 0;
	WCHAR szWant[32];
	RjPut(szWant, _countof(szWant), strGlassID);
	pOut->szGlassID[31] = 0;
	if (wcsncmp(szWant, pOut->szGlassID, 32) != 0) return 0;
	return 1;
}

//----------------------------------------------------------------------------------------------------
// singleton / setup
//----------------------------------------------------------------------------------------------------
CResumeJournal* CResumeJournal::GetInstance()
{
	static CResumeJournal s_inst;		// C++11 thread-safe static init
	return &s_inst;
}

CResumeJournal::CResumeJournal()
	: m_nTupleAll(0), m_nSimMode(0), m_nGlassGen(1), m_nArmedGen(0), m_hWriter(INVALID_HANDLE_VALUE)
	, m_nNextSeq(1), m_nLineCount(0), m_bWriteOK(true), m_bWarnedWrite(false), m_nWriteCount(0)
	, m_bPend(false), m_nPendSeq(1), m_nPendLines(0), m_nPendAttempt(0), m_nPendDropped(0), m_nPendTorn(0), m_llPendReadSize(0)
	, m_nLastDropped(0), m_nLastTorn(0), m_nSnapSerial(0), m_bWarnedSnap(false)
	, m_nResetCount(0), m_nResetMark(-1), m_bVisWait(false), m_nVisWaitGen(0), m_nSimVisToggle(0)
	, m_bFilesMayExist(false)		// LYS 2026-10-08 Hybrid Phase 5 fix (N=1 review 1) SetFolder checks the folder
	, m_nReplayDone(0), m_nReplayTotal(0)	// LYS 2026-10-12 Hybrid Phase 6
{
	::InitializeCriticalSection(&m_cs);
	ZeroMemory(&m_ctx, sizeof(m_ctx));
	m_tPend = RjTup(0, 0, 0);
}

CResumeJournal::~CResumeJournal()
{
	if (m_hWriter != INVALID_HANDLE_VALUE) { ::CloseHandle(m_hWriter); m_hWriter = INVALID_HANDLE_VALUE; }
	::DeleteCriticalSection(&m_cs);
}

void CResumeJournal::SetFolder(const CString& strConfigPath)
{
	CString strDir = strConfigPath;
	strDir.TrimRight(_T('\\'));
	CString strIni = strDir + _T("\\SystemConfig.ini");
	int nAll = (int)::GetPrivateProfileInt(_T("OPT_PARA"), _T("RESUME_TUPLE_ALL"), 0, strIni);
	int nSim = (int)::GetPrivateProfileInt(_T("OPT_PARA"), _T("RESUME_JOURNAL_OOS_SIM"), 0, strIni);

	::EnterCriticalSection(&m_cs);
	m_strFolder		= strDir;
	m_strJournal	= strDir + _T("\\ResumeJournal.csv");
	m_strJournalTmp	= m_strJournal + _T(".tmp");
	m_strSnapTmp	= strDir + _T("\\ResumeCellStatus.tmp");
	m_nTupleAll		= nAll;
	m_nSimMode		= (nSim >= 0 && nSim <= 4) ? nSim : 0;
	// LYS 2026-10-08 Hybrid Phase 5 fix (N=1 review 1) boot: is any journal / ring / tmp file there (existence only, no log)
	m_bFilesMayExist = RjFileExists(m_strJournal) || RjFileExists(m_strJournalTmp) || RjFileExists(m_strSnapTmp);
	for (int i = 0; i < RESUME_SNAP_SLOTS && m_bFilesMayExist == false; i++) m_bFilesMayExist = RjFileExists(SnapPath(i));
	::LeaveCriticalSection(&m_cs);

	CString strLog;
	if (nAll != 0)
	{
		strLog.Format(_T("ResumeJournal: RESUME_TUPLE_ALL=%d - every glass keeps a resume journal (D5-1 b)"), nAll);
		RjLog(SYST, strLog);
	}
	if (nSim != 0)
	{
		strLog.Format(_T("ResumeJournal: RESUME_JOURNAL_OOS_SIM=%d (used %d) - OOS debug key, never on a production machine"), nSim, m_nSimMode);
		RjLog(SYST, strLog);
	}
}

bool CResumeJournal::IsTupleGlass(int nSegCount) const
{
	return (nSegCount > 1) || (m_nTupleAll != 0);
}

bool CResumeJournal::IsArmed() const
{
	return InterlockedCompareExchange((volatile LONG*)&m_nArmedGen, 0, 0) != 0;
}

CString CResumeJournal::SnapPath(int nSlot) const
{
	CString s;
	if (!m_strFolder.IsEmpty()) s.Format(_T("%s\\ResumeCellStatus%d.dat"), m_strFolder.GetString(), nSlot);
	return s;
}

//----------------------------------------------------------------------------------------------------
// pure helpers
//----------------------------------------------------------------------------------------------------
DWORD CResumeJournal::Fnv1a(const void* p, size_t n, DWORD dwHash)
{
	const BYTE* b = (const BYTE*)p;
	for (size_t i = 0; i < n; i++)
	{
		dwHash ^= b[i];
		dwHash *= 16777619UL;
	}
	return dwHash;
}

static CString RjFileFnvHex(const CString& strPath)	// "-" = missing, "?" = unreadable
{
	if (strPath.IsEmpty()) return _T("-");
	HANDLE h = ::CreateFile(strPath, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (h == INVALID_HANDLE_VALUE)
	{
		DWORD e = ::GetLastError();
		return (e == ERROR_FILE_NOT_FOUND || e == ERROR_PATH_NOT_FOUND) ? _T("-") : _T("?");
	}
	DWORD dwHash = 2166136261UL;
	std::vector<BYTE> buf(65536);
	bool bOK = true;
	for (;;)
	{
		DWORD dwGot = 0;
		if (!::ReadFile(h, &buf[0], (DWORD)buf.size(), &dwGot, NULL)) { bOK = false; break; }
		if (dwGot == 0) break;
		dwHash = CResumeJournal::Fnv1a(&buf[0], dwGot, dwHash);
	}
	::CloseHandle(h);
	if (!bOK) return _T("?");
	CString s;
	s.Format(_T("%08x"), dwHash);
	return s;
}

bool CResumeJournal::BuildShape(CRecipeEditMgr* pRcp, const CString& strWadMpFile, S_ResumeShape& out)
{
	out = S_ResumeShape();
	if (pRcp == NULL) return false;
	CString strRecipeID = pRcp->GetRecipeID();
	if (strRecipeID.IsEmpty()) return false;

	C_OutRecipeFunc* pFunc = pRcp->GetRecipeFuncPtr();
	int nSegCount = (pFunc != NULL) ? pFunc->GetSegmentLoopCount() : 1;
	if (nSegCount < 1) nSegCount = 1;

	CString strShape, strOne;
	int nRows = 0;
	for (int k = 0; k < nSegCount; k++)
	{
		int nSticks = 0, nCells = 0, nRot = 0;
		if (nSegCount <= 1)
		{	// 1 segment: the active recipe (what the legacy measure loop walks)
			S_RecipeMgr* pMgr = pRcp->GetRecipeMgrPtr();
			if (pMgr != NULL)
			{
				nSticks = (int)pMgr->m_sData.m_vecStick.size();
				nCells	= (nSticks > 0) ? (int)pMgr->m_sData.m_vecStick[0].m_vecCell.size() : 0;
				nRot	= pMgr->m_sInfo.m_sOptionItem.m_nGlsRotate;
			}
		}
		else
		{
			nSticks = pFunc->GetSegmentStickCount(k);
			nCells	= pFunc->GetSegmentCellCount(k);
			nRot	= pFunc->GetSegmentRotate(k);
		}
		strOne.Format(_T("%dx%dr%d"), nSticks, nCells, nRot);
		if (k > 0) strShape += _T("|");
		strShape += strOne;
		nRows += nSticks * nCells;
	}

	// LYS 2026-10-08 stage 5-1 completion: the file the dfcRecipe DLL loads = {recipe root}Recipe\{ID}.rcp (Recipe.ini RECIPE_FOLDER, the folder
	//   CRecipeEditMgr::UpdateRecipeList lists). The OpenRecipe format line copied before is dead code there and misses Recipe\ (rcp hash was always '-')
	CString strRcpDir = pRcp->GetRecipeFolderPath();
	if (!strRcpDir.IsEmpty() && strRcpDir.Right(1) != _T("\\")) strRcpDir += _T("\\");
	CString strRcp;
	strRcp.Format(_T("%s%s%s%s"), strRcpDir.GetString(), STR_DIR_RECIPE, strRecipeID.GetString(), _RCP_EXT_FILE_NAME);
	CString strAging, strEfa;
	strAging.Format(_T("%s%s.rcp"), CAgingCycleInfo::GetAgingRcpPath().GetString(), strRecipeID.GetString());
	strEfa.Format(_T("%s%s.rcp"), CAgingCycleInfo::GetEfaRcpPath().GetString(), strRecipeID.GetString());

	out.nSegCount		= nSegCount;
	out.strSegShape		= strShape;
	out.nGlassRows		= nRows;
	out.strFingerprint	= RjFileFnvHex(strRcp) + _T(":") + RjFileFnvHex(strAging) + _T(":") + RjFileFnvHex(strEfa) + _T(":") + RjFileFnvHex(strWadMpFile);
	return true;
}

int CResumeJournal::CompareTuple(const S_ResumeTuple& a, const S_ResumeTuple& b)
{
	if (a.nPass != b.nPass)		return (a.nPass < b.nPass) ? -1 : 1;
	if (a.nSeg != b.nSeg)		return (a.nSeg < b.nSeg) ? -1 : 1;
	if (a.nStick != b.nStick)	return (a.nStick < b.nStick) ? -1 : 1;
	return 0;
}

bool CResumeJournal::PredTuple(const S_ResumeTuple& t, int nSegCount, const int* pnSticks, int nPassCount, S_ResumeTuple& out)
{
	UNREFERENCED_PARAMETER(nPassCount);	// the end marker (PassCount,0,0) is handled by the (p>0,0,0) rule
	out = t;
	if (pnSticks == NULL || nSegCount < 1) return false;
	if (t.nStick > 0) { out.nStick = t.nStick - 1; return true; }
	int p = t.nPass, s = t.nSeg;
	for (;;)
	{
		if (s > 0)			s--;
		else if (p > 0)		{ p--; s = nSegCount - 1; }
		else				return false;
		if (s < nSegCount && pnSticks[s] > 0) { out = RjTup(p, s, pnSticks[s] - 1); return true; }	// a segment without sticks is skipped
	}
}

int CResumeJournal::ParseSegShape(const CString& strSegShape, std::vector<int>& vecSticks, std::vector<int>* pvecCells, std::vector<int>* pvecRot)
{
	vecSticks.clear();
	if (pvecCells) pvecCells->clear();
	if (pvecRot) pvecRot->clear();
	if (strSegShape.IsEmpty()) return -1;
	int nPos = 0;
	for (;;)
	{
		CString strTok = strSegShape.Tokenize(_T("|"), nPos);
		if (nPos < 0) break;
		int nS = 0, nC = 0, nR = 0;
		if (_stscanf_s(strTok, _T("%dx%dr%d"), &nS, &nC, &nR) != 3 || nS < 0 || nC < 0) { vecSticks.clear(); return -1; }
		vecSticks.push_back(nS);
		if (pvecCells) pvecCells->push_back(nC);
		if (pvecRot) pvecRot->push_back(nR);
	}
	return vecSticks.empty() ? -1 : (int)vecSticks.size();
}

int CResumeJournal::ResumePointFromLast(const S_ResumeTuple& last, int nSegCount, const int* pnSticks, int nPassCount, S_ResumeTuple& out)
{
	out = RjTup(0, 0, 0);
	if (pnSticks == NULL || nSegCount < 1 || nPassCount < 1) return -1;
	if (last.nStick < 0 || last.nPass < 0 || last.nPass >= nPassCount || last.nSeg < 0 || last.nSeg >= nSegCount) return -1;
	if (last.nStick >= pnSticks[last.nSeg]) return -1;
	int p = last.nPass, s = last.nSeg, k = last.nStick + 1;
	while (k >= pnSticks[s])		// next segment (empty segments skipped), next pass
	{
		k = 0;
		s++;
		if (s >= nSegCount) { s = 0; p++; }
		if (p >= nPassCount) { out = RjTup(nPassCount, 0, 0); return 1; }	// all done: end marker
	}
	out = RjTup(p, s, k);
	return 0;
}

//----------------------------------------------------------------------------------------------------
// lifecycle
//----------------------------------------------------------------------------------------------------
void CResumeJournal::CloseWriterLocked()
{
	if (m_hWriter != INVALID_HANDLE_VALUE)
	{
		::CloseHandle(m_hWriter);
		m_hWriter = INVALID_HANDLE_VALUE;
	}
}

void CResumeJournal::WriteFailedLocked(DWORD dwErr, CString& strWarn)
{
	m_bWriteOK = false;
	if (!m_bWarnedWrite)
	{
		m_bWarnedWrite = true;
		strWarn.Format(_T("ResumeJournal: write failed (%u) - this glass will not be resumable"), dwErr);
	}
}

bool CResumeJournal::Begin(const CString& strGlassID, const CString& strRecipeID, int nSegCount, int nPassCount, int nGlassRows)
{
	SYSTEMTIME st;
	::GetLocalTime(&st);
	CStringA strHdr;
	strHdr.Format("#RJ1,%s,%s,%d,%d,%d,%04d%02d%02d-%02d%02d%02d,0\r\n", RjEscS(strGlassID).GetString(), RjEscS(strRecipeID).GetString(),
		nSegCount, nPassCount, nGlassRows, st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);

	CString strWarn;
	DWORD dwErr = 0;
	::EnterCriticalSection(&m_cs);
	CloseWriterLocked();
	InterlockedExchange(&m_nArmedGen, 0);
	if (m_strFolder.IsEmpty()) dwErr = ERROR_PATH_NOT_FOUND;
	else
	{
		m_bFilesMayExist = true;		// LYS 2026-10-08 Hybrid Phase 5 fix (N=1 review 1) the journal is created below
		::DeleteFile(m_strJournalTmp);
		for (int i = 0; i < RESUME_SNAP_SLOTS; i++) ::DeleteFile(SnapPath(i));
		::DeleteFile(m_strSnapTmp);
		HANDLE h = ::CreateFile(m_strJournal, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_DELETE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (h == INVALID_HANDLE_VALUE) dwErr = ::GetLastError();
		else
		{
			dwErr = RjWriteAll(h, strHdr.GetString(), (DWORD)strHdr.GetLength());
			if (dwErr == 0 && !::FlushFileBuffers(h)) dwErr = ::GetLastError();
			::CloseHandle(h);
		}
	}
	InterlockedIncrement(&m_nGlassGen);
	m_nNextSeq		= 1;
	m_nLineCount	= 0;
	m_nWriteCount	= 0;
	m_bWriteOK		= true;
	m_bWarnedWrite	= false;
	m_nSnapSerial	= 0;
	m_bWarnedSnap	= false;
	m_bPend			= false;
	m_nLastDropped	= m_nLastTorn = 0;
	m_strGlassID	= strGlassID;
	m_strRecipeID	= strRecipeID;
	m_vecSwapExtra.clear();
	m_vecSimX.clear();
	m_nSimVisToggle	= 0;
	if (dwErr != 0) WriteFailedLocked(dwErr, strWarn);
	::LeaveCriticalSection(&m_cs);
	RjLog(WARN, strWarn);
	return (dwErr == 0);
}

void CResumeJournal::DeleteFiles()
{
	CString strWarn, strOne;
	::EnterCriticalSection(&m_cs);
	CloseWriterLocked();
	InterlockedExchange(&m_nArmedGen, 0);
	ZeroMemory(&m_ctx, sizeof(m_ctx));
	m_bVisWait = false;
	m_bPend = false;
	if (!m_strFolder.IsEmpty() && m_bFilesMayExist)	// LYS 2026-10-08 Hybrid Phase 5 fix (N=1 review 1) no file call when none can be there
	{
		bool bLeft = false;
		CString arr[3 + RESUME_SNAP_SLOTS];
		arr[0] = m_strJournal;
		arr[1] = m_strJournalTmp;
		arr[2] = m_strSnapTmp;
		for (int i = 0; i < RESUME_SNAP_SLOTS; i++) arr[3 + i] = SnapPath(i);
		for (int i = 0; i < 3 + RESUME_SNAP_SLOTS; i++)
		{
			if (::DeleteFile(arr[i])) continue;
			DWORD e = ::GetLastError();
			if (e == ERROR_FILE_NOT_FOUND || e == ERROR_PATH_NOT_FOUND) continue;
			strOne.Format(_T("ResumeJournal: delete failed (%u) %s"), e, arr[i].GetString());
			if (strWarn.IsEmpty()) strWarn = strOne;
			bLeft = true;		// still there (sharing violation ...): the next DeleteFiles tries again
		}
		m_bFilesMayExist = bLeft;
	}
	InterlockedIncrement(&m_nGlassGen);
	m_nNextSeq		= 1;
	m_nLineCount	= 0;
	m_nWriteCount	= 0;
	m_bWriteOK		= true;
	m_bWarnedWrite	= false;
	m_nSnapSerial	= 0;
	m_bWarnedSnap	= false;
	m_nLastDropped	= m_nLastTorn = 0;
	m_strGlassID.Empty();
	m_strRecipeID.Empty();
	m_vecSwapExtra.clear();
	m_vecSimX.clear();
	::LeaveCriticalSection(&m_cs);
	RjLog(WARN, strWarn);		// silent when the files were deleted or did not exist (N=1 runs keep their logs byte-identical)
}

//----------------------------------------------------------------------------------------------------
// context
//----------------------------------------------------------------------------------------------------
void CResumeJournal::Arm(int nAttempt)
{
	CString strWarn;
	::EnterCriticalSection(&m_cs);
	CloseWriterLocked();
	if (m_strFolder.IsEmpty()) WriteFailedLocked(ERROR_PATH_NOT_FOUND, strWarn);
	else
	{
		m_bFilesMayExist = true;		// LYS 2026-10-08 Hybrid Phase 5 fix (N=1 review 1) OPEN_ALWAYS can create the journal
		m_hWriter = ::CreateFile(m_strJournal, FILE_APPEND_DATA, FILE_SHARE_READ | FILE_SHARE_DELETE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (m_hWriter == INVALID_HANDLE_VALUE) WriteFailedLocked(::GetLastError(), strWarn);
	}
	ZeroMemory(&m_ctx, sizeof(m_ctx));
	m_ctx.nGen		= (int)m_nGlassGen;
	m_ctx.nAttempt	= nAttempt;
	m_vecSimX.clear();
	InterlockedExchange(&m_nArmedGen, m_nGlassGen);
	::LeaveCriticalSection(&m_cs);
	RjLog(WARN, strWarn);
}

void CResumeJournal::SetStick(int nPass, int nSeg, int nStick, int nGlassStick)
{
	::EnterCriticalSection(&m_cs);
	if (m_nArmedGen != 0)
	{
		m_ctx.nPass			= nPass;
		m_ctx.nSeg			= nSeg;
		m_ctx.nStick		= nStick;
		m_ctx.nGlassStick	= nGlassStick;
	}
	m_vecSimX.clear();
	::LeaveCriticalSection(&m_cs);
}

void CResumeJournal::Disarm()
{
	if (InterlockedCompareExchange(&m_nArmedGen, 0, 0) == 0 && m_hWriter == INVALID_HANDLE_VALUE && m_bVisWait == false) return;	// nothing armed / open: no lock, no I/O
	::EnterCriticalSection(&m_cs);
	CloseWriterLocked();
	InterlockedExchange(&m_nArmedGen, 0);
	ZeroMemory(&m_ctx, sizeof(m_ctx));
	m_bVisWait = false;
	m_vecSimX.clear();
	::LeaveCriticalSection(&m_cs);
}

S_ResumeCtx CResumeJournal::Capture() const
{
	S_ResumeCtx c;
	ZeroMemory(&c, sizeof(c));
	if (!IsArmed()) return c;
	::EnterCriticalSection(&m_cs);
	if (m_nArmedGen != 0) c = m_ctx;
	::LeaveCriticalSection(&m_cs);
	return c;
}

void CResumeJournal::SetThreadCtx(const S_ResumeCtx* pCtx)
{
	if (pCtx != NULL)
	{
		t_ctxOverride = *pCtx;
		t_bOverride = true;
	}
	else
	{
		ZeroMemory(&t_ctxOverride, sizeof(t_ctxOverride));
		t_bOverride = false;
	}
}

//----------------------------------------------------------------------------------------------------
// append
//----------------------------------------------------------------------------------------------------
bool CResumeJournal::WriteLineLocked(const CStringA& strLine, bool bArmedPath, CString& strWarn)
{
	CStringA s = strLine + "\r\n";
	m_nWriteCount++;
	DWORD dwErr = 0;
	if (m_nSimMode == 4 && m_nWriteCount == RJ_SIM_FAIL_WRITE)
		dwErr = ERROR_WRITE_FAULT;							// OOS sim 4: JournalOK test
	else if (m_hWriter != INVALID_HANDLE_VALUE)
		dwErr = RjWriteAll(m_hWriter, s.GetString(), (DWORD)s.GetLength());
	else if (m_strFolder.IsEmpty())
		dwErr = ERROR_PATH_NOT_FOUND;
	else
	{	// armed without a handle (open failed at Arm): retry per line; late row after Disarm: open-append-close, never recreate a deleted journal
		if (bArmedPath) m_bFilesMayExist = true;		// LYS 2026-10-08 Hybrid Phase 5 fix (N=1 review 1) OPEN_ALWAYS can create the journal
		HANDLE h = ::CreateFile(m_strJournal, FILE_APPEND_DATA, FILE_SHARE_READ | FILE_SHARE_DELETE, NULL, bArmedPath ? OPEN_ALWAYS : OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (h == INVALID_HANDLE_VALUE) dwErr = ::GetLastError();
		else
		{
			dwErr = RjWriteAll(h, s.GetString(), (DWORD)s.GetLength());
			::CloseHandle(h);
		}
		if (dwErr == 0 && h == INVALID_HANDLE_VALUE) dwErr = ERROR_GEN_FAILURE;
	}
	if (dwErr != 0)
	{
		WriteFailedLocked(dwErr, strWarn);
		return false;
	}
	m_nLineCount++;
	return true;
}

// R body after <gstick>: kind ... $
static CStringA RjFormatR(const RSRITEMDATA* p)
{
	int nCnt = p->nCntData;
	if (nCnt < 0) nCnt = 0;
	if (nCnt > 100) nCnt = 100;
	CStringA s, t;
	s.Format("%s,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%.17g,%.17g,%.17g,%d,",
		RjKindOfMode(p->nMeasMode), p->nMeasMode, p->iMeasArea, p->m_iProbe, p->iCurArea, p->iCurStep, p->iPanelNo, p->nUseResultFile,
		p->m_iTValue, p->m_nMeasType, p->m_nMeasAreaCnt, p->m_nMaxMeasureRcpFlag, p->m_iRetryCnt, p->m_iRetryReason, p->nMultiPoint,
		p->dOffsetRX, p->dOffsetGX, p->dOffsetBY, nCnt);
	for (int i = 0; i < nCnt; i++)
	{
		t.Format("%s,%s,%d,", RjEscW(p->stData[i].szName, _countof(p->stData[i].szName)).GetString(),
			RjEscW(p->stData[i].szData, _countof(p->stData[i].szData)).GetString(), p->stData[i].nReason);
		s += t;
	}
	s += RjEscW(p->szCellData, _countof(p->szCellData));		s += ",";
	s += RjEscW(p->szLotID, _countof(p->szLotID));			s += ",";
	s += RjEscW(p->szGlassID, _countof(p->szGlassID));		s += ",";
	s += RjEscW(p->szRecipeID, _countof(p->szRecipeID));		s += ",";
	s += RjEscW(p->szResultPath, _countof(p->szResultPath));	s += ",";
	s += RjEscW(p->szResultFile, _countof(p->szResultFile));	s += ",";
	s += RjEscW(p->szImageFile, _countof(p->szImageFile));	s += ",$";
	return s;
}

void CResumeJournal::OnUpload(const RSRITEMDATA* p)
{
	if (p == NULL) return;
	S_ResumeCtx c;
	bool bOverride = t_bOverride;
	if (bOverride)
	{
		c = t_ctxOverride;
		if (c.nGen == 0) return;							// grabbed while not armed, or the slot was reused: never falls back to the armed context
	}
	else if (!IsArmed()) return;							// lock-free: not armed (1-segment default, manual measure, RS1, replay)

	CStringA strBody = RjFormatR(p);						// format outside the lock
	CString strLog, strWarn;
	::EnterCriticalSection(&m_cs);
	if (!bOverride)
	{
		if (m_nArmedGen == 0) { ::LeaveCriticalSection(&m_cs); return; }
		c = m_ctx;
	}
	if (c.nGen != (int)m_nGlassGen)
		strLog.Format(_T("ResumeJournal: late VISION row dropped (generation %d/%d) row(%d)"), c.nGen, (int)m_nGlassGen, p->iMeasArea);
	else
	{
		CStringA strLine;
		strLine.Format("R,%d,%d,%d,%d,%d,%d,", NextSeqLocked(), c.nAttempt, c.nPass, c.nSeg, c.nStick, c.nGlassStick);
		strLine += strBody;
		WriteLineLocked(strLine, m_nArmedGen != 0, strWarn);
	}
	::LeaveCriticalSection(&m_cs);
	RjLog(SYST, strLog);
	RjLog(WARN, strWarn);
}

void CResumeJournal::AppendPending(const S_ResumeCtx& c, int nHead, int nArea, const CString& strInnerID)
{
	if (c.nGen == 0) return;
	CStringA strInner = RjEscS(strInnerID);
	CString strWarn;
	::EnterCriticalSection(&m_cs);
	if (c.nGen == (int)m_nGlassGen)
	{
		CStringA strLine;
		strLine.Format("P,%d,%d,%d,%d,%d,%d,VISION,%d,%d,%s,$", NextSeqLocked(), c.nAttempt, c.nPass, c.nSeg, c.nStick, c.nGlassStick, nHead, nArea, strInner.GetString());
		WriteLineLocked(strLine, m_nArmedGen != 0, strWarn);
		// OOS sim 3: this grab's synthetic close is a timeout (X) instead of an S line
		for (size_t i = 0; i < m_vecSimX.size(); i++)
		{
			const S_SimX& x = m_vecSimX[i];
			if (x.c.nGen == c.nGen && x.c.nPass == c.nPass && x.c.nSeg == c.nSeg && x.c.nStick == c.nStick && x.nArea == nArea)
			{
				strLine.Format("X,%d,%d,%d,%d,%d,%d,VISION,%d,%d,%s,TIMEOUT,$", NextSeqLocked(), c.nAttempt, c.nPass, c.nSeg, c.nStick, c.nGlassStick, nHead, nArea, strInner.GetString());
				WriteLineLocked(strLine, m_nArmedGen != 0, strWarn);
				m_vecSimX.erase(m_vecSimX.begin() + i);
				break;
			}
		}
	}
	::LeaveCriticalSection(&m_cs);
	RjLog(WARN, strWarn);
}

void CResumeJournal::AppendClose(const S_ResumeCtx& c, int nHead, int nArea, const CString& strInnerID)
{
	if (c.nGen == 0) return;
	CStringA strInner = RjEscS(strInnerID);
	CString strWarn;
	::EnterCriticalSection(&m_cs);
	if (c.nGen == (int)m_nGlassGen)
	{
		CStringA strLine;
		strLine.Format("X,%d,%d,%d,%d,%d,%d,VISION,%d,%d,%s,TIMEOUT,$", NextSeqLocked(), c.nAttempt, c.nPass, c.nSeg, c.nStick, c.nGlassStick, nHead, nArea, strInner.GetString());
		WriteLineLocked(strLine, m_nArmedGen != 0, strWarn);
	}
	::LeaveCriticalSection(&m_cs);
	RjLog(WARN, strWarn);
}

void CResumeJournal::AppendSim(const TCHAR* szKind, const int* pnArea, const bool* pbUse, const TCHAR* szCell0, const TCHAR* szCell1)
{
	if (m_nSimMode <= 0 || szKind == NULL || pnArea == NULL || pbUse == NULL) return;
	if (!IsArmed()) return;
	int nMode = RjModeOfKind(szKind);
	bool bVision = (nMode == PGM_MODE_VISION);
	CStringA strKind = RjEscS(CString(szKind));
	CStringA strCell[2];
	strCell[0] = (szCell0 != NULL) ? RjEscS(CString(szCell0)) : CStringA();
	strCell[1] = (szCell1 != NULL) ? RjEscS(CString(szCell1)) : CStringA();
	CString strWarn;
	::EnterCriticalSection(&m_cs);
	if (m_nArmedGen != 0)
	{
		S_ResumeCtx c = m_ctx;
		for (int h = 0; h < 2; h++)
		{
			if (!pbUse[h] || pnArea[h] < 0) continue;
			if (bVision)
			{
				if (m_nSimMode == 2) continue;					// VISION never arrives -> P stays unpaired (pull-back test)
				if (m_nSimMode == 3 && ((m_nSimVisToggle++ % 2) == 1))
				{	// closed by an X line right after the grab's P (AppendPending)
					S_SimX x;
					x.c = c;
					x.nArea = pnArea[h];
					if (m_vecSimX.size() < 8) m_vecSimX.push_back(x);
					continue;
				}
			}
			CStringA strLine;
			strLine.Format("S,%d,%d,%d,%d,%d,%d,%s,%d,%d,%s,$", NextSeqLocked(), c.nAttempt, c.nPass, c.nSeg, c.nStick, c.nGlassStick,
				strKind.GetString(), nMode, pnArea[h], strCell[h].GetString());
			WriteLineLocked(strLine, true, strWarn);
		}
	}
	::LeaveCriticalSection(&m_cs);
	RjLog(WARN, strWarn);
}

bool CResumeJournal::Commit()
{
	CString strWarn;
	::EnterCriticalSection(&m_cs);
	if (m_hWriter != INVALID_HANDLE_VALUE && !::FlushFileBuffers(m_hWriter)) WriteFailedLocked(::GetLastError(), strWarn);
	bool bOK = m_bWriteOK;
	::LeaveCriticalSection(&m_cs);
	RjLog(WARN, strWarn);
	return bOK;
}

int CResumeJournal::GetLineCount() const
{
	::EnterCriticalSection(&m_cs);
	int n = m_nLineCount;
	::LeaveCriticalSection(&m_cs);
	return n;
}

bool CResumeJournal::IsWriteOK() const
{
	::EnterCriticalSection(&m_cs);
	bool b = m_bWriteOK;
	::LeaveCriticalSection(&m_cs);
	return b;
}

//----------------------------------------------------------------------------------------------------
// analysis (plan (4-5))
//----------------------------------------------------------------------------------------------------
static __int64 RjSimKey(const S_ResumeTuple& t, int nArea)
{
	return ((((__int64)t.nPass * 1000 + t.nSeg) * 1000 + t.nStick) * 1000000) + (nArea & 0xFFFFF);
}

// P closed by: a later X with the same InnerID, a later R VISION with that InnerID and its raw file, an S VISION with the same (pass, seg, stick, area)
static void RjPairPending(std::vector<S_JLine>& vec)
{
	std::map<CString, std::deque<size_t>> mapOpen;
	for (size_t i = 0; i < vec.size(); i++)
	{
		S_JLine& L = vec[i];
		if (L.cType == 'P') { mapOpen[L.strInner].push_back(i); continue; }
		if (L.cType == 'X' || (L.cType == 'R' && L.nMode == PGM_MODE_VISION && L.bRawOK))
		{
			std::map<CString, std::deque<size_t>>::iterator it = mapOpen.find(L.strInner);
			if (it != mapOpen.end() && !it->second.empty()) { vec[it->second.front()].bClosed = true; it->second.pop_front(); }
		}
	}
	std::map<__int64, int> mapSim;		// S lines are written before the grab's P (LogHybridRow precedes QualityWaitSet): count based
	for (size_t i = 0; i < vec.size(); i++)
		if (vec[i].cType == 'S' && vec[i].nMode == PGM_MODE_VISION) mapSim[RjSimKey(vec[i].t, vec[i].nArea)]++;
	for (size_t i = 0; i < vec.size(); i++)
	{
		S_JLine& L = vec[i];
		if (L.cType != 'P' || L.bClosed) continue;
		std::map<__int64, int>::iterator it = mapSim.find(RjSimKey(L.t, L.nArea));
		if (it != mapSim.end() && it->second > 0) { L.bClosed = true; it->second--; }
	}
}

struct S_RingInfo { bool bValid; S_ResumeTuple t; int nSerial; };

static void RjReadRing(const CResumeJournal* pJ, const CString& strGlassID, S_RingInfo ring[RESUME_SNAP_SLOTS], const CString arrPath[RESUME_SNAP_SLOTS])
{
	UNREFERENCED_PARAMETER(pJ);
	S_RcsFile* pf = new S_RcsFile;
	for (int i = 0; i < RESUME_SNAP_SLOTS; i++)
	{
		ring[i].bValid = false;
		ring[i].nSerial = 0;
		ring[i].t = RjTup(0, 0, 0);
		if (RjReadSnapSlot(arrPath[i], strGlassID, pf) == 1)
		{
			ring[i].bValid = true;
			ring[i].t = RjTup(pf->nPass, pf->nSeg, pf->nStick);
			ring[i].nSerial = pf->nSerial;
		}
	}
	delete pf;
}

static bool RjRingHasExact(const S_RingInfo ring[RESUME_SNAP_SLOTS], const S_ResumeTuple& t)
{
	for (int i = 0; i < RESUME_SNAP_SLOTS; i++)
		if (ring[i].bValid && CResumeJournal::CompareTuple(ring[i].t, t) == 0) return true;
	return false;
}

static bool RjHeaderMatches(const std::vector<CStringA>& vecHdr, const S_ResumeBookmark& bm)
{
	if (vecHdr.size() != 8) return false;
	if (RjField(vecHdr[1]) != bm.strGlassID) return false;
	if (RjField(vecHdr[2]).CompareNoCase(bm.strRecipeID) != 0) return false;
	return RjInt(vecHdr[3], -1) == bm.nSegCount && RjInt(vecHdr[4], -1) == bm.nPassCount && RjInt(vecHdr[5], -1) == bm.nGlassRows;
}

bool CResumeJournal::Analyze(const S_ResumeBookmark& bm, S_ResumePlan& plan) const
{
	plan = S_ResumePlan();
	plan.Tbook = bm.T;
	plan.T = bm.T;
	plan.bAllDone = (bm.nPoint == 1);

	std::vector<int> vecSticks;
	if (bm.nPoint < 0 || ParseSegShape(bm.strSegShape, vecSticks) != bm.nSegCount)
	{
		plan.nWhy = RESUME_WHY_RANGE;
		plan.strWhyEn = _T("bookmark out of range");
		return false;
	}

	std::vector<CStringA> vecHdr;
	std::vector<S_JLine> vec;
	int nTorn = 0, nMaxSeq = 0;
	LONGLONG llSize = 0;
	DWORD dwErr = 0;
	int nWhy = RjLoadJournal(m_strJournal, true, vecHdr, vec, nTorn, nMaxSeq, llSize, dwErr);
	plan.nTorn = nTorn;
	plan.nLines = (int)vec.size();
	if (nWhy == RESUME_WHY_OK && !RjHeaderMatches(vecHdr, bm)) nWhy = RESUME_WHY_HEADER;
	if (nWhy == RESUME_WHY_UNREADABLE)
	{
		plan.nWhy = nWhy;
		plan.nIoError = (int)dwErr;
		plan.strWhyEn.Format(_T("journal busy/unreadable (%u)"), dwErr);
		return false;
	}
	if (nWhy != RESUME_WHY_OK)
	{
		plan.nWhy = nWhy;
		plan.strWhyEn = _T("journal missing or broken");
		return false;
	}
	if (bm.nJournalOK == 0)
	{
		plan.nWhy = RESUME_WHY_WRITE_FAILED;
		plan.strWhyEn = _T("journal write failed during the run");
		return false;
	}
	if ((int)vec.size() < bm.nJournalLines)
	{
		plan.nWhy = RESUME_WHY_SHORT;
		plan.strWhyEn.Format(_T("journal shorter than recorded (%d < %d)"), (int)vec.size(), bm.nJournalLines);
		return false;
	}

	S_RingInfo ring[RESUME_SNAP_SLOTS];
	CString arrSnap[RESUME_SNAP_SLOTS];
	for (int i = 0; i < RESUME_SNAP_SLOTS; i++) arrSnap[i] = SnapPath(i);
	RjReadRing(this, bm.strGlassID, ring, arrSnap);

	RjPairPending(vec);

	// the last RESUME_QUALITY_MAX P lines of each head (whole file, lines after T included)
	std::map<int, std::vector<size_t>> mapHeadP;
	for (size_t i = 0; i < vec.size(); i++)
		if (vec[i].cType == 'P') mapHeadP[vec[i].nHead].push_back(i);
	std::vector<bool> vecRecent(vec.size(), false);
	for (std::map<int, std::vector<size_t>>::iterator it = mapHeadP.begin(); it != mapHeadP.end(); ++it)
	{
		size_t n = it->second.size();
		for (size_t k = (n > (size_t)RESUME_QUALITY_MAX) ? n - (size_t)RESUME_QUALITY_MAX : 0; k < n; k++) vecRecent[it->second[k]] = true;
	}

	// pull-back eligibility of the unpaired P lines before the bookmark point
	std::vector<CString> vecWhy(vec.size());
	bool bFound = false;
	S_ResumeTuple Tp = RjTup(0, 0, 0);
	for (size_t i = 0; i < vec.size(); i++)
	{
		const S_JLine& L = vec[i];
		if (L.cType != 'P' || L.bClosed || CompareTuple(L.t, bm.T) >= 0) continue;
		CString strWhy;
		if (!vecRecent[i])
			strWhy.Format(_T("older than the last %d grabs of head %d"), RESUME_QUALITY_MAX, L.nHead);
		else if (L.t.nPass < bm.nPassBoundaryDone)
			strWhy = _T("pass already flushed");
		else if (bm.nEfaMode == 1 && !RjIsZero(L.t))
		{
			S_ResumeTuple tp;
			if (!PredTuple(L.t, bm.nSegCount, &vecSticks[0], bm.nPassCount, tp) || !RjRingHasExact(ring, tp)) strWhy = _T("no cell status snapshot");
		}
		vecWhy[i] = strWhy;
		if (strWhy.IsEmpty() && (!bFound || CompareTuple(L.t, Tp) < 0))
		{
			bFound = true;
			Tp = L.t;
			plan.nPullHead = L.nHead;
			plan.strPullInner = L.strInner;
		}
	}
	if (bFound && CompareTuple(Tp, plan.T) < 0)
	{
		plan.T = Tp;
		plan.bRewind = true;
		plan.bAllDone = false;
	}
	else
	{
		plan.nPullHead = -1;
		plan.strPullInner.Empty();
	}

	// keep = tuple < T; unpaired P lines before T could not pull back -> dropped (recorded once in the WARN of the caller)
	for (size_t i = 0; i < vec.size(); i++)
	{
		S_JLine& L = vec[i];
		L.bKeep = (CompareTuple(L.t, plan.T) < 0);
		if (L.bKeep && L.cType == 'P' && !L.bClosed)
		{
			L.bKeep = false;
			if (plan.nPendingNoPull++ == 0)
			{
				plan.nNoPullHead = L.nHead;
				plan.strNoPullInner = L.strInner;
				plan.TnoPull = L.t;
				plan.strNoPullWhyEn = vecWhy[i];
			}
		}
		if (L.bKeep)
		{
			plan.nKeep++;
			if (L.cType == 'R' && !L.bRawOK) plan.nMissing++;
		}
	}

	// cell status snapshot pred(T)
	if (!RjIsZero(plan.T))
	{
		plan.bHasPred = PredTuple(plan.T, bm.nSegCount, &vecSticks[0], bm.nPassCount, plan.Tpred);
		plan.bSnapExact = plan.bHasPred && RjRingHasExact(ring, plan.Tpred);
		if (bm.nEfaMode == 1 && !plan.bSnapExact)
		{
			plan.nWhy = RESUME_WHY_SNAPSHOT;
			plan.strWhyEn.Format(_T("cell status snapshot for T%d seg %d stick %d missing (EFA glass)"), plan.Tpred.nPass, plan.Tpred.nSeg + 1, plan.Tpred.nStick + 1);
			return false;
		}
	}
	return true;
}

bool CResumeJournal::WriteCompacted(const S_ResumeBookmark& bm, const S_ResumePlan& plan, std::vector<CStringA>& vecKeep)
{
	vecKeep.clear();
	if (plan.nWhy != RESUME_WHY_OK) return false;
	CString strWarn;
	bool bOK = false;
	::EnterCriticalSection(&m_cs);
	m_bPend = false;
	std::vector<CStringA> vecHdr;
	std::vector<S_JLine> vec;
	int nTorn = 0, nMaxSeq = 0;
	LONGLONG llSize = 0;
	DWORD dwErr = 0;
	int nWhy = RjLoadJournal(m_strJournal, true, vecHdr, vec, nTorn, nMaxSeq, llSize, dwErr);
	if (nWhy != RESUME_WHY_OK || !RjHeaderMatches(vecHdr, bm))
		strWarn.Format(_T("ResumeJournal: compaction read failed (why %d, error %u) - journal unchanged"), nWhy, dwErr);
	else
	{
		RjPairPending(vec);
		int nAttempt = RjInt(vecHdr[7], 0) + 1;
		CStringA strHdr;
		strHdr.Format("#RJ1,%s,%s,%s,%s,%s,%s,%d\r\n", vecHdr[1].GetString(), vecHdr[2].GetString(), vecHdr[3].GetString(), vecHdr[4].GetString(),
			vecHdr[5].GetString(), vecHdr[6].GetString(), nAttempt);
		std::vector<char> out(strHdr.GetString(), strHdr.GetString() + strHdr.GetLength());
		for (size_t i = 0; i < vec.size(); i++)
		{
			const S_JLine& L = vec[i];
			if (CompareTuple(L.t, plan.T) >= 0) continue;		// the resumed stick and everything after it
			if (L.cType == 'P' && !L.bClosed) continue;			// unpaired P before T: judged now, dropped (no pull-back later)
			vecKeep.push_back(L.strRaw);
			out.insert(out.end(), L.strRaw.GetString(), L.strRaw.GetString() + L.strRaw.GetLength());
			out.push_back('\r');
			out.push_back('\n');
		}
		m_bFilesMayExist = true;		// LYS 2026-10-08 Hybrid Phase 5 fix (N=1 review 1) journal tmp
		HANDLE h = ::CreateFile(m_strJournalTmp, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (h == INVALID_HANDLE_VALUE) dwErr = ::GetLastError();
		else
		{
			dwErr = RjWriteAll(h, &out[0], (DWORD)out.size());
			if (dwErr == 0 && !::FlushFileBuffers(h)) dwErr = ::GetLastError();
			::CloseHandle(h);
		}
		if (dwErr != 0)
		{
			strWarn.Format(_T("ResumeJournal: compaction write failed (%u) - journal unchanged"), dwErr);
			::DeleteFile(m_strJournalTmp);
			vecKeep.clear();
		}
		else
		{
			m_bPend				= true;
			m_tPend				= plan.T;
			m_nPendSeq			= nMaxSeq + 1;
			m_nPendLines		= (int)vecKeep.size();
			m_nPendAttempt		= nAttempt;
			m_nPendDropped		= (int)vec.size() - (int)vecKeep.size();
			m_nPendTorn			= nTorn;
			m_llPendReadSize	= llSize;
			m_strPendGlassID	= RjField(vecHdr[1]);
			m_strPendRecipeID	= RjField(vecHdr[2]);
			bOK = true;
		}
	}
	::LeaveCriticalSection(&m_cs);
	RjLog(WARN, strWarn);
	return bOK;
}

bool CResumeJournal::SwapCompacted()
{
	CString strWarn, strLog;
	bool bOK = false;
	::EnterCriticalSection(&m_cs);
	if (!m_bPend) strWarn = _T("ResumeJournal: swap without a compacted file - journal unchanged");
	else
	{
		CloseWriterLocked();
		// lines this generation appended after WriteCompacted read the journal (late VISION / ANGLE rows): kept when before the cut
		std::vector<char> extra;
		LONGLONG llNow = 0;
		std::vector<CStringA> vecExtraKeep;
		int nMaxSeq = m_nPendSeq - 1;
		if (RjReadAll(m_strJournal, extra, m_llPendReadSize, &llNow) == 0 && !extra.empty())
		{
			std::vector<CStringA> lines;
			bool bLastComplete = true;
			RjSplitLines(extra, lines, bLastComplete);
			size_t nComplete = bLastComplete ? lines.size() : lines.size() - 1;
			for (size_t i = 0; i < nComplete; i++)
			{
				S_JLine L;
				if (!RjParseLine(lines[i], L, false) || L.cType == 'P' || CompareTuple(L.t, m_tPend) >= 0) continue;
				vecExtraKeep.push_back(L.strRaw);
				if (L.nSeq > nMaxSeq) nMaxSeq = L.nSeq;
			}
		}
		DWORD dwErr = 0;
		if (!vecExtraKeep.empty())
		{
			HANDLE h = ::CreateFile(m_strJournalTmp, FILE_APPEND_DATA, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			if (h == INVALID_HANDLE_VALUE) dwErr = ::GetLastError();
			else
			{
				for (size_t i = 0; i < vecExtraKeep.size() && dwErr == 0; i++)
				{
					CStringA s = vecExtraKeep[i] + "\r\n";
					dwErr = RjWriteAll(h, s.GetString(), (DWORD)s.GetLength());
				}
				if (dwErr == 0 && !::FlushFileBuffers(h)) dwErr = ::GetLastError();
				::CloseHandle(h);
			}
		}
		if (dwErr == 0 && !::MoveFileEx(m_strJournalTmp, m_strJournal, MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)) dwErr = ::GetLastError();
		if (dwErr != 0)
			strWarn.Format(_T("ResumeJournal: journal swap failed (%u) - old journal kept"), dwErr);
		else
		{
			InterlockedIncrement(&m_nGlassGen);
			m_nNextSeq		= nMaxSeq + 1;
			m_nLineCount	= m_nPendLines + (int)vecExtraKeep.size();
			m_nWriteCount	= 0;
			m_bWriteOK		= true;
			m_bWarnedWrite	= false;
			m_strGlassID	= m_strPendGlassID;
			m_strRecipeID	= m_strPendRecipeID;
			m_nLastDropped	= m_nPendDropped;
			m_nLastTorn		= m_nPendTorn;
			m_vecSwapExtra	= vecExtraKeep;		// Replay uploads these after its vecKeep
			// snapshot serial continues after the ring's newest slot of this glass
			m_nSnapSerial = 0;
			m_bWarnedSnap = false;
			S_RcsFile* pf = new S_RcsFile;
			for (int i = 0; i < RESUME_SNAP_SLOTS; i++)
				if (RjReadSnapSlot(SnapPath(i), m_strGlassID, pf) == 1 && pf->nSerial > m_nSnapSerial) m_nSnapSerial = pf->nSerial;
			delete pf;
			if (!vecExtraKeep.empty()) strLog.Format(_T("ResumeJournal: %d late line(s) kept at the journal swap"), (int)vecExtraKeep.size());
			bOK = true;
		}
		m_bPend = false;
	}
	::LeaveCriticalSection(&m_cs);
	RjLog(SYST, strLog);
	RjLog(WARN, strWarn);
	return bOK;
}

//----------------------------------------------------------------------------------------------------
// replay (plan (4-4))
//----------------------------------------------------------------------------------------------------
namespace
{
	struct CFastUploadGuard		// FALSE again on every exit path
	{
		CResultMgr* m_p;
		explicit CFastUploadGuard(CResultMgr* p) : m_p(p)	{ m_p->SetFastUpload(true); }
		~CFastUploadGuard()									{ m_p->SetFastUpload(false); }
	};
}

bool CResumeJournal::Replay(const std::vector<CStringA>& vecKeep, S_ResumeReplayStat& st, int& nResult, ResumeContinueFn fnContinue)
{
	st = S_ResumeReplayStat();
	nResult = RESUME_REPLAY_OK;
	DWORD dwStart = ::GetTickCount();

	std::vector<CStringA> vecAll(vecKeep);
	CString strGlass, strRecipe, strSimPath;
	int nSim = 0;
	::EnterCriticalSection(&m_cs);
	vecAll.insert(vecAll.end(), m_vecSwapExtra.begin(), m_vecSwapExtra.end());
	m_vecSwapExtra.clear();
	st.nDropped	= m_nLastDropped;
	st.nTorn	= m_nLastTorn;
	strGlass	= m_strGlassID;
	strRecipe	= m_strRecipeID;
	strSimPath	= m_strFolder + _T("\\ResumeSim\\");		// never exists: dResult answers FILE_NG and writes no row
	nSim		= m_nSimMode;
	::LeaveCriticalSection(&m_cs);

	CResultMgr* pRes = CResultMgr::GetInstance();
	std::vector<CString> vecMiss;
	CString strFail;
	RSRITEMDATA* prd = new RSRITEMDATA;
	// LYS 2026-10-12 Hybrid Phase 6 replay progress for the operate screen: R / S lines to handle (the loop below counts each one it reaches)
	//   LYS 2026-10-12 Hybrid Phase 6 fix (review correctness 3): published after the allocation - a throwing new left the total set and the label stuck
	{
		LONG nCand = 0;
		for (size_t c = 0; c < vecAll.size(); c++)
			if (!vecAll[c].IsEmpty() && (vecAll[c][0] == 'R' || vecAll[c][0] == 'S')) nCand++;
		::InterlockedExchange(&m_nReplayDone, 0);
		::InterlockedExchange(&m_nReplayTotal, nCand);
	}
	try
	{
		CFastUploadGuard guard(pRes);
		int q = 0, qLast = 0, nIdle = 0, nStable = 0, nSent = 0;
		long nDone = 0, nDoneLast = 0, nDone0 = 0;

		// (a) late uploads between Recipe None and now (ANGLE, a waiting VISION slot): let dResult drain and settle, then clear again (review K9)
		pRes->GetUploadState(&q, &nDone);
		qLast = q;
		nDoneLast = nDone;
		for (;;)
		{
			if (q == 0 && nStable >= RJ_SETTLE_POLLS) break;
			if (fnContinue && !fnContinue()) { nResult = RESUME_REPLAY_STOPPED; break; }
			::Sleep(10);
			pRes->GetUploadState(&q, &nDone);
			bool bProgress = (q < qLast) || (nDone != nDoneLast);
			nStable = (q == 0 && nDone == nDoneLast) ? nStable + 1 : 0;
			if (q > 0 && !bProgress)
			{
				if (++nIdle >= RESUME_POLL_IDLE_MAX) { nResult = RESUME_REPLAY_FAILED; strFail = _T("dResult queue not draining"); break; }
			}
			else nIdle = 0;
			qLast = q;
			nDoneLast = nDone;
		}

		if (nResult == RESUME_REPLAY_OK)
		{
			pRes->ClearData();
			pRes->GetUploadState(&q, &nDone0);
			nDoneLast = nDone0;
			nIdle = 0;
			std::vector<CStringA> f;
			CString strLog;
			for (size_t i = 0; i < vecAll.size() && nResult == RESUME_REPLAY_OK; i++)
			{
				if (fnContinue && !fnContinue()) { nResult = RESUME_REPLAY_STOPPED; break; }
				const CStringA& line = vecAll[i];
				if (line.IsEmpty()) continue;
				char cType = line[0];
				if (cType != 'R' && cType != 'S') continue;		// P / X: pairing only
				::InterlockedIncrement(&m_nReplayDone);	// LYS 2026-10-12 Hybrid Phase 6 (sent, sim or skipped - every R / S line reached)
				RjSplit(line, f);
				S_ResumeTuple t = RjTup(RjInt(f.size() > 5 ? f[3] : CStringA(), 0), RjInt(f.size() > 5 ? f[4] : CStringA(), 0), RjInt(f.size() > 5 ? f[5] : CStringA(), 0));
				CStringA strKind;
				if (cType == 'S')
				{
					if (nSim <= 0 || f.size() != 12) continue;
					ZeroMemory(prd, sizeof(RSRITEMDATA));
					prd->nMeasMode = RjInt(f[8], PGM_MODE_LUMIN);
					prd->iMeasArea = RjInt(f[9], 0);
					RjPut(prd->szGlassID, _countof(prd->szGlassID), strGlass);
					RjPut(prd->szRecipeID, _countof(prd->szRecipeID), strRecipe);
					RjPut(prd->szCellData, _countof(prd->szCellData), RjField(f[10]));
					RjPut(prd->szResultPath, _countof(prd->szResultPath), strSimPath);
					CString strFile;
					strFile.Format(_T("S%s.txt"), CString(f[1]).GetString());
					RjPut(prd->szResultFile, _countof(prd->szResultFile), strFile);
					strKind = f[7];
					st.nSim++;
				}
				else
				{
					if (!RjParseR(f, prd)) continue;
					CString strRaw = CString(prd->szResultPath) + prd->szResultFile;
					if (prd->szResultFile[0] == 0 || !RjFileExists(strRaw))
					{
						st.nMissing++;
						if (vecMiss.size() < 5) vecMiss.push_back(strRaw);
						continue;
					}
					strKind = f[7];
					if (prd->nMeasMode == PGM_MODE_LUMIN)		st.nWad++;
					else if (prd->nMeasMode == PGM_MODE_VISION)	st.nVision++;
					else if (prd->nMeasMode == PGM_MODE_ANGLE)	st.nAngle++;
				}
				if (nSim > 0)
				{
					strLog.Format(_T("[RESUME] replay %c %s row(%d) T%d seg %d stick %d"), (TCHAR)cType, CString(strKind).GetString(), prd->iMeasArea, t.nPass, t.nSeg + 1, t.nStick + 1);
					RjLog(SYST, strLog);
				}
				// pace: keep the dResult ring below the cap (20 slots, oldest dropped when full)
				pRes->GetUploadState(&q, &nDone);
				qLast = q;
				while (q >= RESUME_REPLAY_QUEUE_CAP)
				{
					if (fnContinue && !fnContinue()) { nResult = RESUME_REPLAY_STOPPED; break; }
					::Sleep(10);
					pRes->GetUploadState(&q, &nDone);
					if (q < qLast || nDone != nDoneLast) nIdle = 0;
					else if (++nIdle >= RESUME_POLL_IDLE_MAX) { nResult = RESUME_REPLAY_FAILED; strFail = _T("dResult queue stalled"); break; }
					qLast = q;
					nDoneLast = nDone;
				}
				if (nResult != RESUME_REPLAY_OK) break;
				pRes->Updata_Data(prd);		// not armed: the hook does not journal it again
				nSent++;
				st.nRows++;
			}

			// every replayed row processed by the dResult monitor thread
			nIdle = 0;
			while (nResult == RESUME_REPLAY_OK && (nDone - nDone0) < nSent)
			{
				if (fnContinue && !fnContinue()) { nResult = RESUME_REPLAY_STOPPED; break; }
				::Sleep(10);
				pRes->GetUploadState(&q, &nDone);
				if (q < qLast || nDone != nDoneLast) nIdle = 0;
				else if (++nIdle >= RESUME_POLL_IDLE_MAX) { nResult = RESUME_REPLAY_FAILED; strFail = _T("dResult rows not finished"); break; }
				qLast = q;
				nDoneLast = nDone;
			}
		}
	}
	catch (...)
	{
		nResult = RESUME_REPLAY_FAILED;
		strFail = _T("exception");
	}
	delete prd;
	st.dwMs = ::GetTickCount() - dwStart;
	::InterlockedExchange(&m_nReplayTotal, 0);	// LYS 2026-10-12 Hybrid Phase 6 replay over
	::InterlockedExchange(&m_nReplayDone, 0);

	CString strLog;
	for (size_t i = 0; i < vecMiss.size(); i++)
	{
		strLog.Format(_T("[RESUME] journal replay: raw file missing %s"), vecMiss[i].GetString());
		RjLog(SYST, strLog);
	}
	if (nResult == RESUME_REPLAY_FAILED)
	{
		strLog.Format(_T("ResumeJournal: replay failed (%s) after %d rows"), strFail.GetString(), st.nRows);
		RjLog(SYST, strLog);
	}
	return (nResult == RESUME_REPLAY_OK);
}

//----------------------------------------------------------------------------------------------------
// cell status ring (plan (5-1))
//----------------------------------------------------------------------------------------------------
bool CResumeJournal::SaveCellStatus(int (*pnStatus)[RESUME_CELL_Y], int nPass, int nSeg, int nStick)
{
	if (pnStatus == NULL) return false;
	S_RcsFile* pf = new S_RcsFile;
	ZeroMemory(pf, sizeof(S_RcsFile));
	memcpy(pf->szMagic, "RCS1", 4);
	pf->nVersion	= 1;
	pf->nPass		= nPass;
	pf->nSeg		= nSeg;
	pf->nStick		= nStick;
	memcpy(pf->nStatus, pnStatus, sizeof(pf->nStatus));

	CString strWarn;
	DWORD dwErr = 0;
	::EnterCriticalSection(&m_cs);
	RjPut(pf->szGlassID, _countof(pf->szGlassID), m_strGlassID);
	pf->nAttempt	= m_ctx.nAttempt;
	pf->nSerial		= ++m_nSnapSerial;
	pf->dwFnv		= Fnv1a(pf, offsetof(S_RcsFile, dwFnv));
	if (m_strFolder.IsEmpty()) dwErr = ERROR_PATH_NOT_FOUND;
	else
	{
		m_bFilesMayExist = true;		// LYS 2026-10-08 Hybrid Phase 5 fix (N=1 review 1) ring tmp + slot
		HANDLE h = ::CreateFile(m_strSnapTmp, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (h == INVALID_HANDLE_VALUE) dwErr = ::GetLastError();
		else
		{
			dwErr = RjWriteAll(h, (const char*)pf, (DWORD)sizeof(S_RcsFile));
			if (dwErr == 0 && !::FlushFileBuffers(h)) dwErr = ::GetLastError();
			::CloseHandle(h);
		}
		if (dwErr == 0 && !::MoveFileEx(m_strSnapTmp, SnapPath(pf->nSerial % RESUME_SNAP_SLOTS), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)) dwErr = ::GetLastError();
	}
	if (dwErr != 0 && !m_bWarnedSnap)
	{
		m_bWarnedSnap = true;
		strWarn.Format(_T("ResumeJournal: cell status snapshot write failed (%u) at T%d seg %d stick %d"), dwErr, nPass, nSeg + 1, nStick + 1);
	}
	::LeaveCriticalSection(&m_cs);
	delete pf;
	RjLog(WARN, strWarn);
	return (dwErr == 0);
}

// LYS 2026-10-08 stage 5-1 completion: back to IDLE the nCellStatus columns of the sticks at or after T in the measure order of T's pass.
//   Column = segment stick base + (rotate 1 ? local stick : sticks - 1 - local stick) (CDoWorkGlassMeasureStep::MakeCellID + ToCellStatusStick),
//   so "columns >= base + stick" (plan (5-1) gs(T)) is wrong for a segment whose rotate is not 1. Unreadable shape: nothing cleared (display only).
static void RjClearFromT(int (*pnStatus)[RESUME_CELL_Y], const S_ResumeTuple& T, const CString& strSegShape)
{
	std::vector<int> vecSticks, vecRot;
	int nSeg = CResumeJournal::ParseSegShape(strSegShape, vecSticks, NULL, &vecRot);
	if (nSeg <= 0 || T.nSeg < 0 || T.nSeg >= nSeg || T.nStick < 0) return;
	int nBase = 0;
	for (int s = 0; s < nSeg; s++)
	{
		int nMax = vecSticks[s];
		for (int i = 0; i < nMax; i++)
		{
			if (s < T.nSeg || (s == T.nSeg && i < T.nStick)) continue;		// measured before T in this pass
			int nCol = nBase + ((vecRot[s] == 1) ? i : nMax - 1 - i);
			if (nCol < 0 || nCol >= RESUME_CELL_Y) continue;
			for (int x = 0; x < RESUME_CELL_X; x++) pnStatus[x][nCol] = RJ_CELLSTATE_IDLE;
		}
		nBase += nMax;
	}
}

int CResumeJournal::LoadCellStatus(int (*pnStatus)[RESUME_CELL_Y], const S_ResumeTuple& pred, bool bEfa, const S_ResumeTuple& T, const CString& strSegShape, S_ResumeTuple& got) const
{
	got = RjTup(0, 0, 0);
	if (pnStatus == NULL) return RESUME_SNAP_NONE;
	CString strGlass, arrPath[RESUME_SNAP_SLOTS];
	::EnterCriticalSection(&m_cs);
	strGlass = m_strGlassID;
	for (int i = 0; i < RESUME_SNAP_SLOTS; i++) arrPath[i] = SnapPath(i);
	::LeaveCriticalSection(&m_cs);

	S_RcsFile* pf = new S_RcsFile;
	S_RcsFile* pBest = new S_RcsFile;
	int nExact = -1, nOlder = -1;		// best serial
	for (int i = 0; i < RESUME_SNAP_SLOTS; i++)
	{
		if (RjReadSnapSlot(arrPath[i], strGlass, pf) != 1) continue;
		int c = CompareTuple(RjTup(pf->nPass, pf->nSeg, pf->nStick), pred);
		if (c == 0 && pf->nSerial > nExact) { nExact = pf->nSerial; if (nExact >= 0) memcpy(pBest, pf, sizeof(S_RcsFile)); }
	}
	int nRet = RESUME_SNAP_NONE;
	bool bClear = false;
	if (nExact >= 0)
	{
		nRet = RESUME_SNAP_EXACT;
		bClear = (T.nPass == 0);		// T0: columns of the sticks >= T back to IDLE (T1 keeps the T0 final values)
	}
	else if (!bEfa)
	{
		for (int i = 0; i < RESUME_SNAP_SLOTS; i++)
		{
			if (RjReadSnapSlot(arrPath[i], strGlass, pf) != 1) continue;
			if (CompareTuple(RjTup(pf->nPass, pf->nSeg, pf->nStick), pred) < 0 && pf->nSerial > nOlder) { nOlder = pf->nSerial; memcpy(pBest, pf, sizeof(S_RcsFile)); }
		}
		if (nOlder >= 0) { nRet = RESUME_SNAP_OLDER; bClear = (T.nPass == 0); }	// LYS 2026-10-08 the all-done end marker (PassCount,0,0) clears nothing
	}
	if (nRet != RESUME_SNAP_NONE)
	{
		memcpy(pnStatus, pBest->nStatus, sizeof(pBest->nStatus));
		got = RjTup(pBest->nPass, pBest->nSeg, pBest->nStick);
		if (bClear) RjClearFromT(pnStatus, T, strSegShape);	// LYS 2026-10-08 stage 5-1 completion: real column mapping (was columns >= gs(T))
	}
	delete pf;
	delete pBest;
	return nRet;
}

void CResumeJournal::NoteCellStatusReset()
{
	InterlockedIncrement(&m_nResetCount);
}

void CResumeJournal::MarkCellStatusReset()
{
	InterlockedExchange(&m_nResetMark, InterlockedCompareExchange(&m_nResetCount, 0, 0));
}

int CResumeJournal::WaitCellStatusReset(DWORD dwMs, ResumeContinueFn fnContinue)
{
	LONG nMark = InterlockedExchange(&m_nResetMark, -1);	// consumed
	if (nMark < 0) return 0;
	int nPolls = (int)(dwMs / 10);
	if (nPolls < 1) nPolls = 1;
	for (int i = 0; ; i++)
	{
		if (InterlockedCompareExchange(&m_nResetCount, 0, 0) != nMark) return 1;
		if (i >= nPolls) return 0;
		if (fnContinue && !fnContinue()) return -1;
		::Sleep(10);
	}
}

//----------------------------------------------------------------------------------------------------
// D5-15 VISION recipe wait on resume (review C7)
//----------------------------------------------------------------------------------------------------
void CResumeJournal::ArmResumeVisionWait()
{
	::EnterCriticalSection(&m_cs);
	m_bVisWait = true;
	m_nVisWaitGen = m_nGlassGen;		// the generation the following Arm(attempt) arms
	::LeaveCriticalSection(&m_cs);
}

bool CResumeJournal::ConsumeResumeVisionWait()
{
	if (m_bVisWait == false) return false;	// never armed (normal runs): no lock
	::EnterCriticalSection(&m_cs);
	bool b = m_bVisWait && (m_nArmedGen != 0) && (m_nArmedGen == m_nVisWaitGen);
	m_bVisWait = false;
	::LeaveCriticalSection(&m_cs);
	return b;
}
