#if !defined(AFX_IMAQIMAGECTRL_H__E171F082_7B51_48AD_B3D8_1D96353E0928__INCLUDED_)
#define AFX_IMAQIMAGECTRL_H__E171F082_7B51_48AD_B3D8_1D96353E0928__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// IMAQImageCtrl.h : header file
//
#include "IMAQImage.h"
#include <nivision.h>

/////////////////////////////////////////////////////////////////////////////
// CIMAQImageCtrl window
#define DEFAULT_MIN_LINE_SCORE			990
#define DEFAULT_PIXEL_RADIUS			3
#define DEFAULT_REFINEMENTS				0
#define DEFINE_IMAQ_ZOOM_MAX			16.f

#define EVENT_WNDMOVE					1
#define EVENT_CLICK						2
#define EVENT_DRAW						3
#define EVENT_MOUSE_MOVE				4
#define EVENT_KEYBOARD                  5
#define EVENT_DOUBLE_CLICK              6
#define IMAQ_PI							3.1415926535897932384626433832795028841971
#define ROUND_TO_INT(x)					((x > 0) ? (int)(x + 0.5) : (int)(x - 0.5))

#define UWM_IMAQIMAGE_MOUSE_EVENT		WM_USER + 2100  // 마우스 이벤트 후킹용 message
#define UWM_IMAQIMAGE_MOUSE_POS_DISP	WM_USER + 2102  // 마우스 포지션 Display message

#define INT_IMAGE_BORDER_SIZE 7
#define	INT_MONI_WND_HEIGHT		2048
#define	INT_MONI_WND_WIDTH		2560
#define WM_USER                 0x0400

#define IMAQ_DISPOSE(p)	{if(p) { imaqDispose(p); p = NULL; }}


struct stIMAQCTRL_ZOOM_PARAM
{
    Point ptZoomCenter;
    float fZoomRate;

	stIMAQCTRL_ZOOM_PARAM()
	{
		ptZoomCenter = imaqMakePoint(0, 0);
		fZoomRate = 1.f;
	}
};

enum eIMAQCTRL_MOUSE_HANDLE  // Zoom 마우스 이벤트 후킹 용 enum
{
    eIMAQCTRL_MOUSE_HANDLE_MOVE = 0,
    eIMAQCTRL_MOUSE_HANDLE_LCLICK,
    eIMAQCTRL_MOUSE_HANDLE_LDOWN,
    eIMAQCTRL_MOUSE_HANDLE_LDOWN_MOVE,
	eIMAQCTRL_MOUSE_HANDLE_LDOWN_DRAW,
    eIMAQCTRL_MOUSE_HANDLE_LUP,
    eIMAQCTRL_MOUSE_HANDLE_RDOWN,
    eIMAQCTRL_MOUSE_HANDLE_RUP,
    eIMAQCTRL_MOUSE_HANDLE_LDCLICK,
    eIMAQCTRL_MOUSE_HANDLE_WHEEL_UP,
    eIMAQCTRL_MOUSE_HANDLE_WHEEL_DOWN,
    eIMAQCTRL_MOUSE_HANDLE_END,
};

enum eIMAQCTRL_MOUSE_FUNC  // Zoom Sate 용 enum
{
    eIMAQCTRL_MOUSE_FUNC_DRAW_RECT = 0,
    eIMAQCTRL_MOUSE_FUNC_ZOOM,
    eIMAQCTRL_MOUSE_FUNC_PANNING,
	eIMAQCTRL_MOUSE_FUNC_END,
};

enum eNI_OVERLAY_LAYER
{
    eNI_OVERLAY_LAYER_NONE = -1,
    eNI_OVERLAY_LAYER_LP_AVR = 0,
    eNI_OVERLAY_LAYER_LP_EDGE,
    eNI_OVERLAY_LAYER_AREA,
    eNI_OVERLAY_LAYER_CD,
    eNI_OVERLAY_LAYER_RGB,
    eNI_OVERLAY_LAYER_PS,
    eNI_OVERLAY_LAYER_PSCS,
    eNI_OVERLAY_LAYER_TF,
    eNI_OVERLAY_LAYER_OVERLAYCD,
    eNI_OVERLAY_LAYER_AREAMAX,
    eNI_OVERLAY_LAYER_THICK,
    eNI_OVERLAY_LAYER_STEP_HEIGHT,
	eNI_OVERLAY_LAYER_GRID,
	eNI_OVERLAY_LAYER_CALIBRATION,
	eNI_OVERLAY_LAYER_EDIT_RECIPE,
	eNI_OVERLAY_LAYER_GENERAL_OPT,
    eNI_OVERLAY_LAYER_END,
};

enum eROI_TYPE
{
    eROI_TYPE_RECT = 0,
    eROI_TYPE_LINE,
    eROI_TYPE_POINT,
    eROI_TYPE_ROTATEDRECT,
    eROI_TYPE_ANNULUS,
	eROI_TYPE_END,
};

enum eIMAQ_DISPLAY_ZOOM
{
    eIMAQ_DISPLAY_ZOOM_1X = 0,
    eIMAQ_DISPLAY_ZOOM_HALF,
    eIMAQ_DISPLAY_ZOOM_QUARTER,
    eIMAQ_DISPLAY_ZOOM_NONE,
};

enum _ptrn_match_type
{
	MATCH_SCORE = 0,
	MATCH_DISTANCE,
	MATCH_POSITION,
	MATCH_TYPE_END,
};

enum _roiwnd_event_type
{
	UWM_ROIWND_EVENT_CLICK = WM_USER + 4100,
	UWM_ROIWND_EVENT_DRAW,
	UWM_ROIWND_EVENT_MOVE,
	UWM_ROIWND_EVENT_END,
};

//typedef struct IVA_EDGE_Struct
//{
//    CPoint pPreEdge[2];
//    double dThreshold;
//} IVA_EDGE;

class CIMAQImageCtrl : public CStatic
{
    HWND					m_hImaqWindowHandle;
    int						m_iImaqWindowNumber;
    BOOL					m_bImaqWindowCreate;
    CWnd					m_wSub;
    Image					*m_pImage_onlylink;
    Image					*m_pImgDefault;
    Image					*m_pOverlay[eNI_OVERLAY_LAYER_END];
    stIMAQCTRL_ZOOM_PARAM	m_stZoomParam;
    stIMAQCTRL_ZOOM_PARAM	m_stPanningParam;
    Tool_enum				m_ePreviousTool; // Zoom 시 현재 Tool 저장 저장 용. ROI 설정시 복구
    BOOL					m_bDrawRect; // Zoom 이나 Panning 상태로 변경 시 Tool 을 저장 할 것인지 확인 용
    CArray<stIMAQCTRL_ZOOM_PARAM, stIMAQCTRL_ZOOM_PARAM> m_arrayZoomList; // 이전 줌 상태 저장 용 Array

    // keyboard & mouse hook...
public:
    HWND	m_hWndUser;
    HWND	m_hWndParent;
    bool    m_bBreakMsg;
    HHOOK   m_hKeyHook;
    HHOOK   m_hMouseHook;
    BOOL    HookStart();
    BOOL    HookEnd();

public:
    HWND GetWndHandle() { return m_hImaqWindowHandle; }
    int	 GetWndNumber() { return m_iImaqWindowNumber; }
    BOOL IMAQWindowCreate();
    int	 SetWindowSize(int cx, int cy);
    void SetCurrentTool(Tool currTool);
    void SetUserWindow(CWnd *pWnd);
    void ShowScrollBar(BOOL bShow);
    void EnableEvent(BOOL bflag);
    static void		OnEventCallbackIMAQCtrl(WindowEventType iType, int windowNumber, Tool tool, Rect rect);
	static void		MakeScaledImage(Image* pOrig, Image* pScaled, CRect* pRect);
	
public:
    void SetZoom(eIMAQ_DISPLAY_ZOOM eZoom);
    void SetZoom(float fAxisRatio, int iCntX = -1, int iCnty = -1);
    void SetRepaint();
    void SetImage(Image *pImage);
	void SetImage(int index, Image *pImage);
    void DisplayImage();
    void SaveImage(CString sFileName, BOOL bIncludeOverlay = TRUE);
	BOOL LoadPattern(CString strFileName, Image* pDest);
    void ImageToClipboard(BOOL bIncludeOverlay = FALSE);
    void SetStartPanning();
    void SetStartZoom();
    void SetStartDrawRect();
    void ZoomWindow(CPoint cpStart, CPoint cpEnd);
    void ZoomPrevious();
    void ZoomByWheel(BOOL bUp);
    void ZoomAll();
	BOOL PopupROIDefine2(CWnd* pParent, int uiID, Image* pImage, CRect rectOld = CRect(0, 0, 0, 0), BOOL bDis = TRUE);
	void SetROIRect(CRect rectNew);
	void GetPatternImageAndRect(Image* pPtrnImg, CRect* prectROI);
private:
    void CastToU8(Image *pDestU8, Image *pSrc_U16_SGL);
    CPoint TransOriginWindowCoord(CPoint point, stIMAQCTRL_ZOOM_PARAM stZoomParam);

public:
    void ClearGrid();
    void SetGrid(double dUnitLenght_nm, double dStepSize_um, COLORREF color, int iLineType);
    void ClearROI();
    void SetROI(void *vROI, eROI_TYPE tType);
    ROI *GetROI();
	void ClearOverlayAll();
	void ClearOverlay(eNI_OVERLAY_LAYER layer);
    void ClearOverlay(Image *imgOverTarget);
    void OverlayRect(Image *imgOverTarget, Rect r, COLORREF color, unsigned char ucAlpha = 0, DrawMode drawMode = IMAQ_HIGHLIGHT_VALUE);
    void OverlayRotatedRect(Image *imgOverTarget, RotatedRect rr, COLORREF color, unsigned char ucAlpha = 0, DrawMode drawMode = IMAQ_HIGHLIGHT_VALUE);
    void OverlayLine(Image *imgOverTarget, Point pStart, Point pEnd, COLORREF color, unsigned char ucAlpha = 0);
    void OverlayPoint(Image *imgOverTarget, Point pos, COLORREF color, unsigned char ucAlpha = 0, int iEmphasis = 0, COLORREF cEmphasisColor = RGB(255, 0, 0));
    void OverlayText(Image *imgOverTarget, Point pos, CString sMsg, COLORREF color, unsigned char ucAlpha = 0);
	void OverlayText2(eNI_OVERLAY_LAYER layer, Point pos, CString sMsg, COLORREF color, unsigned char ucAlpha = 0);
    void OverlayText2(Image *imgOverTarget, Point pos, CString sMsg, COLORREF color, unsigned char ucAlpha = 0);
    void OverlayAnnulus(Image *imgOverTarget, Annulus Ann, COLORREF color, unsigned char ucAlpha = 0);
    void OverlayDottedLine(Image *imgOverTarget, Point pStart, Point pEnd, int nDot, COLORREF color, unsigned char ucAlpha = 0);
    void OverlayDottedRect(Image *imgOverTarget, Rect rect, int nLinePerDot, COLORREF color, unsigned char ucAlpha = 0);
    void OverlayOval(Image *imgOverTarget, Rect rect, COLORREF color, unsigned char ucAlpha = 0, DrawMode drawMode = IMAQ_DRAW_VALUE);

// Pattern
    static BOOL     PtrnLearning(Image* pPtrnImg);
    static float	GetDistancePtrnToCntr(CPoint pt, int iPgmMode, int iW = INT_MONI_WND_WIDTH, int iH = INT_MONI_WND_HEIGHT);
    BOOL SearchPattern(Image* pImg, Image* pPtrnImg, float minMatchScore, BOOL bLearn, int iMatchMethod, int bMeasPtrn,
                       CRect* pMatchRect /*output*/, float* pfScore /*output*/, int nMatchPositionX = 0, int nMatchPositionY = 0, int iMethod = 0, int nThreshold = 30);

    float CalcAngle2(PointFloat DPonitFStart, PointFloat DPonitFEnd, PointFloat NPonitFStart, PointFloat NPonitFEnd);

    // Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CIMAQImageCtrl)
protected:
    virtual void PreSubclassWindow();
	void		MakePopupWnd(CWnd* pParent, UINT uiID);
	void		MakeROITool(BOOL bShowToolWnd = FALSE);
	void		SetROIToolOption(Tool useTool);
	static void CBEventROIWnd(WindowEventType eventType, int windowNumber, Tool tool, Rect rect);
    //}}AFX_VIRTUAL

public:
    CIMAQImageCtrl();
    virtual ~CIMAQImageCtrl();
	void	DestroyMe();
	BOOL m_bHSLType;

protected:
	BOOL		m_bSetSend;
	CWnd*		m_pHostWnd;
	CWnd*		m_pParent;
	CIMAQImage*	m_pimaqWnd;
	Image*		m_pBaseImage;
	Image*		m_pOverlayImage;
	Image*		m_pPtrnImgBW;
	ROI*		m_pROI;
	CRect		m_rectROI;
    //{{AFX_MSG(CIMAQImageCtrl)
    //}}AFX_MSG

protected:
    DECLARE_MESSAGE_MAP()
    afx_msg void OnPaint();
    afx_msg void OnClose();
    afx_msg void OnDestroy();
    virtual BOOL PreTranslateMessage(MSG *pMsg);
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_IMAQIMAGECTRL_H__E171F082_7B51_48AD_B3D8_1D96353E0928__INCLUDED_)
