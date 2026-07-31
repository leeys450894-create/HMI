#pragma once

#include "../DataStructure.h"
#include "../../CommEthFunction/CommEthFunction.h"

inline BOOL MakeDir(CString sPath)
{
	if (sPath == _T(""))
	{
		return FALSE;
	}

	if (sPath[1] != ':' || sPath[2] != '\\')
	{
		return FALSE;
	}

	int nFind = 2;
	while ((nFind = sPath.Find(_T("\\"), nFind + 1)) >= 0)
	{
		CString sTem = sPath.Left(nFind);
		CreateDirectory(sTem, NULL);
	}

	CreateDirectory(sPath, NULL);

	return TRUE;
}


class CMcProtocolCtrl;
class CLogFileAccessor;
class CMcAddressScannerImpl : public CCommEthFunction
{
protected:
	CLogFileAccessor	*m_pLogFileAccessor;
	CString				m_strAddressName;
	CString				m_strDefaultFilePath;

	HANDLE				m_hWaitTimeEvent;

private:
	CWinThread	*m_pThread;
	bool		m_bThreadLive;
	HANDLE		m_hStartEvent;
	HANDLE		m_hCloseEvent;
	static UINT ThreadRun(LPVOID p1);
public:
	void Start();

	unsigned char HexToByte(char hex);
	int hextoi(CString strHex);



	//------------------------------------------------------------------
	// 공통 인터페이스 함수
	//------------------------------------------------------------------
public:
	virtual long Initialize(bool bRunMode, CString strIP, long nPort);
	virtual long Close();
	virtual bool IsConnected();
	virtual void DisConnect();
	virtual void ThreadRun();
	virtual bool ReadFileCSV(CString strFilename);
	virtual bool ReadFileXLS(CString strFileName);
	virtual bool ReadFileVerify(CString strFileName);
	virtual bool SetDefaultFile(CString strFileName);
	virtual bool SetMemoryBit(short addr, long val, long lBit = 1);
	virtual bool SetMemoryBit(CString strAddr, long val, long lBit = 1);
	virtual bool SetMemoryWord(long addr, float* val, long nWord = 1);
	virtual bool SetMemoryStr(long addr, TCHAR* val, long nWord = 1);
	virtual bool CheckActionDone(long addr, DWORD dwTimeout, bool val = true);
	virtual long GetAddressImportCount();
	virtual long GetAddressExportCount();
	virtual bool GetAddressImportDataValue(UINT Index);
	virtual void GetParam_DIOMapMemoryWordExportArray(TCHAR** pValue, long* plSize);
	virtual CString GetAddressImportDataValueString(UINT Index);
	virtual bool GetAddressExportDataValue(UINT Index);
	virtual CString GetAddressExportDataValueString(UINT Index);
	virtual void GetAddressExportDataValueArray(TCHAR** pValue, long* plSize);
	virtual SADDRESSMAPDATA *GetAddressImportDataPtr();
	virtual SADDRESSMAPDATA *GetAddressExportDataPtr();
	virtual long GetAddressAlarmCount();
	virtual bool IsOccur_addressAlarm();
	virtual SADDRESSMAPDATA	*GetAddressAlarmDataPtr();
	virtual std::vector<CString> GetCommand(CString &strData);

public:
	CMcAddressScannerImpl(CString strDefaultPath);
	virtual ~CMcAddressScannerImpl();
};

