#pragma once


// CDlgSubManual_SACC 대화 상자

class CDlgSubManual_SACC : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgSubManual_SACC)

public:
	CDlgSubManual_SACC(CWnd* pParent = nullptr);   // 표준 생성자입니다.
	virtual ~CDlgSubManual_SACC();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DLG_SUB_MANUAL_SACC };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
};
