#pragma once

class CGlassAlignImpl;
class CGlassAlignFunction
{
	CGlassAlignImpl	*m_pCtrlImpl;
	

	//------------------------------------------------------------------
	// 외부 접근 가능 함수
	//------------------------------------------------------------------
public:
    CString	GetDeviceName();

    // 전용 함수 - 시퀀스 연동
    void ResetAlignMatrix();
	void SetParamRefPos(int index, double posX, double posY, double posZ);
	void SetParamDetPos(int index, double posX, double posY, double posZ);
    bool CalcAlignMatrix();
	bool IsAlignComplete();
	void GetAlignedMotionPos(double posX, double posY, double &NewX, double &NewY);
    void GetAlignedMatrixPos(double posX, double posY, double &NewX, double &NewY);
	void GetAlignResult(double &dtheta, double &deltaX, double &deltaY);
	
public:
	CGlassAlignFunction(CString strDeviceModel, CString strFilePath, CString strCategory);
	~CGlassAlignFunction();	
};

