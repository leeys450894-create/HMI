/*==============================================
FILE				: API_SerialCtrl.h.  
DESCRIPT		: API header file for the SERIALCOM DLL
VERSION		: v1.0.0.3
COPYRIGHT	: January 2013, K-MAC Co., Ltd. by shlee (shlee@kmac.com)
==============================================*/


#ifndef _SERIALCOM_H_
#define _SERIALCOM_H_
#pragma once

// API_SerialCtrl.h

//Port Open
//Parameter :
//nPort : 1 - 256
//dwRate : Baud Rate
//bDataBits : 4 - 8
//bParity : NOPARITY, ODDPARITY, EVENPARITY, MARKPARITY, SPACEPARITY
//bStopBits : ONESTOPBIT,ONE5STOPBITS,TWOSTOPBITS 

//Return :
//Success return Port Nbr,
//Fail to open return 0.
//Port Nbr not available return -1.
//Baud Rate Not available return -2.
//Already Opened return -3.
int WINAPI api_SerialCom_Open(int nPort,DWORD dwRate,BYTE bDataBits, BYTE bParity, BYTE bStopBits, BOOL bDtrEnable = TRUE);

//Port Close
void WINAPI api_SerialCom_Close(int nPort);
//Port Close All
void WINAPI api_SerialCom_CloseAll();

//Is Port Opened?
BOOL WINAPI api_SerialCom_IsOpened(int nPort);
//Clear Received Buffer
BOOL WINAPI api_SerialCom_ClearReceiveBuffer(int nPort);
//Check How Many Bytes are Arrived In Receive-Que
int WINAPI api_SerialCom_GetDataInQueue(int nPort);

//Port Read ( CString, char, BYTE )
BOOL WINAPI api_SerialCom_Read(int nPort, char* szDataRead,int nLength);
BOOL WINAPI api_SerialCom_ReadByte(int iPort, BYTE* pbyteData, int nLength);

//Port write ( CString, char, BYTE )
BOOL WINAPI api_SerialCom_Write(int nPort, char* szDataSend, int nLength);
BOOL WINAPI api_SerialCom_WriteByte(int iPort, const BYTE* pbyteData, int nLength);

HANDLE WINAPI api_SerialCom_GetHwnd(int nPort);

// shlee 20120325 add get last serial error.
void WINAPI api_GetLastSerialError(int iPort, char* szErrorMsg); 

#endif

//