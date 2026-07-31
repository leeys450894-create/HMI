//#include "StdAfx.h"
#include "pch.h"
#include "SystemParamShare.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//----------------------------------------------------------------------------------------------------
// Singleton
//----------------------------------------------------------------------------------------------------
CSystemParamShare *CSystemParamShare::m_pInstance = nullptr;
CSystemParamShare *CSystemParamShare::GetInstance()
{
    return m_pInstance ? m_pInstance : (m_pInstance = new CSystemParamShare);
}
void CSystemParamShare::DelInstance()
{
    if(m_pInstance)
    {
        delete m_pInstance;
        m_pInstance = nullptr;
    }
}

CSystemParamShare::CSystemParamShare(void)
	: m_bForceStopByUser(false)
	, m_bCycleState(false)
	, m_bAutoMode(false)
	, m_bTestMode(false)
	, m_bResumeMode(false)
	, m_nResumeStartStick(0)
	, m_bInitialized(false)
{
	m_strInnerID	= _T("123456");
	m_strGlassID	= _T("ABCDEFG1234");
	m_strLotID		= _T("ABCDEFG1234");
	m_strRecipeID	= _T("UNKNOWN");
	m_strStepID		= _T("UNKNOWN");
	m_strSlotID		= _T("UNKNOWN");
	m_strCassetteID = _T("UNKNOWN");
	m_strOperID		= _T("UNKNOWN");
	m_strGlassRot   = _T("0");
	m_nCycleCount	= 1;
}

CSystemParamShare::~CSystemParamShare(void)
{
	
}

void CSystemParamShare::SetAutoMode(bool value)
{
	m_bAutoMode = value;
	m_bTestMode = false;

	m_bResumeMode = false;
	m_nResumeStartStick = 0;
}

bool CSystemParamShare::IsAutoMode() const
{
	return m_bAutoMode;
}




void CSystemParamShare::SetInitialized(bool value)
{
	m_bInitialized = value;
}

bool CSystemParamShare::IsInitialized() const
{
	return m_bInitialized;
}

void CSystemParamShare::SetForceStopByUser(bool value)	
{
	m_bForceStopByUser = value;	
}

bool CSystemParamShare::IsForceStopByUser() const
{
	return m_bForceStopByUser;	
}

void CSystemParamShare::SetCycleState(bool value)
{
	m_bCycleState = value;			
}

bool CSystemParamShare::IsCycleState() const				
{ 
	return m_bCycleState;			
}

void CSystemParamShare::SetTestMode(bool value)
{
	m_bTestMode = value;			
}

bool CSystemParamShare::IsTestMode() const
{
	return m_bTestMode;			
}


