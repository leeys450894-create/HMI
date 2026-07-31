#pragma once
#include "common/CommonFunc.h"
#include "common\Label.h"
#include "..\CoreFunction\common\MySpreadDll.h"
#include "MotorFunction/AxisType.h"

// CDlgSubManual_STEP 대화 상자

class CDlgSubManual_STEP : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgSubManual_STEP)

private:
	CLabel		m_LabelInfo;

public:
	CDlgSubManual_STEP(UINT idTemplate, CWnd* pParent = nullptr);   // 표준 생성자입니다.
	virtual ~CDlgSubManual_STEP();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DLG_SUB_MANUAL_STEP };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
};
