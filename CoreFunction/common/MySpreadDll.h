#pragma once

// MySpreadDll.h: interface for the CMySpreadDll class.
//
//////////////////////////////////////////////////////////////////////

/*========================================================= 

FPSpreadDll7.0 사용.. : 80은 헤더랑..몇가지 70이랑 달라서..걍 70쓰구있음.
 - Spread 관련 잡동사니 함수들 모아놓은 클래스..
 - Static component를 현재 클래스로 선언해서 이 안에있는 함수를 사용하던지, spread를 바로 찍어서 쓰던지..

1. Header file
	#include "MySpreadDll.h"
	CMySpreadDll*	m_pgrid;
2. 생성자, 소멸자에 처리해주고..
3. Create ( ::OnInit~ )
	uComponentID : static component id
	uid			 : 생성할 id (끝에 +1000 씩하믄..안겹치겠지??)
	pParentWnd	 : evnet-message를 받을 윈도
	bUseMessage	 : evnet-message를 사용할지 여부 (default = FALSE)

	m_pgrid = new CMySpreadDll(IDC_FPSPREAD_GRID, m_uID+FPS_RESOURCE_ID+1000, this);

=========================================================*/

// #if (USE_FPSPREAD_8 == 1)
// 	#include "./FPSpreadDll/8.0/Ssdllmfc.h"
// #elif (USE_FPSPREAD_8 == 0)
// 	#include "./FPSpreadDll/7.0/Ssdllmfc.h"
// #endif

//#include "./FPSpreadDll/Ssdllmfc.h"
#include "..\common\FarPoint64\Ssdllmfc.h"
//=========================================================
// Color
//=========================================================
#if !defined(USER_COLOR_T_DEFINED)
#define USER_COLOR_T_DEFINED

// standard color
#define RGB_WHITE			RGB(255, 255, 255)
#define RGB_BLACK			RGB(0, 0, 0)
#define RGB_RED				RGB(255, 0, 0)
#define RGB_GREEN			RGB(0, 255, 0)
#define RGB_BLUE			RGB(0, 0, 255)
#define RGB_CYAN			RGB(0, 255, 255)
#define RGB_MAGENTA			RGB(255, 0, 255)
#define RGB_YELLOW			RGB(255, 255, 0)
#define RGB_DEEP_GREEN		RGB(0, 128, 0)
#define	RGB_GRAY			RGB(128, 128, 128)
#define	RGB_LIGHT_GRAY		RGB(192, 192, 192)
#define	RGB_DARK_GRAY		RGB(70, 70, 70)
#define	RGB_ORANGE			RGB(255, 128, 0)

#define RGB_RED_TXT			RGB(240, 0, 0)
#define RGB_GREEN_TXT		RGB(0, 170, 0)
#define RGB_BLUE_TXT		RGB(10, 80, 250)

#define RGB_GRAPH_BG		RGB(237, 241, 254)


 ////custom color
 //#define RGB_MAIN			RGB(225, 231, 253)	// main back.	  (흐린파랑부분)
 //#define RGB_MAIN2			RGB(119, 161, 224)	// 테두리 프레임. (진한파랑부분)

 ////gray
 //#define RGB_MAIN			RGB(221, 221, 221)	// main back.	  (흐린회색부분)
 //#define RGB_MAIN2			RGB(180, 180, 180)	// 테두리 프레임. (진한회색부분)
 //#define RGB_CHILD			RGB(204, 218, 233)	// light-blue : Child Dialog


//// light gray
//#define RGB_MAIN			RGB(221, 221, 221)	// main back.	  (흐린회색부분)
//#define RGB_MAIN2			RGB(190, 190, 190)	// 테두리 프레임. (진한회색부분)
//#define RGB_CHILD			RGB(216, 230, 248)	// Child Dialog
//#define RGB_CHILD			RGB(240, 240, 216)	// Child Dialog
//#define RGB_CHILD			RGB(224, 215, 196)	// Child Dialog
//#define RGB_CHILD			RGB(240, 240, 240)	// Child Dialog

 //green
 #define RGB_MAIN			RGB(220, 230, 220)	// main back.
 #define RGB_MAIN2			RGB(190, 200, 190)	// 테두리 프레임
 #define RGB_CHILD			RGB(233, 234, 204)	// Child Dialog

#define RGB_LIGHT_BLUE1		RGB(190, 210, 240)	// grid header
#define RGB_LIGHT_BLUE2		RGB(218, 230, 248)
#define RGB_LIGHT_BLUE3		RGB(239, 242, 254)	// grid back
#define RGB_DARK_BLUE1		RGB(24, 44, 123)	// text
#define RGB_DARK_BLUE2		RGB(25, 65, 139)
#define RGB_DARK_BLUE3		RGB(60, 101, 193)
#define RGB_LIGHT_PINK		RGB(226, 197, 245)	// grid Sel.
#define RGB_DARK_PINK		RGB(116, 16, 158)	// btn text

#define RGB_GRAY_240		RGB(240, 240, 240)
#define RGB_GRAY_230		RGB(230, 230, 230)
#define RGB_GRAY_220		RGB(220, 220, 220)
#define RGB_GRAY_210		RGB(210, 210, 210)
#define RGB_GRAY_200		RGB(200, 200, 200)
#define RGB_GRAY_190		RGB(190, 190, 190)
#define RGB_GRAY_170		RGB(170, 170, 170)
#define RGB_GRAY_150		RGB(150, 150, 150)
#define RGB_GRAY_120		RGB(120, 120, 120)
#define RGB_LIGHT_BEIGE		RGB(255, 255, 250)
//#define RGB_LIGHT_BEIGE2	RGB(246, 246, 235)
#define RGB_LIGHT_BEIGE2	RGB(249, 249, 233)
#define RGB_LIGHT_BLUE4		RGB(198, 215, 254)	// grid sel.

#define FPS_RESOURCE_ID	10000	// FPSpreadDll : resource 생성할 때 쓸 default id

#endif	// USER_COLOR_T_DEFINED



class CMySpreadDll : public TSpread 
{
protected:
	BOOL	m_bUseMessage;
	CWnd*	m_pParentWnd;
	UINT	m_uComponentID;

public:
	CMySpreadDll(UINT uComponentID, UINT uid, CWnd* pParentWnd, BOOL bUseMessage=FALSE);
	virtual ~CMySpreadDll();

	HFONT	m_hfont;	// default font

	CString	GetDataString(int col, int row);
	int		GetDataInt(int col, int row);
	double	GetDataDbl(int col, int row);
	void	SetDataString(int col, int row, CString data);

	void	MakeFont(HFONT* pFont, CString strName= _T("Tahoma"), int iSize=14, BOOL bBold=FALSE, BOOL bItalic=FALSE);
	void	InitDefault();
	void	InitDefault_Progress();

	//=========================================================
	// 특정 뷰에서만 사용하는 초기화 함수들..
	void	InitMySpread_RecipeList();		// CRecipeView
	void	InitMySpread_AlignPos();		// CAlignView
	void	InitMySpread_AreaPos();
	void	InitMySpread_Result();			// COperateView
	void	InitMySpread_MeasCCD();			// CMeasPBView
	void	InitMySpread_MeasPBResult_curr();
	void	InitMySpread_MeasSpecResult_curr(int nCol=1);
	void	InitMySpread_User();			// CSetup_UserDlg
	void    InitMySpread_Matrix();			// CCommon_MatrixDlg



	void	InitMySpread_Info();			// CInfoView
	void	InitMySpread_MeasArea();		// COperatorView
	void	InitMySpread_MeasResult();
	
	void	InitMySpread_FtpList();			// CRecipe_FTPDlg
	
	void	InitMySpread_Log();				// CLog_ViewerDlg

	void	InitMySpread_Param(int nCol, int nRow, BOOL bHAlignCenter/*=TRUE*/);	// CMeasSPIView
	void	InitMySpread_CommonParamTitle(BOOL bUseTubeLens=FALSE);
	void	InitMySpread_CommonParam(BOOL bUseTubeLens=FALSE);

	void	InitMySpread_PtrnThreshold(int nMultiPtrn);	// CRecipeOffset_Dlg
	void	InitMySpread_T8KFilterSet(int nFilter=30);	// CMeasT8KParamView



	void	InitMySpread_SACCIO(int nFilter = 30);	// ALPHA SACC View
	void	InitMySpread_SACCBF(int nFilter = 30);	// ALPHA SACC View
	void	InitMySpread_SACCID(int nFilter = 30);	// ALPHA SACC View
	void	InitMySpread_InsideProbe(int nFilter = 30);	// ALPHA SACC View
	

	void	InitMySpread_AGINGSET(int nFilter = 30); // ALPHA AGINGSET View
	void	InitMySpread_AGINGTEST_MEAS(int nFilter = 30); // ALPHA AGINGTEST Meas View
	void	InitMySpread_AGINGTEST_LIST(int nFilter = 30); // ALPHA AGINGTEST List View
	void	InitMySpread_AUTOCONTACT(int nFilter = 30); // ALPHA AUTOCONTACT View

	void	InitMySpread_MEAS_PROGRESS(int nFilter = 30); // ALPHA MEASURE PROGRESS View


	void    InitMySpread_RESULT(int nFilter = 30); // RESULT View

	//=========================================================
	// 주로 사용하는 Message..

	// Generated message map functions
	//{{AFX_MSG(CMySpreadDll)
	afx_msg LRESULT OnFPSpreadClick(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnFPSpreadDblClick(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnFPSpreadDataChange(WPARAM wParam, LPARAM lParam);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

