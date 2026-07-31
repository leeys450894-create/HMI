#pragma once

#include "..\AlarmDefine.h"

class CAlarmFileTransfer
{
    CMap<UINT, UINT, ALARM_DATA, ALARM_DATA &> m_alarmTable;

public:
	bool LoadFile(CString strFileName);
	bool SaveFile(CString strFileName);

public:
    CAlarmFileTransfer();
    ~CAlarmFileTransfer();
};

