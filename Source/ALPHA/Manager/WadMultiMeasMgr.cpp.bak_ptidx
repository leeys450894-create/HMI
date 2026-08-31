// LYS 2026-07-23 WAD Multi Point measurement data manager (Phase 2)
// See WadMultiMeasMgr.h for design notes and OCP file compatibility.

#include "pch.h"
#include "WadMultiMeasMgr.h"

#define WAD_MULTI_INI_BUF		4096	// OCP used MAX_PATH(260) -> silent truncation; enlarged
#define WAD_MULTI_MAX_STICK		100		// [MASECELL] STICK_n scan upper bound

// [DATA] field order (OCP pAngleItem[] order)
enum
{
	eMM_FIELD_CLASS = 0,
	eMM_FIELD_SUB_CLASS,
	eMM_FIELD_X,
	eMM_FIELD_Y,
	eMM_FIELD_ENABLE,
	eMM_FIELD_RX,
	eMM_FIELD_GX,
	eMM_FIELD_BY,
	eMM_FIELD_WX,
	eMM_FIELD_WY,
	eMM_FIELD_COUNT,
};

static LPCTSTR s_szFieldKey[eMM_FIELD_COUNT] =
{
	_T("CLASS"), _T("SUB_CLASS"), _T("X"), _T("Y"), _T("ENABLE"),
	_T("RX"), _T("GX"), _T("BY"), _T("WX"), _T("WY"),
};

CWadMultiMeasMgr::CWadMultiMeasMgr(CString strRcpFolderPath)
	: m_strRcpFolderPath(strRcpFolderPath)
	, m_bLoaded(false)
	, m_bInspMode(false)
	, m_dCellX(0.0)
	, m_dCellY(0.0)
{
	if (!m_strRcpFolderPath.IsEmpty() && m_strRcpFolderPath.Right(1) != _T("\\"))
		m_strRcpFolderPath += _T("\\");

	m_bRunActive[0] = m_bRunActive[1] = false;
	m_nRunCurrent[0] = m_nRunCurrent[1] = 0;
}

CWadMultiMeasMgr::~CWadMultiMeasMgr()
{
}

//---------------------------------------------------------------
// Path
//---------------------------------------------------------------
void CWadMultiMeasMgr::SetRcpFolderPath(CString strPath)
{
	CSingleLock lock(&m_cs, TRUE);

	m_strRcpFolderPath = strPath;
	if (!m_strRcpFolderPath.IsEmpty() && m_strRcpFolderPath.Right(1) != _T("\\"))
		m_strRcpFolderPath += _T("\\");
}

CString CWadMultiMeasMgr::GetRcpFolderPath()
{
	CSingleLock lock(&m_cs, TRUE);
	return m_strRcpFolderPath;
}

CString CWadMultiMeasMgr::GetRcpFilePath(CString strRecipeID)
{
	CSingleLock lock(&m_cs, TRUE);

	CString strPath;
	strPath.Format(_T("%s%s.rcp"), (LPCTSTR)m_strRcpFolderPath, (LPCTSTR)strRecipeID);
	return strPath;
}

//---------------------------------------------------------------
// Clear
//---------------------------------------------------------------
void CWadMultiMeasMgr::Clear()
{
	CSingleLock lock(&m_cs, TRUE);

	m_bLoaded = false;
	m_strLoadedRecipeID.Empty();
	m_bInspMode = false;
	m_dCellX = 0.0;
	m_dCellY = 0.0;
	m_vecPoint.clear();
	m_vecCellUse.clear();
}

//---------------------------------------------------------------
// LoadMeasData
//---------------------------------------------------------------
bool CWadMultiMeasMgr::LoadMeasData(CString strRecipeID)
{
	CSingleLock lock(&m_cs, TRUE);

	m_bLoaded = false;
	m_strLoadedRecipeID.Empty();
	m_vecPoint.clear();
	m_vecCellUse.clear();
	m_bInspMode = false;
	m_dCellX = 0.0;
	m_dCellY = 0.0;

	CString strFile;
	strFile.Format(_T("%s%s.rcp"), (LPCTSTR)m_strRcpFolderPath, (LPCTSTR)strRecipeID);

	if (::GetFileAttributes(strFile) == INVALID_FILE_ATTRIBUTES)
		return false;

	TCHAR szBuf[WAD_MULTI_INI_BUF];

	//-----------------------------------------------------------
	// [DATA] point list (comma-joined lists per field)
	//-----------------------------------------------------------
	for (int nField = 0; nField < eMM_FIELD_COUNT; nField++)
	{
		ZeroMemory(szBuf, sizeof(szBuf));
		::GetPrivateProfileString(_T("DATA"), s_szFieldKey[nField], _T(""), szBuf, WAD_MULTI_INI_BUF, strFile);
		ParseJoinedList(nField, szBuf);
	}

	//-----------------------------------------------------------
	// [DATA] INSP_MODE / CELL_X / CELL_Y
	//-----------------------------------------------------------
	m_bInspMode = (::GetPrivateProfileInt(_T("DATA"), _T("INSP_MODE"), 0, strFile) != 0);

	ZeroMemory(szBuf, sizeof(szBuf));
	::GetPrivateProfileString(_T("DATA"), _T("CELL_X"), _T("0"), szBuf, WAD_MULTI_INI_BUF, strFile);
	m_dCellX = _tstof(szBuf);

	ZeroMemory(szBuf, sizeof(szBuf));
	::GetPrivateProfileString(_T("DATA"), _T("CELL_Y"), _T("0"), szBuf, WAD_MULTI_INI_BUF, strFile);
	m_dCellY = _tstof(szBuf);

	//-----------------------------------------------------------
	// [MASECELL] STICK_1..N  (section name kept for OCP file compat)
	//-----------------------------------------------------------
	for (int iStick = 0; iStick < WAD_MULTI_MAX_STICK; iStick++)
	{
		CString strKey;
		strKey.Format(_T("STICK_%d"), iStick + 1);

		ZeroMemory(szBuf, sizeof(szBuf));
		::GetPrivateProfileString(_T("MASECELL"), strKey, _T(""), szBuf, WAD_MULTI_INI_BUF, strFile);

		if (szBuf[0] == 0)
			break;

		std::vector<int> vecRow;
		CString strData = szBuf;
		CString strToken;
		int nPos = 0;

		strToken = strData.Tokenize(_T(","), nPos);
		while (!strToken.IsEmpty())
		{
			vecRow.push_back(_tstoi(strToken.Trim()) != 0 ? 1 : 0);
			strToken = strData.Tokenize(_T(","), nPos);
		}

		m_vecCellUse.push_back(vecRow);
	}

	m_bLoaded = true;
	m_strLoadedRecipeID = strRecipeID;

	return true;
}

//---------------------------------------------------------------
// SaveMeasData
//---------------------------------------------------------------
bool CWadMultiMeasMgr::SaveMeasData(CString strRecipeID)
{
	CSingleLock lock(&m_cs, TRUE);

	if (m_strRcpFolderPath.IsEmpty() || strRecipeID.IsEmpty())
		return false;

	::CreateDirectory(m_strRcpFolderPath, NULL);

	CString strFile;
	strFile.Format(_T("%s%s.rcp"), (LPCTSTR)m_strRcpFolderPath, (LPCTSTR)strRecipeID);

	//-----------------------------------------------------------
	// [DATA] point list
	//-----------------------------------------------------------
	bool bOK = true;

	for (int nField = 0; nField < eMM_FIELD_COUNT; nField++)
	{
		CString strJoined = BuildJoinedList(nField);
		bOK &= (::WritePrivateProfileString(_T("DATA"), s_szFieldKey[nField], strJoined, strFile) != 0);
	}

	//-----------------------------------------------------------
	// [DATA] INSP_MODE / CELL_X / CELL_Y
	//-----------------------------------------------------------
	CString strVal;
	strVal.Format(_T("%d"), m_bInspMode ? 1 : 0);
	bOK &= (::WritePrivateProfileString(_T("DATA"), _T("INSP_MODE"), strVal, strFile) != 0);

	strVal.Format(_T("%.4f"), m_dCellX);
	bOK &= (::WritePrivateProfileString(_T("DATA"), _T("CELL_X"), strVal, strFile) != 0);

	strVal.Format(_T("%.4f"), m_dCellY);
	bOK &= (::WritePrivateProfileString(_T("DATA"), _T("CELL_Y"), strVal, strFile) != 0);

	//-----------------------------------------------------------
	// [MASECELL]
	//-----------------------------------------------------------
	for (size_t iStick = 0; iStick < m_vecCellUse.size(); iStick++)
	{
		CString strKey, strRow, strCell;
		strKey.Format(_T("STICK_%d"), (int)iStick + 1);

		for (size_t iCell = 0; iCell < m_vecCellUse[iStick].size(); iCell++)
		{
			strCell.Format(_T("%d,"), m_vecCellUse[iStick][iCell]);
			strRow += strCell;
		}

		bOK &= (::WritePrivateProfileString(_T("MASECELL"), strKey, strRow, strFile) != 0);
	}

	if (bOK)
	{
		m_bLoaded = true;
		m_strLoadedRecipeID = strRecipeID;
	}

	return bOK;
}

//---------------------------------------------------------------
// Getters
//---------------------------------------------------------------
bool CWadMultiMeasMgr::IsLoaded()
{
	CSingleLock lock(&m_cs, TRUE);
	return m_bLoaded;
}

CString CWadMultiMeasMgr::GetLoadedRecipeID()
{
	CSingleLock lock(&m_cs, TRUE);
	return m_strLoadedRecipeID;
}

bool CWadMultiMeasMgr::IsMultiMode()
{
	CSingleLock lock(&m_cs, TRUE);
	return m_bInspMode;
}

int CWadMultiMeasMgr::GetPointCount()
{
	CSingleLock lock(&m_cs, TRUE);
	return (int)m_vecPoint.size();
}

bool CWadMultiMeasMgr::GetPoint(int nIndex, S_WadMultiPoint& outPoint)
{
	CSingleLock lock(&m_cs, TRUE);

	if (nIndex < 0 || nIndex >= (int)m_vecPoint.size())
		return false;

	outPoint = m_vecPoint[nIndex];
	return true;
}

int CWadMultiMeasMgr::GetUseSize()
{
	CSingleLock lock(&m_cs, TRUE);

	int nUse = 0;
	for (size_t i = 0; i < m_vecPoint.size(); i++)
	{
		if (m_vecPoint[i].bEnable)
			nUse++;
	}

	return nUse;
}

int CWadMultiMeasMgr::GetCheckNumData(int nCheckNum)
{
	CSingleLock lock(&m_cs, TRUE);

	int nCheckCnt = 0;
	for (size_t i = 0; i < m_vecPoint.size(); i++)
	{
		if (m_vecPoint[i].bEnable)
			nCheckCnt++;

		if (nCheckCnt == nCheckNum)
			return (int)i;
	}

	return 0;	// OCP semantics: fallback 0 when not found
}

double CWadMultiMeasMgr::GetCellX()
{
	CSingleLock lock(&m_cs, TRUE);
	return m_dCellX;
}

double CWadMultiMeasMgr::GetCellY()
{
	CSingleLock lock(&m_cs, TRUE);
	return m_dCellY;
}

bool CWadMultiMeasMgr::IsMultiCell(int iStick, int iCell)
{
	CSingleLock lock(&m_cs, TRUE);

	// out of range = false (OCP had iArea > iAreaMax off-by-one; fixed here)
	if (iStick < 0 || iStick >= (int)m_vecCellUse.size())
		return false;
	if (iCell < 0 || iCell >= (int)m_vecCellUse[iStick].size())
		return false;

	return (m_vecCellUse[iStick][iCell] != 0);
}

int CWadMultiMeasMgr::GetCellUseStickCount()
{
	CSingleLock lock(&m_cs, TRUE);
	return (int)m_vecCellUse.size();
}

int CWadMultiMeasMgr::GetCellUseCellCount(int iStick)
{
	CSingleLock lock(&m_cs, TRUE);

	if (iStick < 0 || iStick >= (int)m_vecCellUse.size())
		return 0;

	return (int)m_vecCellUse[iStick].size();
}

//---------------------------------------------------------------
// runtime state (Phase 4)
//---------------------------------------------------------------
void CWadMultiMeasMgr::ResetRunState()
{
	CSingleLock lock(&m_cs, TRUE);

	m_bRunActive[0] = m_bRunActive[1] = false;
	m_nRunCurrent[0] = m_nRunCurrent[1] = 0;
}

void CWadMultiMeasMgr::SetRunActive(int iProbe, bool bActive)
{
	CSingleLock lock(&m_cs, TRUE);

	if (iProbe < 0 || iProbe >= 2)
		return;

	m_bRunActive[iProbe] = bActive;
}

bool CWadMultiMeasMgr::IsRunActive(int iProbe)
{
	CSingleLock lock(&m_cs, TRUE);

	if (iProbe < 0 || iProbe >= 2)
		return false;

	return m_bRunActive[iProbe];
}

void CWadMultiMeasMgr::SetRunCurrent(int iProbe, int nCurrent)
{
	CSingleLock lock(&m_cs, TRUE);

	if (iProbe < 0 || iProbe >= 2)
		return;

	m_nRunCurrent[iProbe] = nCurrent;
}

int CWadMultiMeasMgr::GetRunCurrent(int iProbe)
{
	CSingleLock lock(&m_cs, TRUE);

	if (iProbe < 0 || iProbe >= 2)
		return 0;

	return m_nRunCurrent[iProbe];
}

//---------------------------------------------------------------
// Setters (UI side)
//---------------------------------------------------------------
void CWadMultiMeasMgr::SetInspMode(bool bMulti)
{
	CSingleLock lock(&m_cs, TRUE);
	m_bInspMode = bMulti;
}

void CWadMultiMeasMgr::SetCellSize(double dCellX, double dCellY)
{
	CSingleLock lock(&m_cs, TRUE);
	m_dCellX = dCellX;
	m_dCellY = dCellY;
}

void CWadMultiMeasMgr::SetPoints(const std::vector<S_WadMultiPoint>& vecPoints)
{
	CSingleLock lock(&m_cs, TRUE);
	m_vecPoint = vecPoints;
}

void CWadMultiMeasMgr::GetPoints(std::vector<S_WadMultiPoint>& outPoints)
{
	CSingleLock lock(&m_cs, TRUE);
	outPoints = m_vecPoint;
}

void CWadMultiMeasMgr::SetCellUseMap(const std::vector<std::vector<int>>& vecCellUse)
{
	CSingleLock lock(&m_cs, TRUE);
	m_vecCellUse = vecCellUse;
}

void CWadMultiMeasMgr::GetCellUseMap(std::vector<std::vector<int>>& outCellUse)
{
	CSingleLock lock(&m_cs, TRUE);
	outCellUse = m_vecCellUse;
}

bool CWadMultiMeasMgr::IsCellRangeValid(int& outNgIndex)
{
	CSingleLock lock(&m_cs, TRUE);

	outNgIndex = -1;

	double dRangeX = m_dCellX / 2.0;
	double dRangeY = m_dCellY / 2.0;

	for (size_t i = 0; i < m_vecPoint.size(); i++)
	{
		if (!m_vecPoint[i].bEnable)
			continue;

		if (m_vecPoint[i].dPosX >= dRangeX || m_vecPoint[i].dPosX <= -dRangeX ||
			m_vecPoint[i].dPosY >= dRangeY || m_vecPoint[i].dPosY <= -dRangeY)
		{
			outNgIndex = (int)i;
			return false;
		}
	}

	return true;
}

//---------------------------------------------------------------
// helpers (callers hold m_cs)
//---------------------------------------------------------------
CString CWadMultiMeasMgr::BuildJoinedList(int nField)
{
	CString strJoined, strItem;

	for (size_t i = 0; i < m_vecPoint.size(); i++)
	{
		const S_WadMultiPoint& pt = m_vecPoint[i];

		switch (nField)
		{
		case eMM_FIELD_CLASS:		strItem.Format(_T("%s,"), (LPCTSTR)pt.strMainClass);	break;
		case eMM_FIELD_SUB_CLASS:	strItem.Format(_T("%s,"), (LPCTSTR)pt.strSubClass);		break;
		case eMM_FIELD_X:			strItem.Format(_T("%.4f,"), pt.dPosX);					break;
		case eMM_FIELD_Y:			strItem.Format(_T("%.4f,"), pt.dPosY);					break;
		case eMM_FIELD_ENABLE:		strItem.Format(_T("%d,"), pt.bEnable ? 1 : 0);			break;
		case eMM_FIELD_RX:			strItem.Format(_T("%.4f,"), pt.fOffsetRX);				break;
		case eMM_FIELD_GX:			strItem.Format(_T("%.4f,"), pt.fOffsetGX);				break;
		case eMM_FIELD_BY:			strItem.Format(_T("%.4f,"), pt.fOffsetBY);				break;
		case eMM_FIELD_WX:			strItem.Format(_T("%.4f,"), pt.fOffsetWX);				break;
		case eMM_FIELD_WY:			strItem.Format(_T("%.4f,"), pt.fOffsetWY);				break;
		default:					strItem.Empty();										break;
		}

		strJoined += strItem;
	}

	return strJoined;
}

void CWadMultiMeasMgr::ParseJoinedList(int nField, LPCTSTR szData)
{
	CString strData = szData;
	CString strToken;
	int nPos = 0;
	size_t nIndex = 0;

	strToken = strData.Tokenize(_T(","), nPos);
	while (!strToken.IsEmpty())
	{
		// grow vector on first field; later fields fill existing entries
		if (nIndex >= m_vecPoint.size())
			m_vecPoint.push_back(S_WadMultiPoint());

		S_WadMultiPoint& pt = m_vecPoint[nIndex];

		switch (nField)
		{
		case eMM_FIELD_CLASS:		pt.strMainClass = strToken;				break;
		case eMM_FIELD_SUB_CLASS:	pt.strSubClass = strToken;				break;
		case eMM_FIELD_X:			pt.dPosX = _tstof(strToken);			break;
		case eMM_FIELD_Y:			pt.dPosY = _tstof(strToken);			break;
		case eMM_FIELD_ENABLE:		pt.bEnable = (_tstoi(strToken) != 0);	break;
		case eMM_FIELD_RX:			pt.fOffsetRX = _tstof(strToken);		break;
		case eMM_FIELD_GX:			pt.fOffsetGX = _tstof(strToken);		break;
		case eMM_FIELD_BY:			pt.fOffsetBY = _tstof(strToken);		break;
		case eMM_FIELD_WX:			pt.fOffsetWX = _tstof(strToken);		break;
		case eMM_FIELD_WY:			pt.fOffsetWY = _tstof(strToken);		break;
		default:															break;
		}

		nIndex++;
		strToken = strData.Tokenize(_T(","), nPos);
	}
}
