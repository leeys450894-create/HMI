// MXComponentCtrl.h: interface for the CMXComponentCtrl class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MXCOMPONENTCTRL_H__BDC8A993_53A2_4618_85B0_38199FF4101D__INCLUDED_)
#define AFX_MXCOMPONENTCTRL_H__BDC8A993_53A2_4618_85B0_38199FF4101D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "DlgMXComponent.h"

#define MX_COMPONENT_SUCCESS			 1
#define MX_COMPONENT_NOT_INITIALIZE		-1

class CMXComponentCtrl  
{
public:
	CMXComponentCtrl();
	virtual ~CMXComponentCtrl();

	CDlgMXComponent* m_pDlgMXComponent;
	BOOL		m_bMXComponentIni;

	long		Initialize(int iStationNum, HWND hParentWnd);
	long		Close();
	BOOL		Is_Connact() {return m_bMXComponentIni; };
	CString		GetErrorText(long lErrorCode);
	long		GetMemoryBit(CString strDeviceText, long* lplData);
	long		SetMemoryBit(CString strDeviceText, BOOL bSet);
	long		SetMemoryBitBlock(CString strDeviceText, long lSize, BOOL bSet);
	long		SetMemoryBitBlock2(CString strDeviceText, long lSize, long* lplBitData);
	long		GetMemoryBitBlock(CString strDeviceText, long lSize, long* lplBitData);
	long		GetMemoryWordBlock(CString strDeviceText, long lSize, float* fpData, UINT nSize = 2);
	long		SetMemoryWordBlock(CString strDeviceText, long lSize, float* fpData, UINT nSize = 2);
	long		GetMemoryWordBlock2(CString strDeviceText, long lSize, TCHAR* tpData);
	long		SetMemoryWordBlock2(CString strDeviceText, long lSize, TCHAR* tpData);
	long		GetMemoryWordBlock3(CString strDeviceText, long lSize, long* lpData, UINT nSize = 2);
	long		SetMemoryWordBlock3(CString strDeviceText, long lSize, long* lpData, UINT nSize = 2);
};

#endif // !defined(AFX_MXCOMPONENTCTRL_H__BDC8A993_53A2_4618_85B0_38199FF4101D__INCLUDED_)
