#pragma once

#include "ShareMem.h"
#include <afxmt.h>

#define IDS_STAGE_STAGE_NAME            1
#define IDS_STAGE_EQUIP_STATE           2
#define IDS_STAGE_PRESENCE              3
#define IDS_STAGE_PLACEMENT             4
#define IDS_STAGE_CLAMPED               5
#define IDS_STAGE_LOADED                6
#define IDS_STAGE_OPENED                7
#define IDS_STAGE_MAPPED                8
#define IDS_STAGE_PROTRUSION            9
#define IDS_STAGE_ACCESS_MODE           10
#define IDS_STAGE_EXIST                 11
#define IDS_STAGE_PROCESSTYPE           12
#define IDS_STAGE_LOADTYPE              13
#define IDS_STAGE_TYPE                  14
#define IDS_STAGE_CARRIER_ID            15
#define IDS_STAGE_BCR_COMPLETED         16
#define IDS_STAGE_SOUR_STAGE            17
#define IDS_STAGE_DEST_STAGE            18
#define IDS_STAGE_RECIPE_ID             19
#define IDS_STAGE_WAFER_LIMIT           20
#define IDS_STAGE_LOADED_TIME           21
#define IDS_STAGE_RESERVED_AS_SOUR      22
#define IDS_STAGE_RESERVED_AS_DEST      23
#define IDS_STAGE_OPC                   24
#define IDS_STAGE_ORDER                 25
#define IDS_SLOT_SLOT_NO                26
#define IDS_SLOT_STATE                  27
#define IDS_SLOT_CLAMPED                28
#define IDS_SLOT_EXIST                  29
#define IDS_SLOT_TYPE                   30
#define IDS_SLOT_GRADE                  31
#define IDS_SLOT_WID                    32
#define IDS_SLOT_LMID                   33
#define IDS_SLOT_READ_M12               34
#define IDS_SLOT_READ_T7                35
#define IDS_SLOT_OCR_ANGLE              36
#define IDS_SLOT_ALIGN_ANGLE            37
#define IDS_SLOT_OCR_COMPLETED          38
#define IDS_SLOT_SOUR_STAGE             39
#define IDS_SLOT_SOUR_SLOT              40
#define IDS_SLOT_SOUR_CID               41
#define IDS_SLOT_DEST_STAGE             42
#define IDS_SLOT_DEST_SLOT              43
#define IDS_SLOT_DEST_CID               44
#define IDS_SLOT_RECIPE_ID              45
#define IDS_SLOT_LOADED_TIME            46
#define IDS_SLOT_JOB_ID                 47
#define IDS_SLOT_JOB_COMPLETED          48
#define IDS_SLOT_RESERVED_AS_SOUR       49
#define IDS_SLOT_RESERVED_AS_DEST       50

// 순서 변경하지 말것.
static int StageItem[] =   {
	IDS_STAGE_STAGE_NAME     ,
	IDS_STAGE_EQUIP_STATE    ,
	IDS_STAGE_PRESENCE       ,
	IDS_STAGE_PLACEMENT      ,
	IDS_STAGE_CLAMPED        ,
	IDS_STAGE_LOADED         ,
	IDS_STAGE_OPENED         ,
	IDS_STAGE_MAPPED         ,
	IDS_STAGE_PROTRUSION     ,
	IDS_STAGE_ACCESS_MODE    ,
	IDS_STAGE_EXIST          ,
	IDS_STAGE_PROCESSTYPE	 ,
	IDS_STAGE_LOADTYPE		 ,
	IDS_STAGE_TYPE           ,
	IDS_STAGE_CARRIER_ID     ,
	IDS_STAGE_BCR_COMPLETED  ,
	IDS_STAGE_SOUR_STAGE     ,
	IDS_STAGE_DEST_STAGE     ,
	IDS_STAGE_RECIPE_ID      ,
	IDS_STAGE_WAFER_LIMIT    ,
	IDS_STAGE_LOADED_TIME    ,
	IDS_STAGE_RESERVED_AS_SOUR,
	IDS_STAGE_RESERVED_AS_DEST,
};

static int SlotItem[] =   {
	IDS_SLOT_SLOT_NO         ,
	IDS_SLOT_STATE			 ,
	IDS_SLOT_EXIST           ,
	IDS_SLOT_TYPE            ,
	IDS_SLOT_WID			 ,
	IDS_SLOT_LMID            ,
	IDS_SLOT_READ_M12        ,
	IDS_SLOT_READ_T7         ,
	IDS_SLOT_OCR_ANGLE		 ,
	IDS_SLOT_ALIGN_ANGLE     ,
	IDS_SLOT_OCR_COMPLETED   ,
	IDS_SLOT_SOUR_STAGE      ,
	IDS_SLOT_SOUR_SLOT       ,
	IDS_SLOT_SOUR_CID        ,
	IDS_SLOT_DEST_STAGE      ,
	IDS_SLOT_DEST_SLOT       ,
	IDS_SLOT_DEST_CID        ,
	IDS_SLOT_RECIPE_ID       ,
	IDS_SLOT_LOADED_TIME     ,
	IDS_SLOT_JOB_ID          ,
	IDS_SLOT_JOB_COMPLETED   ,
	IDS_SLOT_RESERVED_AS_SOUR,
	IDS_SLOT_RESERVED_AS_DEST,
};

class CShareMemFunction
{
	CCriticalSection m_cs;
	HANDLE		m_hVTMSysMap;
    VTM_SYSTEM *m_pVTMSystem;

public:
    VTM_SYSTEM *GetSharedMemory();
    HWND		GetMetrosysViewWnd();
    void		SetMetrosysViewWnd(HWND hWnd);
    void		SetSys_AlarmStatus(int nState);
    int			GetSys_AlarmStatus();
    void		SetSys_ProcessStatus(int nState);
    int			GetSys_ProcessStatus();
    void		SetSys_UserPermit(char *szPermit);
    CString		GetSys_UserPermit();
    void		SetSys_User(char *szUser);
    CString		GetSys_User();
    int			GetSys_UserNo();
    void		SetSys_UserNo(int nNoUser);
    void		SetSys_MachineMode(int nMode);
    int			GetSys_MachineMode();
    void		SetSys_MetrosysVersion(char *szMetrosysVersion);
    CString		GetSys_MetrosysVersion();
	CString		GetSys_AppPath();
	void		SetSys_MachineType(int nType);
	int			GetSys_MachineType();
	void		SetSys_ProbeChange(int nType);
	int			GetSys_ProbeChange();

public :
    CShareMemFunction();
    ~CShareMemFunction();
};
