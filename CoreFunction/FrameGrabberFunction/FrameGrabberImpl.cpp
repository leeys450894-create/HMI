#include "pch.h"
#include "FrameGrabberImpl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CFrameGrabberImpl::CFrameGrabberImpl()
{
}

CFrameGrabberImpl::~CFrameGrabberImpl(void)
{
}

//=========================================================================
// INTERFACE 관련
//=========================================================================
bool CFrameGrabberImpl::InitializeGrabber(long nImageSizeX, long nImageSizeY, double dFrameRate)
{
	return false;
}

bool CFrameGrabberImpl::FinalizeGrabber()
{
	return false;
}

bool CFrameGrabberImpl::IsOpenGrabber()
{
	return false;
}


bool CFrameGrabberImpl::SnapImage(short* pbyImage)
{
	return false;
}

bool CFrameGrabberImpl::GetSnapImage(long nPulseWidth_ms, BYTE *pbyImage)
{
	return false;
}

bool CFrameGrabberImpl::ResetGrabber()
{
	return false;
}



// 파라미터 설정
void CFrameGrabberImpl::SetParam_InterfaceName(CString strValue)
{
}

void CFrameGrabberImpl::SetParam_GrabMode(CString strValue)
{
}

void CFrameGrabberImpl::SetParam_HBayerPattern(long nValue)
{

}

void CFrameGrabberImpl::SetParam_RotateAngle(long nValue)
{

}

long CFrameGrabberImpl::GetParam_RotateAngle()
{
	return 0;
}

void CFrameGrabberImpl::SetParam_WhiteBalanceRGB(double dR, double dG, double dB)
{

}




bool CFrameGrabberImpl::SetParam_CameraReset()
{
	return false;
}

bool CFrameGrabberImpl::SetParam_CameraResetNanoStagePos()
{
	return false;
}

bool CFrameGrabberImpl::SetParam_CameraGain(long nValue)
{
	return false;
}

long CFrameGrabberImpl::GetParam_CameraGain()
{
	return 1;
}

bool CFrameGrabberImpl::SetParam_CameraExposureTime_ms(long nValue)
{
	return false;
}

long CFrameGrabberImpl::GetParam_CameraExposureTime_ms()
{
	return 0;
}

long CFrameGrabberImpl::GetParam_CameraAcqWidth()
{
	return 0;
}

long CFrameGrabberImpl::GetParam_CameraAcqHeighth()
{
	return 0;
}
