#pragma once

#include "..\AuroraSubMgr.h"
#include "FrameGrabberFunction\FrameGrabberFunction.h"
#include <afxmt.h>
#include "ShareMemFunction\ShareMem.h"
#include "common/IMAQImageCtrl/IMAQImageCtrl.h"

class CIniHelper;
class CFrameGrabberFunction;
class CAlignCameraMgr : public CAuroraSubMgr
{
    CIniHelper				*m_pIniHelper;
	bool					m_bUse;				// 장치 사용여부, UI상에서도 보이지 않게 처리함. OutOfService와는 다름
	bool					m_bOutOfService;	// 모듈 사용 여부 0: USE
	CString					m_strControlType;
	CFrameGrabberFunction	*m_pFunction;
	CCriticalSection		m_cs_get;
	CCriticalSection		m_cs_res;
	CIMAQImageCtrl			* m_pimaqctrlROI;
	Image*					m_pSnapImage;
	Image*					m_pOverlayImage;
	int						m_iCCDNum;
	Rect					m_rectROI;
	bool					m_bShowROI;
	bool					m_bSimImageMissingLogged;	// LYS 2026-09-05 미연결 시 시뮬 이미지(Pattern/AlignN.png) 부재 WARN 1회 로그 플래그

public:
	bool	m_bClose;
    bool	InitDevice();
	void	CloseDevice();
    CString	GetDeviceName();
    bool	IsConnected();
	bool	IsOutOfService();

    // 전용 함수 - 시퀀스 연동
	bool	SnapImage(short* pbyImage);
	bool	GetSnapImage(long nExpTime_ms, BYTE *pbyImage);
	bool    ResetGrabber();

	int		GetImageGrabSizeX();
	int		GetImageGrabSizeY();
	int		GetParam_CameraExposureTime_ms();
	int		GetParam_CameraGain();
	int		SetParam_CameraExposureTime_ms(long nExpTime_ms);
	int		SetParam_CameraGain(long nGain);
	int		PtrnMatch(CPtrnInfo* pPtrnInfo, float* pfScore, int nMeasLens = LENS_USE_MEAS, bool bGlass = true);
	int		PtrnMatchBest(CPtrnInfo* pPtrnInfo, int nCount, float* pfBestScore, int* pnBestIdx, int nMeasLens = LENS_USE_ALIGN, bool bGlass = true);	// LYS 2026-09-25 등록 패턴 전부 매칭 후 최고 점수 패턴 채택 (스냅 1회)
	int		SnapImage(Image* pImage, bool bDisplay = false);
	int		SaveImage(Image* pImage, int iType);
	int		SaveCurImage(int iType);
	int		SetShowROI(bool bShow, Rect rectROI);
	void	GetShowROI(bool* pbShow, Rect* prectROI);
	float	CalcAngle(COORDINATE stBase1, COORDINATE stBase2, COORDINATE stDetect, COORDINATE stDetect2);

//------------------------------------------------------------------
// 공통 인터페이스 함수
//------------------------------------------------------------------
public:
	virtual bool LinkIniFile(bool bSave) override;
private:
	virtual void Finalize() override;
public:
	CAlignCameraMgr(CString strConfigPath, CString strParamPath, CString strResultPath, int iIndex = 0);
	virtual ~CAlignCameraMgr();
	CString m_strRecipePath;
	CString m_strResultPath;
};

