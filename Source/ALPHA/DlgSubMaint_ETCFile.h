#pragma once


// CDlgSubMaint_ETCFile 대화 상자

class CDlgSubMaint_ETCFile : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgSubMaint_ETCFile)

public:
	CDlgSubMaint_ETCFile(CWnd* pParent = nullptr);   // 표준 생성자입니다.
	virtual ~CDlgSubMaint_ETCFile();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DLG_SUB_MAINT_ETC };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
};
