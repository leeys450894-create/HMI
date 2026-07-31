// DlgMXComponent.cpp : implementation file
//

#include "stdafx.h"
#include "MXComponent.h"
#include "DlgMXComponent.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgMXComponent dialog


CDlgMXComponent::CDlgMXComponent(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgMXComponent::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgMXComponent)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	m_bstrErrorMessage = 0;
}

CDlgMXComponent::~CDlgMXComponent()
{

}

void CDlgMXComponent::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgMXComponent)
	DDX_Control(pDX, IDC_ACTUTLTYPE1, m_ctrlActUtlType);
	DDX_Control(pDX, IDC_ACTSUPPORTMSG1, m_ctrlActSupportMsg);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgMXComponent, CDialog)
	//{{AFX_MSG_MAP(CDlgMXComponent)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgMXComponent message handlers


long CDlgMXComponent::Initialize(int iStationNum)
{
	LONG lRet = 0;
	
	m_ctrlActUtlType.SetActLogicalStationNumber(iStationNum);

	lRet = m_ctrlActUtlType.Open();				//0이면 성공
	
	return lRet;
}

long CDlgMXComponent::Close()
{
	LONG lRet = 0;
	
	lRet = m_ctrlActUtlType.Close();
	
	return lRet;
}

CString CDlgMXComponent::GetErrorMessage(long lErrorCode)
{
	long lRet = 0;
	m_bstrErrorMessage = 0;
	
	lRet = m_ctrlActSupportMsg.GetErrorMessage(lErrorCode, &m_bstrErrorMessage);
	
	if(lRet == 0)
		m_strErrorMessage = m_bstrErrorMessage;
	else
		m_strErrorMessage.Format("GetErrorMessage Failed. The Message was not able to be acquired. \r\n<ErrorCode:%08x[Hex]>", lRet);
	
	return m_strErrorMessage;
}

long CDlgMXComponent::GetMemoryBit(CString strDeviceText, long* lplData)	
{
	long lRet = 0;
	CString strDeviceNum;
	strDeviceNum.Empty();

	strDeviceNum.Format(_T("%s"), strDeviceText);

	lRet = m_ctrlActUtlType.GetDevice(strDeviceNum, lplData);

	return lRet;
}

long CDlgMXComponent::SetMemoryBit(CString strDeviceText, long lData)
{
	long lRet = 0;
	CString strDeviceNum;
	strDeviceNum.Empty();
	
	strDeviceNum.Format(_T("%s"), strDeviceText);

	lRet = m_ctrlActUtlType.SetDevice(strDeviceNum, lData);

	return lRet;
}

long CDlgMXComponent::SetMemoryBitBlock(CString strDeviceText, long lSize, BOOL bSet)
{
	long lRet = 0;
	CString strDeviceWord;
	CString strDeviceNum;
	long lMemoryInterval;
	long lWriteWordSize;
	long lRemainderSize;
	long lOriginalValue;
	long lDeviceNum;
	long lstrNumSize;

	long lBitDeviceSize;
	long lMaxWriteValue;
	long lDeviceInterval;

	strDeviceWord.Empty();
	strDeviceNum.Empty();
	lMemoryInterval = 0;
	lWriteWordSize = 0;
	lRemainderSize = 0;
	lOriginalValue = 0;
	lDeviceNum = 0;
	lstrNumSize = 0;
	
	lDeviceInterval = BIT_DEVICE_SIZE;
	lBitDeviceSize = BIT_DEVICE_SIZE;
	lMaxWriteValue = MAX_WRITE_VALUE;	 

	lstrNumSize = strDeviceText.GetLength() - 1;

	strDeviceWord = strDeviceText.Mid(0, 1);
	strDeviceNum = strDeviceText.Mid(1, lstrNumSize);

	if(strDeviceWord == "X" || strDeviceWord == "Y" || strDeviceWord == "B" || strDeviceWord == "SB")	//Device Number가 16진수
	{
		lDeviceNum = HEXtoDEC(strDeviceNum);	//16진수를 10진수로 변환하여 계산해야함..

		//Bit 단위 번지를 WriteDeviceBlock으로 접근하려면 16의 배수 메모리 번지로 접근하여 처리해야함
		lMemoryInterval = lDeviceNum % lBitDeviceSize;
		lDeviceNum = lDeviceNum - lMemoryInterval;		//Bit Device 시작지점
		strDeviceNum.Format(_T("%s%x"), strDeviceWord, lDeviceNum);		//실제로 접근할땐 다시 16진수로 변환...
	}
	else
	{
		lDeviceNum = atoi(strDeviceNum);

		//Bit 단위 번지를 WriteDeviceBlock으로 접근하려면 16의 배수 메모리 번지로 접근하여 처리해야함
		lMemoryInterval = lDeviceNum % lBitDeviceSize;
		lDeviceNum = lDeviceNum - lMemoryInterval;		//Bit Device 시작지점
		strDeviceNum.Format(_T("%s%d"), strDeviceWord, lDeviceNum);
	}

	//현재 참조할 시작 메모리 주소(16배수) 값 읽어옴
	lOriginalValue = 0;
	lRet = m_ctrlActUtlType.ReadDeviceBlock(strDeviceNum, 1, &lOriginalValue);		

	//한 메모리에 16개까지 읽고 쓰기 가능
	//그 이상 넘어가면 다음 메모리 번지 부터 쓰도록

	//Bit 사이즈를 16으로 나눠서 Word단위 메모리 번지(16의 배수) 몇개에 쓸지 결정
	lRemainderSize	= lSize % lBitDeviceSize;

	long lLastDeviceBitSize = lMemoryInterval+lRemainderSize;

	if(lSize <= lBitDeviceSize)	 
	{
		lWriteWordSize = 1;

		//lSize 사이즈가 16보다 작을 때, 다음 Word 번지로 넘어간다면 lWriteWordSize를 하나 올린다.
		//다음 번지에 써야하기 때문
		long lJudgeSize = lMemoryInterval + lSize;
		if(lJudgeSize > lBitDeviceSize)		
			++lWriteWordSize;
	}
	else
	{
		lWriteWordSize = lSize / lBitDeviceSize;

		if(lLastDeviceBitSize != 0)		
			++lWriteWordSize;

		if(lLastDeviceBitSize >= lBitDeviceSize)
			++lWriteWordSize;
	}	

	long* lplWriteData = new long[lWriteWordSize];
	long lSetFalsePosValue = 0;

	long lLastSquareNum = lLastDeviceBitSize % lBitDeviceSize;

	for(int i = 0; i < lWriteWordSize; i++)
	{
		if(bSet == TRUE)
		{
			if(lWriteWordSize > 1)
			{
				if(i == 0)
				{
					if(lMemoryInterval == 0)
						lplWriteData[i] = lMaxWriteValue;
					else
						lplWriteData[i] = (lMaxWriteValue - GetSquareValueForBitMem(SQUARE_BASE_NUM, lMemoryInterval, TRUE)) | lOriginalValue;
				}
				else if(i == lWriteWordSize - 1)
				{
					if(lLastDeviceBitSize == 0)		//lSize가 16의 배수이고, 메모리 시작지점이 16의 배수 번지이면 그냥 다 1로 쓰면 된다.
						lplWriteData[i] = lMaxWriteValue;
					else	
					{
						CString strLastWordDevice;
						long	lLastOriginalValue = 0;

						if(strDeviceWord == "X" || strDeviceWord == "Y" || strDeviceWord == "B" || strDeviceWord == "SB")
							strLastWordDevice.Format("%s%x", strDeviceWord, lDeviceNum + i * lDeviceInterval);					// 마지막 메모리 주소 값 읽어야 함..
						else
							strLastWordDevice.Format("%s%d", strDeviceWord, lDeviceNum + i * lDeviceInterval);					// 마지막 메모리 주소 값 읽어야 함..
						
						m_ctrlActUtlType.ReadDeviceBlock(strLastWordDevice, 1, &lLastOriginalValue);
						lplWriteData[i] = GetSquareValueForBitMem(SQUARE_BASE_NUM, lLastSquareNum, TRUE) | lLastOriginalValue;		//lLastDeviceBitSize가 16이 넘어가는 경우가 있을 수 있으므로...
					}
				}
				else	
					lplWriteData[i] = lMaxWriteValue;
			}
			else
				if(lMemoryInterval == 0)
					lplWriteData[i] = GetSquareValueForBitMem(SQUARE_BASE_NUM, lMemoryInterval + lSize, TRUE) | lOriginalValue;
				else
					lplWriteData[i] = (GetSquareValueForBitMem(SQUARE_BASE_NUM, lMemoryInterval + lSize, TRUE) - GetSquareValueForBitMem(SQUARE_BASE_NUM, lMemoryInterval, TRUE)) | lOriginalValue;	
		}
		else
		{
			if(lWriteWordSize > 1)
			{
				if(i == 0)
				{
					for(int j = lMemoryInterval; j < lBitDeviceSize; j++)							//FALSE로 Set하기 시작하는 위치부터 끝지점까지 1로 Set할 수 있는 값을 구한다.
						lSetFalsePosValue += GetSquareValueForBitMem(SQUARE_BASE_NUM, j, FALSE);

					lplWriteData[i] = (lMaxWriteValue - lSetFalsePosValue) & lOriginalValue;				//11111111...11 - 00111000...00 = 11000111...11
				}																						//구해진 값을 원래 메모리의 값과 AND 연산 실행
				else if(i == lWriteWordSize - 1)
				{
					lSetFalsePosValue = 0;			//초기화 해야함... 안그러면 이전에 계산했던 값이 남아있어 이상한 값 들어감...ㅂㄷㅂㄷ
					CString strLastWordDevice;
					long	lLastOriginalValue = 0;
					
					if(strDeviceWord == "X" || strDeviceWord == "Y" || strDeviceWord == "B" || strDeviceWord == "SB")
						strLastWordDevice.Format("%s%x", strDeviceWord, lDeviceNum + i * lDeviceInterval);					// 마지막 메모리 주소 값 읽어야 함..
					else
						strLastWordDevice.Format("%s%d", strDeviceWord, lDeviceNum + i * lDeviceInterval);					// 마지막 메모리 주소 값 읽어야 함..
			
					m_ctrlActUtlType.ReadDeviceBlock(strLastWordDevice, 1, &lLastOriginalValue);	

					for(int j = 0; j < lLastSquareNum; j++)
						lSetFalsePosValue += GetSquareValueForBitMem(SQUARE_BASE_NUM, j, FALSE);

					if(lLastDeviceBitSize == 0)
						lSetFalsePosValue = lMaxWriteValue;
					else if(lLastDeviceBitSize == lBitDeviceSize)
						lSetFalsePosValue = GetSquareValueForBitMem(SQUARE_BASE_NUM, 0, FALSE);

					lplWriteData[i] = (lMaxWriteValue - lSetFalsePosValue) & lLastOriginalValue;	
				}		
				else
				{
					lplWriteData[i] = 0;
				}
			}
			else
			{
				for(int j = lMemoryInterval; j < lLastDeviceBitSize % lBitDeviceSize; j++)			//FALSE로 Set하기 시작하는 위치부터 입력받은 사이즈만큼 1로 Set할 수 있는 값을 구한다.
					lSetFalsePosValue += GetSquareValueForBitMem(SQUARE_BASE_NUM, j, FALSE);

				if(lLastDeviceBitSize == 0)
						lSetFalsePosValue = lMaxWriteValue;
				
				lplWriteData[i] = lMaxWriteValue - lSetFalsePosValue & lOriginalValue;				//ex)11111111...11 - 00111000...00 = 11000111...11
			}																						//구해진 값을 원래 메모리의 값과 AND 연산 실행
		}		
	}
	
	//이걸 한번만 호출하기 위해 위의 과정 수행.. 
	lRet = m_ctrlActUtlType.WriteDeviceBlock(strDeviceNum, lWriteWordSize, lplWriteData);

	delete [] lplWriteData;

	return lRet;
}

long CDlgMXComponent::SetMemoryBitBlock2(CString strDeviceText, long lSize, long* lplData)
{
	long lRet = 0;
	CString strDeviceWord;
	CString strDeviceNum;
	long lMemoryInterval;
	long lWriteWordSize;
	long lRemainderSize;
	long lOriginalValue;
	long lDeviceNum;
	long lstrNumSize;

	long lBitDeviceSize;
	long lMaxWriteValue;
	long lDeviceInterval;

	strDeviceWord.Empty();
	strDeviceNum.Empty();
	lMemoryInterval = 0;
	lWriteWordSize = 0;
	lRemainderSize = 0;
	lOriginalValue = 0;
	lDeviceNum = 0;
	lstrNumSize = 0;

	lDeviceInterval = BIT_DEVICE_SIZE;
	lBitDeviceSize = BIT_DEVICE_SIZE;
	lMaxWriteValue = MAX_WRITE_VALUE;	 

	lstrNumSize = strDeviceText.GetLength() - 1;

	strDeviceWord = strDeviceText.Mid(0, 1);
	strDeviceNum = strDeviceText.Mid(1, lstrNumSize);

	if(strDeviceWord == "X" || strDeviceWord == "Y" || strDeviceWord == "B" || strDeviceWord == "SB")	//Device Number가 16진수
	{
		lDeviceNum = HEXtoDEC(strDeviceNum);	//16진수를 10진수로 변환하여 계산해야함..

		//Bit 단위 번지를 WriteDeviceBlock으로 접근하려면 16의 배수 메모리 번지로 접근하여 처리해야함
		lMemoryInterval = lDeviceNum % lBitDeviceSize;
		lDeviceNum = lDeviceNum - lMemoryInterval;		//Bit Device 시작지점
		strDeviceNum.Format(_T("%s%x"), strDeviceWord, lDeviceNum);		//실제로 접근할땐 다시 16진수로 변환...
	}
	else
	{
		lDeviceNum = atoi(strDeviceNum);

		//Bit 단위 번지를 WriteDeviceBlock으로 접근하려면 16의 배수 메모리 번지로 접근하여 처리해야함
		lMemoryInterval = lDeviceNum % lBitDeviceSize;
		lDeviceNum = lDeviceNum - lMemoryInterval;		//Bit Device 시작지점
		strDeviceNum.Format(_T("%s%d"), strDeviceWord, lDeviceNum);
	}

	//현재 참조할 시작 메모리 주소(16배수) 값 읽어옴
	lOriginalValue = 0;
	lRet = m_ctrlActUtlType.ReadDeviceBlock(strDeviceNum, 1, &lOriginalValue);		

	//한 메모리에 16개까지 읽고 쓰기 가능
	//그 이상 넘어가면 다음 메모리 번지 부터 쓰도록

	//Bit 사이즈를 16으로 나눠서 Word단위 메모리 번지(16의 배수) 몇개에 쓸지 결정
	lRemainderSize	= lSize % lBitDeviceSize;

	long lLastDeviceBitSize = lMemoryInterval+lRemainderSize;

	if(lSize <= lBitDeviceSize)	 
	{
		lWriteWordSize = 1;

		//lSize 사이즈가 16보다 작을 때, 다음 Word 번지로 넘어간다면 lWriteWordSize를 하나 올린다.
		//다음 번지에 써야하기 때문
		long lJudgeSize = lMemoryInterval + lSize;
		if(lJudgeSize > lBitDeviceSize)		
			++lWriteWordSize;
	}
	else
	{
		lWriteWordSize = lSize / lBitDeviceSize;

		if(lLastDeviceBitSize != 0)		
			++lWriteWordSize;

		if(lLastDeviceBitSize >= lBitDeviceSize)
			++lWriteWordSize;
	}	

	long* lplWriteData = new long[lWriteWordSize];
	int i = 0;
	for(i = 0; i < lWriteWordSize; i++)
	{
		lplWriteData[i] = 0;
	}

	long lSetFalsePosValue = 0;

	long lLastSquareNum = lLastDeviceBitSize % lBitDeviceSize;


	int k = 0;
	for(i = 0; i < lWriteWordSize; i++)
	{
		if(lWriteWordSize > 1)
		{
			if(i == 0)
			{
				for(int j = lMemoryInterval; j < lBitDeviceSize; j++)	
				{
					lplWriteData[i] += (GetSquareValueForBitMem2(SQUARE_BASE_NUM, j, (BOOL)lplData[k]/*TRUE*/));	
					k++;
				}

				if(lMemoryInterval != 0)
				{
					lplWriteData[i] += ((GetSquareValueForBitMem(SQUARE_BASE_NUM, lMemoryInterval, TRUE)) & lOriginalValue);
				}
			}
			else if(i == lWriteWordSize - 1)
			{
//				for(int j = 0; j < lLastDeviceBitSize; j++)	
				for(int j = 0; j < lLastSquareNum; j++)	
				{
					lplWriteData[i] += (GetSquareValueForBitMem2(SQUARE_BASE_NUM, j, (BOOL)lplData[k]/*TRUE*/));	
					k++;
				}
			}
			else
			{
				for(int j = 0; j < lBitDeviceSize; j++)	
				{
					lplWriteData[i] += (GetSquareValueForBitMem2(SQUARE_BASE_NUM, j, (BOOL)lplData[k]/*TRUE*/));	
					k++;
				}
			}
		}
		else
		{
//			for(int j = lMemoryInterval; j < lLastDeviceBitSize % lBitDeviceSize; j++)	
			int j = 0;
			for(j = lLastDeviceBitSize; j < lBitDeviceSize; j++)	
			{
				lplWriteData[i] += ((GetSquareValueForBitMem2(SQUARE_BASE_NUM, j, TRUE)) & lOriginalValue);	
			}

			for(j = lMemoryInterval; j < lLastDeviceBitSize; j++)	
			{
				lplWriteData[i] += (GetSquareValueForBitMem2(SQUARE_BASE_NUM, j, (BOOL)lplData[k]/*TRUE*/));	
				k++;
			}

			if(lMemoryInterval != 0)
			{
				lplWriteData[i] += ((GetSquareValueForBitMem(SQUARE_BASE_NUM, lMemoryInterval, TRUE)) & lOriginalValue);
			}
		}
	}

	//이걸 한번만 호출하기 위해 위의 과정 수행.. 
	lRet = m_ctrlActUtlType.WriteDeviceBlock(strDeviceNum, lWriteWordSize, lplWriteData);

	delete [] lplWriteData;

	return lRet;
}

long CDlgMXComponent::HEXtoDEC(CString strHexNum)
{
	long lDecNum = 0;
	long lDecTotalNum = 0;
	long lHexLength = 0;
	CString strHex;
	

	strHex.Empty();

	lHexLength = strHexNum.GetLength();

	for(int iConvertStep = 0; iConvertStep < lHexLength; iConvertStep++)
	{
		strHex = strHexNum.Mid(lHexLength - iConvertStep - 1, 1);

		if(strHex.Compare("A") == FALSE)
			lDecNum = 10;
		else if(strHex.Compare("B")== FALSE)
			lDecNum = 11;
		else if(strHex.Compare("C")== FALSE)
			lDecNum = 12;
		else if(strHex.Compare("D")== FALSE)
			lDecNum = 13;
		else if(strHex.Compare("E")== FALSE)
			lDecNum = 14;
		else if(strHex.Compare("F")== FALSE)
			lDecNum = 15;
		else
			lDecNum = atoi(strHex);

		lDecNum = lDecNum * GetSquareValueForBitMem(BIT_DEVICE_SIZE, iConvertStep, FALSE);
		lDecTotalNum += lDecNum;
	}

	return lDecTotalNum;
}

long CDlgMXComponent::GetSquareValueForBitMem(long lBaseNum, long lSquareNum, BOOL bSet)
{
	long lResultSquareValue = 1; 
											
	for(int i = 0; i < lSquareNum; i++)		//2^n == 100000...00
		lResultSquareValue *= lBaseNum;

	if(bSet == TRUE && lSquareNum != 0)
		lResultSquareValue -= 1;		//2^n - 1 == 11111....11

	return lResultSquareValue;
}

long CDlgMXComponent::GetSquareValueForBitMem2(long lBaseNum, long lSquareNum, BOOL bSet)
{
	long lResultSquareValue = 1; 
	
	if(bSet != FALSE )
	{
		for(int i = 0; i < lSquareNum; i++)		//2^n == 100000...00
			lResultSquareValue *= lBaseNum;
	}
	else
	{
		return 0;
	}

	return lResultSquareValue;
}


long CDlgMXComponent::GetMemoryBitBlock(CString strDeviceText, long lSize, long* lplBitData)
{
	long lRet = 0;
	CString strDeviceWord;
	CString strDeviceNum;
	long lMemoryInterval;
	long lRemainderSize;
	long lWordReadSize;
	long lTotalMemory;
	long lLastDeviceBitSize;
	long lDeviceNum;
	long lstrNumSize;

	long lBitDeviceSize;
	long lMaxWriteValue;

	strDeviceWord.Empty();
	strDeviceNum.Empty();
	lMemoryInterval = 0;
	lRemainderSize = 0;
	lWordReadSize = 0;
	lTotalMemory = 0;
	lLastDeviceBitSize = 0;
	lDeviceNum = 0;
	lstrNumSize = 0;

	lBitDeviceSize = BIT_DEVICE_SIZE;
	lMaxWriteValue = MAX_WRITE_VALUE;
	
	lstrNumSize = strDeviceText.GetLength() - 1;

	strDeviceWord = strDeviceText.Mid(0, 1);
	strDeviceNum = strDeviceText.Mid(1, lstrNumSize);

// 	lMemoryInterval = lDeviceNum % lBitDeviceSize;
// 	lDeviceNum = lDeviceNum - lMemoryInterval;	
// 
// 	strDeviceNum.Format(_T("%s%d"), strDeviceWord, lDeviceNum);

	if(strDeviceWord == "X" || strDeviceWord == "Y" || strDeviceWord == "B" || strDeviceWord == "SB")	//Device Number가 16진수
	{
		lDeviceNum = HEXtoDEC(strDeviceNum);	
		
		lMemoryInterval = lDeviceNum % lBitDeviceSize;
		lDeviceNum = lDeviceNum - lMemoryInterval;		
		strDeviceNum.Format(_T("%s%x"), strDeviceWord, lDeviceNum);	
	}
	else
	{
		lDeviceNum = atoi(strDeviceNum);

		lMemoryInterval = lDeviceNum % lBitDeviceSize;
		lDeviceNum = lDeviceNum - lMemoryInterval;	
		strDeviceNum.Format(_T("%s%d"), strDeviceWord, lDeviceNum);
	}

	lRemainderSize	= lSize % lBitDeviceSize;
	
	lLastDeviceBitSize = lMemoryInterval+lRemainderSize;
	
	if(lSize <= lBitDeviceSize)	 
	{
		lWordReadSize = 1;
		
		long lJudgeSize = lMemoryInterval + lSize;
		if(lJudgeSize > lBitDeviceSize)		
			++lWordReadSize;
	}
	else
	{
		lWordReadSize = lSize / lBitDeviceSize;
		
		if(lLastDeviceBitSize != 0)		
			++lWordReadSize;
	}	
	
	lTotalMemory = lSize;

	long* lplReadWordData	= new long[lWordReadSize];
	long* lplReadData		= new long[lWordReadSize * lBitDeviceSize];
	long* lpReadBitValue	= new long[lTotalMemory];

	memset(lplBitData, 0, lSize * sizeof(long));
	memset(lplReadWordData, 0, lWordReadSize * sizeof(long));
	memset(lplReadData, 0, lWordReadSize * lBitDeviceSize* sizeof(long));
	memset(lpReadBitValue, 0, lTotalMemory * sizeof(long));	
				
	lRet = m_ctrlActUtlType.ReadDeviceBlock(strDeviceNum, lWordReadSize, lplReadWordData);

	if(lRet != 0)
		return lRet;

	//Decimal to Binary
	int iBitReadSize = 0;
	for(int i = 0; i < lWordReadSize; i++)
	{
		iBitReadSize = i*lBitDeviceSize;
		for(int j = 0; j < lBitDeviceSize; j++)
		{	
			lplReadData[iBitReadSize] = lplReadWordData[i] % BINARY_NUM;
			lplReadWordData[i] /= BINARY_NUM;

			if(lplReadWordData[i] < BINARY_NUM)
			{
				lplReadData[iBitReadSize+1] = lplReadWordData[i];
				break;
			}
			iBitReadSize++;
		}
	}	
	
	int iRealBitReadSize=0;
	for(int iReadNum = lMemoryInterval; iReadNum < lSize + lMemoryInterval; iReadNum++)
		lpReadBitValue[iRealBitReadSize++] = lplReadData[iReadNum];

	memcpy(lplBitData, lpReadBitValue, lTotalMemory * sizeof(long));

	delete [] lpReadBitValue;
	delete [] lplReadWordData;
	delete [] lplReadData;
	
	return lRet;
}

long CDlgMXComponent::GetMemoryWordBlock(CString strDeviceText, long lSize, float* fpData, UINT nSize)
{
	long lRet = 0;
	CString strDeviceNum;
	strDeviceNum.Empty();
	strDeviceNum.Format(_T("%s"), strDeviceText);

	int iNum = 0;
	FLOAT_FIELD u_FloatField;
	long lGetSize = 0;
	lGetSize = lSize * nSize;
	long* lpGetData = new long[lGetSize]; 
	memset(lpGetData, 0, lGetSize * sizeof(long));

	lRet = m_ctrlActUtlType.ReadDeviceBlock(strDeviceNum, lGetSize, lpGetData);

	for(int i = 0; i < lSize; i++)
	{
		if( nSize == 3)
		{
			lpGetData[iNum];					// INDEX
			u_FloatField.m_16bit.m_Bottom1Byte = lpGetData[++iNum];
			u_FloatField.m_16bit.m_Top1Byte    = lpGetData[++iNum];
			++iNum;
		}
		else
		{
			u_FloatField.m_16bit.m_Bottom1Byte = lpGetData[iNum];
			u_FloatField.m_16bit.m_Top1Byte    = lpGetData[++iNum];
			++iNum;
		}
		
		fpData[i] =  u_FloatField.m_float;
		
	}

	delete[] lpGetData;

	return lRet;
}

long CDlgMXComponent::SetMemoryWordBlock(CString strDeviceText, long lSize, float* fpData, UINT nSize)
{
	long lRet = 0;
	CString strDeviceNum;
 	strDeviceNum.Empty();
	 	
 	strDeviceNum.Format(_T("%s"), strDeviceText);

	int iNum = 0;
	FLOAT_FIELD u_FloatField;
	long lSetSize = 0;
	lSetSize = lSize * nSize;
	long* lpSetData = new long[lSetSize]; 
	memset(lpSetData, 0, lSetSize * sizeof(long));

// 	for(int i = 0; i < lSize; i++)
// 	{
// 		u_FloatField.m_float = fpData[i];
// 		lpSetData[iNum] = u_FloatField.m_16bit.m_Bottom1Byte;
// 		lpSetData[++iNum] = u_FloatField.m_16bit.m_Top1Byte;
// 		++iNum;
// 	}

	for(int i = 0; i < lSize; i++)
	{
		if( nSize == 3)
		{
			u_FloatField.m_float = fpData[i];
			lpSetData[  iNum] = i+1;
			lpSetData[++iNum] = u_FloatField.m_16bit.m_Bottom1Byte;
			lpSetData[++iNum] = u_FloatField.m_16bit.m_Top1Byte;
			++iNum;
		}
		else
		{
			u_FloatField.m_float = fpData[i];
			lpSetData[  iNum] = u_FloatField.m_16bit.m_Bottom1Byte;
			lpSetData[++iNum] = u_FloatField.m_16bit.m_Top1Byte;
			++iNum;
		}	
	}

	lRet = m_ctrlActUtlType.WriteDeviceBlock(strDeviceNum, lSetSize, lpSetData);

	delete [] lpSetData;

	return lRet;
}


long CDlgMXComponent::GetMemoryWordBlock2(CString strDeviceText, long lSize, TCHAR* tpData)
{
	long lRet = 0;
	CString strDeviceNum;
	strDeviceNum.Empty();
	strDeviceNum.Format(_T("%s"), strDeviceText);

	int iNum = 0;
	FLOAT_FIELD u_FloatField;
	long lGetSize = lSize * 2;
	long* lpGetData = new long[lGetSize]; 
	memset(lpGetData, 0, lGetSize * sizeof(long));

	lRet = m_ctrlActUtlType.ReadDeviceBlock(strDeviceNum, lGetSize, lpGetData);

	for(int i = 0; i < lSize; i++)
	{
		u_FloatField.m_16bit.m_Bottom1Byte = lpGetData[iNum];
//		u_FloatField.m_16bit.m_Top1Byte    = lpGetData[++iNum];

		if( lSize == 8)
		{
			if( u_FloatField.m_char[0] != 0)
				tpData[i]   =  u_FloatField.m_char[0];
			else
				tpData[i]   = '0';
		}
		else
		{
			if( u_FloatField.m_char[0] != 0)
				tpData[2*i]   =  u_FloatField.m_char[0];
			else
				tpData[2*i]   = '0';

			if( u_FloatField.m_char[1] != 0)
				tpData[2*i+1] =  u_FloatField.m_char[1];
			else
				tpData[2*i+1]   = '0';
		}
		

		++iNum;
	}

	delete[] lpGetData;

	return lRet;
}

long CDlgMXComponent::SetMemoryWordBlock2(CString strDeviceText, long lSize, TCHAR* tpData)
{
	long lRet = 0;
	CString strDeviceNum;
	strDeviceNum.Empty();

	strDeviceNum.Format(_T("%s"), strDeviceText);

	int iNum = 0;
	FLOAT_FIELD u_FloatField;
	long lSetSize = lSize * 2;
	long* lpSetData = new long[lSetSize]; 
	memset(lpSetData, 0, lSetSize * sizeof(long));

	ZeroMemory(&u_FloatField, sizeof(u_FloatField));

	for(int i = 0; i < lSize; i++)
	{
		// jsyoon 202005493 change
		if( lSize == 8)
		{
			// 16에 하나 
			u_FloatField.m_char[0] = tpData[i];
			u_FloatField.m_char[1] = 0;
			lpSetData[iNum]   = u_FloatField.m_16bit.m_Bottom1Byte;
		}
		else
		{
			// 16에 두자리
			u_FloatField.m_char[0] = tpData[2*i];
			u_FloatField.m_char[1] = tpData[2*i+1];
 			lpSetData[iNum]   = u_FloatField.m_16bit.m_Bottom1Byte;
 		//	lpSetData[++iNum] = u_FloatField.m_16bit.m_Top1Byte;
		}
	
		++iNum;
	}

	lRet = m_ctrlActUtlType.WriteDeviceBlock(strDeviceNum, lSetSize, lpSetData);

	delete [] lpSetData;

	return lRet;
}

long CDlgMXComponent::GetMemoryWordBlock3(CString strDeviceText, long lSize, long* lpData, UINT nSize)
{
	long lRet = 0;
	CString strDeviceNum;
	strDeviceNum.Empty();
	strDeviceNum.Format(_T("%s"), strDeviceText);

	int iNum = 0;
	FLOAT_FIELD u_FloatField;
	long lGetSize = 0;
	lGetSize = lSize * nSize;
	long* lpGetData = new long[lGetSize]; 
	memset(lpGetData, 0, lGetSize * sizeof(long));

	lRet = m_ctrlActUtlType.ReadDeviceBlock(strDeviceNum, lGetSize, lpGetData);

	for(int i = 0; i < lSize; i++)
	{
		if( nSize == 3)
		{
			lpGetData[iNum];		// INDEX
			u_FloatField.m_16bit.m_Bottom1Byte = lpGetData[++iNum];
			u_FloatField.m_16bit.m_Top1Byte    = lpGetData[++iNum];
			++iNum;
		}
		else
		{
			u_FloatField.m_16bit.m_Bottom1Byte = lpGetData[iNum];
			u_FloatField.m_16bit.m_Top1Byte    = lpGetData[++iNum];
			++iNum;
		}
		
		lpData[i] =  u_FloatField.lfloat;	
	}

	delete[] lpGetData;

	return lRet;
}

long CDlgMXComponent::SetMemoryWordBlock3(CString strDeviceText, long lSize, long* lpData, UINT nSize)
{
	long lRet = 0;
	CString strDeviceNum;
	strDeviceNum.Empty();

	strDeviceNum.Format(_T("%s"), strDeviceText);

	int iNum = 0;
	FLOAT_FIELD u_FloatField;
	long lSetSize = 0;
	lSetSize = lSize * nSize;
	long* lpSetData = new long[lSetSize]; 
	memset(lpSetData, 0, lSetSize * sizeof(long));

	for(int i = 0; i < lSize; i++)
	{
		u_FloatField.lfloat = lpData[i];

		if( nSize == 3)
		{
			lpSetData[  iNum]   = i+1;
			lpSetData[++iNum] = u_FloatField.m_16bit.m_Bottom1Byte;
			lpSetData[++iNum] = u_FloatField.m_16bit.m_Top1Byte;
			++iNum;
		}
		else
		{
			lpSetData[  iNum] = u_FloatField.m_16bit.m_Bottom1Byte;
			lpSetData[++iNum] = u_FloatField.m_16bit.m_Top1Byte;
			++iNum;
		}
	}

	lRet = m_ctrlActUtlType.WriteDeviceBlock(strDeviceNum, lSetSize, lpSetData);

	delete [] lpSetData;

	return lRet;
}