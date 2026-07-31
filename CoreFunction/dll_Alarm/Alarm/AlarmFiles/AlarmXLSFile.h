#pragma once

#include "..\AlarmDefine.h"

class CAlarmXLSFile
{
    CMap<UINT, UINT, ALARM_DATA, ALARM_DATA &> m_alarmTable;

public:
	bool XLStoDAT(CString strFileName);
    BOOL LoadFile(CString strFileName);
	bool SaveFile(CString strFileName);
    BOOL Lookup(UINT id, ALARM_DATA &data);

private:
	bool LoadXLSFile(CString strFileName);


public:
    CAlarmXLSFile();
    ~CAlarmXLSFile();
};

