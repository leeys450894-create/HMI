#pragma once
#include "CommEthFunction\CommEthFunction.h"
#include ".\Manager\CommTcpIpDefine.h"
#include "AuroraMgrPool.h"

// LYS 2026-07-29 ELP ElecBox monitor protocol
// Doc: ElecBoxProtocol_SDC HMI(OCP) 2026-07-28 (based on A6 8.6G GAI 24.08.20)
// ELP pushes S10F01(status) / S10F20(alarm), HMI replies S20F01 / S20F20 ACK.

#define eMON_S10F1		_T("1001")		// ElecBox temp / smoke / power report
#define eMON_S10F20		_T("1020")		// ElecBox alarm report (max 10ea)

#define DF_ELPMON_BOX_MAX		2		// ElecBox index 01~02 (doc error code table)
#define DF_ELPMON_DATA_CNT		9		// temp, smoke, 7 power values
#define DF_ELPMON_BODY_LEN		80		// S10F01 Len field value
#define DF_ELPMON_ALARM_MAX		10		// S10F20 max alarm count per frame

// S10F01 body layout: 80 ASCII chars (doc 2.2.1)
typedef struct {
	TCHAR	szIndex[2];			// ElecBox Index. Ex) 01 -> ElecBox #1
	TCHAR	szTemp[4];			// ElecBox temp. Ex) 1234 -> 123.4 C
	TCHAR	szSmoke[4];			// ElecBox smoke density. Ex) 99 -> 99 %
	TCHAR	szVolt[10];			// voltage. Ex) 12345 -> 12.345 V
	TCHAR	szHarmVolt[10];		// harmonic voltage. Ex) 12345 -> 12.345 V
	TCHAR	szCurr[10];			// current. Ex) 12345 -> 12.345 A
	TCHAR	szHarmCurr[10];		// harmonic current. Ex) 12345 -> 12.345 mA
	TCHAR	szLeakCurr[10];		// leakage current. Ex) 12345 -> 12.345 mA
	TCHAR	szInstPwr[10];		// instantaneous power. Ex) 12345 -> 12.345 kW
	TCHAR	szAccumPwr[10];		// accumulated power. Ex) 12345 -> 12345 kWh
} ELPMONPROTOCOL_M, * PELPMONPROTOCOL_M;

// m_dElpData[box] item index
enum enumElpMonData
{
	enumElpMonData_TEMP = 0,	// C
	enumElpMonData_SMOKE,		// %
	enumElpMonData_VOLT,		// V
	enumElpMonData_HARM_VOLT,	// V
	enumElpMonData_CURR,		// A
	enumElpMonData_HARM_CURR,	// mA
	enumElpMonData_LEAK_CURR,	// mA
	enumElpMonData_INST_PWR,	// kW
	enumElpMonData_ACCUM_PWR,	// kWh
};

class CLogFileAccessor;
class CCommTcpIpObjectPGMon :
	public CCommEthFunction
{
	CLogFileAccessor*	m_pLogFileAccessor;
	CSystemConfigMgr*	m_pSystemConfigMgr;
	bool				m_bOutOfService;

	CCriticalSection	m_cs_Data;
	double				m_dElpData[DF_ELPMON_BOX_MAX][DF_ELPMON_DATA_CNT];
	bool				m_bDataValid[DF_ELPMON_BOX_MAX];
	DWORD				m_dwLastRecvTick;

public:
	bool IsOutOfService() { return m_bOutOfService; };
	void SetOutOfService(bool bOn) { m_bOutOfService = bOn; };
	double GetElpData(int iBox, int iItem);
	bool IsElpDataValid(int iBox);
	DWORD GetLastRecvTick() { return m_dwLastRecvTick; };
	CLogFileAccessor* GetLogFileAccessorPtr();

	//=========================================================================
	// INTERFACE
	//=========================================================================
private:
	virtual void xNetCallbackRecieveData(LPVOID ud) override;
	void ParseS10F01(const CString& strCmd);
	void ParseS10F20(const CString& strCmd);
	void SendAck(int iFunc, int iAck, int iCause);
	bool IsValidAlarmCode(int iCode);
	static UINT ThreadRaiseAlarm(LPVOID pParam);

public:
	CCommTcpIpObjectPGMon(int iSeqItemIndex, stAuroraMgrPool& MgrPool);
	virtual ~CCommTcpIpObjectPGMon();

};
