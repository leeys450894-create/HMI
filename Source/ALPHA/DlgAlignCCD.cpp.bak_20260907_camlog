// DlgAlignCCD.cpp : 구현 파일입니다.
//

#include "pch.h"
#include "ALPHA.h"
#include "DlgAlignCCD.h"
#include "afxdialogex.h"
#include "AuroraMgr.h"
#include "Manager\AlignCameraMgr.h"
#include "common\IMAQImageCtrl\IMAQImage.h"
#include "D_CCD_Review.h"

// CDlgAlignCCD 대화 상자입니다.
#define		WM_MSG_DISPLAY_IMAGE (WM_USER + 0x100 + 401)

IMPLEMENT_DYNAMIC(CDlgAlignCCD, CMainSubDlgWin)

bool g_bShowImage;

CDlgAlignCCD::CDlgAlignCCD(CAuroraMgr *pAuroraMgr, int nNum, CWnd* pParent /*=NULL*/)
	: CMainSubDlgWin(IDD_DLG_SUB_CCD, pParent)
	, m_pAuroraMgr(pAuroraMgr)
	, m_iCCDNum(nNum)
	, m_iGrabSizeW(0)
	, m_iGrabSizeH(0)
	, m_iMoniImageWnd(0)
//	, m_iMoniImageWnd2(0)
	, m_pWorkThread(NULL)
	, m_iFocus(0)
	, m_nLiveTick(0)
	, m_bShowCrossLine(true)
	, m_nGrabNgCount(0)
	, m_fScale(0)
	, m_bShowImage2(false)
	, m_bCCDLive(true)
{
	g_bShowImage = false;

	m_pAqcImage = imaqCreateImage(IMAQ_IMAGE_RGB, 7);
	m_pLiveImage = imaqCreateImage(IMAQ_IMAGE_RGB, 7);

	m_hEvtKillThread   = CreateEvent(NULL, FALSE, FALSE, NULL);	// auto reset, initially Reset
	m_hEvtThreadKilled = CreateEvent(NULL, FALSE, FALSE, NULL);	// auto reset, initially Reset	

	m_pimaqctrlROI = nullptr;
	m_pCCDReview = nullptr;
}

CDlgAlignCCD::~CDlgAlignCCD()
{
	if (m_pCCDReview != nullptr)
	{
		m_pCCDReview->DestroyWindow();
		m_pCCDReview = nullptr;
	}

	TerminateWorkerThread();

	WaitTerminateWorkerThread();

	imaqDispose(m_pAqcImage);
	imaqDispose(m_pLiveImage);

	CloseHandle(m_hEvtKillThread);
	CloseHandle(m_hEvtThreadKilled);
}

void CDlgAlignCCD::DoDataExchange(CDataExchange* pDX)
{
	CMainSubDlgWin::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDlgAlignCCD, CMainSubDlgWin)
	ON_WM_TIMER()
	ON_MESSAGE(WM_MSG_DISPLAY_IMAGE, &CDlgAlignCCD::OnMsgDisplayImage)
END_MESSAGE_MAP()


// CDlgAlignCCD 메시지 처리기입니다.


int CDlgAlignCCD::CCDDisplayDlg(CWnd* pHostWnd, CRect rect)
{
	//m_iCCDNum = 
	//CString str = _T("img0");
	//str.Format(_T("%s"), m_strCCDInterface);

	//// jsyoon 20210118 change
	//int iCamNum = 0, nFine = 0;
	//nFine = str.Find(_T("cam"));
	//if (nFine >= 0)
	//{
	//	iCamNum = _ttoi(str.Right(1));
	//}

	CWnd* pWnd = GetDlgItem(IDC_STC_CCD_POS);
	pWnd->MoveWindow(0, 0, rect.Width(), rect.Height());

	return 0;
}


int CDlgAlignCCD::DoWorkStartGrab()
{
	int nRet = 0;
	bool bRet = false;
	if (m_pAuroraMgr->GetAlignCameraMgr(m_iCCDNum)->IsOutOfService() != true)
	{
		if (m_pAuroraMgr->GetAlignCameraMgr(m_iCCDNum)->IsConnected() != true)
		{
			bRet = m_pAuroraMgr->GetAlignCameraMgr(m_iCCDNum)->InitDevice();

			if (bRet == true)
			{
				m_iGrabSizeW = m_pAuroraMgr->GetAlignCameraMgr(m_iCCDNum)->GetImageGrabSizeX();
				m_iGrabSizeH = m_pAuroraMgr->GetAlignCameraMgr(m_iCCDNum)->GetImageGrabSizeY();
			}
		}
	}
	else
	{
		bRet = true;

		m_iGrabSizeW = 1280;
		m_iGrabSizeH = 960;
	}
	

	if (bRet == true)
	{
		nRet = 1;

		double dScaleX = 0, dScaleY = 0, dScale = 0;
		CRect rect;
		this->GetWindowRect(rect);
		imaqSetWindowSize(m_iMoniImageWnd, m_iGrabSizeW, m_iGrabSizeH);

		if (m_pCCDReview != nullptr)
		{
			m_pCCDReview->MoveWindow(0, 0, m_iGrabSizeW, m_iGrabSizeH);
			m_pCCDReview->SetWindowSize(m_iGrabSizeW, m_iGrabSizeH);
		}

//		imaqSetWindowSize(m_iMoniImageWnd2, m_iGrabSizeW, m_iGrabSizeH);
		imaqDisplayImage(m_pAqcImage, m_iMoniImageWnd, TRUE);

		dScaleX = (double)rect.Width()/*320*/ / m_iGrabSizeW;
		dScaleY = (double)rect.Height()/*266*/ / m_iGrabSizeH;
		dScale = (dScaleX > dScaleY ? dScaleY : dScaleX);
		imaqZoomWindow2(m_iMoniImageWnd, (float)dScale, (float)dScale, IMAQ_NO_POINT);

		m_fScale = (float)dScale;
//		
		BeginThread_WorkProc();
		//size_t nPixel = m_iGrabSizeW * m_iGrabSizeH * 4;
		//BYTE *ucImg = new BYTE[nPixel];
		//memset(ucImg, 0, sizeof(BYTE)*nPixel);
		//m_pAuroraMgr->GetAlignCameraMgr(m_iCCDNum)->GetSnapImage(3000, ucImg);
		//imaqArrayToImage(m_pAqcImage, (BYTE*)ucImg, m_iGrabSizeW, m_iGrabSizeH);

		//imaqWritePNGFile(m_pAqcImage, "D:\\Temp\\pSnapImage.png", 750, NULL);
		//delete[] ucImg;
	}

	return nRet;
}


void CDlgAlignCCD::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	if (nIDEvent == 100)
	{
//		DisplayImage();
	}

	CMainSubDlgWin::OnTimer(nIDEvent);
}


BOOL CDlgAlignCCD::OnInitDialog()
{
	CMainSubDlgWin::OnInitDialog();

	// TODO:  여기에 추가 초기화 작업을 추가합니다.
	SetBackgroundColor(RGB(128, 128, 128));

	imaqGetWindowHandle(&m_iMoniImageWnd);
	m_hMoniImage = (HWND)imaqGetSystemWindowHandle(m_iMoniImageWnd);

	// To make child window
	::SetParent(m_hMoniImage, this->GetSafeHwnd());

	Point pt = { 0, 0 };
	imaqMoveWindow(m_iMoniImageWnd, pt);


	// Window attribute could not be set by imaqSetupWindow as what I want..
	// So, I used SetWindowLong for making child window.
	// But, this scheme support displaying image, but not support ROI tools...
	// So, no way except using extra top-level window for selecting pattern, etc.
	::SetWindowLong(m_hMoniImage, GWL_STYLE, WS_CHILDWINDOW | WS_VISIBLE | WS_OVERLAPPED);

	/*if (m_pimaqctrlROI == nullptr)
	{
		m_pimaqctrlROI = new CIMAQImageCtrl();
	}

	if (m_pimaqctrlROI != nullptr)
	{
		m_pimaqctrlROI->IMAQWindowCreate();
	}*/
		
	/*imaqGetWindowHandle(&m_iMoniImageWnd2);
	m_hMoniImage2 = (HWND)imaqGetSystemWindowHandle(m_iMoniImageWnd);
	::SetParent(m_hMoniImage2, this->GetSafeHwnd());
	imaqMoveWindow(m_iMoniImageWnd2, pt);
	::SetWindowLong(m_hMoniImage2, GWL_STYLE, WS_POPUPWINDOW | WS_VISIBLE | WS_OVERLAPPED);*/

	imaqSetEventCallback(CBEventROIWnd, FALSE);

	m_pCCDReview = new CD_CCD_Review();
	m_pCCDReview->Create(IDD_DLG_SUB_CCD_FULL, this);
	m_pCCDReview->MoveWindow(0,0, 1280, 900);
	m_pCCDReview->ShowWindow(SW_HIDE);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}

void CDlgAlignCCD::BeginThread_WorkProc()
{
	if (m_pWorkThread == NULL)
	{
		m_pWorkThread =
			AfxBeginThread(WorkThreadProbeProc, this);
	}
}


int CDlgAlignCCD::TerminateWorkerThread()
{
	if (m_pWorkThread != NULL)	// jsyoon 20141126 add
	{
		SetEvent(m_hEvtKillThread);
	}

	return 0;
}


int CDlgAlignCCD::WaitTerminateWorkerThread()
{
	if (m_pWorkThread != NULL)	// jsyoon 20141126 add
	{
		if (WaitForSingleObject(m_hEvtThreadKilled, 100)
			== WAIT_OBJECT_0)
		{
			m_pWorkThread = NULL;
		}
	}

	if (m_pWorkThread == NULL)
		TRACE("CCD NULL @@@@@@@@@@@\n");

	if (m_pAuroraMgr->GetAlignCameraMgr(m_iCCDNum)->IsConnected() != true)
	{
		m_pAuroraMgr->GetAlignCameraMgr(m_iCCDNum)->CloseDevice();
	}

	return 0;
}

UINT CDlgAlignCCD::WorkThreadProbeProc(LPVOID pParam)
{
	CDlgAlignCCD* pProc = (CDlgAlignCCD*)pParam;

	while (TRUE)
	{
		// m_hEvtKillThread Event 시그널이 Set 되면 Loop를 빠져 나간다
		if (WaitForSingleObject(pProc->m_hEvtKillThread, 100)
			== WAIT_OBJECT_0)
		{
			break; // Terminate this thread by exiting the proc.
		}

		///////////////////////////////
		pProc->GrabImage();	//실제 작업을 진행하는 함수
	}

	SetEvent(pProc->m_hEvtThreadKilled);				// "나(쓰레드) 죽었다"라고 해 줌

	return 0;
}

void CDlgAlignCCD::DisplayImage()
{
	int	   iMoniWnd = m_iMoniImageWnd;
	imaqDuplicate(m_pLiveImage, m_pAqcImage);
	if(m_bShowCrossLine == true ) ShowCrossLine(m_pLiveImage);
	OverlayText(m_pLiveImage);

	imaqDisplayImage(m_pLiveImage, iMoniWnd, TRUE);
}

void CDlgAlignCCD::DisplayImage2()
{
//	if (m_bShowImage2 == false)
//	if(g_bShowImage == false)
//	{
//		return;
//	}

	if (m_pCCDReview->IsWindowVisible() == true)
	{
		imaqDuplicate(m_pLiveImage, m_pAqcImage);
		if (m_bShowCrossLine == true) ShowCrossLine(m_pLiveImage);
		OverlayText(m_pLiveImage);

		m_pCCDReview->DisplayImage(m_pLiveImage);
	}

//	int	   iMoniWnd = m_iMoniImageWnd2;
	int nVisible = 0;
	
//	imaqIsWindowVisible(iMoniWnd, &nVisible);

	if (nVisible == 1)
	{
		imaqDuplicate(m_pLiveImage, m_pAqcImage);
		if (m_bShowCrossLine == true) ShowCrossLine(m_pLiveImage);
		OverlayText(m_pLiveImage);

//		imaqDisplayImage(m_pLiveImage, iMoniWnd, TRUE);
	}
}

void CDlgAlignCCD::GrabImage()
{
	if (m_pAuroraMgr->GetAlignCameraMgr(m_iCCDNum)->SnapImage(m_pAqcImage, true) == VTM_RV_SUCCESSED)
	{
		m_nLiveTick = GetTickCount() / 100;

		::PostMessage(this->m_hWnd, WM_MSG_DISPLAY_IMAGE, (WPARAM)0, (LPARAM)0);
	}
	else
	{
		m_nGrabNgCount++;

		if (m_nGrabNgCount > 2)
		{
			m_nGrabNgCount = 0;

			m_pAuroraMgr->GetAlignCameraMgr(m_iCCDNum)->ResetGrabber();
		}
	}

	//size_t nPixel = m_iGrabSizeW * m_iGrabSizeH * 4;
	//BYTE *ucImg = new BYTE[nPixel];
	//memset(ucImg, 0, sizeof(BYTE)*nPixel);

	//int nExposureTime = 0;

	//if (m_pAuroraMgr->GetAlignCameraMgr(m_iCCDNum)->IsOutOfService() == true)
	//{
	//	CString sFileName;
	//	sFileName.Format(_T("D:\\Temp\\Align%d.png"), m_iCCDNum + 1);
	//	USES_CONVERSION;
	//	imaqReadVisionFile(m_pAqcImage, T2A(sFileName), NULL, NULL);

	//	m_nLiveTick = GetTickCount() / 100;

	//	::PostMessage(this->m_hWnd, WM_MSG_DISPLAY_IMAGE, (WPARAM)0, (LPARAM)0);
	//}
	//else
	//{
	//	//	nExposureTime = m_pAuroraMgr->GetAlignCameraMgr(m_iCCDNum)->GetParam_CameraExposureTime_ms();
	//	if (m_pAuroraMgr->GetAlignCameraMgr(m_iCCDNum)->GetSnapImage(nExposureTime, ucImg) == true)
	//	{
	//		imaqArrayToImage(m_pAqcImage, (BYTE*)ucImg, m_iGrabSizeW, m_iGrabSizeH);

	//		//	imaqClearOverlay(m_pLiveImage, NULL);
	//		//	imaqDuplicate(m_pLiveImage, m_pAqcImage);
	//		m_nLiveTick = GetTickCount() / 100;

	//		::PostMessage(this->m_hWnd, WM_MSG_DISPLAY_IMAGE, (WPARAM)0, (LPARAM)0);
	//	}
	//	else
	//	{
	//		m_nGrabNgCount++;

	//		if (m_nGrabNgCount > 2)
	//		{
	//			m_nGrabNgCount = 0;

	//			m_pAuroraMgr->GetAlignCameraMgr(m_iCCDNum)->ResetGrabber();
	//		}
	//	}
	//}

	//delete[] ucImg;
}


void CDlgAlignCCD::ShowCrossLine(Image* pImage)
{
	int iW = 0, iH = 0;
	imaqGetImageSize(pImage, &iW, &iH);

	m_iFocus = iW / 640;

	float iPer = (float)iH / (float)iW;

	Point hstart, hend, vstart, vend;
	hstart.x = iW * 0.1;    hstart.y = iH / 2;
	hend.x = iW - (iW * 0.1); hend.y = iH / 2;
	imaqOverlayLine(pImage, hstart, hend, &IMAQ_RGB_GREEN, NULL);

	int ixnum, iynum;
	int xOffset, yOffset, a;
	ixnum = 10;
	iynum = 10 * iPer;
	a = 7;
	xOffset = (hend.x - hstart.x) / ixnum;

	hstart.y -= a;
	hend.y += a;

	int i = 0;
	hstart.x = (iW / 2);
	for (i = 0; i<ixnum / 2; i++)
	{
		if (i + 1 == ixnum / 2) break;
		/*hstart.x += (xOffset);
		hend.x = hstart.x;*/
		hstart.x += xOffset;
		hend.x = hstart.x;

		imaqOverlayLine(pImage, hstart, hend, &IMAQ_RGB_GREEN, NULL);
	}

	hstart.x = (iW / 2);
	for (i = 0; i<ixnum / 2; i++)
	{
		if (i + 1 == ixnum / 2) break;
		/*hstart.x += (xOffset);
		hend.x = hstart.x;*/
		hstart.x -= xOffset;
		hend.x = hstart.x;

		imaqOverlayLine(pImage, hstart, hend, &IMAQ_RGB_GREEN, NULL);
	}

	////////////////////		
	vstart.x = iW / 2; vstart.y = iH * 0.1;
	vend.x = iW / 2; vend.y = iH - (iH * 0.1);
	imaqOverlayLine(pImage, vstart, vend, &IMAQ_RGB_GREEN, NULL);

	//	yOffset = (vend.y - vstart.y)/iynum;

	vstart.x -= a;
	vend.x += a;

	vstart.y = (iH / 2);
	for (i = 0; i<iynum / 2; i++)
	{
		vstart.y += xOffset;
		vend.y = vstart.y;
		imaqOverlayLine(pImage, vstart, vend, &IMAQ_RGB_GREEN, NULL);
	}

	vstart.y = (iH / 2);
	for (i = 0; i<iynum / 2; i++)
	{
		vstart.y -= xOffset;
		vend.y = vstart.y;
		imaqOverlayLine(pImage, vstart, vend, &IMAQ_RGB_GREEN, NULL);
	}

	bool bShowROI = false;
	Rect rectROI;
	m_pAuroraMgr->GetAlignCameraMgr(m_iCCDNum)->GetShowROI(&bShowROI, &rectROI);

	if (bShowROI == true)
	{
		imaqOverlayRect(pImage, rectROI, &IMAQ_RGB_RED, IMAQ_DRAW_VALUE, NULL);
	}
}


void CDlgAlignCCD::OverlayText(Image* pImage)
{
	Point ptMarkT;
	int fSize = 20;
	fSize = 20 * m_iFocus;		// 30

	ptMarkT.x = 1;
	ptMarkT.y = fSize;

	RGBValue rgbText = IMAQ_RGB_GREEN;
	OverlayTextOptions textoption;
	CString str = _T("111111");
	textoption.fontName = "Consolas";
	textoption.fontSize = fSize/*(int)((double)40)*/;
	textoption.bold = 0;
	textoption.italic = 0;
	textoption.underline = 0;
	textoption.strikeout = 0;
	textoption.horizontalTextAlignment = IMAQ_LEFT;
	textoption.verticalTextAlignment = IMAQ_BOTTOM;
	textoption.backgroundColor = IMAQ_RGB_TRANSPARENT;
	textoption.angle = 0;

	double dFocusVal = 0, dBright = 0;
	str.Format(_T("Live(%s):%d\nE:%d\nG:%d"), m_pAuroraMgr->GetAlignCameraMgr(m_iCCDNum)->GetDeviceName(), GetTickCount() / 100, m_pAuroraMgr->GetAlignCameraMgr(m_iCCDNum)->GetParam_CameraExposureTime_ms(), m_pAuroraMgr->GetAlignCameraMgr(m_iCCDNum)->GetParam_CameraGain());
//	str.Format(_T("Live(%s):%d"), m_pAuroraMgr->GetAlignCameraMgr(m_iCCDNum)->GetDeviceName(), GetTickCount() / 100);
	imaqOverlayText(pImage, ptMarkT, (char *)CT2A(LPCTSTR(str)), &rgbText, &textoption, NULL);
}

LRESULT CDlgAlignCCD::OnMsgDisplayImage(WPARAM wParam, LPARAM lParam)
{
	if (m_bCCDLive == true)
	{
		DisplayImage();

		DisplayImage2();
	}

	return 1;
}

void CDlgAlignCCD::ShowImage2(bool bShow)
{
	if( bShow == true)
	{
		m_pCCDReview->ShowWindow(SW_SHOW);

//		imaqShowWindow(m_iMoniImageWnd2, TRUE);
	}
	else
	{
		m_pCCDReview->ShowWindow(SW_HIDE);

		m_bShowImage2 = false;
//		imaqShowWindow(m_iMoniImageWnd2, FALSE);
	}
}

void CDlgAlignCCD::CBEventROIWnd(WindowEventType eventType, int windowNumber, Tool tool, Rect rect)
{ 
	windowNumber; 

	//if (IMAQ_RECTANGLE_TOOL == tool)
	//{
	//	switch (eventType)
	//	{
	//	case IMAQ_NO_EVENT:				// 0 No event occurred since the last call to imaqGetLastEvent().
	//	case IMAQ_MOVE_EVENT:			// 3 moved a window.
	//	case IMAQ_SIZE_EVENT:			// 4 sized a window.
	//	case IMAQ_SCROLL_EVENT:			// 5 scrolled a window.
	//	case IMAQ_ACTIVATE_EVENT:		// 6 activated a window.
	//	case IMAQ_CLOSE_EVENT:			// 7 closed a window.
	//	case IMAQ_DOUBLE_CLICK_EVENT:	// 8 double-clicked in a window.
	//		break;

	//	case IMAQ_CLICK_EVENT:			// 1 clicked on a window.
	//	{
	//		CRect rectROI(rect.left, rect.top, rect.left + rect.width, rect.top + rect.height);
	//	//	g_imaqImgCtrl->m_rectROI = rectROI;
	//	//::PostMessage(g_imaqImgCtrl->m_pParent->GetSafeHwnd(), UWM_ROIWND_EVENT_CLICK, (WPARAM)&rectROI, 0);


	//	}
	//	break;

	//	case IMAQ_DRAW_EVENT:			// 2 drew an ROI in a window.
	//	{
	//		CRect rectROI(rect.left, rect.top, rect.left + rect.width, rect.top + rect.height);
	//		g_imaqImgCtrl->m_rectROI = rectROI;

	//		if (g_imaqImgCtrl->m_bSetSend)
	//		{
	//			::SendMessage(g_imaqImgCtrl->m_pParent->GetSafeHwnd(), UWM_ROIWND_EVENT_DRAW, (WPARAM)&rectROI, 0);
	//			TRACE("IMAQ_DRAW_EVENT\n");
	//		}
	//	}
	//	break;
	//	}
	//}
	//else if (IMAQ_LINE_TOOL == tool)
	//{
	//	if (IMAQ_DRAW_EVENT == eventType)
	//	{
	//		CRect rectROI(rect.left, rect.top, rect.left + rect.width, rect.top + rect.height);
	//		::SendMessage(g_imaqImgCtrl->m_pParent->GetSafeHwnd(), UWM_ROIWND_EVENT_CLICK, (WPARAM)&rectROI, 0);
	//	}
	//}

	if (IMAQ_ANNULUS_TOOL == eventType)
	{
		g_bShowImage = false;
	//	imaqShowWindow(windowNumber, FALSE);

	//	if (windowNumber == m_iMoniImageWnd2)
	//	{
	//		ShowImage2(false);
	//	}
	}

//	TRACE(">> ROI_CB : %x Tool[%d] event[%d]\n", g_imaqImgCtrl->m_pParent, tool, eventType);
	TRACE(">> ROI_CB : WINDOW[%d] Tool[%d] event[%d]\n", windowNumber, tool, eventType);

	return;
}

int CDlgAlignCCD::SetCCDLive(bool bLive)
{
	// TODO: 여기에 구현 코드 추가.
	m_bCCDLive = bLive;

	return 0;
}
