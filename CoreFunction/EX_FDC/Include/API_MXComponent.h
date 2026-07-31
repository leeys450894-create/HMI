// API_MXComponent.h: interface for the API_MXComponent class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_API_MXCOMPONENT_H__3412424F_506A_4355_AD8B_0DD3D8DD6597__INCLUDED_)
#define AFX_API_MXCOMPONENT_H__3412424F_506A_4355_AD8B_0DD3D8DD6597__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//////////////////////////////////////////////////////////////////////
//UINT index : MXComponent 접근 포인터 배열 수
//같은 DLL 사용할 경우 먼저 호출한 DLL 에서만 접속되고 나중에 호출되는 DLL 에서 접속안되는 문제가 있어 배열로 사용
//총 5개까지 접속 가능 
//////////////////////////////////////////////////////////////////////
#define COMPONENT_SUCCESS			 1
#define COMPONENT_NOT_INITIALIZE	-1

//MXComponent 설치 후 Commonication Setup Utility 프로그램에 Setting된 StationNumber를 인자로 넣어주면 됩니다.
//ex) BOOL bRet = API_MXComponent_Initialize(0, 1, GetSafeHwnd());
long	WINAPI API_MXComponent_Initialize(UINT index, int iStationNum, HWND hParentWnd);

long	WINAPI API_MXComponent_Close(UINT index);

//각 함수는 실패시 에러코드 Number를 리턴한다. 리턴값을 lErrorCode에 넣어주면 에러코드 확인 가능
CString WINAPI API_MXComponent_GetErrorText(UINT index, long lErrorCode);

//Bit 번지 값 읽기
//strDeviceText  : Bit Device 기호(X, Y, M, L, B, F, SB, V, S)
//iDeviceNum : Set하기 원하는 시작 Bit번지 주소
long	WINAPI API_MXComponent_GetMemoryBit(UINT index, CString strDeviceText, long* lplData);

//Bit 번지에 Set하기
//strDeviceText  : Bit Device 기호(X, Y, M, L, B, F, SB, V, S)
//iDeviceNum : Set 하기 원하는 Bit번지 주소
long	WINAPI API_MXComponent_SetMemoryBit(UINT index, CString strDeviceText, BOOL bSet);

//Bit 메모리 영역에 원하는 크기 만큼 TRUE, FALSE 할당
//strDeviceText  : Bit Device Address(X, Y, M, L, B, F, SB, V, S)
//lSize : Set 하기 원하는 M번지 갯수
//bSet : TRUE 또는 FALSE
//EX)	strDeviceText : M4530, lSize : 20, bSet : TRUE
//		M4530부터 M4550까지 TRUE로 Set 한다.
long	WINAPI API_MXComponent_SetMemoryBitBlock(UINT index, CString strDeviceText, long lSize, BOOL bSet);

long	WINAPI API_MXComponent_SetMemoryBitBlock2(UINT index, CString strDeviceText, long lSize, long* lplData);

//Bit 메모리 영역을 원하는 사이즈 만큼 버퍼에 담아 온다.
//strDeviceText  : Bit Device Address(X, Y, M, L, B, F, SB, V, S)
//lSize : 읽기 원하는 메모리 크기 = 배열의 크기(포인터로 넘기는 배열의 크기와 일치해야 함)
//lplBitData : 배열 포인터
long	WINAPI API_MXComponent_GetMemoryBitBlock(UINT index, CString strDeviceText, long lSize, long* lplBitData);

//Word 단위로 원하는 만큼 Word번지 읽기
//strDeviceText : Word Device Address(T, C, D, W, SW)
//lSize : 읽기 원하는 메모리 크기 = 배열의 크기(포인터로 넘기는 배열의 크기와 일치해야 함)
//lplData : 배열 포인터
long	WINAPI API_MXComponent_GetMemoryWordBlock(UINT index, CString strDeviceText, long lSize, float* fpData, UINT nSize = 2);

//Word 단위로 원하는 사이즈 만큼 D번지에 쓰기
//////////////////////////////////////
//-32768~32767 범위의 숫자 쓰기 가능//
//////////////////////////////////////
//strDeviceText : Word Device Address(T, C, D, W, SW)
//lSize : 쓰기 원하는 메모리 크기 = 배열의 크기(포인터로 넘기는 배열의 크기와 일치해야 함)
//lplData : 배열 포인터
long	WINAPI API_MXComponent_SetMemoryWordBlock(UINT index, CString strDeviceText, long lSize, float* fpData, UINT nSize = 2);

//Word 단위로 원하는 만큼 Word번지 읽기
//strDeviceText : Word Device Address(T, C, D, W, SW)
//lSize : 읽기 원하는 메모리 크기 = 배열의 크기(포인터로 넘기는 배열의 크기와 일치해야 함)
//lplData : 배열 포인터
long	WINAPI API_MXComponent_GetMemoryWordBlock2(UINT index, CString strDeviceText, long lSize, TCHAR* tpData);

//Word 단위로 원하는 사이즈 만큼 D번지에 쓰기
//////////////////////////////////////
//-32768~32767 범위의 숫자 쓰기 가능//
//////////////////////////////////////
//strDeviceText : Word Device Address(T, C, D, W, SW)
//lSize : 쓰기 원하는 메모리 크기 = 배열의 크기(포인터로 넘기는 배열의 크기와 일치해야 함)
//lplData : 배열 포인터
long	WINAPI API_MXComponent_SetMemoryWordBlock2(UINT index, CString strDeviceText, long lSize, TCHAR* tpData);

//Word 단위로 원하는 만큼 Word번지 읽기
//strDeviceText : Word Device Address(T, C, D, W, SW)
//lSize : 읽기 원하는 메모리 크기 = 배열의 크기(포인터로 넘기는 배열의 크기와 일치해야 함)
//lplData : 배열 포인터
long	WINAPI API_MXComponent_GetMemoryWordBlock3(UINT index, CString strDeviceText, long lSize, long* lpData, UINT nSize = 2);

//Word 단위로 원하는 사이즈 만큼 D번지에 쓰기
//////////////////////////////////////
//-32768~32767 범위의 숫자 쓰기 가능//
//////////////////////////////////////
//strDeviceText : Word Device Address(T, C, D, W, SW)
//lSize : 쓰기 원하는 메모리 크기 = 배열의 크기(포인터로 넘기는 배열의 크기와 일치해야 함)
//lplData : 배열 포인터
long	WINAPI API_MXComponent_SetMemoryWordBlock3(UINT index, CString strDeviceText, long lSize, long* lpData, UINT nSize = 2);

#endif // !defined(AFX_API_MXCOMPONENT_H__3412424F_506A_4355_AD8B_0DD3D8DD6597__INCLUDED_)
