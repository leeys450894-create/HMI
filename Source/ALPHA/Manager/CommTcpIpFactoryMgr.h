#pragma once

#include "..\AuroraSubMgr.h"
#include "..\AuroraMgrPool.h"
#include "CommTcpIpDefine.h"

class CCommTcpIpObjectUTG;
class CCommTcpIpObjectVision;	// 화질
class CCommTcpIpObjectColor;	// 색이상
class CCommTcpIpObjectWAD;		// WAD
class CCommTcpIpObjectAngle;	// 시야각
class CCommTcpIpObjectPG;		// PG
class CCommTcpIpObjectAgingPG;	// Aging PG
class CCommTcpIpObjectLCR;		// LCR
class CCommTcpIpObjectTSP;		// TSP
class CCommTcpIpObjectCIM;		// CIM
class CCommEthFunction;
class CCommTcpIpObjectSysTime;
class CCommTcpIpObjectMotion;
class CCommTcpIpObjectPGMon;	// LYS 2026-07-29 ELP ElecBox monitor (doc 2026-07-28)
class CCommTcpIpFactoryMgr : public CAuroraSubMgr
{
	stAuroraMgrPool	m_MgrPool_link;

	// tcp/ip 쪽
	CCommTcpIpObjectUTG		*m_pCommTcpIpObjUTG1;
	CCommTcpIpObjectUTG		*m_pCommTcpIpObjUTG2;

	CCommTcpIpObjectVision* m_pCommTcpIpObj_Vision[2];
	CCommTcpIpObjectColor*	m_pCommTcpIpObj_Color[2];
	CCommTcpIpObjectAngle*	m_pCommTcpIpObj_Angle[2];
	CCommTcpIpObjectWAD*	m_pCommTcpIpObj_WAD;
	CCommTcpIpObjectPG*		m_pCommTcpIpObj_PG;
	CCommTcpIpObjectAgingPG* m_pCommTcpIpObj_AgingPG;
	CCommTcpIpObjectLCR*	m_pCommTcpIpObj_LCR;
//	CCommTcpIpObjectTSP*	m_pCommTcpIpObj_TSP;		// jsyoon 2022.10.19 change
	CCommTcpIpObjectTSP*	m_pCommTcpIpObj_TSP[6];		// 6개로 접속

	CCommTcpIpObjectCIM*	m_pCommTcpIpObj_CIM;
	CCommTcpIpObjectSysTime* m_pCommTcpIpObj_SysTime;
	CCommTcpIpObjectMotion* m_pCommTcpIpObj_VisMotion;
	CCommTcpIpObjectPGMon*	m_pCommTcpIpObj_PGMon;	// LYS 2026-07-29 ELP ElecBox monitor


public:
	CCommTcpIpObjectUTG		*GetCommTcpIpObjectUTG1()	{ return m_pCommTcpIpObjUTG1;	}
	CCommTcpIpObjectUTG		*GetCommTcpIpObjectUTG2()	{ return m_pCommTcpIpObjUTG2;   }

	// LYS 2026-08-25 Network.ini의 DEVICEID가 배열 인덱스로 들어온다.
	// 범위 밖이면 nullptr를 반환해 설정 오타가 배열 밖을 읽지 못하게 한다.
	CCommTcpIpObjectVision* GetCommTcpIpObjectVision(int i) { return (0 <= i && i < (int)_countof(m_pCommTcpIpObj_Vision)) ? m_pCommTcpIpObj_Vision[i] : nullptr; }
	CCommTcpIpObjectColor* GetCommTcpIpObjectColor(int i) { return (0 <= i && i < (int)_countof(m_pCommTcpIpObj_Color)) ? m_pCommTcpIpObj_Color[i] : nullptr; }
	CCommTcpIpObjectAngle* GetCommTcpIpObjectAngle(int i) { return (0 <= i && i < (int)_countof(m_pCommTcpIpObj_Angle)) ? m_pCommTcpIpObj_Angle[i] : nullptr; }
	CCommTcpIpObjectWAD* GetCommTcpIpObjectWAD() { return m_pCommTcpIpObj_WAD; }
	CCommTcpIpObjectPG* GetCommTcpIpObjectPG() { return m_pCommTcpIpObj_PG; }
	CCommTcpIpObjectAgingPG* GetCommTcpIpObjectAGING() { return m_pCommTcpIpObj_AgingPG; }
	CCommTcpIpObjectLCR* GetCommTcpIpObjectLCR() { return m_pCommTcpIpObj_LCR; }
//	CCommTcpIpObjectTSP* GetCommTcpIpObjectTSP() { return m_pCommTcpIpObj_TSP; }	// jsyoon 2022.10.19 change
	CCommTcpIpObjectTSP* GetCommTcpIpObjectTSP(int i) { return (0 <= i && i < (int)_countof(m_pCommTcpIpObj_TSP)) ? m_pCommTcpIpObj_TSP[i] : nullptr; }
	CCommTcpIpObjectCIM* GetCommTcpIpObjectCIM() { return m_pCommTcpIpObj_CIM; }
	CCommTcpIpObjectSysTime* GetCommTcpIpObjectSystime() { return m_pCommTcpIpObj_SysTime; }
	CCommTcpIpObjectMotion* GetCommTcpIpObjectVisMotion() { return m_pCommTcpIpObj_VisMotion; }
	CCommTcpIpObjectPGMon* GetCommTcpIpObjectPGMon() { return m_pCommTcpIpObj_PGMon; }	// LYS 2026-07-29


private:
	void CreateComm(long nID);
	void DeleteComm(long nID);



//------------------------------------------------------------------
// 공통 인터페이스 함수
//------------------------------------------------------------------
private:
	virtual void Finalize() override;
public:
	CCommTcpIpFactoryMgr(stAuroraMgrPool &MgrPool);
	virtual ~CCommTcpIpFactoryMgr();

	long DoWorkSetParameter(long lMeasType);
	long DoWorkRecipeOpen(long lMeasType);
	long DoWorkRecipeOpenComplate(long lMeasType, DWORD dwTimeOut = 10);
	//long DoWorkInspectStart(long lMeasType, int iMeasMode = 0, int nCellNum = 0, bool bSideOut = true);  // iMeasMode 0: Head 1,2, 1:Head1, 2:Head2
	long DoWorkInspectStart(long lMeasType, int iMeasMode = 0, int nCellNum = 0, bool bSideOut = true, bool bWadRHeadUse = false, bool bWait = true);  // iMeasMode 0: Head 1,2, 1:Head1, 2:Head2 //wadheaduse : Right Head 로 Left 특성 진행
	long DoWorkInspectStartWait(long lMeasType, int iMeasMode = 0);
	long DoWorkInspectEnd(long lMeasType, int iMeasMode = 0, DWORD dwTimeOut = DEFINE_INSPECITON_WAIT_TIME);
	long DoWorkInspectComplate(long lMeasType, int iMeasMode = 0, DWORD dwTimeOut = DEFINE_INSPECITON_WAIT_TIME, int nRetry = 1); // iMeasMode 0: Head 1,2, 1:Head1, 2:Head2
	void SetOutOfService(long nID, bool bOn);
	void SetTcpConnection(long nID, bool bConnect);
	CLogFileAccessor *GetLogFileAccessorPtr(long nID);
	bool IsOutOfService(long nID);
	int GetCount();
	CString GetParam_Name(long nID);
	CString GetParam_IPNumber(long nID);
	long GetParam_PortNumger(long nID);
	bool IsConnected(long nID);
	void SetMeasPos(long lMeasType, double dx, double dy1, double dy2);
	BOOL CmQualityEmptyTimeOut(BOOL bLast, int nTimeOut);
	int CmQualityAlarm();
	long DoWorkVisionCmd(int nItem, BOOL bQualityAutoSet = FALSE, int nHead = -1);
	void SetMeasUseInfo(bool *bUseMeas, int *nAreaNum, int nTimeOut);
	BOOL CmFileFind(CString strFile);
	BOOL CmFileFindTimeOut(CString strFile, int nTimeOut);
	void DisConnect();
	bool IsUsed(long n1ID);

public:
	bool m_bUseMeas[DF_MAX_HEAD];
	int  m_nAreaNum[DF_MAX_HEAD];
	int  m_nTimeOut;
};

