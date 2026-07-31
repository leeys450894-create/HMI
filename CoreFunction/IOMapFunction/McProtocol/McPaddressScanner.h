#pragma once

#include "McAddressScannerImpl.h"

class CMcPaddressScanner : public CMcAddressScannerImpl
{
	long				m_lMcProtocolReadData[dfPACCADDRESS_TOTAL_COUNT];
	long				m_lMcProtocolReadData_clone[dfPACCADDRESS_TOTAL_COUNT];
	SADDRESSMAPDATA		m_vecRData;
	SADDRESSMAPDATA		m_vecWData;
	CString				m_strQueueMessage;
	bool				m_bUseBit;
	bool m_bThreadRunStop;
	CString m_strOldData;
	bool SendCommand(bool bUseBit = true);


	//------------------------------------------------------------------
	// 공통 인터페이스 함수
	//------------------------------------------------------------------
public:
	virtual long Initialize(bool bRunMode, CString strIP, long nPort)		override;
	virtual long Close()									override;
	virtual bool IsConnected()								override;
	virtual void DisConnect()								override;
	virtual void ThreadRun()								override; 
	virtual bool ReadFileCSV(CString strFilename)			override;
	virtual bool ReadFileXLS(CString strFileName)			override; 
	virtual bool ReadFileVerify(CString strFileName)		override;
	virtual bool SetDefaultFile(CString strFileName)		override;
	virtual bool CheckActionDone(long addr, DWORD dwTimeout, bool val = true) override;

	virtual bool SetMemoryBit(short addr, long val, long nBit = 1)		override;
	virtual bool SetMemoryBit(CString strAddr, long val, long nBit = 1) override;
	virtual long GetAddressImportCount()					override; 
	virtual long GetAddressExportCount()					override; 
	virtual bool GetAddressImportDataValue(UINT Index)		override;
	virtual bool GetAddressExportDataValue(UINT Index)		override;
	virtual SADDRESSMAPDATA *GetAddressImportDataPtr()		override;
	virtual SADDRESSMAPDATA *GetAddressExportDataPtr()		override;
	virtual void xNetCallbackRecieveData(LPVOID ud)			override;
	virtual std::vector<CString> GetCommand(CString& strData)		override;


public:
	//CMcPaddressScanner(CString strDefaultFilePath, CMcProtocolCtrl *pMcProtocolCtrl);
	CMcPaddressScanner(CString strLoggerFolderPath, CString strDefaultFilePath);
	virtual ~CMcPaddressScanner();
};

