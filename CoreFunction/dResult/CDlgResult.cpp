// CDlgResult.cpp: 구현 파일
//

#include "pch.h"
#include "dResult.h"
#include "CDlgResult.h"
#include "afxdialogex.h"

#define ALPHA_RESULT_MEMMAP_BUFFER_NAME_NEW _T("sNEW_ALPHA_1_RESULT_MEMMAP_BUFFER_NAME")
#define ALPHA_RESULT_MEMMAP_AREA_NAME_NEW   _T("sNEW_ALPHA_1_RESULT_MEMMAP_AREA_NAME")
#define ALPHA_RESULT_MEMMAP_MUTEX_NAME_NEW  _T("sNEW_ALPHA_1_RESULT_MEMMAP_MUTEX_NAME")

HANDLE g_hMutex = NULL;
CCriticalSection	g_csUpdateSpead;
bool srMutexCreate()
{
	if (g_hMutex != NULL) return true;
	g_hMutex = CreateMutex(NULL, FALSE, ALPHA_RESULT_MEMMAP_MUTEX_NAME_NEW);
	if (g_hMutex == NULL) return false;
	return true;
}
bool srMutexClose()
{
	if (g_hMutex == NULL) return true;
	CloseHandle(g_hMutex);
	g_hMutex = NULL;
	return true;
}
bool srMutexLock()
{
	if (g_hMutex == NULL) return false;
	WaitForSingleObject(g_hMutex, INFINITE);
	return true;
}
bool srMutexUnlock()
{
	if (g_hMutex == NULL) return false;
	ReleaseMutex(g_hMutex);
	return true;
}

// 공유메모리 생성
LPVOID AllocateSPIMemory(HANDLE* pMemHandle, size_t nByte, TCHAR* cMemoryMapName, bool* bRet)
{
	LPVOID pMem;
	bool   bCRet;
//	*pMemHandle = ::CreateFileMapping((HANDLE)0xFFFFFFFF, NULL, PAGE_READWRITE, 0, nByte, cMemoryMapName);
	*pMemHandle = ::CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, nByte, cMemoryMapName);
	if (GetLastError() == ERROR_ALREADY_EXISTS) bCRet = FALSE;
	else bCRet = TRUE;

	if (*pMemHandle == NULL)
	{
		*bRet = FALSE;
		return NULL;
	}
	pMem = ::MapViewOfFile(*pMemHandle, FILE_MAP_ALL_ACCESS, 0, 0, nByte);
	if (pMem == NULL)
	{
		*bRet = FALSE;
		return NULL;
	}

	*bRet = bCRet;
	return pMem;
}

// 공유메모리 해제
void FreeSPIMemory(HANDLE* pMemHandle, LPVOID pParam)
{
	if (pParam == NULL || *pMemHandle == NULL) return;

	UnmapViewOfFile(pParam);
	CloseHandle(*pMemHandle);
	pParam = NULL;
	*pMemHandle = NULL;
}

// CDlgResult 대화 상자

IMPLEMENT_DYNAMIC(CDlgResult, CDialog)

CDlgResult::CDlgResult(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_DLG_RESULT, pParent)
{
	m_pRMonitorThread = NULL;
	nBuffer = NULL;
	UploadDataDone = false;

	m_iTValue = 0;
	m_nMeasMode = 0;
	m_nMeasType = 0;
	m_nMeasAreaCnt = 0;
	m_nMaxMeasureRcpFlag = 0;
	m_nMeasureGlassCnt = 0;
	m_bDisplayResulted = false;
	m_bFastUpload = false;			// LYS 2026-10-06 Hybrid Phase 5 resume replay
	m_nUploadDoneCount = 0;
	m_stMeasDay = _T("");
	m_strConfigPath = _T("");

	memset(m_dICPSTAT, 0, sizeof(m_dICPSTAT));
	ZeroMemory(&m_RData, sizeof(RSRITEMDATA));

	m_vecVisDefecs.clear();
	m_vecMicroDefecs.clear();

	m_bSaveResultDone = false;

	m_nCellCountX = 0;
	m_nCellCountY = 0;
}

CDlgResult::~CDlgResult()
{
	SetResultMonitoring(FALSE);
}

void CDlgResult::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDlgResult, CDialog)
END_MESSAGE_MAP()


// CDlgResult 메시지 처리기

int	 CDlgResult::GetNBuffer()
{
	if (!m_bInit) return 0;
	return (*nBuffer);
}

// LYS 2026-10-06 Hybrid Phase 5 resume replay: rows still waiting in the shared ring + rows the monitor thread finished
void CDlgResult::GetUploadState(int* pQueue, long* pDone)
{
	if (pQueue != NULL)
		*pQueue = (m_bInit && nBuffer != NULL) ? *((volatile int*)nBuffer) : 0;
	if (pDone != NULL)
		*pDone = InterlockedCompareExchange(&m_nUploadDoneCount, 0, 0);
}

TCHAR* CDlgResult::GetErrMsg()
{
	return sErrMsg;
}

bool CDlgResult::ClearAll()
{
	(*nBuffer) = 0;
	return true;
}


void CDlgResult::OnDestroy()
{
}

BOOL CDlgResult::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_sprdResult = new CMySpreadDll(IDC_FPSPREAD_RESULT_DATA, FPS_RESOURCE_ID + 19, this, TRUE);
	m_sprdTempResult = new CMySpreadDll(IDC_FPSPREAD_RESULT_TEMP_DATA, FPS_RESOURCE_ID + 20, this, TRUE);

	if (m_sprdResult->GetSafeHwnd() != NULL)
	{
		//Spread Sheet Naming 
		//m_sprdResult->SetSheetCount(5)
		m_sprdResult->SetSheetCount(4);

		//TCHAR* szText[5] = { _T("특성"),_T("화질"),_T("면계측"), _T("시야각"), _T("APD") };
		TCHAR* szText[4] = { _T("특성"),_T("화질"), _T("시야각"), _T("APD")};		// LYS 2026-07-02 안쓰는 Mode 삭제
		CString strValue;
		//for (int iSheet = 0; iSheet < 5; iSheet++)
		for (int iSheet = 0; iSheet < 4; iSheet++)		// LYS 2026-07-02 안쓰는 Mode 삭제
		{
			m_sprdResult->SetSheet(iSheet + 1);
			strValue.Format(_T("%s"), szText[iSheet]);
			m_sprdResult->SetSheetName(iSheet + 1, strValue);

			m_sprdResult->InitDefault();

			SS_CELLTYPE cellType;
			m_sprdResult->SetTypeEdit(&cellType, SSS_ALIGN_VCENTER | SSS_ALIGN_CENTER, 30, NULL, SS_CASE_NOCASE);
			m_sprdResult->SetCellType(SS_ALLCOLS, SS_ALLROWS, &cellType);

			m_sprdResult->MakeFont(&m_hfont);
			m_sprdResult->SetFont(SS_ALLCOLS, SS_ALLROWS, m_hfont, TRUE);

			m_sprdResult->SetBool(SSB_AUTOCLIPBOARD, TRUE);
			m_sprdResult->SetClipboardOptions(SS_CLIP_NOHEADERS);

			m_sprdResult->SetColHeaderRows(1);


			//m_sprdResult->SetMaxCols(0);
			m_sprdResult->SetMaxRows(0);
			//m_sprdResult->SetSheet(1);	// 화질
			//m_sprdResult->SetMaxCols(MEAS_RESULT_ICP_REJECT_DATA_END);
			//
			//m_sprdResult->SetSheet(2);	// 특성 
			//m_sprdResult->SetMaxCols(MEAS_RESULT_ICP_CRAC_DATA_END);
			//
			//m_sprdResult->SetSheet(3);	// 면계측 
			//m_sprdResult->SetMaxCols(MEAS_RESULT_ICP_VISION_DATA_END);
			//
			//m_sprdResult->SetSheet(4);	// 시야각
			//m_sprdResult->SetMaxCols(MEAS_RESULT_ICP_ANGLE_DATA_END);
		}		
	}

	GetConfig();

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}

// 공유메모리 생성
void CDlgResult::SetSearchMode(int iMode)
{
	InitShareMemory();

	if (iMode == 1)
	{
		m_bStop = FALSE;
		m_hEventStop = CreateEvent(NULL, TRUE, TRUE, NULL);
		m_hEventResume = CreateEvent(NULL, TRUE, FALSE, NULL);
		m_pRMonitorThread = AfxBeginThread(CBResultMonitorProc, this);

		int nRes = GetNBuffer();

		SetResultMonitoring(TRUE);
	}
}

void CDlgResult::SetConfigPath(CString str)
{
	m_strConfigPath = str;

	GetConfig();
}


bool CDlgResult::InitShareMemory()	
{
	m_bInit = false;
	bool bRet = true;
	bool bRet2 = true;

	TCHAR* sName = (TCHAR*)malloc((_tcslen(ALPHA_RESULT_MEMMAP_AREA_NAME_NEW) + 4) * sizeof(TCHAR));
	for (int i = 0; i < RITEM_BUFFER_MAX; i++)
	{
		_stprintf(sName, _T("%s_%02d"), ALPHA_RESULT_MEMMAP_AREA_NAME_NEW, i);

		pBuffer[i] = NULL;

		int     nSize_nBuffer = sizeof(RSRITEMDATA);
		bool    bCreate_nBuffer = false;

		pBuffer[i] = (RSRITEMDATA*)AllocateSPIMemory(&hMem_nBuffer, nSize_nBuffer, sName, &bCreate_nBuffer);
		if (pBuffer[i] == NULL)
		{
			_stprintf_s(sErrMsg, 1024, _T("Cannot init memory.."));
		}
		bRet = bRet && pBuffer[i] != NULL ? true : false;

		_stprintf(sName, _T(""));
	}

	free(sName);
	nBuffer = NULL;

	TCHAR* sMMFName_nBuffer = (TCHAR*)malloc((_tcslen(ALPHA_RESULT_MEMMAP_BUFFER_NAME_NEW) + 9) * sizeof(TCHAR));
	_stprintf(sMMFName_nBuffer, _T("%s_nBuffer"), ALPHA_RESULT_MEMMAP_BUFFER_NAME_NEW);

	int     nSize_nBuffer = sizeof(int);
	bool    bCreate_nBuffer = false;

	nBuffer = (int*)AllocateSPIMemory(&hMem_nBuffer, nSize_nBuffer, sMMFName_nBuffer, &bCreate_nBuffer);
	free(sMMFName_nBuffer);
	if (nBuffer == NULL)
	{
		return false;
	}
	if (bCreate_nBuffer) *nBuffer = 0;

	bRet2 = srMutexCreate();
	if (!bRet2) _stprintf_s(sErrMsg, 1024, _T("Cannot Create mutex"));


	m_bInit = bRet && bRet2;

	return bRet && bRet2;
}

UINT CDlgResult::CBResultMonitorProc(LPVOID pParam)
{
	CDlgResult* me = (CDlgResult*)pParam;
	me->CBResultMonitorBody();
	return 0;
}

void CDlgResult::CBResultMonitorBody()
{
	while (!m_bStop)
	{
		DWORD dwResult = WaitForSingleObject(m_hEventStop, 100);
		switch (dwResult)
		{
		case WAIT_OBJECT_0:
		// jsyoon 2022.09.28 change
		//	WaitForSingleObject(m_hEventResume, INFINITE);
			m_bStop = TRUE;
			break;

		case WAIT_TIMEOUT:
		{
			int nCnt = 0, i = 0;
			int nRet = VTM_RV_SUCCESSED;
			nCnt = GetNBuffer();		// 공유메모리에 쌓인 개수확인
			if (nCnt > 0)
			{
				for (i = 0; i < nCnt; i++)
				{
					Download(&m_RData);		// 공유메모리에서 가져오기

					//if (m_pDlgDataView != NULL)
					//{
					//	m_pDlgDataView->UploadDataDone = false;
					//
					//	nRet = m_pDlgDataView->UploadData(&m_RData);
					//}
					UploadDataDone = false;
					UploadData(&m_RData); // 결과 데이터 디스플레이
					InterlockedIncrement(&m_nUploadDoneCount);	// LYS 2026-10-06 Hybrid Phase 5 resume replay: finished-row counter (replay pacing)

				}
				TRACE(_T("Download %d, %s \n"), m_RData.m_iProbe, m_RData.szResultFile);
			}
			else
			{
				//if (m_pDlgDataView != NULL)
				//	m_pDlgDataView->UploadDataDone = true;

				UploadDataDone = true;
			}
		}
		break;
		}
	}
}

void CDlgResult::SetResultMonitoring(BOOL bSet)
{
	if (m_pRMonitorThread == NULL) return;

	if (bSet)
	{
		m_bStop = FALSE;
		ResetEvent(m_hEventStop);
		SetEvent(m_hEventResume);
	}
	else
	{
		ResetEvent(m_hEventResume);
		SetEvent(m_hEventStop);
	}
}

// 공유메모리에 올리기
bool CDlgResult::UploadDataToShare(PRSRITEMDATA result)
{
	bool bRet = false;
	if (result == NULL)
		return bRet;

	int iMode = 0;
	int i = 0, nItem = 0;
	nItem = result->nCntData;
	iMode = result->nMeasMode;
	switch (iMode)
	{
	case PGM_MODE_8K:
	case PGM_MODE_6K:
	case PGM_MODE_TR:
	case PGM_MODE_SE:
	case PGM_MODE_RS:
	case PGM_MODE_PPA:
	case PGM_MODE_3D:
	case PGM_MODE_CM:
	case PGM_MODE_OD:
	case PGM_MODE_REVIEW:
	case PGM_MODE_ANGLE:
	case PGM_MODE_LUMIN:
	case PGM_MODE_VISION:
	case PGM_MODE_COLOR:
	case PGM_MODE_PIXEL:
	case PGM_MODE_IVL:		// jdm 20190202 add.
	{
		bRet = Upload(result);
	}
	break;
	}

	return bRet;
}

bool CDlgResult::Upload(PRSRITEMDATA pArea)
{
	if (!m_bInit)
	{
		_stprintf_s(sErrMsg, 1024, _T("Cannot init memory.."));
		return false;
	}

	srMutexLock();

	if (*nBuffer == RITEM_BUFFER_MAX)
	{
		for (int i = 1; i < (*nBuffer); i++)
		{
			memcpy(pBuffer[i - 1], pBuffer[i], sizeof(RSRITEMDATA));
		}

		(*nBuffer)--;
	}

	if ((*nBuffer) < RITEM_BUFFER_MAX)
	{
		memcpy(pBuffer[(*nBuffer)], pArea, sizeof(RSRITEMDATA));
		(*nBuffer)++;
		srMutexUnlock();
		_stprintf_s(sErrMsg, 1024, _T(""));
		return true;
	}

	srMutexUnlock();
	_stprintf_s(sErrMsg, 1024, _T("Buffer Full!!"));
	return false;

}

bool CDlgResult::Download(PRSRITEMDATA pArea, bool bJustMonitoring, int iIdx)
{
	if (!m_bInit) { _stprintf_s(sErrMsg, 1024, _T("Cannot init memory..")); return false; }

	srMutexLock();

	if (bJustMonitoring)
	{
		if (iIdx < (*nBuffer))
		{
			memcpy(pArea, pBuffer[(*nBuffer)], sizeof(RSRITEMDATA));
			srMutexUnlock();
			_stprintf_s(sErrMsg, 1024, _T(""));
			return true;
		}
		else
		{
			_stprintf_s(sErrMsg, 1024, _T("Idx err.."));
			return false;
		}
	}
	else
	{
		if ((*nBuffer) != 0)
		{
			memcpy(pArea, pBuffer[(0)], sizeof(RSRITEMDATA));

			for (int i = 1; i < (*nBuffer); i++)
			{
				memcpy(pBuffer[i - 1], pBuffer[i], sizeof(RSRITEMDATA));
			}
			(*nBuffer)--;

			srMutexUnlock();
			_stprintf_s(sErrMsg, 1024, _T(""));
			return true;
		}
	}

	srMutexUnlock();
	_stprintf_s(sErrMsg, 1024, _T("Buffer empty.."));
	return false;
}

int CDlgResult::UploadData(PRSRITEMDATA result)
{
	if (result == NULL)
		return MEAS_RESULT_FAIL;

	int nRet = MEAS_RESULT_FAIL;

	CString strDay = gf_GetCurDayString();

	int i = 0, nItem = 0, iProbe = 0, nStep = 0;
	CString strPath;

	iProbe = result->m_iProbe;
	nItem = result->nCntData;
	nStep = result->iCurStep;

	m_nMeasMode = result->nMeasMode;
	// LYS 2026-07-24 WAD Multi Point (Phase 6): offsets for current point
	m_nMultiPoint    = result->nMultiPoint;
	m_dMultiOffsetRX = result->dOffsetRX;
	m_dMultiOffsetGX = result->dOffsetGX;
	m_dMultiOffsetBY = result->dOffsetBY;
	if (m_nMultiPoint < 0 || m_nMultiPoint > 99)
		m_nMultiPoint = 0;
	m_iTValue = result->m_iTValue;							// Recipe Param t-VALUE 설정 시
	m_nMeasType = result->m_nMeasType;						// Recipe Param CS SUB 측정 타입
	m_nMeasAreaCnt = result->m_nMeasAreaCnt;				// 측정 Area 총 개수
	m_nMaxMeasureRcpFlag = result->m_nMaxMeasureRcpFlag;	// 측정한 Recipe 개수, 멀티레시피 최대 10개
	_tcscpy_s(m_szGlassID, _countof(m_szGlassID), result->szGlassID);		// GLASS ID
	_tcscpy_s(m_szLotID, _countof(m_szLotID), result->szLotID);				// LOT ID
	_tcscpy_s(m_szRecipeID, _countof(m_szRecipeID), result->szRecipeID);	// RECIPE ID


	m_stReceiveMeasResult[iProbe].m_iProbe = iProbe;
	m_stReceiveMeasResult[iProbe].m_iGroup = result->iCurArea;
	m_stReceiveMeasResult[iProbe].m_nGroup = result->m_nMeasAreaCnt;
	m_stReceiveMeasResult[iProbe].m_nStep = nStep;
	m_stReceiveMeasResult[iProbe].m_iRetryCnt = result->m_iRetryCnt;
	m_stReceiveMeasResult[iProbe].m_iRetryReason = result->m_iRetryReason;
	m_stReceiveMeasResult[iProbe].m_nPanelNo = result->iPanelNo;	
	m_stReceiveMeasResult[iProbe].m_nAreaNo = result->iMeasArea;	

	CString strData;
	if (nItem > 0)
	{
		strData.Format(_T("%s"), result->stData[0].szData);
		m_stReceiveMeasResult[iProbe].m_dX = _ttof(strData);
		strData.Format(_T("%s"), result->stData[1].szData);
		m_stReceiveMeasResult[iProbe].m_dY = _ttof(strData);
		strData.Format(_T("%s"), result->stData[2].szData);
		m_stReceiveMeasResult[iProbe].m_dZ = _ttof(strData);
		strData.Format(_T("%s"), result->stData[3].szData);
		m_stReceiveMeasResult[iProbe].m_dS = _ttof(strData);

		// CELL MAX
		strData.Format(_T("%s"), result->stData[4].szData);
		m_nCellCountX = _ttoi(strData);

		// STICK MAX
		strData.Format(_T("%s"), result->stData[5].szData);
		m_nCellCountY = _ttoi(strData);
	}

	_tcscpy_s(m_stReceiveMeasResult[iProbe].m_szCellID,	_countof(m_stReceiveMeasResult[iProbe].m_szCellID),	result->szCellData);// Cell ID
	_tcscpy_s(m_stReceiveMeasResult[iProbe].m_szGlassID,_countof(m_stReceiveMeasResult[iProbe].m_szGlassID),result->szGlassID);	// GLASS ID

	// 결과파일 읽어오고
	if (m_nMeasMode != PGM_MODE_REVIEW)
	{
		CString strResultPath, strResultFile, strImageFile;
		strResultPath.Format(_T("%s"), result->szResultPath);
		strResultFile.Format(_T("%s"), result->szResultFile);
		strImageFile.Format(_T("%s"), result->szImageFile);		// jsyoon 20210101 add

		nRet = ReadResultData(strResultPath, strResultFile, strImageFile, iProbe, &m_stReceiveMeasResult[iProbe]);

		// jsyoon 20210512 add
		if (nRet == MEAS_RESULT_FAIL)
		{
			if (m_stReceiveMeasResult[iProbe].m_iRetryReason == MEAS_RESULT_NONE)
			{
				nRet = _REASON_MEAS_FAIL;
				m_stReceiveMeasResult[iProbe].m_iRetryReason = _REASON_MEAS_FAIL;
			}
		}
		else if (nRet != MEAS_RESULT_SUCCESS)
		{
			m_stReceiveMeasResult[iProbe].m_iRetryReason = nRet;
		}
		else
		{
			m_stReceiveMeasResult[iProbe].m_iRetryReason = 0;
		}
	}
	else
	{
		RESULTSTRUCT	stResult;
		memset(&stResult, 0, sizeof(RESULTSTRUCT));

		stResult.dX = m_stReceiveMeasResult[iProbe].m_dX;
		stResult.dY = m_stReceiveMeasResult[iProbe].m_dY;
		stResult.dZ = m_stReceiveMeasResult[iProbe].m_dZ;
		stResult.nSelectGroup = result->iCurArea - 1;

		_tcscpy_s(stResult.szLotID,		_countof(stResult.szLotID),		result->szLotID);
		_tcscpy_s(stResult.szGlassID,	_countof(stResult.szGlassID),	result->szGlassID);
		_tcscpy_s(stResult.szRecipeID,	_countof(stResult.szRecipeID),	result->szRecipeID);
		_tcscpy_s(stResult.szImageName,	_countof(stResult.szImageName),	result->szImageFile);
		_tcscpy_s(stResult.szDate,		_countof(stResult.szDate),		gf_GetCurDayString(2));
		_tcscpy_s(stResult.szTime,		_countof(stResult.szTime),		gf_GetCurTimeString(3));

		strData.Format(_T("%s"), result->stData[4].szData);
		_tcscpy_s(stResult.szMachineID, _countof(stResult.szMachineID), strData);

		nRet = MEAS_RESULT_SUCCESS;
	}

	//////////////////////////////////////////////////////////////////////////
	if (nRet != MEAS_RESULT_NONE)
	{
		m_bDisplayResulted = true;

		::PostMessage(this->m_hWnd, WM_MSG_DISPLAY_RESULT, (WPARAM)iProbe, (LPARAM)nStep);

		// LYS 2026-10-06 Hybrid Phase 5 resume replay: the 1 s display wait is skipped only while fast upload is on (journal replay); off = unchanged
		if (m_bFastUpload == false)
		{
			int iLoop = 0;
			do
			{
				if (m_bDisplayResulted == false)
				{
					if (iLoop > 0)
						TRACE("m_bDisplayResulted %d, TimeOut %d\n", iProbe + 1, iLoop);

					break;
				}

				Sleep(10);

				iLoop++;

			} while (iLoop < 100);	// 1000 msec
		}
	}
	else
	{
		TRACE("LOAD FAIL\n");
		;
	}

	//m_pSystem->stSystem[VTM_PROBE1 + iProbe].m_nReason = nRet;

	return nRet;
}

int CDlgResult::ReadResultData(CString strPath, CString strFileName, CString strImageFile, int nProbe, PMEASRESULT strMeasResult)
{
	CSingleLock sLock(&g_csUpdateSpead);	//크리티컬 섹션 동기화
	sLock.Lock();

	if (sLock.IsLocked())
	{
		CString str, strKey, strResultFile;
		CFileFind	finder;
		TCHAR szValue[MAX_PATH];
		int i = 0;
		CString strTime;				//테스트 로그용 시간 확인
		int iType = m_nMeasMode;		//화질,특성,색이상,시야각 정보 줌

		//파일 존재 유무 확인
		strResultFile.Format(_T("%s%s"), strPath, strFileName);
		if (finder.FindFile(strResultFile) == FALSE)
			return MEAS_RESULT_FILE_NG;

		if (iType == PGM_MODE_VISION)	// 화질
		{
			m_sprdResult->SetSheet(2);	// Set sheet names

			if (strMeasResult->m_nAreaNo + 1 > m_sprdResult->GetMaxRows())
				m_sprdResult->SetMaxRows(strMeasResult->m_nAreaNo + 1); // Row Add

			// HEADER
			strKey = _T("Number");
			m_sprdResult->SetColWidth(MEAS_RESULT_ICP_REJECT_NO, 20.0);
			m_sprdResult->SetValue(MEAS_RESULT_ICP_REJECT_NO, 1, strKey);

			// VALUE
			strKey.Format(_T("%d"), strMeasResult->m_nAreaNo);
			m_sprdResult->SetValue(MEAS_RESULT_ICP_REJECT_NO, strMeasResult->m_nAreaNo + 1, strKey);

			// HEADER 
			strKey = _T("File Name");
			m_sprdResult->SetColWidth(2, 20.0);
			m_sprdResult->SetValue(2, 1, strKey);

			// VALUE
			strKey.Format(_T("%s"), strFileName);
			m_sprdResult->SetColWidth(2, 20.0);
			m_sprdResult->SetValue(2, strMeasResult->m_nAreaNo + 1, strKey);

			TCHAR* pKey[] = { _T("ACTIVE_REJECT_TIME"), _T("ACTIVE_REJECT_NAME"), };
			for (i = 0; i < 2; i++)
			{
				// HEADER
				strKey.Format(_T("%s"), pKey[i]);
				m_sprdResult->SetColWidth(MEAS_RESULT_ICP_REJECT_TIME + i, 20.0);
				m_sprdResult->SetDataString(MEAS_RESULT_ICP_REJECT_TIME + i, 1, strKey);

				// VALUE	
				GetPrivateProfileString(_T("ACTIVE RESULT"), strKey, _T(""), szValue, MAX_PATH, strResultFile);
				str.Format(_T("%s"), szValue);
				str.TrimRight();
				m_sprdResult->SetDataString(MEAS_RESULT_ICP_REJECT_TIME + i, strMeasResult->m_nAreaNo + 1, str);

				if (i == 0) strTime = str;
			}

			FILE* pFile = NULL;
			_tfopen_s(&pFile, strResultFile, _T("r"));

			if (pFile != NULL)
			{
				char strTemp[1000];
				char* pStr;

				CString strTitle;
				CString strTempValue;
				CString strValue[10];

				BOOL bSearch = FALSE;

				while (!feof(pFile))
				{
					pStr = fgets(strTemp, sizeof(strTemp), pFile);
					printf("%s", strTemp);

					if (bSearch)
					{
						//20170725 add by Hee (W_PTN 값 0으로 올라오는거 수정)
						strTitle.Format(_T("%S"), strTemp);
						if (strTitle.Find(_T("[VIS_RESULT]")) >= 0 || strTitle.Find(_T("[MICRO RESULT]")) >= 0 ||
							strTitle.Find(_T("[MICRO DEFECT]")) >= 0 || strTitle.Find(_T("[DETAIL RESULT]")) >= 0)
						{
							bSearch = FALSE;
							break;
						}

						CString strNum;
						CString strVal;
						strTempValue.Format(_T("%S"), strTemp);
						AfxExtractSubString(strNum, strTempValue, 0, ',');

						AfxExtractSubString(strVal, strTempValue, 1, ',');
						if (strVal == _T("SectRatio") || strVal == _T("StepSR")) continue;

						AfxExtractSubString(strVal, strTempValue, 18, ',');

						int nNum = _ttoi(strNum);
						if (nNum >= 0 && nNum <= 9)
						{
							if (_ttof(strValue[nNum]) < _ttof(strVal)) strValue[nNum] = strVal;
						}
					}

					strTitle.Format(_T("%S"), strTemp);
					if (strTitle.Find(_T("[VIS_DEFECT]")) >= 0)
					{
						bSearch = TRUE;
					}
				}
				fclose(pFile);

				//20170609 Modify by Hee
				TCHAR* pPTNData[] = { _T("W_PTN"), _T("R_PTN"), _T("G_PTN"), _T("B_PTN"), _T("GR_PTN"), _T("U_PTN") };
				for (int i = 0; i < (MEAS_RESULT_ICP_VIS_DEFECT5 - MEAS_RESULT_ICP_VIS_DEFECT0 + 1); i++)
				{
					// HEADER
					strKey.Format(_T("%s"), pPTNData[i]);
					m_sprdResult->SetColWidth(MEAS_RESULT_ICP_VIS_DEFECT0 + i, 20.0);
					m_sprdResult->SetDataString(MEAS_RESULT_ICP_VIS_DEFECT0 + i, 1, strKey);

					// VALUE
					m_sprdResult->SetDataString(MEAS_RESULT_ICP_VIS_DEFECT0 + i, strMeasResult->m_nAreaNo + 1,  strValue[i]); //20210209 JCY 임시 수정
				}
			}
			else
			{
				//에러 처리
			}

			int nDetailRejectCount = 0;

			// HEADER
			strKey.Format(_T("%s"), _T("DETAIL_REJECT_COUNT"));
			m_sprdResult->SetColWidth(MEAS_RESULT_ICP_REJECT_COUNT, 20.0);
			m_sprdResult->SetDataString(MEAS_RESULT_ICP_REJECT_COUNT, 1, strKey);

			GetPrivateProfileString(_T("DETAIL RESULT"), strKey, _T(""), szValue, MAX_PATH, strResultFile);
			str.Format(_T("%s"), szValue);
			str.TrimRight();
			nDetailRejectCount = _ttoi(str);

			//if (nDetailRejectCount > m_pSystem->m_nDefectCnt)
			//	nDetailRejectCount = m_pSystem->m_nDefectCnt;

			str.Format(_T("%d"), nDetailRejectCount);
			// VALUE
			m_sprdResult->SetDataString(MEAS_RESULT_ICP_REJECT_COUNT, strMeasResult->m_nAreaNo + 1, str);
			nDetailRejectCount = _ttoi(str);

			int iColMax = 0;
			iColMax = m_sprdResult->GetMaxCols();
			if (iColMax < MEAS_RESULT_ICP_REJECT_COUNT + (nDetailRejectCount * 4))
			{
				m_sprdResult->SetMaxCols(MEAS_RESULT_ICP_REJECT_COUNT + (nDetailRejectCount * 4));
			}

			TCHAR* pData[] = { _T("DETAIL_REJECT_NAME"), _T("DETAIL_REJECT_RGB"),_T("DETAIL_REJECT_X"), _T("DETAIL_REJECT_Y") };
			double dColwidth[4] = { 30.0,20.0,20.0,20.0 };
			for (int j = 0; j < nDetailRejectCount; j++)
			{
				for (i = 0; i < 2; i++)
				{
					// HEADER
					strKey.Format(_T("%s_%d"), pData[i], j + 1);
					m_sprdResult->SetColWidth((MEAS_RESULT_ICP_REJECT_DATA_NAME_1 + i) + (4 * j), dColwidth[i]);
					m_sprdResult->SetDataString((MEAS_RESULT_ICP_REJECT_DATA_NAME_1 + i) + (4 * j), 1, strKey);

					// VALUE
					GetPrivateProfileString(_T("DETAIL RESULT"), strKey, _T(""), szValue, MAX_PATH, strResultFile);
					str.Format(_T("%s"), szValue);
					str.TrimRight();
					m_sprdResult->SetDataString((MEAS_RESULT_ICP_REJECT_DATA_NAME_1 + i) + (4 * j), strMeasResult->m_nAreaNo + 1, str);
				}

				// VALUE
				strKey.Format(_T("DETAIL_REJECT_XY_%d"), j + 1);
				GetPrivateProfileString(_T("DETAIL RESULT"), strKey, _T(""), szValue, MAX_PATH, strResultFile);
				str.Format(_T("%s"), szValue);

				int iEnd = 0;
				CString strValueX, strValueY;
				iEnd = str.Find(_T(","));
				strValueX = str.Mid(0, iEnd);

				str.Delete(0, iEnd + 1);
				strValueY = str;
				strValueY.TrimRight(_T(","));

				// DETAIL_REJECT_X
				// HEADER
				i = 2;
				strKey.Format(_T("%s_%d"), pData[i], j + 1);
				m_sprdResult->SetColWidth((MEAS_RESULT_ICP_REJECT_DATA_NAME_1 + i) + (4 * j), dColwidth[i]);
				m_sprdResult->SetDataString((MEAS_RESULT_ICP_REJECT_DATA_NAME_1 + i) + (4 * j), 1, strKey);

				// VALUE
				m_sprdResult->SetDataString((MEAS_RESULT_ICP_REJECT_DATA_NAME_1 + i) + (4 * j), strMeasResult->m_nAreaNo + 1, strValueX);

				// DETAIL_REJECT_y
				// HEADER
				i = 3;
				strKey.Format(_T("%s_%d"), pData[i], j + 1);
				m_sprdResult->SetColWidth((MEAS_RESULT_ICP_REJECT_DATA_NAME_1 + i) + (4 * j), dColwidth[i]);
				m_sprdResult->SetDataString((MEAS_RESULT_ICP_REJECT_DATA_NAME_1 + i) + (4 * j), 1, strKey);

				// VALUE
				m_sprdResult->SetDataString((MEAS_RESULT_ICP_REJECT_DATA_NAME_1 + i) + (4 * j), strMeasResult->m_nAreaNo + 1, strValueY);
			}

			//20170702 add by Hee ([VIS_DEFECT] 로그로 남겨달라고함, 테스트용 추후 APD로 적용)
			pFile = NULL;
			_tfopen_s(&pFile, strResultFile, _T("r"));

			CString strMerge, strTitle, strTempValue;
			int nTimeSize = 0;

			if (pFile != NULL)
			{
				char strTemp[1000];
				BOOL bSearch = FALSE;

				m_vecVisDefec.clear();

				while (!feof(pFile))
				{
					fgets(strTemp, sizeof(strTemp), pFile);

					if (bSearch)
					{
						strTitle.Format(_T("%S"), strTemp);
						if (strTitle.Find(_T("[VIS_RESULT]")) >= 0 || strTitle.Find(_T("[MICRO RESULT]")) >= 0 ||
							strTitle.Find(_T("[MICRO DEFECT]")) >= 0 || strTitle.Find(_T("[DETAIL RESULT]")) >= 0)
						{
							bSearch = FALSE;
							m_vecVisDefecs.push_back(m_vecVisDefec);
							break;
						}

						nTimeSize = strTime.GetLength();

						Sleep(1);
						strMerge.Empty();
						//strMerge = dfcStr(_T("%s,%s,%s,%s,%s"), m_pSystem->stTitle.m_strEquipNum, //20210209 JCY 임시 수정
						strMerge = dfcStr(_T("%s,%s,%s,%S"),
							strTime.Mid(0, nTimeSize - 4), strMeasResult->m_szGlassID, strMeasResult->m_szCellID, strTemp);

						//20171226 add by Hee (VIS_DEFECT 내용 저장)
						strMerge = dfcStr(_T("%s,%S"), strMeasResult->m_szCellID, strTemp);
						m_vecVisDefec.push_back(strMerge);
					}

					strTitle.Format(_T("%S"), strTemp);
					if (strTitle.Find(_T("[VIS_DEFECT]")) >= 0)
					{
						bSearch = TRUE;
					}
				}
				fclose(pFile);
			}
			else
			{
				//에러 처리
			}

			//20170725 add by Hee ([MICRO DEFECT] 로그로 남겨달라고함, 테스트용 추후 APD로 적용)
			pFile = NULL;
			_tfopen_s(&pFile, strResultFile, _T("r"));

			if (pFile != NULL)
			{
				char strTemp[1000];
				BOOL bSearch = FALSE;

				m_vecMicroDefec.clear();

				while (!feof(pFile))
				{
					fgets(strTemp, sizeof(strTemp), pFile);

					if (bSearch)
					{
						strTitle.Format(_T("%S"), strTemp);
						if (strTitle.Find(_T("[VIS_RESULT]")) >= 0 || strTitle.Find(_T("[MICRO RESULT]")) >= 0 ||
							strTitle.Find(_T("[VIS_DEFECT]")) >= 0 || strTitle.Find(_T("[DETAIL RESULT]")) >= 0)
						{
							bSearch = FALSE;
							m_vecMicroDefecs.push_back(m_vecMicroDefec);
							break;
						}

						nTimeSize = strTime.GetLength();

						Sleep(1);
						strMerge.Empty();
						//strMerge = dfcStr(_T("%s,%s,%s,%s,%s"), m_pSystem->stTitle.m_strEquipNum, 20210209 JCY 임시 수정
						strMerge = dfcStr(_T("%s,%s,%s,%S"),
							strTime.Mid(0, nTimeSize - 4), strMeasResult->m_szGlassID, strMeasResult->m_szCellID, strTemp);

						strMerge = dfcStr(_T("%s,%S"), strMeasResult->m_szCellID, strTemp);
						m_vecMicroDefec.push_back(strMerge);
					}

					strTitle.Format(_T("%S"), strTemp);
					if (strTitle.Find(_T("[MICRO DEFECT]")) >= 0)
					{
						bSearch = TRUE;
					}

				}
				fclose(pFile);
			}
			else
			{
				//에러 처리
			}
		}
		else if (iType == PGM_MODE_LUMIN) //특성
		{
			if (strResultFile.Find(_T(".dat") > 0))		// 특성
			{
				double fRepeatData[PROPERTY_DEG_MAX * PROPERTY_HEAD_MAX/*6*/]; //X, Y (0도, 30도, 45도) // 60도 추가
				memset(fRepeatData, 0.0, sizeof(double) * (PROPERTY_DEG_MAX * PROPERTY_HEAD_MAX)/*6*/);

				m_sprdResult->SetSheet(1);	// Set sheet names

				int nMaxRows = 0;
				nMaxRows = m_sprdResult->GetMaxRows();
				if (nMaxRows < strMeasResult->m_nAreaNo + 1)
				{
					m_sprdResult->SetMaxRows(strMeasResult->m_nAreaNo + 1);
				}

				//2022.05.27 JCY 수정
				TCHAR* pdata[] = { _T("SERIAL"), _T("CELLID"), _T("COORD X"), _T("COORD Y"), _T("EL_PTP"), _T("REPEAT") };

				for (i = 0; i < 6; i++)
				{
					// HEADER
					strKey.Format(_T("%s"), pdata[i]);
					m_sprdResult->SetColWidth(MEAS_RESULT_ICP_CRAC_NO + i, 10);
					m_sprdResult->SetDataString(MEAS_RESULT_ICP_CRAC_NO + i, 1, strKey);					
				}		

				// VALUE
				str.Format(_T("%d"), strMeasResult->m_nAreaNo);
				m_sprdResult->SetDataString(MEAS_RESULT_ICP_CRAC_NO, strMeasResult->m_nAreaNo + 1, str);
				str.Format(_T("%s"), strMeasResult->m_szCellID);
				m_sprdResult->SetDataString(2, strMeasResult->m_nAreaNo + 1, str);
				str.Format(_T("%d"), strMeasResult->m_iRetryReason);
				m_sprdResult->SetDataString(5, strMeasResult->m_nAreaNo + 1, str);
				str.Format(_T("%d"), strMeasResult->m_iRetryCnt);
				m_sprdResult->SetDataString(6, strMeasResult->m_nAreaNo + 1, str);

				if (strMeasResult->m_iRetryReason == 1)
				{
					str = _T("");
					m_sprdResult->SetDataString(3, strMeasResult->m_nAreaNo + 1, str);
					str = _T("");
					m_sprdResult->SetDataString(4, strMeasResult->m_nAreaNo + 1, str);

					return VTM_RV_SUCCESSED;
				}
				else
				{
					str.Format(_T("%.4f"), strMeasResult->m_dX);
					m_sprdResult->SetDataString(3, strMeasResult->m_nAreaNo + 1, str);
					str.Format(_T("%.4f"), strMeasResult->m_dY);
					m_sprdResult->SetDataString(4, strMeasResult->m_nAreaNo + 1, str);
				}
				/////////////////////////////////////////////////////////////////////////////////////////////////////////



				
				TCHAR* pKey[] = { _T("INNER_ID"), _T("START_TIME"), _T("END_TIME"), _T("CELL_JUDGE"), _T("PTN_JUDGE"), _T("SPC_JUDGE"), };
				double dColwidth[6] = { 10.0,20.0,20.0,10.0,10.0,10.0 };
				for (i = 0; i < 6; i++)
				{
					// HEADER
					strKey.Format(_T("%s"), pKey[i]);
					m_sprdResult->SetColWidth(MEAS_RESULT_ICP_INNERID + i, dColwidth[i]);
					m_sprdResult->SetDataString(MEAS_RESULT_ICP_INNERID + i, 1, strKey);

					// VALUE
					GetPrivateProfileString(_T("HEADER"), strKey, _T(""), szValue, MAX_PATH, strResultFile);
					str.Format(_T("%s"), szValue);
					m_sprdResult->SetDataString(MEAS_RESULT_ICP_INNERID + i, strMeasResult->m_nAreaNo + 1, str);
				}

				TCHAR* pColor2[] = { _T("W") };
				//TCHAR* pData2[] = { _T("x_AFT"), _T("y_AFT"), _T("U_DOT_AFT"), _T("V_DOT_AFT"), _T("휘도_AFT"), _T("전류_AFT"), _T("효율_AFT"), };
				TCHAR* pData2[] = { _T("x"), _T("y"), _T("U_DOT"), _T("V_DOT"), _T("휘도"), _T("전류"), _T("효율"), };
				CString strValue, strJudge;
				int iEnd = 0;
				double dTempCU = 0.0;

				for (i = 0; i < 7; i++)
				{
					int j = 0;

					// HEADER
					strKey.Format(_T("%s_%s_AFT"), pColor2[j], pData2[i]);
					m_sprdResult->SetColWidth(MEAS_RESULT_ICP_CRAC_DATA_1 + (2 * i) + (j * 14), 10.0);
					m_sprdResult->SetDataString(MEAS_RESULT_ICP_CRAC_DATA_1 + (2 * i) + (j * 14), 1, strKey);

					// VALUE
					GetPrivateProfileString(_T("DATA"), strKey, _T(""), szValue, MAX_PATH, strResultFile);
					str.Format(_T("%s"), szValue);
							
					iEnd = str.Find(_T(","));	//ALARM ID
					strValue = str.Mid(0, iEnd);
					 
					//20170109 lhj add
					if (i == 5)
					{
						dTempCU = _ttof(strValue);
					}
					else if (i == 6)
					{
						if (dTempCU == 0.0)
							strValue = "0.0000";
					}
					else if (i == 0) fRepeatData[0] = _ttof(strValue);
					else if (i == 1) fRepeatData[1] = _ttof(strValue);
					//

					str.Delete(0, iEnd + 1);
					strJudge = str;
					strJudge.TrimRight(_T(","));
					m_sprdResult->SetDataString(MEAS_RESULT_ICP_CRAC_DATA_1 + (2 * i) + (j * 14), strMeasResult->m_nAreaNo + 1, strValue);

					// HEADER
					strKey.Format(_T("%s_%s_%s"), pColor2[j], pData2[i], _T("JUDGE"));
					m_sprdResult->SetColWidth(MEAS_RESULT_ICP_CRAC_DATA_1 + (2 * i) + 1 + (j * 14), 10.0);
					m_sprdResult->SetDataString(MEAS_RESULT_ICP_CRAC_DATA_1 + (2 * i) + 1 + (j * 14), 1, strKey);

					//VALUE
					m_sprdResult->SetDataString(MEAS_RESULT_ICP_CRAC_DATA_1 + (2 * i) + 1 + (j * 14), strMeasResult->m_nAreaNo + 1, strJudge);
				}

				TCHAR* pColor[] = { _T("W"), _T("R"), _T("G"), _T("B"), };
				TCHAR* pData[] = { _T("X"), _T("Y"), _T("U"), _T("V"), _T("LV"), _T("CU"), _T("EF"), };
				dTempCU = 0.0;
				for (int j = 1; j < 4; j++) 
				{					
					for (i = 0; i < 7; i++)
					{
						// HEADER
						strKey.Format(_T("%s_%s"), pColor[j], pData[i]);
						m_sprdResult->SetColWidth(MEAS_RESULT_ICP_CRAC_DATA_1 + (2 * i) + (j * 14), 10.0);
						m_sprdResult->SetDataString(MEAS_RESULT_ICP_CRAC_DATA_1 + (2 * i) + (j * 14), 1, strKey);

						// VALUE
						GetPrivateProfileString(_T("DATA"), strKey, _T(""), szValue, MAX_PATH, strResultFile);
						str.Format(_T("%s"), szValue);

						iEnd = 0;
						iEnd = str.Find(_T(","));	//ALARM ID
						strValue = str.Mid(0, iEnd);

						if (i == 5)
						{
							dTempCU = _ttof(strValue);
						}
						else if (i == 6)
						{
							if (dTempCU == 0.0)
								strValue = "0.0000";
						}

						str.Delete(0, iEnd + 1);
						strJudge = str;
						strJudge.TrimRight(_T(","));

						m_sprdResult->SetDataString(MEAS_RESULT_ICP_CRAC_DATA_1 + (2 * i) + (j * 14), strMeasResult->m_nAreaNo + 1, strValue);

						// HEADER
						strKey.Format(_T("%s_%s_%s"), pColor[j], pData[i], _T("JUDGE"));
						m_sprdResult->SetColWidth(MEAS_RESULT_ICP_CRAC_DATA_1 + (2 * i) + 1 + (j * 14), 10.0);
						m_sprdResult->SetDataString(MEAS_RESULT_ICP_CRAC_DATA_1 + (2 * i) + 1 + (j * 14), 1, strKey);

						// VALUE
						m_sprdResult->SetDataString(MEAS_RESULT_ICP_CRAC_DATA_1 + (2 * i) + 1 + (j * 14), strMeasResult->m_nAreaNo + 1, strJudge);
					}
				}

				// HEADER
				strKey.Format(_T("%s"), _T("W_MTP_JUDGE"));
				m_sprdResult->SetColWidth(MEAS_RESULT_ICP_JUDGE_MTP_W, 20.0);
				m_sprdResult->SetValue(MEAS_RESULT_ICP_JUDGE_MTP_W, 1 , strKey);

				// VALUE
				GetPrivateProfileString(_T("DATA"), strKey, _T(""), szValue, MAX_PATH, strResultFile);
				str.Format(_T("%s"), szValue);
				m_sprdResult->SetDataString(MEAS_RESULT_ICP_JUDGE_MTP_W, strMeasResult->m_nAreaNo + 1, str);

				// HEADER
				strKey.Format(_T("%s"), _T("WG_MTP_JUDGE"));
				m_sprdResult->SetColWidth(MEAS_RESULT_ICP_JUDGE_MTP_WG, 20.0);
				m_sprdResult->SetDataString(MEAS_RESULT_ICP_JUDGE_MTP_WG, 1, strKey);

				// VALUE
				GetPrivateProfileString(_T("DATA"), strKey, _T(""), szValue, MAX_PATH, strResultFile);
				str.Format(_T("%s"), szValue);
				m_sprdResult->SetDataString(MEAS_RESULT_ICP_JUDGE_MTP_WG, strMeasResult->m_nAreaNo + 1, str);


				// HEADER
				//strKey.Format(_T("%s"), _T("WG_x_AFT"));
				m_sprdResult->SetColWidth(MEAS_RESULT_ICP_WG_x_AFT, 20.0);
				m_sprdResult->SetDataString(MEAS_RESULT_ICP_WG_x_AFT, 1, _T("WG_x"));

				// VALUE
				GetPrivateProfileString(_T("DATA"), strKey, _T(""), szValue, MAX_PATH, strResultFile);
				str.Format(_T("%s"), szValue);
				iEnd = 0;
				iEnd = str.Find(_T(","));
				strValue = str.Mid(0, iEnd);
				m_sprdResult->SetDataString(MEAS_RESULT_ICP_WG_x_AFT, strMeasResult->m_nAreaNo + 1, strValue);

				// HEADER
				//strKey.Format(_T("%s"), _T("WG_y_AFT"));
				m_sprdResult->SetColWidth(MEAS_RESULT_ICP_WG_y_AFT, 20.0);
				m_sprdResult->SetDataString(MEAS_RESULT_ICP_WG_y_AFT, 1, _T("WG_x"));

				// VALUE
				GetPrivateProfileString(_T("DATA"), strKey, _T(""), szValue, MAX_PATH, strResultFile);
				str.Format(_T("%s"), szValue);
				iEnd = 0;
				iEnd = str.Find(_T(","));
				strValue = str.Mid(0, iEnd);
				m_sprdResult->SetValue(MEAS_RESULT_ICP_WG_y_AFT, strMeasResult->m_nAreaNo + 1, strValue);

				// HEADER
				//strKey.Format(_T("%s"), _T("WG_LV_AFT"));
				m_sprdResult->SetColWidth(MEAS_RESULT_ICP_WG_LV_AFT, 20.0);
				m_sprdResult->SetDataString(MEAS_RESULT_ICP_WG_LV_AFT, 1, _T("WG_LV"));

				// VALUE
				GetPrivateProfileString(_T("DATA"), strKey, _T(""), szValue, MAX_PATH, strResultFile);
				str.Format(_T("%s"), szValue);
				iEnd = 0;
				iEnd = str.Find(_T(","));
				strValue = str.Mid(0, iEnd);
				m_sprdResult->SetValue(MEAS_RESULT_ICP_WG_LV_AFT, strMeasResult->m_nAreaNo + 1, strValue);

				// HEADER
				//strKey.Format(_T("%s"), _T("WG_CU_AFT"));
				m_sprdResult->SetColWidth(MEAS_RESULT_ICP_WG_CU_AFT, 20.0);
				m_sprdResult->SetValue(MEAS_RESULT_ICP_WG_CU_AFT, 1, _T("WG_CU"));

				// VALUE
				GetPrivateProfileString(_T("DATA"), strKey, _T(""), szValue, MAX_PATH, strResultFile);
				str.Format(_T("%s"), szValue);
				iEnd = 0;
				iEnd = str.Find(_T(","));
				strValue = str.Mid(0, iEnd);
				m_sprdResult->SetValue(MEAS_RESULT_ICP_WG_CU_AFT, strMeasResult->m_nAreaNo + 1, strValue);

				dTempCU = _ttof(strValue);

				// HEADER
				strKey.Format(_T("%s"), _T("WG_EF_AFT"));
				m_sprdResult->SetColWidth(MEAS_RESULT_ICP_WG_EF_AFT, 20.0);
				m_sprdResult->SetValue(MEAS_RESULT_ICP_WG_EF_AFT, 1, _T("WG_EF"));

				// VALUE
				GetPrivateProfileString(_T("DATA"), strKey, _T(""), szValue, MAX_PATH, strResultFile);
				str.Format(_T("%s"), szValue);
				iEnd = 0;
				iEnd = str.Find(_T(","));
				strValue = str.Mid(0, iEnd);

				if (dTempCU == 0.0)
					strValue = "0.0000";

				m_sprdResult->SetValue(MEAS_RESULT_ICP_WG_EF_AFT, strMeasResult->m_nAreaNo + 1, strValue);

				TCHAR* pColor3[] = { _T("W") };
				TCHAR* pData3[] = { _T("X_WAD_AFT"), _T("Y_WAD_AFT"), _T("U_DOT_WAD_AFT"), _T("V_DOT_WAD_AFT"), _T("LV_WAD_AFT"), _T("CU_WAD_AFT"), _T("EF_WAD_AFT"), };
				for (i = 0; i < 7; i++)
				{
					int j = 0;

					// HEADER
					strKey.Format(_T("%s_%s"), pColor3[j], pData3[i]);
					m_sprdResult->SetColWidth(MEAS_RESULT_ICP_CRAC_DATA_2 + (2 * i) + (j * 14), 10.0);
					m_sprdResult->SetValue(MEAS_RESULT_ICP_CRAC_DATA_2 + (2 * i) + (j * 14), 1, strKey);

					// VALUE
					GetPrivateProfileString(_T("DATA"), strKey, _T(""), szValue, MAX_PATH, strResultFile);
					str.Format(_T("%s"), szValue);

					iEnd = 0;
					iEnd = str.Find(_T(","));	//ALARM ID
					strValue = str.Mid(0, iEnd);

					if (i == 5)
					{
						dTempCU = _ttof(strValue);
					}
					else if (i == 6)
					{
						if (dTempCU == 0.0)
							strValue = "0.0000";
					}
					else if (i == 0) fRepeatData[4] = _ttof(strValue);
					else if (i == 1) fRepeatData[5] = _ttof(strValue);

					str.Delete(0, iEnd + 1);
					strJudge = str;
					strJudge.TrimRight(_T(","));

					m_sprdResult->SetValue(MEAS_RESULT_ICP_CRAC_DATA_2 + (2 * i) + (j * 14), strMeasResult->m_nAreaNo + 1, strValue);

					// HEADER
					strKey.Format(_T("%s_%s_%s"), pColor3[j], pData3[i], _T("JUDGE"));
					m_sprdResult->SetColWidth(MEAS_RESULT_ICP_CRAC_DATA_2 + (2 * i) + 1 + (j * 14), 10.0);
					m_sprdResult->SetValue(MEAS_RESULT_ICP_CRAC_DATA_2 + (2 * i) + 1 + (j * 14), 1, strKey);

					// VALUE
					m_sprdResult->SetValue(MEAS_RESULT_ICP_CRAC_DATA_2 + (2 * i) + 1 + (j * 14), strMeasResult->m_nAreaNo + 1, strJudge);
				}

				TCHAR* pColor4[] = { _T("W"), _T("R"), _T("G"), _T("B"), };
				TCHAR* pData4[] = { _T("X_WAD"), _T("Y_WAD"), _T("U_DOT_WAD"), _T("V_DOT_WAD"), _T("LV_WAD"), _T("CU_WAD"), _T("EF_WAD"), };
				for (int j = 1; j < 4; j++)
				{
					double dTempCU = 0.0; //20170109 lhj add
					for (i = 0; i < 7; i++)
					{
						// HEADER
						CString strW;
						strW.Format(_T("%s"), pColor4[j]);

						strKey.Format(_T("%s_%s"), pColor4[j], pData4[i]);
						m_sprdResult->SetColWidth(MEAS_RESULT_ICP_CRAC_DATA_2 + (2 * i) + (j * 14), 10.0);
						m_sprdResult->SetValue(MEAS_RESULT_ICP_CRAC_DATA_2 + (2 * i) + (j * 14), 1, strKey);

						// VALUE
						GetPrivateProfileString(_T("DATA"), strKey, _T(""), szValue, MAX_PATH, strResultFile);
						str.Format(_T("%s"), szValue);

						iEnd = 0;
						iEnd = str.Find(_T(","));		//ALARM ID
						strValue = str.Mid(0, iEnd);

						if (i == 5)
						{
							dTempCU = _ttof(strValue);
						}
						else if (i == 6)
						{
							if (dTempCU == 0.0)
								strValue = "0.0000";
						}
	
						// LYS 2026-07-24 WAD Multi Point (Phase 6): add color offsets (R_X, G_X, B_Y)
						if (m_nMultiPoint > 1)
						{
							if ((j == 1 && i == 0) || (j == 2 && i == 0) || (j == 3 && i == 1))
							{
								double dOffVal = _ttof(strValue);
								if (j == 1)			dOffVal += m_dMultiOffsetRX;
								else if (j == 2)	dOffVal += m_dMultiOffsetGX;
								else				dOffVal += m_dMultiOffsetBY;
								strValue.Format(_T("%.4lf"), dOffVal);
							}
						}

						str.Delete(0, iEnd + 1);
						strJudge = str;
						strJudge.TrimRight(_T(","));

						m_sprdResult->SetValue(MEAS_RESULT_ICP_CRAC_DATA_2 + (2 * i) + (j * 14), strMeasResult->m_nAreaNo + 1, strValue);

						// HEADER
						strKey.Format(_T("%s_%s_%s"), pColor4[j], pData4[i], _T("JUDGE"));
						m_sprdResult->SetColWidth(MEAS_RESULT_ICP_CRAC_DATA_2 + (2 * i) + 1 + (j * 14), 10.0);
						m_sprdResult->SetValue(MEAS_RESULT_ICP_CRAC_DATA_2 + (2 * i) + 1 + (j * 14), 1, strKey);

						// VALUE
						m_sprdResult->SetValue(MEAS_RESULT_ICP_CRAC_DATA_2 + (2 * i) + 1 + (j * 14), strMeasResult->m_nAreaNo + 1, strJudge);
					}
				}

				// HEADER
				//strKey.Format(_T("%s"), _T("WG_X_WAD_AFT"));
				m_sprdResult->SetColWidth(MEAS_RESULT_ICP_WG_x_AFT_WAD, 20.0);
				m_sprdResult->SetValue(MEAS_RESULT_ICP_WG_x_AFT_WAD, 1, _T("WG_X_WAD"));

				// VALUE
				GetPrivateProfileString(_T("DATA"), strKey, _T(""), szValue, MAX_PATH, strResultFile);
				str.Format(_T("%s"), szValue);
				iEnd = 0;
				iEnd = str.Find(_T(","));
				strValue = str.Mid(0, iEnd);
				m_sprdResult->SetValue(MEAS_RESULT_ICP_WG_x_AFT_WAD, strMeasResult->m_nAreaNo + 1, strValue);

				// HEADER
				//strKey.Format(_T("%s"), _T("WG_Y_WAD_AFT"));
				m_sprdResult->SetColWidth(MEAS_RESULT_ICP_WG_y_AFT_WAD, 20.0);
				m_sprdResult->SetValue(MEAS_RESULT_ICP_WG_y_AFT_WAD, 1, _T("WG_Y_WAD"));

				// VALUE
				GetPrivateProfileString(_T("DATA"), strKey, _T(""), szValue, MAX_PATH, strResultFile);
				str.Format(_T("%s"), szValue);
				iEnd = 0;
				iEnd = str.Find(_T(","));
				strValue = str.Mid(0, iEnd);
				m_sprdResult->SetValue(MEAS_RESULT_ICP_WG_y_AFT_WAD, strMeasResult->m_nAreaNo + 1, strValue);

				// HEADER
				//strKey.Format(_T("%s"), _T("WG_LV_WAD_AFT"));
				m_sprdResult->SetColWidth(MEAS_RESULT_ICP_WG_LV_AFT_WAD, 20.0);
				m_sprdResult->SetValue(MEAS_RESULT_ICP_WG_LV_AFT_WAD, 1, _T("WG_LV_WAD"));

				// VALUE
				GetPrivateProfileString(_T("DATA"), strKey, _T(""), szValue, MAX_PATH, strResultFile);
				str.Format(_T("%s"), szValue);
				iEnd = 0;
				iEnd = str.Find(_T(","));
				strValue = str.Mid(0, iEnd);
				m_sprdResult->SetValue(MEAS_RESULT_ICP_WG_LV_AFT_WAD, strMeasResult->m_nAreaNo + 1, strValue);

				// HEADER
				//strKey.Format(_T("%s"), _T("WG_CU_WAD_AFT"));
				m_sprdResult->SetColWidth(MEAS_RESULT_ICP_WG_CU_AFT_WAD, 20.0);
				m_sprdResult->SetValue(MEAS_RESULT_ICP_WG_CU_AFT_WAD, 1, _T("WG_CU_WAD"));

				// VALUE
				GetPrivateProfileString(_T("DATA"), strKey, _T(""), szValue, MAX_PATH, strResultFile);
				str.Format(_T("%s"), szValue);
				iEnd = 0;
				iEnd = str.Find(_T(","));
				strValue = str.Mid(0, iEnd);

				m_sprdResult->SetValue(MEAS_RESULT_ICP_WG_CU_AFT_WAD, strMeasResult->m_nAreaNo + 1, strValue);

				dTempCU = _ttof(strValue);

				// HEADER
				//strKey.Format(_T("%s"), _T("WG_EF_WAD_AFT"));
				m_sprdResult->SetColWidth(MEAS_RESULT_ICP_WG_EF_AFT_WAD, 20.0);
				m_sprdResult->SetValue(MEAS_RESULT_ICP_WG_EF_AFT_WAD, 1, _T("WG_EF_WAD"));

				// VALUE
				GetPrivateProfileString(_T("DATA"), strKey, _T(""), szValue, MAX_PATH, strResultFile);
				str.Format(_T("%s"), szValue);
				iEnd = 0;
				iEnd = str.Find(_T(","));
				strValue = str.Mid(0, iEnd);

				if (dTempCU == 0.0)
					strValue = "0.0000";

				m_sprdResult->SetValue(MEAS_RESULT_ICP_WG_EF_AFT_WAD, strMeasResult->m_nAreaNo + 1, strValue);

				TCHAR* pColor5[] = { _T("LR"), _T("LG"), _T("LB") };
				TCHAR* pData5[] = { _T("X"), _T("Y"), _T("L"), _T("CurrentData"), _T("Efficience") };
				TCHAR* pData6[] = { _T("X"), _T("Y"), _T("휘도"), _T("전류"), _T("효율") };
				int nPos = 0;

				for (int i = 0; i < 3; i++)
				{
					for (int j = 0; j < 5; j++)
					{
						// HEADER
						strKey.Format(_T("%s_%s"), pColor5[i], pData6[j]);
						m_sprdResult->SetColWidth(MEAS_RESULT_ICP_L_DATA_BEGIN + j + (i * 5), 10.0);
						m_sprdResult->SetValue(MEAS_RESULT_ICP_L_DATA_BEGIN + j + (i * 5), 1, strKey);

						// VALUE
						strKey.Format(_T("%s_%s"), pColor5[i], pData5[j]);
						GetPrivateProfileString(_T("DATA"), strKey, _T(""), szValue, MAX_PATH, strResultFile);
						str.Format(_T("%s"), szValue);

						nPos = str.Find(_T(","));
						if (nPos > 0) str = str.Mid(0, nPos);

						//20200615 add (SDC 요청으로 0값 NULL 로 표시)
						if (_ttof(str) == 0.0) str = _T("");

						m_sprdResult->SetValue(MEAS_RESULT_ICP_L_DATA_BEGIN + j + (i * 5), strMeasResult->m_nAreaNo + 1, str); 
					}
				}

				//20180327 add by Hee (특성 30도 데이터 추가)
				TCHAR* pSubWADCol[] = { _T("W_X"), _T("W_Y"), _T("W_LV"), _T("W_CU"), _T("W_EF") };

				for (int i = 0; i < 5; i++)
				{
					// HEADER
					strKey.Format(_T("%s_SUB_WAD_AFT"), pSubWADCol[i]);
					m_sprdResult->SetColWidth(MEAS_RESULT_ICP_SUB_WAD_DATA_BEGIN + i, 10.0);
					m_sprdResult->SetValue(MEAS_RESULT_ICP_SUB_WAD_DATA_BEGIN + i, 1, strKey);

					// VALUE
					GetPrivateProfileString(_T("DATA"), strKey, _T(""), szValue, MAX_PATH, strResultFile);
					str.Format(_T("%s"), szValue);

					nPos = str.Find(_T(","));
					if (nPos > 0) str = str.Mid(0, nPos);

					m_sprdResult->SetValue(MEAS_RESULT_ICP_SUB_WAD_DATA_BEGIN + i, strMeasResult->m_nAreaNo + 1, str);

					if (i == 0) fRepeatData[2] = _ttof(str);
					else if (i == 1) fRepeatData[3] = _ttof(str);
				}

				//20180419 add by Hee (특성 30도 단색 데이터 추가)
				TCHAR* pSubRGBCol1[] = { _T("R"), _T("G"), _T("B") };
				TCHAR* pSubRGBCol2[] = { _T("X"), _T("Y"), _T("U_DOT"), _T("V_DOT"), _T("LV"), _T("CU"), _T("EF") };

				for (int i = 0; i < 3; i++)
				{
					for (int j = 0; j < 7; j++)
					{
						// HEADER
						strKey.Format(_T("%s_%s_SUB_WAD"), pSubRGBCol1[i], pSubRGBCol2[j]);
						m_sprdResult->SetColWidth(MEAS_RESULT_ICP_SUB_RGB_DATA_BEGIN + j + (i * 7), 10.0);
						m_sprdResult->SetValue(MEAS_RESULT_ICP_SUB_RGB_DATA_BEGIN + j + (i * 7), 1, strKey);

						// VALUE
						GetPrivateProfileString(_T("DATA"), strKey, _T(""), szValue, MAX_PATH, strResultFile);
						str.Format(_T("%s"), szValue);

						nPos = str.Find(_T(","));
						if (nPos > 0) str = str.Mid(0, nPos);

						if (j == 5 || j == 6) str = _T("");

						m_sprdResult->SetValue(MEAS_RESULT_ICP_SUB_RGB_DATA_BEGIN + j + (i * 7), strMeasResult->m_nAreaNo + 1, str);
					}
				}

				//20180423 add by Hee (특성 데이터 추가)
				TCHAR* pMTPData1[] = { _T("W_MTP_JUDGE"), _T("W_ITERATION") };
				TCHAR* pMTPData2[] = { _T("W_MTP_JUDGE"), _T("W_iteration") };
				for (int i = 0; i < 2; i++)
				{
					// HEADER
					strKey.Format(_T("%s"), pMTPData1[i]);
					m_sprdResult->SetColWidth(MEAS_RESULT_ICP_MTP_JUDGE + i, 10.0);
					m_sprdResult->SetValue(MEAS_RESULT_ICP_MTP_JUDGE + i, 1, strKey);

					// VALUE
					strKey.Format(_T("%s"), pMTPData2[i]);
					GetPrivateProfileString(_T("DATA"), strKey, _T(""), szValue, MAX_PATH, strResultFile);
					str.Format(_T("%s"), szValue);

					nPos = str.Find(_T(","));
					if (nPos > 0) str = str.Mid(0, nPos);

					m_sprdResult->SetValue(MEAS_RESULT_ICP_MTP_JUDGE + i, strMeasResult->m_nAreaNo + 1, str);
				}

				//20200525 add by Hee (APD 데이터 추가, STAGE_POS, ELVSS1~7, OFFSET)
				// HEADER
				strKey.Format(_T("%s"), _T("STAGE_POS"));
				m_sprdResult->SetColWidth(MEAS_RESULT_ICP_STAGE, 10.0);
				m_sprdResult->SetValue(MEAS_RESULT_ICP_STAGE, 1, strKey);

				// VALUE
				GetPrivateProfileString(_T("HEADER"), strKey, _T(""), szValue, MAX_PATH, strResultFile);
				str.Format(_T("%s"), szValue);
				m_sprdResult->SetValue(MEAS_RESULT_ICP_STAGE, strMeasResult->m_nAreaNo + 1, str);


				//ELVSS_1_x ELVSS_1_y ELVSS_1_휘도 ELVSS_1_전류 ~~ ELVSS7
				TCHAR* pEvlssCol1[] = { _T("V"),_T("X"), _T("Y"), _T("휘도"), _T("전류") }; //20200615 add
				TCHAR* pEvlssCol2[] = { _T("V"),_T("x"), _T("y"), _T("휘도"), _T("전류") };

				for (int i = 0; i < 7; i++) 
				{
					for (int j = 0; j < 5; j++) 
					{
						// HEADER
						strKey.Format(_T("ELVSS_%d_%s"), i + 1, pEvlssCol1[j]);
						m_sprdResult->SetColWidth(MEAS_RESULT_ICP_ELVSS_BEGIN + j + (i * 5), 10.0);
						m_sprdResult->SetValue(MEAS_RESULT_ICP_ELVSS_BEGIN + j + (i * 5), 1, strKey);

						// VALUE
						strKey.Format(_T("ELVSS_%d_%s"), i + 1, pEvlssCol2[j]);
						GetPrivateProfileString(_T("DATA"), strKey, _T(""), szValue, MAX_PATH, strResultFile);
						str.Format(_T("%s"), szValue);

						//20200615 add (SDC 요청으로 0값 NULL 로 표시)
						if (_ttof(str) == 0.0) str = _T("");
						m_sprdResult->SetValue(MEAS_RESULT_ICP_ELVSS_BEGIN + j + (i * 5), strMeasResult->m_nAreaNo + 1, str);
					}
				}

				//RED_X_OFFSET RED_Y_OFFSET ~~ W_WAD_SUB_
				TCHAR* pOffsetCol1[] = { _T("RED"), _T("GREEN"), _T("BLUE"), _T("W"), _T("W_WAD"), _T("W_WAD_SUB") };
				TCHAR* pOffsetCol2[] = { _T("X_OFFSET"), _T("Y_OFFSET") };

				for (int i = 0; i < 6; i++)
				{
					for (int j = 0; j < 2; j++)
					{
						// HEADER
						strKey.Format(_T("%s_%s"), pOffsetCol1[i], pOffsetCol2[j]);
						m_sprdResult->SetColWidth(MEAS_RESULT_ICP_OFFSET_BEGIN + j + (i * 2), 10.0);
						m_sprdResult->SetValue(MEAS_RESULT_ICP_OFFSET_BEGIN + j + (i * 2), 1, strKey);

						// VALUE
						GetPrivateProfileString(_T("DATA"), strKey, _T(""), szValue, MAX_PATH, strResultFile);
						str.Format(_T("%s"), szValue);

						m_sprdResult->SetValue(MEAS_RESULT_ICP_OFFSET_BEGIN + j + (i * 2), strMeasResult->m_nAreaNo + 1, str);
					}
				}

				//RED_EFF_OFFSET ~
				TCHAR* pEffOffsetCol1[] = { _T("RED"), _T("GREEN"), _T("BLUE") };

				for (int i = 0; i < 3; i++)
				{
					// HEADER
					strKey.Format(_T("%s_EFF_OFFSET"), pEffOffsetCol1[i]);
					m_sprdResult->SetColWidth(MEAS_RESULT_ICP_EFF_OFFSET_BEGIN + i, 10.0);
					m_sprdResult->SetValue(MEAS_RESULT_ICP_EFF_OFFSET_BEGIN + i, 1, strKey);

					// VALUE
					GetPrivateProfileString(_T("DATA"), strKey, _T(""), szValue, MAX_PATH, strResultFile);
					str.Format(_T("%s"), szValue);

					m_sprdResult->SetValue(MEAS_RESULT_ICP_EFF_OFFSET_BEGIN + i, strMeasResult->m_nAreaNo + 1, str);
				}

				//20210518 add by Hee (특성 60도 데이터 추가)
				TCHAR* pSub2WADCol[] = { _T("W_X"), _T("W_Y"), _T("W_LV"), _T("W_CU"), _T("W_EF") };
				for (int i = 0; i < 5; i++) 
				{
					// HEADER
					strKey.Format(_T("%s_SUB2_WAD_AFT"), pSub2WADCol[i]);
					m_sprdResult->SetColWidth(MEAS_RESULT_ICP_SUB2_WAD_DATA_BEGIN + i, 10.0);
					m_sprdResult->SetValue(MEAS_RESULT_ICP_SUB2_WAD_DATA_BEGIN + i, 1, strKey);

					// VALUE
					GetPrivateProfileString(_T("DATA"), strKey, _T(""), szValue, MAX_PATH, strResultFile);
					str.Format(_T("%s"), szValue);

					nPos = str.Find(_T(","));
					if (nPos > 0) str = str.Mid(0, nPos);

					m_sprdResult->SetValue(MEAS_RESULT_ICP_SUB2_WAD_DATA_BEGIN + i, strMeasResult->m_nAreaNo + 1, str);

					if (i == 0) fRepeatData[6] = _ttof(str);
					else if (i == 1) fRepeatData[7] = _ttof(str);
				}

				//특성 60도 단색 데이터 추가
				TCHAR* pSub2RGBCol1[] = { _T("R"), _T("G"), _T("B") };
				TCHAR* pSub2RGBCol2[] = { _T("X"), _T("Y"), _T("U_DOT"), _T("V_DOT"), _T("LV"), _T("CU"), _T("EF") };
				for (int i = 0; i < 3; i++) 
				{
					for (int j = 0; j < 7; j++) 
					{
						// HEADER
						strKey.Format(_T("%s_%s_SUB2_WAD"), pSub2RGBCol1[i], pSub2RGBCol2[j]);
						m_sprdResult->SetColWidth(MEAS_RESULT_ICP_SUB2_RGB_DATA_BEGIN + j + (i * 7), 10.0);
						m_sprdResult->SetValue(MEAS_RESULT_ICP_SUB2_RGB_DATA_BEGIN + j + (i * 7), 1, strKey);

						// VALUE
						GetPrivateProfileString(_T("DATA"), strKey, _T(""), szValue, MAX_PATH, strResultFile);
						str.Format(_T("%s"), szValue);

						nPos = str.Find(_T(","));
						if (nPos > 0) str = str.Mid(0, nPos);

						if (j == 5 || j == 6) str = _T("");

						m_sprdResult->SetValue(MEAS_RESULT_ICP_SUB2_RGB_DATA_BEGIN + j + (i * 7), strMeasResult->m_nAreaNo + 1, str);
					}
				}

				//W_WAD_SUB2_X_OFFSET W_WAD_SUB2_Y_OFFSET
				TCHAR* pSub2OffsetCol1[] = { _T("X_OFFSET"), _T("Y_OFFSET") };
				for (int i = 0; i < 2; i++) 
				{
					// HEADER
					strKey.Format(_T("W_WAD_SUB2_%s"), pSub2OffsetCol1[i]);
					m_sprdResult->SetColWidth(MEAS_RESULT_ICP_SUB2_OFFSET_BEGIN + i, 10.0);
					m_sprdResult->SetValue(MEAS_RESULT_ICP_SUB2_OFFSET_BEGIN + i, 1, strKey);

					// VALUE
					GetPrivateProfileString(_T("DATA"), strKey, _T(""), szValue, MAX_PATH, strResultFile);
					str.Format(_T("%s"), szValue);

					m_sprdResult->SetValue(MEAS_RESULT_ICP_SUB2_OFFSET_BEGIN + i, strMeasResult->m_nAreaNo + 1, str);
				}
			}
		}
		else if (iType == PGM_MODE_COLOR)	// 색이상
		{
			m_sprdResult->SetSheet(3);	// Set sheet names
			m_sprdResult->SetMaxRows(strMeasResult->m_nAreaNo + 1);

			TCHAR* pKey[] = { _T("VISION_RESULT_MCR"), _T("VISION_RESULT_GRAB"), _T("VISION_RESULT_CLASS"), _T("VISION_RESULT_TIME"), _T("VISION_REJECT_NAME"), _T("VISION_REJECT_BIN_CLASS"), _T("VISION_REJECT_BIN_CODE"), };
			for (i = 0; i < 7; i++)
			{
				// HEADER
				strKey.Format(_T("%s"), pKey[i]);
				m_sprdResult->SetColWidth(MEAS_RESULT_ICP_VISION_MCR + i, 20.0);
				m_sprdResult->SetValue(MEAS_RESULT_ICP_VISION_MCR + i, 1, strKey);

				// VALUE
				GetPrivateProfileString(_T("VISION_RESULT"), strKey, _T(""), szValue, MAX_PATH, strResultFile);
				str.Format(_T("%s"), szValue);
				str.TrimRight();
				m_sprdResult->SetValue(MEAS_RESULT_ICP_VISION_MCR + i, strMeasResult->m_nAreaNo + 1, str);
			}

			TCHAR* pData[] = { _T("POC JUDGE"), _T("DETAIL ERROR NAME"), _T("W_SpecA"), _T("W_SpecB"), _T("WG_SpecA"), _T("WG_SpecB"), _T("W_MPCD MIN"), _T("W_MPCD MAX"), _T("W_CCT MIN"),_T("W_CCT MAX"), _T("WG_MPCD MIN"), _T("WG_MPCD MAX"), _T("WG_CCT MIN"),_T("WG_CCT MAX"), };
			for (i = 0; i < 14; i++)
			{
				// HEADER
				strKey.Format(_T("%s"), pData[i]);
				m_sprdResult->SetColWidth(MEAS_RESULT_ICP_VISION_DATA_1 + i, 20.0);
				m_sprdResult->SetValue(MEAS_RESULT_ICP_VISION_DATA_1 + i, 1, strKey);

				// VALUE
				GetPrivateProfileString(_T("VISION_RESULT"), strKey, _T(""), szValue, MAX_PATH, strResultFile);
				str.Format(_T("%s"), szValue);
				str.TrimRight();
				m_sprdResult->SetValue(MEAS_RESULT_ICP_VISION_DATA_1 + i, strMeasResult->m_nAreaNo + 1, str);
			}
		}
		else if (iType == PGM_MODE_ANGLE) // 시야각
		{
			//m_sprdResult->SetSheet(4);	// Set sheet names
			m_sprdResult->SetSheet(3);	// Set sheet names		// LYS 2026-07-02 안쓰는 Mode 삭제

			if(strMeasResult->m_nAreaNo + 1 > m_sprdResult->GetMaxRows())
				m_sprdResult->SetMaxRows(strMeasResult->m_nAreaNo + 1); // Row Add

			// HEADER
			strKey = _T("Number");
			m_sprdResult->SetColWidth(MEAS_RESULT_ICP_ANGLE_NO, 20.0);
			m_sprdResult->SetValue(MEAS_RESULT_ICP_ANGLE_NO, 1, strKey);

			// VALUE
			strKey.Format(_T("%d"), strMeasResult->m_nAreaNo);
			m_sprdResult->SetValue(MEAS_RESULT_ICP_ANGLE_NO, strMeasResult->m_nAreaNo + 1, strKey);

			// HEADER 
			strKey = _T("File Name");
			m_sprdResult->SetColWidth(2, 20.0);
			m_sprdResult->SetValue(2, 1, strKey);

			// VALUE
			strKey.Format(_T("%s"), strFileName);
			m_sprdResult->SetColWidth(2, 20.0);
			m_sprdResult->SetValue(2, strMeasResult->m_nAreaNo + 1, strKey);

			TCHAR* pKey[] = { _T("Inner ID"), _T("Final Result Name"), _T("Detail Result Name"), _T("ANGLE"), _T("Rotation"), _T("FOCUS") };
			for (i = 0; i < 6; i++)
			{
				// HEADER
				strKey.Format(_T("%s"), pKey[i]);
				m_sprdResult->SetColWidth(MEAS_RESULT_ICP_ANGLE_MCR + i, 20.0);
				m_sprdResult->SetValue(MEAS_RESULT_ICP_ANGLE_MCR + i, 1, strKey);

				// VALUE
				GetPrivateProfileString(_T("Final Result"), strKey, _T(""), szValue, MAX_PATH, strResultFile);
				str.Format(_T("%s"), szValue);
				str.TrimRight();
				m_sprdResult->SetValue(MEAS_RESULT_ICP_ANGLE_MCR + i, strMeasResult->m_nAreaNo + 1, str);
			}

			TCHAR* pData[] = { _T("PTN 00"), _T("PTN 01"), _T("PTN 02"), _T("PTN 03"), _T("PTN 04"), _T("PTN 05"), _T("PTN 06"), _T("PTN 07") };
			TCHAR* pHeader[] = { _T("WHITE_CAM01"), _T("RED_CAM01"), _T("GREEN_CAM01"), _T("RED2_CAM01"), _T("GREEN2_CAM01"), _T("BLUE_CAM01"), _T("GRAY_010N_CAM01"), _T("GRAY_065N_CAM01") };
			for (i = 0; i < 8; i++)
			{
				// HEADER
				strKey.Format(_T("%s"), pHeader[i]);
				m_sprdResult->SetColWidth(MEAS_RESULT_ICP_ANGLE_DATA_1 + i, 20.0);
				m_sprdResult->SetValue(MEAS_RESULT_ICP_ANGLE_DATA_1 + i, 1, strKey);

				// VALUE
				strKey.Format(_T("%s"), pData[i]);
				GetPrivateProfileString(_T("Pattern Mean"), strKey, _T(""), szValue, MAX_PATH, strResultFile);
				str.Format(_T("%s"), szValue);
				str.TrimRight();
				m_sprdResult->SetValue(MEAS_RESULT_ICP_ANGLE_DATA_1 + i, strMeasResult->m_nAreaNo + 1, str);
			}			
		}
	}

	return 1;
}

bool CDlgResult::SaveResult(bool bSaveFile, bool bRemote, CString strStart)
{
	bool bRet = false;

	int iLoop = 0;
	do
	{
		if (UploadDataDone == true)
		{
			ExportAsTotalData(true, bSaveFile, bRemote, strStart);
			bRet = true;
			break;
		}

		Sleep(100);

		iLoop++;

	} while (iLoop < 50);

	return bRet;
}

int CDlgResult::ExportAsTotalData(bool bAuto, bool bSaveFile, bool bRemote, CString strStart)
{	
	m_strStartTime = strStart;

	if (bAuto == true)
	{
		AfxBeginThread(WorkThreadExportDataProc, this);
	}
	else
	{
		TotalResultData();
	}

	return 0;
}

UINT CDlgResult::WorkThreadExportDataProc(LPVOID pParam)
{
	CDlgResult* pProc = (CDlgResult*)pParam;

	pProc->TotalResultData();

	return 1;
}

int CDlgResult::TotalResultData(void)
{
	m_bSaveResultDone = false;

	ExportAsRawDateICP();

	m_bSaveResultDone = true;

	return 0;
}

int CDlgResult::ExportAsRawDateICP(void)
{
	CString strResultPath, strResultFile, strTime, strResultPathAPD;

	// jsyoon 20170104 add Marge Sheet
	int iSheet = 0, nMaxRows = 0, nMaxCols = 0;
	int j = 2, nCols = 0;
	CString str;

	// Temp 에 Meas 결과 복사하기
	m_sprdTempResult->ClearRange(1, 1, 5000, 500);

	for (iSheet = 0; iSheet < 4; iSheet++) 
	{
		j = 3;
		nMaxCols = 0;

		m_sprdResult->SetSheet(iSheet + 1);

		do
		{
			str = m_sprdResult->GetDataString(j, 1);

			if (str.GetLength() < 1)
			{
				break;
			}

			nMaxCols++;

			j++;

		} while (1);

		if(m_sprdResult->GetMaxRows() != 0)
			nMaxRows = m_sprdResult->GetMaxRows();

		for (int iRow = 1; iRow <= nMaxRows; iRow++)
		{
			for (int iCol = 1; iCol <= nMaxCols; iCol++)
			{
				str = m_sprdResult->GetDataString(iCol, iRow);

				m_sprdTempResult->SetDataString(iCol + nCols, iRow, str);
			}
		}
		nCols += nMaxCols;
	}

	// Meas 에 Temp 결과 복사하기
	m_sprdResult->SetSheet(5);
	m_sprdResult->SetMaxCols(nCols);
	m_sprdResult->SetMaxRows(nMaxRows);
	for (int iRow = 1; iRow <= nMaxRows; iRow++)
	{
		for (int iCol = 1; iCol <= nCols; iCol++)
		{
			str = m_sprdTempResult->GetDataString(iCol, iRow);

			m_sprdResult->SetColWidth(iCol, 20.0);

			m_sprdResult->SetDataString(iCol, iRow, str);
		}
	}

//	strResultPath.Format(_T("%s%s\\%s\\%s\\"), m_strResultPath, gf_GetCurDayString(), _T("RAWDATA"), m_szGlassID);
	strResultPath.Format(_T("%s%s\\%s\\%s\\"), m_strResultPath, _T("DATA"), gf_GetCurDayString(), m_szGlassID);		// jsyoon 2022.06.24 change
	gf_MakeDir(strResultPath);
	strTime = gf_GetCurTimeString();
	strResultFile.Format(_T("%s%s_%s.xls"), strResultPath, m_szGlassID, strTime);
	ExportToExcel(strResultFile);

	strResultPathAPD.Format(_T("%s"), m_strResultPath);	//ksk 20150522 add
	ExportAsAPDICP(strResultPathAPD, strTime);

	//	if(m_pSystem->stTitle.m_nHostMode == df_HOST_ONLINE_REMOTE)	// jsyoon 20150205 add
	ExportAsMarge(strResultPath);

	return 0;
}

int CDlgResult::ExportAsAPDICP(CString strResultPath, CString strTime)
{
	CString strApdData = _T("");
	CString strApdDataPath = _T("");
	CString strApdCimPath = _T("");

	strApdData = strResultPath + _T("\\APD.txt");
	strApdDataPath = strResultPath + _T("\\APD") + _T("\\");

	CString strGlassID;
	strGlassID.Format(_T("%s"), m_szGlassID);
	if (strGlassID.IsEmpty())
	{
		strGlassID.Format(_T("ABCDEFG1234"));
	}

//	strApdCimPath.Format(_T("%s\\%s%s"), m_strCimData, strGlassID, _T("\\APD.txt"));

	strApdData.Replace(_T("\\\\"), _T("\\"));
	strApdDataPath.Replace(_T("\\\\"), _T("\\"));
//	strApdCimPath.Replace(_T("\\\\"), _T("\\"));
//	gf_MakeDir(strApdCimPath);

//	CreateDirectory(strApdDataPath, NULL);
	m_strAPDBackUpPath.Format(_T("%sAPD_%s.txt"), strApdDataPath, strTime);
	gf_MakeDir(m_strAPDBackUpPath);

	strApdData1 = m_strAPDBackUpPath;		// jsyoon 20161006 add

	ResultCalculateICP();

	CStdioFile File;
	if (!File.Open(strApdData1, CFile::modeCreate | CFile::modeWrite))
	{
		return VTM_RV_FAILED; 
	}

	CString strBody;
	File.WriteString(_T("[DATA_BEGIN]\n"));

	strBody.Format(_T("%-20s,%-20s,\n"), _T("MEAS_MODE"), GetMeasMode()); 
	File.WriteString(strBody);

	strBody.Format(_T("%-20s,%-20s,\n"), _T("LOT_ID"), m_szLotID);
	File.WriteString(strBody);

	strBody.Format(_T("%-20s,%-20s,\n"), _T("GLASS_ID"), m_szGlassID);
	File.WriteString(strBody);

	strBody.Format(_T("%-20s,%-20s,\n"), _T("RECIPE_ID"), m_szRecipeID);
	File.WriteString(strBody);

	strBody.Format(_T("%-20s,%-20s,\n"), _T("START_TIME"), m_strStartTime);
	File.WriteString(strBody);

	strBody.Format(_T("%-20s,%-20s,\n"), _T("END_TIME"), gf_GetCurTimeString(4));
	File.WriteString(strBody);


	int iAreaCnt = 0;
	int i = 0, nGroupCnt = 0;

	m_sprdResult->SetSheet(5);
	nGroupCnt = m_sprdResult->GetMaxRows();

	for (i = 2; i <= nGroupCnt; i++)
	{
		iAreaCnt++;
	}

	strBody.Format(_T("%-20s,%-20d,\n"), _T("AREA_CNT"), iAreaCnt);
	File.WriteString(strBody);

	// LYS 2026-08-18 AREA_MAX_CNT 1000 -> 2500.
	// 최대 행 = 에이징 항목 x 멀티포인트 x 글라스당 셀. 3 x 7 x 108 = 2268로 기존 한계를 넘었다.
	int iArea = 2500;
	strBody.Format(_T("%-20s,%-20d,\n"), _T("AREA_MAX_CNT"), iArea);
	File.WriteString(strBody);

	strBody.Format(_T("%-20s,%-20s,\n"), _T("GLASS_JUDGE"), _T("OK"));
	File.WriteString(strBody);

	strBody.Format(_T("%-20s,%-20d,\n"), _T("CELL_X_CNT"), m_nCellCountX);	// Cell
	File.WriteString(strBody);

	strBody.Format(_T("%-20s,%-20d,\n"), _T("CELL_Y_CNT"), m_nCellCountY);	// Stick
	File.WriteString(strBody);


	File.WriteString(_T("[STATISTICS_DATA]\n"));

	CStringArray strName, strResult;
	CString strVal, strT, strT2, strT3;
	strName.RemoveAll();

	CString strGL, strAver, strMax, strMin, strUnif;
	TCHAR* stName[] = { _T("SUM"),_T("MIN"),_T("MAX"),_T("AVG"),_T("UNIF"),_T("RNG"),_T("3STD"), };

	int iCol = 0, iRow = 0, iMaxRow = 0, iMaxCol = 0;
	iMaxCol = m_sprdResult->GetMaxCols() + 1;

	m_sprdResult->SetSheet(5);

	for (iCol = 1; iCol < iMaxCol; iCol++)				// jsyoon 20161013 change
	{
		strVal = m_sprdResult->GetDataString(iCol, 1);

		strName.Add(strVal);
	}

	for (int i = 0; i < 7; i++)
	{
		strVal.Empty();
		strT.Empty();

		for (int j = 0; j < 56; j++)
		{
			iCol = MEAS_RESULT_ICP_CRAC_DATA_1 -1 + j; // 특성 결과에 따라서 Col 초기값 세팅 필요

			if (strName.GetSize() > iCol)
				strT2.Format(_T("%s_%s"), stName[i], strName.GetAt(iCol));
			else
			{
				//strT2.Format(_T("%s_%s"), stName[i], _T(""));
				continue;
			}				

			if (j % 2)
			{
				if (i == 0)
				{
					strT3.Format(_T("%.4f"), fabs(m_dICPSTAT[i][j]));
				}
				else
					strT3.Format(_T("%.4f"), 0.0);
			}
			else
				strT3.Format(_T("%.4f"), m_dICPSTAT[i][j]);

			if(strT2.GetLength() < 20)
				strT.Format(_T("%-20s,%-20s,"), strT2, strT3);
			else
				strT.Format(_T("%s,%-20s,"), strT2, strT3);

			File.WriteString(strT);	// jsyoon 20170214 change
		}

		if(strT.IsEmpty() != true)
		File.WriteString(_T("\n"));	// jsyoon 20170214 change
	}

	File.WriteString(_T("[RAW_DATA]\n"));
	iMaxRow = m_sprdResult->GetMaxRows();

	for (iRow = 2; iRow <= iMaxRow; iRow++)
	{
		strResult.RemoveAll();

		for (iCol = 1; iCol < iMaxCol; iCol++)				// jsyoon 20161013 change
		{
			strVal = m_sprdResult->GetDataString(iCol, iRow);
			strResult.Add(strVal);
		}

		strVal.Empty();

		// strName, strResult 개수 같아야 함
		for (iCol = 0; iCol < strName.GetSize(); iCol++)
		{
			strT2.Format(_T("%s"), strName.GetAt(iCol));
			strT3.Format(_T("%s"), strResult.GetAt(iCol));

			if (strT2.IsEmpty() || strT2.Find(_T("_TIME")) > 0 || strT2.Find(_T("_IMAGE")) > 0)
				continue;

			if (strT2.GetLength() < 20)
			{
				if (strT3.GetLength() < 20)
					strT.Format(_T("%-20s,%-20s,"), strT2, strT3);
				else
					strT.Format(_T("%-20s,%s,"), strT2, strT3);
			}
			else
			{
				if (strT3.GetLength() < 20)
					strT.Format(_T("%s,%-20s,"), strT2, strT3);
				else
					strT.Format(_T("%s,%s,"), strT2, strT3);
			}

			File.WriteString(strT);
		}

		File.WriteString(_T("\n"));
	}

	File.WriteString(_T("[DATA_END]\n"));

	// 
	File.WriteString(_T("[UFFIG_BEGIN]\n"));

	for (size_t i = 0; i < m_vecVisDefecs.size(); i++)
	{
		m_vecVisDefec = m_vecVisDefecs[i];
		for (size_t j = 0; j < m_vecVisDefec.size(); j++)
		{
			strT = dfcStr(_T("%s"), m_vecVisDefec[j]);
			File.WriteString(strT);
		}
	}

	File.WriteString(_T("[UFFIG_END]\n"));


	File.WriteString(_T("[MICRO_BEGIN]\n"));

	for (size_t i = 0; i < m_vecMicroDefecs.size(); i++)
	{
		m_vecMicroDefec = m_vecMicroDefecs[i];
		for (size_t j = 0; j < m_vecMicroDefec.size(); j++)
		{
			strT = dfcStr(_T("%s"), m_vecMicroDefec[j]);
			File.WriteString(strT);
		}
	}

	File.WriteString(_T("[MICRO_END]\n"));
	
	File.Close();

	CopyFile(strApdData1, strApdData, FALSE);

	return VTM_RV_SUCCESSED;
}

int CDlgResult::ResultCalculateICP(void)
{
	CString strdSubVal;
	double dSum = 0., dAvr = 0., d3Sigma = 0., dMin = 0., dMax = 0., dUnif = 0., dVal = 0.;
	int nRowCount = 0, nResultOkCnt = 0;

	m_sprdResult->SetSheet(5);	// 결과 Sheet 선택
	nRowCount = m_sprdResult->GetMaxRows();

	for (int j = 0; j < 100; j++)
	{
		dSum = 0;
		d3Sigma = 0.00;
		dMin = 100000;
		dMax = 0;
		dAvr = 0.00;
		nResultOkCnt = 0;

		int iCol = 0;
		for (int iRow = 1; iRow < nRowCount; iRow++)
		{
			//iCol = MEAS_RESULT_ICP_CRAC_DATA_1 + j;
			iCol = MEAS_RESULT_ICP_CRAC_DATA_1 + j; // 특성 결과에 따라서 Col 초기값 세팅 필요 

			if (iCol > MEAS_RESULT_ICP_JUDGE_MTP_W)
				continue;

			strdSubVal = m_sprdResult->GetDataString(iCol,  iRow + 1);
			if (strdSubVal.IsEmpty())
				continue;

			if (TRUE)
			{
				nResultOkCnt++;

				dVal = _ttof(strdSubVal);

				dSum += dVal;
				d3Sigma += (dVal * dVal);

				if (dVal <= dMin)
				{
					dMin = dVal;
				}

				if (dVal >= dMax)
				{
					dMax = dVal;
				}
			}
		}

		if (strdSubVal.IsEmpty())
			continue;

		if (nResultOkCnt != 0)
			dAvr = dSum / nResultOkCnt;

		if (nResultOkCnt >= 2)
		{
			d3Sigma /= (double)(nResultOkCnt);
			d3Sigma = 3.0 * sqrt(d3Sigma - (dAvr * dAvr));
		}
		else
		{
			d3Sigma = 0.0;
		}

		double bSumMinMax = dMin + dMax;
		if (bSumMinMax == 0)
			dUnif = 0.0;
		else
			dUnif = ((dMax - dMin) / (dMin + dMax)) * 100;

		m_dICPSTAT[0][j] = dSum;
		m_dICPSTAT[1][j] = dMin;
		m_dICPSTAT[2][j] = dMax;
		m_dICPSTAT[3][j] = dAvr;
		m_dICPSTAT[4][j] = dUnif;
		m_dICPSTAT[5][j] = dMax - dMin;
		m_dICPSTAT[6][j] = d3Sigma;
	}

	return 0;
}

void CDlgResult::ExportAsAPDHead(CArchive& ar)
{
	CString strHead, strBody;

	ar.WriteString(_T("[DATA_BEGIN]\r\n"));

	strBody.Format(_T("%-20s,%-20s,\r\n"), _T("MEAS_MODE"), GetMeasMode());
	ar.WriteString(strBody);

	strBody.Format(_T("%-20s,%-20s,\r\n"), _T("LOT_ID"), /*m_pSystem->stLotInfo.m_szLotID*/m_szLotID);
	ar.WriteString(strBody);
	
	strBody.Format(_T("%-20s,%-20s,\r\n"), _T("GLASS_ID"), /*m_pSystem->stLotInfo.m_szGlassID*/m_szGlassID);
	ar.WriteString(strBody);

	strBody.Format(_T("%-20s,%-20s,\r\n"), _T("RECIPE_ID"), /*m_pSystem->stLotInfo.m_szRecipeID*/m_szRecipeID);
	ar.WriteString(strBody);
	
	//strBody.Format(_T("%-20s,%-20s,\r\n"), _T("START_TIME"), /*m_pSystem->sz_MeasStartTime*/);
	//ar.WriteString(strBody);
	//
	//strBody.Format(_T("%-20s,%-20s,\r\n"), _T("END_TIME"), m_pSystem->sz_MeasEndTime);
	//ar.WriteString(strBody);


	int iAreaCnt = 0;
	int i = 0, nGroupCnt = 0;
	
	m_sprdResult->SetSheet(5);
	nGroupCnt = m_sprdResult->GetMaxRows();

	for (i = 2; i <= nGroupCnt; i++)
	{
		iAreaCnt++;
	}	

	strBody.Format(_T("%-20s,%-20d,\r\n"), _T("AREA_CNT"), iAreaCnt);
	ar.WriteString(strBody);

	// LYS 2026-08-18 AREA_MAX_CNT 1000 -> 2500.
	int iArea = 2500;	
	strBody.Format(_T("%-20s,%-20d,\r\n"), _T("AREA_MAX_CNT"), iArea);
	ar.WriteString(strBody);

	//strBody.Format(_T("%-20s,%-20s,\r\n"), _T("GLASS_JUDGE"), (m_pSystem->m_bGlassJudge) ? _T("OK") : _T("NG"));
	//ar.WriteString(strBody);

	//20171027 add by Hee
	//strBody.Format(_T("%-20s,%-20d,\r\n"), _T("CELL_X_CNT"), m_pSystem->m_nCellCount_X);
	//ar.WriteString(strBody);

	//strBody.Format(_T("%-20s,%-20d,\r\n"), _T("CELL_Y_CNT"), m_pSystem->m_nCellCount_Y);
	//ar.WriteString(strBody);
	//

	//20181026 add by Hee (단변검사 적용)
	//CString strPlate;
	//if (m_pSystem->stInterLock.m_bLongPlate) strPlate = _T("L");
	//else									 strPlate = _T("S");

	//strBody.Format(_T("%-20s,%-20s,\r\n"), _T("INSPECT_TYPE"), strPlate);
	//ar.WriteString(strBody);

	ar.WriteString(_T("[STATISTICS_DATA]\r\n"));
}

CString CDlgResult::GetMeasMode()
{
	CString strRtn = _T("");
		
	char cW = 'W';
	char cT = 'T';
	char cS = 'S';
	//if (!m_pDlgNaviRecipe->m_stRecipeMeas.bUse[3]) // 화질
	//	cW = '0';
	//if (!m_pDlgNaviRecipe->m_stRecipeMeas.bUse[2]) // 특성
	//	cT = '0';
	//if (!m_pDlgNaviRecipe->m_stRecipeMeas.bUse[4]) // 색이상
	//	cS = '0';
	strRtn.Format(_T("%c%c%c"), cW, cT, cS);
		
	return strRtn;
}

void CDlgResult::ExportToExcel(CString strFile)
{
	m_sprdResult->ExportToExcel(strFile, _T("Meas Results"), _T("D:\\LOGFILE.TXT"));
}

void CDlgResult::ExportAsMarge(CString strPath)
{
	int iStartRow = 0;
	CString strResultFile;
	strResultFile.Format(_T("%s%s_Marge.csv"), strPath, gf_GetCurDayString());
	CFileFind finder;
	BOOL bNew = FALSE;

	if (finder.FindFile(strResultFile))
	{
		iStartRow = 2;
		bNew = FALSE;
	}
	else
	{
		bNew = TRUE;
		iStartRow = 1;
	}

	UINT nOpenFlags = CFile::modeReadWrite | CFile::shareDenyWrite;
	if (bNew)
		nOpenFlags |= CFile::modeCreate;

	CFile cfgFile(strResultFile, nOpenFlags);
	CArchive ar(&cfgFile, CArchive::store);
	cfgFile.SeekToEnd();

	int i = 0, iMaxCols = 0;
	int nGroupCnt = 0;

	m_sprdResult->SetSheet(5);
	nGroupCnt = m_sprdResult->GetMaxRows();
	for (i = iStartRow; i <= nGroupCnt; i++)
	{
		ar.WriteString(GetSpredtoStr2(i));
	}

	ar.Close();
	cfgFile.Flush();
	cfgFile.Close();
}

CString CDlgResult::GetSpredtoStr2(int iRow)
{
	CStringArray strResult;
	CString strVal, strT;
	int iCol = 0;

	strResult.RemoveAll();

	int iMaxCol = 0;
	iMaxCol = m_sprdResult->GetMaxCols();
	for (iCol = 1; iCol <= iMaxCol; iCol++)			// jsyoon 20161013 change
	{
		strVal = m_sprdResult->GetDataString(iCol, iRow);
		strResult.Add(strVal);
	}

	strVal.Empty();
	for (iCol = 0; iCol < strResult.GetSize(); iCol++)
	{
		strT.Format(_T("%s,"), strResult.GetAt(iCol));
		strVal += strT;
	}

	strVal += _T("\r\n");

	return strVal;
}

void CDlgResult::GetConfig()
{
	TCHAR szValue[MAX_PATH];
	ZeroMemory(szValue, MAX_PATH);

	TCHAR szConfigFile[MAX_PATH];
	_tcscpy_s(szConfigFile, _countof(szConfigFile), m_strConfigPath);

	GetPrivateProfileString(_T("DIR"), _T("ROOT"), _T("C:\\KMAC_MH"), szValue, MAX_PATH, szConfigFile);
	m_strRootPath.Format(_T("%s"), szValue);

	GetPrivateProfileString(_T("DIR"), _T("LOG"), _T("C:\\KMAC_MH"), szValue, MAX_PATH, szConfigFile);
	m_strLogPath.Format(_T("%s"), szValue);

	GetPrivateProfileString(_T("DIR"), _T("RESULT"), _T("C:\\KMAC_MH"), szValue, MAX_PATH, szConfigFile);
	m_strResultPath.Format(_T("%s"), szValue);

	GetPrivateProfileString(_T("DIR"), _T("CIM_PATH"), _T("D:\CIMDATA"), szValue, MAX_PATH, szConfigFile);
	m_strCimData.Format(_T("%s"), szValue);

	GetPrivateProfileString(_T("MEASURE"), _T("UPLOADNOMEAS"), _T("0"), szValue, 10, szConfigFile);
	m_bLoadNoMeas = _ttoi(szValue);

	gf_MakeDir(m_strLogPath);
	gf_MakeDir(m_strResultPath);
	gf_MakeDir(m_strCimData);

	//lhj add 161115 ////////////////////////////////////////////////////////////////////////////
	GetPrivateProfileString(_T("APD_OPTION"), _T("WHITE"), _T("0"), szValue, MAX_PATH, szConfigFile);
	//m_pSystem->m_nApdOpt[0] = _ttoi(szValue);
	GetPrivateProfileString(_T("APD_OPTION"), _T("RED"), _T("1"), szValue, MAX_PATH, szConfigFile);
	//m_pSystem->m_nApdOpt[1] = _ttoi(szValue);
	GetPrivateProfileString(_T("APD_OPTION"), _T("GREEN"), _T("2"), szValue, MAX_PATH, szConfigFile);
	//m_pSystem->m_nApdOpt[2] = _ttoi(szValue);
	GetPrivateProfileString(_T("APD_OPTION"), _T("BLUE"), _T("3"), szValue, MAX_PATH, szConfigFile);
	//m_pSystem->m_nApdOpt[3] = _ttoi(szValue);
	GetPrivateProfileString(_T("APD_OPTION"), _T("GRAY"), _T("5"), szValue, MAX_PATH, szConfigFile);
	//m_pSystem->m_nApdOpt[4] = _ttoi(szValue);
	GetPrivateProfileString(_T("APD_OPTION"), _T("USER_OPT"), _T("6"), szValue, MAX_PATH, szConfigFile);
	//m_pSystem->m_nApdOpt[5] = _ttoi(szValue);
	GetPrivateProfileString(_T("APD_OPTION"), _T("DEFECT_COUNT"), _T("50"), szValue, MAX_PATH, szConfigFile);
	//m_pSystem->m_nDefectCnt = _ttoi(szValue);
	/////////////////////////////////////////////////////////////////////////////////////////////
}

void CDlgResult::DataClear()
{
	m_vecVisDefecs.clear();
	m_vecMicroDefecs.clear();

	for (int iSheet = 0; iSheet < 5; iSheet++)
	{
		m_sprdResult->SetSheet(iSheet + 1);
		
		m_sprdResult->SetMaxRows(0);
	}
}