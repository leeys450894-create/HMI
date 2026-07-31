//#include "stdafx.h"
#include "pch.h"
#include "AlignCameraMgr.h"

#include "common\IniHelper.h"
#include "common/CommonFunc.h"
#include "SystemParamShare.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CAlignCameraMgr::CAlignCameraMgr(CString strConfigPath, CString strParamPath, CString strResultPath, int iIndex)
    : m_pIniHelper(new CIniHelper)
	, m_pimaqctrlROI(nullptr)
	, m_bShowROI(false)
	, m_strRecipePath(strParamPath)
	, m_strResultPath(strResultPath)
{
	if (iIndex == 0)
		SetName(_T("ALIGN_CAMERA1_MGR"));
	else if(iIndex == 1)
		SetName(_T("ALIGN_CAMERA2_MGR"));
	else if (iIndex == 2)
		SetName(_T("ALIGN_CAMERA3_MGR"));
	else if (iIndex == 3)
		SetName(_T("ALIGN_CAMERA4_MGR"));
	else
		SetName(_T("ALIGN_CAMERA_MGR"));
 //   SetName(_T("ALIGN_CAMERA_MGR"));

	m_iCCDNum = iIndex;

	const CString strFilePath = strConfigPath + _T("\\CoreFunction\\CoreFunction.ini");
    m_pIniHelper->SetFilePath(strFilePath);

    LinkIniFile(false);

    //----------------------------------------------------------------------
    //----------------------------------------------------------------------
	m_pFunction = new CFrameGrabberFunction(strConfigPath, strConfigPath, m_strControlType, strFilePath, GetName());

	if (m_pimaqctrlROI == nullptr)
	{
		m_pimaqctrlROI = new CIMAQImageCtrl();
	}

//	m_pSnapImage = imaqCreateImage(IMAQ_IMAGE_U8, INT_IMAGE_BORDER_SIZE);
	m_pSnapImage = imaqCreateImage(IMAQ_IMAGE_RGB, INT_IMAGE_BORDER_SIZE);
	m_pOverlayImage = imaqCreateImage(IMAQ_IMAGE_RGB, INT_IMAGE_BORDER_SIZE);

	m_bClose = false;
}

CAlignCameraMgr::~CAlignCameraMgr()
{
    Finalize();
}

bool CAlignCameraMgr::LinkIniFile(bool bSave)
{
    m_pIniHelper->Link(bSave, GetName(), _T("USE"),				&m_bUse,				true);
    m_pIniHelper->Link(bSave, GetName(), _T("OUT_OF_SERVICE"),	&m_bOutOfService,		true);
    m_pIniHelper->Link(bSave, GetName(), _T("DEVICE_TYPE"),		&m_strControlType,		(CString)_T("CL_CAMERA_SP5000C_NI_BOARD"));

    return m_pIniHelper->IsReadEnable();
}

void CAlignCameraMgr::Finalize()
{
	m_bClose = true;

	IMAQ_DISPOSE(m_pSnapImage);
	IMAQ_DISPOSE(m_pOverlayImage);
	SAFE_DELETE(m_pimaqctrlROI);
	

    //----------------------------------------------------------------------
    // 데이터 삭제
    //----------------------------------------------------------------------
	if(m_pFunction)
	{
		delete m_pFunction;
		m_pFunction = nullptr;
	}
	
	if(m_pIniHelper)
    {
        delete m_pIniHelper;
        m_pIniHelper = nullptr;
    }
}

bool CAlignCameraMgr::InitDevice()
{
	if (m_pFunction != nullptr)
		return 	m_pFunction->CreateGrabber();
	else
		return false;
}

void CAlignCameraMgr::CloseDevice()
{
	if (m_pFunction != nullptr)
		m_pFunction->DeleteGrabber();
}

CString	CAlignCameraMgr::GetDeviceName()
{
	return GetName();
}

bool CAlignCameraMgr::IsConnected()
{
	if (m_pFunction != nullptr)
		return m_pFunction->IsOpenGrabber();
	else 
		return false;
}

bool CAlignCameraMgr::IsOutOfService()
{
	return m_bOutOfService;
}


bool CAlignCameraMgr::SnapImage(short* pbyImage)
{
	CCriticalSectionSync single(&m_cs_get, TRUE);

	if (m_pFunction != nullptr)
		return m_pFunction->SnapImage(pbyImage);
	else
		return 0;
}

// 전용 함수 - 시퀀스 연동
bool CAlignCameraMgr::GetSnapImage(long nExpTime_ms, BYTE *pbyImage)
{
	CCriticalSectionSync single(&m_cs_get, TRUE);

	if (m_pFunction != nullptr)
		return m_pFunction->GetSnapImage(nExpTime_ms, pbyImage);
	else
		return 0;
}

bool CAlignCameraMgr::ResetGrabber()
{
	CCriticalSectionSync single(&m_cs_get, TRUE);

	if (m_pFunction != nullptr)
		return m_pFunction->ResetGrabber();
	else
		return 0;
}

int CAlignCameraMgr::GetImageGrabSizeX()
{
	if (m_pFunction != nullptr)
		return m_pFunction->GetImageGrabSizeX();
	else
		return 0;
}

int CAlignCameraMgr::GetImageGrabSizeY()
{
	if (m_pFunction != nullptr)
		return m_pFunction->GetImageGrabSizeY();
	else
		return 0;
}

int CAlignCameraMgr::GetParam_CameraExposureTime_ms()
{
	if (m_bClose != true)
	{
		if (m_pFunction != nullptr)
			return m_pFunction->GetParam_CameraExposureTime_ms();
		else
			return 0;
	}

	return 0;
}

int CAlignCameraMgr::GetParam_CameraGain()
{
	if (m_pFunction != nullptr)
		return m_pFunction->GetParam_CameraGain();
	else
		return 0;
}

int CAlignCameraMgr::SetParam_CameraExposureTime_ms(long nExpTime_ms)
{
	if (m_pFunction != nullptr)
		return m_pFunction->SetParam_CameraExposureTime_ms(nExpTime_ms);
	else
		return 0;
}

int CAlignCameraMgr::SetParam_CameraGain(long nGain)
{
	if (m_pFunction != nullptr)
		return m_pFunction->SetParam_CameraGain(nGain);
	else
		return 0;
}

int CAlignCameraMgr::SnapImage(Image* pImage, bool bDisplay)
{
	int nRet = VTM_RV_SUCCESSED;
	int iret = 0;
	bool ret = false;

	Image* pSnapImage = imaqCreateImage(IMAQ_IMAGE_RGB, 7);

	if(IsConnected() != true)
	{
		CString sFileName;
	//	sFileName.Format(_T("D:\\Temp\\Align%d.png"), m_iCCDNum + 1);
	
		if (m_pimaqctrlROI != NULL)
		{
			CString strFilePath;
			sFileName.Format(_T("%s%sAlign%d.png"), m_strRecipePath, STR_DIR_PATTERN, m_iCCDNum + 1);
			if (m_pimaqctrlROI->LoadPattern(sFileName, pSnapImage) == TRUE)
				ret = true;
		}
	}
	else
	{		
		if (SnapImage((short*)pSnapImage) == true)
		{
			ret = true;
		}

		/*int nExposureTime = 0;
		int nGrabSizeW = 0, nGrabSizeH = 0;
		nGrabSizeW = GetImageGrabSizeX();
		nGrabSizeH = GetImageGrabSizeY();

		size_t nPixel = (nGrabSizeW)*nGrabSizeH * 4;
		BYTE* ucImg = new BYTE[nPixel];
		memset(ucImg, 0, sizeof(BYTE) * nPixel);

		if (GetSnapImage(nExposureTime, ucImg) == true)
		{
			imaqArrayToImage(pSnapImage, (BYTE*)ucImg, nGrabSizeW, nGrabSizeH);
			ret = true;
		}

		delete[] ucImg;*/
	}

	if (ret == true)
	{
		ImageType type;
		imaqGetImageType(pImage, &type);

		if (bDisplay == true)
		{
			imaqClearOverlay(m_pOverlayImage, NULL);

			if (type == IMAQ_IMAGE_RGB)
				iret = imaqDuplicate(pImage, pSnapImage);
			else
				iret = imaqCast(pImage, pSnapImage, type, NULL, 0);
		}
		else
		{
			if (type == IMAQ_IMAGE_RGB)
				iret = imaqDuplicate(pImage, pSnapImage);
			else
				iret = imaqCast(pImage, pSnapImage, type, NULL, 0);
		}

	//	imaqDuplicate(m_pSnapImage, pSnapImage);

		if (iret == 0)
		{
			//PixelValue value;
			//value.grayscale = 0;
			//imaqShift(pImage, pImage, int shiftX, int shiftY, value);

			

			CString str;
			str.Format(_T("SnapImage"));
			TRACE("%s(%i) : %s\n", __FILE__, __LINE__, str);

			ret = false;
		}
		else
		{
			ret = true;
		}
	}
	
	imaqDispose(pSnapImage);

	if (ret != true)
		nRet = VISION_SNAP_FAIL;

	return nRet;
}

int CAlignCameraMgr::SetShowROI(bool bShow, Rect rectROI)
{
	if (bShow == true)
	{
		m_rectROI = rectROI;
	}
		
	m_bShowROI = bShow;
		
	/*imaqClearOverlay(m_pOverlayImage, NULL);

	RGBValue rgbval;
	rgbval = IMAQ_RGB_RED;

	imaqOverlayRect(m_pSnapImage, rectROI, &rgbval, IMAQ_DRAW_VALUE, NULL);
	imaqMergeOverlay(m_pOverlayImage, m_pSnapImage, NULL, 256, NULL);

	imaqClearOverlay(m_pSnapImage, NULL);*/
	return 0;
}

void CAlignCameraMgr::GetShowROI(bool* pbShow, Rect* prectROI)
{
	*pbShow = m_bShowROI;
	*prectROI = m_rectROI;
}

int CAlignCameraMgr::PtrnMatch(CPtrnInfo* pPtrnInfo, float* pfScore, int nMeasLens, bool bGlass)
{
	int nRet = VTM_RV_SUCCESSED;

	CRect rect;
	BOOL bSearch = FALSE;
	CString strLog = _T("");
	CString strName;
	Rect rectROI;
	ZeroMemory(&rectROI, sizeof(Rect));

	nRet = SnapImage(m_pSnapImage);	// jsyoon 2022.06.17 remove
	imaqDuplicate(pPtrnInfo->m_pImgInput, m_pSnapImage);
	Image* pSnapImage = imaqCreateImage(IMAQ_IMAGE_RGB, INT_IMAGE_BORDER_SIZE);
	imaqDuplicate(pSnapImage, m_pSnapImage);

	imaqClearOverlay(m_pOverlayImage, NULL);

	if (nRet == VTM_RV_SUCCESSED)
	{
		if (m_pimaqctrlROI)
		{
			// Learn Pattern
			BOOL bLearn = FALSE;
			CString strPath, strTime;

			strName.Format(_T("%s"), pPtrnInfo->m_strName);
			if (strName.IsEmpty() != TRUE)
			{
				if(nMeasLens == LENS_USE_MEAS)
					strPath.Format(_T("%s%s%s"), m_strRecipePath, STR_DIR_PATTERN, strName);
				else
					strPath.Format(_T("%s%s%s"), m_strRecipePath, STR_DIR_ALIGN, strName);

				bLearn = m_pimaqctrlROI->LoadPattern(strPath, pPtrnInfo->m_pImgOrg);

				if (bLearn == false)
				{
					strPath.Format(_T("%s%s%s"), m_strRecipePath, STR_DIR_ALIGN, strName);
					bLearn = m_pimaqctrlROI->LoadPattern(strPath, pPtrnInfo->m_pImgOrg);
				}

				if (bLearn == true)
				{
					// Search Pattern
					if (nMeasLens == LENS_USE_ALIGN)
					{
						bSearch = m_pimaqctrlROI->SearchPattern(pPtrnInfo->m_pImgInput, pPtrnInfo->m_pImgOrg, (float)pPtrnInfo->m_dThreshold, bLearn, MATCH_SCORE, FALSE, &rect, pfScore, pPtrnInfo->m_iMethod, pPtrnInfo->m_nThreshold);
					}
					else
					{
						//if (m_iCCDNum == 1)
						//	bSearch = m_pimaqctrlROI->SearchPattern(pPtrnInfo->m_pImgInput, pPtrnInfo->m_pImgOrg, (float)pPtrnInfo->m_dThreshold, bLearn, MATCH_DISTANCE, TRUE, &rect, pfScore);
						//else
						bSearch = m_pimaqctrlROI->SearchPattern(pPtrnInfo->m_pImgInput, pPtrnInfo->m_pImgOrg, (float)pPtrnInfo->m_dThreshold, bLearn, MATCH_DISTANCE, TRUE, &rect, pfScore, 0, 0, 1);
					}
				}
			}
			else
			{
				int iLoop = 0;
				for (iLoop = 0; iLoop < 10; iLoop++)
				{
					strName.Format(_T("%s"), pPtrnInfo->m_cMultiPtrn[iLoop]);
					if (strName.IsEmpty() != TRUE)
					{
						strPath.Format(_T("%s%s%s"), m_strRecipePath, STR_DIR_PATTERN, strName);

						bLearn = m_pimaqctrlROI->LoadPattern(strPath, pPtrnInfo->m_pImgOrg);

						// Search Pattern
						if (nMeasLens == LENS_USE_ALIGN)
						{
							bSearch = m_pimaqctrlROI->SearchPattern(pPtrnInfo->m_pImgInput, pPtrnInfo->m_pImgOrg, (float)pPtrnInfo->m_dThreshold, bLearn, MATCH_SCORE, FALSE, &rect, pfScore, 1, 30);

							//	bSearch = m_pimaqctrlROI->SearchPattern(pPtrnInfo->m_pImgInput, pPtrnInfo->m_pImgOrg, (float)pPtrnInfo->m_dThreshold, bLearn, MATCH_SCORE, FALSE, &rect, pfScore, pPtrnInfo->m_iMethod, pPtrnInfo->m_nThreshold);
						}
						else
						{
							//if (m_iCCDNum == 1)
							//	bSearch = m_pimaqctrlROI->SearchPattern(pPtrnInfo->m_pImgInput, pPtrnInfo->m_pImgOrg, (float)pPtrnInfo->m_dThreshold, bLearn, MATCH_DISTANCE, TRUE, &rect, pfScore);
							//else
								bSearch = m_pimaqctrlROI->SearchPattern(pPtrnInfo->m_pImgInput, pPtrnInfo->m_pImgOrg, (float)pPtrnInfo->m_dThreshold, bLearn, MATCH_DISTANCE, TRUE, &rect, pfScore, 0, 0, 1);
						}
					}

					if (bSearch == TRUE)
					{
						pPtrnInfo->m_strName.Format(_T("%s"), strName); // jsyoon 20210519 add
						break;
					}
				}
			}

			if (bSearch == TRUE)
			{
				double  mmPerPixelX = 3.78, mmPerPixelY = 3.78;
				mmPerPixelX /= 1000.0;
				mmPerPixelY /= 1000.0;

				// Search Overlay Image Save
				RGBValue rectColor = IMAQ_INIT_RGB_RED;
				rectROI.left = rect.left;
				rectROI.top = rect.top;
				rectROI.height = rect.Height();
				rectROI.width = rect.Width();
		//		m_pimaqctrlROI->SetOverLayRect(rectROI, rectColor);

				Point TextPoint;
				PointFloat midPoint;
				CString strROI;
				TextPoint.x = 5;
				TextPoint.y = 50;

				int iW = 0, iH = 0;
				imaqGetImageSize(pPtrnInfo->m_pImgInput, &iW, &iH);

				midPoint.x = (float)(rect.left + (rect.Width() / 2)) - (iW / 2);
				midPoint.y = (float)(rect.top  + (rect.Height() / 2)) - (iH / 2);

				pPtrnInfo->m_dRealX = (midPoint.x * mmPerPixelX);
				pPtrnInfo->m_dRealY = (midPoint.y * mmPerPixelY);

		//		strROI.Format(_T("POINT : %.1f, %.1f"), midPoint.x, midPoint.y);
		//		ZeroMemory(&szName, sizeof(szName));
		//		strcpy_s(szName, _countof(szName), gf_StringToChar(strROI));

		//		m_pimaqctrlROI->SetOverText(TextPoint, szName, rectColor);
		//		m_pimaqctrlROI->SetOverLayCrose(midPoint, rectColor);

				// jsyoon 20180914 add
				// Cross add
				midPoint.x = (float)iW / 2;
				midPoint.y = (float)iH / 2;
				int iWide = 100, iWideW = 0, iWideH = 0;
				iWideW = iW / 640;
				iWideH = iH / 480;

				if (iWideW > iWideH)
					iWide *= iWideW;
				else
					iWide *= iWideH;


				

		//		m_pimaqctrlROI->SetOverLayCrose(midPoint, rectColor, iWide);
				//////////////////////////////////////////////////////////////////////////

		//		m_pimaqctrlROI->CopyOverlayImage(pPtrnInfo->m_pImgInput);		// Overlay copy
			}

			SetShowROI((bool)bSearch, rectROI);

		//	imaqOverlayRect(m_pSnapImage, rectROI, &IMAQ_RGB_RED, IMAQ_DRAW_VALUE, NULL);
		//	imaqMergeOverlay(m_pOverlayImage, m_pSnapImage, NULL, 256, NULL);
			imaqOverlayRect(pSnapImage, rectROI, &IMAQ_RGB_RED, IMAQ_DRAW_VALUE, NULL);
			imaqMergeOverlay(m_pOverlayImage, pSnapImage, NULL, 256, NULL);
		}

		if (nMeasLens == LENS_USE_ALIGN)
		{
			if(bGlass == true)
				SaveImage(m_pOverlayImage, 0);		// Glass
			else
				SaveImage(m_pOverlayImage, 1);		// Probe
		}
		else
		{
			SaveImage(m_pOverlayImage, 2);		// MEASURE
		}
	}

	//int iW = 0, iH = 0;
	//if (bSearch == TRUE)
	//{
	//	imaqGetImageSize(pPtrnInfo->m_pImgInput, &iW, &iH);
	//	//m_rectPatROI.left = rect.Width() / 2 + rect.left;
	//	//m_rectPatROI.top = rect.Height() / 2 + rect.top;
	//	//m_rectPatROI.height = m_rectPatROI.top - iH / 2; /*rect.Height()*/;
	//	//m_rectPatROI.width = m_rectPatROI.left - iW / 2;/*rect.Width()*/;
	//}
	//else
	//{
	//	//m_rectPatROI.left = 0;
	//	//m_rectPatROI.top = 0;
	//	//m_rectPatROI.height = 0;
	//	//m_rectPatROI.width = 0;
	//}

//	strLog.Format(_T("MeasPtrnMatch Ret,%d,Score,%.3f, X, Y : %4d/%4d, %4d/%4d"), bSearch, *pfScore, m_rectPatROI.left, iW, m_rectPatROI.top, iH);

	IMAQ_DISPOSE(pSnapImage);

	// jsyoon 20210427 change
	if (bSearch)
	{
//		EventLog(strLog, LOG_LEVEL_D);
		nRet = VTM_RV_SUCCESSED;
	}
	else
	{
//		EventLog(strLog, LOG_LEVEL_E);
		if (nMeasLens == LENS_USE_ALIGN)
			nRet = VISION_ALIGN_DETECTED_FAIL;
		else
			nRet = VISION_PATTERN_DETECTED_FAIL;
	}
	///

	return nRet;
}

float CAlignCameraMgr::CalcAngle(COORDINATE stBase1, COORDINATE stBase2, COORDINATE stDetect1, COORDINATE stDetect2)
{
	float fAngle = 0;
	PointFloat DPonitFStart, DPonitFEnd, NPonitFStart, NPonitFEnd;

	DPonitFStart.x = (float)stBase1.m_dx;		DPonitFStart.y = (float)stBase1.m_dy;
	DPonitFEnd.x = (float)stBase2.m_dx;		DPonitFEnd.y = (float)stBase2.m_dy;
	NPonitFStart.x = (float)stDetect1.m_dx;	NPonitFStart.y = (float)stDetect1.m_dy;
	NPonitFEnd.x = (float)stDetect2.m_dx;	NPonitFEnd.y = (float)stDetect2.m_dy;

	fAngle = m_pimaqctrlROI->CalcAngle2(DPonitFStart, DPonitFEnd, NPonitFStart, NPonitFEnd);	// jsyoon 20141203 change
	return fAngle;
}

int CAlignCameraMgr::SaveImage(Image* pImage, int iType)
{
	CString strPath, strSaveImage;
	CString strCellID;

	switch (iType)
	{
	case 0:
	{
		strPath.Format(_T("%s%s\\%s\\%s\\"), m_strResultPath, _T("ALIGN"), CSystemParamShare::GetInstance()->GetRecipeID(), CSystemParamShare::GetInstance()->GetGlassID());

	//	strSaveImage.Format(_T("%s%s_%s_%s_%d_%s.png"), strPath, CSystemParamShare::GetInstance()->GetRecipeID(), CSystemParamShare::GetInstance()->GetGlassID(), _T("GlassAlign"), m_iCCDNum + 1, gf_GetCurTimeString());
		strSaveImage.Format(_T("%s%s_%s_%s_%s_%d.png"), strPath, CSystemParamShare::GetInstance()->GetRecipeID(), CSystemParamShare::GetInstance()->GetGlassID(), gf_GetCurTimeString(), _T("GlassAlign"), m_iCCDNum + 1);
	}
	break;
	case 1:
	{
		strPath.Format(_T("%s%s\\%s\\%s\\"), m_strResultPath, _T("ALIGN"), CSystemParamShare::GetInstance()->GetRecipeID(), CSystemParamShare::GetInstance()->GetGlassID());

	//	strSaveImage.Format(_T("%s%s_%s_%s_%d_%s.png"), strPath, CSystemParamShare::GetInstance()->GetRecipeID(), CSystemParamShare::GetInstance()->GetGlassID(), _T("ProbeAlign"), m_iCCDNum + 1, gf_GetCurTimeString());
		strSaveImage.Format(_T("%s%s_%s_%s_%s_%d.png"), strPath, CSystemParamShare::GetInstance()->GetRecipeID(), CSystemParamShare::GetInstance()->GetGlassID(), gf_GetCurTimeString(), _T("ProbeAlign"), m_iCCDNum + 1);
	}
	break;
	case 2:
	{
		strPath.Format(_T("%s%s\\%s\\%s\\"), m_strResultPath, _T("ALIGN"), CSystemParamShare::GetInstance()->GetRecipeID(), CSystemParamShare::GetInstance()->GetGlassID());

	//	strSaveImage.Format(_T("%s%s_%s_%s_%d_%s.png"), strPath, CSystemParamShare::GetInstance()->GetRecipeID(), CSystemParamShare::GetInstance()->GetGlassID(), _T("Meas_Ptrn"), m_iCCDNum + 1, gf_GetCurTimeString() );
		strSaveImage.Format(_T("%s%s_%s_%s_%s_%d.png"), strPath, CSystemParamShare::GetInstance()->GetRecipeID(), CSystemParamShare::GetInstance()->GetGlassID(), gf_GetCurTimeString(), _T("Meas_Ptrn"), m_iCCDNum + 1);
	}
	break;
	case 3:
	{
		strPath.Format(_T("%s%s\\%s\\%s\\"), m_strResultPath, _T("Contact"), CSystemParamShare::GetInstance()->GetRecipeID(), CSystemParamShare::GetInstance()->GetGlassID());

	//	strSaveImage.Format(_T("%s%s_%s_%s_%d_%s.png"), strPath, CSystemParamShare::GetInstance()->GetRecipeID(), CSystemParamShare::GetInstance()->GetGlassID(), _T("Meas_Ptrn"), m_iCCDNum + 1, gf_GetCurTimeString());
		strSaveImage.Format(_T("%s%s_%s_%s_%s_%d.png"), strPath, CSystemParamShare::GetInstance()->GetRecipeID(), CSystemParamShare::GetInstance()->GetGlassID(), gf_GetCurTimeString(), _T("Meas_Ptrn"), m_iCCDNum + 1);
	}
	break;
	case 4:
	{
		strPath.Format(_T("%s%s\\%s\\%s\\"), m_strResultPath, _T("AGING"), CSystemParamShare::GetInstance()->GetRecipeID(), CSystemParamShare::GetInstance()->GetGlassID());

		strSaveImage.Format(_T("%s%s_%s_%s_%s_%d.png"), strPath, CSystemParamShare::GetInstance()->GetRecipeID(), CSystemParamShare::GetInstance()->GetGlassID(), gf_GetCurTimeString(), _T("AgingPrbe4"), m_iCCDNum + 1);
	}
	break;
	case 5:
	{
		strPath.Format(_T("%s%s\\%s\\%s\\"), m_strResultPath, _T("AGING"), CSystemParamShare::GetInstance()->GetRecipeID(), CSystemParamShare::GetInstance()->GetGlassID());

		strSaveImage.Format(_T("%s%s_%s_%s_%s_%d.png"), strPath, CSystemParamShare::GetInstance()->GetRecipeID(), CSystemParamShare::GetInstance()->GetGlassID(), gf_GetCurTimeString(), _T("AgingPrbe3"), m_iCCDNum + 1);
	//	strSaveImage.Format(_T("%s%s_%s_%s_%s.png"), strPath, CSystemParamShare::GetInstance()->GetRecipeID(), CSystemParamShare::GetInstance()->GetGlassID(), _T("AgingPrbe3"), m_iCCDNum + 1, gf_GetCurTimeString());
	}
	break;
	case 6:
	{
		strPath.Format(_T("%s%s\\%s\\%s\\"), m_strResultPath, _T("AGING"), CSystemParamShare::GetInstance()->GetRecipeID(), CSystemParamShare::GetInstance()->GetGlassID());

		strSaveImage.Format(_T("%s%s_%s_%s_%s_%d.png"), strPath, CSystemParamShare::GetInstance()->GetRecipeID(), CSystemParamShare::GetInstance()->GetGlassID(), gf_GetCurTimeString(), _T("AgingPrbe2"), m_iCCDNum + 1);
	//	strSaveImage.Format(_T("%s%s_%s_%s_%d_%s.png"), strPath, CSystemParamShare::GetInstance()->GetRecipeID(), CSystemParamShare::GetInstance()->GetGlassID(), _T("AgingPrbe2"), m_iCCDNum + 1, gf_GetCurTimeString());
	}
	break;
	case 7:
	{
		strPath.Format(_T("%s%s\\%s\\%s\\"), m_strResultPath, _T("AGING"), CSystemParamShare::GetInstance()->GetRecipeID(), CSystemParamShare::GetInstance()->GetGlassID());

		strSaveImage.Format(_T("%s%s_%s_%s_%s_%d.png"), strPath, CSystemParamShare::GetInstance()->GetRecipeID(), CSystemParamShare::GetInstance()->GetGlassID(), gf_GetCurTimeString(), _T("AgingPrbe1"), m_iCCDNum + 1);
	//	strSaveImage.Format(_T("%s%s_%s_%s_%d_%s.png"), strPath, CSystemParamShare::GetInstance()->GetRecipeID(), CSystemParamShare::GetInstance()->GetGlassID(), _T("AgingPrbe1"), m_iCCDNum + 1, gf_GetCurTimeString());
	}
	break;
	case 8:
	{
		strPath.Format(_T("%s%s\\%s\\%s\\"), m_strResultPath, _T("AGING"), CSystemParamShare::GetInstance()->GetRecipeID(), CSystemParamShare::GetInstance()->GetGlassID());

		strSaveImage.Format(_T("%s%s_%s_%s_%s_%d.png"), strPath, CSystemParamShare::GetInstance()->GetRecipeID(), CSystemParamShare::GetInstance()->GetGlassID(), gf_GetCurTimeString(), _T("MeasPrbe"), m_iCCDNum + 1);
		//	strSaveImage.Format(_T("%s%s_%s_%s_%d_%s.png"), strPath, CSystemParamShare::GetInstance()->GetRecipeID(), CSystemParamShare::GetInstance()->GetGlassID(), _T("AgingPrbe1"), m_iCCDNum + 1, gf_GetCurTimeString());
	}
	break;
	default:
		break;
	}

	
	gf_MakeDir(strPath);

	// Save Image
	USES_CONVERSION;
	imaqWritePNGFile2(pImage, T2A(strSaveImage), 750, NULL, FALSE);

	return 1;
}


int CAlignCameraMgr::SaveCurImage(int iType)
{
	int nRet = VTM_RV_SUCCESSED;

	nRet = SnapImage(m_pSnapImage);
	if (nRet == VTM_RV_SUCCESSED)
	{
		nRet = SaveImage(m_pSnapImage, iType);
	}
	
	return nRet;
}