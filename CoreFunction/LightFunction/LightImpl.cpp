#include "pch.h"
#include "LightImpl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CLightImpl::CLightImpl()
{
}

CLightImpl::~CLightImpl()
{
}



//------------------------------------------------------------------
// 공통 인터페이스 함수
//------------------------------------------------------------------
bool CLightImpl::SetLedLampIntensity(long nChannel, long nIntensity, CString &strCommand)	
{
	return false;		
}

bool CLightImpl::SetLedLampTurnOnOff(long nChannel, long nIntensity, enumLED_LAMP_CONTROL nCONTROL, CString &strCommand)
{
	return false;		
}

bool CLightImpl::GetLedLampIntensity(long nChannel, CString &strCommand)	
{ 
	return false;		
}

bool CLightImpl::GetLedLampTurnOnOff(long nChannel, CString &strCommand)	
{ 
	return false;		
}

bool CLightImpl::WaitLedLampResponce(long nChannel, long nIntensity, enumRES_CHECK_TYPE nTYPE)
{ 
	return false;		
}

bool CLightImpl::GetCurLedLampIntensity(long nChannel, long &nIntensity, CString &strCommand)
{
	return false;
}
