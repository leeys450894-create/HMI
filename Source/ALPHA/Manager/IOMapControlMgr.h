#pragma once

#include "..\AuroraSubMgr.h"
#include <afxmt.h>
#include "IOMapAddressDefine.h"

class CIniHelper;
class CLogFileAccessor;
class CIOFunction;
class CIOMapControlMgr : public CAuroraSubMgr
{
    CIniHelper				*m_pIniHelper;
	CLogFileAccessor		*m_pLogFileAccessor;
	bool					m_bUse;				// 장치 사용여부, UI상에서도 보이지 않게 처리함. OutOfService와는 다름
	bool					m_bOutOfService;	// 모듈 사용 여부 0: USE
	CString					m_strControlType;
	CIOFunction				*m_pFunction;
	bool					m_bInitialized;		// 초기화 상태 여부
	CCriticalSection		m_cs;
	CCriticalSection		m_cs_get[4];
	CCriticalSection		m_cs_set[4];

	int* m_piPLCMAdr;

public:
	bool ReadAdressFileCSV(CString strFilename);
    CString	GetDeviceName();
    bool	InitDevice(short index = 0);
	void	SetInitialize(bool value);
	bool	IsInitialized() const;
    bool	IsConnected() const;
	bool	IsUseDevice() const;
	bool	IsOutOfService() const;

	// address data value 관련 - teach, 알람에서 사용한다.
	int		GetParam_DIOMapAddressTotalCount();
	int		GetParam_DIOMapAddressBitCount(int nType = 0);
	CString GetParam_DIOMapAddressBitImport(int index);
	CString GetParam_DIOMapAddressBitImportName(int index);
	int		GetParam_DIOMapMemoryBitImport(int index);
	bool	GetParam_DIOMapMemoryBitImport2(int index);
	void    GetParam_DIOMapMemoryWordImportArray(int index, TCHAR** pValue, long* plSize);

	CString GetParam_DIOMapAddressBitExport(int index);
	CString GetParam_DIOMapAddressBitExportName(int index);
	int		GetParam_DIOMapMemoryBitExport(int index);
	bool	GetParam_DIOMapMemoryBitExport2(int index);
	int		GetParam_DIOMapAddressWordCount(int nType = 0);
	CString GetParam_DIOMapAddressWordImport(int index);
	CString GetParam_DIOMapAddressWordImportName(int index);
	long	GetParam_DIOMapMemoryWordImport(int index);
	CString GetParam_DIOMapMemoryWordImportString(int index);
	CString GetParam_DIOMapAddressWordExport(int index);
	CString GetParam_DIOMapAddressWordExportName(int index);
	long	GetParam_DIOMapMemoryWordExport(int index);
	CString GetParam_DIOMapMemoryWordExportString(int index);

	// address data value 관련 - inline
	bool	SetMemoryBitToDevice_Sub(int index, bool value);
	bool	SetMemoryBitToDevice(int index, bool value);
	bool	SetMemoryBitToDevice(CString address, bool value);
	bool	SetMemoryNBitToDevice(int index, long value, long lSize);
	bool	SetMemoryNBitToDevice(CString address, long value, long lSize);
	bool	SetMemoryWordToDevice(int index, float value);
	bool	SetMemoryWordToDevice(CString address, float value);
	bool	SetMemoryNWordToDevice(int index, float* value, long lSize);
	bool	SetMemoryNWordToDevice(CString address, float* value, long lSize);
	bool	SetMemoryNStrToDevice(int index, TCHAR* value, long lSize);
	bool	SetMemoryNStrToDevice(CString address, TCHAR* value, long lSize);
	bool	CheckActionDone(int index, DWORD dwTimeout, bool value = true);
	bool	ClearLinkSignalBit();
	bool	ClearLinkSignalWord();
	bool	ClearLinkSignalAll();

	CLogFileAccessor *GetLogFileAccessorPtr();


//------------------------------------------------------------------
// 공통 인터페이스 함수
//------------------------------------------------------------------
public:
	virtual bool LinkIniFile(bool bSave) override;
//private:
	virtual void Finalize() override;
public:
	CIOMapControlMgr(CString strAppPath, CString strConfigPath, CString strLogPath);
	virtual ~CIOMapControlMgr();

	bool	SetSinalTowerAlarm();
	bool	SetSinalTowerIdle();
	bool	SetSinalTowerRun();
	bool	SetSinalTowerWait();

	bool	SetPickertoStage();
	bool	SetStagetoPicker();
	bool	SetStageVaccumOn();
	bool	SetStageVaccumOff();

	int		GetMoveInterlock();
	void	DisConnect();
};

