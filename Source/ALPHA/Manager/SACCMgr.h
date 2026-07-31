#pragma once
#include "..\Include\API_SACC.h"

class CIniHelper;
class CSACCMgr
{
private:
	CIniHelper*		m_pIniHelper;
	bool			m_bUse;				// 장치 사용여부, UI상에서도 보이지 않게 처리함. OutOfService와는 다름
	bool			m_bOutOfService;	// 모듈 사용 여부 0: USE

	CString		m_strProbeCardID[5];
	CString		m_strProbeCardIDTime[5];
	UINT		m_nUseCnt[5];
	bool		m_bSaccRun;

public:
	bool	IsOutOfService();
	void    Initialize(HWND hParentWnd, CRect rtROI, CString strAppName, CString strAppPath, CString strConfigPath, CString strLogPath);
	void    InitializeProbeLog(HWND hParentWnd, CRect rtROI, CString strAppName, CString strAppPath, CString strConfigPath, CString strLogPath);
	CWnd*	GetReportmWnd();
	CWnd*	GetProbeLogWnd();
	void	Updata_Data();
	void	OnControlMsgReceive(WPARAM wParam, LPARAM lParam);
	void	Probe_Mount(int iProbe, CString strRFID, CString strMountTime );
	void	Probe_ContactCount(int iProbe, CString strRFID, CString strMountTime, int nContactCount);
	void	Probe_Unmount(int iProbe, CString strRFID, CString strMountTime, CString strUnmountTime);
	void	Probe_GetInfo(int iProbe, CString* strRFID, CString* strMountTime, int* nContactCount);
	bool    Is_Complete();
	bool    Is_Fail();
	bool    Is_AlarmOn();

	//----------------------------------------------------------------------------------------------------
	// Singleton
	//----------------------------------------------------------------------------------------------------
private:
	static CSACCMgr* m_pInstance;
public:
	static CSACCMgr* GetInstance();
	static void		DelInstance();

//------------------------------------------------------------------
// 공통 인터페이스 함수
//------------------------------------------------------------------
private:
	bool LinkIniFile(bool bSave);
	void Finalize();
	CSACCMgr(void);
	virtual ~CSACCMgr(void);
};

