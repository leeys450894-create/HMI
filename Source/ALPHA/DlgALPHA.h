
// DlgALPHA.h: 헤더 파일
//

#pragma once


// CDlgALPHA 대화 상자
class CAuroraMgr;
class CDlgALPHA : public CDialogEx
{
// 생성입니다.
	CAuroraMgr* m_pAuroraMgr;

public:
	CDlgALPHA(CAuroraMgr* pAuroraMgr, CWnd* pParent = nullptr);	// 표준 생성자입니다.

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ALPHA_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 지원입니다.


// 구현입니다.
protected:
	HICON m_hIcon;

	// 생성된 메시지 맵 함수
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnClose();
};
