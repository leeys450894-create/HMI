#pragma once

#include <windows.h>

class CTimeChecker
{
	// 시작, 끝, 주기 진동수를 얻을 구조체를 선언한다.
	LARGE_INTEGER m_freq;
    LARGE_INTEGER m_st;
    LARGE_INTEGER m_ed;	

public:
    void Reset();
	CString GetSTtimeString();
	CString GetEDtimeString();
    float Elapsed_ms();
    float Elapsed();

public:
    CTimeChecker(void);
    ~CTimeChecker(void);
};
