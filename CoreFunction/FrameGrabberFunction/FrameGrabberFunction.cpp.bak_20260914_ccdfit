#include "pch.h"
#include "FrameGrabberFunction.h"

//#include "CLCamera_Vieworks_VA8C\FrameGrabber_CL_VA8C.h"
//#include "CLCamera_Vieworks_VN29MC_MIL10\FrameGrabber_CL_VN29Mono.h"
//#include "CLCamera_JAI_SP5000C_PMCL\FrameGrabber_CL_SP5000Cpmcl.h"
#include "GigECamera_Basler\FrameGrabber_GigE_BS4M.h"
//#include "GigECamera_Vieworks\FrameGrabber_GigE_VW4M.h"
#include "common\IniHelper.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CFrameGrabberFunction::CFrameGrabberFunction(CString strConfigFolderPath, CString strResultFolderPath, CString strDeviceModel, CString strFilePath, CString strCategory)
	: m_pIniHelper(new CIniHelper)
	, m_strResultFolderPath(strResultFolderPath)
	, m_strConfigFolderPath(strConfigFolderPath)
	, m_strCategory(strCategory)
	, m_pCtrlImpl(nullptr)
{
	m_pIniHelper->SetFilePath(strFilePath);

	LinkIniFile(false);



	//-----------------------------------------------------------------------
	// 카메라를 생성한다.
	//-----------------------------------------------------------------------
	if (strDeviceModel == _T("GIGE_CAMERA_BASLER"))
	{
		//LinkIcdFile(m_strConfigFolderPath + _T("\\Camera\\Basler_acA2040_25gc.icd"), m_nImageGrabSizeX, m_nImageGrabSizeY);

		//m_nImageGrabSizeX = 2048;
		//m_nImageGrabSizeY = 2048;

		m_pCtrlImpl = dynamic_cast<CFrameGrabberImpl *>(new CFrameGrabber_GigE_BS4M);

		m_nImageGrabSizeX = 0;
		m_nImageGrabSizeY = 0;
	}
//	if(strDeviceModel == _T("CL_CAMERA_8M_NI_BOARD"))
//	{
//		LinkIcdFile(m_strConfigFolderPath + _T("\\Camera\\Vieworks_VA-8MC-C16_2Tap_8Bit.icd"), m_nImageGrabSizeX, m_nImageGrabSizeY);
//
//		m_pCtrlImpl = dynamic_cast<CFrameGrabberImpl *>(new CFrameGrabber_CL_VA8C);
//	}
//	else if(strDeviceModel == _T("CL_CAMERA_29M_MATROX"))
//	{
//		CString strInterfaceType = _T("M_SYSTEM_SOLIOS");
//		CString strDCFfile = m_strConfigFolderPath + _T("\\Camera\\Continuous_VN-29M5(80MHz)_8Bit_4tap_Top_Bottom_CC1TriggerEvent.dcf");
//
//#ifdef WIN64
//		m_pCtrlImpl = dynamic_cast<CFrameGrabberImpl *>(new CFrameGrabber_CL_VN29Mono(m_strConfigFolderPath, strInterfaceType, strDCFfile));
//#else
//#endif
//	}
//	else if(strDeviceModel == _T("CL_CAMERA_SP5000C_NI_BOARD"))
//	{
//		LinkIcdFile(m_strConfigFolderPath + _T("\\Camera\\JAI SP5000_1920(1427).icd"), m_nImageGrabSizeX, m_nImageGrabSizeY);
//
//		m_pCtrlImpl = dynamic_cast<CFrameGrabberImpl *>(new CFrameGrabber_CL_SP5000Cpmcl);
//	}
//	else if(strDeviceModel == _T("GIGE_CAMERA_BASLER"))
//	{
//		LinkIcdFile(m_strConfigFolderPath + _T("\\Camera\\Basler_acA2040_25gc.icd"), m_nImageGrabSizeX, m_nImageGrabSizeY);
//
//		m_nImageGrabSizeX = 2048;
//		m_nImageGrabSizeY = 2048;
//
//		m_pCtrlImpl = dynamic_cast<CFrameGrabberImpl *>(new CFrameGrabber_GigE_BS4M);
//	}
//	else if(strDeviceModel == _T("GIGE_CAMERA_VIEWORKS"))
//	{
//		LinkIcdFile(m_strConfigFolderPath + _T("\\Camera\\VIEWORKS_VH_4MG2_C20.icd"), m_nImageGrabSizeX, m_nImageGrabSizeY);
//
//		m_nImageGrabSizeX = 2048;
//		m_nImageGrabSizeY = 2048;
//
//		m_pCtrlImpl = dynamic_cast<CFrameGrabberImpl *>(new CFrameGrabber_GigE_VW4M);
//	}
//	else
//	{
//		LinkIcdFile(m_strConfigFolderPath + _T("\\Camera\\Vieworks_VA-8MC-C16_2Tap_8Bit.icd"), m_nImageGrabSizeX, m_nImageGrabSizeY);
//
//		m_pCtrlImpl = dynamic_cast<CFrameGrabberImpl *>(new CFrameGrabber_CL_VA8C);
//	}

	m_nImageFOVSizeX = m_nImageGrabSizeX * 1;
	m_nImageFOVSizeY = m_nImageGrabSizeY * 1;
}

CFrameGrabberFunction::~CFrameGrabberFunction()
{
	if(m_pIniHelper)
	{
		delete m_pIniHelper;
		m_pIniHelper = nullptr;
	}

	DeleteGrabber();
}

bool CFrameGrabberFunction::LinkIniFile(bool bSave)
{
	//-----------------------------------------------------------------------
	// 파라미터 연동
	//-----------------------------------------------------------------------
	m_pIniHelper->Link(bSave, m_strCategory, _T("FILE_SAVE_PATH"),			&m_strFileSavePath,				(CString)_T("IMAGE"));
	m_pIniHelper->Link(bSave, m_strCategory, _T("GRAB_WAIT_COUNT"),			&m_nGrabWaitCount,				(long)2);
	m_pIniHelper->Link(bSave, m_strCategory, _T("FRAME_RATE_COUNT(fps)"),	&m_dImageFrameRate,				1.0);
	m_pIniHelper->Link(bSave, m_strCategory, _T("INTERFACE_NAME"),			&m_strInterfaceName,			(CString)_T("img0"));
	m_pIniHelper->Link(bSave, m_strCategory, _T("GRAB_MODE"),				&m_strGrabMode,					(CString)_T("1SHOT"));
	m_pIniHelper->Link(bSave, m_strCategory, _T("CCD_BAYER_PATTERN"),		&m_nCameraCCD_HBayerPattern,	(long)1);
	m_pIniHelper->Link(bSave, m_strCategory, _T("CCD_ROTATE_ANGLE"),		&m_nCameraCCD_RotateAngle,		(long)0);
	m_pIniHelper->Link(bSave, m_strCategory, _T("CCD_WHITE_BALANCE_R"),		&m_nCameraCCD_WhiteBalanceR,	(double)1.0);
	m_pIniHelper->Link(bSave, m_strCategory, _T("CCD_WHITE_BALANCE_G"),		&m_nCameraCCD_WhiteBalanceG,	(double)1.0);
	m_pIniHelper->Link(bSave, m_strCategory, _T("CCD_WHITE_BALANCE_B"),		&m_nCameraCCD_WhiteBalanceB,	(double)1.0);

	//메뉴얼로 생성한 이미지를 저장하는 폴더 경로
	m_strImageFileSavePath = m_strResultFolderPath + _T("\\") + m_strFileSavePath;
//	MakeDir(m_strImageFileSavePath);
	
	return m_pIniHelper->IsReadEnable();
}





//bool CFrameGrabberFunction::LinkIniFile(CIniHelper *pIniHelper, bool bSave)
//{
//	// recipe 연동 변수
//	// 여기 있으면 안되고 recipe 쪽으로 이동해야 한다.
//	pIniHelper->Link(bSave, _T("CAMERA_PARA"), _T("CCD_GAIN"),			&m_nCameraCCD_Gain,				(long)30);
//	pIniHelper->Link(bSave, _T("CAMERA_PARA"), _T("CCD_EXPOSURE_TIME"),	&m_nCameraCCD_Exposure_ms,		(long)18);
//
//	return true;
//}

bool CFrameGrabberFunction::LinkIcdFile(CString strFilePath, long &nImageSizeX, long &nImageSizeY)
{
	//----------------------------------------------------------------
	// Search Icd File Full Path
	//----------------------------------------------------------------
	CFileFind finder;

	BOOL bWorking = finder.FindFile(strFilePath);

	CString strFileFullPath;

	while (bWorking)
	{
		bWorking = finder.FindNextFile();

		if (finder.IsArchived())
		{
			const CString strfileName = finder.GetFileName();

			// 현재폴더 상위폴더 썸네일파일은 제외
			if (strfileName == _T(".") || strfileName == _T("..") || strfileName == _T("Thumbs.db"))
			{
				continue;
			}

			strFileFullPath = finder.GetFilePath();
			break;
		}

		Sleep(1);
	}

	//----------------------------------------------------------------
	// 파라미터 연동
	//----------------------------------------------------------------
	if (PathFileExists(strFileFullPath))
	{
		CStdioFile file(strFileFullPath, CFile::modeRead);

		CString strLine;
		int n = 0;

		while (1)
		{
			if (file.ReadString(strLine))
			{
				if (0 <= strLine.Find(_T("MaxImageSize")))
				{
					CString strValue;
					AfxExtractSubString(strValue, strLine, 1, _T('('));
					strValue.Replace(_T(")"), _T(""));

					CString strFovSizeX, strFovSizeY;
					AfxExtractSubString(strFovSizeX, strValue, 0, _T(','));
					AfxExtractSubString(strFovSizeY, strValue, 1, _T(','));

					nImageSizeX = _ttoi(strFovSizeX);
					nImageSizeY = _ttoi(strFovSizeY);
					return true;
				}
			}
			else
				break;

			Sleep(1);
		}
	}

	return false;
}



CString CFrameGrabberFunction::GetImageFileSavePath() const 
{ 
	return m_strImageFileSavePath; 
}
	
CString CFrameGrabberFunction::GetImageGrabMode() const
{ 
	return m_strGrabMode; 
}

long CFrameGrabberFunction::GetImageGrabSizeX() const	
{
	return m_nImageGrabSizeX; 
}

long CFrameGrabberFunction::GetImageGrabSizeY() const	
{
	return m_nImageGrabSizeY; 
}





bool CFrameGrabberFunction::CreateGrabber()
{
	if (m_pCtrlImpl)
	{
		bool bRet = false;
		m_pCtrlImpl->SetParam_InterfaceName(m_strInterfaceName);
		m_pCtrlImpl->SetParam_GrabMode(GetImageGrabMode());
		bRet = m_pCtrlImpl->InitializeGrabber(m_nImageGrabSizeX, m_nImageGrabSizeY, m_dImageFrameRate);
		m_nImageGrabSizeX = m_pCtrlImpl->GetParam_CameraAcqWidth();
		m_nImageGrabSizeY = m_pCtrlImpl->GetParam_CameraAcqHeighth();

		m_nImageFOVSizeX = m_nImageGrabSizeX * 1;
		m_nImageFOVSizeY = m_nImageGrabSizeY * 1;

		SetParam_CameraExposureTime_ms(9000);
		SetParam_CameraGain(10);

		return bRet;
	}
	else
	{
		return false;
	}
	
}

void CFrameGrabberFunction::DeleteGrabber()
{
	if (m_pCtrlImpl)
	{
		m_pCtrlImpl->FinalizeGrabber();
		delete m_pCtrlImpl;
		m_pCtrlImpl = nullptr;
	}
}

bool CFrameGrabberFunction::IsOpenGrabber()
{
	if (m_pCtrlImpl != nullptr)
		return m_pCtrlImpl->IsOpenGrabber();
	else
		return false;
}

bool CFrameGrabberFunction::SnapImage(short* pbyImage)
{
	if (m_pCtrlImpl != nullptr)
		return m_pCtrlImpl->SnapImage(pbyImage);
	else
		return false;
}

bool CFrameGrabberFunction::GetSnapImage(long nExpTime_ms, BYTE *pbyImage)
{
	if (m_pCtrlImpl != nullptr)
		return m_pCtrlImpl->GetSnapImage(nExpTime_ms, pbyImage);
	else
		return false;
}

bool CFrameGrabberFunction::ResetGrabber()
{
	if (m_pCtrlImpl != nullptr)
		return m_pCtrlImpl->ResetGrabber();
	else
		return false;
}

//------------------------------------------------------------------
// 카메라와 파라미터 연동하는 함수
//------------------------------------------------------------------
bool CFrameGrabberFunction::SetParam_CameraReset()
{
	return m_pCtrlImpl->SetParam_CameraReset();
}

bool CFrameGrabberFunction::SetParam_CameraGain(int value)
{
	m_nCameraCCD_Gain = value;

	return m_pCtrlImpl->SetParam_CameraGain(value);
}

long CFrameGrabberFunction::GetParam_CameraGain()
{
	return m_nCameraCCD_Gain = m_pCtrlImpl->GetParam_CameraGain();
}

bool CFrameGrabberFunction::SetParam_CameraExposureTime_ms(int value)
{
	m_nCameraCCD_Exposure_ms = value;

	return m_pCtrlImpl->SetParam_CameraExposureTime_ms(value);
}

long CFrameGrabberFunction::GetParam_CameraExposureTime_ms()
{
	return m_nCameraCCD_Exposure_ms;
}

void CFrameGrabberFunction::SetParam_HBayerPattern(long nIndex)
{
	m_nCameraCCD_HBayerPattern = nIndex;

	m_pCtrlImpl->SetParam_HBayerPattern(nIndex);
}

long CFrameGrabberFunction::GetParam_HBayerPatternIndex() const
{
	return 	m_nCameraCCD_HBayerPattern;
}

long CFrameGrabberFunction::GetParam_HBayerPatternIndex(CString strValue)
{
	if (_T("PATTERN_GBGB_RGRG") == strValue)	return 0;
	if (_T("PATTERN_GRGR_BGBG") == strValue)	return 1;
	if (_T("PATTERN_BGBG_GRGR") == strValue)	return 2;
	if (_T("PATTERN_RGRG_GBGB") == strValue)	return 3;

	return 0;
}

CString CFrameGrabberFunction::GetParam_HBayerPatternName(long nIndex)
{
	switch (nIndex)
	{
		case 0: return _T("PATTERN_GBGB_RGRG");
		case 1: return _T("PATTERN_GRGR_BGBG");
		case 2: return _T("PATTERN_BGBG_GRGR");
		case 3: return _T("PATTERN_RGRG_GBGB");
	}

	return _T("IMG_BAYER_PATTERN_GBGB_RGRG");
}

void CFrameGrabberFunction::SetParam_RotateAngle(long nValue)
{
	m_nCameraCCD_RotateAngle = nValue;

	m_pCtrlImpl->SetParam_RotateAngle(nValue);
}

long CFrameGrabberFunction::GetParam_RotateAngle() const
{
	return m_nCameraCCD_RotateAngle;
}

void CFrameGrabberFunction::SetParam_WhiteBalanceRGB(double dR, double dG, double dB)
{
	m_pCtrlImpl->SetParam_WhiteBalanceRGB(dR, dG, dB);
}

void CFrameGrabberFunction::SetParam_WhiteBalanceR(double dValue)
{
	m_nCameraCCD_WhiteBalanceR = dValue;

	SetParam_WhiteBalanceRGB(m_nCameraCCD_WhiteBalanceR, m_nCameraCCD_WhiteBalanceG, m_nCameraCCD_WhiteBalanceB);
}

void CFrameGrabberFunction::SetParam_WhiteBalanceG(double dValue)
{
	m_nCameraCCD_WhiteBalanceG = dValue;

	SetParam_WhiteBalanceRGB(m_nCameraCCD_WhiteBalanceR, m_nCameraCCD_WhiteBalanceG, m_nCameraCCD_WhiteBalanceB);
}

void CFrameGrabberFunction::SetParam_WhiteBalanceB(double dValue)
{
	m_nCameraCCD_WhiteBalanceB = dValue;

	SetParam_WhiteBalanceRGB(m_nCameraCCD_WhiteBalanceR, m_nCameraCCD_WhiteBalanceG, m_nCameraCCD_WhiteBalanceB);
}

double CFrameGrabberFunction::GetParam_WhiteBalanceRGB_R() const
{
	return m_nCameraCCD_WhiteBalanceR;
}

double CFrameGrabberFunction::GetParam_WhiteBalanceRGB_G() const
{
	return m_nCameraCCD_WhiteBalanceG;
}

double CFrameGrabberFunction::GetParam_WhiteBalanceRGB_B() const
{
	return m_nCameraCCD_WhiteBalanceB;
}
