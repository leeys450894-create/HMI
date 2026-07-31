#pragma once

#include "McAddressScannerImpl.h"

class CMcFaddressScanner : public CMcAddressScannerImpl
{
	long				m_lMcProtocolReadData[dfFADDRESS_TOTAL_COUNT];
	long				m_lMcProtocolReadData_clone[dfFADDRESS_TOTAL_COUNT];
	SADDRESSMAPDATA		m_vecRData;
	SADDRESSMAPDATA		m_vecAData;
	int					m_iDataLast;
	CString				m_strQueueMessage;
	bool				m_bUseBit;

	bool SendCommand();

	bool m_bThreadRunStop;

	CString m_strOldData;

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
	//virtual bool GetAddressExportDataValue(UINT Index)	override;
	virtual SADDRESSMAPDATA *GetAddressImportDataPtr()		override;
	//virtual SADDRESSMAPDATA *GetAddressExportDataPtr()	override;
	virtual long GetAddressAlarmCount()						override;
	virtual bool IsOccur_addressAlarm()						override;
	virtual SADDRESSMAPDATA	*GetAddressAlarmDataPtr()		override;
	virtual void xNetCallbackRecieveData(LPVOID ud)			override;
	virtual std::vector<CString> GetCommand(CString &strData)		override;

public:
	//CMcFaddressScanner(CString strDefaultFilePath, CMcProtocolCtrl *pMcProtocolCtrl);
	CMcFaddressScanner(CString strLoggerFolderPath, CString strDefaultFilePath);
	virtual ~CMcFaddressScanner();
};

