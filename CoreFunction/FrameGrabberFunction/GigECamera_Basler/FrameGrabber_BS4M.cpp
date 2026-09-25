#include "pch.h"
#include "FrameGrabber_BS4M.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CFrameGrabber_BS4M::CFrameGrabber_BS4M()
{
    m_uiAcqWinWidth				= INT_MONITOR_WND_WIDTH;
    m_uiAcqWinHeight			= INT_MONITOR_WND_HEIGHT;

    m_bFGOpen					= false;
    m_uiSessionId				= 0;
    m_uiIntfaceId				= 0;
    m_strIntfName				= _T("img0");
	m_nHBayerPattern			= 0;
	m_nRotateAngle				= 0;

 //   m_pAcqImage					= imaqCreateImage(IMAQ_IMAGE_RGB, FG_IMAGE_BORDER_SIZE);	// Prepare buffer to grab Image
	m_pAcqImage					= imaqCreateImage(IMAQ_IMAGE_U8, FG_IMAGE_BORDER_SIZE);	// Prepare buffer to grab Image
	m_pRotImage					= imaqCreateImage(IMAQ_IMAGE_U8, FG_IMAGE_BORDER_SIZE);	// Prepare buffer to grab Image
	m_pDstImage_R				= imaqCreateImage(IMAQ_IMAGE_U8,  FG_IMAGE_BORDER_SIZE);
	m_pDstImage_G				= imaqCreateImage(IMAQ_IMAGE_U8,  FG_IMAGE_BORDER_SIZE);
	m_pDstImage_B				= imaqCreateImage(IMAQ_IMAGE_U8,  FG_IMAGE_BORDER_SIZE);
	m_pAcqImageRGB				= imaqCreateImage(IMAQ_IMAGE_RGB, FG_IMAGE_BORDER_SIZE);

    m_nCCDGain					= 0;
    m_nCCDExposure_us			= 20000;
}

CFrameGrabber_BS4M::~CFrameGrabber_BS4M()
{
    imaqDispose(m_pAcqImage);
	imaqDispose(m_pRotImage);
	imaqDispose(m_pDstImage_R);
	imaqDispose(m_pDstImage_G);
	imaqDispose(m_pDstImage_B);
	imaqDispose(m_pAcqImageRGB);
}

bool CFrameGrabber_BS4M::InitializeGrab(long nFovSizeX, long nFovSizeY, double dFrameRate)
{
	if(IsOpenGrabber())
        return true;

    // Interface
    if(!InitInterface())
        return false;

	// LYS 2026-09-07 open 이후 단계 실패 시 세션을 닫지 않으면 IMAQdx 세션이 누수돼 이후 모든 open이 실패(카메라 고착)한다 - 실패 시 반드시 close
	// open camera serial
	if(!InitCamSerial())
	{ IMAQdxCloseCamera(m_uiSessionId); m_uiSessionId = 0; return false; }

    // ready to Grabber
    if(!InitGrabSetup())
	{ IMAQdxCloseCamera(m_uiSessionId); m_uiSessionId = 0; return false; }

	// Image buffer
    if(!InitResource(nFovSizeX, nFovSizeY))
	{ IMAQdxCloseCamera(m_uiSessionId); m_uiSessionId = 0; return false; }

    m_bFGOpen = true;

    return true;
}

bool CFrameGrabber_BS4M::InitInterface()
{
    //char errStr[255];

	IMAQdxError imaqErr = IMAQdxOpenCamera((char *)CT2A(LPCTSTR(m_strIntfName)), IMAQdxCameraControlModeController, &m_uiSessionId);
	if (imaqErr != IMAQdxErrorSuccess)
	{
		// LYS 2026-09-07 open 실패 원인 로그 - 객체당 1회(성공하면 다시 1회 허용). 리뷰 카메라(img0) 등 미장착 카메라도 기동 시 1회만 찍힌다
		if (m_bOpenErrLogged == false)
		{
			m_bOpenErrLogged = true;
			char szErr[256] = { 0 };
			IMAQdxGetErrorString(imaqErr, szErr, sizeof(szErr));
			CString strLog;
			strLog.Format(_T("[Camera %s] IMAQdxOpenCamera NG err=0x%08X (%S)"), m_strIntfName, (unsigned)imaqErr, szErr);
			CLogWrapper::GetInstance()->KMLOG(WARN, strLog);
		}
		//IMAQdxGetErrorString(imaqErr, errStr, 255);
		//CString cStr;
		//cStr.Format(_T("Frame grabber session NOT opened.\n%s"), errStr);
		//TRACE("%s(%i) : %s\n", __FILE__, __LINE__, cStr);
		return false;
	}

	m_bOpenErrLogged = false;	// LYS 2026-09-07 open 성공
	int w, h, res;
	res = IMAQdxGetAttribute(m_uiSessionId, IMAQdxAttributeWidth, IMAQdxValueTypeU32, &w);
	if (res == IMAQdxErrorSuccess)
	{
		m_uiAcqWinWidth = w;
	}

	res = IMAQdxGetAttribute(m_uiSessionId, IMAQdxAttributeHeight, IMAQdxValueTypeU32, &h);
	if (res == IMAQdxErrorSuccess)
	{
		m_uiAcqWinHeight = h;
	}

	return true;
}

bool CFrameGrabber_BS4M::InitCamSerial()
{
	//-------------------------------------------------------------------------
	// 그랩 버퍼용 메모리 영역이 lock 걸려 있을때 풀리도록 한번 강제로 그랩해 준다.
	//-------------------------------------------------------------------------
	for(int i = 0; i < 3; ++i)
	{
		IMAQdxSnap(m_uiSessionId, m_pAcqImage);
	}

	return true;
}

bool CFrameGrabber_BS4M::InitGrabSetup()
{
	IMAQdxError imaqErr = IMAQdxConfigureGrab(m_uiSessionId);
	if (imaqErr != IMAQdxErrorSuccess)
	{
		char errStr[255];
		IMAQdxGetErrorString(imaqErr, errStr, 255);
		return false;
	}

	IMAQdxStartAcquisition(m_uiSessionId);

	return true;
}

bool CFrameGrabber_BS4M::InitResource(long nFovSizeX, long nFovSizeY)
{
    // Prepare Image buffer
	if( nFovSizeX > 0 )
		m_uiAcqWinWidth  = nFovSizeX;

	if (nFovSizeY > 0)
		m_uiAcqWinHeight = nFovSizeY;

	// blue 2014.05.28 SSL CS#4에서 Size, PixelFormat 바뀌는 증상 발생하여 추가.
	char* strPixelFormat[256];

	int w = 0, h = 0;
	int res = IMAQdxGetAttribute(m_uiSessionId, IMAQdxAttributeWidth, IMAQdxValueTypeU32, &w);

	if (res == IMAQdxErrorSuccess && w != m_uiAcqWinWidth)
	{
		res = IMAQdxSetAttribute(m_uiSessionId, IMAQdxAttributeWidth, IMAQdxValueTypeU32, INT_MONITOR_WND_WIDTH);
	}

	res = IMAQdxGetAttribute(m_uiSessionId, IMAQdxAttributeHeight, IMAQdxValueTypeU32, &h);

	if (res == IMAQdxErrorSuccess && h != m_uiAcqWinHeight)
	{
		res = IMAQdxSetAttribute(m_uiSessionId, IMAQdxAttributeHeight, IMAQdxValueTypeU32, INT_MONITOR_WND_HEIGHT);
	}

	// LYS 2026-09-14 위 Set 은 호출부 FOV(nFovSize)와 카메라 값이 다를 때 매크로 640x480 을 카메라에 쓰려는 구코드로,
	//   Acquisition 이 이미 시작된 뒤라 실패하고 m_uiAcqWinWidth/Height 만 호출부 값으로 남는다. 보고 크기와 실제 프레임 크기가
	//   어긋나면 Live 표시 배율이 틀어지므로(CCD RESET 후 FOV 좌상단만 보임), 마지막에 카메라의 실제 Width/Height 를 다시 읽어 저장한다.
	w = 0; h = 0;
	res = IMAQdxGetAttribute(m_uiSessionId, IMAQdxAttributeWidth, IMAQdxValueTypeU32, &w);
	if (res == IMAQdxErrorSuccess && w > 0)	m_uiAcqWinWidth = w;
	res = IMAQdxGetAttribute(m_uiSessionId, IMAQdxAttributeHeight, IMAQdxValueTypeU32, &h);
	if (res == IMAQdxErrorSuccess && h > 0)	m_uiAcqWinHeight = h;

	res = IMAQdxGetAttribute(m_uiSessionId, IMAQdxAttributePixelFormat, IMAQdxValueTypeString, &strPixelFormat);
	
	if ((res == IMAQdxErrorSuccess) && (strcmp((char*)strPixelFormat, (char*)"YUV 422 Packed") != 0))
	{
		res = IMAQdxSetAttribute(m_uiSessionId, IMAQdxAttributePixelFormat, IMAQdxValueTypeString, "YUV 422 Packed");
	}
	
	return true;
}

bool CFrameGrabber_BS4M::IsOpenGrabber()
{
    return m_bFGOpen;
}

bool CFrameGrabber_BS4M::ResetGrabber()
{
	IMAQdxStopAcquisition(m_uiSessionId);
	IMAQdxCloseCamera(m_uiSessionId);
	m_bFGOpen = false;	// LYS 2026-09-07 재오픈 전까지 미연결로 표시
	Sleep(1000);

	// Interface
	if (!InitInterface())
		return false;

	// LYS 2026-09-07 open 이후 단계 실패 시 세션 close (누수 방지)
	// open camera serial
	if (!InitCamSerial())
	{ IMAQdxCloseCamera(m_uiSessionId); m_uiSessionId = 0; return false; }

	// ready to Grabber
	if (!InitGrabSetup())
	{ IMAQdxCloseCamera(m_uiSessionId); m_uiSessionId = 0; return false; }

	m_bFGOpen = true;

	return true;

	//bool32 resetAll = true;
	//IMAQdxError imaqErr = IMAQdxResetCamera((char *)CT2A(LPCTSTR(m_strIntfName)), resetAll);
	//if (imaqErr != IMAQdxErrorSuccess)
	//{
	//	//char errStr[256];
	//	//IMAQdxGetErrorString(imaqErr, errStr, 255);
	//	//CString cStr;
	//	//cStr.Format(_T("Frame grabber reset failed.\n%s"), errStr);
	//	//TRACE("%s(%i) : %s\n", __FILE__, __LINE__, errStr);
	//	return false;
	//}

	// 카메라 내부적으로 저정되어 있는 USER 1 번에 대한 통신 Command 를 보내야 한다.
	//IMAQdxError imaqErr = IMAQdxSetAttribute(m_uiSessionId, "CameraAttributes::Reset", IMAQdxValueTypeU32, nGain);
	//
	//if (imaqErr != IMAQdxErrorSuccess)
	//{
	//	char errStr[256];
	//	IMAQdxGetErrorString(imaqErr, errStr, 255);
	//	CString cStr;
	//	cStr.Format(_T("Frame grabber SetCCDGain failed.\n%s"), errStr);
	//	TRACE("%s(%i) : %s\n", __FILE__, __LINE__, errStr);
	//	return false;
	//}

//	return InitGrabSetup();
}

bool CFrameGrabber_BS4M::RefreshGrabber()
{
	IMAQdxStopAcquisition(m_uiSessionId);

	return InitGrabSetup();
}

bool CFrameGrabber_BS4M::FinalizeGrab()
{
	IMAQdxStopAcquisition(m_uiSessionId);
	IMAQdxCloseCamera(m_uiSessionId);
	Sleep(200);

    // Close session
    if(m_uiSessionId != 0)
    {
        imgClose (m_uiSessionId, TRUE);
        m_uiSessionId = 0;
    }

    // Close interface
    if(m_uiIntfaceId != 0)
    {
        imgClose (m_uiIntfaceId, TRUE);
        m_uiIntfaceId = 0;
    }

    m_bFGOpen = false;

    return true;
}




//---------------------------------------------------------------------------
// 이미지 획득을 위해 공유되는 함수
//---------------------------------------------------------------------------
bool CFrameGrabber_BS4M::GetLiveImage(bool bFlag)
{
	return false;
}

bool CFrameGrabber_BS4M::SnapImage(short* pbyImage)
{
	if (IsOpenGrabber() && m_pAcqImageRGB)
	{
		uInt32 uiBufferNumber;
		IMAQdxError imaqErr = IMAQdxGrab(m_uiSessionId, m_pAcqImageRGB, TRUE, &uiBufferNumber);
		if (imaqErr == IMAQdxErrorSuccess)
		{
			imaqDuplicate((Image*)pbyImage, m_pAcqImageRGB);

			return true;
		}
	}

	return false;
}

bool CFrameGrabber_BS4M::GetSnapImage(long nPulseWidth_ms, BYTE *pbyImage)
{
	if(IsOpenGrabber() && m_pAcqImage)
	{
		//uInt32	uiBufferNumber;
		//IMAQdxError imaqErr = IMAQdxGrab(m_uiSessionId, m_pAcqImage, TRUE, &uiBufferNumber);

	//	if (IMAQdxSnap(m_uiSessionId, m_pAcqImage) == IMAQdxErrorSuccess)
		uInt32 uiBufferNumber;

	//	if (IMAQdxGrab(m_uiSessionId, m_pAcqImage, TRUE, &uiBufferNumber) == IMAQdxErrorSuccess)
		IMAQdxError imaqErr = IMAQdxGrab(m_uiSessionId, m_pAcqImage, TRUE, &uiBufferNumber);
		if (imaqErr == IMAQdxErrorSuccess)
		{
			imaqCast(m_pAcqImageRGB, m_pAcqImage, IMAQ_IMAGE_RGB, NULL, 0);
			if(m_nRotateAngle == 0)
			{
				CopyImageData(m_pAcqImageRGB, pbyImage);
			}
			else
			{
				PixelValue value;
				value.grayscale = 0;
				imaqRotate2(m_pRotImage, m_pAcqImage, (float)(m_nRotateAngle), value, IMAQ_BILINEAR, FALSE);

				CopyImageData(m_pRotImage, pbyImage);
			}

			return true;
		}
	}

    return false;
}

void CFrameGrabber_BS4M::CopyImageData(Image *pSrcImage, BYTE *pDstPtr)
{
//	imaqWritePNGFile(pSrcImage, "D:\\Temp\\pSnapImageB.png", 750, NULL);

	//---------------------------------------------------------------------------------------
	// open cv library 와 연동시켜서 사용하려고 할때는 3color 로 copy 하도록 함.
	//---------------------------------------------------------------------------------------
	imaqExtractColorPlanes(pSrcImage, IMAQ_RGB, m_pDstImage_R, m_pDstImage_G, m_pDstImage_B);
	
	ImageInfo infoR, infoG, infoB;
	imaqGetImageInfo(m_pDstImage_R, &infoR);
	imaqGetImageInfo(m_pDstImage_G, &infoG);
	imaqGetImageInfo(m_pDstImage_B, &infoB);
	
	int w = 0, h = 0;
	imaqGetImageSize(m_pDstImage_R, &w, &h);

	const long nSrcWidthBytes = infoR.pixelsPerLine;
	const long nDstWidthBytes = (w + 3) / 4 * 4 * 4;

	LPBYTE pRSrcPtr = (LPBYTE)infoR.imageStart;
	LPBYTE pGSrcPtr = (LPBYTE)infoG.imageStart;
	LPBYTE pBSrcPtr = (LPBYTE)infoB.imageStart;

	for (long nY = 0; nY < h; nY++)
	{
		for (long nX = 0, nX2 = 0; nX < w; nX++, nX2 += 4)
		{
			pDstPtr[nY * nDstWidthBytes + nX2 + 0] = pBSrcPtr[nY * nSrcWidthBytes + nX];
			pDstPtr[nY * nDstWidthBytes + nX2 + 1] = pGSrcPtr[nY * nSrcWidthBytes + nX];
			pDstPtr[nY * nDstWidthBytes + nX2 + 2] = pRSrcPtr[nY * nSrcWidthBytes + nX];
			pDstPtr[nY * nDstWidthBytes + nX2 + 3] = 0x00;
		}
	}
	
	//const long nSrcWidthBytes = infoR.pixelsPerLine;
	//const long nDstWidthBytes = (w + 3) / 4 * 4 * 3;
	//
	//LPBYTE pRSrcPtr = (LPBYTE)infoR.imageStart;
	//LPBYTE pGSrcPtr = (LPBYTE)infoG.imageStart;
	//LPBYTE pBSrcPtr = (LPBYTE)infoB.imageStart;
	//
	//for (long nY = 0; nY < h; nY++)
	//{
	//	for (long nX = 0, nX2 = 0; nX < w; nX++, nX2 += 3)
	//	{
	//		pDstPtr[nY * nDstWidthBytes + nX2 + 0] = pBSrcPtr[nY * nSrcWidthBytes + nX];
	//		pDstPtr[nY * nDstWidthBytes + nX2 + 1] = pGSrcPtr[nY * nSrcWidthBytes + nX];
	//		pDstPtr[nY * nDstWidthBytes + nX2 + 2] = pRSrcPtr[nY * nSrcWidthBytes + nX];
	//	}
	//}
}

// 카메라와 파라미터 연동하는 함수
bool CFrameGrabber_BS4M::SetParam_CameraGain(long nCCDGain)
{
    if(nCCDGain < 0)
        return false;
    if(m_nCCDGain == nCCDGain)
        return true;

	IMAQdxStopAcquisition(m_uiSessionId);

	//if(nCCDGain < 36)	nCCDGain = 36;
 //   if(nCCDGain > 512) nCCDGain = 512;

	//IMAQdxError imaqErr = IMAQdxSetAttribute(m_uiSessionId, "CameraAttributes::AnalogControls::GainRaw", IMAQdxValueTypeU32, nCCDGain);

	IMAQdxError imaqErr = IMAQdxErrorSuccess;
//	if (m_nTypeCCD == 0)
	if( 1 )
	{
		if (nCCDGain < 300)
			nCCDGain = 300;
		else if (nCCDGain > 850)
			nCCDGain = 850;

		imaqErr = IMAQdxSetAttribute(m_uiSessionId
			, "CameraAttributes::AnalogControls::GainRaw"
			, IMAQdxValueTypeU32, nCCDGain);
	}
	else
	{
		if (nCCDGain < 1)
			nCCDGain = 1;
		else if (nCCDGain > 31)
			nCCDGain = 31;

		imaqErr = IMAQdxSetAttribute(m_uiSessionId
			, "CameraAttributes::Controls::GainControl::Gain"
			, IMAQdxValueTypeU32, nCCDGain);
	}

	if (imaqErr != IMAQdxErrorSuccess)
	{
		//char errStr[256];
		//IMAQdxGetErrorString(imaqErr, errStr, 255);
		//CString cStr;
		//cStr.Format(_T("Frame grabber SetCCDGain failed.\n%s"), errStr);
		//TRACE("%s(%i) : %s\n", __FILE__, __LINE__, errStr);
		//return false;
	}
	else
	{
		m_nCCDGain = nCCDGain;
	}

	return InitGrabSetup();
}

long CFrameGrabber_BS4M::GetParam_CameraGain()
{
    return m_nCCDGain;
}

bool CFrameGrabber_BS4M::SetParam_CameraExposureTime_ms(long nCCDExposure)
{
//	long nCCDExposureTime_us = nCCDExposure * 1000;
	long nCCDExposureTime_us = nCCDExposure;

	if(nCCDExposureTime_us < 0)
        return false;
	if (m_nCCDExposure_us == nCCDExposureTime_us)
		return true;

	IMAQdxStopAcquisition(m_uiSessionId);

	//if (nCCDExposureTime_us < 100)		nCCDExposureTime_us = 100;
	//if (nCCDExposureTime_us > 5000000)	nCCDExposureTime_us = 5000000;

	//IMAQdxError imaqErr = IMAQdxSetAttribute(m_uiSessionId, "CameraAttributes::AcquisitionTrigger::ExposureTimeRaw", IMAQdxValueTypeU32, nCCDExposureTime_us);

	IMAQdxError imaqErr = IMAQdxErrorSuccess;
//	if (m_nTypeCCD == 0)

	if (nCCDExposureTime_us < 16)			nCCDExposureTime_us = 16;
	if (nCCDExposureTime_us > 90000)		nCCDExposureTime_us = 90000;

	if( 1 )
	{
		imaqErr = IMAQdxSetAttribute(m_uiSessionId
			, "CameraAttributes::AcquisitionTrigger::ExposureTimeRaw"
			, IMAQdxValueTypeU32, nCCDExposureTime_us);
	}
	else
	{
		imaqErr = IMAQdxSetAttribute(m_uiSessionId
			, "CameraAttributes::Controls::Exposure::ExposureTimeAbs"
			, IMAQdxValueTypeU32, nCCDExposureTime_us);
	}

	if (imaqErr != IMAQdxErrorSuccess)
	{
		//char errStr[256];
		//IMAQdxGetErrorString(imaqErr, errStr, 255);
		//CString cStr;
		//cStr.Format(_T("Frame grabber SetCCDExposure failed.\n%s"), errStr);
		//TRACE("%s(%i) : %s\n", __FILE__, __LINE__, errStr);
		//return false;
	}
	else
	{
		m_nCCDExposure_us = nCCDExposureTime_us;
	}

	return InitGrabSetup();
}

long CFrameGrabber_BS4M::GetParam_CameraExposureTime_ms()
{
	// 내부적으로는 us 이지만, 외부로 노출할때는 ms 로 한다.
    return m_nCCDExposure_us;
}

void CFrameGrabber_BS4M::SetParam_HBayerPattern(long nIndex)
{
    m_nHBayerPattern = nIndex;
}

void CFrameGrabber_BS4M::SetParam_RotateAngle(long nValue)
{
	m_nRotateAngle = nValue;
}

long CFrameGrabber_BS4M::GetParam_RotateAngle()
{
	return m_nRotateAngle;
}

