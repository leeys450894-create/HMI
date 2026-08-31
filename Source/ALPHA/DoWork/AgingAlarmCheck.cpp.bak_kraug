// AgingAlarmCheck.cpp
// LYS 2026-08-05 AgingSet Phase 4: non-UI aging target-current alarm checker.
// Parser kept byte-compatible with CDlgSubManual_AgingSet::FindAgingAlarm
// (which itself is the port of OCP C_DlgNaviManualAgingSet, JOB_Proc 6924-6969).
#include "pch.h"
#include "AgingAlarmCheck.h"

#include <fstream>
#include <string>
using namespace std;

CString CAgingAlarmCheck::GetTargetRcpPath()
{
	return AfxGetApp()->GetProfileString(_T("DIR"), _T("AGING_TARGET_RCP"),
		_T("D:\\HBS\\PARAM_OCP\\PARAM\\AgingTargetRcp\\"));
}

CString CAgingAlarmCheck::GetAgingFilePath()
{
	return AfxGetApp()->GetProfileString(_T("DIR"), _T("AGING_FILE"), _T("T:\\AGING.txt"));
}

BOOL CAgingAlarmCheck::GetAlarmUse()
{
	return (AfxGetApp()->GetProfileInt(_T("CONFIG"), _T("AGING_ALARM_USE"), 1) != 0);
}

void CAgingAlarmCheck::DeleteAgingResultFile()
{
	DeleteFile(GetAgingFilePath());
}

BOOL CAgingAlarmCheck::LoadTargetRcp(CString strRcpName, std::vector<S_AgingTargetAlarm>& vecOut)
{
	vecOut.clear();
	if (strRcpName.IsEmpty() || strRcpName == _T("UNKNOWN")) return FALSE;

	CString strPath;
	strPath.Format(_T("%s%s.rcp"), (LPCTSTR)GetTargetRcpPath(), (LPCTSTR)strRcpName);

	TCHAR cValue[MAX_PATH];
	ZeroMemory(cValue, sizeof(cValue));

	GetPrivateProfileString(_T("COUNT"), _T("MaxCount"), _T("0"), cValue, MAX_PATH, strPath);
	int nMaxCount = _ttoi(cValue);
	if (nMaxCount <= 0) return FALSE;

	CString strItem;
	S_AgingTargetAlarm sItem;
	for (int i = 0; i < nMaxCount; i++) {
		strItem.Format(_T("ITEM_%d"), i + 1);

		GetPrivateProfileString(strItem, _T("USE_ALARM"), _T("0"), cValue, MAX_PATH, strPath);
		sItem.m_bUseAlarm = _ttoi(cValue);

		GetPrivateProfileString(strItem, _T("PTN_RGB"), _T("0"), cValue, MAX_PATH, strPath);
		sItem.m_nPtnRGB = _ttoi(cValue);

		GetPrivateProfileString(strItem, _T("BASE_VAL"), _T("0.0"), cValue, MAX_PATH, strPath);
		sItem.m_fPtnBase = _tstof(cValue);

		GetPrivateProfileString(strItem, _T("UP_VAL"), _T("0.0"), cValue, MAX_PATH, strPath);
		sItem.m_fPtnUp = _tstof(cValue);

		GetPrivateProfileString(strItem, _T("LO_VAL"), _T("0.0"), cValue, MAX_PATH, strPath);
		sItem.m_fPtnLo = _tstof(cValue);

		vecOut.push_back(sItem);
	}

	return TRUE;
}

BOOL CAgingAlarmCheck::FileFind(CString strFile)
{
	int pos = 0;
	CString strID;
	CString resToken = strFile.Tokenize(_T("\\"), pos);
	while (resToken != _T("")) {
		strID = resToken.Trim();
		resToken = strFile.Tokenize(_T("\\"), pos);
	}

	BOOL bExist = FALSE;
	CFileFind finder;
	BOOL bWorking = finder.FindFile(strFile);
	while (bWorking) {
		bWorking = finder.FindNextFile();
		if (finder.IsDots() || finder.IsDirectory()) continue;
		if (strID == finder.GetFileName()) {
			bExist = TRUE;
			break;
		}
	}
	finder.Close();

	return bExist;
}

BOOL CAgingAlarmCheck::FileFindTimeOut(CString strFile, int nTimeOutMs)
{
	BOOL bTimeOut = FALSE;

	DWORD dwTimeout = (DWORD)nTimeOutMs;
	DWORD dwBeginTick = GetTickCount();
	DWORD dwRecheck = 1000;	// 1 sec
	DWORD dwCurrentTick = 0;

	if (!FileFind(strFile)) {
		while (1) {
			Sleep(100);
			dwCurrentTick = GetTickCount();

			if ((dwCurrentTick - dwBeginTick) > dwTimeout) {
				bTimeOut = TRUE;
				break;
			}
			else if ((dwCurrentTick - dwBeginTick) > dwRecheck) {
				if (FileFind(strFile)) break;
				dwRecheck += 1000;
			}
		}
	}

	return bTimeOut;
}

int CAgingAlarmCheck::Find(const std::vector<S_AgingTargetAlarm>& vecTarget, CString& strPtnOut, int& nCellOut)
{
	CString strTemp = GetAgingFilePath();
	CString strPtn;

	strPtnOut = _T("");
	nCellOut = 0;

	if (FileFindTimeOut(strTemp, 10000)) return AGING_NOT_FILE;

	char cLineRead[1024];
	ZeroMemory(cLineRead, sizeof(cLineRead));

	int nBegin = 0, nEnd = 0;
	CStringArray strArrData;

	ifstream fin;
	fin.open(strTemp);
	if (!fin.is_open()) return AGING_NOT_FILE;

	while (!fin.eof()) {
		fin.getline(cLineRead, 1024, '\n');
		strTemp = cLineRead;

		if (strTemp.IsEmpty()) break;

		if (strTemp.Find(_T("(")) < 0) continue;

		nBegin = 0; nEnd = 0; nCellOut = 0;
		int nLength = strTemp.GetLength();

		strArrData.RemoveAll();
		for (int i = 0; i < nLength; i++) {
			if (strTemp.GetAt(i) == ',' || i == nLength - 1) {

				if (i == nLength - 1) {
					nEnd = i;
				}
				else {
					nEnd = i - nBegin;
				}

				strArrData.Add(strTemp.Mid(nBegin, nEnd).Trim());
				nBegin = i + 1;
			}
		}

		nBegin = 0;
		nEnd = 0;
		int nItem = 0;
		BOOL bStart = FALSE;

		nLength = (int)strArrData.GetSize();
		for (int i = 0; i < nLength; i++) {
			strTemp = strArrData.GetAt(i);

			if (i == 0) {
				nEnd = strTemp.Find(_T("("));
				strPtn = strTemp.Mid(nBegin, nEnd);

				for (int j = 0; j < (int)vecTarget.size(); j++) {
					if (_ttoi(strPtn) == vecTarget[j].m_nPtnRGB) {
						bStart = TRUE;
						nItem = j;
						break;
					}
				}

				if (bStart) {
					if (!vecTarget[nItem].m_bUseAlarm) break;

					strPtnOut = strTemp;
				}
			}

			if (!bStart) break;

			if (i > 1 && i < nLength - 2) {
				nCellOut++;
				if (_tstof(strTemp) > vecTarget[nItem].m_fPtnBase + vecTarget[nItem].m_fPtnUp ||
					_tstof(strTemp) < vecTarget[nItem].m_fPtnBase - vecTarget[nItem].m_fPtnLo) {
					return AGING_FIND_ALARM;
				}
			}
		}
	}

	return AGING_NO_ALARM;
}
