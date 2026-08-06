// AgingAlarmCheck.h
// LYS 2026-08-05 AgingSet Phase 4: non-UI aging target-current alarm checker.
// Port of CDlgSubManual_AgingSet file logic (LoadAgingInfo / FindAgingAlarm)
// for use from the sequence thread (CDoWorkGlassMeasureStep_AGING).
// Settings come from the app registry profile shared with the dialog:
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
