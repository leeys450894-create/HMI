// IMAQImageCtrl.cpp : implementation file
//

#include "pch.h"
#include "IMAQImageCtrl.h"
#include "math.h"

#include "nimachinevision.h"
#include "FineCentering.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CIMAQImageCtrl
static bool g_bMouseState = 0; // check mouse click
static CIMAQImageCtrl *gIMAQImageCtrl = NULL;
//########################################################################
//#
//# 마우스 후킹하기..
//#
//########################################################################
LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if ((gIMAQImageCtrl == NULL) || ((gIMAQImageCtrl->m_hWndUser == NULL) && (gIMAQImageCtrl->m_hWndParent == NULL)))
        return 0;
    if (nCode < 0)
        return 0;

    HWND hWndHookTarget = gIMAQImageCtrl->GetWndHandle();
    HWND hWndMsgReceiver = (gIMAQImageCtrl->m_hWndUser != NULL) ? gIMAQImageCtrl->m_hWndUser : gIMAQImageCtrl->m_hWndParent;

    if (!(lParam & 0x80000000))
    {
        if (hWndHookTarget != NULL)
        {
            if (hWndMsgReceiver != NULL)
            {
                //::PostMessage(hWndMsgReceiver, UWM_IMAQIMAGE_MOUSE_EVENT, EVENT_KEYBOARD, MAKELPARAM(wParam, -1));
                if (gIMAQImageCtrl->m_bBreakMsg) return 1;
            }

        }
    }

    return CallNextHookEx(gIMAQImageCtrl->m_hKeyHook, nCode, wParam, lParam);
}

//########################################################################
//#
//# 키보드 후킹하기...
//#
//########################################################################
LRESULT CALLBACK MouseProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    MOUSEHOOKSTRUCTEX *pInfo = (MOUSEHOOKSTRUCTEX *)lParam;

    if ((gIMAQImageCtrl == NULL) || ((gIMAQImageCtrl->m_hWndUser == NULL) && (gIMAQImageCtrl->m_hWndParent == NULL)))
        return 0;
    if ((nCode < 0) || (nCode == HC_NOREMOVE) )
        return 0;

    HWND hWndHookTarget = gIMAQImageCtrl->GetWndHandle();
    HWND hWndMsgReceiver = (gIMAQImageCtrl->m_hWndUser != NULL) ? gIMAQImageCtrl->m_hWndUser : gIMAQImageCtrl->m_hWndParent;
    HWND hWndZoomMsgReceiver = gIMAQImageCtrl->m_hWndParent;

    switch (wParam)
    {
        //case WM_MOUSEMOVE:
        //    if (hWndHookTarget != NULL && pInfo->hwnd == hWndHookTarget && hWndZoomMsgReceiver != NULL)
        //    {
        //        CPoint p = pInfo->pt;
        //        //gIMAQImageCtrl->ScreenToClient(&p);
        //        //::PostMessage(hWndMsgReceiver, UWM_IMAQIMAGE_MOUSE_EVENT, EVENT_MOUSE_MOVE, MAKELPARAM(p.x, p.y));
        //        //p = pInfo->pt;
        //        if (g_bMouseState)  // 클릭하고 있을 때
        //        {
        //            ::PostMessage(hWndZoomMsgReceiver, UWM_IMAQIMAGE_MOUSE_EVENT, eIMAQCTRL_MOUSE_HANDLE_LDOWN_MOVE, MAKELPARAM(p.x, p.y));
        //        }
        //        else  // 클릭 안할 때
        //        {
        //            ::PostMessage(hWndZoomMsgReceiver, UWM_IMAQIMAGE_MOUSE_EVENT, eIMAQCTRL_MOUSE_HANDLE_MOVE, MAKELPARAM(p.x, p.y));
        //        }
        //
        //        if (gIMAQImageCtrl->m_bBreakMsg) return 1;
        //    }
        //    break;
        //
        case WM_RBUTTONUP:
            if (hWndZoomMsgReceiver != NULL)
            {
                CPoint p = pInfo->pt;
                ::PostMessage(hWndZoomMsgReceiver, UWM_IMAQIMAGE_MOUSE_EVENT, eIMAQCTRL_MOUSE_HANDLE_RUP, MAKELPARAM(p.x, p.y));
                if (gIMAQImageCtrl->m_bBreakMsg) return 1;
            }
            break;

        //case WM_RBUTTONDOWN:
        //    if (hWndZoomMsgReceiver != NULL)
        //    {
        //        CPoint p = pInfo->pt;
        //        ::PostMessage(hWndZoomMsgReceiver, UWM_IMAQIMAGE_MOUSE_EVENT, eIMAQCTRL_MOUSE_HANDLE_RDOWN, MAKELPARAM(p.x, p.y));
        //        if (gIMAQImageCtrl->m_bBreakMsg) return 1;
        //    }
        //    break;
        //
        //case WM_LBUTTONUP:
        //    g_bMouseState = false;// 클릭 체크
        //    if (hWndZoomMsgReceiver != NULL)
        //    {
        //        CPoint p = pInfo->pt;
        //        ::PostMessage(hWndZoomMsgReceiver, UWM_IMAQIMAGE_MOUSE_EVENT, eIMAQCTRL_MOUSE_HANDLE_LUP, MAKELPARAM(p.x, p.y));
        //        if (gIMAQImageCtrl->m_bBreakMsg) return 1;
        //    }
        //    break;
        //
        //case WM_LBUTTONDOWN:
        //    g_bMouseState = true; // 클릭 체크
        //    if (hWndZoomMsgReceiver != NULL)
        //    {
        //        CPoint p = pInfo->pt;
        //        ::PostMessage(hWndZoomMsgReceiver, UWM_IMAQIMAGE_MOUSE_EVENT, eIMAQCTRL_MOUSE_HANDLE_LDOWN, MAKELPARAM(p.x, p.y));
        //        if (gIMAQImageCtrl->m_bBreakMsg) return 1;
        //    }
        //    break;
        //
        //case WM_LBUTTONDBLCLK:
        //    g_bMouseState = true;
        //    if (hWndZoomMsgReceiver != NULL)
        //    {
        //        CPoint p = pInfo->pt;
        //        ::PostMessage(hWndZoomMsgReceiver, UWM_IMAQIMAGE_MOUSE_EVENT, eIMAQCTRL_MOUSE_HANDLE_LDCLICK, MAKELPARAM(p.x, p.y));
        //        if (gIMAQImageCtrl->m_bBreakMsg) return 1;
        //    }
        //    break;

        case WM_MOUSEWHEEL:
            if ((int)(pInfo->mouseData) > 0)
            {
				 CPoint p = pInfo->pt;
                // 휠 업
                ::PostMessage(hWndZoomMsgReceiver, UWM_IMAQIMAGE_MOUSE_EVENT, eIMAQCTRL_MOUSE_HANDLE_WHEEL_UP, MAKELPARAM(p.x, p.y));
            }
            else if ((int)(pInfo->mouseData) < 0)
            {
				 CPoint p = pInfo->pt;
                // 휠 다운
                ::PostMessage(hWndZoomMsgReceiver, UWM_IMAQIMAGE_MOUSE_EVENT, eIMAQCTRL_MOUSE_HANDLE_WHEEL_DOWN, MAKELPARAM(p.x, p.y));
            }
            break;
    }

    return CallNextHookEx(gIMAQImageCtrl->m_hMouseHook, nCode, wParam, lParam);
}

CIMAQImageCtrl::CIMAQImageCtrl()
{
	gIMAQImageCtrl = this;
	m_pParent = NULL;
	m_pimaqWnd = NULL;
	m_pBaseImage = NULL;
	m_pROI = NULL;
	m_bHSLType = FALSE;
	m_bSetSend = TRUE;
	m_pOverlayImage = NULL;
	m_pPtrnImgBW = NULL;
	m_pHostWnd = NULL;
    m_iImaqWindowNumber = -1;
    m_hImaqWindowHandle = NULL;
    m_bImaqWindowCreate = FALSE;
    m_hWndUser			= NULL;
    m_hWndParent		= NULL;
    m_pImage_onlylink	= NULL;
    m_pImgDefault		= NULL;
    for (int i = 0; i < eNI_OVERLAY_LAYER_END; i++)
    {
        m_pOverlay[i] = NULL;
    }
    m_ePreviousTool		= IMAQ_NO_TOOL;
    m_bDrawRect			= FALSE;

    // keyboard & mouse hook...
    m_bBreakMsg		= true;
    m_hKeyHook		= NULL;
    m_hMouseHook	= NULL;
}

CIMAQImageCtrl::~CIMAQImageCtrl()
{
    HookEnd();

    if (gIMAQImageCtrl != NULL)
    {
        EnableEvent(FALSE);
        SetCurrentTool(IMAQ_NO_TOOL);
    }

    if (m_bImaqWindowCreate)
    {
        imaqCloseWindow(m_iImaqWindowNumber);
        m_bImaqWindowCreate = FALSE;
        m_iImaqWindowNumber = -1;
    }

    if (m_pImgDefault != NULL)
    {
        imaqDispose(m_pImgDefault);
        m_pImgDefault = NULL;
    }
    for (int i = 0; i < eNI_OVERLAY_LAYER_END; i++)
    {
        if (m_pOverlay[i] != NULL)
        {
            imaqDispose(m_pOverlay[i]);
            m_pOverlay[i] = NULL;
        }
    }
}

BEGIN_MESSAGE_MAP(CIMAQImageCtrl, CStatic)
    //{{AFX_MSG_MAP(CIMAQImageCtrl)
    ON_WM_PAINT()
    //}}AFX_MSG_MAP
    ON_WM_CLOSE()
    ON_WM_DESTROY()
    //    ON_WM_MOUSEMOVE()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CIMAQImageCtrl HOOK message handlers

BOOL CIMAQImageCtrl::HookStart()
{
    gIMAQImageCtrl = this;
    m_bBreakMsg = false;

    if (m_hKeyHook == NULL)
        m_hKeyHook = SetWindowsHookEx(WH_KEYBOARD, &KeyboardProc, GetModuleHandle(NULL), GetCurrentThreadId());

    if (m_hMouseHook == NULL)
        m_hMouseHook = SetWindowsHookEx(WH_MOUSE, &MouseProc, GetModuleHandle(NULL), GetCurrentThreadId());

    if (m_hKeyHook == NULL || m_hMouseHook == NULL)
        return FALSE;

    return TRUE;
}

BOOL CIMAQImageCtrl::HookEnd()
{
    if (m_hKeyHook != NULL)
    {
        UnhookWindowsHookEx(m_hKeyHook);
        m_hKeyHook = NULL;
    }

    if (m_hMouseHook != NULL)
    {
        UnhookWindowsHookEx(m_hMouseHook);
        m_hMouseHook = NULL;
    }

    return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CIMAQImageCtrl message handlers

void CIMAQImageCtrl::OnPaint()
{
    CPaintDC dc(this); // device context for painting
    DisplayImage();
}

void CIMAQImageCtrl::OnClose()
{
    HookEnd();

    if (gIMAQImageCtrl != NULL)
    {
        EnableEvent(FALSE);
        SetCurrentTool(IMAQ_NO_TOOL);
    }

    if (m_bImaqWindowCreate)
    {
        imaqCloseWindow(m_iImaqWindowNumber);
        m_bImaqWindowCreate = FALSE;
        m_iImaqWindowNumber = -1;
    }

    if (m_pImgDefault != NULL)
    {
        imaqDispose(m_pImgDefault);
        m_pImgDefault = NULL;
    }
    for (int i = 0; i < eNI_OVERLAY_LAYER_END; i++)
    {
        if (m_pOverlay[i] != NULL)
        {
            imaqDispose(m_pOverlay[i]);
            m_pOverlay[i] = NULL;
        }
    }

    CStatic::OnClose();
}

void CIMAQImageCtrl::DestroyMe()
{
	imaqSetCurrentTool(IMAQ_NO_TOOL);
	imaqSetEventCallback(NULL, FALSE);
	imaqCloseToolWindow();

	imaqDispose(m_pOverlayImage);
	imaqDispose(m_pBaseImage);
	imaqDispose(m_pROI);

	if (m_pimaqWnd)		imaqCloseWindow(m_pimaqWnd->GetIWndImg());
	//	SAFE_DELETE(m_pimaqWnd);	// jsyoon 20120720 change
	if (m_pimaqWnd)
	{
		delete m_pimaqWnd;
		m_pimaqWnd = NULL;
	}
}

void CIMAQImageCtrl::OnDestroy()
{
    CStatic::OnDestroy();

    // TODO: 여기에 메시지 처리기 코드를 추가합니다.
    HookEnd();

    if (gIMAQImageCtrl != NULL)
    {
        EnableEvent(FALSE);
        SetCurrentTool(IMAQ_NO_TOOL);

    }

    if (m_bImaqWindowCreate)
    {
        imaqCloseWindow(m_iImaqWindowNumber);
        m_bImaqWindowCreate = FALSE;
        m_iImaqWindowNumber = -1;
    }

    if (m_pImgDefault != NULL)
    {
        imaqDispose(m_pImgDefault);
        m_pImgDefault = NULL;
    }
    for (int i = 0; i < eNI_OVERLAY_LAYER_END; i++)
    {
        if (m_pOverlay[i] != NULL)
        {
            imaqDispose(m_pOverlay[i]);
            m_pOverlay[i] = NULL;
        }
    }
}

//########################################################################
//#
//# static에서 발생한 메시지 전달
//#      (static을 포함한 dlg가 활성화되었을 때 키보드 메시지를를 받기위해서..
//#
//########################################################################
BOOL CIMAQImageCtrl::PreTranslateMessage(MSG *pMsg)
{
    if (pMsg->message == WM_KEYDOWN)
    {
        if ((gIMAQImageCtrl == NULL) || ((gIMAQImageCtrl->m_hWndUser == NULL) && (gIMAQImageCtrl->m_hWndParent == NULL)))
            return CStatic::PreTranslateMessage(pMsg);

        HWND hWnd = (gIMAQImageCtrl->m_hWndUser != NULL) ? gIMAQImageCtrl->m_hWndUser : gIMAQImageCtrl->m_hWndParent;

        int  modifiers = -1;

        if ((GetKeyState(VK_LMENU) & 0x8000) != 0 || (GetKeyState(VK_RMENU) & 0x8000) != 0)
        {
            modifiers = IMAQ_ALT;
        }
        else if ((GetKeyState(VK_LCONTROL) & 0x8000) != 0 || (GetKeyState(VK_RCONTROL) & 0x8000) != 0)
        {
            modifiers = IMAQ_CTRL;
        }
        else if ((GetKeyState(VK_LSHIFT) & 0x8000) != 0 || (GetKeyState(VK_RSHIFT) & 0x8000) != 0)
        {
            modifiers = IMAQ_SHIFT;
        }

        //::PostMessage(hWnd, UWM_IMAQIMAGE_MOUSE_EVENT, EVENT_KEYBOARD, MAKELPARAM(keyPressed, modifiers));
    }

    return CStatic::PreTranslateMessage(pMsg);
}

void CIMAQImageCtrl::PreSubclassWindow()
{
    m_bImaqWindowCreate = IMAQWindowCreate();

    CStatic::PreSubclassWindow();
}

void CIMAQImageCtrl::MakePopupWnd(CWnd* pParent, UINT uiID)
{
	if (!m_pimaqWnd)
	{
		m_pimaqWnd = new CIMAQImage();
		m_pimaqWnd->Prepare((CWnd*)pParent, FALSE);
	}

	CRect uiRect;
	(m_pParent->GetDlgItem(uiID))->GetWindowRect(&uiRect);
	m_pParent->ScreenToClient(&uiRect);

	Point pt = { uiRect.left, uiRect.top };
	imaqMoveWindow(m_pimaqWnd->GetIWndImg(), pt);

	imaqSetWindowSize(m_pimaqWnd->GetIWndImg(), uiRect.right, uiRect.bottom); 
}

void CIMAQImageCtrl::MakeROITool(BOOL bShowToolWnd)
{
	if (!m_pROI)
		m_pROI = imaqCreateROI();

	if (bShowToolWnd)
	{
		ToolWindowOptions twOptions;
		memset(&twOptions, 0, sizeof(ToolWindowOptions));
		twOptions.showRectangleTool = TRUE;

		imaqSetupToolWindow(TRUE, 4, &twOptions);
		imaqSetCurrentTool(IMAQ_NO_TOOL);
		imaqSetToolContextSensitivity(FALSE);	// When the mouse cursor is near contour, Selection Tool is activated.

		imaqShowToolWindow(TRUE);
	}
}

// 실제 툴에 따라서 셋팅해준건 요넘..
void CIMAQImageCtrl::SetROIToolOption(Tool useTool)
{
	if (useTool == IMAQ_RECTANGLE_TOOL)
	{
		imaqSetCurrentTool(IMAQ_RECTANGLE_TOOL);
		imaqSetToolContextSensitivity(TRUE);	// When the mouse cursor is near contour, Selection Tool is activated.

		if (!m_pROI || !m_pimaqWnd) return;
		imaqAddRectContour(m_pROI, imaqMakeRect(m_rectROI.top,
			m_rectROI.left,
			m_rectROI.Height(),
			m_rectROI.Width()));

		imaqSetWindowROI(m_pimaqWnd->GetIWndImg(), m_pROI);
	}
}

void CIMAQImageCtrl::CBEventROIWnd(WindowEventType eventType, int windowNumber, Tool tool, Rect rect)
{
	windowNumber;

	if (IMAQ_RECTANGLE_TOOL == tool)
	{
		switch (eventType)
		{
		case IMAQ_NO_EVENT:				// 0 No event occurred since the last call to imaqGetLastEvent().
		case IMAQ_MOVE_EVENT:			// 3 moved a window.
		case IMAQ_SIZE_EVENT:			// 4 sized a window.
		case IMAQ_SCROLL_EVENT:			// 5 scrolled a window.
		case IMAQ_ACTIVATE_EVENT:		// 6 activated a window.
		case IMAQ_CLOSE_EVENT:			// 7 closed a window.
		case IMAQ_DOUBLE_CLICK_EVENT:	// 8 double-clicked in a window.
			break;

		case IMAQ_CLICK_EVENT:			// 1 clicked on a window.
		{
			CRect rectROI(rect.left, rect.top, rect.left + rect.width, rect.top + rect.height);
			gIMAQImageCtrl->m_rectROI = rectROI;
			::PostMessage(gIMAQImageCtrl->m_pParent->GetSafeHwnd(), UWM_ROIWND_EVENT_CLICK, (WPARAM)&rectROI, 0);
		}
		break;

		case IMAQ_DRAW_EVENT:			// 2 drew an ROI in a window.
		{
			CRect rectROI(rect.left, rect.top, rect.left + rect.width, rect.top + rect.height);
			gIMAQImageCtrl->m_rectROI = rectROI;

			if (gIMAQImageCtrl->m_bSetSend)
			{
				::SendMessage(gIMAQImageCtrl->m_pParent->GetSafeHwnd(), UWM_ROIWND_EVENT_DRAW, (WPARAM)&rectROI, 0);
				TRACE("IMAQ_DRAW_EVENT\n");
			}
		}
		break;
		}
	}
	else if (IMAQ_LINE_TOOL == tool)
	{
		if (IMAQ_DRAW_EVENT == eventType)
		{
			CRect rectROI(rect.left, rect.top, rect.left + rect.width, rect.top + rect.height);
			::SendMessage(gIMAQImageCtrl->m_pParent->GetSafeHwnd(), UWM_ROIWND_EVENT_CLICK, (WPARAM)&rectROI, 0);
		}
	}

	TRACE(">> ROI_CB : %x Tool[%d] event[%d]\n", gIMAQImageCtrl->m_pParent, tool, eventType);

	return;
}

BOOL CIMAQImageCtrl::IMAQWindowCreate()
{
    if (m_bImaqWindowCreate)
        return TRUE;

    if (!imaqGetWindowHandle(&m_iImaqWindowNumber))
    {
        TRACE("[IMAQ CTRL] can't get imaq windownumber!\n");
        return FALSE;
    }

    m_hImaqWindowHandle = (HWND)imaqGetSystemWindowHandle(m_iImaqWindowNumber);
    if (m_hImaqWindowHandle == NULL)
    {
        return FALSE;
    }

    m_wSub.FromHandle(m_hImaqWindowHandle);

    ::SetParent(m_hImaqWindowHandle, this->GetSafeHwnd());
    CRect r; GetClientRect(&r);
    this->SetWindowPos(&wndTop, 0, 0, r.Width(), r.Height(), SWP_NOMOVE);
    ::MoveWindow(m_hImaqWindowHandle, 0, 0, r.Width(), r.Height(), FALSE);
    imaqSetupWindow(m_iImaqWindowNumber, IMAQ_WIND_TOPMOST);//|IMAQ_WIND_RESIZABLE);
    imaqSetWindowSize(m_iImaqWindowNumber, r.Width(), r.Height());
    imaqShowWindow(m_iImaqWindowNumber, TRUE);


#ifdef _USE_10BIT_T
    m_pImgDefault = imaqCreateImage(IMAQ_IMAGE_I16, 3);
#else
    m_pImgDefault = imaqCreateImage(IMAQ_IMAGE_U8, 4);
#endif


    //CiWLScanningInterferometerApp *pApp = (CiWLScanningInterferometerApp *)AfxGetApp();
    int iW = 1024;//pApp->m_Config.m_Data[WLSI_CONF_IMG_W].iVal;
    int iH = 1024;//pApp->m_Config.m_Data[WLSI_CONF_IMG_H].iVal;
    size_t nPixel = iW * iH;
    BYTE *ucImg = new BYTE[nPixel];
    memset(ucImg, 0, sizeof(BYTE)*nPixel);
    imaqArrayToImage(m_pImgDefault, ucImg, iW, iH);
    SetImage(m_pImgDefault);
    DisplayImage();
    delete[] ucImg;


    ToolWindowOptions twOptions;
    memset(&twOptions, 0, sizeof(ToolWindowOptions));
    twOptions.showSelectionTool = TRUE;
    twOptions.showPointTool = TRUE;
    twOptions.showRectangleTool = TRUE;
    twOptions.showLineTool = TRUE;
    twOptions.showAnnulusTool = TRUE;
    twOptions.showOvalTool = TRUE;
    twOptions.showZoomTool = TRUE;
    twOptions.showRotatedRectangleTool = TRUE;
    twOptions.showPolyLineTool = TRUE;

    imaqSetupToolWindow(TRUE, 4, &twOptions);
    Point imagePos;
    int imageWidth;
    imaqGetWindowPos(m_iImaqWindowNumber, &imagePos);
    imaqGetWindowSize(m_iImaqWindowNumber, &imageWidth, NULL);
    imagePos.x += imageWidth + 1;
    imaqMoveToolWindow(imagePos);
    imaqShowToolWindow(FALSE);
    imaqSetToolContextSensitivity(TRUE);

    m_hWndParent = this->GetParent()->GetSafeHwnd();
    m_bImaqWindowCreate = TRUE;

    for (int i = 0; i < eNI_OVERLAY_LAYER_END; i++)
    {
#ifdef _USE_10BIT_T
        m_pOverlay[i] = imaqCreateImage(IMAQ_IMAGE_I16, 3);
#else
        m_pOverlay[i] = imaqCreateImage(IMAQ_IMAGE_U8, 4);
#endif
    }

    return TRUE;
}

int CIMAQImageCtrl::SetWindowSize(int cx, int cy)
{
    if (!m_bImaqWindowCreate) return 0;

    this->SetWindowPos(&wndTop, 0, 0, cx, cy, SWP_NOMOVE);

    imaqSetWindowSize(m_iImaqWindowNumber, cx, cy);

    return 1;
}

void CIMAQImageCtrl::SetCurrentTool(Tool currTool)
{
    imaqSetCurrentTool(currTool);
}

void CIMAQImageCtrl::SetUserWindow(CWnd *pWnd)
{
	if(pWnd == NULL)
	{
		m_hWndUser = NULL;
	}
	else
	{
		m_hWndUser = pWnd->GetSafeHwnd();
	}    
}

void CIMAQImageCtrl::ShowScrollBar(BOOL bShow)
{
    imaqShowScrollbars(m_iImaqWindowNumber, bShow);
}

void CIMAQImageCtrl::EnableEvent(BOOL bflag)
{
    if (bflag)
    {
        imaqSetEventCallback(OnEventCallbackIMAQCtrl, FALSE);
    }
    else
    {
        imaqSetEventCallback(NULL, FALSE);
    }
}


//########################################################################
//#
//# imaq event처리하기..
//#
//########################################################################
void CIMAQImageCtrl::OnEventCallbackIMAQCtrl(WindowEventType iType, int windowNumber, Tool tool, Rect rect)
{
    if ((gIMAQImageCtrl == NULL) || ((gIMAQImageCtrl->m_hWndUser == NULL) && (gIMAQImageCtrl->m_hWndParent == NULL)))
        return;

    HWND hWnd = (gIMAQImageCtrl->m_hWndUser != NULL) ? gIMAQImageCtrl->m_hWndUser : gIMAQImageCtrl->m_hWndParent;

    switch (iType)
    {
        case IMAQ_NO_EVENT:
            break;

        case IMAQ_CLICK_EVENT:
            ::SendMessage(hWnd, UWM_IMAQIMAGE_MOUSE_EVENT, eIMAQCTRL_MOUSE_HANDLE_LCLICK, MAKELPARAM(rect.left, rect.top));
            break;

        case IMAQ_MOVE_EVENT:
            ::SendMessage(hWnd, UWM_IMAQIMAGE_MOUSE_EVENT, eIMAQCTRL_MOUSE_HANDLE_MOVE, MAKELPARAM(rect.left, rect.top));
            break;

        case IMAQ_SIZE_EVENT: break;
        case IMAQ_SCROLL_EVENT: break;
        case IMAQ_ACTIVATE_EVENT: break;
        case IMAQ_CLOSE_EVENT: break;
        case IMAQ_DRAW_EVENT:
        {
            if (rect.height == 0 && rect.width == 0)
                break;

            ::SendMessage(hWnd, UWM_IMAQIMAGE_MOUSE_EVENT, eIMAQCTRL_MOUSE_HANDLE_LDOWN_DRAW, (LPARAM)&rect);
        }
        break;

        case IMAQ_DOUBLE_CLICK_EVENT:
            ::SendMessage(hWnd, UWM_IMAQIMAGE_MOUSE_EVENT, eIMAQCTRL_MOUSE_HANDLE_LDCLICK, MAKELPARAM(rect.left, rect.top));
            break;

        default:
            break;
    }
}



//########################################################################
//#
//# imaq 설정하기..
//#
//########################################################################
void CIMAQImageCtrl::SetZoom(eIMAQ_DISPLAY_ZOOM eZoom)
{
    switch (eZoom)
    {
        case eIMAQ_DISPLAY_ZOOM_NONE:
        case eIMAQ_DISPLAY_ZOOM_1X:
            imaqZoomWindow(m_iImaqWindowNumber, 1, 1, IMAQ_NO_POINT);
            break;

        case eIMAQ_DISPLAY_ZOOM_HALF:
            imaqZoomWindow(m_iImaqWindowNumber, -2, -2, IMAQ_NO_POINT);
            break;

        case eIMAQ_DISPLAY_ZOOM_QUARTER:
            imaqZoomWindow(m_iImaqWindowNumber, -4, -4, IMAQ_NO_POINT);
            break;
    }
}

void CIMAQImageCtrl::SetZoom(float fAxisRatio, int iCntX/*=-1*/, int iCntY/*=-1*/)
{
    if (iCntX == -1 || iCntY == -1)
    {
        imaqZoomWindow2(m_iImaqWindowNumber, fAxisRatio, fAxisRatio, IMAQ_NO_POINT);
    }
    else
    {
        Point pCenter;
        pCenter.x = iCntX;
        pCenter.y = iCntY;
        imaqZoomWindow2(m_iImaqWindowNumber, fAxisRatio, fAxisRatio, pCenter);
    }
}

void CIMAQImageCtrl::SetRepaint()// 그리기만 하기
{
    imaqDisplayImage(m_pImage_onlylink, m_iImaqWindowNumber, FALSE);
}

void CIMAQImageCtrl::SetImage(Image *pImage)
{
    m_pImage_onlylink = pImage;
}

void CIMAQImageCtrl::SetImage(int index, Image *pImage)
{
	if( (0 <= index) && (index < eNI_OVERLAY_LAYER_END) )
	{
		imaqCopyOverlay(m_pOverlay[index], pImage, NULL);
	}
}

void CIMAQImageCtrl::DisplayImage()
{
    if (!m_bImaqWindowCreate) return;
    if (m_pImage_onlylink == NULL) return;

    imaqClearOverlay(m_pImage_onlylink, NULL);

    for (int i = 0; i < eNI_OVERLAY_LAYER_END; i++)
    {
        imaqCopyOverlay(m_pImage_onlylink, m_pOverlay[i], NULL);
    }

    imaqDisplayImage(m_pImage_onlylink, m_iImaqWindowNumber, FALSE);
}

void CIMAQImageCtrl::SaveImage(CString sFileName, BOOL bIncludeOverlay)
{
    ImageType type;
    imaqGetImageType(m_pImage_onlylink, &type);

    if (bIncludeOverlay == TRUE)
    {
        Image *pMerge = imaqCreateImage(IMAQ_IMAGE_RGB, 3);

        if (type != IMAQ_IMAGE_U8)
        {
            Image *pTemp = imaqCreateImage(IMAQ_IMAGE_U8, 4);

            CastToU8(pTemp, m_pImage_onlylink);
            imaqMergeOverlay(pMerge, pTemp, NULL, 256, NULL);
            
			imaqDispose(pTemp); 
			pTemp = NULL;
        }
        else
        {
            imaqMergeOverlay(pMerge, m_pImage_onlylink, NULL, 256, NULL);
        }

        USES_CONVERSION;
        imaqWriteJPEGFile(pMerge, T2A(sFileName), 750, NULL);
        
		imaqDispose(pMerge);
        pMerge = NULL;
    }
    else
    {
        if (type == IMAQ_IMAGE_U8)
        {
            USES_CONVERSION;
            imaqWriteJPEGFile(m_pImage_onlylink, T2A(sFileName), 750, NULL);
        }
        else if (type == IMAQ_IMAGE_I16)
        {
            USES_CONVERSION;
            imaqWritePNGFile2(m_pImage_onlylink, T2A(sFileName), 750, NULL, FALSE);
        }
        else
        {
            Image *pTemp = imaqCreateImage(IMAQ_IMAGE_U8, 4);
            CastToU8(pTemp, m_pImage_onlylink);

            USES_CONVERSION;
            imaqWriteJPEGFile(pTemp, T2A(sFileName), 750, NULL);
            
			imaqDispose(pTemp);
            pTemp = NULL;
        }
    }
}

void CIMAQImageCtrl::MakeScaledImage(Image* pOrig, Image* pScaled, CRect* pRect)
{
	//  Find out how big an area we have and how big the image is.
	int width = 0, height = 0;
	imaqGetImageSize(pOrig, &width, &height);

	// If image to display is smaller than the specified display area, just use a copy of it, 
	// Otherwise, scale it to fit the display area.
	if (width <= pRect->Width() && height <= pRect->Height())
	{
		imaqDuplicate(pScaled, pOrig);
	}
	else
	{
		double hScale = (double)pRect->Width() / width;
		double vScale = (double)pRect->Height() / height;
		double scale = (hScale < vScale) ? hScale : vScale;
		width = (int)(width  * scale);
		height = (int)(height * scale);

		imaqResample(pScaled, pOrig, width, height, IMAQ_BILINEAR, IMAQ_NO_RECT);
	}

	//  Now that we have a new height & width for the image, center it.
	pRect->OffsetRect((pRect->Width() - width) / 2, (pRect->Height() - height) / 2);
	pRect->right = pRect->left + width;
	pRect->bottom = pRect->top + height;
}

BOOL CIMAQImageCtrl::LoadPattern(CString strFileName, Image* pDest)
{
	if (strFileName.IsEmpty())
		return FALSE;

	USES_CONVERSION;
	if (!imaqReadFile(pDest, T2A(strFileName), NULL, NULL))
	{
		char* errStr = imaqGetErrorText(imaqGetLastError());
		TRACE("%s(%i) : %s\n", __FILE__, __LINE__, errStr);
		imaqDispose(errStr);
		return FALSE;
	}

	return TRUE;
}

void CIMAQImageCtrl::ImageToClipboard(BOOL bIncludeOverlay)
{
    ImageType type;
    imaqGetImageType(m_pImage_onlylink, &type);

    if (bIncludeOverlay == TRUE)
    {
        Image *pMerge = imaqCreateImage(IMAQ_IMAGE_RGB, 3);

        if (type != IMAQ_IMAGE_U8)
        {
            Image *pTemp = imaqCreateImage(IMAQ_IMAGE_U8, 4);

            CastToU8(pTemp, m_pImage_onlylink);
            imaqMergeOverlay(pMerge, pTemp, NULL, 256, NULL);
            
			imaqDispose(pTemp);
			pTemp = NULL;
        }
        else
        {
            imaqMergeOverlay(pMerge, m_pImage_onlylink, NULL, 256, NULL);
        }
		
        imaqImageToClipboard(pMerge, NULL);
        
		imaqDispose(pMerge);
		pMerge = NULL;
    }
    else
    {
        if (type == IMAQ_IMAGE_U8)
        {
            imaqImageToClipboard(m_pImage_onlylink, NULL);
        }
        else
        {
            Image *pTemp = imaqCreateImage(IMAQ_IMAGE_U8, 4);

            CastToU8(pTemp, m_pImage_onlylink);
            imaqImageToClipboard(pTemp, NULL);

            imaqDispose(pTemp);
			pTemp = NULL;
        }
    }
}

void CIMAQImageCtrl::SetStartPanning()
{
    if (m_bDrawRect == TRUE)
    {
        imaqGetCurrentTool(&m_ePreviousTool);
        m_bDrawRect = FALSE;
    }

    imaqSetCurrentTool(IMAQ_PAN_TOOL);
}

void CIMAQImageCtrl::SetStartZoom()
{
    if (m_bDrawRect == TRUE)
    {
        imaqGetCurrentTool(&m_ePreviousTool);
        m_bDrawRect = FALSE;
    }

    imaqSetCurrentTool(IMAQ_NO_TOOL);
}

void CIMAQImageCtrl::SetStartDrawRect()
{
    if (m_bDrawRect == FALSE)
    {
        m_bDrawRect = TRUE;
        imaqSetCurrentTool(m_ePreviousTool);
    }

	//imaqSetCurrentTool(IMAQ_RECTANGLE_TOOL);
}

void CIMAQImageCtrl::ZoomWindow(CPoint cpStart, CPoint cpEnd)
{
    CRect rect;
    GetWindowRect(&rect);

	imaqGetWindowCenterPos(m_iImaqWindowNumber, &m_stZoomParam.ptZoomCenter);	// 현재의 Zoom Center 값을 가져 옴

    CPoint ptTransStart = TransOriginWindowCoord(cpStart, m_stZoomParam);		// 입력된 Zoom 시작점을 원본 배율의 좌표값으로 변경
    CPoint ptTransEnd = TransOriginWindowCoord(cpEnd, m_stZoomParam);			// 입력된 Zoom 끝 점을 원본 배율의 좌표값으로 변경

	stIMAQCTRL_ZOOM_PARAM stZoom;
    stZoom.ptZoomCenter.x = (ptTransStart.x + ptTransEnd.x) / 2; // 입력된 Zoom의 Center 값을 계산
    stZoom.ptZoomCenter.y = (ptTransStart.y + ptTransEnd.y) / 2; // 입력된 Zoom의 Center 값을 계산

    float fZoomRateX;
    float fZoomRateY;
    if (ptTransStart.x == ptTransEnd.x)   // 입력된 Zoom의 X값변화가 없으면 Y를 기준으로 Zoom
    {
        fZoomRateX = 0;
    }
    else
    {
        fZoomRateX = (float)rect.Width() / ((float)ptTransStart.x - (float)ptTransEnd.x); // 입력된 Zoom Area의 Width 배율을 구함
    }
    if (ptTransStart.y == ptTransEnd.y)   // 입력된 Zoom의 Y값변화가 없으면 X를 기준으로 Zoom
    {
        fZoomRateY = 0;
    }
    else
    {
        fZoomRateY = (float)rect.Height() / ((float)ptTransStart.y - (float)ptTransEnd.y); // 입력된 Zoom Area의 Height 배율을 구함
    }

    if (fZoomRateX < 0) fZoomRateX *= -1.f; // 음수일때 양수로
    if (fZoomRateY < 0) fZoomRateY *= -1.f;

    if (fZoomRateX < 1.f)		stZoom.fZoomRate = fZoomRateY;
    else if (fZoomRateY < 1.f)	stZoom.fZoomRate = fZoomRateX;
    else						stZoom.fZoomRate = min(fZoomRateX, fZoomRateY); // X, Y 중 작은배율을 사용

    if (stZoom.fZoomRate < 1.f)   // 1보다 작으면 return
    {
        return;
    }

    if (stZoom.fZoomRate > DEFINE_IMAQ_ZOOM_MAX)   // 16보다 크면 16으로 (Max)
    {
        stZoom.fZoomRate = DEFINE_IMAQ_ZOOM_MAX;
    }

    m_arrayZoomList.Add(m_stZoomParam); // 현재 Zoom 상태 저장
    m_stZoomParam = stZoom;				// Zoom Update

    imaqZoomWindow2(m_iImaqWindowNumber, stZoom.fZoomRate, stZoom.fZoomRate, m_stZoomParam.ptZoomCenter);
}

void CIMAQImageCtrl::ZoomPrevious()  // 이전 Zoom 불러오기
{
    int iZoomListCount = (int)m_arrayZoomList.GetCount(); // 20181127 sje (int) add
    if (iZoomListCount <= 0) // 저장된 Zoom이 없으면 return
    {
        return;
    }

    stIMAQCTRL_ZOOM_PARAM stZoomParam = m_arrayZoomList[iZoomListCount - 1];
    m_arrayZoomList.RemoveAt(iZoomListCount - 1);
    m_stZoomParam.fZoomRate = stZoomParam.fZoomRate;

    imaqZoomWindow2(m_iImaqWindowNumber, m_stZoomParam.fZoomRate, m_stZoomParam.fZoomRate, m_stZoomParam.ptZoomCenter);
}

void CIMAQImageCtrl::ZoomByWheel(BOOL bUp)
{
    if (bUp == TRUE)
    {
        if (m_stZoomParam.fZoomRate * 1.1 > DEFINE_IMAQ_ZOOM_MAX)
		{
            m_stZoomParam.fZoomRate = DEFINE_IMAQ_ZOOM_MAX;
		}
        else
        {
            m_stZoomParam.fZoomRate *= 1.1f;
        }

        imaqGetWindowCenterPos(m_iImaqWindowNumber, &m_stZoomParam.ptZoomCenter);
        imaqZoomWindow2(m_iImaqWindowNumber, m_stZoomParam.fZoomRate, m_stZoomParam.fZoomRate, m_stZoomParam.ptZoomCenter);
    }
    else
    {
        if (m_stZoomParam.fZoomRate / 1.1 < 0.1)
            m_stZoomParam.fZoomRate = 0.1f;
        else
        {
            m_stZoomParam.fZoomRate /= 1.1f;
        }

        imaqGetWindowCenterPos(m_iImaqWindowNumber, &m_stZoomParam.ptZoomCenter);
        imaqZoomWindow2(m_iImaqWindowNumber, m_stZoomParam.fZoomRate, m_stZoomParam.fZoomRate, m_stZoomParam.ptZoomCenter);
    }
}

void CIMAQImageCtrl::SetROIRect(CRect rectNew)
{
	if (!m_pimaqWnd || !m_pROI) return;

	m_rectROI = rectNew;

	int contourID = imaqGetContour(m_pROI, 0);
	imaqRemoveContour(m_pROI, contourID);

	imaqAddRectContour(m_pROI, imaqMakeRect(m_rectROI.top,
		m_rectROI.left,
		m_rectROI.Height(),
		m_rectROI.Width()));
	imaqSetWindowROI(m_pimaqWnd->GetIWndImg(), m_pROI);
}

void CIMAQImageCtrl::GetPatternImageAndRect(Image* pPtrnImg, CRect* prectROI)
{
	if (!m_pBaseImage)	return;
	ImageType type;
	imaqGetImageType(pPtrnImg, &type);
	imaqGetImageType(m_pBaseImage, &type);
	if (m_bHSLType) // JAI
		imaqCast(pPtrnImg, pPtrnImg, IMAQ_IMAGE_HSL, NULL, 0);
	else
		//		imaqCast(pPtrnImg, pPtrnImg,IMAQ_IMAGE_U8, NULL, 0);
		imaqCast(pPtrnImg, pPtrnImg, IMAQ_IMAGE_RGB, NULL, 0);
	*prectROI = &m_rectROI;
	Rect imaqRect = imaqMakeRect(prectROI->top, prectROI->left, prectROI->Height(), prectROI->Width());
	int aa = imaqSetImageSize(pPtrnImg, imaqRect.width, imaqRect.height);
	int a = imaqCopyRect(pPtrnImg, m_pBaseImage, imaqRect, imaqMakePoint(0, 0));
	if (!a)
	{
		char* errStr = imaqGetErrorText(imaqGetLastError());
		//		traceF(M_METROSYS, LV_ALARM, CODE_ER_IMAQ, "LoadPtrn() : %s", errStr);
		TRACE("%s(%i) : %s\n", __FILE__, __LINE__, errStr);
		imaqDispose(errStr);
	}

}

BOOL CIMAQImageCtrl::PopupROIDefine2(CWnd* pParent, int uiID, Image* pImage, CRect rectOld, BOOL bDis)
{
	if (!m_pParent)
	{
		m_pParent = pParent;
	}

	// make image
	if (!m_pBaseImage)	// blue 2010.01.14 이건 타입이 크게 의미가 없어서 걍 U8로 간다.
	{
		if (m_bHSLType) //JAI CAMERA
		{
			m_pBaseImage = imaqCreateImage(IMAQ_IMAGE_HSL, 7);
			m_pOverlayImage = imaqCreateImage(IMAQ_IMAGE_HSL, 7);
		}
		else
		{
			m_pBaseImage = imaqCreateImage(IMAQ_IMAGE_RGB, 7);
			m_pOverlayImage = imaqCreateImage(IMAQ_IMAGE_RGB, 7);
		}
	}
	imaqDuplicate(m_pBaseImage, pImage);	// blue 2010.01.14 여기서 타입 바뀔수 있다.
	imaqDuplicate(m_pOverlayImage, pImage);	// blue 2010.01.14 여기서 타입 바뀔수 있다.

	imaqCast(m_pBaseImage, m_pBaseImage, IMAQ_IMAGE_RGB, NULL, 0);
	imaqScale(m_pBaseImage, m_pBaseImage, 1, 1, IMAQ_SCALE_SMALLER, IMAQ_NO_RECT);

	CRect uiRect;
	// make popupWnd & displayImage
	MakePopupWnd(pParent, uiID);		// make CIMAQImage

	(m_pParent->GetDlgItem(uiID))->GetWindowRect(&uiRect);
	m_pParent->ScreenToClient(&uiRect);

	

	if (bDis)
	{
		imaqShowScrollbars(m_pimaqWnd->GetIWndImg(), TRUE);
		double dScaleX = 0, dScaleY = 0, dScale = 0;
		int width = 0, height = 0/*, width2 = 0, height2 = 0*/;
		imaqGetImageSize(pImage, &width, &height);
//		imaqSetWindowSize(m_pimaqWnd->GetIWndImg(), width, height);
		

		dScaleX = (double)uiRect.Width() / width;
		dScaleY = (double)uiRect.Height() / height;
		dScale = (dScaleX > dScaleY ? dScaleY : dScaleX);

		imaqZoomWindow2(m_pimaqWnd->GetIWndImg(), (float)dScale, (float)dScale, IMAQ_NO_POINT);

		imaqDisplayImage(m_pBaseImage, m_pimaqWnd->GetIWndImg(), TRUE);

		MakeROITool();
		SetROIToolOption(IMAQ_RECTANGLE_TOOL);

		// startCB
		imaqSetEventCallback(CBEventROIWnd, FALSE);
	}
	else
	{
		imaqSetWindowSize(m_pimaqWnd->GetIWndImg(), 2560 / 3, 2048 / 3);
		imaqZoomWindow(m_pimaqWnd->GetIWndImg(), -3, -3, IMAQ_NO_POINT);
		imaqDisplayImage(m_pBaseImage, m_pimaqWnd->GetIWndImg(), TRUE);	// Without this line, image is displayed in wrong position.
	}


#if _DEBUG
	imaqWriteJPEGFile(m_pBaseImage, "D:\\Temp\\Snap3.jpg", 750, NULL);
#endif

	// make ROITool
	SetROIRect(rectOld);
	m_rectROI = rectOld;

	//	TRACE(">> ROI_CB : %x, %x, %x\n", g_imaqImgCtrl, m_pParent, g_imaqImgCtrl->m_pParent);

	return TRUE;
}

void CIMAQImageCtrl::ZoomAll()  // Zoom 초기화
{
    m_stZoomParam.fZoomRate = 1.f;
    imaqZoomWindow2(m_iImaqWindowNumber, m_stZoomParam.fZoomRate, m_stZoomParam.fZoomRate, IMAQ_NO_POINT);

	m_arrayZoomList.RemoveAll();
}


//########################################################################
//#
//# imaq 내부적으로 사용하는 함수..
//#
//########################################################################
void CIMAQImageCtrl::CastToU8(Image *pDestU8, Image *pSrc_U16_SGL)
{
    ImageType typeSorce;
    int       width;
    int       height;
    imaqGetImageType(pSrc_U16_SGL, &typeSorce);
    imaqGetImageSize(pSrc_U16_SGL, &width, &height);

    if (typeSorce != IMAQ_IMAGE_I16 && typeSorce != IMAQ_IMAGE_SGL) return;

    Rect   rect = imaqMakeRect(0, 0, height, width);
    Image *Mask = imaqCreateImage(IMAQ_IMAGE_U8, 4);
    ROI   *roi = imaqCreateROI();
    int    inSpace;

    imaqAddRectContour(roi, rect);
    imaqROIToMask(Mask, roi, 1, m_pImage_onlylink, &inSpace);

    QuantifyReport *pReport = imaqQuantify(m_pImage_onlylink, Mask);
    if (pReport == NULL || pReport->regions == NULL)
    {
        imaqDispose(Mask); Mask = NULL;
        imaqDispose(roi); roi = NULL;
        return;
    }

    float fMin = pReport->global.min;
    float fMax = pReport->global.max;

    imaqDispose(pReport); pReport = NULL;
    imaqDispose(Mask); Mask = NULL;
    imaqDispose(roi); roi = NULL;

    int nPixel = height * width;
    double dStep = (fMax - fMin) / 255;
    unsigned char *ucImg = new unsigned char[nPixel];

    int iSw, iSh;
    switch (typeSorce)
    {
        case IMAQ_IMAGE_I16:
        {
            short *pI16Src = (short *)imaqImageToArray(pSrc_U16_SGL, IMAQ_NO_RECT, &iSw, &iSh);
            for (int i = 0; i < nPixel; i++) ucImg[i] = (int)((pI16Src[i] - fMin) / dStep);
            imaqDispose(pI16Src); pI16Src = NULL;
        }
        break;

        case IMAQ_IMAGE_SGL:
        {
            float *pSGLSrc = (float *)imaqImageToArray(pSrc_U16_SGL, IMAQ_NO_RECT, &iSw, &iSh);
            for (int i = 0; i < nPixel; i++) ucImg[i] = (int)((pSGLSrc[i] - fMin) / dStep);
            imaqDispose(pSGLSrc); pSGLSrc = NULL;
        }
        break;
    }

    imaqArrayToImage(pDestU8, ucImg, iSw, iSh);
    imaqCopyOverlay(pDestU8, pSrc_U16_SGL, NULL);

    delete[] ucImg;
}

// Zoom 상태의 Window Position에서 x1 상태의 Window Position으로 변환
CPoint CIMAQImageCtrl::TransOriginWindowCoord(CPoint point, stIMAQCTRL_ZOOM_PARAM stZoomParam)  // 좌표 변경(Zoom 상태 -> 1배 상태로)
{
    CRect rect;
    CPoint ptRet;
    GetWindowRect(&rect);
    ptRet.x = (LONG)((point.x - rect.Width() / 2) / stZoomParam.fZoomRate) + stZoomParam.ptZoomCenter.x;
    ptRet.y = (LONG)((point.y - rect.Height() / 2) / stZoomParam.fZoomRate) + stZoomParam.ptZoomCenter.y;

    return ptRet;
}




//########################################################################
//#
//# imaq Overlay 관련으로 사용하는 함수..
//#
//########################################################################
void CIMAQImageCtrl::ClearGrid()
{
    ClearOverlay(m_pOverlay[eNI_OVERLAY_LAYER_GRID]);

    return;
}

void CIMAQImageCtrl::SetGrid(double dUnitLenght_nm, double dStepSize_um, COLORREF color, int iLineType)
{
    if (m_pImage_onlylink == NULL)
        return;

    if (dStepSize_um < (dUnitLenght_nm / 1000.0))
    {
        //g_Log->Log(LOG_LEVEL_ERROR, _T("need check grid size(%fum/%fnm).."), dStepSize_um, dUnitLenght_nm);
        return;
    }

    Rect imgSize;
    imgSize.left = 0;
    imgSize.top = 0;
    imaqGetImageSize(m_pImage_onlylink, &(imgSize.width), &(imgSize.height));

    Point pCenter;
    pCenter.x = (int)((imgSize.width / 2.0) + 0.5);
    pCenter.y = (int)((imgSize.height / 2.0) + 0.5);

    int nStepPixel = (int)((dStepSize_um / (dUnitLenght_nm / 1000.0)) + 0.5);

    Point pS, pE;
    pS.x = pCenter.x;
    pS.y = 0;
    pE.x = pCenter.x;
    pE.y = imgSize.height;
    OverlayLine(m_pOverlay[eNI_OVERLAY_LAYER_GRID], pS, pE, RGB(255, 0, 0), 0);
    pS.x = 0;
    pS.y = pCenter.y;
    pE.x = imgSize.width;
    pE.y = pCenter.y;
    OverlayLine(m_pOverlay[eNI_OVERLAY_LAYER_GRID], pS, pE, RGB(255, 0, 0), 0);

    for (int ix = (pCenter.x + nStepPixel); ix < imgSize.width; ix += nStepPixel)
    {
        pS.x = ix;
        pS.y = 0;
        pE.x = ix;
        pE.y = imgSize.height;
        OverlayLine(m_pOverlay[eNI_OVERLAY_LAYER_GRID], pS, pE, color, 0);
    }

    for (int ix = (pCenter.x - nStepPixel); ix > 0; ix -= nStepPixel)
    {
        pS.x = ix;
        pS.y = 0;
        pE.x = ix;
        pE.y = imgSize.height;

        OverlayLine(m_pOverlay[eNI_OVERLAY_LAYER_GRID], pS, pE, color, 0);
    }

    for (int iy = (pCenter.y + nStepPixel); iy < imgSize.height; iy += nStepPixel)
    {
        pS.x = 0;
        pS.y = iy;
        pE.x = imgSize.width;
        pE.y = iy;
        OverlayLine(m_pOverlay[eNI_OVERLAY_LAYER_GRID], pS, pE, color, 0);
    }

    for (int iy = (pCenter.y - nStepPixel); iy > 0; iy -= nStepPixel)
    {
        pS.x = 0;
        pS.y = iy;
        pE.x = imgSize.width;
        pE.y = iy;
        OverlayLine(m_pOverlay[eNI_OVERLAY_LAYER_GRID], pS, pE, color, 0);
    }

    return;
}

void CIMAQImageCtrl::ClearROI()
{
    imaqSetWindowROI(m_iImaqWindowNumber, NULL);
}

void CIMAQImageCtrl::SetROI(void *vROI, eROI_TYPE tType)
{
    ROI *roi = imaqCreateROI();

    switch (tType)
    {
        case eROI_TYPE_RECT:
        {
            Rect r = *((Rect *)vROI);
            imaqAddRectContour(roi, r);
        }
        break;

        case eROI_TYPE_LINE:
        {
            Point s, e;
            s = ((Point *)vROI)[0];
            e = ((Point *)vROI)[1];
            imaqAddLineContour(roi, s, e);
        }
        break;

        case eROI_TYPE_POINT:
        {
            Point p = *((Point *)vROI);
            imaqAddPointContour(roi, p);
        }
        break;

        case eROI_TYPE_ROTATEDRECT:
        {
            RotatedRect r = *((RotatedRect *)vROI);
            imaqAddRotatedRectContour2(roi, r);
        }
        break;

        case eROI_TYPE_ANNULUS:
        {
            Annulus a = *((Annulus *)vROI);
            imaqAddAnnulusContour(roi, a);
        }
        break;

        default:
        break;
    }

    imaqSetWindowROI(m_iImaqWindowNumber, roi);

    if (roi != NULL)
    {
        imaqDispose(roi);
        roi = NULL;
    }
}

ROI *CIMAQImageCtrl::GetROI()
{
    return imaqGetWindowROI(m_iImaqWindowNumber);
}

void CIMAQImageCtrl::ClearOverlayAll()
{
    imaqClearOverlay(m_pImage_onlylink, NULL);

    for (int i = 0; i < eNI_OVERLAY_LAYER_END; i++)
    {
        imaqClearOverlay(m_pOverlay[i], NULL);
    }
}

void CIMAQImageCtrl::ClearOverlay(eNI_OVERLAY_LAYER layer)
{
    imaqClearOverlay(m_pOverlay[layer], NULL);
}

void CIMAQImageCtrl::ClearOverlay(Image *imgOverTarget)
{
    imaqClearOverlay(imgOverTarget, NULL);
}

void CIMAQImageCtrl::OverlayRect(Image *imgOverTarget, Rect r, COLORREF color, unsigned char ucAlpha/*=0*/, DrawMode drawMode/*=IMAQ_HIGHLIGHT_VALUE*/)
{
    unsigned char ucRGBValue[4];
    ucRGBValue[0] = GetBValue(color);
    ucRGBValue[1] = GetGValue(color);
    ucRGBValue[2] = GetRValue(color);
    ucRGBValue[3] = ucAlpha;
    imaqOverlayRect(imgOverTarget, r, (RGBValue *)ucRGBValue, drawMode, NULL);
}

void CIMAQImageCtrl::OverlayRotatedRect(Image *imgOverTarget, RotatedRect rr, COLORREF color, unsigned char ucAlpha/*=0*/, DrawMode drawMode/*=IMAQ_HIGHLIGHT_VALUE*/)
{
    unsigned char ucRGBValue[4];
    ucRGBValue[0] = GetBValue(color);
    ucRGBValue[1] = GetGValue(color);
    ucRGBValue[2] = GetRValue(color);
    ucRGBValue[3] = ucAlpha;

    Point p[4];
    p[0].x = rr.left; p[0].y = rr.top;
    p[1].x = rr.left + rr.width; p[1].y = rr.top;
    p[2].x = rr.left + rr.width; p[2].y = rr.top + rr.height;
    p[3].x = rr.left; p[3].y = rr.top + rr.height;

    double cx = (double)rr.left + (double)rr.width / 2.0;
    double cy = (double)rr.top + (double)rr.height / 2.0;

    double dT = IMAQ_PI * rr.angle / 180.0;
    double dCos = cos(dT);
    double dSin = sin(dT);

    Point cp[4];
    Point tp;
    for (int i = 0; i < 4; i++)
    {
        tp = imaqMakePoint((int)(p[i].x - cx), (int)(p[i].y - cy));
        cp[i] = imaqMakePoint((int)((tp.x * dCos + tp.y * dSin) + cx + 0.5), (int)((-tp.x * dSin + tp.y * dCos) + cy + 0.5));
    }

    imaqOverlayClosedContour(imgOverTarget, cp, 4, (RGBValue *)ucRGBValue, drawMode, NULL);
}

void CIMAQImageCtrl::OverlayLine(Image *imgOverTarget, Point pStart, Point pEnd, COLORREF color, unsigned char ucAlpha/*=0*/)
{
    unsigned char ucRGBValue[4];
    ucRGBValue[0] = GetBValue(color);
    ucRGBValue[1] = GetGValue(color);
    ucRGBValue[2] = GetRValue(color);
    ucRGBValue[3] = ucAlpha;
    imaqOverlayLine(imgOverTarget, pStart, pEnd, (RGBValue *)ucRGBValue, NULL);
}

void CIMAQImageCtrl::OverlayPoint(Image *imgOverTarget, Point pos, COLORREF color, unsigned char ucAlpha, int iEmphasis, COLORREF cEmphasisColor)
{
    unsigned char ucRGBValue[4];
    ucRGBValue[0] = GetBValue(color);
    ucRGBValue[1] = GetGValue(color);
    ucRGBValue[2] = GetRValue(color);
    ucRGBValue[3] = ucAlpha;

    if (iEmphasis != 0)
    {
        Rect r = imaqMakeRect(pos.y - iEmphasis, pos.x - iEmphasis, (iEmphasis * 2) + 1, (iEmphasis * 2) + 1);

        OverlayRect(imgOverTarget, r, cEmphasisColor, 100, IMAQ_HIGHLIGHT_VALUE);
    }

    imaqOverlayPoints(imgOverTarget, &pos, 1, (RGBValue *)ucRGBValue, 1, IMAQ_POINT_AS_PIXEL, NULL, NULL);
}

void CIMAQImageCtrl::OverlayText(Image *imgOverTarget, Point pos, CString sMsg, COLORREF color, unsigned char ucAlpha)
{
    unsigned char ucRGBValue[4];
    ucRGBValue[0] = GetBValue(color);
    ucRGBValue[1] = GetGValue(color);
    ucRGBValue[2] = GetRValue(color);
    ucRGBValue[3] = ucAlpha;

    USES_CONVERSION;
    imaqOverlayText(imgOverTarget, pos, T2A(sMsg), (RGBValue *)ucRGBValue, NULL, NULL);
}

void CIMAQImageCtrl::OverlayText2(eNI_OVERLAY_LAYER layer, Point pos, CString sMsg, COLORREF color, unsigned char ucAlpha)
{
	OverlayText2(m_pOverlay[layer], pos, sMsg, color, ucAlpha);
}

void CIMAQImageCtrl::OverlayText2(Image *imgOverTarget, Point pos, CString sMsg, COLORREF color, unsigned char ucAlpha)
{
    unsigned char ucRGBValue[4];
    ucRGBValue[0] = GetBValue(color);
    ucRGBValue[1] = GetGValue(color);
    ucRGBValue[2] = GetRValue(color);
    ucRGBValue[3] = ucAlpha;

    OverlayTextOptions option;
    option.fontSize = 20;
    option.fontName = (CStringA)sMsg;
    option.bold = 4;
    option.angle = 0;
    option.horizontalTextAlignment = IMAQ_LEFT;
    option.verticalTextAlignment = IMAQ_BOTTOM;
    option.italic = 0;
    option.strikeout = 0;
    option.underline = 0;

    USES_CONVERSION;
    imaqOverlayText(imgOverTarget, pos, T2A(sMsg), (RGBValue *)ucRGBValue, &option, NULL);
}

void CIMAQImageCtrl::OverlayAnnulus(Image *imgOverTarget, Annulus Ann, COLORREF color, unsigned char ucAlpha)
{
    unsigned char ucRGBValue[4];
    ucRGBValue[0] = GetBValue(color);
    ucRGBValue[1] = GetGValue(color);
    ucRGBValue[2] = GetRValue(color);
    ucRGBValue[3] = ucAlpha;

    ROI *roi = imaqCreateROI();
    imaqAddAnnulusContour(roi, Ann);
    imaqSetROIColor(roi, (RGBValue *)ucRGBValue);
    imaqOverlayROI(imgOverTarget, roi, IMAQ_POINT_SYMBOL_SIZE_GUARD, NULL, NULL);

    if (roi != NULL)
    {
        imaqDispose(roi);
        roi = NULL;
    }
}

void CIMAQImageCtrl::OverlayDottedLine(Image *imgOverTarget, Point pStart, Point pEnd, int nDot, COLORREF color, unsigned char ucAlpha/*=0*/)
{
    unsigned char ucRGBValue[4];
    ucRGBValue[0] = GetBValue(color);
    ucRGBValue[1] = GetGValue(color);
    ucRGBValue[2] = GetRValue(color);
    ucRGBValue[3] = ucAlpha;

    float fW = (float)(pEnd.x - pStart.x);
    float fH = (float)(pEnd.y - pStart.y);

    float fWS = (float)(fW / ((2.0 * nDot) + 1));
    float fHS = (float)(fH / ((2.0 * nDot) + 1));

    Point iS, iE;

    for (int i = 0; i < nDot; i++)
    {
        iS.x = pStart.x + (int)((fWS * 2 * i) + 0.5);
        iE.x = pStart.x + (int)((fWS * ((2 * i) + 1)) + 0.5);
        iS.y = pStart.y + (int)((fHS * 2 * i) + 0.5);
        iE.y = pStart.y + (int)((fHS * ((2 * i) + 1)) + 0.5);;
        imaqOverlayLine(imgOverTarget, iS, iE, (RGBValue *)ucRGBValue, NULL);
    }

    imaqOverlayLine(imgOverTarget, iE, pEnd, (RGBValue *)ucRGBValue, NULL);
}

void CIMAQImageCtrl::OverlayDottedRect(Image *imgOverTarget, Rect rect, int nLinePerDot, COLORREF color, unsigned char ucAlpha/*=0*/)
{
    Point S, E;
    S.x = rect.left;
    S.y = rect.top;
    E.x = rect.left + rect.width;
    E.y = rect.top;
    OverlayDottedLine(imgOverTarget, S, E, nLinePerDot, color, ucAlpha);

    S.x = rect.left + rect.width;
    S.y = rect.top;
    E.x = rect.left + rect.width;
    E.y = rect.top + rect.height;
    OverlayDottedLine(imgOverTarget, S, E, nLinePerDot, color, ucAlpha);

    S.x = rect.left + rect.width;
    S.y = rect.top + rect.height;
    E.x = rect.left;
    E.y = rect.top + rect.height;
    OverlayDottedLine(imgOverTarget, S, E, nLinePerDot, color, ucAlpha);

    S.x = rect.left;
    S.y = rect.top + rect.height;
    E.x = rect.left;
    E.y = rect.top;
    OverlayDottedLine(imgOverTarget, S, E, nLinePerDot, color, ucAlpha);
}

void CIMAQImageCtrl::OverlayOval(Image *imgOverTarget, Rect rect, COLORREF color, unsigned char ucAlpha/*=0*/, DrawMode drawMode/*=IMAQ_DRAW_VALUE*/)
{
    unsigned char ucRGBValue[4];
    ucRGBValue[0] = GetBValue(color);
    ucRGBValue[1] = GetGValue(color);
    ucRGBValue[2] = GetRValue(color);
    ucRGBValue[3] = ucAlpha;

    if (imaqOverlayOval(imgOverTarget, rect, (RGBValue *)ucRGBValue, drawMode, NULL) == 0)
    {
        AfxMessageBox(_T("Err"));
    }
}

BOOL CIMAQImageCtrl::PtrnLearning(Image* pPtrnImg)
{
    // Learn pattern -----------------------------------------
    int learn = 0;
    BOOL bRet = FALSE;   
//	learn = imaqLearnPattern(pPtrnImg, IMAQ_LEARN_SHIFT_INFORMATION); // OLD

     // imaqMatchPattern3 for Use
     learn = imaqLearnPattern3(pPtrnImg, IMAQ_LEARN_SHIFT_INFORMATION, NULL, NULL);
  
    if (!learn)
    {
        char* errStr = imaqGetErrorText(imaqGetLastError());
        TRACE("%s(%i) : %s\n", __FILE__, __LINE__, errStr);
        imaqDispose(errStr);
        bRet = FALSE;
    }
    else
    {
        bRet = TRUE;
    }

    return learn;
}

float CIMAQImageCtrl::GetDistancePtrnToCntr(CPoint pt, int iPgmMode, int iW, int iH)
{
	float fdelX, fdelY;

	fdelX = float(iW / 2) - (float)pt.x;
	fdelY = float(iH / 2) - (float)pt.y;

	float fDist = fdelX * fdelX + fdelY * fdelY;

	return fDist;
}

//=========================================================
// [i] pImg : 비교하기위한 바탕이미지
// [i] pPtrnImg : 비교할 패턴 이미지
// [i] minMatchScore : Matching score
// [i] bLearn : Ptrn Learning 여부.
// [i] iMatchMethod : 0:Score가 높은넘 찾기 / 1:거리가 가까운넘 찾기
// [o] pMatchRect : 찾은 렉트
// [o] pfScore : 찾은패턴의 스코어
//=========================================================
BOOL CIMAQImageCtrl::SearchPattern(Image* pImg, Image* pPtrnImg, float minMatchScore, BOOL bLearn, int iMatchMethod, int bMeasPtrn,
	CRect* pMatchRect, float* pfScore, int nMatchPositionX, int nMatchPositionY, int iMethod, int nThreshold)
{
	ImageType type1, type2;
	imaqGetImageType(pImg, &type1);
	imaqGetImageType(pPtrnImg, &type2);

	BOOL bUseClrMatch = FALSE;	//uhkang 20100317 컬러 매칭할것인지 아닌지를 받아와야 함. 우선 무조건 FALSE로 가자!@#

// Learn pattern -----------------------------------------
	if (bLearn)
	{
		if ((type2 == IMAQ_IMAGE_RGB) && bUseClrMatch)
		{
			LearnColorPatternOptions ClrOptions;
			//Color Pattern Matching Learn Option.
			ClrOptions.featureMode = IMAQ_COLOR_AND_SHAPE_FEATURES;		//IMAQ_SHAPE_FEATURES
			ClrOptions.learnMode = IMAQ_LEARN_SHIFT_INFORMATION;
			//pOptions->threshold		= 128;

			// shlee 20060710 change 120 -> 80;
			ClrOptions.threshold = 80;//120; //80;	// shlee 20060227 modify 128 -> 80  // shlee 20060530 80 -> 120
			ClrOptions.ignoreMode = IMAQ_IGNORE_NONE;
			ClrOptions.colorsToIgnore = NULL;
			ClrOptions.numColorsToIgnore = 0;
			if (!imaqLearnColorPattern(pPtrnImg, &ClrOptions)) return FALSE;
		}
		else if (!bUseClrMatch)
		{
			if (m_pPtrnImgBW == NULL)
				m_pPtrnImgBW = imaqCreateImage(IMAQ_IMAGE_U8, INT_IMAGE_BORDER_SIZE);

			if (type2 != IMAQ_IMAGE_U8)
			{
				imaqCast(m_pPtrnImgBW, pPtrnImg, IMAQ_IMAGE_U8, NULL, 0);
			}
			else
			{
				imaqDuplicate(m_pPtrnImgBW, pPtrnImg);
			}

			BOOL bRet = FALSE;
			bRet = PtrnLearning(m_pPtrnImgBW);
			if (!bRet)
			{
				//ksk 20150716 add
				if (m_pPtrnImgBW != NULL)
				{
					imaqDispose(m_pPtrnImgBW);
					m_pPtrnImgBW = NULL;
				}

				//end
				return FALSE;
			}
		}
		else
		{
			if (!PtrnLearning(pPtrnImg))
				return FALSE;
		}
	}

	ROI* roi = NULL;
	Rect rectROI;
	roi = imaqCreateROI();
	// Creates a new rectangle ROI contour and adds the rectangle to the provided ROI.

	int width = 0, height = 0;
	imaqGetImageSize(pImg, &width, &height);
	int nMargin = 20;

	rectROI.left = nMargin;
	rectROI.top = nMargin;
	rectROI.height = height - nMargin;
	rectROI.width = width - nMargin;

	imaqAddRotatedRectContour2(roi, imaqMakeRotatedRect(rectROI.top, rectROI.left, rectROI.height, rectROI.width, 0));

	// Match pattern ------------------------------------------
	int nMatch = 0;
	PatternMatch* pMatchInfo = NULL;

	if (bUseClrMatch)
	{
		MatchColorPatternOptions MatchColorPtrnOptions;
		RotationAngleRange AngleRanges;
		MatchColorPtrnOptions.featureMode = IMAQ_COLOR_AND_SHAPE_FEATURES;
		//Set matching options
		AngleRanges.upper = 10.0;								//ko
		AngleRanges.lower = -10.0;								//ko
		MatchColorPtrnOptions.matchMode = IMAQ_MATCH_SHIFT_INVARIANT;
		MatchColorPtrnOptions.minContrast = 0;	// from Change setup dlg.
		MatchColorPtrnOptions.subpixelAccuracy = TRUE;
		MatchColorPtrnOptions.angleRanges = &AngleRanges;
		MatchColorPtrnOptions.numRanges = 0;	// shlee 20060227 modify  1 -> 0
		MatchColorPtrnOptions.colorWeight = 50;	// from  Change Setup dlg.

		MatchColorPtrnOptions.sensitivity = IMAQ_SENSITIVITY_HIGH;//IMAQ_SENSITIVITY_LOW;
		MatchColorPtrnOptions.strategy = IMAQ_CONSERVATIVE; //IMAQ_BALANCED;//
		MatchColorPtrnOptions.numMatchesRequested = 10; //1->100	// shlee 20060227 modify
		MatchColorPtrnOptions.minMatchScore = minMatchScore; \

			if ((type1 == IMAQ_IMAGE_RGB) && bUseClrMatch)
			{
				pMatchInfo = imaqMatchColorPattern(pImg, pPtrnImg, &MatchColorPtrnOptions, IMAQ_NO_RECT, &nMatch);
				if (pMatchInfo == NULL)
				{
					char* errStr = imaqGetErrorText(imaqGetLastError());
					TRACE("%s(%i) : %s\n", __FILE__, __LINE__, errStr);
					imaqDispose(errStr);
				}
			}
	}
	else
	{
		MatchPatternOptions options;

		options.mode = DEFAULT_MATCH_MODE;
		options.minMatchScore = minMatchScore;
		options.subpixelAccuracy = DEFAULT_SUBPIXEL;
		options.angleRanges = DEFAULT_ANGLE_RANGES;
		options.numRanges = DEFAULT_NUM_RANGES;
		//	options.numMatchesRequested = DEFAULT_NUM_MATCHES;
		options.minContrast = DEFAULT_MIN_CONTRAST;
		options.matchFactor = DEFAULT_MATCH_FACTOR;

		//options.mode				= IMAQ_MATCH_SHIFT_INVARIANT;
		//options.minContrast			= 10;
		//options.subpixelAccuracy	= FALSE;
		//options.angleRanges			= NULL;
		//options.numRanges			= 0;
		//options.matchFactor			= 0;
		//options.minMatchScore		= minMatchScore;
		//
		//// jsyoon 20210427 change
		//options.numMatchesRequested = 10;	// jsyoon 20150304 chane 50 to 10
		if (bMeasPtrn == TRUE)
			options.numMatchesRequested = 10;	// jsyoon 20150304 chane 50 to 10
		else
			options.numMatchesRequested = DEFAULT_NUM_MATCHES;

		if (type1 != IMAQ_IMAGE_U8)
		{
			Image* pImgBW = imaqCreateImage(IMAQ_IMAGE_U8, INT_IMAGE_BORDER_SIZE);
			Image* pPtrnImgBW = imaqCreateImage(IMAQ_IMAGE_U8, INT_IMAGE_BORDER_SIZE);


			if (m_pOverlayImage == NULL)
				m_pOverlayImage = imaqCreateImage(IMAQ_IMAGE_RGB, INT_IMAGE_BORDER_SIZE);

			imaqDuplicate(m_pOverlayImage, pImg);	// blue 2010.01.14 여기서 타입 바뀔수 있다.
			imaqCast(pImgBW, pImg, IMAQ_IMAGE_U8, NULL, 0);		// row image

	//		imaqMathTransform(pImgBW, pImgBW, IMAQ_TRANSFORM_POWX, 0.0, 0.0, 1.5, NULL);

			if (bLearn)
			{
				imaqDuplicate(pPtrnImgBW, m_pPtrnImgBW);		// Pattern image
			}
			else
			{
				if (type2 != IMAQ_IMAGE_U8)
				{
					imaqCast(pPtrnImgBW, pPtrnImg, IMAQ_IMAGE_U8, NULL, 0);		// row image
				}
				else
				{
					imaqDuplicate(pPtrnImgBW, pPtrnImg);		// Pattern image
				}
			}

		//	pMatchInfo = imaqMatchPattern(pImgBW, pPtrnImgBW, &options, IMAQ_NO_RECT, &nMatch);	// OLD
			pMatchInfo = imaqMatchPattern3(pImgBW, pPtrnImgBW, &options, NULL, roi, &nMatch);

			if (pMatchInfo == NULL)
			{
				char* errStr = imaqGetErrorText(imaqGetLastError());
				TRACE("%s(%i) : %s\n", __FILE__, __LINE__, errStr);
				imaqDispose(errStr);
			}
	

#if _DEBUG
			imaqWriteJPEGFile(pImgBW, "D:\\Temp\\1.jpg", 750, NULL);
			imaqWriteJPEGFile(pPtrnImgBW, "D:\\Temp\\2.jpg", 750, NULL);
#endif			
			imaqDispose(pImgBW);
			imaqDispose(pPtrnImgBW);
		}
		else
		{
			pMatchInfo = imaqMatchPattern(pImg, pPtrnImg, &options, IMAQ_NO_RECT, &nMatch);
		}
	}

	BOOL bRet = (nMatch > 0);
	*pfScore = 0.;	// blue 2008.08.25 
	BOOL bPositionSearch = FALSE;

	int iIdx = 0;
	if (bRet)
	{
		if (1 == nMatch) 			// Match Found! Use the first one only.
		{
			iIdx = 0;

			*pfScore = pMatchInfo[iIdx].score;
		
			bPositionSearch = TRUE;
		}
		else						// Match Found! Use the Search
		{
			// superman 2006.11.03 finds pattern that are nearest to target position
			// align pattern must be only one in display area
			float fNear = 1e+9f;
			int iMinGab = 1000;
			float fMaxScore = 0.;	// blue 2008.08.12 compare score

			// jsyoon 20210423 add For Color Image
			bool* pbColor = NULL;
			pbColor = new bool[nMatch];
			int iColor = 0;
			for (iColor = 0; iColor < nMatch; iColor++)
			{
				pbColor[iColor] = true;
			}

			bRet = FALSE;

			if (bMeasPtrn == TRUE && iMethod > 0)
			{
				//if ((type1 == IMAQ_IMAGE_RGB) && (type2 == IMAQ_IMAGE_RGB))
				//{
				//	int nColorType1 = 0, nColorType2 = 0;
				//	int nRoi = 10, k = 0;
				//	//-------------------------------------------------------------------//
				//	//                       Color Pattern Matching                      //
				//	//-------------------------------------------------------------------//
				//	// Pattern
				//	imaqGetImageSize(pPtrnImg, &width, &height);

				//	Rect rectROIImage;
				//	Rect rectROIPtrnImage;
				//	if (width > nRoi && height > nRoi)
				//	{
				//		if (iMethod == 1)
				//		{
				//			rectROIPtrnImage.left = (width / 2) - (int)(nRoi / 2);
				//			rectROIPtrnImage.top = (height / 2) - (int)(nRoi / 2);
				//			rectROIPtrnImage.height = nRoi;
				//			rectROIPtrnImage.width = nRoi;

				//			nColorType2 = CalcModeType(pPtrnImg, rectROIPtrnImage);		// jsyoon 20210517 add
				//			if (nColorType2 != 5)
				//			{
				//				// RawImage
				//				for (k = 0; k < nMatch; k++)
				//				{
				//					rectROIImage.left = pMatchInfo[k].position.x - (int)(nRoi / 2);
				//					rectROIImage.top = pMatchInfo[k].position.y - (int)(nRoi / 2);
				//					rectROIImage.height = nRoi;
				//					rectROIImage.width = nRoi;

				//					if (CheckColorImage(pImg, pPtrnImg, rectROIImage, rectROIPtrnImage) != TRUE)
				//					{
				//						pbColor[k] = false;
				//					}
				//				}
				//			}
				//		}
				//		else if (iMethod == 2)
				//		{
				//			rectROIImage.left = (width / 2) - (int)(nRoi / 2);
				//			rectROIImage.top = (height / 2) - (int)(nRoi / 2);
				//			rectROIImage.height = nRoi;
				//			rectROIImage.width = nRoi;
				//			nColorType2 = CalcModeType(pPtrnImg, rectROIImage);

				//			// RawImage
				//			for (k = 0; k < nMatch; k++)
				//			{
				//				rectROIImage.left = pMatchInfo[k].position.x - (int)(nRoi / 2);
				//				rectROIImage.top = pMatchInfo[k].position.y - (int)(nRoi / 2);
				//				rectROIImage.height = nRoi;
				//				rectROIImage.width = nRoi;

				//				nColorType1 = CalcModeType(pImg, rectROIImage);
				//				if (nColorType1 != nColorType2)
				//				{
				//					pbColor[k] = false;
				//				}
				//			}
				//		}

				//		int iMatch = 0;
				//		for (k = 0; k < nMatch; k++)
				//		{
				//			if (pbColor[k] == true)
				//			{
				//				iMatch++;
				//			}
				//		}

				//		// First search 
				//		if (iMatch < 1)
				//		{
				//			pbColor[0] = true;
				//		}
				//	}
				//}
			}
			///

			imaqGetImageSize(pImg, &width, &height);

			if (iMatchMethod == MATCH_SCORE)	// 1)max score를 찾고.. 2)max score가 같은넘은 distance 짧은놈으로 찾기..
			{
				for (int k = 0; k < nMatch; k++)
				{
					if (pbColor[k] == true)		// jsyoon 20210423 add
					{
						bRet = TRUE;

						if (pMatchInfo[k].score > fMaxScore)
						{
							fMaxScore = pMatchInfo[k].score;
							*pfScore = fMaxScore;
							iIdx = k;
						}
						else if (pMatchInfo[k].score == fMaxScore)
						{
							float fDist = GetDistancePtrnToCntr(CPoint((int)pMatchInfo[k].position.x, (int)pMatchInfo[k].position.y), 0, width, height);
							if (fDist < fNear)
							{
								fNear = fDist;
								*pfScore = pMatchInfo[k].score;
								iIdx = k;
							}
						}
					}
				}
			}
			else if (iMatchMethod == MATCH_POSITION)	// 1)distance 짧은놈으로 찾기..  2)max score를 찾고..
			{
				int iSecend = -1;
				if (nMatchPositionX == 0 && nMatchPositionY == 0)
				{
					nMatchPositionX = width / 2;
					nMatchPositionY = height / 2;
				}

				int igabX = 0, igabY = 0;

				for (int k = 0; k < nMatch; k++)
				{
					if (pbColor[k] == true)		// jsyoon 20210423 add
					{
						bRet = TRUE;

						igabX = abs((int)pMatchInfo[k].corner[0].x - nMatchPositionX);
						igabY = abs((int)pMatchInfo[k].corner[0].y - nMatchPositionY);

						float fDist = igabX * igabX + igabY * igabY;
						if (fDist < fNear)
						{
							fNear = fDist;
							iSecend = iIdx;
							iIdx = k;
						}
					}
				}

				if (iSecend > -1)
				{
					if (pMatchInfo[iIdx].score < pMatchInfo[iSecend].score)
					{
						iIdx = iSecend;
						*pfScore = pMatchInfo[iIdx].score;
						bPositionSearch = TRUE;
					}
					else
					{
						*pfScore = pMatchInfo[iIdx].score;
						bPositionSearch = TRUE;
					}
				}
				else
				{
					*pfScore = pMatchInfo[iIdx].score;
					bPositionSearch = TRUE;
				}
			}
			else	// MATCH_DISTANCE	// 1)가까운놈 찾기..
			{
				for (int k = 0; k < nMatch; k++)
				{
					if (pbColor[k] == true)		// jsyoon 20210423 add
					{
						bRet = TRUE;

						float fDist = GetDistancePtrnToCntr(CPoint((int)pMatchInfo[k].position.x, (int)pMatchInfo[k].position.y),
							0, width, height);
						if (fDist < fNear)
						{
							fNear = fDist;
							*pfScore = pMatchInfo[k].score;
							iIdx = k;
						}
					}
				}
			} // if(bScoreMatch)

			delete[] pbColor; // jsyoon 20210423 add
		}
	}

	if (bRet == TRUE)
	{
		Image* pTempImage = imaqCreateImage(IMAQ_IMAGE_U8, INT_IMAGE_BORDER_SIZE);
		imaqExtractColorPlanes(pImg, IMAQ_HSL, NULL, NULL, pTempImage);
		//	imaqCast(pTempImage, pImg, IMAQ_IMAGE_U8, NULL, 0);

		Image* pEdgeImage = imaqCreateImage(IMAQ_IMAGE_U8, INT_IMAGE_BORDER_SIZE);
		//		imaqDuplicate(pEdgeImage, pTempImage);
		imaqEdgeFilter(pEdgeImage, pTempImage, IMAQ_EDGE_SOBEL, NULL);

		//		float kernel[9] = { -1,-1,-1,-1,8,-1,-1,-1,-1 };
		//		imaqConvolve(pEdgeImage, pEdgeImage, kernel, 3, 3, 0, NULL);

		if (!bMeasPtrn)
		{
			Point start, end;
			start = imaqMakePoint((int)(pMatchInfo[iIdx].corner[0].x), (int)(pMatchInfo[iIdx].corner[0].y));
			end   = imaqMakePoint((int)(pMatchInfo[iIdx].corner[2].x), (int)(pMatchInfo[iIdx].corner[2].y));

			float iGapX = 0.5, iGapY = 0.5;
			PointFloat FindEdgeLEFT, FindEdgeRIGHT, FindEdgeTOP, FindEdgeBOTTOM;
			CPoint pEdge[2];
			CPoint pPreEdge[2];
			ROI* pROI = NULL;
			int nEdge = 1;

			FindEdgeLEFT.x = 0, FindEdgeLEFT.y = 0;
			FindEdgeRIGHT.x = 0, FindEdgeRIGHT.y = 0;
			FindEdgeTOP.x = 0, FindEdgeTOP.y = 0;
			FindEdgeBOTTOM.x = 0, FindEdgeBOTTOM.y = 0;

			// overlay the line
			Point EdgeStart, EdgeEnd;

			//		int iMethod = 2;
			int iRoiWidth = 0, iRoiHeight = 0;
			int iRetry = 0, nRetyMax = 3, nSetThreshold = 40;

			bRet = FALSE;

			switch (iMethod)
			{
			case 1:
			{
				iGapX = 0.0, iGapY = 0.0;

				// First Line Detect edge
				if (nEdge > 0)
				{
					iRoiWidth = (end.x - start.x) / 2;
					iRoiHeight = (end.y - start.y);

					// Second Detect edge
					pROI = imaqCreateROI();
					imaqAddRotatedRectContour2(pROI, imaqMakeRotatedRect(start.y, start.x, iRoiHeight, iRoiWidth, 0));

					iRetry = 0;
					nSetThreshold = nThreshold;
					do
					{
					/*	FindEdgeLEFT = IVA_FindEdge(pEdgeImage, pROI, nSetThreshold, 4, 2, 4, IMAQ_LEFT_TO_RIGHT, pEdge);
						if (FindEdgeLEFT.x >= start.x && FindEdgeLEFT.x <= end.x && FindEdgeLEFT.y >= start.y && FindEdgeLEFT.y <= end.y)*/
						{
							break;
						}

						iRetry++;
						nSetThreshold -= 10;

					} while (iRetry < nRetyMax);

					imaqDispose(pROI);

					pROI = imaqCreateROI();
					imaqAddRotatedRectContour2(pROI, imaqMakeRotatedRect(start.y, end.x - iRoiWidth, iRoiHeight, iRoiWidth, 0));

					iRetry = 0;
					nSetThreshold = nThreshold;
					do
					{
					/*	FindEdgeRIGHT = IVA_FindEdge(pEdgeImage, pROI, nSetThreshold, 4, 2, 4, IMAQ_RIGHT_TO_LEFT, pEdge);
						if (FindEdgeRIGHT.x > start.x && FindEdgeRIGHT.x <= end.x && FindEdgeRIGHT.y > start.y && FindEdgeRIGHT.y <= end.y)*/
						{
							break;
						}

						iRetry++;
						nSetThreshold -= 10;

					} while (iRetry < nRetyMax);


					imaqDispose(pROI);

					// overlay the line
					EdgeStart.x = (int)FindEdgeLEFT.x;
					EdgeStart.y = (int)FindEdgeLEFT.y;
					EdgeEnd.x = (int)FindEdgeRIGHT.x;
					EdgeEnd.y = (int)FindEdgeRIGHT.y;
					imaqOverlayLine(pTempImage, EdgeStart, EdgeEnd, &IMAQ_RGB_RED, NULL);

					if (FindEdgeLEFT.x != 0 && FindEdgeLEFT.y != 0 && FindEdgeRIGHT.x != 0 && FindEdgeRIGHT.y != 0 && abs(EdgeEnd.x - EdgeStart.x) > 30)
					{
						iGapX = ((FindEdgeLEFT.x + FindEdgeRIGHT.x) / 2) - ((pMatchInfo[iIdx].corner[1].x + pMatchInfo[iIdx].corner[0].x) / 2);
					}

				}

				if (nEdge > 0)
				{
					iRoiWidth = end.x - start.x;
					iRoiHeight = (end.y - start.y) / 2;

					pROI = imaqCreateROI();
					imaqAddRotatedRectContour2(pROI, imaqMakeRotatedRect(start.y, start.x, iRoiHeight, iRoiWidth, 0));

					iRetry = 0;
					nSetThreshold = nThreshold;
					do
					{
					/*	FindEdgeTOP = IVA_FindEdge(pEdgeImage, pROI, nSetThreshold, 4, 2, 4, IMAQ_TOP_TO_BOTTOM, pEdge);
						if (FindEdgeTOP.x >= start.x && FindEdgeTOP.x <= end.x && FindEdgeTOP.y >= start.y && FindEdgeTOP.y <= end.y)*/
						{
							break;
						}

						iRetry++;
						nSetThreshold -= 10;

					} while (iRetry < nRetyMax);


					imaqDispose(pROI);

					pROI = imaqCreateROI();
					imaqAddRotatedRectContour2(pROI, imaqMakeRotatedRect(end.y - iRoiHeight, start.x, iRoiHeight, iRoiWidth, 0));

					iRetry = 0;
					nSetThreshold = nThreshold;
					do
					{
						//FindEdgeBOTTOM = IVA_FindEdge(pEdgeImage, pROI, nSetThreshold, 4, 2, 4, IMAQ_BOTTOM_TO_TOP, pEdge);
						//if (FindEdgeBOTTOM.x >= start.x && FindEdgeBOTTOM.x <= end.x && FindEdgeBOTTOM.y >= start.y && FindEdgeBOTTOM.y <= end.y)
						{
							break;
						}

						iRetry++;
						nSetThreshold -= 10;

					} while (iRetry < nRetyMax);

					imaqDispose(pROI);

					// overlay the line
					EdgeStart.x = (int)FindEdgeTOP.x;
					EdgeStart.y = (int)FindEdgeTOP.y;
					EdgeEnd.x = (int)FindEdgeBOTTOM.x;
					EdgeEnd.y = (int)FindEdgeBOTTOM.y;
					imaqOverlayLine(pTempImage, EdgeStart, EdgeEnd, &IMAQ_RGB_RED, NULL);

					if (FindEdgeTOP.x != 0 && FindEdgeTOP.y != 0 && FindEdgeBOTTOM.x != 0 && FindEdgeBOTTOM.y != 0 && abs(EdgeEnd.y - EdgeStart.y) > 30)
					{
						iGapY = ((FindEdgeTOP.y + FindEdgeBOTTOM.y) / 2) - ((pMatchInfo[iIdx].corner[3].y + pMatchInfo[iIdx].corner[0].y) / 2);
					}
				}
				//////////////////////////////////////////////////////////////////////////


				pMatchRect->left = (int)(pMatchInfo[iIdx].corner[0].x + iGapX);
				pMatchRect->top = (int)(pMatchInfo[iIdx].corner[0].y + iGapY);
				pMatchRect->right = (int)(pMatchInfo[iIdx].corner[2].x + iGapX);
				pMatchRect->bottom = (int)(pMatchInfo[iIdx].corner[2].y + iGapY);

				if (fabs(iGapX) > 0 && fabs(iGapY) > 0)
					bRet = TRUE;
			}
			break;
			case 2:
			{
				iGapX = 0.0, iGapY = 0.0;

				ROI* pROI1 = NULL;
				ROI* pROI2 = NULL;
				Rect rROI;
				int iRet = 0, iRoiLength = 0;
				iRoiWidth = (end.x - start.x);
				iRoiHeight = (end.y - start.y);

				if (iRoiHeight >= iRoiWidth)
					iRoiLength = iRoiWidth;
				else
					iRoiLength = iRoiHeight;

				nSetThreshold = nThreshold;

				////////////////////////////////////////////////////
				pROI1 = imaqCreateROI();

				imaqAddRotatedRectContour2(pROI1, imaqMakeRotatedRect(start.y, start.x, iRoiLength, iRoiLength, -90));

				//iRet = IVA_ClampMax2(pEdgeImage, pROI1, nSetThreshold, 10, 10, IMAQ_CONTOUR_TRACING,
				//	TRUE, 3, IMAQ_SEARCH_FOR_ALL_EDGES, IMAQ_SEARCH_FOR_ALL_EDGES, &FindEdgeTOP, &FindEdgeBOTTOM);

				rROI.left = (int)(start.x + 0.5);
				rROI.top = (int)(start.y + 0.5);
				rROI.width = (int)(iRoiLength + 0.5);
				rROI.height = (int)(iRoiLength + 0.5);
				imaqOverlayRect(pTempImage, rROI, &IMAQ_RGB_YELLOW, IMAQ_DRAW_VALUE, NULL);

				imaqDispose(pROI1);

				if (iRet == TRUE)
				{
					// overlay the line
					EdgeStart.x = (int)FindEdgeTOP.x;
					EdgeStart.y = (int)FindEdgeTOP.y;
					EdgeEnd.x = (int)FindEdgeBOTTOM.x;
					EdgeEnd.y = (int)FindEdgeBOTTOM.y;
					imaqOverlayLine(pTempImage, EdgeStart, EdgeEnd, &IMAQ_RGB_RED, NULL);

					if (FindEdgeTOP.x != 0 && FindEdgeTOP.y != 0 && FindEdgeBOTTOM.x != 0 && FindEdgeBOTTOM.y != 0 && abs(EdgeEnd.y - EdgeStart.y) > 30)
					{
						iGapY = ((FindEdgeTOP.y + FindEdgeBOTTOM.y) / 2) - ((pMatchInfo[iIdx].corner[3].y + pMatchInfo[iIdx].corner[0].y) / 2);
					}
				}

				if (iRet == TRUE)
				{
					////////////////////////////////////////////////////
					pROI2 = imaqCreateROI();

					imaqAddRotatedRectContour2(pROI2, imaqMakeRotatedRect(start.y, start.x, iRoiWidth, iRoiHeight, 0));

					/*iRet = IVA_ClampMax2(pEdgeImage, pROI2, nSetThreshold, 10, 10, IMAQ_CONTOUR_TRACING,
						TRUE, 3, IMAQ_SEARCH_FOR_ALL_EDGES, IMAQ_SEARCH_FOR_ALL_EDGES, &FindEdgeLEFT, &FindEdgeRIGHT);*/


					rROI.left = (int)(start.x + 0.5);
					rROI.top = (int)(start.y + 0.5);
					rROI.width = (int)(iRoiWidth + 0.5);
					rROI.height = (int)(iRoiHeight + 0.5);
					imaqOverlayRect(pTempImage, rROI, &IMAQ_RGB_GREEN, IMAQ_DRAW_VALUE, NULL);

					imaqDispose(pROI2);
				}

				if (iRet == TRUE)
				{
					// overlay the line
					EdgeStart.x = (int)FindEdgeLEFT.x;
					EdgeStart.y = (int)FindEdgeLEFT.y;
					EdgeEnd.x = (int)FindEdgeRIGHT.x;
					EdgeEnd.y = (int)FindEdgeRIGHT.y;
					imaqOverlayLine(pTempImage, EdgeStart, EdgeEnd, &IMAQ_RGB_RED, NULL);

					if (FindEdgeLEFT.x != 0 && FindEdgeLEFT.y != 0 && FindEdgeRIGHT.x != 0 && FindEdgeRIGHT.y != 0 && abs(EdgeEnd.x - EdgeStart.x) > 30)
					{
						iGapX = ((FindEdgeLEFT.x + FindEdgeRIGHT.x) / 2) - ((pMatchInfo[iIdx].corner[1].x + pMatchInfo[iIdx].corner[0].x) / 2);
					}
				}

				pMatchRect->left = (int)(pMatchInfo[iIdx].corner[0].x + iGapX);
				pMatchRect->top = (int)(pMatchInfo[iIdx].corner[0].y + iGapY);
				pMatchRect->right = (int)(pMatchInfo[iIdx].corner[2].x + iGapX);
				pMatchRect->bottom = (int)(pMatchInfo[iIdx].corner[2].y + iGapY);
				
				if (fabs(iGapX) > 0 && fabs(iGapY) > 0)
					bRet = TRUE;
			}
			break;
			case 3:
			{
				iGapX = 0.0, iGapY = 0.0;

				iRoiWidth = (end.x);
				iRoiHeight = (end.y);

				CRect rtROI(start.x, start.y, iRoiWidth, iRoiHeight);
				CFineCentering fc;
				PointFloat FindEdge;
				FindEdge.x = 0;
				FindEdge.y = 0;

				iRetry = 0;
				nSetThreshold = nThreshold;
				do
				{
					fc.SetParam_threshold(nSetThreshold);
					if (fc.Run(pTempImage, rtROI) == true)
					{
						FindEdge = fc.GetResult_FoundPoint();

						iGapX = (FindEdge.x) - ((pMatchInfo[iIdx].corner[1].x + pMatchInfo[iIdx].corner[0].x) / 2);
						iGapY = (FindEdge.y) - ((pMatchInfo[iIdx].corner[3].y + pMatchInfo[iIdx].corner[0].y) / 2);

						bRet = TRUE;
						break;
					}

					iRetry++;
					nSetThreshold -= 2;

				} while (iRetry < 3);

				pMatchRect->left = (int)(pMatchInfo[iIdx].corner[0].x + iGapX);
				pMatchRect->top = (int)(pMatchInfo[iIdx].corner[0].y + iGapY);
				pMatchRect->right = (int)(pMatchInfo[iIdx].corner[2].x + iGapX);
				pMatchRect->bottom = (int)(pMatchInfo[iIdx].corner[2].y + iGapY);
			}
			break;
			default:
			{
				pMatchRect->left = (int)(pMatchInfo[iIdx].corner[0].x + iGapX);
				pMatchRect->top = (int)(pMatchInfo[iIdx].corner[0].y + iGapY);
				pMatchRect->right = (int)(pMatchInfo[iIdx].corner[2].x + iGapX);
				pMatchRect->bottom = (int)(pMatchInfo[iIdx].corner[2].y + iGapY);

				bRet = TRUE;
			}
			break;
			}
			//			TRACE("Ptrn(%f, %f), Edge(%f, %f, %f, %f), Gap(%f, %f), \n", pMatchInfo[iIdx].corner[0].x, pMatchInfo[iIdx].corner[1].x, FindEdgeLEFT.x, FindEdgeRIGHT.x, FindEdgeTOP.y, FindEdgeBOTTOM.y, iGapX, iGapY);			 
		}
		else
		{
			if (pMatchInfo != NULL)
			{
				pMatchRect->left = (int)(pMatchInfo[iIdx].corner[0].x + 0.5);
				pMatchRect->top = (int)(pMatchInfo[iIdx].corner[0].y + 0.5);
				pMatchRect->right = (int)(pMatchInfo[iIdx].corner[2].x + 0.5);
				pMatchRect->bottom = (int)(pMatchInfo[iIdx].corner[2].y + 0.5);
			}
		}

#if _DEBUG

		PointFloat TextPoint;
		Rect r, rOrg, rROI;

		// write image for Test
		int ret = 0;
		Image* pMergeOverlay = imaqCreateImage(IMAQ_IMAGE_U8, INT_IMAGE_BORDER_SIZE);
		if (!bMeasPtrn)
		{
			if (iMethod == 1 || iMethod == 2)
			{
				imaqCopyOverlay(pTempImage, pEdgeImage, NULL);
				imaqMergeOverlay(pMergeOverlay, pTempImage, NULL, NULL, NULL);
				ret = imaqWriteJPEGFile(pMergeOverlay, "D:\\Temp\\MergeOverlay2.jpg", 750, NULL);
				if (!ret)
				{
					char* errStr = imaqGetErrorText(imaqGetLastError());
					TRACE("%s(%i) : %s\n", __FILE__, __LINE__, errStr);
					imaqDispose(errStr);
				}
			}
		}

		imaqClearOverlay(pTempImage, NULL);

		rOrg.left = (int)(pMatchInfo[iIdx].corner[0].x + 0.5);
		rOrg.top = (int)(pMatchInfo[iIdx].corner[0].y + 0.5);
		rOrg.width = (int)((pMatchInfo[iIdx].corner[2].x - pMatchInfo[iIdx].corner[0].x) + 0.5);
		rOrg.height = (int)((pMatchInfo[iIdx].corner[2].y - pMatchInfo[iIdx].corner[0].y) + 0.5);

		imaqOverlayRect(pTempImage, rOrg, &IMAQ_RGB_WHITE, IMAQ_DRAW_VALUE, NULL);

		r.left = pMatchRect->left;
		r.top = pMatchRect->top;
		r.width = pMatchRect->Width();
		r.height = pMatchRect->Height();
		imaqOverlayRect(pTempImage, r, &IMAQ_RGB_RED, IMAQ_DRAW_VALUE, NULL);

		TextPoint.x = r.left + (r.width / 2);
		TextPoint.y = r.top + (r.height / 2);

		int iWide = 10;
		Point pt1, pt2;
		pt1.x = (int)(TextPoint.x - iWide);
		pt2.x = (int)(TextPoint.x + iWide);
		pt1.y = pt2.y = (int)(TextPoint.y);
		imaqOverlayLine(pTempImage, pt1, pt2, &IMAQ_RGB_RED, NULL);

		pt2.x = pt1.x = (int)(TextPoint.x);
		pt1.y = (int)(TextPoint.y - iWide);
		pt2.y = (int)(TextPoint.y + iWide);
		imaqOverlayLine(pTempImage, pt1, pt2, &IMAQ_RGB_RED, NULL);

		// write image for Test
//		int ret = 0;
//		Image* pMergeOverlay = imaqCreateImage(IMAQ_IMAGE_U8, INT_IMAGE_BORDER_SIZE);
		imaqMergeOverlay(pMergeOverlay, pTempImage, NULL, NULL, NULL);
		ret = imaqWriteJPEGFile(pMergeOverlay, "D:\\Temp\\MergeOverlay.jpg", 750, NULL);
		if (!ret)
		{
			char* errStr = imaqGetErrorText(imaqGetLastError());
			TRACE("%s(%i) : %s\n", __FILE__, __LINE__, errStr);
			imaqDispose(errStr);
		}

		IMAQ_DISPOSE(pMergeOverlay);
#endif

		IMAQ_DISPOSE(pEdgeImage);

		IMAQ_DISPOSE(pTempImage);
	}

	// Cleans up resources associated with the object
	IMAQ_DISPOSE(roi);

	IMAQ_DISPOSE(pMatchInfo);

	if (bRet == TRUE && iMatchMethod == MATCH_POSITION)
	{
		if (bPositionSearch)
			return TRUE;
		else
			return FALSE;
	}

	if (bRet != TRUE)
	{
		pMatchRect->left = 0;
		pMatchRect->top = 0;
		pMatchRect->right = 0;
		pMatchRect->bottom = 0;
	}

	return bRet;
}

float CIMAQImageCtrl::CalcAngle2(PointFloat DPonitFStart, PointFloat DPonitFEnd, PointFloat NPonitFStart, PointFloat NPonitFEnd)
{
	int success = 1;
	float fRet = 0;

	success = imaqGetAngle(DPonitFStart, DPonitFEnd, NPonitFStart, NPonitFEnd, &fRet);
	if (success == 0)
		fRet = -1;

	return fRet;
}