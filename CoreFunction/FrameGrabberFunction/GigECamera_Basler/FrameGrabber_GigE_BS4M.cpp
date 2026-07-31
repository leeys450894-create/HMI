#include "pch.h"
#include "FrameGrabber_GigE_BS4M.h"
#include "FrameGrabber_BS4M.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CFrameGrabber_GigE_BS4M::CFrameGrabber_GigE_BS4M()
{
    m_pImpl = new CFrameGrabber_BS4M;
}

CFrameGrabber_GigE_BS4M::~CFrameGrabber_GigE_BS4M(void)
{
	if(m_pImpl)
	{
	    delete m_pImpl;
	    m_pImpl = nullptr;
	}
}

bool CFrameGrabber_GigE_BS4M::InitializeGrabber(long nImageSizeX, long nImageSizeY, double dFrameRate)
{
	if(m_pImpl)
	{
		return m_pImpl->InitializeGrab(nImageSizeX, nImageSizeY, dFrameRate);
	}

	return false;
}

bool CFrameGrabber_GigE_BS4M::FinalizeGrabber()
{
	if(m_pImpl)
	{
		return m_pImpl->FinalizeGrab();
	}

	return false;
}

bool CFrameGrabber_GigE_BS4M::IsOpenGrabber()
{
	if(m_pImpl)
	{
		return m_pImpl->IsOpenGrabber();
	}

	return false;
}

bool CFrameGrabber_GigE_BS4M::SnapImage(short* pbyImage)
{
	if (m_pImpl)
	{
		return m_pImpl->SnapImage(pbyImage);
	}

	return false;
}

bool CFrameGrabber_GigE_BS4M::GetSnapImage(long nPulseWidth_ms, BYTE *pbyImage)
{
	if(m_pImpl)
	{
	    return m_pImpl->GetSnapImage(nPulseWidth_ms, pbyImage);
	}

	return false;
}



bool CFrameGrabber_GigE_BS4M::ResetGrabber()
{
	if (m_pImpl)
	{
		return m_pImpl->ResetGrabber();
	}

	return false;
}




//------------------------------------------------------
// 파라미터 설정
//------------------------------------------------------
void CFrameGrabber_GigE_BS4M::SetParam_InterfaceName(CString strInterface)
{
	if(m_pImpl)
	{
	    m_pImpl->SetParam_InterfaceName(strInterface);
	}
}

void CFrameGrabber_GigE_BS4M::SetParam_HBayerPattern(long nIndex)
{
	if(m_pImpl)
	{
		m_pImpl->SetParam_HBayerPattern(nIndex);
	}
}

void CFrameGrabber_GigE_BS4M::SetParam_RotateAngle(long nValue)
{
	if(m_pImpl)
	{
	    m_pImpl->SetParam_RotateAngle(nValue);
	}
}

long CFrameGrabber_GigE_BS4M::GetParam_RotateAngle()
{
	if(m_pImpl)
	{
		return m_pImpl->GetParam_RotateAngle();
	}

	return 0;
}

bool CFrameGrabber_GigE_BS4M::SetParam_CameraGain(long nCCDGain)
{
	if(m_pImpl)
	{
		return m_pImpl->SetParam_CameraGain(nCCDGain);
	}

	return false;
}

long CFrameGrabber_GigE_BS4M::GetParam_CameraGain()
{
	if(m_pImpl)
	{
		return m_pImpl->GetParam_CameraGain();
	}

	return 1;
}

bool CFrameGrabber_GigE_BS4M::SetParam_CameraExposureTime_ms(long nCCDExposure)
{
	if(m_pImpl)
	{
		return m_pImpl->SetParam_CameraExposureTime_ms(nCCDExposure);
	}

	return false;
}

long CFrameGrabber_GigE_BS4M::GetParam_CameraExposureTime_ms()
{
	if(m_pImpl)
	{
		return m_pImpl->GetParam_CameraExposureTime_ms();
	}

	return 0;
}

long CFrameGrabber_GigE_BS4M::GetParam_CameraAcqWidth()
{
	if (m_pImpl)
	{
		return m_pImpl->GetParam_FOVImageSizeX();
	}

	return 0;
}

long CFrameGrabber_GigE_BS4M::GetParam_CameraAcqHeighth()
{
	if (m_pImpl)
	{
		return m_pImpl->GetParam_FOVImageSizeY();
	}

	return 0;
}