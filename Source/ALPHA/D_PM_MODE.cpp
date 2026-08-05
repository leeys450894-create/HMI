// D_PM_MODE.cpp : 구현 파일입니다.
//

#include "pch.h"
#include "ALPHA.h"
#include "D_PM_MODE.h"
#include "afxdialogex.h"

//#include "Manager\IOMapControlMgr.h"
#include "Manager/MaintControlMgr.h"
#include "Manager\IOMapAddressDefine.h"
#include "D_InputPassWord.h"
#include "CD_PopUp_Motion_Move.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CD_PM_MODE 대화 상자입니다.
#define      MAINT_POPUP_INFOR_TIMER_ID			7001

typedef BOOL(WINAPI *SetLayer)(HWND hWnd, COLORREF crKey, BYTE bAlpha, DWORD dwFlags);


IMPLEMENT_DYNAMIC(CD_PM_MODE, CDialogEx)

CD_PM_MODE::CD_PM_MODE(CMaintControlMgr* pMaintControlMgr, CLogInControlMgr *pLogInControlMgr, CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_POPUP_PM_MODE, pParent)
	, m_pMaintControlMgr(pMaintControlMgr)
	, m_pLogInControlMgr(pLogInControlMgr)
	, m_nNGCount(0)
{
	m_hbitmapImage = NULL;
}

CD_PM_MODE::~CD_PM_MODE()
{
	if (m_pMotionMove != nullptr)
	{
		m_pMotionMove->DestroyWindow();
		delete m_pMotionMove;
		m_pMotionMove = nullptr;
	}

	if(m_hbitmapImage)
	{
		::DeleteObject(m_hbitmapImage);
	}
}

void CD_PM_MODE::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_WARNING_MESSAGE_IMAGE,	m_stPModeImage);
	DDX_Control(pDX, IDC_STATIC_DOOR_STATE, m_statusInfo[0]);
	DDX_Control(pDX, IDC_STATIC_MUTING_STATE, m_statusInfo[1]);
}


BEGIN_MESSAGE_MAP(CD_PM_MODE, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON_DOOR_LOCK,		&CD_PM_MODE::OnBnClickedButtonDoorLock)
	ON_BN_CLICKED(IDC_BUTTON_DOOR_UNLOCK,	&CD_PM_MODE::OnBnClickedButtonDoorUnlock)
	ON_WM_TIMER()
	ON_WM_DESTROY()
	ON_WM_LBUTTONDBLCLK()
	ON_BN_CLICKED(IDC_BUTTON_BUZZER_OFF, &CD_PM_MODE::OnBnClickedButtonBuzzerOff)
	ON_BN_CLICKED(IDC_BUTTON_MUTING_ON, &CD_PM_MODE::OnBnClickedButtonMutingOn)
END_MESSAGE_MAP()


// CD_PM_MODE 메시지 처리기입니다.


BOOL CD_PM_MODE::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  여기에 추가 초기화 작업을 추가합니다.
	m_hbitmapImage = ::LoadBitmap(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_BITMAP_WARNING_MESSAGE));

	HBITMAP h_old_bitmap = m_stPModeImage.SetBitmap(m_hbitmapImage);

	if(h_old_bitmap)
	{
		::DeleteObject(h_old_bitmap);
	}

	CRect rtROI;
	GetDlgItem(IDC_STATIC_WARNING_MESSAGE)->GetWindowRect(&rtROI);
	ScreenToClient(&rtROI);
	m_stPModeImage.MoveWindow(rtROI);
	m_stPModeImage.Invalidate(FALSE);

	HMODULE hUser32 = GetModuleHandle(_T("USER32.DLL"));
	SetLayer pSetLayer = (SetLayer)GetProcAddress(hUser32, "SetLayeredWindowAttributes");


	m_pMotionMove = new CD_PopUp_Motion_Move(m_pMaintControlMgr);
	m_pMotionMove->Create(IDD_PUPUP_MOTION_MOVE, this);
	m_pMotionMove->ShowWindow(SW_HIDE);

	m_statusInfo[0].SetBorder(TRUE, TRUE);
	m_statusInfo[0].SetBkColor(RGB(0, 32, 0));
	m_statusInfo[0].SetTextColor(RGB(192, 255, 255));
	m_statusInfo[0].SetFontBold(TRUE);
	m_statusInfo[0].SetText(_T("DOOR"));

	m_statusInfo[1].SetBorder(TRUE, TRUE);
	m_statusInfo[1].SetBkColor(RGB(0, 32, 0));
	m_statusInfo[1].SetTextColor(RGB(192, 255, 255));
	m_statusInfo[1].SetFontBold(TRUE);
	m_statusInfo[1].SetText(_T("MUTING"));

	if (pSetLayer == NULL)
		return FALSE;

	int alphaValue = 150; //투명도 설정 0 ~ 255 ( 0은 완전투명 )
	SetWindowLong(this->m_hWnd, GWL_EXSTYLE,
		GetWindowLong(this->m_hWnd, GWL_EXSTYLE) | 0x80000);
	pSetLayer(this->m_hWnd, 0, alphaValue, LWA_ALPHA);

	SetTimer(MAINT_POPUP_INFOR_TIMER_ID, 1000, NULL);
	SetTimer(MAINT_POPUP_INFOR_TIMER_ID +2, 1000, NULL);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}


BOOL CD_PM_MODE::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN)
	{
		// To prevent to close a dialog.
		switch (pMsg->wParam)
		{
			case VK_ESCAPE:
			case VK_RETURN:
			case VK_SPACE:
			{
				CD_InputPassWord dlg(m_pLogInControlMgr);				
				if (dlg.DoModal() == IDOK)
				{
					SetTimer(MAINT_POPUP_INFOR_TIMER_ID + 1, 500, NULL);
				}
				{
				    CLogWrapper::GetInstance()->KMLOG(OPER, _T("The password is wrong of PM_MODE or Empty"));
				}

				return TRUE;
			}
		}
	}

	return CDialogEx::PreTranslateMessage(pMsg);
}

void CD_PM_MODE::OnBnClickedButtonDoorLock()
{
    CLogWrapper::GetInstance()->KMLOG(OPER, _T("CD_PM_MODE::OnBnClickedButtonDoorLock"));

	// jsyoon 2022.05.23 add
	int nBit = 0;
	nBit = m_pMaintControlMgr->GetIOMapControlMgrPtr()->GetParam_DIOMapMemoryBitImport(PLC_BIT_SUTTER_SENSOR_CLOSE);
	if (nBit != -1)
	{
		if (nBit != 1)
		{
			AfxMessageBox(_T("Shtter를 확인하세요."));

			return;
		}
	}

	bool bDoorOpen = false;
	for (int i = 0; i < 8; i++)
	{
		int nAddress = PLC_BIT_DOOR_SENSOR1 + i;
		nBit = m_pMaintControlMgr->GetIOMapControlMgrPtr()->GetParam_DIOMapMemoryBitImport(nAddress);
		if (nBit != 1)
		{
			if (nBit == 1)
				bDoorOpen |= true;
		}
	}

	if (bDoorOpen == true)
	{
		AfxMessageBox(_T("Door Open을 확인하세요."));

		return;
	}

	CD_InputPassWord dlg(m_pLogInControlMgr);
	if (dlg.DoModal() != IDOK)
	{
		AfxMessageBox(_T("비밀번호를 확인하세요."));
		CLogWrapper::GetInstance()->KMLOG(OPER, _T("The password is wrong of PM_MODE"));

		return;
	}

#ifdef _USE_IOBOARD
	m_pIOMapControlMgr->SetMemoryBitToDevice(IO_BOARD_READ_OUTPUT_012, false);
#else
	m_pMaintControlMgr->GetIOMapControlMgrPtr()->SetMemoryBitToDevice(PLC_BIT_DOOR_UNLOCK, false);

#endif // _USE_IOBOARD
}

void CD_PM_MODE::OnBnClickedButtonDoorUnlock()
{
    CLogWrapper::GetInstance()->KMLOG(OPER, _T("CD_PM_MODE::OnBnClickedButtonDoorUnlock"));

	// jsyoon 2022.05.23 add
	int nBit = 0;
	nBit = m_pMaintControlMgr->GetIOMapControlMgrPtr()->GetParam_DIOMapMemoryBitImport(PLC_BIT_SUTTER_SENSOR_CLOSE);
	if (nBit != -1)
	{
		if (nBit != 1)
		{
			AfxMessageBox(_T("Shtter를 확인하세요."));

			return;
		}
	}

	CD_InputPassWord dlg(m_pLogInControlMgr);
	if (dlg.DoModal() != IDOK)
	{
		AfxMessageBox(_T("비밀번호를 확인하세요."));
		CLogWrapper::GetInstance()->KMLOG(OPER, _T("The password is wrong of PM_MODE"));

		return;
	}

#ifdef _USE_IOBOARD
	m_pIOMapControlMgr->SetMemoryBitToDevice(IO_BOARD_READ_OUTPUT_012, true);
#else
	m_pMaintControlMgr->GetIOMapControlMgrPtr()->SetMemoryBitToDevice(PLC_BIT_DOOR_UNLOCK, true);
#endif // _USE_IOBOARD
}

void CD_PM_MODE::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	if (nIDEvent == MAINT_POPUP_INFOR_TIMER_ID)		
	{
		if (m_pMaintControlMgr->GetIOMapControlMgrPtr() != NULL)
		{
			BOOL bDoorUnlock = FALSE; 

			// jsyoon 2022.06.21 add
			if (m_pMaintControlMgr->GetIOMapControlMgrPtr()->GetParam_DIOMapMemoryBitImport(PLC_BIT_DOOR_UNLOCK01) == true)
			{
				m_statusInfo[0].SetTextColor(RGB(255, 0, 0));
				m_statusInfo[0].SetText(_T("DOOR UNLOCK"));
	//			GetDlgItem(IDC_STATIC_DOOR_STATE)->SetWindowText(_T("DOOR UNLOCK"));
			}
			else
			{
				m_statusInfo[0].SetTextColor(RGB(192, 255, 255));
				m_statusInfo[0].SetText(_T("DOOR LOCK"));
	//			GetDlgItem(IDC_STATIC_DOOR_STATE)->SetWindowText(_T("DOOR LOCK"));
			}

			if (m_pMaintControlMgr->GetIOMapControlMgrPtr()->GetParam_DIOMapMemoryBitExport(PLC_BIT_W_MUTIONG_ON) == true)
			{
				m_statusInfo[1].SetTextColor(RGB(255, 0, 0));
				m_statusInfo[1].SetText(_T("MUTING ON"));
	//			GetDlgItem(IDC_STATIC_MUTING_STATE)->SetWindowText(_T("MUTING ON"));
			}
			else
			{
				m_statusInfo[1].SetTextColor(RGB(192, 255, 255));
				m_statusInfo[1].SetText(_T("MUTING OFF"));
	//			GetDlgItem(IDC_STATIC_MUTING_STATE)->SetWindowText(_T("MUTING OFF"));
			}
			

			// Teach Key
#ifdef _USE_IOBOARD
			if (m_pIOMapControlMgr->GetParam_DIOMapMemoryBitImport(IO_BOARD_READ_INPUT_102) == true)
#else	
			if (m_pMaintControlMgr->GetIOMapControlMgrPtr()->GetParam_DIOMapMemoryBitImport(PLC_BIT_RUN_TEACH_SWITCH) == true)
#endif // _USE_IOBOARD
			{
				KillTimer(MAINT_POPUP_INFOR_TIMER_ID);
				KillTimer(MAINT_POPUP_INFOR_TIMER_ID + 2);

				m_pMaintControlMgr->GetIOMapControlMgrPtr()->SetMemoryBitToDevice(PLC_BIT_W_MUTIONG_ON, false);

				CDialog::OnOK();
			}
		}
	}
	else if (nIDEvent == MAINT_POPUP_INFOR_TIMER_ID+1)
	{
		m_nNGCount++;

		if (m_nNGCount > 100)
		{
			m_nNGCount = 0;

			KillTimer(MAINT_POPUP_INFOR_TIMER_ID+1);

			AfxMessageBox(_T("Door lock Fail"));
		}

		// Door Unlock
		if (m_pMaintControlMgr->GetIOMapControlMgrPtr() != NULL)
		{
#ifdef _USE_IOBOARD
			if (m_pIOMapControlMgr->GetParam_DIOMapMemoryBitImport(IO_BOARD_READ_OUTPUT_012) == true)
#else	
			if (m_pMaintControlMgr->GetIOMapControlMgrPtr()->GetParam_DIOMapMemoryBitImport(PLC_BIT_DOOR_UNLOCK01) == true)
#endif // _USE_IOBOARD
			{
				m_nNGCount = 0;
				KillTimer(MAINT_POPUP_INFOR_TIMER_ID);
				KillTimer(MAINT_POPUP_INFOR_TIMER_ID + 1);
				KillTimer(MAINT_POPUP_INFOR_TIMER_ID + 2);

				m_pMaintControlMgr->GetIOMapControlMgrPtr()->SetMemoryBitToDevice(PLC_BIT_W_MUTIONG_ON, false);

				CLogWrapper::GetInstance()->KMLOG(OPER, _T("PM_MODE Close"));

				CDialog::OnOK();
			}
		}
	}
	else if (nIDEvent == MAINT_POPUP_INFOR_TIMER_ID + 2)
	{
		// Door Unlock
		if (m_pMaintControlMgr->GetIOMapControlMgrPtr() != NULL)
		{
			if (m_pMaintControlMgr->GetIOMapControlMgrPtr()->GetParam_DIOMapMemoryBitImport(PLC_BIT_HAND_GRIP) == true)
			{
				m_pMotionMove->ShowWindow(SW_SHOW);
		//		this->ShowWindow(SW_HIDE);
			}
			else
			{
				m_pMotionMove->ShowWindow(SW_HIDE);
		//		this->ShowWindow(SW_SHOW);
			}
		}
	}

	CDialogEx::OnTimer(nIDEvent);
}


void CD_PM_MODE::OnDestroy()
{
	CDialogEx::OnDestroy();

	// TODO: 여기에 메시지 처리기 코드를 추가합니다.
	KillTimer(MAINT_POPUP_INFOR_TIMER_ID);
	KillTimer(MAINT_POPUP_INFOR_TIMER_ID + 1);
	KillTimer(MAINT_POPUP_INFOR_TIMER_ID + 2);

}


void CD_PM_MODE::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	CD_InputPassWord dlg(m_pLogInControlMgr);
	if (dlg.DoModal() == IDOK)
	{
		GetDlgItem(IDC_BUTTON_DOOR_LOCK)->EnableWindow(TRUE);
		GetDlgItem(IDC_BUTTON_DOOR_UNLOCK)->EnableWindow(TRUE);
		SetTimer(MAINT_POPUP_INFOR_TIMER_ID + 1, 500, NULL);
	}
	else
	{
		AfxMessageBox(_T("비밀번호를 확인하세요."));
		CLogWrapper::GetInstance()->KMLOG(OPER, _T("The password is wrong of PM_MODE"));
	}

	CDialogEx::OnLButtonDblClk(nFlags, point);
}


void CD_PM_MODE::OnBnClickedButtonBuzzerOff()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CLogWrapper::GetInstance()->KMLOG(OPER, _T("CD_PM_MODE::OnBnClickedButtonBuzzerOff"));

#ifdef _USE_IOBOARD
	m_pIOMapControlMgr->SetMemoryBitToDevice(IO_BOARD_READ_OUTPUT_000, false);
#else
	m_pMaintControlMgr->GetIOMapControlMgrPtr()->SetMemoryBitToDevice(PLC_BIT_EQ_BUZZER_01, false);
#endif // _USE_IOBOARD

}


void CD_PM_MODE::OnBnClickedButtonMutingOn()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CLogWrapper::GetInstance()->KMLOG(OPER, _T("CD_PM_MODE::OnBnClickedButtonMutingOn"));

	m_pMaintControlMgr->GetIOMapControlMgrPtr()->SetMemoryBitToDevice(PLC_BIT_W_MUTIONG_ON, true);

}
