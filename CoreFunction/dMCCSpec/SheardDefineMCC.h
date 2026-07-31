#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//#include "pch.h"

#define		VTM_SHARE_MEM_STRING_MCC	_T("VTM_MCC_Shared_Memory_KMAC")

typedef struct  {
	BOOL	bUse;
	BOOL	bEnable;
	BOOL	bLimitP;	// +
	BOOL	bLimitN;	// -
	BOOL	bHomeDone;
	double	dSetAxisSpeed;	// Set Speed
	double	dAxisSpeed;		// Current Speed
	double  dSetAxisPos;	// Set position
	double	dAxisPos;		// Current position
	double	dAxisRMS;		// ºÎÇÏ·ü
}MCC_MOTIONSTATUS, *PMCC_MOTIONSTATUS;

typedef struct {
	TCHAR szEquipID[260];
	TCHAR szEquipToName[260];
	TCHAR szEquipFromName[260];
	TCHAR szModuleID[260];

	TCHAR szStepID[32];
	TCHAR szLotID[32];
	TCHAR szGlassID[32];
	TCHAR szSlotID[32];
	TCHAR szRecipeID[32];
}MCC_EQINFO, *PMCC_EQINFO;

typedef struct  {
	// A/S/E/W Log Type
	int iEvtFlag;			// 0:A, 1:S, 2:E, 3:W
	TCHAR m_szEvtTime[64];	// Event Time
	int iSpqNum[10];		// NO
	int iStepNum[10];		// Meas Step
	int iFromto[10];		// FROM=TO
	BOOL bFlagStoE[10];		// Start to End	

	// I Log Type
	MCC_MOTIONSTATUS stMoterInfo[100];

	int nCntILog;			// ILOG Count		// ICP: 17, ICE: 31 ICM: OCP: 
	double dData[100];

}MCC_EQDATA, *PMCC_EQDATA;

// sih 20180201 add 
typedef struct  {
	BOOL	   bMCCSimulationModeOn;
	UINT	   uMCCActionLogPeriod;
	UINT	   uMCCSignalLogPeriod;
;
}MCC_OPTION, *PMCC_OPTION;
//

//typedef struct  {
//	UINT	   nEQType;
//	UINT	   uMCCUploadTime;
//	BOOL	   bMCCTransfer;
//	MCC_EQINFO stMCCInfoEQ;
//	MCC_EQDATA stMCCDataEQ;
//	MCC_OPTION stMCCOption;		// sih 20180201 add 
//}VTM_MCC, *PVTM_MCC;

// jsyoon 2022.02.24 change
// MCC Action
typedef struct {
	// A/S/E/W Log Type
	int iEvtFlag;			// 0:A, 1:S, 2:E, 3:W
	TCHAR m_szEvtTime[64];	// Event Time
	int iSpqNum;		// NO
	int iStepNum;		// Meas Step
	int iFromto;		// FROM=TO
	BOOL bFlagStoE;		// Start to End	

}MCC_EQDATA_T, * PMCC_EQDATA_T;

// MCC Info
typedef struct {
	// I Log Type
	MCC_MOTIONSTATUS stMoterInfo[100];

	int nCntILog;			// ILOG Count		// ICP: 17, ICE: 31 ICM: OCP: 
	double dData[100];

}MCC_EQDATA_I, * PMCC_EQDATA_I;

#define MAX_QUEUE_COUNT 10000

typedef struct {
	UINT	   nEQType;
	UINT	   uMCCUploadTime;
	BOOL	   bMCCTransfer;
	MCC_EQINFO stMCCInfoEQ;		// RECIPE Info
//	MCC_EQDATA stMCCDataEQ;

	MCC_EQDATA_I stMCCDataEQ_ILog;

	int			m_front;
	int			m_rear;
	MCC_EQDATA_T stMCCDataEQ;	// Main Upload
	MCC_EQDATA_T m_queue_data[MAX_QUEUE_COUNT];	// Queue

	MCC_OPTION stMCCOption;		// sih 20180201 add 
}VTM_MCC, * PVTM_MCC;