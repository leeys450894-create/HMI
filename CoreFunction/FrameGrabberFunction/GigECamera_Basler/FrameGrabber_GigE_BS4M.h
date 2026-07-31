#pragma once

#include "..\FrameGrabberImpl.h"

class CFrameGrabber_BS4M;
class CFrameGrabber_GigE_BS4M : public CFrameGrabberImpl
{
	CFrameGrabber_BS4M *m_pImpl;

public:
	virtual bool    InitializeGrabber(long nImageSizeX, long nImageSizeY, double dFrameRate) override;
	virtual bool    FinalizeGrabber() override;
	virtual bool    IsOpenGrabber() override;

	virtual bool    SnapImage( short* pbyImage) override;
	virtual bool    GetSnapImage(long nPulseWidth_ms, BYTE *pbyImage) override;
	virtual bool	ResetGrabber() override;

	//------------------------------------------------------
	// 파라미터 설정
	//------------------------------------------------------
	virtual void    SetParam_InterfaceName(CString strInterface) override;
	virtual void	SetParam_HBayerPattern(long nIndex) override;
	virtual void	SetParam_RotateAngle(long nValue) override;
	virtual long	GetParam_RotateAngle() override;

	virtual bool    SetParam_CameraGain(long nCCDGain) override;
	virtual long    GetParam_CameraGain() override;
	virtual bool    SetParam_CameraExposureTime_ms(long nCCDExposure) override;
	virtual long    GetParam_CameraExposureTime_ms() override;

	virtual long	GetParam_CameraAcqWidth() override;
	virtual long	GetParam_CameraAcqHeighth() override;

public:
	CFrameGrabber_GigE_BS4M();
	virtual ~CFrameGrabber_GigE_BS4M(void);
};

