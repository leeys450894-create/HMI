#pragma once

#include "..\CoreFunction\common\Label.h"

// D_DisplayMessage 대화 상자입니다.

class CD_DisplayMessage : public CDialogEx
{
	DECLARE_DYNAMIC(CD_DisplayMessage)

private:
	CLabel	m_labelInfo;
	CString m_strMessage;

public:
	CD_DisplayMessage(CString strMessage, CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~CD_DisplayMessage();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_POPUP_DIPLAY_MESSAGE };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnBnClickedButtonInspComplete();
};
