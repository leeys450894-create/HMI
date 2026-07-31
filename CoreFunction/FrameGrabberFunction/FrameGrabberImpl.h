#pragma once

#include "FrameGrabberImplDefine.h"

class CFrameGrabberImpl
{

//------------------------------------------------------------------
// 공통 인터페이스 함수
//------------------------------------------------------------------
public:
    virtual bool    InitializeGrabber(long nImageSizeX, long nImageSizeY, double dFrameRate);
    virtual bool    FinalizeGrabber();
    virtual bool    IsOpenGrabber();
    virtual bool    SnapImage(short* pbyImage);	// pure virtual
    virtual bool    GetSnapImage(long nPulseWidth_ms, BYTE *pbyImage);	// pure virtual
	virtual bool	ResetGrabber();
    //virtual bool    GetSnapImageSet(long nPulseWidth_ms, BYTE *pbyImage);	// pure virtual

    // 파라미터 설정
    virtual void    SetParam_InterfaceName(CString strValue);
    virtual void    SetParam_GrabMode(CString strValue);
    virtual	void	SetParam_HBayerPattern(long nValue);
    virtual	void	SetParam_RotateAngle(long nValue);
    virtual	long	GetParam_RotateAngle();
    virtual void	SetParam_WhiteBalanceRGB(double dR, double dG, double dB);

    virtual bool    SetParam_CameraReset();
	virtual bool	SetParam_CameraResetNanoStagePos();
    virtual bool    SetParam_CameraGain(long nValue);
    virtual long    GetParam_CameraGain();
    virtual bool    SetParam_CameraExposureTime_ms(long nValue);
    virtual long    GetParam_CameraExposureTime_ms();
	virtual long	GetParam_CameraAcqWidth();
	virtual long	GetParam_CameraAcqHeighth();

public:
    CFrameGrabberImpl();
    virtual ~CFrameGrabberImpl();
};
