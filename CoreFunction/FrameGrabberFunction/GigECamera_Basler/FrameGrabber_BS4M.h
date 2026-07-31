#pragma once

#include <nivision.h>
#include <niimaq.h>
#include <NIIMAQdx.h>
#pragma comment(lib, "imaq.lib")
#pragma comment(lib, "NIImaqdx.lib")

#include "..\FrameGrabberImplDefine.h"

class CFrameGrabber_BS4M	// 4M Color Camera
{
	int				m_uiAcqWinWidth;
    int				m_uiAcqWinHeight;

	bool			m_bFGOpen;
    INTERFACE_ID	m_uiIntfaceId;
    SESSION_ID		m_uiSessionId;
    CString			m_strIntfName;
	long			m_nHBayerPattern;
	long			m_nRotateAngle;

	Image			*m_pAcqImage;
	Image			*m_pRotImage;
	Image			*m_pDstImage_R;
	Image			*m_pDstImage_G;
	Image			*m_pDstImage_B;
	Image			*m_pAcqImageRGB;

    long			m_nCCDGain;
    long			m_nCCDExposure_us;

public:
	bool InitializeGrab(long nFovSizeX, long nFovSizeY, double dFrameRate);
	bool InitInterface();
	bool InitCamSerial();
	bool InitGrabSetup();
	bool InitResource(long nFovSizeX, long nFovSizeY);
	bool IsOpenGrabber();
	bool ResetGrabber();
	bool RefreshGrabber();
	bool FinalizeGrab();

	// 이미지 획득을 위해 공유되는 함수
	bool GetLiveImage(bool bFlag);
	bool SnapImage(short* pbyImage);
	bool GetSnapImage(long nPulseWidth_ms, BYTE *pbyImage);

	// 카메라에 파라미터 설정 및 CCD Dialog 와 연동되는 함수들
	long GetParam_FOVImageSizeX() const						{ return m_uiAcqWinWidth;			}
	long GetParam_FOVImageSizeY() const						{ return m_uiAcqWinHeight;			}
	void SetParam_InterfaceName(LPCTSTR lpszInterface)		{ m_strIntfName = lpszInterface;	}
	void SetParam_HBayerPattern(long nIndex);
	void SetParam_RotateAngle(long nValue);
	long GetParam_RotateAngle();

	bool SetParam_CameraGain(long nCCDGain);
	long GetParam_CameraGain();
    bool SetParam_CameraExposureTime_ms(long nCCDExposure);
    long GetParam_CameraExposureTime_ms();
private:
	void CopyImageData(Image *pSrcImage, BYTE *pDstPtr);

public:
	CFrameGrabber_BS4M();
	~CFrameGrabber_BS4M();
};