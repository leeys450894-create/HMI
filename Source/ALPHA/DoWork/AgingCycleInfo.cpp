// AgingCycleInfo.cpp
// LYS 2026-08-06 AgingCycle E2: 에이징 사이클 레시피 로더(목록 + [MASECELL] 맵).
// 파일 형식은 CDlgSubManual_AgingTest가 저장하는 그대로:
//   [COUNT] MaxCount=N
//   [ITEM_n] USE_AGING / PTN_RGB / PTN_SEC / USE_PROPERTY / USE_QUALITY / USE_ANGLE
//   [MASECELL] STICK_n = "m,m,..."  (셀당 3bit 마스크, bit0..2 = WAD/VISION/ANGLE)
//   LYS 2026-08-06: HMI has no COLOR mode (OCP 4 kinds -> HMI 3 kinds)
#include "pch.h"
#include "AgingCycleInfo.h"

#define DEFINE_AGING_CYCLE_STICK_MAX	100		// STICK_n scan upper bound (safety)

CAgingCycleInfo::CAgingCycleInfo()
{
}

CAgingCycleInfo::~CAgingCycleInfo()
{
}

CString CAgingCycleInfo::GetAgingRcpPath()
{
	return AfxGetApp()->GetProfileString(_T("DIR"), _T("AGING_RCP"),
		_T("D:\\HBS\\PARAM_OCP\\PARAM\\AgingRcp\\"));
}

void CAgingCycleInfo::Clear()
{
	m_vecItem.clear();
	m_vecCellMask.clear();
}

bool CAgingCycleInfo::Load(CString strRcpName)
{
	Clear();

	if (strRcpName.IsEmpty() || strRcpName == _T("UNKNOWN")) return false;

	CString strPath;
	strPath.Format(_T("%s%s.rcp"), (LPCTSTR)GetAgingRcpPath(), (LPCTSTR)strRcpName);

	TCHAR cValue[1024];
	ZeroMemory(cValue, sizeof(cValue));

	// ---- item list ----
	GetPrivateProfileString(_T("COUNT"), _T("MaxCount"), _T("0"), cValue, MAX_PATH, strPath);
	int nMaxCount = _ttoi(cValue);
	if (nMaxCount <= 0) return false;

	CString strItem;
	S_AgingTest sItem;
	for (int i = 0; i < nMaxCount; i++) {
		strItem.Format(_T("ITEM_%d"), i + 1);

		GetPrivateProfileString(strItem, _T("USE_AGING"), _T("0"), cValue, MAX_PATH, strPath);
		sItem.m_bUseAging = _ttoi(cValue);

		GetPrivateProfileString(strItem, _T("PTN_RGB"), _T("0"), cValue, MAX_PATH, strPath);
		sItem.m_nPtnRGB = _ttoi(cValue);

		GetPrivateProfileString(strItem, _T("PTN_SEC"), _T("0"), cValue, MAX_PATH, strPath);
		sItem.m_nPtnSec = _ttoi(cValue);

		GetPrivateProfileString(strItem, _T("USE_PROPERTY"), _T("0"), cValue, MAX_PATH, strPath);
		sItem.m_bUseProperty = _ttoi(cValue);

		GetPrivateProfileString(strItem, _T("USE_QUALITY"), _T("0"), cValue, MAX_PATH, strPath);
		sItem.m_bUseQuality = _ttoi(cValue);

		GetPrivateProfileString(strItem, _T("USE_ANGLE"), _T("0"), cValue, MAX_PATH, strPath);
		sItem.m_bUseAngle = _ttoi(cValue);

		m_vecItem.push_back(sItem);
	}

	// ---- [MASECELL] cell map: STICK_1.. until missing ----
	for (int iStick = 0; iStick < DEFINE_AGING_CYCLE_STICK_MAX; iStick++) {
		ZeroMemory(cValue, sizeof(cValue));
		GetPrivateProfileString(_T("MASECELL"), dfcStr(_T("STICK_%d"), iStick + 1), _T(""),
			cValue, _countof(cValue), strPath);

		CString strRow = cValue;
		if (strRow.IsEmpty()) break;

		std::vector<int> vecRow;
		int pos = 0;
		CString resToken = strRow.Tokenize(_T(","), pos);
		while (resToken != _T("")) {
			vecRow.push_back(_ttoi(resToken.Trim()));
			resToken = strRow.Tokenize(_T(","), pos);
		}

		m_vecCellMask.push_back(vecRow);
	}

	return true;
}

const S_AgingTest* CAgingCycleInfo::GetItem(int iItem) const
{
	if (iItem < 0 || iItem >= (int)m_vecItem.size()) return NULL;
	return &m_vecItem[iItem];
}

// LYS 2026-08-12 AgingCycle R1: 해당 종류를 쓰는 활성 항목 수
// (OCP nMaxProperty/nMaxeQuality/nMaxLCR 대응, JOB_Proc.cpp:12562-12570).
int CAgingCycleInfo::GetKindUseCount(int iKind) const
{
	int nCount = 0;

	for (size_t i = 0; i < m_vecItem.size(); i++)
	{
		if (m_vecItem[i].m_bUseAging == FALSE) continue;

		if (iKind == enumAgingMeasKind_WAD)
		{
			if (m_vecItem[i].m_bUseProperty) nCount++;
		}
		else if (iKind == enumAgingMeasKind_VISION)
		{
			if (m_vecItem[i].m_bUseQuality) nCount++;
		}
		else if (iKind == enumAgingMeasKind_ANGLE)
		{
			if (m_vecItem[i].m_bUseAngle) nCount++;
		}
	}

	return nCount;
}

int CAgingCycleInfo::GetCellCount(int iStick) const
{
	if (iStick < 0 || iStick >= (int)m_vecCellMask.size()) return 0;
	return (int)m_vecCellMask[iStick].size();
}

bool CAgingCycleInfo::IsCellUse(int iStick, int iCell, int iKind) const
{
	if (iKind < 0 || iKind >= enumAgingMeasKind_MAX) return false;
	if (iStick < 0 || iStick >= (int)m_vecCellMask.size()) return false;
	if (iCell < 0 || iCell >= (int)m_vecCellMask[iStick].size()) return false;

	return (((m_vecCellMask[iStick][iCell] >> iKind) & 0x01) != 0);
}

bool CAgingCycleInfo::IsStickUse(int iStick, int iKind) const
{
	if (iStick < 0 || iStick >= (int)m_vecCellMask.size()) return false;

	for (size_t i = 0; i < m_vecCellMask[iStick].size(); i++) {
		if (IsCellUse(iStick, (int)i, iKind)) return true;
	}

	return false;
}
