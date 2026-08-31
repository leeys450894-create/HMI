#pragma once

#define PLC_M_BEGIN_BIT					4000
#define PLC_M_END_BIT					5000

namespace MADDR {
	enum PLC_BITMAP_ADDRESS_M {
		//PLC_M_READY_STATE = 4000,
		//PLC_M_LOAD_STATE,
		//PLC_M_MEASURE_STATE,

		//PLC_M_ALARM_STATE = 4008,
		//PLC_M_WARNING_STATE,

		PLC_M_TEST_BEGIN = 4000,
		PLC_M_TEST_END = 4999,
	};
}

struct S_MAddressCodeText
{
	TCHAR* m_szCode;
	TCHAR* m_szText;
};

#define MAddressSize	2
const S_MAddressCodeText g_MAddressTable[MAddressSize] = {
	{_T("4000"), _T("PLC_M_TEST_BEGIN")},
	{_T("4999"), _T("PLC_M_TEST_END")},
};