
#ifndef __SHAREDMEMORYDEFINE_H__
#define __SHAREDMEMORYDEFINE_H__
#pragma once

#define	MONITORING_SHARED_MEMORY_STRING_1 "KMAC SHARED MEMORY_1"	//20140729 changed by Jay Hong "Monitoring_Shared_Memory"--> "KMAC SHARED MEMORY"
#define	MONITORING_SHARED_MEMORY_STRING_2 "KMAC SHARED MEMORY_2"
#define	MONITORING_SHARED_MEMORY_STRING_3 "KMAC SHARED MEMORY_3"
#define	MONITORING_SHARED_MEMORY_STRING_4 "KMAC SHARED MEMORY_4"
#define	MONITORING_SHARED_MEMORY_STRING_5 "KMAC SHARED MEMORY_5"

#define MAX_PLC_M 1000
#define MAX_PLC_F 400
#define MAX_FFU	160		//20181207 edhan change 32 to 160
#define MAX_HEAD 8

//PLC Data
typedef struct  {
	
	BOOL bPLCData_M[MAX_PLC_M];
	BOOL bPLCData_F[MAX_PLC_F];
	
} PLC_MON;

//FFU
typedef struct  {
	
	int iEDUID[MAX_FFU];
	int iPV[MAX_FFU];
	int iALARM[MAX_FFU];
	int iSV[MAX_FFU];
	BOOL bAlarm;
	BOOL bAllOff;
	BOOL bRestore;
	int nFFU;
	int strTime;
	int nFFUSWNum;		//20181207 edhan add

	
} FFU_MON;

//Head Position
typedef struct  {

	int iHeadNum;
	int iPositionX;
	int iPositionY;
	int iPositionZ;
	int iPositionSubX;

} HEAD_MON;

typedef struct  {

	PLC_MON stPLCMon;
	FFU_MON stFFUMon;
	HEAD_MON stHeadMon[MAX_HEAD];

} SHARED_MEMORTY, *LPSHARED_MEMORTY;


#define NORMAL_ALARM 128 //0x80
#define EC_ALARM 160 //0xA0
#define MOTOR_ALARM 192 //0xC0
#define COMM_ALARM 0 //0x00

#endif