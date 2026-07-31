#pragma once

//FFU
void WINAPI API_SM_SetFFU_PV(int iFFUID, int iPV, int nIdex );
void WINAPI API_SM_SetFFU_ALARM(int iFFUID, int iALARM, int nIdex);
void WINAPI API_SM_SetFFU_SV(int iFFUID, int iSV, int nIdex);
int WINAPI API_SM_GetFFU_PV(int iFFUID, int nIdex);
int WINAPI API_SM_GetFFU_ALARM(int iFFUID, int nIdex);
int WINAPI API_SM_GetFFU_SV(int iFFUID, int nIdex);
BOOL WINAPI API_SM_GetFFU_ALARM_CHECK(int iFFUID, int nIdex);
void WINAPI API_SM_SetFFU_ALARM_CHECK(int iFFUID, int nValue, int nIdex);//20140721 added by Jay Hong
void WINAPI API_SM_SetFFU_ALLOFF(BOOL bAllOff, int nIdex);
void WINAPI API_SM_SetFFU_RESTORE(BOOL bRestore, int nIdex);
BOOL WINAPI API_SM_GetFFU_ALLOFF(int nIdex);
BOOL WINAPI API_SM_GetFFU_RESTORE(int nIdex);
void WINAPI API_SM_SetFFU_NUM(int nFFU, int nIdex);
int  WINAPI API_SM_GetFFU_NUM(int nIdex);
int  WINAPI API_SM_GetFFU_LiveCheck(int nIdex);
void WINAPI API_SM_SetFFU_LiveCheck(int iTime, int nIdex);

//Head Position
void WINAPI API_SM_SetHEAD_PosX(int iHead, int iPosX, int nIdex);
int WINAPI API_SM_GetHEAD_PosX(int iHead, int nIdex);
void WINAPI API_SM_SetHEAD_PosY(int iHead, int iPosY, int nIdex);
int WINAPI API_SM_GetHEAD_PosY(int iHead, int nIdex);
void WINAPI API_SM_SetHEAD_PosZ(int iHead, int iPosZ, int nIdex);
int WINAPI API_SM_GetHEAD_PosZ(int iHead, int nIdex);
void WINAPI API_SM_SetHEAD_PosSubX(int iHead, int iPosSubX, int nIdex);
int WINAPI API_SM_GetHEAD_PosSubX(int iHead, int nIdex);

//PLC Data
void WINAPI API_SM_SetPLC_PLCData_M(int iAddr, BOOL bSet, int nIdex);
BOOL WINAPI API_SM_GetPLC_PLCData_M(int iAddr, int nIdex);
void WINAPI API_SM_SetPLC_PLCData_F(int iAddr, BOOL bSet, int nIdex);
BOOL WINAPI API_SM_GetPLC_PLCData_F(int iAddr, int nIdex);
void WINAPI API_SM_SetPLC_ALL(int idex, BOOL* bAddr, int nIdex);
BOOL* WINAPI API_SM_GetPLC_ALL_M(int nIdex);
BOOL* WINAPI API_SM_GetPLC_ALL_F(int nIdex);
BOOL WINAPI API_SM_GetPLC_Check();

