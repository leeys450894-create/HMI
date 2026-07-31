#if !defined( DEFINESCIM_H_INCLUDED )
#define DEFINESCIM_H_INCLUDED

#define df_WAIT_TIMEOUT			3
#define df_WAIT_TIMEOUT_LONG	30

//Made By Jay Hong : 2009.09.02
//나머지 추가 안된 부분은 장비 테스트 하면서 추가하자
#define dfMDREQ				_T("MDREQ$")
#define dfMD_1				_T("MD1$")
#define dfMD_1_OK			_T("MD1_OK$")
#define dfMD_0				_T("MD0$")
#define dfMD_0_OK			_T("MD0_OK$")
#define dfAT_1				_T("AT1$")
#define dfAT_1_OK			_T("AT1_OK$")
#define dfAT_1_NG			_T("AT1_NG$")
#define dfAT_0				_T("AT0$")
#define dfAT_0_OK			_T("AT0_OK$")

#define dfGLSSTART			_T("GLSSTART$")
#define dfGLSSTART_OK		_T("GLSSTART_OK$")
#define dfEQPSTART			_T("EQPSTART$")
#define dfEQPSTART_OK		_T("EQPSTART_OK$")

#define dfRCPLIST			_T("RCPLIST$")
#define dfRCPLIST_OK		_T("RCPLIST_OK$")
#define dfRCPLIST_NG		_T("RCPLIST_NG$")
#define dfALMACL			_T("ALMACL$")
#define dfALMACL_OK			_T("ALMACL_OK$")
#define dfALMALLCLR			_T("ALMALLCLR$")
#define dfALMALLCLR_OK		_T("ALMALLCLR_OK$")

#define dfATREQ				_T("ATREQ$")
#define dfTIMEREQ			_T("TIMEREQ$")
#define dfTIMESET			_T("TIMESET")
#define dfHSON				_T("HS_ON$")				// pwk 20100630 add.
#define dfHSOFF				_T("HS_OFF$")				// pwk 20100630 add.

#define dfSTS_01_04			_T("STS0104$")		//Initialize Step#1
#define dfSTS_01_04_OK		_T("STS0104_OK$")		//Reply for Initialize Step#1
#define dfSTS_02_04			_T("STS0204$")		//Initailize Step#2
#define dfSTS_02_04_OK		_T("STS0204_OK$")		//Reply for Initialize Step#2
#define dfSTS_03_04			_T("STS0304$")		//Initialize Step#3
#define dfSTS_03_04_OK		_T("STS0304_OK$")		//Reply for Initialize Step#3
#define dfSTS_00_04			_T("STS0004$")		//Glass is Not Exist On Stage
#define dfSTS_00_04_OK		_T("STS0004_OK$")		//Reply for Glass is Not Exist On Stage
#define dfSTS_05_06			_T("STS0506$")		//Glass is Exist On Stage
#define dfSTS_05_06_OK		_T("STS0506_OK$")		//Reply for Glass is Exist On Stage
#define dfSTS_21_04			_T("STS2104$")		//EQP 상태보고 Prepare Glass In Wait
#define dfSTS_21_04_OK		_T("STS2104_OK$")		//Reply for EQP 상태보고 Prepare Glass In Wait
#define dfSTS_21_05			_T("STS2105$")		//Glass In Wait, Glass is not Exist
#define dfSTS_21_05_OK		_T("STS2105_OK$")		//Reply for Glass In Wait, Glass is not Exist
#define dfSTS_20_04			_T("STS2004$")		//EQP 상태보고
#define dfSTS_20_04_OK		_T("STS2004_OK$")		//Reply for EQP 상태보고
#define dfSTS_00_06			_T("STS0006$")		//Glass In Complete, Glass is Exist
#define dfSTS_00_06_OK		_T("STS0006_OK$")		//Reply for Glass In Complete, Glass is Exist
#define dfSTS_10_05			_T("STS1005$")		//Process Start Status.(Step 1)
#define dfSTS_10_05_OK		_T("STS1005_OK$")		//Process Start Status.(Step 1)
#define dfSTS_10_06			_T("STS1006$")		//Now EQ Process. Inspection 중 확인
#define dfSTS_10_06_OK		_T("STS1006_OK$")		//Now EQ Process. Inspection 중 확인
#define dfSTS_11_06			_T("STS1106$")		//Now EqQ Process End
#define dfSTS_11_06_OK		_T("STS1106_OK$")		//Now EqQ Process End
#define dfSTS_21_06			_T("STS2106$")		//Prepare Glass Out Wait.
#define dfSTS_21_06_OK		_T("STS2106_OK$")		//Reply for Prepare Glass Out Wait.
#define dfSTS_21_07			_T("STS2107$")		//Glass Out Wait. Glass is Exist
#define dfSTS_21_07_OK		_T("STS2107_OK$")		//Reply for Prepare Glass Out Wait.
#define dfSTS_00_99			_T("STS0099$")		//설비 MANUAL 상태 전환 상태
#define dfSTS_00_99_OK		_T("STS0099_OK$")		
#define dfSTS_00_99_NG		_T("STS0099_NG$")


#define dfSTS_ALARM			_T("Alarm")
#define	dfSTS_HANDSHAKE		_T("HANDSHAKE")
#define	dfSTS_COMPONENT		_T("COMPONENT")

#define	dfSTS_DATAJUDGE_OK		_T("DATAJUDGE_OK$")
#define	dfSTS_DATAJUDGE_SPCOUT	_T("DATAJUDGE_SPECOUT$")
#define	dfSTS_DATAJUDGE_NOTMEAS	_T("DATAJUDGE_NOTMEAS$")

#define dfSTS_ORG			_T("ORG$")				//Origin Command Waiting (Homing)
#define dfSTS_ORG_OK		_T("ORG_OK$")			//Reply for Origin Command Waiting (Homing)

#define dfLDREQ				_T("LDREQ$")			//Load Request (S6F67, S6F68)
#define dfLDREQ_OK			_T("LDREQ_OK$")			//Reply for LDREQ
#define dfLDREQ_NG			_T("LDREQ_NG$")			//Reply for LDREQ (NG)

#define dfPPID				_T("PPID")
#define dfPPIDREQ			_T("PPIDREQ$")			//PPID REQ
#define dfPPIDREQ_OK		_T("PPID_OK$")			//PPID OK //ksk 20100625 add
#define dfPPIDREQ_NG		_T("PPID_NG$")			//PPID NG //ksk 20100625 add

#define dfLOTREQ			_T("LOTREQ$")			//LOT REQ
#define dfLOTID				_T("LOTID")				//Reply for LOT REQ
#define dfLOTID_OK			_T("LOTID_OK")			//Reply for LOT REQ
#define dfLOTID_NG			_T("LOTID_NG")			//Reply for LOT REQ

#define dfGLSREQ			_T("GLSREQ$")			//GLASS REQ
#define dfGLSID				_T("GLSID")				//Reply for GLASS REQ
#define dfGLSID_OK			_T("GLSID_OK")			//Reply for GLASS REQ
#define dfGLSID_NG			_T("GLSID_NG")			//Reply for GLASS REQ

#define dfLDCPL				_T("LDCPL$")			//Load Complete 
#define dfLDCPL_OK			_T("LDCPL_OK$")			//Reply Load Complete 
#define dfLDCPL_NG			_T("LDCPL_NG$")			//Reply Load Complete 

//#define dfLDCPL				_T("LDCPL$")			//Load Complete 
//#define dfLDCPL_OK			_T("LDCPL_OK$")			//Reply Load Complete 
//#define dfLDCPL_NG			_T("LDCPL_NG$")

#define dfPRCS				_T("PRCS$")				//Process Start ( Measure Start )
#define dfPRCS_OK			_T("PRCS_OK$")			//NG:Incorrect status OK:PRCS Response(Process Start)
#define dfPRCS_NG			_T("PRCS_NG$")			//NG:Incorrect status OK:PRCS Response(Process Start)


#define dfGDTREQ			_T("GDTREQ$")			//GDTREQ Response(Glass Data)
#define dfGDTREQ_OK			_T("GDTREQ_OK$")		//GDTREQ Response(Glass Data)
#define dfGDTCPL_OK			_T("GDTCPL_OK$")
#define dfGDTCPL_NG			_T("GDTCPL_NG$")

#define dfDTREQ				_T("DTREQ$")
#define dfDTREQ_OK			_T("DTREQ_OK$")
#define dfDTCPL				_T("DTCPL$")
#define dfDTCPL_OK			_T("DTCPL_OK$")

#define dfDTNG				_T("DTNG$")
#define dfDTNG_OK			_T("DTNG_OK$")

#define dfGDTCPL_TR_OK		_T("GDTCPL_TR_OK$")		//sklee 20201104 add
#define dfGDTCPL_TR_NG		_T("GDTCPL_TR_NG$")		//sklee 20201104 add

#define dfULDREQ			_T("ULDREQ$")			//Unload Request
#define dfULDREQ_OK			_T("ULDREQ_OK$")		//Reply for Unload Request
#define dfULDREQ_NG			_T("ULDREQ_NG$")		//Reply for Unload Request

#define dfRCPCHG			_T("RCPCHG")			//RCPCHG#+A[16],# : 1->생성 , 2->수정 , 3->삭제
#define dfRCPCHG_OK			_T("RCPCHG_OK$")

#define dfSRMMODE			_T("SRMMODE")			//SRMMODE#,# : 0->SRM MODE 완료 , 1->SRM MODE 시작
//SRMMODE#_OK

#define dfPMMODE1			_T("PMMODE1$")			//PMMODE#,PM MODE 전환 EVENT,#: 0->PM 해제 , 1->PM 시작
#define dfPMMODE0			_T("PMMODE0$")
#define dfPMMODE_OK			_T("PMMODE_OK$")

#define dfGLSREADY			_T("GLSREADY$")			//Next Glass Ready
#define dfGLSREADY_OK		_T("GLSREADY_OK$")		//Reply for Next Glass Ready

#define dfMULTIRCP			_T("MULTIRECIPE$")		//Next Glass Ready
#define dfMULTIRCP_OK		_T("MULTIRECIPE_OK$")	//Reply for Next Glass Ready

#define dfULDCPL			_T("ULDCPL$")			//Unload Complete
#define dfULDCPL_OK			_T("ULDCPL_OK$")		//Reply for Unload Complete
#define dfULDCPL_NG			_T("ULDCPL_NG$")

#define dfEQPABORT			_T("EQPABORT$")			//EQP Abort Current Glass
#define dfEQPPAUSE			_T("EQPPAUSE$")			//EQP Pause Current Glass
#define dfEQPRESTART		_T("EQPRESTART$")		//EQP Restart Current Glass
#define dfEQPABORT_OK		_T("EQPABORT_OK$")		//EQP Reply for Abort Current Glass
#define dfEQPPAUSE_OK		_T("EQPPAUSE_OK$")		//EQP Reply for Pause Current Glass
#define dfEQPRESTART_OK		_T("EQPRESTART_OK$")	//EQP Reply for Restart Current Glass

#define dfFORCEINIT			_T("FORCEINIT$")		//Force Initialize Request
#define dfFORCEINIT_OK		_T("FORCEINIT_OK$")		//Reply OK for Force Initialize Request
#define dfFORCEINIT_NG		_T("FORCEINIT_NG$")		//Reply NG for Force Initialize Request
#define dfFORCEINIT_END		_T("FORCEINIT_END$")	//Reply for Force Initialize Completed

#define dfVCRREQ			_T("VCRREQ$")			//2010.04.22
#define dfVCRUSE			_T("VCRUSE$")

// MCC
#define dfMCCON				_T("MCC_ON$")
#define dfMCCOFF			_T("MCC_OFF$")
#define dfMCCTIME			_T("MCCTIMESET")

//20210224 jychoi add PASSMODE
#define dfPASSMODE			_T("PASS_MODE$")		// PASS_MODE (COLD RUN)

//20211110 ksm Machine Tact Time
#define dfMTACTTIME         _T("MTACT_TIME")

const enum CIM_CMD_SERVER_TO_CIM{
	
	TO_CIM_RESPONSE	= 0,	//Response
	TO_CIM_STS0104	= 1,	//Initialize Step 1
	TO_CIM_STS0204,			//Initialize Step 2
	TO_CIM_STS0304,			//Initialize Step 3
	TO_CIM_STS0004,			//Glass is Not Exist On Stage	
	TO_CIM_STS0506,			//Glass is Exist On Stage	
	TO_CIM_STS2004,			//Reserved	
	TO_CIM_STS2104,			//Prepare Glass In Wait.	
	TO_CIM_STS2105,			//Glass In Wait, Glass is Not Exist	
	TO_CIM_STS0006,			//Glass In Complete, Glass is Exist	
	TO_CIM_STS1005,			//Process Start Status.(Step 1)	
	TO_CIM_STS1006,			//Now EQ Process.	
	TO_CIM_STS1106,			//Now EqQ Process End	
	TO_CIM_STS2106,			//Prepare Glass Out Wait.	
	TO_CIM_STS2107,			//Glass Out Wait. Glass is Exist	
	
	TO_CIM_ORG,				//Loader zero point position(reserved)	
	TO_CIM_MDREQ,			//Mode Request(CIM Online/Offline Mode)
	TO_CIM_AT0,				//1 - Online, 0 - Offline
	TO_CIM_AT1,
		
	TO_CIM_EQPSTART,		//EQPSTART Response(Online Start)
	TO_CIM_ALM,				//Alarm happen
	TO_CIM_WRN,				//Warning happen
	TO_CIM_WRNRPT,			//Warning Report - #(4) : Warning ID, #(8) : X Pos, #(8) : Y Pos, #(5) : Z Pos
		
	TO_CIM_ALMCLR,			//Alarm clear
	TO_CIM_WRNCLR,			//Warning clear
	TO_CIM_TIMEREQ,			//Host time request	
	TO_CIM_PPIDREQ,			//PPID request
	TO_CIM_LOTREQ,			//Lot data request
	TO_CIM_GLSREQ,			//Glass data request
	TO_CIM_RCPLIST,			//Recipe update,( FTP file name : RcpList.txt )	
		
	TO_CIM_ALMACL,				//Alarm all clear ( initialize step )
	TO_CIM_LDREQ,				//NG:Incorrect status OK:LDREQ Response
	TO_CIM_LDCPL,				//NG:Incorrect status OK:ULDCPL Response(Unload Complete)
	TO_CIM_PPID,				//Incorrect status	
	TO_CIM_LOTID,				//Incorrect status
	TO_CIM_GLSID,				//Incorrect status
	TO_CIM_DATA,				//Normal measure data(JUDGE_OK$),Abnormal measure data ( Spec Out)(JUDGE_SPECOUT$),Not Measured(JUDGE_NOTMEAS$)
		
		
	TO_CIM_GDTREQ,				//GDTREQ Response(Glass Data)
	TO_CIM_GDTCPL,				//NG:APD Data Send Fail OK:APD Data Send Success
	TO_CIM_PRCS,				//NG:Incorrect status OK:PRCS Response(Process Start)
	TO_CIM_ULDREQ,				//NG:Incorrect status OK:ULDREQ Response
	TO_CIM_ULDCPL,				//
	TO_CIM_GLSREADY,			//NG:Incorrect status OK:GLSREADY Response
	TO_CIM_EQPPAUSE,			//EQP Pause ( Measure Pause Cur. EQP)	
	TO_CIM_EQPABORT,			//EQP Abort ( Measure Abort Cur. EQP)
	TO_CIM_EQPRESTART,			//EQP Restart ( Measure Restart Cur. EQP)
	TO_CIM_OPCALL,				//Operator call : 1 : ON, 0 : OFF
	TO_CIM_FORCEINIT_OK,		//FORCEINIT Response.Start Force Init
	TO_CIM_FORCEINIT_NG,		//FORCEINIT Response.Impossible FORCEINIT or EQ Offline Status
	TO_CIM_FORCEINIT_END,		//FORCEINIT Complete
	TO_CIM_REMEASURE,			//Remeasure Sequence
	TO_CIM_DAYCHK,				//DAYCHKREQ Report,#(5):TK Lamp Life Time,TR Lamp Life Time(reserved),CA Lamp Life Time(reserved),Main Air Gague(reserved),Vaccum Gauge(reserved),C Drive Free Space(reserved),Ex)DAYCHK^1500^ ^ ^ ^ ^

	TO_CIM_GDTCPL_TR_OK,		//sklee 20201104 add
	TO_CIM_GDTCPL_TR_NG,		//sklee 20201104 add

	CIM_CMD_SERVER_TO_CIM_END,
};


const enum CIM_CMD_CIM_TO_SERVER{
	TO_SERVER_ORG = 1,			//Loader zero postion move request(reserved)	
	TO_SERVER_MD,					//#(1)		MDREQ Response 1:Online, 0:Offline CIM Mode Status	
	TO_SERVER_AT,					//#(1)_OK$		Mode request  1 ; Online, 0 : Offline	
	TO_SERVER_EQPSTART,			//Online Start request	
	TO_SERVER_TIMESET,			//^YYYYMMDDHHMMSS$		Host Time Report	
	TO_SERVER_TIMERST,			//^YYYYMMDDHHMMSS$		Host TIMe Report ( EQP idle time )	
	TO_SERVER_LDREQ,				//Load Request to EQP	

	TO_SERVER_PPID ,				//#(20)$		PPID Report,#(20) : Recipe ID					 		
	TO_SERVER_LOTID,				//^#(30)^#(30)^#(30)$		LOTID report,#(30) : LOT ID#(30),  Port ID#(30),#(30) : Reserved													
	TO_SERVER_GLSID,				//^#(30)^#(30)^#(30)$		Glass ID report,#(30) : Glass ID#(30), Slot ID#(30),#(30) : Reserved		
														
	TO_SERVER_LDCPL,				//Glass Load Complete		
	TO_SERVER_PRCS,				//Process Start ( Measure Start )		
	TO_SERVER_GDTREQ,				//Glass Data Request		
	TO_SERVER_ULDREQ,				//Unload Request		
	TO_SERVER_ULDCPL,				//Unload Complete		
	TO_SERVER_GLSREADY,			//Next Glass Ready
	TO_SERVER_EQPABORT,			//EQP Abort Current Glass		
	TO_SERVER_EQPPAUSE,			//EQP Pause Current Glass		
	TO_SERVER_EQPRESTART,			//EQP Restart Current Glass		
	TO_SERVER_DAYCHKREQ,			//Daily Check Item Request		
	TO_SERVER_FORCEINIT,			//Force Initialize Request		
	TO_SERVER_OPCALL,				//Operator call : 1 : ON, 0 : OFF		
	TO_SERVER_MULTIRCP,			//_1$	Lift-Pin Up/Down Action Disable	,_0$ Lift-Pin Up/Down Action Enable	
	TO_SERVER_GDTCPL_TR,			//sklee 20201105 add

	TO_SERVER_INIT_START,
	TO_SERVER_MEAS_START,
	TO_SERVER_MEAS_END,

	TO_SERVER_ALARM,			// jsyoon 2022.10.04 add
	CIM_CMD_CIM_TO_SERVER_END,
};































#endif // DEFINESCIM_H_INCLUDED