#pragma once

#include "McAddressScannerImpl.h"

class CMcCaddressScanner : public CMcAddressScannerImpl
{
	long				m_lMcProtocolReadData[dfPROBEADDRESS_TOTAL_COUNT];
	long				m_lMcProtocolReadData_clone[dfPROBEADDRESS_TOTAL_COUNT];
	SADDRESSMAPDATA		m_vecRData;
	SADDRESSMAPDATA		m_vecAData;
	int					m_iDataLast;
	CString				m_strQueueMessage;
	HANDLE				m_hRecvDEvent;

	TCHAR	m_szLoadProbeCardID[10][32];	// Cassette
	TCHAR	m_szProbeCardBF[2][32];			// Buffer
	TCHAR	m_szUnloadProbeCardID[10][32];	// EQ ProbeID

	bool ReadCommand(long addr, long nCount);
	bool ReadProbeLoadData();
	bool ReadProbeBufferData();
	bool ReadProbeUnloadData();

	bool SendCommand();
	int m_nReadType;
	void UpdateProbeID(CString strCommand, int iType);


	//------------------------------------------------------------------
	// 공통 인터페이스 함수
	//------------------------------------------------------------------
public:
	virtual long Initialize(bool bRunMode, CString strIP, long nPort)		override;
	virtual long Close()									override;
	virtual bool IsConnected()								override;
	virtual void DisConnect()								override;
	virtual void ThreadRun()								override;
	virtual bool ReadFileCSV(CString strFileName)			override;
	virtual bool ReadFileXLS(CString strFileName)			override; 
	virtual bool ReadFileVerify(CString strFileName)		override;
	virtual bool SetDefaultFile(CString strFileName)		override;
	virtual long GetAddressImportCount()					override; 
	virtual long GetAddressExportCount()					override; 
	virtual bool GetAddressImportDataValue(UINT Index)		override;
	virtual void GetAddressExportDataValueArray(TCHAR** pValue, long* plSize) override;
	//virtual bool GetAddressExportDataValue(UINT Index)	override;
	virtual SADDRESSMAPDATA *GetAddressImportDataPtr()		override;
	//virtual SADDRESSMAPDATA *GetAddressExportDataPtr()	override;
	virtual void xNetCallbackRecieveData(LPVOID ud)			override;
	virtual std::vector<CString> GetCommand(CString& strData)		override;
	virtual bool SetMemoryWord(long addr, float* val, long nWord = 1)	override;

public:
	long GetAlarmMapCount();
	bool IsOccurFAlarm();
	SADDRESSMAPDATA	&GetPROBEAlarmMapData() { return m_vecAData; }

public:
	//CMcCaddressScanner(CString strDefaultFilePath, CMcProtocolCtrl *pMcProtocolCtrl);
	CMcCaddressScanner(CString strLoggerFolderPath, CString strDefaultFilePath);
	virtual ~CMcCaddressScanner();
};

