#pragma once

#include "..\AlarmDefine.h"

class CAlarmCSVFile
{
    CMap<UINT, UINT, ALARM_MESSAGE, ALARM_MESSAGE &> m_alarmMessage;

public:
	bool CSVtoDAT(CString strFileName);
    BOOL LoadFile(CString strFileName);
    bool SaveFile(CString strFileName);
    BOOL Lookup(UINT id, ALARM_MESSAGE &data);

private:
	bool LoadCSVFile(CString strFileName);

public:
    CAlarmCSVFile();
    ~CAlarmCSVFile();
};