#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "stdafx.h"

#define		VTM_SHARE_MEM_STRING_FDC	"KMAC_Shared_Memory_For_FDC"

// const enum NUM_FDC_ICP{
// 		FDC_NUM_CB_O2 = 0,
// 		FDC_NUM_CB_H20,
// 		FDC_NUM_CB_PRESS,
// 		FDC_NUM_CB_O2_R,
// 		FDC_NUM_SACC_O2,
// 		FDC_NUM_SACC_H20,
// 		FDC_NUM_SACC_PRESS,
// 		FDC_NUM_PROBE_TEMP,
// 		FDC_NUM_MAIN_CDA,
// 		FDC_NUM_MAIN_VACUUM1,
// 		FDC_NUM_MAIN_VACUUM2,
// 		FDC_NUM_MAIN_VACUUM3,
// 		FDC_NUM_MAIN_VACUUM4,
// 		FDC_NUM_MAIN_VACUUM5,
// 		FDC_NUM_MAIN_N2_R,		
// 		FDC_NUM_CHUCK_VACUUM1,
// 		FDC_NUM_CHUCK_VACUUM2,
// 		FDC_NUM_CHUCK_VACUUM3,
// 		FDC_NUM_CHUCK_VACUUM4,
// 		FDC_NUM_CHUCK_VACUUM5,
// 		FDC_NUM_IONIZER_N2,		
// 		FDC_NUM_ISOLATOR_N2,
// 		FDC_NUM_MAIN_N2_L,
// 		FDC_NUM_SACC_TEMP,
// 		FDC_NUM_EBOX_TEMP,
// 		FDC_NUM_PURI_O2,
// 		FDC_NUM_PURI_H20,
// 		FDC_NUM_PURI_PRESS,
// 		FDC_NUM_AXIS_LOAD_R_TOG,
// 		FDC_NUM_AXIS_LOAD_X_TOG,
// 		FDC_NUM_AXIS_LOAD_Y_TOG,
// 		FDC_NUM_AXIS_TOG_END = FDC_NUM_AXIS_LOAD_R_TOG + 31,
// 		FDC_NUM_AXIS_LOAD_R_POS = FDC_NUM_AXIS_TOG_END,
// 		FDC_NUM_AXIS_POS_END = FDC_NUM_AXIS_LOAD_R_POS + 31,
// 
// 
// 		
// 		FDC_NUM_FFU_RPM_1 = FDC_NUM_AXIS_POS_END,
// 		FDC_NUM_FFU_RPM_2,
// 		FDC_NUM_FFU_RPM_3,
// 		FDC_NUM_FFU_RPM_4,
// 		FDC_NUM_FFU_RPM_5,
// 		FDC_NUM_FFU_RPM_6,
// 		FDC_NUM_FFU_RPM_7,
// 		FDC_NUM_FFU_RPM_8,
// 		FDC_NUM_FFU_RPM_9,
// 		FDC_NUM_FFU_RPM_10,
// 		FDC_NUM_FFU_RPM_11,
// 		FDC_NUM_FFU_RPM_12,
// 		FDC_NUM_FFU_RPM_13,
// 		FDC_NUM_FFU_RPM_14,
// 		FDC_NUM_FFU_RPM_15,
// 		FDC_NUM_FFU_RPM_16,
// 		FDC_NUM_FFU_RPM_17,
// 		FDC_NUM_FFU_RPM_18,
// 		FDC_NUM_FFU_RPM_19,
// 		FDC_NUM_FFU_RPM_20,
// 		FDC_NUM_FFU_RPM_21,
// 		FDC_NUM_FFU_RPM_END,
// 
// 		FDC_NUM_IONIZER_STATE,	
// 		
// 		FDC_NUM_DISK_PERST_1,
// 		FDC_NUM_DISK_PERST_2,
// 		FDC_NUM_DISK_PERST_3,
// 		FDC_NUM_DISK_PERST_4,
// 
// 		FDC_NUM_LAMP_LIFE_TIME_1,
// 		FDC_NUM_LAMP_LIFE_TIME_2,
// 		FDC_NUM_LAMP_LIFE_TIME_3,
// 		FDC_NUM_LAMP_LIFE_TIME_4,
// 		FDC_NUM_LAMP_LIFE_TIME_5,
// 		FDC_NUM_LAMP_LIFE_TIME_6,
// 		FDC_NUM_LAMP_LIFE_TIME_7,
// 		FDC_NUM_LAMP_LIFE_TIME_8,
// 		FDC_NUM_LAMP_LIFE_TIME_9,
// 		FDC_NUM_LAMP_LIFE_TIME_10,
// 		FDC_NUM_LAMP_LIFE_TIME_11,
// 
// 
// 
// 		//20170102 lhj add
// 		FDC_NUM_ISOLATOR_VALUE1,
// 		FDC_NUM_ISOLATOR_VALUE2,
// 		FDC_NUM_ISOLATOR_VALUE3,
// 		//
// 		
// 		FDC_NUM_MAIN_VER, // 20170303 lhj add
// 		FDC_NUM_CIM_VER,
// 		FDC_NUM_STATIC_ELECTRIC,
// 		//
// 
// 		//jh.kang 20171025 add
// 		// 이온나이져 상태
// 		FDC_HEAD1_CONNECT,		
// 		FDC_HEAD1_ALARM,
// 		FDC_HEAD1_RUNTIME,
// 		FDC_HEAD2_CONNECT,
// 		FDC_HEAD2_ALARM,
// 		FDC_HEAD2_RUNTIME,
// 		FDC_HEAD3_CONNECT,		
// 		FDC_HEAD3_ALARM,
// 		FDC_HEAD3_RUNTIME,
// 		FDC_HEAD4_CONNECT,
// 		FDC_HEAD4_ALARM,
// 		FDC_HEAD4_RUNTIME,
// 		//
// 
// 		// jsyoon 20171101 add SACC
// 		FDC_NUM_AXIS_SACC_Z_TOG,
// 		FDC_NUM_AXIS_SACC_AZ_TOG,
// 		FDC_NUM_AXIS_SACC_AY_TOG,
// 		FDC_NUM_AXIS_SACC_Z_POS,
// 		FDC_NUM_AXIS_SACC_AZ_POS,
// 		FDC_NUM_AXIS_SACC_AY_POS,
// 
// 		FDC_NUM_END,
// 
// 		FDC_NUM_PC_TEMP,
// 		FDC_NUM_CB_TEMP,
// };
// 
// const enum NUM_FDC_ICE{
// 	FDC_ICE_MAIN_VER	= 0,
// 	FDC_ICE_CIM_VER,
// 	FDC_ICE_NUM_DISK_PERST_1,
// 	FDC_ICE_NUM_DISK_PERST_2,
// 	FDC_ICE_NUM_DISK_PERST_3,
// 	FDC_ICE_NUM_DISK_PERST_4,
// 	FDC_ICE_NUM_DISK_PERST_5,
// 
// 	FDC_ICE_NUM_SE_INTENS_1,	// DUV
// 	FDC_ICE_NUM_SE_INTENS_2,	// UV
// 	FDC_ICE_NUM_SE_INTENS_3,	// IR
// 	FDC_ICE_NUM_SE_INTENS_4,	// AVE
// 
// 
// 	FDC_ICE_NUM_SE_LAMP_1,		// LAMP USE A 1 or 0
// 	FDC_ICE_NUM_SE_LAMP_2,		// LAMP A TIME
// 	FDC_ICE_NUM_SE_LAMP_3,		// LAMP B TIME
// 
// 	FDC_ICE_NUM_SE_ANGLE,		// SE ANGLE
// 
// 	FDC_ICE_NUM_SE_SRM_1,
// 	FDC_ICE_NUM_SE_SRM_2,
// 	FDC_ICE_NUM_SE_SRM_3,
// 
// 	FDC_ICE_NUM_MAIN_CDA,
// 	FDC_ICE_NUM_MAIN_N2_R,
// 	FDC_ICE_NUM_MAIN_N2_L,
// 
// 	FDC_ICE_NUM_CHUCK_VACUUM1,
// 	FDC_ICE_NUM_CHUCK_VACUUM2,
// 	FDC_ICE_NUM_CHUCK_VACUUM3,
// 	FDC_ICE_NUM_CHUCK_VACUUM4,
// 	FDC_ICE_NUM_CHUCK_VACUUM5,
// 
// 	FDC_ICE_NUM_ISOLATOR_N2,
// 	FDC_ICE_NUM_IONIZER_N2,
// 
// 	FDC_ICE_NUM_PC_TEMP,
// 
// 	FDC_ICE_NUM_PURI_O2,
// 	FDC_ICE_NUM_PURI_O2PPM,
// 	FDC_ICE_NUM_PURI_H20,
// 	FDC_ICE_NUM_PURI_PRESS,
// 
// 	FDC_ICE_NUM_CB_O2,
// 	FDC_ICE_NUM_CB_O2_R,
// 	FDC_ICE_NUM_CB_O2PPM,
// 	FDC_ICE_NUM_CB_O2PPM_R,
// 	FDC_ICE_NUM_CB_H20,
// 	FDC_ICE_NUM_CB_PRESS,
// 	
// 	FDC_ICE_NUM_AXIS_POS_X,
// 	FDC_ICE_NUM_AXIS_POS_Y,
// 	FDC_ICE_NUM_AXIS_SPEED_X,
// 	FDC_ICE_NUM_AXIS_SPEED_Y,
// 
// 	FDC_ICE_NUM_IONIZER_STATE,
// 
// 	FDC_ICE_NUM_FFU_RPM_1,
// 	FDC_ICE_NUM_FFU_RPM_2,
// 	FDC_ICE_NUM_FFU_RPM_3,
// 	FDC_ICE_NUM_FFU_RPM_4,
// 	FDC_ICE_NUM_FFU_RPM_5,
// 	FDC_ICE_NUM_FFU_RPM_6,
// 	FDC_ICE_NUM_FFU_RPM_7,
// 	FDC_ICE_NUM_FFU_RPM_8,
// 	FDC_ICE_NUM_FFU_RPM_9,
// 	FDC_ICE_NUM_FFU_RPM_10,
// 	FDC_ICE_NUM_FFU_RPM_11,
// 	FDC_ICE_NUM_FFU_RPM_12,
// 	FDC_ICE_NUM_FFU_RPM_END = FDC_ICE_NUM_FFU_RPM_1+32,
// 	
// 
// //	FDC_ICE_NUM_STATIC_ELECTRIC,	//jh.kang 20171025 remove
// 
// 	// 이온나이져 상태
// 	FDC_ICE_NUM_HEAD1_CONNECT,	
// 	FDC_ICE_NUM_HEAD1_ALARM,
// 	FDC_ICE_NUM_HEAD1_RUNTIME,
// 	FDC_ICE_NUM_HEAD2_CONNECT,
// 	FDC_ICE_NUM_HEAD2_ALARM,
// 	FDC_ICE_NUM_HEAD2_RUNTIME,
// 	//jh.kang 20171025 add
// 	FDC_ICE_NUM_HEAD3_CONNECT,		
// 	FDC_ICE_NUM_HEAD3_ALARM,
// 	FDC_ICE_NUM_HEAD3_RUNTIME,
// 	FDC_ICE_NUM_HEAD4_CONNECT,
// 	FDC_ICE_NUM_HEAD4_ALARM,
// 	FDC_ICE_NUM_HEAD4_RUNTIME,
// 	//
// 
// 	// 조도계
// 	FDC_ICE_ILLUMINATION_1,
// 	FDC_ICE_ILLUMINATION_2,
// 	FDC_ICE_ILLUMINATION_3,
// 	FDC_ICE_ILLUMINATION_4,
// 
// 	FDC_ICE_NUM_LAMP_LIFE_TIME_1,
// 	FDC_ICE_NUM_LAMP_LIFE_TIME_2,
// 	FDC_ICE_NUM_LAMP_LIFE_TIME_3,
// 	FDC_ICE_NUM_LAMP_LIFE_TIME_4,
// 	FDC_ICE_NUM_LAMP_LIFE_TIME_5,
// 	FDC_ICE_NUM_LAMP_LIFE_TIME_6,
// 	FDC_ICE_NUM_LAMP_LIFE_TIME_7,
// 	FDC_ICE_NUM_LAMP_LIFE_TIME_8,
// 	FDC_ICE_NUM_LAMP_LIFE_TIME_9,
// 	FDC_ICE_NUM_LAMP_LIFE_TIME_10,
// 	FDC_ICE_NUM_LAMP_LIFE_TIME_11,
// 
// 	//20170102 lhj add
// 	FDC_ICE_NUM_ISOLATOR_VALUE1,
// 	FDC_ICE_NUM_ISOLATOR_VALUE2,
// 	FDC_ICE_NUM_ISOLATOR_VALUE3,
// 	//
// 
// 	FDC_ICE_NUM_AIRPURGE,
// 	FDC_ICE_NUM_EBOX_TEMP,
// 	FDC_ICE_NUM_CB_TEMP,
// 
// 	FDC_ICE_NUM_END,
// };
// 
// //jh.kang 20171025 add
// const enum NUM_FDC_ICM{
// 	FDC_ICM_MAIN_VER	= 0,
// 	FDC_ICM_CIM_VER,
// 	FDC_ICM_NUM_DISK_PERST_1,
// 	FDC_ICM_NUM_DISK_PERST_2,
// 	FDC_ICM_NUM_DISK_PERST_3,
// 	FDC_ICM_NUM_DISK_PERST_4,
// 	FDC_ICM_NUM_DISK_PERST_5,
// 	FDC_ICM_NUM_DISK_PERST_6,
// 
// 	FDC_ICM_NUM_LAMPTIME_WHITE_H1,	
// 	FDC_ICM_NUM_LAMPTIME_UV_H1,	
// 	FDC_ICM_NUM_LAMPTIME_WHITE_H2,	
// 	FDC_ICM_NUM_LAMPTIME_UV_H2,	
// 
// 	FDC_ICM_NUM_MAIN_CDA_AIR,		
// 	FDC_ICM_NUM_MAIN_N2_AIR1,		
// 	FDC_ICM_NUM_MAIN_N2_AIR2,		
// 
// 	FDC_ICM_NUM_CHUCK_VACUUM1,
// 	FDC_ICM_NUM_CHUCK_VACUUM2,
// 	FDC_ICM_NUM_CHUCK_VACUUM3,
// 	FDC_ICM_NUM_CHUCK_VACUUM4,
// 	FDC_ICM_NUM_CHUCK_VACUUM5,
// 
// 	FDC_ICM_NUM_ISOLATOR_N2,
// 	FDC_ICM_NUM_IONIZER_N2,
// 	FDC_ICM_NUM_PC_TEMP,
// 
// 	FDC_ICM_NUM_PURI_O2,
// 	FDC_ICM_NUM_PURI_O2PPM,
// 	FDC_ICM_NUM_PURI_H20,
// 	FDC_ICM_NUM_PURI_PRESS,
// 
// 	FDC_ICM_NUM_CB_O2,
// 	FDC_ICM_NUM_CB_O2_R,
// 	FDC_ICM_NUM_CB_O2PPM,
// 	FDC_ICM_NUM_CB_O2PPM_R,
// 	FDC_ICM_NUM_CB_H20,
// 	FDC_ICM_NUM_CB_PRESS,
// 
// 	FDC_ICM_NUM_AXIS_POS_X,
// 	FDC_ICM_NUM_AXIS_POS_Y,
// 	FDC_ICM_NUM_AXIS_SPEED_X,
// 	FDC_ICM_NUM_AXIS_SPEED_Y,
// 
// 
// 	FDC_ICM_NUM_IONIZER_STATE,	
// 
// 	FDC_ICM_NUM_FFU_RPM_1,
// 	FDC_ICM_NUM_FFU_RPM_2,
// 	FDC_ICM_NUM_FFU_RPM_3,
// 	FDC_ICM_NUM_FFU_RPM_4,
// 	FDC_ICM_NUM_FFU_RPM_5,
// 	FDC_ICM_NUM_FFU_RPM_6,
// 	FDC_ICM_NUM_FFU_RPM_7,
// 	FDC_ICM_NUM_FFU_RPM_8,
// 	FDC_ICM_NUM_FFU_RPM_9,
// 	FDC_ICM_NUM_FFU_RPM_10,
// 	FDC_ICM_NUM_FFU_RPM_11,
// 	FDC_ICM_NUM_FFU_RPM_12,
// 	FDC_ICM_NUM_FFU_RPM_END = FDC_ICM_NUM_FFU_RPM_12,
// 
// 
// //	FDC_ICM_NUM_STATIC_ELECTRIC,	//jh.kang 20171025 remove
// 
// 	//jh.kang 20171025 add
// 	FDC_ICM_NUM_MTR_STG1_X,
// 	FDC_ICM_NUM_MTR_STG2_X,
// 	FDC_ICM_NUM_MTR_HEAD1_Y,
// 	FDC_ICM_NUM_MTR_HEAD2_Y,
// 	//
// 
// 	// 이온나이져 상태
// 	FDC_ICM_NUM_HEAD1_CONNECT,		
// 	FDC_ICM_NUM_HEAD1_ALARM,
// 	FDC_ICM_NUM_HEAD1_RUNTIME,
// 	FDC_ICM_NUM_HEAD2_CONNECT,
// 	FDC_ICM_NUM_HEAD2_ALARM,
// 	FDC_ICM_NUM_HEAD2_RUNTIME,
// 	//jh.kang 20171025 add
// 	FDC_ICM_NUM_HEAD3_CONNECT,		
// 	FDC_ICM_NUM_HEAD3_ALARM,
// 	FDC_ICM_NUM_HEAD3_RUNTIME,
// 	FDC_ICM_NUM_HEAD4_CONNECT,
// 	FDC_ICM_NUM_HEAD4_ALARM,
// 	FDC_ICM_NUM_HEAD4_RUNTIME,
// 	//
// 
// 	// jsyoon 20170830 add
// 	// 조도계
// 	FDC_ICM_ILLUMINATION_1,
// 	FDC_ICM_ILLUMINATION_2,
// 	FDC_ICM_ILLUMINATION_3,
// 	FDC_ICM_ILLUMINATION_4,
// 
// 	FDC_ICM_NUM_END,
// 
// 
// 	FDC_ICM_NUM_LAMP_LIFE_TIME_1,
// 	FDC_ICM_NUM_LAMP_LIFE_TIME_2,
// 	FDC_ICM_NUM_LAMP_LIFE_TIME_3,
// 	FDC_ICM_NUM_LAMP_LIFE_TIME_4,
// 	FDC_ICM_NUM_LAMP_LIFE_TIME_5,
// 	FDC_ICM_NUM_LAMP_LIFE_TIME_6,
// 	FDC_ICM_NUM_LAMP_LIFE_TIME_7,
// 	FDC_ICM_NUM_LAMP_LIFE_TIME_8,
// 	FDC_ICM_NUM_LAMP_LIFE_TIME_9,
// 	FDC_ICM_NUM_LAMP_LIFE_TIME_10,
// 	FDC_ICM_NUM_LAMP_LIFE_TIME_11,
// 
// 	//20170102 lhj add
// 	FDC_ICM_NUM_ISOLATOR_VALUE1,
// 	FDC_ICM_NUM_ISOLATOR_VALUE2,
// 	FDC_ICM_NUM_ISOLATOR_VALUE3,
// 	//
// 
// 	FDC_ICM_NUM_AIRPURGE,
// 	FDC_ICM_NUM_EBOX_TEMP,
// 	FDC_ICM_NUM_CB_TEMP,
// };
// 
// const enum NUM_FDC_OCP{
// 	FDC_OCP_NUM_MAIN_CDA1,
// 	FDC_OCP_NUM_MAIN_CDA2,
// 	FDC_OCP_NUM_MAIN_CDA3,	
// 	FDC_OCP_NUM_ZERO_POS1,
// 	FDC_OCP_NUM_ZERO_POS2,
// 	FDC_OCP_NUM_ZERO_POS3,
// 	FDC_OCP_NUM_ZERO_POS4,
// 	FDC_OCP_NUM_ZERO_POS5,
// 	FDC_OCP_NUM_MAIN_VACUUM1,
// 	FDC_OCP_NUM_CHUCK_VACUUM1,
// 	FDC_OCP_NUM_CHUCK_VACUUM2,
// 	FDC_OCP_NUM_FIXTURE1,
// 	FDC_OCP_NUM_FIXTURE2,
// 	FDC_OCP_NUM_ISOLATOR,
// 	FDC_OCP_NUM_PROBEMODULE,
// 	FDC_OCP_NUM_SHUTTER1,
// 	FDC_OCP_NUM_SHUTTER2,
// 	FDC_OCP_NUM_PC_TEMP,
// 	FDC_OCP_NUM_EBOX_TEMP1,
// 	FDC_OCP_NUM_EBOX_TEMP2,
// 	FDC_OCP_NUM_AXIS_LOAD_R_TOG,
// 	FDC_OCP_NUM_AXIS_TOG_END = FDC_OCP_NUM_AXIS_LOAD_R_TOG + 39,
// 
// 	FDC_OCP_NUM_AXIS_LOAD_R_POS = FDC_OCP_NUM_AXIS_TOG_END,
// 	FDC_OCP_NUM_AXIS_POS_END	= FDC_OCP_NUM_AXIS_LOAD_R_POS + 39,
// 
// 	FDC_OCP_NUM_FFU_RPM_1 = FDC_OCP_NUM_AXIS_POS_END,
// 	FDC_OCP_NUM_FFU_RPM_2,
// 	FDC_OCP_NUM_FFU_RPM_3,
// 	FDC_OCP_NUM_FFU_RPM_4,
// 	FDC_OCP_NUM_FFU_RPM_5,
// 	FDC_OCP_NUM_FFU_RPM_6,
// 	FDC_OCP_NUM_FFU_RPM_7,
// 	FDC_OCP_NUM_FFU_RPM_8,
// 	FDC_OCP_NUM_FFU_RPM_9,
// 	FDC_OCP_NUM_FFU_RPM_10,
// 	FDC_OCP_NUM_FFU_RPM_11,
// 	FDC_OCP_NUM_FFU_RPM_12,
// 	FDC_OCP_NUM_FFU_RPM_13,
// 	FDC_OCP_NUM_FFU_RPM_14,
// 	FDC_OCP_NUM_FFU_RPM_15,
// 	FDC_OCP_NUM_FFU_RPM_16,
// 	FDC_OCP_NUM_FFU_RPM_17,
// 	FDC_OCP_NUM_FFU_RPM_18,
// 	FDC_OCP_NUM_FFU_RPM_END = FDC_OCP_NUM_FFU_RPM_1 + 32,
// 
// 	//jh.kang 20171025 remove
// // 	FDC_OCP_NUM_IONIZER_STATE,	
// // 
// // 	FDC_OCP_NUM_DISK_PERST_1,
// // 	FDC_OCP_NUM_DISK_PERST_2,
// // 	FDC_OCP_NUM_DISK_PERST_3,
// // 	FDC_OCP_NUM_DISK_PERST_4,
// // 
// // 	FDC_OCP_NUM_LAMP_LIFE_TIME_1,
// // 	FDC_OCP_NUM_LAMP_LIFE_TIME_2,
// // 	FDC_OCP_NUM_LAMP_LIFE_TIME_3,
// // 	FDC_OCP_NUM_LAMP_LIFE_TIME_4,
// // 	FDC_OCP_NUM_LAMP_LIFE_TIME_5,
// // 	FDC_OCP_NUM_LAMP_LIFE_TIME_6,
// // 	FDC_OCP_NUM_LAMP_LIFE_TIME_7,
// //
// //	FDC_OCP_MAIN_VER, // 20170303 lhj add
// //	FDC_OCP_CIM_VER,
// //	FDC_OCP_NUM_STATIC_ELECTRIC,
// 	//
// 
// 	// 이온나이져 상태
// 	FDC_OCP_NUM_HEAD1_CONNECT,		
// 	FDC_OCP_NUM_HEAD1_ALARM,
// 	FDC_OCP_NUM_HEAD1_RUNTIME,
// 	FDC_OCP_NUM_HEAD2_CONNECT,
// 	FDC_OCP_NUM_HEAD2_ALARM,
// 	FDC_OCP_NUM_HEAD2_RUNTIME,
// 
// 	// 조도계
// 	FDC_OCP_ILLUMINATION_1,
// 	FDC_OCP_ILLUMINATION_2,
// 	FDC_OCP_ILLUMINATION_3,
// 	FDC_OCP_ILLUMINATION_4,
// 	//
// 
// 
// 	// jsyoon 20171101 add SACC
// 	FDC_OCP_AXIS_SACC_Z_TOG,
// 	FDC_OCP_AXIS_SACC_AZ_TOG,
// 	FDC_OCP_AXIS_SACC_AY_TOG,
// 	FDC_OCP_AXIS_SACC_Z_POS,
// 	FDC_OCP_AXIS_SACC_AZ_POS,
// 	FDC_OCP_AXIS_SACC_AY_POS,
// 
// 	FDC_OCP_NUM_END,
// };
// 
// //jh.kang 20171025 add
// const enum NUM_FDC_TFE{
// 	FDC_TFE_NUM_MAIN_VER	= 0,
// 	FDC_TFE_NUM_SERVER_VER,
// 	FDC_TFE_NUM_CIM_VER,
// 	FDC_TFE_NUM_DISK_PERST_1,
// 	FDC_TFE_NUM_DISK_PERST_2,
// 	FDC_TFE_NUM_DISK_PERST_3,
// 	FDC_TFE_NUM_DISK_PERST_4,
// 
// 	FDC_TFE_NUM_SE_INTENS_1,	// DUV
// 	FDC_TFE_NUM_SE_INTENS_2,	// UV
// 	FDC_TFE_NUM_SE_INTENS_3,	// IR
// 	FDC_TFE_NUM_SE_INTENS_4,	// AVE
// 
// 	FDC_TFE_NUM_SE_LAMP_1,		// LAMP USE A 1 or 0
// 	FDC_TFE_NUM_SE_LAMP_2,		// LAMP A TIME
// 	FDC_TFE_NUM_SE_LAMP_3,		// LAMP B TIME
// 
// 	FDC_TFE_NUM_SR_LAMP_1,		// SR LAMP TIME
// 
// 	FDC_TFE_NUM_SE_ANGLE,		// SE ANGLE
// 
// 	FDC_TFE_NUM_SE_SRM_1,
// 	FDC_TFE_NUM_SE_SRM_2,
// 	FDC_TFE_NUM_SE_SRM_3,
// 
// 	FDC_TFE_NUM_SR_SRM_1,
// 	FDC_TFE_NUM_SR_SRM_2,
// 	FDC_TFE_NUM_SR_SRM_3,
// 
// 	FDC_TFE_NUM_MAIN_CDA1_PRESS,
// 	FDC_TFE_NUM_MAIN_N21_PRESS,
// 	FDC_TFE_NUM_MAIN_V1_PRESS,
// 	FDC_TFE_NUM_FIX_L_B_CDA_PRESS,
// 	FDC_TFE_NUM_ISOLATOR_CDA_PRESS,	
// 	FDC_TFE_NUM_STG_VACUUM1_ON,
// 	FDC_TFE_NUM_STG_VACUUM2_ON,
// //	FDC_TFE_NUM_IONZR_PRESS,		
// 	FDC_TFE_NUM_RESERVED,		// sih 20171204 change 
// 	FDC_TFE_NUM_PC_TEMP,
// 
// 	FDC_TFE_NUM_FFU_RPM_1,
// 	FDC_TFE_NUM_FFU_RPM_2,
// 	FDC_TFE_NUM_FFU_RPM_3,
// 	FDC_TFE_NUM_FFU_RPM_4,
// 	FDC_TFE_NUM_FFU_RPM_5,
// 	FDC_TFE_NUM_FFU_RPM_6,
// // 	FDC_TFE_NUM_FFU_RPM_7,
// // 	FDC_TFE_NUM_FFU_RPM_8,
// // 	FDC_TFE_NUM_FFU_RPM_9,
// // 	FDC_TFE_NUM_FFU_RPM_10,
// // 	FDC_TFE_NUM_FFU_RPM_11,
// // 	FDC_TFE_NUM_FFU_RPM_12,
// 	FDC_TFE_NUM_FFU_RPM_END = FDC_TFE_NUM_FFU_RPM_6,
// 
// 	FDC_TFE_X_POS_SET,
// 	FDC_TFE_Y_POS_SET,
// 
// 	// sih 20171204 change 
// 	// 이온나이져 상태
// // 	FDC_TFE_NUM_HEAD1_CONNECT,	
// // 	FDC_TFE_NUM_HEAD1_ALARM,
// // 	FDC_TFE_NUM_HEAD1_RUNTIME,
// // 	FDC_TFE_NUM_HEAD2_CONNECT,
// // 	FDC_TFE_NUM_HEAD2_ALARM,
// // 	FDC_TFE_NUM_HEAD2_RUNTIME,
// // 	//jh.kang 20171025 add
// // 	FDC_TFE_NUM_HEAD3_CONNECT,		
// // 	FDC_TFE_NUM_HEAD3_ALARM,
// // 	FDC_TFE_NUM_HEAD3_RUNTIME,
// // 	FDC_TFE_NUM_HEAD4_CONNECT,
// // 	FDC_TFE_NUM_HEAD4_ALARM,
// // 	FDC_TFE_NUM_HEAD4_RUNTIME,
// 	//
// 
// 	FDC_TFE_NUM_IONZR_PRESS_1,
// 	FDC_TFE_NUM_IONZR_PRESS_2,
// 	FDC_TFE_NUM_IONZR_PRESS_3,
// 	FDC_TFE_NUM_IONZR_PRESS_4,
// 	FDC_TFE_NUM_IONZR_STATE_1,
// 	FDC_TFE_NUM_IONZR_STATE_2,
// 	FDC_TFE_NUM_IONZR_STATE_3,
// 	FDC_TFE_NUM_IONZR_STATE_4,
// 	//
// 
// 	FDC_TFE_NUM_END = 47,
// 
// };
// //
// 
// //jh.kang 20171025 add
// const enum NUM_FDC_EVEN{
// 	FDC_EVEN_NUM_DISK_PERST_1	=	0,
// 	FDC_EVEN_NUM_DISK_PERST_2,
// 	FDC_EVEN_NUM_DISK_PERST_3,
// 	FDC_EVEN_NUM_DISK_PERST_4,
// 	FDC_EVEN_NUM_SE_INTENS_1,	// DUV
// 	FDC_EVEN_NUM_SE_INTENS_2,	// UV
// 	FDC_EVEN_NUM_SE_INTENS_3,	// IR
// 	FDC_EVEN_NUM_SE_INTENS_4,	// AVE
// 	FDC_EVEN_NUM_SE_LAMP_1,		// LAMP USE A 1 or 0
// 	FDC_EVEN_NUM_SE_LAMP_2,		// LAMP A TIME
// 	FDC_EVEN_NUM_SE_LAMP_3,		// LAMP B TIME
// 
// 	FDC_EVEN_NUM_SR_LAMP_1,		// SR LAMP TIME
// 
// 	FDC_EVEN_NUM_SE_ANGLE,		// SE ANGLE
// 
// 	FDC_EVEN_NUM_SE_SRM_1,
// 	FDC_EVEN_NUM_SE_SRM_2,
// 	FDC_EVEN_NUM_SE_SRM_3,
// 
// 	FDC_EVEN_NUM_SR_SRM_1,
// 	FDC_EVEN_NUM_SR_SRM_2,
// 	FDC_EVEN_NUM_SR_SRM_3,
// 
// 	FDC_EVEN_NUM_MAIN_CDA1_PRESS,
// 	FDC_EVEN_NUM_MAIN_N21_PRESS,
// 	FDC_EVEN_NUM_MAIN_V1_PRESS,
// 	FDC_EVEN_NUM_FIX_L_B_CDA_PRESS,
// 	FDC_EVEN_NUM_ISOLATOR_CDA_PRESS,
// 
// 	FDC_EVEN_NUM_STG_VACUUM1_ON,
// 	FDC_EVEN_NUM_STG_VACUUM2_ON,
// 
// 	FDC_EVEN_NUM_PC_TEMP,
// 
// 	FDC_EVEN_NUM_FFU_RPM_1,
// 	FDC_EVEN_NUM_FFU_RPM_2,
// 	FDC_EVEN_NUM_FFU_RPM_3,
// 	FDC_EVEN_NUM_FFU_RPM_4,
// 	FDC_EVEN_NUM_FFU_RPM_5,
// 	FDC_EVEN_NUM_FFU_RPM_6,
// 	// 	FDC_EVEN_NUM_FFU_RPM_7,
// 	// 	FDC_EVEN_NUM_FFU_RPM_8,
// 	// 	FDC_EVEN_NUM_FFU_RPM_9,
// 	// 	FDC_EVEN_NUM_FFU_RPM_10,
// 	// 	FDC_EVEN_NUM_FFU_RPM_11,
// 	// 	FDC_EVEN_NUM_FFU_RPM_12,
// 	FDC_EVEN_NUM_FFU_RPM_END = FDC_EVEN_NUM_FFU_RPM_6,
// 
// 	FDC_EVEN_X_POS_SET,
// 	FDC_EVEN_Y_POS_SET,
// 
// 	FDC_EVEN_NUM_LAMPTIME_WHITE_H1,	
// 	FDC_EVEN_NUM_LAMPTIME_UV_H1,	
// 	FDC_EVEN_NUM_LAMPTIME_WHITE_H2,	
// 	FDC_EVEN_NUM_LAMPTIME_UV_H2,	
// 
// 	// 이온나이져
// 	// pys 20171204 change
// // 	FDC_EVEN_NUM_HEAD1_RUNTIME,
// // 	FDC_EVEN_NUM_HEAD2_RUNTIME,
// // 	FDC_EVEN_NUM_HEAD3_RUNTIME,
// // 	FDC_EVEN_NUM_HEAD4_RUNTIME,
// // 	FDC_EVEN_NUM_HEAD1_CONNECT,	
// // 	FDC_EVEN_NUM_HEAD2_CONNECT,
// // 	FDC_EVEN_NUM_HEAD3_CONNECT,
// // 	FDC_EVEN_NUM_HEAD4_CONNECT,
// 
// 	FDC_EVEN_NUM_IONZR_PRESS_1,
// 	FDC_EVEN_NUM_IONZR_PRESS_2,
// 	FDC_EVEN_NUM_IONZR_PRESS_3,
// 	FDC_EVEN_NUM_IONZR_PRESS_4,
// 	FDC_EVEN_NUM_IONZR_STATE_1,
// 	FDC_EVEN_NUM_IONZR_STATE_2,
// 	FDC_EVEN_NUM_IONZR_STATE_3,
// 	FDC_EVEN_NUM_IONZR_STATE_4,
// 	//
// 
// 	FDC_EVEN_NUM_END,
// 
// };
// 
// //jh.kang 20171025 add
// const enum NUM_FDC_BP{
// 
// 	FDC_BP_NUM_CIM_DATA_FAIL = 0,
// 	FDC_BP_NUM_SE_INTENS_1,	// DUV
// 	FDC_BP_NUM_SE_INTENS_2,	// UV
// 	FDC_BP_NUM_SE_INTENS_3,	// IR
// 	FDC_BP_NUM_SE_INTENS_4,	// AVE
// 
// 	FDC_BP_NUM_SR_INTENS_1,	// AVE
// 
// 	FDC_BP_NUM_SE_LAMP_1,		// LAMP USE A 1 or 0
// 	FDC_BP_NUM_SE_LAMP_2,		// LAMP A TIME
// 	FDC_BP_NUM_SE_LAMP_3,		// LAMP B TIME
// 
// 	FDC_BP_NUM_SR_LAMP_1,		// SR LAMP TIME
// 	FDC_BP_NUM_NGF_LAMP_1,	// SR LAMP TIME
// 
// 	FDC_BP_NUM_SE_SRM_THICKRSRV1,
// 	FDC_BP_NUM_SE_SRM_THICKRSRV2,
// 
// 	FDC_BP_NUM_SR_SRM_1,
// 	FDC_BP_NUM_SR_SRM_2,
// 	FDC_BP_NUM_SR_SRM_3,
// 	FDC_BP_NUM_SR_SRM_4,
// 
// 	FDC_BP_NUM_CDA_PRESSURE_01,
// 	FDC_BP_NUM_VACUUM_PRESSURE,
// 	FDC_BP_NUM_FIXTURE_PRESSURE,
// 	FDC_BP_NUM_ISOLATORPRESSURE,
// 	FDC_BP_NUM_CDA_AIR_BLOWING_PRESSURE_01,
// 	FDC_BP_NUM_CDA_AIR_BLOWING_PRESSURE_02,
// 	FDC_BP_NUM_PC_TEMP,
// 
// 	FDC_BP_NUM_FFU_RPM_1,
// 	FDC_BP_NUM_FFU_RPM_2,
// 	FDC_BP_NUM_FFU_RPM_3,
// 	FDC_BP_NUM_FFU_RPM_4,
// 	FDC_BP_NUM_FFU_RPM_5,
// 	FDC_BP_NUM_FFU_RPM_6,
// 	FDC_BP_NUM_FFU_RPM_7,
// 	FDC_BP_NUM_FFU_RPM_8,
// 	FDC_BP_NUM_FFU_RPM_9,
// 	FDC_BP_NUM_FFU_RPM_10,
// 	FDC_BP_NUM_FFU_RPM_11,
// 	FDC_BP_NUM_FFU_RPM_12,
// 	FDC_BP_NUM_FFU_RPM_13,
// 	FDC_BP_NUM_FFU_RPM_14,
// 	FDC_BP_NUM_FFU_RPM_END = FDC_BP_NUM_FFU_RPM_14,
// 
// 	FDC_BP_NUM_IONIZER_STATUS1,
// 	FDC_BP_NUM_IONIZER_STATUS2,
// 	FDC_BP_NUM_IONIZER_STATUS3,
// 	FDC_BP_NUM_IONIZER_STATUS4,
// 
// 	FDC_BP_NUM_X_OVERLOAD,
// 	FDC_BP_NUM_Y_OVERLOAD,
// 	FDC_BP_NUM_Z_OVERLOAD,
// 	FDC_BP_NUM_MSE_VALUE,
// 
// 	FDC_BP_NUM_LIFTPIN_FIRST_UP_TIME,
// 	FDC_BP_NUM_LIFTPIN_SECOND_UP_TIME,
// 	FDC_BP_NUM_LIFTPIN_FIRST_DELAY_TIME,
// 	FDC_BP_NUM_LIFTPIN_FIRST_UP_POS,
// 	FDC_BP_NUM_LIFTPIN_SECOND_UP_POS,
// 	FDC_BP_NUM_LIFTPIN_DOWN_TIME,
// 
// 	FDC_BP_NUM_MAIN_VER,
// 
// 	FDC_BP_NUM_END,
// };
// //
// 
// //jh.kang 20171025 add
// const enum NUM_FDC_SRM{
// 
// 	FDC_SRM_NUM_CIM_DATA_FAIL = 0,
// 	FDC_SRM_NUM_RS_CONTACT_CNT,
// 	FDC_SRM_NUM_RS_SRM,
// 
// 	FDC_SRM_NUM_CDA_PRESSURE_01,
// 	FDC_SRM_NUM_VACUUM_PRESSURE,
// 	FDC_SRM_NUM_FIXTURE_PRESSURE,
// //	FDC_SRM_NUM_ISOLATORPRESSURE,
// 	FDC_SRM_NUM_CDA_AIR_BLOWING_PRESSURE_01,
// 	FDC_SRM_NUM_CDA_AIR_BLOWING_PRESSURE_02,
// 	FDC_SRM_NUM_PC_TEMP,
// 
// 	FDC_SRM_NUM_FFU_RPM_1,
// 	FDC_SRM_NUM_FFU_RPM_2,
// 	FDC_SRM_NUM_FFU_RPM_3,
// 	FDC_SRM_NUM_FFU_RPM_4,
// 	FDC_SRM_NUM_FFU_RPM_5,
// 	FDC_SRM_NUM_FFU_RPM_6,
// 	FDC_SRM_NUM_FFU_RPM_7,
// 	FDC_SRM_NUM_FFU_RPM_8,
// 	FDC_SRM_NUM_FFU_RPM_9,
// 	FDC_SRM_NUM_FFU_RPM_10,
// 	FDC_SRM_NUM_FFU_RPM_11,
// 	FDC_SRM_NUM_FFU_RPM_12,
// 	FDC_SRM_NUM_FFU_RPM_13,
// 	FDC_SRM_NUM_FFU_RPM_14,
// 	FDC_SRM_NUM_FFU_RPM_END = FDC_SRM_NUM_FFU_RPM_14,
// 
// 	FDC_SRM_NUM_IONIZER_STATUS1,
// 	FDC_SRM_NUM_IONIZER_STATUS2,
// 	FDC_SRM_NUM_IONIZER_STATUS3,
// 	FDC_SRM_NUM_IONIZER_STATUS4,
// 
// 	FDC_SRM_NUM_X_OVERLOAD,
// 	FDC_SRM_NUM_Y_OVERLOAD,
// 	
// 	FDC_SRM_NUM_CURRENT_VALUE,
// 	FDC_SRM_NUM_VOLTAGE_VALUE,
// 
// 	FDC_SRM_NUM_LIFTPIN_FIRST_UP_TIME,
// 	FDC_SRM_NUM_LIFTPIN_SECOND_UP_TIME,
// 	FDC_SRM_NUM_LIFTPIN_FIRST_DELAY_TIME,
// 	FDC_SRM_NUM_LIFTPIN_FIRST_UP_POS,
// 	FDC_SRM_NUM_LIFTPIN_SECOND_UP_POS,
// 	FDC_SRM_NUM_LIFTPIN_DOWN_TIME,
// 
// 	FDC_SRM_NUM_MAIN_VER,
// 
// 	FDC_SRM_NUM_END,
// 
// };
// //
// 
// //jh.kang 20171025 add
// const enum NUM_FDC_TM{
// 
// 	FDC_TM_CDA_PRESS = 0, 
// 	FDC_TM_GATE, 
// 	FDC_TM_ANGLE, 
// 	FDC_TM_MAIN_TM_N2, 
// 	FDC_TM_MLC_N2, 
// 	FDC_TM_CHAMBER_O2_PPM_DATA_PERSENT_1, 
// 	FDC_TM_CHAMBER_O2_PPM_DATA_PERSENT_2, 
// 	FDC_TM_MLC_O2_PPM_DATA_PERSENT_1, 
// 	FDC_TM_CHAMBER_O2_PPM_DATA_1, 
// 	FDC_TM_CHAMBER_O2_PPM_DATA_2, 
// 	FDC_TM_MLC_O2_PPM_DATA_1, 
// 	FDC_TM_S_RBT_PRESS_DATA_1, 
// 	FDC_TM_MLC_PRESS_DATA_1, 
// 	FDC_TM_S_RBT_H2O_DATA_1, 
// 	FDC_TM_MLC_H2O_DATA_1, 
// 	FDC_TM_PURIFIER_O2_PERSENT, 
// 	FDC_TM_PURIFIER_O2_PPM, 
// 	FDC_TM_PURIFIER_PRESSUR, 
// 	FDC_TM_PURIFIER_H2O_PPM, 
// 	FDC_TM_PURIFIER_AIR_PURGE_TIME, 
// 	FDC_TM_NUM_PC_TEMP,
// 	
// 	FDC_TM_NUM_EFU_FFU_RPM_1,
// 	FDC_TM_NUM_EFU_FFU_RPM_2,
// 	FDC_TM_NUM_EFU_FFU_RPM_3,
// 	FDC_TM_NUM_EFU_FFU_RPM_4,
// 	FDC_TM_NUM_EFU_FFU_RPM_5,
// 	FDC_TM_NUM_EFU_FFU_RPM_6,
// 	FDC_TM_NUM_EFU_FFU_RPM_7,
// 	FDC_TM_NUM_EFU_FFU_RPM_8,
// 	FDC_TM_NUM_EFU_FFU_RPM_9,
// 	FDC_TM_NUM_EFU_FFU_RPM_10,
// 	FDC_TM_NUM_EFU_FFU_RPM_11,
// 	FDC_TM_NUM_EFU_FFU_RPM_12,
// 	FDC_TM_NUM_EFU_FFU_RPM_13,
// 	FDC_TM_NUM_EFU_FFU_RPM_14,
// 	FDC_TM_NUM_EFU_FFU_RPM_15,
// 	FDC_TM_NUM_EFU_FFU_RPM_16,
// 	FDC_TM_NUM_EFU_FFU_RPM_17,
// 	FDC_TM_NUM_EFU_FFU_RPM_18,
// 	FDC_TM_NUM_EFU_FFU_RPM_19,
// 	FDC_TM_NUM_EFU_FFU_RPM_20,
// 	FDC_TM_NUM_EFU_FFU_RPM_21,
// 	FDC_TM_NUM_EFU_FFU_RPM_22,
// 	FDC_TM_NUM_EFU_FFU_RPM_23,
// 	FDC_TM_NUM_EFU_FFU_RPM_24,
// 	FDC_TM_NUM_EFU_FFU_RPM_25,
// 	FDC_TM_NUM_EFU_FFU_RPM_26,
// 	FDC_TM_NUM_EFU_FFU_RPM_27,
// 	FDC_TM_NUM_EFU_FFU_RPM_28,
// 	FDC_TM_NUM_EFU_FFU_RPM_29,
// 	FDC_TM_NUM_EFU_FFU_RPM_30,
// 	FDC_TM_NUM_EFU_FFU_RPM_31,
// 	FDC_TM_NUM_EFU_FFU_RPM_32,
// 	FDC_TM_NUM_EFU_FFU_RPM_33,
// 	FDC_TM_NUM_EFU_FFU_RPM_34,
// 	FDC_TM_NUM_EFU_FFU_RPM_35,
// 	FDC_TM_NUM_EFU_FFU_RPM_36,
// 	FDC_TM_NUM_EFU_FFU_RPM_37,
// 	FDC_TM_NUM_EFU_FFU_RPM_38,
// 	FDC_TM_NUM_EFU_FFU_RPM_39,
// 	FDC_TM_NUM_EFU_FFU_RPM_40,
// 	FDC_TM_NUM_EFU_FFU_RPM_41,
// 	FDC_TM_NUM_FFU_RPM_END = FDC_TM_NUM_EFU_FFU_RPM_41,
// 	FDC_TM_NUM_END,
// };
// //
// typedef struct // DLL Input데이터
// {
// 	double dO2ppm;	
// 	double dO2pct;
// 	double dH2O;							
// 	double dTemper[5];						
// //	double dEFU[32];
// 	double dEFU[64];		// 20161226 kks 
// 	double dPressure;
// 	double dStaticElectric; //jh.kang 20170518 add
// 	DWORD   lLiveCheck;
// 
// 	// sih 20160817 add 
// 	double dChamberO2pct_1;
// 	double dChamberO2pct_2;
// 	double dChamberO2ppm_1;	
// 	double dChamberO2ppm_2;	
// 	double dChamberH2O;
// 	double dChamberPressure;
// 
// 	// jsyoon 20171121 add
// 	// Ionizer
// 	BOOL	bIonizerConnect[4];
// 	BOOL	bIonizerAlarm[4];
// 	double  dIonizerTime[4];
// 
// }FFU, *PFFU;
// 
// //A2
// typedef struct
// {
// 	BOOL			bSELampAOn;
// 	double			dSELampTimeA;
// 	double			dSELampTimeB;
// 	double			dAngle;		
// 	double			dIntensity;
// 	double			dAVGSignal;
// 	double			dDUVSignal;
// 	double			dUVSignal;
// 	double			dIRSignal;
// 	double			dFosLampTime; //->0000000
// 	double			dFosCorRep[50];				
// 	double			dFosZMtrFac; //->00000000
// 	double			dRS;
// 	BOOL			bCimConnect;		//연결 상태
// 	BOOL			bSeConnect;		//연결 상태
// 	double			dInsZMtrFac; //->00000000
// 	double			dInsZCorRep;			
// 	double			dLoadFactor[10];		
// 	double			dInsXCorRep;			
// 	BOOL			bGlassExist_LiftPin;	
// 	int				iFixtureOnTime;			
// 	int				iFixtureOffTime;		
// 	double			dBstgZmtrFac; //0000000
// 	double			dBSTG_TMTR_FAC; //00000
// 	double			dBSTG_TCOR_REP;//000000
// 	BOOL			bGlassExist_Chuck;
// 	double			dInsYCorRep;			
// 	double			dWSTG_TMTR_FAC;	//00000	
// 	double			dWSTG_TCOR_REP;	//00000	
// 	double			dFDC_TM_ZMTR_SPD;	//00000	
// 	int				iVelocity[10];			
// 	double			dBSTG_ZMTR_SPD;	//00000	
// 	double			dBSTG_TMTR_SPD;	//00000	
// 	double			dWSTG_TMTR_SPD;	//00000	
// 	int				iClampOnTime;			
// 	int				iClampOffTime;			
// 	double			dMark1PosX[4];			
// 	double			dMark1PosY[4];			
// 	double			dICEGATE_ON_OFF;	//00000
// 	double			dICPGATE_ON_OFF;	//00000
// 	double			dTMGATE_ON_OFF;	//00000	
// 	double			dEQP_STATE;	//00000		
// 	double			dCHM_TEMP;	//00000		
// 	double			dWSTG_VAC;	//00000		
// 	double			dWSTG_BLOW;	//00000
// 	double			ddPURI_STATE; //00000
// 	double			dEMPTE;	//00000
// 	double			dAVGSignal_2;			
// 	double			dDUVSignal_2;
// 	double			dUVSignal_2;
// 	double			dIRSignal_2;
// 	double			dAVGSignal_10;				
// 	double			dDUVSignal_10;
// 	double			dUVSignal_10;
// 	double			dIRSignal_10;
// 	double			dAVGSignal_25;				
// 	double			dDUVSignal_25;
// 	double			dUVSignal_25;
// 	double			dIRSignal_25;
// 	double			dAVGSignal_60;			
// 	double			dDUVSignal_60;
// 	double			dUVSignal_60;
// 	double			dIRSignal_60;
// 	double			dCimHandling[8];	//00000		
// 
// 	DWORD			lLiveCheck;	// jsyoon 20150113 add
// //	FFU				ffu;
// }FDC_Shared_A2, *PFDC_Shared_A2;
// 
// //A3 ICE
// typedef struct
// {
// 	//SW Ver.
// //	BOOL			bMetroVer;		//Main에서 처리
// 	double			dMetroVer;		//Main에서 처리		// sih 20161228 add 
// //	BOOL			bServerVer;		//Main에서 처리
// 	BOOL			bCimVer;		//cim에서 처리
// 
// 	//HDD 용량
// 	int				nMetroHDD_C;	//Main에서 처리
// 	int				nMetroHDD_D;	//Main에서 처리
// 	int				nMetroHDD_E;	//Main에서 처리		
// 	int				nCimHDD_C;		//cim에서 처리
// 	int				nCimHDD_D;		//cim에서 처리
// 
// 	//SE INTENSITY
// 	double			dAVGSignal;	//Main에서 처리
// 	double			dDUVSignal;	//Main에서 처리
// 	double			dUVSignal;	//Main에서 처리
// 	double			dIRSignal;	//Main에서 처리
// 
// 	//Lamp Time
// 	BOOL			bSELampAOn; //Main에서 처리
// 	double			dSELampTimeA; //Main에서 처리
// 	double			dSELampTimeB; //Main에서 처리
// 
// 	//SE Angle
// 	double			dAngle;	 //Main에서 처리
// 
// 	//SRM
// 	double			dSRMThick100A; //Main에서 처리
// 	double			dSRMThick250A; //Main에서 처리
// 	double			dSRMThick600A; //Main에서 처리
// 
// 	//압력 (Main 1차)
// // 	double			dMainCDAAir;	//FDC모듈에서 처리
// // 	double			dMainN2Air1;	//FDC모듈에서 처리
// // 	double			dMainN2Air2;	//FDC모듈에서 처리
// // 	double			dMainVacuum1;	//FDC모듈에서 처리
// // 	double			dMainVacuum2;	//FDC모듈에서 처리
// // 	double			dMainVacuum3;	//FDC모듈에서 처리
// // 	double			dMainVacuum4;	//FDC모듈에서 처리
// // 	double			dMainVacuum5;	//FDC모듈에서 처리
// 
// 	//압력 (Main 2차)
// // 	double			dIsolaterPress;	//FDC모듈에서 처리
// // 	double			dIonizerPress;	//FDC모듈에서 처리
// 
// 	//온도
// 	double			dPCTemp[5];	//FDC모듈에서 처리
// 
// 
// 	//전력량
// // 	double			dN01TPW1;	//CIM에서 처리
// // 	double			dN01TPW2;	//CIM에서 처리
// // 	double			dN01TPW3;	//CIM에서 처리
// // 	double			dN01TPW4;	//CIM에서 처리
// // 	double			dN01TPW5;	//CIM에서 처리 
// 	//전력량 SEM Controller
// 	double			dN01TPW1;	//CIM에서 처리 N_01_T_PW (KWh) UPS 적산 전력량 
// 	double			dN01TPW2;	//CIM에서 처리 N_01_I_PW (kW)  UPS 순시 전력량 
// 	double			dN01TPW3;	//CIM에서 처리 N_01_R_VOL (V)  UPS R상 전압
// 	double			dN01TPW4;	//CIM에서 처리 N_01_T_VOL (V)  UPS T상 전압 
// 	double			dN01TPW5;	//CIM에서 처리 N_01_R_CUR (A)  UPS R상 전류
// 	double			dN01TPW6;	//CIM에서 처리 N_01_T_CUR (A)  UPS T상 전류
// 	double			dN02TPW7;	//CIM에서 처리 N_02_T_PW (kWh) GPS 적산 전력량 
// 	double			dN02TPW8;	//CIM에서 처리 N_02_I_PW  (kW) GPS 순시 전력량
// 	double			dN02TPW9;	//CIM에서 처리 N_02_R_VOL (V)  GPS R상 전압
// 	double			dN02TPW10;	//CIM에서 처리 N_02_T_VOL (V)  GPS T상 전압
// 	double			dN02TPW11;	//CIM에서 처리 N_02_R_CUR (A)  GPS R상 전류
// 	double			dN02TPW12;	//CIM에서 처리 N_02_T_CUR (A)  GPS T상 전류
// 	double			dN02TPW13;	//CIM에서 처리 N_01_CDA        CDA Quick Purge
// 	double			dN02TPW14;	//CIM에서 처리 N_01_N2         N2 Quick Purge
// 
// 	//Purifier 정보
// // 	double			dPuriH2O;		//CIM에서 처리
// // 	double			dPuriO2;		//CIM에서 처리
// // 	double			dPuriO2ppm;		//CIM에서 처리
// // 	double			dPuriPress;		//CIM에서 처리
// 	
// 	//Chamber 정보
// // 	double			dChamberO2_1;	//FDC모듈에서 처리
// // 	double			dChamberO2_2;	//FDC모듈에서 처리
// // 	double			dChamberO2ppm_1;//FDC모듈에서 처리
// // 	double			dChamberO2ppm_2;//FDC모듈에서 처리
// // 	double			dChamberH2O;	//FDC모듈에서 처리
// // 	double			dChamberPress;	//FDC모듈에서 처리
// 
// 	//MOTOR
// 	double			dHomePosX; //Main에서 처리
// 	double			dHomePosY; //Main에서 처리
//  	double			dSpeedX; //Main에서 처리
//  	double			dSpeedY; //Main에서 처리
// 	BOOL			bionizerstatus;	 //Main에서 처리	//ksk 20150529 add
// 
// 	//FFU 정보		
// 	double          dFanSpeed[32];
// 
// //	double			dAirPurgeValue;		//ksk 20160324 add
// 
// // jsyoon 20171121 remove
// // 	//jh.kang 20171025 add
// // 	BOOL			bHead1_Connect;
// // 	BOOL			bHead1_Alarm;
// // 	double			dHead1_Runtime;
// // 	BOOL			bHead2_Connect;
// // 	BOOL			bHead2_Alarm;
// // 	double			dHead2_Runtime;
// // 	BOOL			bHead3_Connect;
// // 	BOOL			bHead3_Alarm;
// // 	double			dHead3_Runtime;
// // 	BOOL			bHead4_Connect;
// // 	BOOL			bHead4_Alarm;
// // 	double			dHead4_Runtime;
// // 	//
// 
// 	double			dFDCData[300];
// 
// }FDC_Shared_ICE, *PFDC_Shared_ICE;
// 
// //A3 ICM
// typedef struct
// {
// 	//SW Ver.
// //	BOOL			bMetroVer;		//Main에서 처리
// 	double			dMetroVer;		//Main에서 처리		// sih 20161228 add 
// //	BOOL			bServerVer;		//Main에서 처리
// 	BOOL			bCimVer;		//cim에서 처리
// 	
// 	//HDD 용량
// 	int				nMetroHDD_C;	//Main에서 처리
// 	int				nMetroHDD_D;	//Main에서 처리
// 	int				nMetroHDD_E;	//Main에서 처리	
// 	int				nMetroHDD_G;	//Main에서 처리
// 	int				nCimHDD_D;		//cim에서 처리
// 	int				nCimHDD_E;		//cim에서 처리
// 	
// 	//Head INTENSITY
// 	double			bImgHistoH1;	//Main에서 처리
// 	double			bImgHistoH2;	//Main에서 처리
// 	
// 	//Lamp Time
// 	double			dLampTimeWhiteH1; //Main에서 처리
// 	double			dLampTimeUVH1; //Main에서 처리
// 	double			dLampTimeWhiteH2; //Main에서 처리
// 	double			dLampTimeUVH2; //Main에서 처리
// 	
// 	//SRM
// 	double			dSRMPPAH1; //Main에서 처리
// 	double			dSRMPPAH2; //Main에서 처리
// 	
// 	//압력 (Main 1차)
// 	double			dMainCDAAir;	//FDC모듈에서 처리
// 	double			dMainN2Air1;	//FDC모듈에서 처리
// 	double			dMainN2Air2;	//FDC모듈에서 처리
// 	double			dMainVacuum1;	//FDC모듈에서 처리
// 	double			dMainVacuum2;	//FDC모듈에서 처리
// 	double			dMainVacuum3;	//FDC모듈에서 처리
// 	double			dMainVacuum4;	//FDC모듈에서 처리
// 	double			dMainVacuum5;	//FDC모듈에서 처리
// 	
// 	//압력 (Main 2차)
// 	double			dIsolaterPress;	//FDC모듈에서 처리
// //	double			dShutterPress;	//FDC모듈에서 처리
// // 	double			dAirBrowing1;	//FDC모듈에서 처리
// // 	double			dAirBrowing2;	//FDC모듈에서 처리
// // 	double			dPlatePress1;	//FDC모듈에서 처리
// // 	double			dPlatePress2;	//FDC모듈에서 처리
// // 	double			dPlatePress3;	//FDC모듈에서 처리
// // 	double			dPlatePress4;	//FDC모듈에서 처리
// 	double			dIonizerPress;	//FDC모듈에서 처리
// //	double			dFixSuction;	//FDC모듈에서 처리
// 	
// 	//온도
// 	double			dPCTemp[5];	//FDC모듈에서 처리
// 
// 
// 	//전력량
// // 	double			dN01TPW1;	//CIM에서 처리
// // 	double			dN01TPW2;	//CIM에서 처리
// // 	double			dN01TPW3;	//CIM에서 처리
// // 	double			dN01TPW4;	//CIM에서 처리
// // 	double			dN01TPW5;	//CIM에서 처리
// 
// 	//전력량 SEM Controller
// 	double			dN01TPW1;	//CIM에서 처리 N_01_T_PW (KWh) UPS 적산 전력량 
// 	double			dN01TPW2;	//CIM에서 처리 N_01_I_PW (kW)  UPS 순시 전력량 
// 	double			dN01TPW3;	//CIM에서 처리 N_01_R_VOL (V)  UPS R상 전압
// 	double			dN01TPW4;	//CIM에서 처리 N_01_T_VOL (V)  UPS T상 전압 
// 	double			dN01TPW5;	//CIM에서 처리 N_01_R_CUR (A)  UPS R상 전류
// 	double			dN01TPW6;	//CIM에서 처리 N_01_T_CUR (A)  UPS T상 전류
// 	double			dN02TPW7;	//CIM에서 처리 N_02_T_PW (kWh) GPS 적산 전력량 
// 	double			dN02TPW8;	//CIM에서 처리 N_02_I_PW  (kW) GPS 순시 전력량
// 	double			dN02TPW9;	//CIM에서 처리 N_02_R_VOL (V)  GPS R상 전압
// 	double			dN02TPW10;	//CIM에서 처리 N_02_T_VOL (V)  GPS T상 전압
// 	double			dN02TPW11;	//CIM에서 처리 N_02_R_CUR (A)  GPS R상 전류
// 	double			dN02TPW12;	//CIM에서 처리 N_02_T_CUR (A)  GPS T상 전류
// 	double			dN02TPW13;	//CIM에서 처리 N_01_CDA        CDA Quick Purge
// 	double			dN02TPW14;	//CIM에서 처리 N_01_N2         N2 Quick Purge
// 
// 	//Purifier 정보
// 	double			dPuriH2O;			//CIM에서 처리
// 	double			dPuriO2;			//CIM에서 처리
// 	double			dPuriO2ppm;			//CIM에서 처리
// 	double			dPuriPress;			//CIM에서 처리
// 	
// 	//Chamber 정보
// 	double			dChamberO2_1;		//FDC모듈에서 처리
// 	double			dChamberO2_2;		//FDC모듈에서 처리
// 	double			dChamberO2ppm_1;	//FDC모듈에서 처리
// 	double			dChamberO2ppm_2;	//FDC모듈에서 처리
// 	double			dChamberH2O;		//FDC모듈에서 처리
// 	double			dChamberPress;		//FDC모듈에서 처리
// 	
// 
// 	//MOTOR
// 	double			dHomePosX; //Main에서 처리
// 	double			dHomePosY; //Main에서 처리
//  	double			dSpeedX; //Main에서 처리
//  	double			dSpeedY; //Main에서 처리
// 
// 	BOOL			bionizerstatus;	//Main에서 처리 
// 
// 	//FFU 정보		
// 	double          dFanSpeed[32];
// 
// 	//Motor 부하율 
// 	double			GantryRMSMaster;	//ksk 20160222 add
// 	double			GantryRMSSlave;
// 	double			CrossRMSHead1;
// 	double			CrossRMSHead2;		//end
// 
// 	double			dAirPurgeValue;		//ksk 20160324 add
// 	
// 	// eh.kim 20170512 add.
// 	double			dIllumination1;		
// 	double			dIllumination2;		
// 	double			dIllumination3;		
// 	double			dIllumination4;		
// 	//
// 
// 	//jh.kang 20171025 add
// 	BOOL			bHead1_Connect;
// 	BOOL			bHead1_Alarm;
// 	double			dHead1_Runtime;
// 	BOOL			bHead2_Connect;
// 	BOOL			bHead2_Alarm;
// 	double			dHead2_Runtime;
// 	BOOL			bHead3_Connect;
// 	BOOL			bHead3_Alarm;
// 	double			dHead3_Runtime;
// 	BOOL			bHead4_Connect;
// 	BOOL			bHead4_Alarm;
// 	double			dHead4_Runtime;
// 	//
// 
// 	double			dFDCData[300];
// 	DWORD			lLiveCheck;	
// }FDC_Shared_ICM, *PFDC_Shared_ICM;
// 
// //A3 ICP
// typedef struct
// {
// 	//SW Ver.
// //	BOOL			bMetroVer;		//Main에서 처리
// 	double			dMetroVer;		//Main에서 처리 //20170303 lhj add
// 	BOOL			bCimVer;		//cim에서 처리
// 	
// 	//HDD 용량
// 	int				nMetroHDD_D;	//Main에서 처리
// 	int				nMetroHDD_E;	//Main에서 처리		
// 	int				nCimHDD_D;		//cim에서 처리
// 	int				nCimHDD_E;		//cim에서 처리
// 	
// 	//Lamp Time
// 	double			dLampTime[20]; //Main에서 처리
// 	
// // 	//압력 (Main 1차)
// // 	double			dMainN2Air1;	//FDC모듈에서 처리 N2      PressureSwitch01Main		D3010
// // 	double			dMainN2Air2;	//FDC모듈에서 처리 N2      PressureSwitch02Main		D3011
// // 	double			dIonizerPress;	//FDC모듈에서 처리		   PressureSwitch03Ionizer  D3012
// // 	double			dMainVacuum1;	//FDC모듈에서 처리 D3013
// // 	double			dMainVacuum2;	//FDC모듈에서 처리 D3014
// // 	double			dMainVacuum3;	//FDC모듈에서 처리 D3015
// // 	double			dMainVacuum4;	//FDC모듈에서 처리 D3016
// // 	double			dMainVacuum5;	//FDC모듈에서 처리 D3017
// // 	
// // 	//압력 (Main 2차)
// // 	double			dIsolaterPress;			//FDC모듈에서 처리 N2       PressureSwitch03Isolator		D3020
// // 	double			dMainCDAAir;			//FDC모듈에서 처리 N2       Air     PressureSwitch01Min		D3021
// // 	double			dAirPressSwitch02SACC;	//FDC모듈에서 처리 			Air     PressureSwitch01Main	D3022
// // 	double			dChuckVacuum1;	//FDC모듈에서 처리 Vacuum  PressureSwitch  01 D3023
// // 	double			dChuckVacuum2;	//FDC모듈에서 처리 Vacuum  PressureSwitch  02 D3024
// // 	double			dChuckVacuum3;	//FDC모듈에서 처리 Vacuum  PressureSwitch  03 D3025
// // 	double			dChuckVacuum4;	//FDC모듈에서 처리 Vacuum  PressureSwitch  04 D3026
// // 	double			dChuckVacuum5;	//FDC모듈에서 처리 Vacuum  PressureSwitch  05 D3027
// 	
// 	//온도
// 	double			dPCTemp[5];	//FDC모듈에서 처리
// 	
// 	//전력량 SEM Controller
// 	double			dN01TPW1;	//CIM에서 처리 N_01_T_PW (KWh) UPS 적산 전력량 
// 	double			dN01TPW2;	//CIM에서 처리 N_01_I_PW (kW)  UPS 순시 전력량 
// 	double			dN01TPW3;	//CIM에서 처리 N_01_R_VOL (V)  UPS R상 전압
// 	double			dN01TPW4;	//CIM에서 처리 N_01_T_VOL (V)  UPS T상 전압 
// 	double			dN01TPW5;	//CIM에서 처리 N_01_R_CUR (A)  UPS R상 전류
// 	double			dN01TPW6;	//CIM에서 처리 N_01_T_CUR (A)  UPS T상 전류
// 	double			dN02TPW7;	//CIM에서 처리 N_02_T_PW (kWh) GPS 적산 전력량 
// 	double			dN02TPW8;	//CIM에서 처리 N_02_I_PW  (kW) GPS 순시 전력량
// 	double			dN02TPW9;	//CIM에서 처리 N_02_R_VOL (V)  GPS R상 전압
// 	double			dN02TPW10;	//CIM에서 처리 N_02_T_VOL (V)  GPS T상 전압
// 	double			dN02TPW11;	//CIM에서 처리 N_02_R_CUR (A)  GPS R상 전류
// 	double			dN02TPW12;	//CIM에서 처리 N_02_T_CUR (A)  GPS T상 전류
// 	double			dN02TPW13;	//CIM에서 처리 N_01_CDA        CDA Quick Purge
// 	double			dN02TPW14;	//CIM에서 처리 N_01_N2         N2 Quick Purge
// 	
// // 	//Purifier 정보
// // 	double			dPuriH2O_2;	//CIM에서 처리
// // 	double			dPuriO2_2;	//CIM에서 처리
// // 	double			dPuriO2ppm_2;	//CIM에서 처리
// // 	double			dPuriPress_2;	//CIM에서 처리
// // 	
// // 	//Chamber 정보
// // 	double			dPuriH2O;			//FDC모듈에서 처리 Main    H20     Sensor  01		D3030
// // 	double			dPuriPress;			//FDC모듈에서 처리 Main    PressureSensor  01	D3031
// // 	double			dSACCPuriH2O;		//FDC모듈에서 처리 SACC    H20     Sensor  01		D3032
// // 	double			dSACCChamberPress;	//FDC모듈에서 처리 SACC    PressureSensor  01	D3033
// // 	double			dPuriO2;			//FDC모듈에서 처리  
// // 	double			dPuriO2ppm;			//FDC모듈에서 처리   
// // 	double			dPuriO2_SACC;		//FDC모듈에서 처리  
// // 	double			dPuriO2_SACCppm;	//FDC모듈에서 처리  
// 		
// 	BOOL			bionizerstatus;		//Main에서 처리 
// 	
// 	//FFU 정보		
// 	double          dFanSpeed[32];
// 	
// // 	//Motor 부하율 
// // 	double			GantryRMSMaster_WT_X;	//Main에서 처리
// // 	double			GantryRMSMaster_CAM_X;  //Main에서 처리
// 
// 	double			dAirPurgeValue;		//ksk 20160324 add
// 
// // jsyoon 20171121 remove
// // 	//jh.kang 20171025 add
// // 	BOOL			bHead1_Connect;
// // 	BOOL			bHead1_Alarm;
// // 	double			dHead1_Runtime;
// // 	BOOL			bHead2_Connect;
// // 	BOOL			bHead2_Alarm;
// // 	double			dHead2_Runtime;
// // 	BOOL			bHead3_Connect;
// // 	BOOL			bHead3_Alarm;
// // 	double			dHead3_Runtime;
// // 	BOOL			bHead4_Connect;
// // 	BOOL			bHead4_Alarm;
// // 	double			dHead4_Runtime;
// // 	//
// 
// 	double			dFDCData[300];
// 
// }FDC_Shared_ICP, *PFDC_Shared_ICP;
// 
// //A3 EVEN
// typedef struct
// {
// 	//SW Ver.
// 	BOOL			bMetroVer;		//Main에서 처리
// 	BOOL			bServerVer;		//Main에서 처리
// 	BOOL			bCimVer;		//cim에서 처리
// 	
// 	//HDD 용량
// 	int				nMetroHDD_D;	//Main에서 처리
// 	int				nMetroHDD_E;	//Main에서 처리		
// 	int				nCimHDD_D;		//cim에서 처리
// 	int				nCimHDD_E;		//cim에서 처리
// 	
// 	//SE INTENSITY
// 	double			dAVGSignal;	//Main에서 처리
// 	double			dDUVSignal;	//Main에서 처리
// 	double			dUVSignal;	//Main에서 처리
// 	double			dIRSignal;	//Main에서 처리
// 	
// 	//Lamp Time
// 	BOOL			bSELampAOn;		
// 	double			dSELampTimeA;	
// 	double			dSELampTimeB;	
// 	double			dSRLampTime;	
// 	//SE Angle
// 	double			dAngle;	 //Main에서 처리
// 	
// 	//SRM
// 	double			dSRMThickSE100A;	
// 	double			dSRMThickSE250A;	
// 	double			dSRMThickSE600A;	
// 	double			dSRMThickSR2000A;	
// 	double			dSRMThickSR5000A;	
// 	double			dSRMThickSR10000A;  
// 	
// 	//압력 (Main 1차)
// 	double			dMainCDAAir1;	//FDC모듈에서 처리
// 	double			dMainCDAAir2;	//FDC모듈에서 처리
// 	
// 	double			dMainVacuum1;	//FDC모듈에서 처리
// // 	double			dMainVacuum2;	//FDC모듈에서 처리
// // 	double			dMainVacuum3;	//FDC모듈에서 처리
// // 	double			dMainVacuum4;	//FDC모듈에서 처리
// // 	double			dMainVacuum5;	//FDC모듈에서 처리
// 	
// 	//압력 (Main 2차)
// 	double			dFixturePress;	//FDC모듈에서 처리
// 	double			dIsolaterPress;	//FDC모듈에서 처리
// //	double			dShutterPress;	//FDC모듈에서 처리
// // 	double			dAirBrowing1;	//FDC모듈에서 처리
// // 	double			dAirBrowing2;	//FDC모듈에서 처리
// 	double			dVaccumMain1;
// 	double			dVaccumMain2;
// // 	double			dPlatePress1;	//FDC모듈에서 처리
// // 	double			dPlatePress2;	//FDC모듈에서 처리
// 	
// 	double			dIonizerPress;	//FDC모듈에서 처리
// //	double			dFixSuction;	//FDC모듈에서 처리
// 	
// 	//온도
// 	double			dPCTemp;	//FDC모듈에서 처리
// 	
// 	//FFU
// 	double			dFanSpeed1;	//FDC모듈에서 처리
// 	double			dFanSpeed2;	//FDC모듈에서 처리
// 	double			dFanSpeed3;	//FDC모듈에서 처리
// 	double			dFanSpeed4;	//FDC모듈에서 처리
// 	double			dFanSpeed5;	//FDC모듈에서 처리
// 	double			dFanSpeed6;	//FDC모듈에서 처리
// 	
// 	//MOTOR
// 	double			dHomePosX; //Main에서 처리
// 	double			dHomePosY; //Main에서 처리
// // 	double			dSpeedX; //Main에서 처리
// // 	double			dSpeedY; //Main에서 처리
// 	double			dWhiteLampTime;					// White Lamp Time		// pys 20171103 add
// 	double			dUVLampTime;					// UV Lamp Time			// pys 20171103 add
// 	DWORD			lLiveCheck;	// jsyoon 20150113 add
// 	int				nIonizer[4];	// pys 20171204 add
// 
// 	double			dFDCData[300];	//jh.kang 20171025 add
// 	double          dFanSpeed[64];//jh.kang 20171025 add
// }FDC_Shared_EVEN, *PFDC_Shared_EVEN;
// 
// //A3 BP
// typedef struct
// {
// 	int				nCimDataFail;				// SVID 1			
// 	double			dSEDuvSignal;				// SVID 2
// 	double			dSEUvSignal;				// SVID 3
// 	double			dSEIrSignal;				// SVID 4
// 	double			dSEAvgSignal;				// SVID 5
// 	double			dSRAvgSignal;				// SVID 6
// 	BOOL			bSELampAOn;					// SVID 7
// 	double			dSELampTimeA;				// SVID 8
// 	double			dSELampTimeB;				// SVID 9
// 	double			dSRLampTime;				// SVID 10
// 	double			dNGFLampTime;				// SVID 11
// 
// 	// sih 20161226 change 
// // 	double			dSESrmThick100A;			// SVID 12
// // 	double			dSESrmThick250A;			// SVID 13
// // 	double			dSESrmThick600A;			// SVID 14	
//  	double			dSESrmThickRsrv1;			// SVID 12		
//  	double			dSESrmThickRsrv2;			// SVID 13	
// 	//
// 
// 	double			dSRSrmThick500A;			// SVID 14		// sih 20161226 add 
// 	double			dSRSrmThick2000A;			// SVID 15	
// 	double			dSRSrmThick5000A;			// SVID 16	
// 	double			dSRSrmThick10000A;			// SVID 17	
// 	double			dCDAPressure;				// SVID 18
// 	double			dVacuumPressure;			// SVID 19
// 	double			dFixturePressure;			// SVID 20
// 	double			dIsolatorPressure;			// SVID 21
// 	double			dAirBlowingPressure1;		// SVID 22
// 	double			dAirBlowingPressure2;		// SVID 23
// 	double			dPCTemp;					// SVID 24	
// 	double			dFanSpeed1;					// SVID 25
// 	double			dFanSpeed2;					// SVID 26
// 	double			dFanSpeed3;					// SVID 27
// 	double			dFanSpeed4;					// SVID 28
// 	double			dFanSpeed5;					// SVID 29
// 	double			dFanSpeed6;					// SVID 30
// 	double			dFanSpeed7;					// SVID 31
// 	double			dFanSpeed8;					// SVID 32
// 	double			dFanSpeed9;					// SVID 33
// 	double			dFanSpeed10;				// SVID 34
// 	double			dFanSpeed11;				// SVID 35	
// 	double			dFanSpeed12;				// SVID 36
// 	double			dFanSpeed13;				// SVID 37
// 	double			dFanSpeed14;				// SVID 38
// 	int				nIonizerStatus1;			// SVID 39		
// 	int				nIonizerStatus2;			// SVID 40		
// 	int				nIonizerStatus3;			// SVID 41		
// 	int				nIonizerStatus4;			// SVID 42	
// 	double			dXOverLoad;					// SVID 43	
// 	double			dYOverLoad;					// SVID 44	
// 	double			dZOverLoad;					// SVID 45	
// 	double			dMseValue;					// SVID 46
// 	double			dLiftPinFirstUpTime;		// SVID 47		// MetroSys
// 	double			dLiftPinSecondUpTime;		// SVID 48		// MetroSys
// 	double			dLiftpinFirstUpDelayTime;	// SVID 49		// MetroSys
// 	double			dLiftpinFirstUpPos;			// SVID 50		// MetroSys
// 	double			dLiftpinSecondUpPos;		// SVID 51		// MetroSys
// 	double			dLiftPinDownTime;			// SVID 52		// MetroSys
// 	double			dMetroVer;					// SVID 53		// eh.kim 20161226 change.
// 
// 	double			dFDCData[300];	//jh.kang 20171025 add
// 	double          dFanSpeed[64];//jh.kang 20171025 add
// }FDC_Shared_BP, *PFDC_Shared_BP;
// //
// 
// //A3 BP #4 ST8K
// typedef struct
// {
// 	int				nCimDataFail;				// SVID 1			
// 	double			dSRAvgSignal;				// SVID 6
// 	double			dSRLampTime;				// SVID 10
// 	double			dNGFLampTime;				// SVID 11
// 	double			dSRSrmThick2000A;			// SVID 15	
// 	double			dSRSrmThick5000A;			// SVID 16	
// 	double			dSRSrmThick10000A;			// SVID 17	
// 	double			dCDAPressure;				// SVID 18
// 	double			dVacuumPressure;			// SVID 19
// 	double			dFixturePressure;			// SVID 20
// 	double			dIsolatorPressure;			// SVID 21
// 	double			dAirBlowingPressure1;		// SVID 22
// 	double			dAirBlowingPressure2;		// SVID 23
// 	double			dPCTemp;					// SVID 24
// 	double			dFanSpeed[32];				// SVID 25
// 	int				nIonizer[4];				// SVID 39		
// 	double			dXOverLoad;					// SVID 43	
// 	double			dYOverLoad;					// SVID 44	
// 	double			dZOverLoad;					// SVID 45	
// 	double			dMseValue;					// SVID 46
// 	double			dLiftPinFirstUpTime;		// SVID 47		// MetroSys
// 	double			dLiftPinSecondUpTime;		// SVID 48		// MetroSys
// 	double			dLiftpinFirstUpDelayTime;	// SVID 49		// MetroSys
// 	double			dLiftpinFirstUpPos;			// SVID 50		// MetroSys
// 	double			dLiftpinSecondUpPos;		// SVID 51		// MetroSys
// 	double			dLiftPinDownTime;			// SVID 52		// MetroSys
// 	
// 	DWORD			lLiveCheck;	// jsyoon 20150113 add
// }FDC_Shared_BP2, *PFDC_Shared_BP2;
// //
// 
// //A3 TFE
// typedef struct
// {
// //	BOOL			bMetroVer;		
// 	double			dMetroVer;		// eh.kim 20161222 change.
// 	BOOL			bServerVer;		
// 	BOOL			bCimVer;		
// 	int				nMetroHDD_D;	
// 	int				nMetroHDD_E;			
// 	int				nCimHDD_D;		
// 	int				nCimHDD_E;		
// 	double			dAVGSignal;
// 	double			dSRAvgSignal;		//jh.kang 20170202 add
// 	double			dDUVSignal;		
// 	double			dUVSignal;		
// 	double			dIRSignal;		
// 	BOOL			bSELampAOn;		
// 	double			dSELampTimeA;	
// 	double			dSELampTimeB;	
// 	double			dSRLampTime;	
// 	double			dAngle;			
// 	double			dSRMThickSE100A;	
// 	double			dSRMThickSE250A;	
// 	double			dSRMThickSE600A;	
// 	double			dSRMThickSR2000A;	
// 	double			dSRMThickSR5000A;	
// 	double			dSRMThickSR10000A;  
// 	double			dMainCDAAir1;		
// 	double			dMainCDAAir2;		
// 	double			dMainVacuum1;		
// 	double			dFixturePress;		
// 	double			dIsolaterPress;		
// 	double			dVaccumMain1;
// 	double			dVaccumMain2;
// 	double			dIonizerPress;		
// 	double			dPCTemp;			
// 	double			dFanSpeed1;			
// 	double			dFanSpeed2;			
// 	double			dFanSpeed3;			
// 	double			dFanSpeed4;			
// 	double			dFanSpeed5;			
// 	double			dFanSpeed6;			
// 	double			dHomePosX;			
// 	double			dHomePosY;			
// 	double			dLiftPinFirstUpTime;
// 	double			dLiftPinSecondUpTime;
// 	double			dLiftPinThirdUpTime;
// 	double			dLiftPinDownTime;
// 	int				nIonizer[4];	
// 	DWORD			lLiveCheck;	
// 
// 	// sih 20171204 change
// 	//jh.kang 20171025 add
// // 	BOOL			bHead1_Connect;
// // 	BOOL			bHead1_Alarm;
// // 	double			dHead1_Runtime;
// // 	BOOL			bHead2_Connect;
// // 	BOOL			bHead2_Alarm;
// // 	double			dHead2_Runtime;
// // 	BOOL			bHead3_Connect;
// // 	BOOL			bHead3_Alarm;
// // 	double			dHead3_Runtime;
// // 	BOOL			bHead4_Connect;
// // 	BOOL			bHead4_Alarm;
// // 	double			dHead4_Runtime;
// 	//
// 
// 	double			dIonizer_Press1;
// 	double			dIonizer_Press2;
// 	double			dIonizer_Press3;
// 	double			dIonizer_Press4;
// 	//
// 
// 	double			dFDCData[300];//jh.kang 20171025 add
// 	double          dFanSpeed[64];//jh.kang 20171025 add
// }FDC_Shared_TFE, *PFDC_Shared_TFE;
// //
// 
// //A3 SRM
// typedef struct
// {
// 	int				nCimDataFail;				// SVID 1		// MetroSys 
// 	int				nRsContactCnt;				// SVID 2		// MetroSys 
// 	double			dRsSRM;						// SVID 3		// MetroSys 
// 	double			dCDAPressure;				// SVID 4		// PLC 
// 	double			dVacuumPressure;			// SVID 5		// PLC 
// 	double			dFixturePressure;			// SVID 6		// PLC 
// 	double			dAirBlowingPressure1;		// SVID 7		// PLC 
// 	double			dAirBlowingPressure2;		// SVID 8		// PLC 
// 	double			dPCTemp;					// SVID 9		// Sensor 
// 	double			dFanSpeed[32];				// SVID 10 ~ 21	// Sensor	// sih 20170206 change 
// 	int				nIonizerStatus[4];			// SVID 22 ~ 25 // MetroSys 			
// 	double			dXOverLoad;					// SVID 26		// MetroSys 	
// 	double			dYOverLoad;					// SVID 27		// MetroSys 	
// 	double			dCurrentValue;				// SVID 28		// MetroSys 	
// 	double			dVoltageValue;				// SVID 29		// MetroSys 
// 	double			dLiftPinFirstUpTime;		// SVID 30		// MetroSys
// 	double			dLiftPinSecondUpTime;		// SVID 31		// MetroSys
// 	double			dLiftpinFirstUpDelayTime;	// SVID 32		// MetroSys
// 	double			dLiftpinFirstUpPos;			// SVID 33		// MetroSys
// 	double			dLiftpinSecondUpPos;		// SVID 34		// MetroSys
// 	double			dLiftPinDownTime;			// SVID 35		// MetroSys
// 	double			dMetroVer;					// SVID 36		// jh.kang 20170106 add
// 
// 	double			dFDCData[300];	//jh.kang 20171025 add
// 
// }FDC_Shared_SRM, *PFDC_Shared_SRM;
// //
// 
// // 20161226 kks
// //TM
// typedef struct
// {
// 	double			dCDA_PRESS;
// 	double			dGATE;
// 	double			dANGLE;
// 	double			dMAIN_TM_N2;
// 	double			dMLC_N2;
// 	double			dCHAMBER_O2_PPM_DATA_PERSENT_1;
// 	double			dCHAMBER_O2_PPM_DATA_PERSENT_2;
// 	double			dMLC_O2_PPM_DATA_PERSENT_1;
// 	double			dCHAMBER_O2_PPM_DATA_1;
// 	double			dCHAMBER_O2_PPM_DATA_2;
// 	double			dMLC_O2_PPM_DATA_1;
// 	double			dS_RBT_PRESS_DATA_1;
// 	double			dMLC_PRESS_DATA_1;
// 	double			dS_RBT_H2O_DATA_1;
// 	double			dS_RBT_H2O_DATA_2;	//jh.kang 20171030 add
// 	double			dMLC_H2O_DATA_1;
// 	double			dPURIFIER_O2_PERSENT;
// 	double			dPURIFIER_O2_PPM;
// 	double			dPURIFIER_PRESSUR;
// 	double			dPURIFIER_H2O_PPM;
// 	double			dPURIFIER_AIR_PURGE_TIME;
// 	
// 	//온도
// 	double			dPCTemp;					//FDC모듈에서 처리
// 	
// 	//FFU 정보		
// 	double          dFanSpeed[64];
// 	double			dFDCData[300];	//jh.kang 20171025 add
// 
// }FDC_Shared_TM, *PFDC_Shared_TM;
// 

const enum NUM_FDC_WDATA{
	FDC_WDATA_TEMP_1	= 0,
	FDC_WDATA_TEMP_2,
	FDC_WDATA_TEMP_3,
	FDC_WDATA_TEMP_4,
	FDC_WDATA_TEMP_5,
	FDC_WDATA_WT_VACUUM_1,			// VACUUM 
	FDC_WDATA_WT_VACUUM_2,
	FDC_WDATA_WT_VACUUM_3,
	FDC_WDATA_WT_VACUUM_4,
	FDC_WDATA_WT_VACUUM_5,
	FDC_WDATA_CB_O2PPM_1,			// CHAMBER 설비
	FDC_WDATA_CB_O2PPM_2,
	FDC_WDATA_CB_H20PPM,
	FDC_WDATA_CB_PRESS,				
	FDC_WDATA_PACC_H20PPM,			// OV-LI 사용
	FDC_WDATA_END,
};

typedef struct {
	char		szName[32];
	char		szData[32];
	double		nMax;
	double		nMin;
}RDATA, *PRDATA;

typedef struct
{
	int				nMainDataCount;			// Main SW에서 올려주는 Data 개수
	RDATA			stMainWData[1000];		// Main SW에서 올려주는 정보

	DWORD			lLiveCheck;				// FDC SW Tick count
	RDATA			stFDCWData[100];		// FDC SW에서 올려주는 정보 NUM_FDC_WDATA 순으로 입력
}FDC_For_Shared, *PFDC_For_Shared;

typedef struct  {
	FDC_For_Shared FDCValue;
}VTM_FOR_FDC, *PVTM_FOR_FDC;



