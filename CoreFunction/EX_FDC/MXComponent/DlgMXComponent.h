//{{AFX_INCLUDES()
#include "actutltype.h"
#include "actsupportmsg.h"
//}}AFX_INCLUDES
#if !defined(AFX_DLGMXCOMPONENT_H__C42A0C38_4220_43B6_94D7_14A41DDB8D64__INCLUDED_)
#define AFX_DLGMXCOMPONENT_H__C42A0C38_4220_43B6_94D7_14A41DDB8D64__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgMXComponent.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDlgMXComponent dialog

#define SQUARE_BASE_NUM		2
#define BINARY_NUM			2
#define BIT_DEVICE_SIZE		16
#define HEX_DEVICE_INTERVAL	10
#define MAX_WORD_VALUE		32767
#define MIN_WORD_VALUE		-32768
#define MAX_WRITE_VALUE		65535		//2^16 - 1


struct BITFIELD
{          
	unsigned char bit_31:1; unsigned char bit_30:1; unsigned char bit_29:1; unsigned char bit_28:1;
	
	unsigned char bit_27:1; unsigned char bit_26:1; unsigned char bit_25:1; unsigned char bit_24:1;
	
	unsigned char bit_23:1; unsigned char bit_22:1; unsigned char bit_21:1; unsigned char bit_20:1;
	
	unsigned char bit_19:1; unsigned char bit_18:1; unsigned char bit_17:1; unsigned char bit_16:1;
	
	unsigned char bit_15:1; unsigned char bit_14:1; unsigned char bit_13:1; unsigned char bit_12:1;
	
	unsigned char bit_11:1; unsigned char bit_10:1; unsigned char bit_9:1; unsigned char bit_8:1;
	
	unsigned char bit_7:1; unsigned char bit_6:1; unsigned char bit_5:1; unsigned char bit_4:1;
	
	unsigned char bit_3:1; unsigned char bit_2:1; unsigned char bit_1:1; unsigned char bit_0:1;	
};



struct BYTEFIELD
{          	
	unsigned int m_Bottom1Byte:16;			//실수부
	
	unsigned int m_Top1Byte:16;				//정수부
};



union FLOAT_FIELD
{
	TCHAR m_char[2];

	float m_float;
	
	long lfloat;
	
	struct BITFIELD m_bit;
	
	struct BYTEFIELD m_16bit;
};


class CDlgMXComponent : public CDialog
{
// Construction
public:
	CDlgMXComponent(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgMXComponent();

// Dialog Data
	//{{AFX_DATA(CDlgMXComponent)
	enum { IDD = IDD_DLG_MXCOMPONENT };
	CActUtlType	m_ctrlActUtlType;
	CActSupportMsg	m_ctrlActSupportMsg;
	//}}AFX_DATA
	
	//about error message
	BSTR		m_bstrErrorMessage;
	CString		m_strErrorMessage;
	//

	long		Initialize(int iStationNum);
	long		Close();
	CString		GetErrorMessage(long lErrorCode);
	long		GetMemoryBit(CString strDeviceText, long* lplData);
	long		SetMemoryBit(CString strDeviceText, long lData);
	long		SetMemoryBitBlock(CString strDeviceText, long lSize, BOOL bSet);
	long		SetMemoryBitBlock2(CString strDeviceText, long lSize, long* lplData);
	long		HEXtoDEC(CString strHexNum);
	long		GetSquareValueForBitMem(long lBaseNum, long lSquareNum, BOOL bSet);
	long		GetSquareValueForBitMem2(long lBaseNum, long lSquareNum, BOOL bSet);
	long		GetMemoryBitBlock(CString strDeviceText, long lSize, long* lplBitData);
	long		GetMemoryWordBlock(CString strDeviceText, long lSize, float* fpData, UINT nSize = 2);
	long		SetMemoryWordBlock(CString strDeviceText, long lSize, float* fpData, UINT nSize = 2);
	long		GetMemoryWordBlock2(CString strDeviceText, long lSize, TCHAR* tpData);
	long		SetMemoryWordBlock2(CString strDeviceText, long lSize, TCHAR* tpData);
	long		GetMemoryWordBlock3(CString strDeviceText, long lSize, long* lpData, UINT nSize = 2);
	long		SetMemoryWordBlock3(CString strDeviceText, long lSize, long* lpData, UINT nSize = 2);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgMXComponent)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgMXComponent)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGMXCOMPONENT_H__C42A0C38_4220_43B6_94D7_14A41DDB8D64__INCLUDED_)
