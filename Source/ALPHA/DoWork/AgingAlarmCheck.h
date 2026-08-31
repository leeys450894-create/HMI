// AgingAlarmCheck.h
// LYS 2026-08-05 AgingSet Phase 4: 비UI 에이징 목표전류 알람 체커.
// 시퀀스 스레드(CDoWorkGlassMeasureStep_AGING)에서 쓰려고 다이얼로그 로직을 이식.
// 설정은 다이얼로그와 공유하는 앱 레지스트리 프로파일에서 읽는다:
//   DIR\AGING_TARGET_RCP, DIR\AGING_FILE, CONFIG\AGING_ALARM_USE
#pragma once

#include <vector>
#include "common\CommonFunc.h"		// S_AgingTargetAlarm

#ifndef AGING_NO_ALARM
#define AGING_NO_ALARM	 0
#define AGING_NOT_FILE	 1
#define AGING_FIND_ALARM 2
#endif

class CAgingAlarmCheck
{
public:
	static CString	GetTargetRcpPath();
	static CString	GetAgingFilePath();
	static BOOL		GetAlarmUse();
	static void		DeleteAgingResultFile();

	// Reads {AGING_TARGET_RCP}\{strRcpName}.rcp into vecOut. FALSE if missing/empty.
	static BOOL		LoadTargetRcp(CString strRcpName, std::vector<S_AgingTargetAlarm>& vecOut);

	// Parses the AGING result file against vecOut.
	// Returns AGING_NO_ALARM / AGING_NOT_FILE / AGING_FIND_ALARM.
	// On AGING_FIND_ALARM strPtnOut/nCellOut hold the offending pattern and cell.
	static int		Find(const std::vector<S_AgingTargetAlarm>& vecTarget, CString& strPtnOut, int& nCellOut);

private:
	static BOOL		FileFind(CString strFile);
	static BOOL		FileFindTimeOut(CString strFile, int nTimeOutMs);	// TRUE = timed out
};
