#pragma once
#include <atlimage.h>
#include "ButtonEx.h"
#include "preDefine.h"

class C_AlignRcpDlg : public C_BaseDialog
{
	DECLARE_DYNAMIC(C_AlignRcpDlg)

	public:
		C_AlignRcpDlg (CWnd* pParent = NULL);   // 표준 생성자입니다.
		virtual ~C_AlignRcpDlg();

	protected:
		CWnd		*m_hWnd;
		CImage		m_image;
		CStatic		m_stcView;
		CRect		m_rcDraw;

		C_ButtonEx	m_btnStyle[3];

		C_GridCtrl	m_gridGlassAlignHead;
		C_GridCtrl	m_gridGlassAlignData;
		
		C_GridCtrl	m_gridProbeAlignHead;
		C_GridCtrl	m_gridProbeAlignData;

		C_GridCtrl	m_gridCcdSetMntHead;
		C_GridCtrl	m_gridCcdSetMntData;

		C_GridCtrl	m_gridRefImageHead;
		C_GridCtrl	m_gridRefImageData;

		C_GridCtrl	m_gridMarginHead;
		C_GridCtrl	m_gridMarginData;

		C_GridCtrl	m_gridAlignProcData;

		C_GridCtrl	m_gridAlignPosHead;
		C_GridCtrl	m_gridAlignPosData;

		C_GridCtrl	m_gridAlignName;
		int			m_nSelCcd;
		int			m_nSelProbe;
		int			m_nRefRow;
		int			m_nRefCol;

	public:
		//VTM_RECIPE	*m_pRecipeMem;

	public:
		int GetSelHead () {return m_nSelCcd;};
		void InitAlignGridUI (C_GridCtrl &Grid, int nDlgItem, int nRow, int nCol, int nType);
		void InitAlignName ();
		void SetAlignName (CString str);
		CString GetAlignName ();
		
		void InitGlassAlign ();
		void SetGlassAlignData ();
		BOOL GetGlassAlignData (BOOL bShare = FALSE, S_GlsAlign *pAlignShare = NULL);
		void SetGlassAlignKeyPos ();

		void InitProbeAlign ();
		void SetProbeAlignData ();
		BOOL GetProbeAlignData (BOOL bShare = FALSE, S_ProbeAlign *pProbeShare = NULL);

		void InitCcdSetMonitor ();
		void SetCcdSetMntData ();
		BOOL GetCcdSetMntData (BOOL bShare = FALSE, S_AlignCcdSet *pCcdShare = NULL);
		int	 GetMatchingScore () { return _wtoi (m_gridCcdSetMntData.GetItemText (m_nSelCcd, 4));};

		void InitRefImage ();
		void SetRefImageData ();
		BOOL GetRefImageData (BOOL bShare = FALSE, S_Align *pPatternShare = NULL);
		CString GetRefImageName () { return m_gridRefImageData.GetItemText (m_nRefRow, m_nRefCol);};

		void InitMargin ();
		void SetMarginData ();
		BOOL GetMarginData (BOOL bShare = FALSE, S_AlignMargin *pMarginShare = NULL);

		void InitAlignProc ();

		void InitAlignPos ();

		void SetAlignView ();
		BOOL GetAlignView ();
		BOOL SaveAlignFile (CString strRcpName);
		void GetCcdCurrentView (int nHEAD);
		void OnSelCcdCurrent (NMHDR *pNMHDR, LRESULT *pResult);
		void OnSelRefImg (NMHDR *pNMHDR, LRESULT *pResult);
		void ShowRefImage ();
		CString GetCcdItemText (int m_nSelCcd);
		void GetCcdCurrentVal (int *nData);
		void GetAlignPos (double *fData);
		void SetAlignPos (double *fData);
		void GetProbePos (double *fData);
		void SetProbePos (double *fData);
		void SetAlignEndPos (double *fData, int nItem);
		int GetProbeSel ();

		void OnSelGlassAlign (NMHDR *pNMHDR, LRESULT *pResult);
		void OnSelProbeAlign (NMHDR *pNMHDR, LRESULT *pResult);
		void OnAlignProc (NMHDR *pNMHDR, LRESULT *pResult);

		BOOL LoadAlignRecipe (CString strName);

	protected:
		//afx_msg void OnPaint ();
		virtual BOOL OnInitDialog ();
		virtual void DoDataExchange (CDataExchange* pDX);    // DDX/DDV 지원입니다.
		afx_msg void OnDestroy ();
		afx_msg void OnAlignLoad ();
		afx_msg void OnAlignSave ();
		afx_msg void OnAlignSaveAs ();
		afx_msg void OnCcdSetUpdate ();
		afx_msg void OnShowRefImageSet ();
		afx_msg void OnPatternSearch ();
		afx_msg void OnAlignCcdSet ();


	DECLARE_MESSAGE_MAP()
};
