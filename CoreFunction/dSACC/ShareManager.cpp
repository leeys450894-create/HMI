// ShareManager.cpp : 구현 파일입니다.
//

#include "pch.h"
//#include "dShareControl.h"
#include "ShareManager.h"

// CShareManager
#define METROSYS_RESULT_MEMMAP_BUFFER_NAME_NEW _T("sNEW_METROSYS_RESULT_MEMMAP_BUFFER_NAME")
#define METROSYS_RESULT_MEMMAP_AREA_NAME_NEW   _T("sNEW_METROSYS_RESULT_MEMMAP_AREA_NAME")
#define METROSYS_RESULT_MEMMAP_MUTEX_NAME_NEW  _T("sNEW_METROSYS_RESULT_MEMMAP_MUTEX_NAME")
#define METROSYS_RESULT_MEMMAP_MONITER_NEW     _T("sNEW_METROSYS_RESULT_MEMMAP_MONITER__NAME")

HANDLE g_hMutex=NULL;

bool srMutexCreate()
{
	if(g_hMutex != NULL) return true;
	g_hMutex = CreateMutex(NULL ,FALSE,METROSYS_RESULT_MEMMAP_MUTEX_NAME_NEW);
	if(g_hMutex==NULL) return false;
	return true;
}
bool srMutexClose()
{
	if(g_hMutex == NULL) return true;
	CloseHandle(g_hMutex);
	g_hMutex = NULL;
	return true;
}
bool srMutexLock()
{
	if(g_hMutex == NULL) return false;
	WaitForSingleObject(g_hMutex,INFINITE);
	return true;
}
bool srMutexUnlock()
{
	if(g_hMutex == NULL) return false;
	ReleaseMutex(g_hMutex);
	return true;
}

// 공유메모리 생성
LPVOID AllocateSPIMemory(HANDLE *pMemHandle, size_t nByte, TCHAR * cMemoryMapName, bool *bRet)
{
	LPVOID pMem;
	bool   bCRet = false;

	*pMemHandle = ::OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, cMemoryMapName);
	if(*pMemHandle==NULL) 
	{
		*pMemHandle = ::CreateFileMapping((HANDLE)0xFFFFFFFF, NULL, PAGE_READWRITE,0, nByte, cMemoryMapName);
		if(GetLastError()==ERROR_ALREADY_EXISTS) bCRet=false;
		else bCRet=true;
	}
	else
	{
		bCRet=true;
	}

	if(*pMemHandle==NULL) 
	{  
		*bRet = false;
		return NULL;
	}
	pMem = ::MapViewOfFile(*pMemHandle,FILE_MAP_ALL_ACCESS,0, 0, nByte);
	if(pMem == NULL) 
	{  
		*bRet = false;
		return NULL;
	}

	*bRet = bCRet;
	return pMem;
}

LPVOID AllocateMemory(HANDLE *pMemHandle, size_t nByte, TCHAR * cMemoryMapName, bool *bRet)
{
	LPVOID pMem;
	bool   bCRet = false;
	*pMemHandle = ::OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, cMemoryMapName);
	if(*pMemHandle==NULL) 
	{
		*pMemHandle = ::CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE,0, nByte, cMemoryMapName);
		if(GetLastError()==ERROR_ALREADY_EXISTS) bCRet=false;
		else bCRet=true;
	}
	else
	{
		bCRet=true;
	}

// 	*pMemHandle = ::CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE,0, nByte, cMemoryMapName);
// 	if(GetLastError()==ERROR_ALREADY_EXISTS) bCRet=FALSE;
// 	else bCRet=TRUE;

	if(*pMemHandle==NULL) 
	{  
		*bRet = false;
		return NULL;
	}
	pMem = ::MapViewOfFile(*pMemHandle,FILE_MAP_ALL_ACCESS,0, 0, nByte);
	if(pMem == NULL) 
	{  
		*bRet = false;
		return NULL;
	}

	*bRet = bCRet;
	return pMem;
}

// 공유메모리 해제
void FreeSPIMemory(HANDLE *pMemHandle, LPVOID pParam)
{
	if(pParam==NULL && *pMemHandle==NULL) return;

	if(pParam!=NULL )
	{
		UnmapViewOfFile(pParam);
		pParam      = NULL;
	}
	
	if(*pMemHandle!=NULL)
	{
		CloseHandle(*pMemHandle);
		*pMemHandle = NULL;	
	}
}



IMPLEMENT_DYNAMIC(CShareManager, CWnd)

CShareManager::CShareManager()
: m_bStop(false)
, m_bInit(false)
{
	hMem_pBuffer = NULL;
	hMem_nBuffer = NULL;
	hMem_nMoniter = NULL;
	m_pRMonitorThread	= NULL;
	nBuffer = NULL;
	pMoniter = NULL;

	ZeroMemory(&m_RData, sizeof(RITEMDATA));	
}

CShareManager::~CShareManager()
{
	CloseShareMemory();
}


BEGIN_MESSAGE_MAP(CShareManager, CWnd)
END_MESSAGE_MAP()



// CShareManager 메시지 처리기입니다.

int CShareManager::InitializeReport(HWND hParentWnd)
{
	// Only call for regular MFC DLL's
#ifndef _AFXDLL
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
#endif

	m_hParentWnd = hParentWnd;

	return 0;
}


int CShareManager::FinalizeReport(void)
{
#ifndef _AFXDLL
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
#endif

	SetResultMonitoring(FALSE);

	return 0;
}


int CShareManager::UploadData(PRITEMDATA result)
{
	return Upload(result);
}

int CShareManager::SetHostMode(int iMode)
{
	SetSearchMode(iMode);

	return 0;
}


int CShareManager::SetSearchMode(int iMode)
{
	InitShareMemory();

	if( iMode == 1 )
	{
		m_bStop = false;
		m_hEventStop	= CreateEvent(NULL, TRUE, TRUE, NULL);
		m_hEventResume	= CreateEvent(NULL, TRUE, FALSE, NULL);
		m_pRMonitorThread = AfxBeginThread(CBResultMonitorProc, this);

		int nRes = GetNBuffer();

		SetResultMonitoring(TRUE);
	}

	return 0;
}


UINT CShareManager::CBResultMonitorProc(LPVOID pParam) 
{
	CShareManager* me = (CShareManager*)pParam;
	me->CBResultMonitorBody();
	return 0;
}

void CShareManager::CBResultMonitorBody() 
{
	while(!m_bStop)
	{
		DWORD dwResult = WaitForSingleObject(m_hEventStop, 100);
		switch(dwResult)
		{
		case WAIT_OBJECT_0 :
			// jsyoon 2022.09.28 change
		//	WaitForSingleObject(m_hEventResume, INFINITE);
			m_bStop = true; 
			break;

		case WAIT_TIMEOUT :
			{
				int nCnt = 0, i=0;
				nCnt = GetNBuffer();		// 공유메모리에 쌓인 개수확인
				if(nCnt > 0)
				{
					for(i = 0; i < nCnt; i++)
					{
						Download(&m_RData);		// 공유메모리에서 가져오기

						// 메시지를 보내자
						::SendMessage(m_hParentWnd/*this->GetParent()->m_hWnd*/, WM_CONTRIL_DOWN_EVENT, (WPARAM)&m_RData, 0);
					}
				}
			}
			break;
		}
	}
}

// 쓰레드 동작 
void CShareManager::SetResultMonitoring(BOOL bSet) 
{
	if(m_pRMonitorThread == NULL) return;

	if(bSet)
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

int CShareManager::InitShareMemory(void)
{
	m_bInit = false;
	bool bRet  = true;
	bool bRet2 = true;

	// Monitoring
	{
		pMoniter = NULL;

		TCHAR * sMoniter_nBuffer = (TCHAR *) malloc((_tcslen(METROSYS_RESULT_MEMMAP_MONITER_NEW)+9)*sizeof(TCHAR));
		_stprintf(sMoniter_nBuffer,_T("%s_nBuffer"),METROSYS_RESULT_MEMMAP_MONITER_NEW);

		int     nSize_nBuffer2   = sizeof(DITEMDATA);
		bool    bCreate_nBuffer2 = false;

		pMoniter = (PDITEMDATA) AllocateMemory(&hMem_nMoniter, nSize_nBuffer2, sMoniter_nBuffer,&bCreate_nBuffer2);
		free(sMoniter_nBuffer);
		if(pMoniter == NULL)
		{
			return false;
		}
	}

	// Buffer
	TCHAR * sName = (TCHAR *) malloc((_tcslen(METROSYS_RESULT_MEMMAP_AREA_NAME_NEW)+3)*sizeof(TCHAR));
	for(int i=0;i<RITEM_BUFFER_MAX;i++)
	{
		_stprintf(sName,_T("%s_%d"),METROSYS_RESULT_MEMMAP_AREA_NAME_NEW, i);

		pBuffer[i] = NULL;

		int     nSize_nBuffer   = sizeof(RITEMDATA);
		bool    bCreate_nBuffer = false;

		pBuffer[i] = (RITEMDATA *)AllocateMemory(&hMem_pBuffer, nSize_nBuffer, sName,&bCreate_nBuffer);
		if(pBuffer[i]==NULL) 
		{
			_stprintf(sErrMsg,_T("Cannot init memory.."));
		}
		bRet = bRet && pBuffer[i]!=NULL ? true:false;

		_stprintf(sName,_T(""));
	}
	free(sName);
	
	// Buffer Count
	nBuffer = NULL;

	TCHAR * sMMFName_nBuffer = (TCHAR *) malloc((_tcslen(METROSYS_RESULT_MEMMAP_BUFFER_NAME_NEW)+9)*sizeof(TCHAR));
	_stprintf(sMMFName_nBuffer,_T("%s_nBuffer"),METROSYS_RESULT_MEMMAP_BUFFER_NAME_NEW);

	int     nSize_nBuffer   = sizeof(int);
	bool    bCreate_nBuffer = false;

	nBuffer = (int *)AllocateMemory(&hMem_nBuffer, nSize_nBuffer, sMMFName_nBuffer,&bCreate_nBuffer);
	free(sMMFName_nBuffer);
	if(nBuffer == NULL)
	{
		return false;
	}
	if(bCreate_nBuffer) *nBuffer    = 0;

	bRet2 = srMutexCreate();
	if(!bRet2) _stprintf(sErrMsg,_T("Cannot Create mutex"));

	m_bInit = bRet && bRet2;

	return bRet && bRet2;
}

int	 CShareManager::GetNBuffer()
{
	if(!m_bInit) return 0;
	return (*nBuffer);
}

TCHAR *CShareManager::GetErrMsg()
{
	return sErrMsg;
}

bool CShareManager::ClearAll()
{
	(*nBuffer) = 0;
	return true;
}

bool CShareManager::Upload  (PRITEMDATA pArea)
{
	if(!m_bInit) 
	{
		_stprintf(sErrMsg,_T("Cannot init memory.."));
		return false;
	}

	srMutexLock();

	if( *nBuffer == RITEM_BUFFER_MAX)
	{
		for(int i=1;i<(*nBuffer); i++)
		{
			memcpy(pBuffer[i-1],pBuffer[i],sizeof(RITEMDATA));
		}

		(*nBuffer) --;
	}

	if((*nBuffer)<RITEM_BUFFER_MAX)
	{
		memcpy(pBuffer[(*nBuffer)],pArea,sizeof(RITEMDATA));
		(*nBuffer) ++;
		srMutexUnlock();
		_stprintf(sErrMsg,_T(""));
		return true;
	}

	srMutexUnlock();
	_stprintf(sErrMsg,_T("Buffer Full!!"));
	return false;

}

bool CShareManager::Download(PRITEMDATA pArea, bool bJustMonitoring/*=false*/, int iIdx/*=0*/)
{
	if(!m_bInit) {_stprintf(sErrMsg,_T("Cannot init memory.."));return false;}

	srMutexLock();

	if(bJustMonitoring)
	{
		if(iIdx<(*nBuffer))
		{
			memcpy(pArea, pBuffer[(*nBuffer)],sizeof(RITEMDATA));
			srMutexUnlock();
			_stprintf(sErrMsg,_T(""));
			return true;
		}
		else
		{
			_stprintf(sErrMsg,_T("Idx err.."));
			return false;
		}
	}
	else
	{
		if((*nBuffer) != 0)
		{
			memcpy(pArea, pBuffer[(0)],sizeof(RITEMDATA));

			for(int i=1;i<(*nBuffer); i++)
			{
				memcpy(pBuffer[i-1],pBuffer[i],sizeof(RITEMDATA));
			}
			(*nBuffer) --;

			srMutexUnlock();
			_stprintf(sErrMsg,_T(""));
			return true;
		}
	}

	srMutexUnlock();
	_stprintf(sErrMsg,_T("Buffer empty.."));
	return false;
}

int CShareManager::DownloadData(PDITEMDATA data)
{
	if( pMoniter != NULL)
	{
		memcpy(data, pMoniter, sizeof(DITEMDATA));
	}

	return 0;
}

int CShareManager::Updata(PDITEMDATA data)
{
	if( pMoniter != NULL)
	{
		memcpy(pMoniter, data, sizeof(DITEMDATA));
	}
	return 0;
}


void CShareManager::CloseShareMemory(void)
{
	for(int i=0;i<RITEM_BUFFER_MAX;i++)
	{
		FreeSPIMemory(&hMem_pBuffer, pBuffer[i]);
	}

	FreeSPIMemory(&hMem_nBuffer, nBuffer);

	FreeSPIMemory(&hMem_nMoniter, pMoniter);
}
