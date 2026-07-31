#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "stdafx.h"

#define dfDEFAULT_PLOG_PATH					"PLog\\"
#define dfDEFAULT_CLOG_PATH					"CLog\\"
#define dfDEFAULT_ILOG_PATH					"ILog\\"
#define dfDEFAULT_CONFIG_FILE_PATH			"FDC_Config\\Config_FDC.ini"
#define dfDEFAULT_FDCLIST_PATH				"FDC_LIST\\"
#define dfDEFAULT_FDCLIST_FILE_PATH			"FDC_LIST.xlsx"

#define PLC_D_WRITE_STARY_BIT						10000
#define PLC_D_READ_STARY_BIT						11500	
#define PLC_M_READ_START_BIT						4000

//최대 갯수 맞춰야함
#define dfMAXNUM_PCTEMP						50		
#define dfMAXNUM_EFU						64
#define dfMAXNUM_IONIZER					8		// jsyoon 20200526 change 4 to 8
#define dfMAXNUM_KMN2						2
#define dfMAXNUM_ACCURA						10	
#define dfMAXNUM_H2O						10
#define dfMAXNUM_ESD						10
#define dfMAXNUM_AZBIL						5
#define dfMAXNUM_PM5500						20

enum index_type_t {
	dfStart_H20 = 0,
	dfStart_ESD    = dfMAXNUM_H2O,
	dfStart_AZBIL  = dfStart_ESD   + dfMAXNUM_ESD,
	dfStart_PM5500 = dfStart_AZBIL + dfMAXNUM_AZBIL,
};

#define dfMAXACCURA_ITEM		44

//source Address와 맞춰야함. 
enum serial_start_index_type_t {
// 	serial_pc_temp1_start			= 1,
// 	serial_efu_start				= 3,
// 	serial_ionizer_connect_start	= 9,
// 	serial_ionizer_time_start		= 13,
// 	serial_StaticElectric_start		= 20,
// 	serial_KMN2Result_start			= 24,
	serial_pc_temp1_start			= 1,
	serial_efu_start				= serial_pc_temp1_start			+dfMAXNUM_PCTEMP,
	serial_ionizer_connect_start	= serial_efu_start				+dfMAXNUM_EFU,
	serial_ionizer_time_start		= serial_ionizer_connect_start	+dfMAXNUM_IONIZER,
	serial_StaticElectric_start		= serial_ionizer_time_start		+dfMAXNUM_IONIZER,
	serial_KMN2Result_start			= serial_StaticElectric_start	+1,
	serial_ACCURA_start				= serial_KMN2Result_start		+dfMAXNUM_KMN2,
	serial_H2O_start				= serial_ACCURA_start			+dfMAXNUM_ACCURA*dfMAXACCURA_ITEM, // (20 * 2 + 4),
	serial_ESD_START				= serial_H2O_start				+dfMAXNUM_H2O,
	serial_AZBIL_START				= serial_ESD_START				+dfMAXNUM_ESD,
	serial_PM5500_START				= serial_AZBIL_START			+dfMAXNUM_AZBIL,
	serial_TEMP_START				,
	serial_End						,
};

#define dfCOLOR_WHITE				COLORREF(RGB(255,255,255))
#define dfCOLOR_BLACK				COLORREF(RGB(0,0,0))
#define dfCOLOR_BLUE				COLORREF(RGB(0,0,255))
#define dfCOLOR_GRAY				COLORREF(RGB(192,192,192))	
#define dfCOLOR_RED					COLORREF(RGB(255,0,0))	
#define dfCOLOR_GREEN				COLORREF(RGB(0,255,0))	
#define dfCOLOR_YELLOW				COLORREF(RGB(255,255,0))	
#define dfCOLOR_VIOLET				COLORREF(RGB(186,157,187))	
#define dfCOLOR_LIGHTYELLOW			COLORREF(RGB(244,240,166))	
#define dfCOLOR_LIGHTGREEN			COLORREF(RGB(181,255,181))
#define dfCOLOR_NAVY_BLUE			COLORREF(RGB(0, 0, 128))
#define dfCOLOR_STEEL_BLUE			COLORREF(RGB(99, 184, 255))
#define dfCOLOR_LIGHT_GREY			COLORREF(RGB(211, 211, 211))
#define dfCOLOR_ORANGE_RED			COLORREF(RGB(255, 69, 0))
#define dfCOLOR_DARK_GREEN			COLORREF(RGB(0, 100, 0))
#define dfCOLORREF_RESERVE_SOUR		COLORREF(RGB(128,0,255))
#define dfCOLORREF_RESERVE_DEST		COLORREF(RGB(148,255,40))
#define dfCOLORREF_RESERVE_SELECT	COLORREF(RGB(255,128,192))
#define dfCOLOR_NONE				COLORREF(RGB(238,238,255))

typedef struct  
{
	double			dMetroVer;		//Main에서 처리	
	BOOL			bCimVer;		//cim에서 처리

}PROGRMAVER, *PPROGRMAVER;

typedef struct
{
	int				nMetroHDD_C;	//Main에서 처리
	int				nMetroHDD_D;	//Main에서 처리
	int				nMetroHDD_E;	//Main에서 처리		
	int				nCimHDD_C;		//cim에서 처리
	int				nCimHDD_D;		//cim에서 처리
}HDD, *PHDD;


typedef struct{
	double			dMainCDAAir1;	//FDC모듈에서 처리
	double			dMainCDAAir2;	//FDC모듈에서 처리
}CDAAIR, *PCDAAIR;

typedef struct{
	double			dMainVacuum1;	//FDC모듈에서 처리
	double			dMainVacuum2;	//FDC모듈에서 처리
	double			dMainVacuum3;	//FDC모듈에서 처리
	double			dMainVacuum4;	//FDC모듈에서 처리
}MAINVACUUM, *PMAINVACUUM;

typedef struct
{
	DWORD  lLiveCheck;									
	double dTemper[dfMAXNUM_PCTEMP];					
	double dEFU[dfMAXNUM_EFU];							

	// Ionizer
	BOOL	bIonizerConnect[dfMAXNUM_IONIZER];			
	BOOL	bIonizerAlarm[dfMAXNUM_IONIZER];			
	double  dIonizerTime[dfMAXNUM_IONIZER];				

	double dKMN2Result[2];								

	double dStaticElectric;		

	double dTmp[100];

}FFU, *PFFU;


typedef struct
{
	float Vavg_ll_1;    //삼상의 선간전압 평균
	float lavg_1;	    //삼상의 전류 평균
	float Ptot_1;	    //삼상의 유효전력 종합
	int	  Net_1;		//수전 유효전력량과 송전 유효전력량의 차

	int nSubDev;
	int N2_1;		    //N2_1 수치
	int N2_1_Total;		//N2_1 총합수치
	int N2_2;		    //N2_2 수치
	int N2_2_Total;		//N2_2 총합수치

	int CDA_1;			//CDA_1 수치
	int CDA_1_Total;	//CDA_1 총합수치

	// Accura Data		// jsyoon 20190318 add
	int nValueCnt;
	int nValue[20];
	int nValueTotal[20];
	
}ADDR_DATA;

typedef struct{
	//FAN
	double			dFanSpeed1;	//FDC모듈에서 처리
	double			dFanSpeed2;	//FDC모듈에서 처리
	double			dFanSpeed3;	//FDC모듈에서 처리
	double			dFanSpeed4;	//FDC모듈에서 처리
	double			dFanSpeed5;	//FDC모듈에서 처리
	double			dFanSpeed6;	//FDC모듈에서 처리
}FANSPEED, *PFANSPEED;

