#pragma once

class CVCRImpl;
class CVCRFunction
{
	CVCRImpl	*m_pCtrlImpl;
	

	//------------------------------------------------------------------
	// 외부 접근 가능 함수
	//------------------------------------------------------------------
public:
    bool	InitDevice(int port);
	bool	CloseDevice();
    CString	GetDeviceName();
    bool	IsConnected();

    // 전용 함수 - 시퀀스 연동
	bool	Req_VCR();
	bool	Res_VCR(CString &value);
	
public:
	CVCRFunction(CString strAppPath, CString strConfigPath, CString strDeviceModel);
	~CVCRFunction();	
};

