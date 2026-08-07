#pragma once

typedef	void	(*CIM_EVENT_HANDLER)(TCHAR*, int, int); //메시지 전달

enum CIMItem {
	CIM_WAIT_CLEAR				= -1,
	CIM_WAIT_AT_1_OK			= 0,
	CIM_WAIT_EQSTART_RCV,
	CIM_WAIT_ALMACL_OK,
	CIM_WAIT_CIM_TIMESET,
	CIM_WAIT_STS0104_OK,
	CIM_WAIT_STS0204_OK,
	CIM_WAIT_ORG_RCV,
	CIM_WAIT_STS0304_OK,

	CIM_WAIT_STS0004_OK,

	CIM_WAIT_STS0506_OK,
	CIM_WAIT_PPID,
	CIM_WAIT_STS2004_OK,
	CIM_WAIT_LOTID,
	CIM_WAIT_GLSID,
	CIM_WAIT_STS0006_OK,
	CIM_WAIT_PRCS_RCV,
	CIM_WAIT_STS1005_OK,
	CIM_WAIT_STS1006_OK,
	CIM_WAIT_STS1106_OK,
	CIM_WAIT_GDTREQ_RCV,
	CIM_WAIT_ULDREQ_OR_MULTIRCP_RCV,	//CIM_WAIT_ULDREQ_RCV,
	CIM_WAIT_STS2106_OK,
	CIM_WAIT_STS2107_OK,
	
	CIM_WAIT_HS_OFF,
	CIM_WAIT_EXCHANGE_OR_UNLOAD_RCV,



	CIM_WAIT_MAX,
};

//Group1
#define	CIM_G1_UNLOAD_REQ_RCV		1	//ULDREQ$
#define	CIM_G1_MULTI_RCP_RCV		2	//MULTIRECIPE$

//Group2
#define CIM_G2_EXCHANGE_RCV			1	//GLSREADY$
#define CIM_G2_UD_CPL_RCV			2	//ULDCPL$



class	AFX_EXT_CLASS	C_CIMComm : public C_ClientSocket
{
	public:
		C_CIMComm (QUALITY_EVENT_HANDLER pHandler, CString IniPath, int nIndex);
		virtual ~C_CIMComm ();

	public:
		int					m_nRunning;
		int					m_nIndex;	//Socket 통신 순서
		CIM_EVENT_HANDLER	m_pHandler;
		BOOL				m_bWaitItem[CIM_WAIT_MAX];
		S_TcpIpInfo			m_sInfo;

		int					m_nUDReq_or_MultiRcp;
		int					m_nExchange_or_Unload;
		BOOL				m_bLoadGlassExist;

		CString				m_strCIMTime;
		CString				m_strPPID,  m_strFlag;
		CString				m_strLotID, m_strCstID,  m_strOperID;
		CString				m_strGlsID, m_strSlotID, m_strStepID, m_strRotate;

	public:
		virtual	void	ReceiveCode (TCHAR* pBuf);
		virtual void	OnClose (int nErrorCode);
		virtual void	OnConnect (int nErrorCode);

	public:
		void			ConnectCtrl (BOOL bConnect);
		void			ReadIniInfo ();
		void			WriteIniInfo (S_TcpIpInfo *sInfo);
		S_TcpIpInfo		GetIniInfo () { return m_sInfo; };

		void	SetWaitItem (int nItem, BOOL bSet);
		BOOL	GetWaitItem (int nItem);
		void	TimeOutAlarm (CString strCmd, int nTime);
		void	NakAlarm (CString strCmd);
		void	SendAlarm (CString strCmd, int nTime);

		BOOL	WaitSTOP ();

		////명령
		//초기화
		BOOL	ActAT1		  ();	//CIM Online 진행
		BOOL	SetRecipeList ();	//레시피 리스트 복사 후 CIM에 보고
		BOOL	ActAlarmClear ();	//CIM Alarm Clear 요청
		BOOL	ActCIMTime    ();	//CIM PC 시간 요청
		BOOL	ActSTS0104    ();	//H/W Initialize Start
		BOOL	ActSTS0204    ();   //
		BOOL	ActSTS0304    ();

		//팝업 선택 Measure : Glass 없을시 Ready 상태
		BOOL	ActSTS0004 ();

		//팝업 선택 Measure : Glass 있으면 측정 진행
		BOOL	ActSTS0506		();
		BOOL	ActPPID_Req		();
		BOOL	SetPPID_OK		();
		BOOL	SetPPID_NG		();
		BOOL	ActSTS2004		();
		BOOL	ActLotID_Req	();
		BOOL	ActGlsID_Req	();
		BOOL	ActSTS0006		();
		BOOL	SetPCRS_OK		();
		BOOL	SetPCRS_NG		();
		BOOL	ActSTS1005		();	//Lift Pin Down
		BOOL	ActSTS1006		();	//Measure Start
		BOOL	ActSTS1106		();	//Measure End
		BOOL	ActDataJudge_OK ();
		BOOL	ActDataJudge_NG ();
		BOOL	SetGDTREQ_OK	();
		BOOL	SetGDTREQ_NG	();
		BOOL	SetGDTCPL_OK	();
		BOOL	SetGDTCPL_NG	();
		BOOL	SetULDREQ_OK	(); //GDTCPL -> ULDREQ$ -> ULDREQ_OK or NG -> Unloading 위치 이동 -> STS2106
		BOOL	SetULDREQ_NG	();	//GDTCPL -> MultiRecipe$ -> MultiRecipe_OK or NG -> PPIDREQ
		BOOL	ActSTS2106		();
		BOOL	ActSTS2107		(); //HS_OFF -> GLSREADY$ -> GLSREADY_OK or NG -> PPIDREQ	//HS_OFF -> ULDCPL$ -> ULDCPL_OK or NG -> STS0004

		//Multi Recipe
		BOOL	SetMULTIRECIPE_OK ();
		BOOL	SetMULTIRECIPE_NG ();

		//Exchange or Unload
		BOOL	SetGLSREADY_OK ();
		BOOL	SetGLSREADY_NG ();
		BOOL	SetULDCPL_OK   ();
		BOOL	SetULDCPL_NG   ();

		//Glass Load
		BOOL	SetLDREQ_OK	   ();
		BOOL	SetLDREQ_NG    ();


		////응답 처리
		void	ReceiveEventOK (int nEvent, TCHAR* pBuf);
		void	ReceiveEventNG ();
		//void	ReceiveClassEnd (CString strRcv);
		//void	ReceiveFilter (int nItem);

		void	WriteComm			(LPCTSTR lpszMsg);
		BOOL	Delay				(DWORD dwTime = 100);
		BOOL	RunModalLoop		(int nWaitItem = -1, DWORD dwTime = 20000);
		BOOL	RunModalFinish		(BOOL bFinishSet = TRUE);
		void	GetParsingCIMData	(CString strRcv, CStringArray &strArr);
		void	GetCIMInfoData		(CString strRcv, int nDataCnt, int nWaitItem);

		void	Reset_UD_or_MultiRcp     ()						   { m_nUDReq_or_MultiRcp = -1; };
		void	Set_UD_or_MultiRcp	     (int nUDReq_or_MultiRcp)  { m_nUDReq_or_MultiRcp = nUDReq_or_MultiRcp; };
		int		Get_UD_or_MultiRcp	     ()						   { return m_nUDReq_or_MultiRcp; };
		void	Reset_Exchange_or_Unload ()						   { m_nExchange_or_Unload = -1; };
		void	Set_Exchange_or_Unload	 (int nExchange_or_Unload) { m_nExchange_or_Unload = nExchange_or_Unload; };
		int		Get_Exchange_or_Unload	 ()						   { return m_nExchange_or_Unload; };
		void	Reset_LoadGlassInfo		 ()						   { m_bLoadGlassExist = FALSE; };
		void	Set_LoadGlassInfo		 (BOOL bLoadGlassExist)    { m_bLoadGlassExist = bLoadGlassExist; };
		BOOL	Get_LoadGlassInfo	     ()						   { return m_bLoadGlassExist; };


		//파일 설정 정보
		BOOL	GetCommUse			() { return m_sInfo.m_bUse; };
		BOOL	GetCommOutOfService () { return m_sInfo.m_bOutOfService; };
};

