#pragma once
#include "nivision.h"

// CD_CCD_Review 대화 상자
class CIMAQImageCtrl;
class CD_CCD_Review : public CDialogEx
{
	DECLARE_DYNAMIC(CD_CCD_Review)

	int m_iMoniImageWnd;	// Image window number

	HWND					m_hMoniImage;		// Handle for monitoring image window
	CIMAQImageCtrl* m_pimaqctrlROI;

public:
	CD_CCD_Review(CWnd* pParent = nullptr);   // 표준 생성자입니다.
	virtual ~CD_CCD_Review();


// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DLG_SUB_CCD_FULL };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	int DisplayImage(Image* pImage);
	int SetWindowSize(int iGrabSizeW, int iGrabSizeH);
};
