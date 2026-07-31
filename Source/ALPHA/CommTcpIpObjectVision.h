#pragma once
#include "CommEthFunction\CommEthFunction.h"
#include ".\Manager\CommTcpIpDefine.h"
#include "AuroraMgrPool.h"

#define QUALITY_1		0
#define QUALITY_2		1
#define QUALITY_MAX		2

struct S_QualityWait {
	BOOL	m_bWaitStatus;
	BOOL	m_bRunStatus;
	CString m_strInnerID;
	CString m_strCellID;

	int		m_nSheetRow; //Data 출력 위치

	S_QualityWait() {
		Clear();
	}

	void Clear() {
		m_bWaitStatus = FALSE;
		m_bRunStatus = FALSE;
		m_strInnerID = _T("");
		m_strCellID = _T("");
		m_nSheetRow = -1;
	}
};

class CRecipeData;
class CLogFileAccessor;
class CShareMemoryMgr;
class CCommTcpIpObjectVision :
    public CCommEthFunction
{
	CLogFileAccessor* m_pLogFileAccessor;
	CSystemConfigMgr* m_pSystemConfigMgr;
	CRecipeEditMgr* m_pRecipeEditMgr;
	CResultCollectMgr* m_pResultCollectMgr;
	CRecipeData* m_pRecipeDataClone;
	CShareMemoryMgr* m_pShareMemory;
	CMaintControlMgr* m_pMaintControlMgr;
	CCommTcpIpFactoryMgr* m_pCommTcpIpFactoryMgr;
	CString					m_strQueueMessage;
	HANDLE					m_hRecvDEvent[enumRecvEventIDCount];
	bool					m_bOutOfService;
	int WriteRecipeFile();
	int	CopyRecipeFile();

public:
	void UpdateRecipeInformation();
	bool SetTime_Sync();
	bool RecipeChange();
	bool SetParameter();
	bool InspectStart(int iMeasType = 0, bool bWait = true, int iStepNum = 0);
	bool InspectStartMulti(int iMeasType, int nArea);
	bool InspectStartWait();
	bool InspectEnd();
	bool WaitRecipeChangeEnd(DWORD dwTimeOut);
	bool WaitInspectionEnd(DWORD dwTimeOut);
	bool WaitInspectionComplete(DWORD dwTimeOut);
	CLogFileAccessor* GetLogFileAccessorPtr();
	bool IsOutOfService() { return m_bOutOfService; };
	void SetOutOfService(bool bOn) { m_bOutOfService = bOn; };


	//=========================================================================
	// INTERFACE
	//=========================================================================
private:
	virtual void xNetCallbackRecieveData(LPVOID ud) override;

public:
	CCommTcpIpObjectVision(int iSeqItemIndex, stAuroraMgrPool& MgrPool);
	virtual ~CCommTcpIpObjectVision();
	int m_iAlarmIndex;
	int m_iDeviceIndex;
	int SetRunToThread(int iPattern);
	int m_iPattern;
	static UINT ThreadRun(LPVOID pParam);
	void RunMoveFilter();
	////////////////////////////////
	//화질검사 결과 자동 처리 (2개 버퍼 사용)
public:
	BOOL	m_bQualityWait;
	int		m_nHead;
	CString m_strNetDrive;
	CString m_strNetRcpDrive;

protected:
	S_QualityWait	sQualityWait[QUALITY_MAX];
	CWinThread*		m_pWaitThread;
	BOOL			m_bRunThread;
	//BOOL			m_nQualityDelay; //화질 파일 읽기전 딜레이

protected:
	static UINT QualityWaitThread(LPVOID pParam);

public:
	BOOL QualityBufferEmpty();
	BOOL QualityBufferEmptyAll();
	void QualityWaitSet(CString strInnerID, int nSheetRow, CString strCellID);
	void QualityWaitRun(CString strInnerID);
	void QualityWaitUse(BOOL bSet) { m_bQualityWait = bSet; };
	void QualityWaitClear();
	//BOOL CmFileFind(CString strFile);
	//BOOL CmFileFindTimeOut(CString strFile, int nTimeOut);
	BOOL TossResultFile(CString strInnerID, int nArea);
};

