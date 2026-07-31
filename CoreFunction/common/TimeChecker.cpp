#include "pch.h"
#include "TimeChecker.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CTimeChecker::CTimeChecker(void)
{
    // 고해상도 타이머의 주기를 얻는다.
    QueryPerformanceFrequency(&m_freq);

    // 시작 시점의 진동수를 얻는다.
    QueryPerformanceCounter(&m_st);
}

CTimeChecker::~CTimeChecker(void)
{
}

void CTimeChecker::Reset()
{
    // 고해상도 타이머의 주기를 얻는다.
    QueryPerformanceFrequency(&m_freq);

    // 시작 시점의 진동수를 얻는다.
    QueryPerformanceCounter(&m_st);
}

CString CTimeChecker::GetSTtimeString()
{
	// LARGE_INTEGER -> FILETIME 변환
	FILETIME ftm;
    ftm.dwHighDateTime = m_st.HighPart;
    ftm.dwLowDateTime = m_st.LowPart;

	SYSTEMTIME stm;
//	FileTimeToSystemTime(&ftm, &stm);		// FILETIME -> SYSTEMTIME 변환
	::GetLocalTime(&stm);
	
	CString strTime;
	strTime.Format(_T("%02d:%02d:%02d"), stm.wHour, stm.wMinute, stm.wSecond);
	return strTime;
}

CString CTimeChecker::GetEDtimeString()
{
	// LARGE_INTEGER -> FILETIME 변환
	FILETIME ftm;
    ftm.dwHighDateTime = m_ed.HighPart;
    ftm.dwLowDateTime = m_ed.LowPart;

	SYSTEMTIME stm;
//	FileTimeToSystemTime(&ftm, &stm); // FILETIME -> SYSTEMTIME 변환
	::GetLocalTime(&stm);

	CString strTime;
	strTime.Format(_T("%02d:%02d:%02d"), stm.wHour, stm.wMinute, stm.wSecond);
	return strTime;
}

float CTimeChecker::Elapsed_ms()
{
    // 끝 시점의 진동수를 얻는다.
    QueryPerformanceCounter(&m_ed);

    // 끝 시점의 진동수에서 시작 시점의 진동수를 뺀 후 진동수에서 1000으로 나눈 값을 나눈다.
    // 1초 기준의 주기를 1000 으로 나누었기 때문에 1밀리초 동안 발생하는 진동수로 나눈 셈이다.
    //__int64 ms_interval = (m_ed.QuadPart - m_st.QuadPart) / (m_freq.QuadPart / 1000);

    // 끝 시점의 진동수에서 시작 시점의 진동수를 뺀 후 진동수에서 1000000으로 나눈 값을 나눈다.
    // 1초 기준의 주기를 1000000 으로 나누었기 때문에 1마이크로초 동안 발생하는 진동수로 나눈 셈이다.
    //__int64 micro_interval = (end.QuadPart - start.QuadPart)/(f.QuadPart/1000000);

    // 결과 값을 밀리초와 마이크로초로 출력한다.
    //printf("millisecond : %d, microsecond : %d\n", (int)ms_interval, (int)micro_interval);

    // 소요 시간 계산
    double gap = ((double)(m_ed.QuadPart) - (double)(m_st.QuadPart)) / (double)(m_freq.QuadPart);

    return (float)(gap * 1000.0); // msec

}

float CTimeChecker::Elapsed()
{
    // 끝 시점의 진동수를 얻는다.
    QueryPerformanceCounter(&m_ed);

    // 끝 시점의 진동수에서 시작 시점의 진동수를 뺀 후 진동수에서 1000으로 나눈 값을 나눈다.
    // 1초 기준의 주기를 1000 으로 나누었기 때문에 1밀리초 동안 발생하는 진동수로 나눈 셈이다.
    //__int64 ms_interval = (m_ed.QuadPart - m_st.QuadPart) / (m_freq.QuadPart / 1000);

    // 끝 시점의 진동수에서 시작 시점의 진동수를 뺀 후 진동수에서 1000000으로 나눈 값을 나눈다.
    // 1초 기준의 주기를 1000000 으로 나누었기 때문에 1마이크로초 동안 발생하는 진동수로 나눈 셈이다.
    //__int64 micro_interval = (end.QuadPart - start.QuadPart)/(f.QuadPart/1000000);

    // 결과 값을 밀리초와 마이크로초로 출력한다.
    //printf("millisecond : %d, microsecond : %d\n", (int)ms_interval, (int)micro_interval);

    // 소요 시간 계산
    double gap = ((double)(m_ed.QuadPart) - (double)(m_st.QuadPart)) / (double)(m_freq.QuadPart);

    return (float)(gap); // sec

}
