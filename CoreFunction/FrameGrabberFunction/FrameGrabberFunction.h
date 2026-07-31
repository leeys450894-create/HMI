#pragma once

class CIniHelper;
class CFrameGrabberImpl;
class CFrameGrabberFunction
{
	CIniHelper			*m_pIniHelper;
	CString				m_strConfigFolderPath;			// 환경 설정 저장된 위치
	CString				m_strResultFolderPath;			// 영상 획득한 정보를 저장할 위치
	CString				m_strCategory;
	CFrameGrabberImpl	*m_pCtrlImpl;					// 영상 획득을 위한 카메라 모듈
	CString				m_strImageFileSavePath;			// 영상 획득한 정보를 저장할 위치
	CString				m_strFileSavePath;				// 영상 획득한 이미지 경로
	CString				m_strInterfaceName;				// 영상 획득을 위한 연동 이름
	CString				m_strGrabMode;					// 영상 획득을 위한 방식 - 1SHOT, 4SHOT
	long				m_nGrabWaitCount;				// 영상 획득을 완료하는 대기 횟수
	double				m_dImageFrameRate;				// 영상 획득을 수행하는 속도 - per 1s
	long				m_nImageGrabSizeX;				// 영상 사이즈 X
	long				m_nImageGrabSizeY;				// 영상 사이즈 Y
	long				m_nImageFOVSizeX;				// 이미지 FOV 사이즈 X
	long				m_nImageFOVSizeY;				// 이미지 FOV 사이즈 Y
	long				m_nCameraCCD_HBayerPattern;		// 영상 획득한 이미지 패턴 순서
	long				m_nCameraCCD_RotateAngle;		// 영상 획득한 이미지 회전
	double				m_nCameraCCD_WhiteBalanceR;
	double				m_nCameraCCD_WhiteBalanceG;
	double				m_nCameraCCD_WhiteBalanceB;
	long				m_nCameraCCD_Gain;				// 영상 획득을 위한 게인 - 레시피와 연동 되어 있음
	long				m_nCameraCCD_Exposure_ms;			// 영상 획득을 위한 노출 - 레시피와 연동 되어 있음

private:
	bool	LinkIcdFile(CString strFilePath, long &nImageSizeX, long &nImageSizeY);
public:
	// INI-File Load & Save
	bool	LinkIniFile(bool bSave);
	//bool	LinkIniFile(CIniHelper *pIniHelper, bool bSave);	// recipe 연동

	CString GetImageFileSavePath() const;
	CString GetImageGrabMode() const;
	long	GetImageGrabSizeX() const;
	long	GetImageGrabSizeY() const;
	long	GetImageFOVSizeX() const { return m_nImageFOVSizeX; }
	long	GetImageFOVSizeY() const { return m_nImageFOVSizeY; }

public:
	bool	CreateGrabber();
	void	DeleteGrabber();
	bool	IsOpenGrabber();
	bool	SnapImage(short* pbyImage);
	bool	GetSnapImage(long nExpTime_ms, BYTE *pbyImage);
	bool	ResetGrabber();

	//------------------------------------------------------------------
	// 카메라와 파라미터 연동하는 함수
	//------------------------------------------------------------------
	bool	SetParam_CameraReset();
	bool	SetParam_CameraGain(int value);
	long	GetParam_CameraGain();
	bool	SetParam_CameraExposureTime_ms(int value);
	long	GetParam_CameraExposureTime_ms();
	void	SetParam_HBayerPattern(long nIndex);
	long	GetParam_HBayerPatternIndex() const;
	long	GetParam_HBayerPatternIndex(CString strValue);
	CString GetParam_HBayerPatternName(long nIndex);
	void	SetParam_RotateAngle(long nValue);
	long	GetParam_RotateAngle() const;
	void	SetParam_WhiteBalanceRGB(double dR, double dG, double dB);
	void	SetParam_WhiteBalanceR(double dValue);
	void	SetParam_WhiteBalanceG(double dValue);
	void	SetParam_WhiteBalanceB(double dValue);
	double	GetParam_WhiteBalanceRGB_R() const;
	double	GetParam_WhiteBalanceRGB_G() const;
	double	GetParam_WhiteBalanceRGB_B() const;
		
public:
	CFrameGrabberFunction(CString strConfigFolderPath, CString strResultFolderPath, CString strDeviceModel, CString strFilePath, CString strCategory);
	~CFrameGrabberFunction();
};