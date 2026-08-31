#pragma once

#define PLC_F_BEGIN_BIT					0
#define PLC_F_END_BIT					2000

struct S_FAddressCodeText
{
	TCHAR* m_szCode;
	TCHAR* m_szText;
};

#define FAddressSize	5
const S_FAddressCodeText g_FAddressTable[FAddressSize] = {
	{_T("1"), _T("[F0001]PLC_F_ALARM_1")},
	{_T("2"), _T("[F0002]PLC_F_ALARM_2")},
	{_T("3"), _T("[F0003]PLC_F_ALARM_3")},
	{_T("4"), _T("[F0004]PLC_F_ALARM_4")},
	{_T("5"), _T("[F0005]PLC_F_ALARM_5")},
};
