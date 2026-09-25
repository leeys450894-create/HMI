#pragma once

#include "MainSubDlgWin.h"
#include "nivision.h"

// CDlgAlignCCD 대화 상자입니다.
class CAuroraMgr;
class CIMAQImageCtrl;
class CD_CCD_Review;
class CDlgAlignCCD : public CMainSubDlgWin
{
	DECLARE_DYNAMIC(CDlgAlignCCD)

private:
	CAuroraMgr				*m_pAuroraMgr;
	HWND					m_hMoniImage;		// Handle for monitoring image window
//	HWND					m_hMoniImage2;		// Handle for monitoring image window

	HANDLE		m_hEvtKillThread;       //set by UI		쓰레드를 강제로 죽이는 핸들
	HANDLE		m_hEvtThreadKilled;     //set by self	쓰레드가 죽었다는 핸들

	CD_CCD_Review *m_pCCDReview;
	
	static UINT WorkThreadProbeProc(LPVOID pParam);
	void BeginThread_WorkProc();
	
	int WaitTerminateWorkerThread();
	CWinThread* m_pWorkThread;
	int m_iMoniImageWnd;	// Image window number
//	int m_iMoniImageWnd2;	// Image window number
	Image* m_pAqcImage;
	Image* m_pLiveImage;
	int m_iCCDNum;
	int m_iGrabSizeW;
	int m_iGrabSizeH;
	virtual BOOL OnInitDialog();
	void DisplayImage();
	void DisplayImage2();
	void GrabImage();
	long m_nGrabNgCount;
	bool m_bGrabFail;			// LYS 2026-09-07 라이브 그랩 실패 진행 중 (전이 시점 로그용)
	bool m_bResetFailReported;	// LYS 2026-09-07 ResetGrabber 실패 로그 1회 게이트
	bool m_bReapplyZoom;			// LYS 2026-09-07 재연결 후 첫 표시 때 시작 시 배율/창크기 재적용
	int  m_nReapplyFitCount;		// LYS 2026-09-11 RESET 후 표시 크기 재적용을 남은 프레임 수만큼 반복(그랩 안정화 전 1회만 하면 놓친다)
	bool m_bReconnecting;			// LYS 2026-09-07 재연결 중 라이브 스레드 스킵

	CIMAQImageCtrl* m_pimaqctrlROI;

	static void CBEventROIWnd(WindowEventType eventType, int windowNumber, Tool tool, Rect rect);

public:
	CDlgAlignCCD(CAuroraMgr *pAuroraMgr, int nNum, CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~CDlgAlignCCD();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DLG_SUB_CCD };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	LRESULT OnMsgDisplayImage(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
public:
	int CCDDisplayDlg(CWnd* pHostWnd, CRect rect);
	int DoWorkStartGrab();
	int TerminateWorkerThread();
	void ShowCrossLine(Image* pImage);
	void OverlayText(Image* pImage);
	int m_iFocus;
	int m_nLiveTick;
	bool m_bShowCrossLine;
	void ShowImage2(bool bShow);
	float m_fScale;	// LYS 2026-09-11 가로 배율 (세로와 다를 수 있어 이름만 유지)
	float m_fScaleY;	// LYS 2026-09-11 세로 배율 - CCD 자리를 꽉 채우려면 가로/세로를 따로 준다
	void ApplyLiveViewFit();	// LYS 2026-09-11 Live 표시를 CCD 자리(IDC_STATIC_POSITION_CCDn) Rect 크기에 맞춘다
	bool m_bShowImage2;
	int SetCCDLive(bool bLive);
	int ReconnectCCD();	// LYS 2026-09-07 CCD RESET 버튼용: 재오픈/재접속 + 라이브 ON
	bool m_bCCDLive;
};
