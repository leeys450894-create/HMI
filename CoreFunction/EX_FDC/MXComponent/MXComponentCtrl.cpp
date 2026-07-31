// MXComponentCtrl.cpp: implementation of the CMXComponentCtrl class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MXComponent.h"
#include "MXComponentCtrl.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMXComponentCtrl::CMXComponentCtrl()
{
	m_pDlgMXComponent = NULL;

	m_bMXComponentIni = FALSE;
}

CMXComponentCtrl::~CMXComponentCtrl()
{
	
}

long CMXComponentCtrl::Initialize(int iStationNum, HWND hParentWnd)
{
	long lRet = 0;

	if(m_pDlgMXComponent == NULL)
		m_pDlgMXComponent = new CDlgMXComponent;

	if(IsWindow(m_pDlgMXComponent->GetSafeHwnd()) == FALSE)
	{
		m_pDlgMXComponent->Create(IDD_DLG_MXCOMPONENT, CWnd::FromHandle(hParentWnd));

		if(IsWindow(m_pDlgMXComponent->GetSafeHwnd())!=FALSE)
		{
			m_pDlgMXComponent->CenterWindow();
			m_pDlgMXComponent->ShowWindow(SW_HIDE);
		}
		else
			return lRet;
	}

	lRet = m_pDlgMXComponent->Initialize(iStationNum);

	if(lRet == 0)
	{
		m_bMXComponentIni = TRUE;
		return MX_COMPONENT_SUCCESS;
	}
	else
	{
		return lRet;
	}
}

long CMXComponentCtrl::Close()
{
	long lRet = 0;

	if( m_bMXComponentIni == TRUE)
	{
		lRet = m_pDlgMXComponent->Close();
	}

	m_pDlgMXComponent->DestroyWindow();

	if(m_pDlgMXComponent)
	{
		delete m_pDlgMXComponent;
		m_pDlgMXComponent = NULL;
	}

	if(lRet == 0)
		return MX_COMPONENT_SUCCESS;
	else
		return lRet;
}

CString CMXComponentCtrl::GetErrorText(long lErrorCode)
{
	CString strErrorRet;
	strErrorRet.Empty();
	
	if(lErrorCode == MX_COMPONENT_SUCCESS)
	{
		strErrorRet.Format(_T("MXComponent Success."));
		return strErrorRet;
	}
	else if(lErrorCode == MX_COMPONENT_NOT_INITIALIZE)
	{
		strErrorRet.Format(_T("MXComponent Not Initialized."));
		return strErrorRet;
	}
	
	strErrorRet = m_pDlgMXComponent->GetErrorMessage(lErrorCode);
	
	return strErrorRet;
}

long CMXComponentCtrl::GetMemoryBit(CString strDeviceText, long* lplData)
{
	long lRet = 0;
	
	lRet = m_pDlgMXComponent->GetMemoryBit(strDeviceText, lplData);
	
	if(lRet == 0)
		return MX_COMPONENT_SUCCESS;
	else
		return lRet;
}

long CMXComponentCtrl::SetMemoryBit(CString strDeviceText, BOOL bSet)
{
	long lRet = 0;
	
	lRet = m_pDlgMXComponent->SetMemoryBit(strDeviceText, bSet);
	
	if(lRet == 0)
		return MX_COMPONENT_SUCCESS;
	else
		return lRet;
}

long CMXComponentCtrl::SetMemoryBitBlock(CString strDeviceText, long lSize, BOOL bSet)
{
	long lRet = 0;	
	
	lRet = m_pDlgMXComponent->SetMemoryBitBlock(strDeviceText, lSize, bSet);
	
	if(lRet == 0)
		return MX_COMPONENT_SUCCESS;
	else
		return lRet;
}	

long CMXComponentCtrl::SetMemoryBitBlock2(CString strDeviceText, long lSize, long* lplBitData)
{
	long lRet = 0;	

	lRet = m_pDlgMXComponent->SetMemoryBitBlock2(strDeviceText, lSize, lplBitData);

	if(lRet == 0)
		return MX_COMPONENT_SUCCESS;
	else
		return lRet;
}	


long CMXComponentCtrl::GetMemoryBitBlock(CString strDeviceText, long lSize, long* lplBitData)
{
	long lRet = 0;
	
	lRet = m_pDlgMXComponent->GetMemoryBitBlock(strDeviceText, lSize, lplBitData);
	
	if(lRet == 0)
		return MX_COMPONENT_SUCCESS;
	else
		return lRet;
}

long CMXComponentCtrl::GetMemoryWordBlock(CString strDeviceText, long lSize, float* fpData, UINT nSize)
{
	long lRet = 0;
	
	lRet = m_pDlgMXComponent->GetMemoryWordBlock(strDeviceText, lSize, fpData, nSize);
	
	if(lRet == 0)
		return MX_COMPONENT_SUCCESS;
	else
		return lRet;
}

long CMXComponentCtrl::SetMemoryWordBlock(CString strDeviceText, long lSize, float* fpData, UINT nSize)
{
	long lRet = 0;
	
	lRet = m_pDlgMXComponent->SetMemoryWordBlock(strDeviceText, lSize, fpData, nSize);
	
	if(lRet == 0)
		return MX_COMPONENT_SUCCESS;
	else
		return lRet;
}

long CMXComponentCtrl::GetMemoryWordBlock2(CString strDeviceText, long lSize, TCHAR* tpData)
{
	long lRet = 0;

	lRet = m_pDlgMXComponent->GetMemoryWordBlock2(strDeviceText, lSize, tpData);

	if(lRet == 0)
		return MX_COMPONENT_SUCCESS;
	else
		return lRet;
}

long CMXComponentCtrl::SetMemoryWordBlock2(CString strDeviceText, long lSize, TCHAR* tpData)
{
	long lRet = 0;

	lRet = m_pDlgMXComponent->SetMemoryWordBlock2(strDeviceText, lSize, tpData);

	if(lRet == 0)
		return MX_COMPONENT_SUCCESS;
	else
		return lRet;
}

long CMXComponentCtrl::GetMemoryWordBlock3(CString strDeviceText, long lSize, long* lpData, UINT nSize)
{
	long lRet = 0;

	lRet = m_pDlgMXComponent->GetMemoryWordBlock3(strDeviceText, lSize, lpData, nSize);

	if(lRet == 0)
		return MX_COMPONENT_SUCCESS;
	else
		return lRet;
}

long CMXComponentCtrl::SetMemoryWordBlock3(CString strDeviceText, long lSize, long* lpData, UINT nSize)
{
	long lRet = 0;

	lRet = m_pDlgMXComponent->SetMemoryWordBlock3(strDeviceText, lSize, lpData, nSize);

	if(lRet == 0)
		return MX_COMPONENT_SUCCESS;
	else
		return lRet;
}
