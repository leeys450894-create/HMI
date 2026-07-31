// D_RoiSet.cpp: 구현 파일
//

#include "pch.h"
#include "ALPHA.h"
#include "D_RoiSet.h"
#include "afxdialogex.h"
#include "common\IMAQImageCtrl\IMAQImageCtrl.h"
#include "common/CommonFunc.h"

// CD_RoiSet 대화 상자

IMPLEMENT_DYNAMIC(CD_RoiSet, CDialogEx)

CD_RoiSet::CD_RoiSet(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_PUPUP_ROI_SET, pParent)
{
	m_iCallingView = VI_PTRN_MEAS;
	m_pimaqctrlROI = nullptr;
	m_iWIDTH = INT_MONI_WND_WIDTH;
	m_iHEIGHT = INT_MONI_WND_HEIGHT;
}

CD_RoiSet::~CD_RoiSet()
{
}

void CD_RoiSet::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_ROI_START_X, m_editY);
	DDX_Control(pDX, IDC_EDIT_ROI_START_Y, m_editX);
	DDX_Control(pDX, IDC_EDIT_ROI_SIZE_W, m_editW);
	DDX_Control(pDX, IDC_EDIT_ROI_SIZE_H, m_editH);
	DDX_Control(pDX, IDC_STC_ROI_TITLE, m_LabelInfo);
}


BEGIN_MESSAGE_MAP(CD_RoiSet, CDialogEx)
	ON_BN_CLICKED(IDC_BTN_APPLY_ROI, &CD_RoiSet::OnBnClickedBtnApplyRoi)
	ON_BN_CLICKED(IDC_BTN_CANCEL_ROI, &CD_RoiSet::OnBnClickedBtnCancelRoi)
	ON_BN_CLICKED(IDC_BTN_SAVE_IMAGE, &CD_RoiSet::OnBnClickedBtnSaveImage)
	ON_MESSAGE(UWM_ROIWND_EVENT_CLICK, &CD_RoiSet::OnROIEvent_Click)
	ON_MESSAGE(UWM_ROIWND_EVENT_DRAW, &CD_RoiSet::OnROIEvent_Draw)
	ON_MESSAGE(UWM_ROIWND_EVENT_MOVE, &CD_RoiSet::OnROIEvent_Move)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BTN_CHANGE_ROI, &CD_RoiSet::OnBnClickedBtnChangeRoi)
END_MESSAGE_MAP()


// CD_RoiSet 메시지 처리기


BOOL CD_RoiSet::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  여기에 추가 초기화 작업을 추가합니다.
	m_pBaseImage_u8 = imaqCreateImage(IMAQ_IMAGE_RGB, INT_IMAGE_BORDER_SIZE);

	//m_editX.SetMinMax(0, 600, INT_MONI_WND_WIDTH / 2);
	//m_editY.SetMinMax(0, 200, INT_MONI_WND_HEIGHT / 2);
	//m_editW.SetMinMax(0, INT_MONI_WND_WIDTH, 200);
	//m_editH.SetMinMax(0, INT_MONI_WND_HEIGHT, 200);

	m_editX.SetMinMax(0, m_iWIDTH, m_iWIDTH / 2);
	m_editY.SetMinMax(0, m_iHEIGHT, m_iHEIGHT / 2);
	m_editW.SetMinMax(0, m_iWIDTH, 200);
	m_editH.SetMinMax(0, m_iHEIGHT, 200);

	//---------------------------------------------------------------------
	// 로그 타이틀에 색상을 입혀준다.
	//---------------------------------------------------------------------
	m_LabelInfo.SetBorder(TRUE, TRUE);
	m_LabelInfo.SetBkColor(RGB(0, 32, 0));
	m_LabelInfo.SetTextColor(RGB(255, 255, 192));
	m_LabelInfo.SetFontSize(12);
	m_LabelInfo.SetFontBold(TRUE);
	
//	m_lblRoiTilte.SetBkColor(RGB_LIGHT_GRAY);
//	m_lblRoiTilte.SetFontSize(12);

	DefineROI();

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}


void CD_RoiSet::OnBnClickedBtnApplyRoi()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CString filter = _T("PNG Image Files (*.png)|*.png|");
	CString pathName = m_strSavePath;
	gf_MakeDir(pathName);

	CString fileName = pathName + _T("*.png");

	CFileDialog dlg(FALSE, _T(""), fileName, OFN_OVERWRITEPROMPT, filter);
	if (dlg.DoModal() == IDOK)
	{
		CString pathname = dlg.GetPathName();
		CString filename = dlg.GetFileName();

		int iCurr = -1;
		m_pimaqctrlROI->GetPatternImageAndRect(m_ptrnROI->m_pImgOrg, &m_ptrnROI->m_rectPtrn);

		//#ifdef _DEBUG		//jhseo 20140522 remove
		USES_CONVERSION;
		int ret = imaqWritePNGFile(m_ptrnROI->m_pImgOrg, T2A(pathname), 750, NULL);
		if (!ret)
		{
			char* errStr = imaqGetErrorText(imaqGetLastError());
	//		MessageBox(errStr, _T("Error Saving Image"), MB_OK);
			TRACE("%s(%i) : %s\n", __FILE__, __LINE__, errStr);
			imaqDispose(errStr);
		}
		//#endif

		m_ptrnROI->m_strName = filename;
		m_ptrnROI->m_bDetected = TRUE;

		//// update ui
		//switch (m_iCallingView)
		//{
		//case VI_PTRN_MULTI:
		//	break;
		//case VI_PTRN_VISION:
		//	m_pDlgNaviMaint->m_dlgNaviMaintVision.UpdatePatternImageScaled(TRUE, FALSE);
		//	break;
		//case VI_PTRN_MEAS:
		//	break;
		//case VI_ALIGN:
		//	break;
		//}
	}
}


void CD_RoiSet::OnBnClickedBtnCancelRoi()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CDialog::OnOK();
}


void CD_RoiSet::OnBnClickedBtnSaveImage()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CString strExtension = _T(".png");
	CString strFilter;
	strFilter.Format(_T("Image File(*%s)|*%s||"), strExtension, strExtension);

	CString strDefName;
	//	strDefName.Format("filter_%s.ini", m_pAutoFocus->GetCurrentDateTimeToString());

	CFileDialog cfd(FALSE,
		strExtension,
		_T("tempImage.png"),
		OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_LONGNAMES,
		strFilter);

	CString strPath = m_strSavePath; // _T("");
	cfd.m_ofn.lpstrInitialDir = strPath;
	cfd.m_ofn.lpstrTitle = _T("Save As ImageFile");

	if (cfd.DoModal() == IDOK)
	{
		CString strName = cfd.GetPathName();

		USES_CONVERSION;
		imaqWritePNGFile(m_pBaseImage_u8, T2A(strName), 750, NULL);
	}
}

LRESULT CD_RoiSet::OnROIEvent_Click(WPARAM wp, LPARAM lp)
{
	return 0;
}

LRESULT CD_RoiSet::OnROIEvent_Draw(WPARAM wp, LPARAM lp)
{
	CRect* rect = (CRect*)wp;
	m_editX.SetValue(rect->left);
	m_editY.SetValue(rect->top);
	m_editW.SetValue(rect->Width());
	m_editH.SetValue(rect->Height());
	UpdateData(FALSE);

	m_ptrnROI->m_rectPtrn = rect;

	return 0;
}

LRESULT CD_RoiSet::OnROIEvent_Move(WPARAM wp, LPARAM lp)
{
	return 0;
}

void CD_RoiSet::DefineROI()
{
	// TODO: 여기에 구현 코드 추가.
	CString strTitle;
	switch (m_iCallingView)
	{
	case VI_ALIGN:
		strTitle.Format(_T("Define : Align Pattern"));	
		break;

	case VI_PTRN_VISION:
	case VI_PTRN_MEAS:

		strTitle.Format(_T("Define : Meas.Pattern"));
		break;

	case VI_PTRN_MULTI:
		strTitle.Format(_T("Define : Multi-Pattern"));
		break;
	}

	m_LabelInfo.SetWindowText(strTitle);
	imaqDuplicate(m_pBaseImage_u8, m_ptrnROI->m_pImgInput);

#if _DEBUG
	imaqWriteJPEGFile(m_pBaseImage_u8, "D:\\Temp\\Snap2.jpg", 750, NULL);
#endif

	if (m_pimaqctrlROI == nullptr)
		m_pimaqctrlROI = new CIMAQImageCtrl();

	m_pimaqctrlROI->PopupROIDefine2(this,
		IDC_STC_ROI_BG_IMAGE,
		m_pBaseImage_u8,
		&m_ptrnROI->m_rectPtrn);
}


void CD_RoiSet::OnDestroy()
{
	CDialogEx::OnDestroy();

	// TODO: 여기에 메시지 처리기 코드를 추가합니다.
	if (m_pimaqctrlROI)	m_pimaqctrlROI->DestroyMe();
	SAFE_DELETE(m_pimaqctrlROI);

	IMAQ_DISPOSE(m_pBaseImage_u8);
}


void CD_RoiSet::OnBnClickedBtnChangeRoi()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	if (m_pimaqctrlROI != nullptr) return;

	int x0 = m_editX.GetValue();
	int y0 = m_editY.GetValue();
	int x2 = x0 + m_editW.GetValue();
	int y2 = y0 + m_editH.GetValue();

	m_pimaqctrlROI->SetROIRect(CRect(x0, y0, x2, y2));
}

void CD_RoiSet::SetROIViewInfo(int iCallingView, CPtrnInfo* pInfo)
{
	m_iCallingView = iCallingView;
	m_ptrnROI = pInfo;

	int iW = 0, iH = 0;
	imaqGetImageSize(m_ptrnROI->m_pImgInput, &iW, &iH);

#if _DEBUG
	imaqWriteJPEGFile(m_ptrnROI->m_pImgInput, "D:\\Temp\\Snap4.jpg", 750, NULL);
#endif

	if (iW > 0 && iH > 0)
	{
		m_ptrnROI->m_rectPtrn.top = iH / 2 - 100;
		m_ptrnROI->m_rectPtrn.bottom = iH / 2 + 100;
		m_ptrnROI->m_rectPtrn.left = iW / 2 - 200;
		m_ptrnROI->m_rectPtrn.right = iW / 2 + 200;

		m_iWIDTH = iW;
		m_iHEIGHT = iH;
	}
	else
	{
		m_ptrnROI->m_rectPtrn.top = INT_MONI_WND_HEIGHT / 2 - 100;
		m_ptrnROI->m_rectPtrn.bottom = INT_MONI_WND_HEIGHT / 2 + 100;
		m_ptrnROI->m_rectPtrn.left = INT_MONI_WND_WIDTH / 2 - 200;
		m_ptrnROI->m_rectPtrn.right = INT_MONI_WND_WIDTH / 2 + 200;
	}
}

void CD_RoiSet::SetSavePath(CString sPath)
{
	m_strSavePath = sPath;
}