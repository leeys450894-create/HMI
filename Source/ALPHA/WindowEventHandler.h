#pragma once

#include <map>
#include <vector>

enum eMessage
{
	WM_MSG_UPDATE_LOGIN_STATE = WM_USER + 100,
	WM_MSG_UPDATE_LOGIN_INFO,
	WM_MSG_UPDATE_CONNECT_STATE,
	WM_MSG_UPDATE_AUTO_CHANGE_INLINE_MODE,
	WM_MSG_UPDATE_CHANGE_NOT_INITIALIZE,
	WM_MSG_UPDATE_CHANGE_INITIALIZE,
	WM_MSG_UPDATE_CHANGE_VIEW_STATE,
	WM_MSG_UPDATE_RECIPE_INFORMATION,
	WM_MSG_UPDATE_REMOTE_INTERLOCK,
	WM_MSG_UPDATE_SEQ_ITEM_ELAPSED_TIME,
	WM_MSG_UPDATE_CYCLE_INSPECTION_START,
	WM_MSG_UPDATE_CYCLE_INSPECTION_RESULT,
	WM_MSG_UPDATE_CYCLE_INSPECTION_COMPLETE,
	WM_MSG_UPDATE_POSITION_STATE,
	WM_MSG_UPDATE_STATE_INITIALIZE,
	WM_MSG_UPDATE_LAMP_TIME,
	WM_MSG_WRITE_LAMP_TIME,
	WM_MSG_UPDATE_COLOR,
	WM_MSG_UPDATE_CLOSE_SYSTEM,
	WM_MSG_UPDATE_MEASURS_ELAPSED_TIME,
	WM_MSG_UPDATE_CHANGE_CIM_STATE,
	WM_MSG_UPDATE_CHANGE_CONTROLMODE,
	WM_MSG_CCD_LIVE,
	WM_MSG_CIM_ALARM,	// jsyoon 2022.10.04 add

	WM_MSG_LAST,
};

enum enumSeqItemMessage
{
	WM_MSG_SEQ_ITEM_COMPLTE = WM_USER + 300,
	WM_MSG_SEQ_ITEM_INITIALIZE_COMPLTE,
	WM_MSG_SEQ_ITEM_LOADING_COMPLTE,
	WM_MSG_SEQ_ITEM_UNLOADING_COMPLTE,
	WM_MSG_SEQ_ITEM_PPA_COMPLTE,
	WM_MSG_SEQ_ITEM_SE_COMPLTE,
	WM_MSG_SEQ_ITEM_TR_COMPLTE,
	WM_MSG_SEQ_ITEM_PACC_COMPLTE,
	WM_MSG_SEQ_ITEM_LAST,
};

enum enumWndID
{
	enumWndID_TITLE = 0,
	enumWndID_FRAME,
	enumWndIDCount
};

enum enumSeqItemID
{
	enumSeqItemID_READY_TO_LOADING = 0,
	enumSeqItemID_LOADING,
	enumSeqItemID_READY_TO_MEASURE,	
	enumSeqItemID_MEASURE,	
	enumSeqItemID_READY_TO_REPORT,	
	enumSeqItemID_REPORT,	
	enumSeqItemID_READY_TO_UNLOADING,	
	enumSeqItemID_UNLOADING,
	enumSeqItemIDCount,

	enumSeqItemID_GLASS_LOAD = 0,
	enumSeqItemID_GLASS_RECIPE_CHANGE,
	enumSeqItemID_GLASS_PRE_ALIGN,
	enumSeqItemID_GLASS_PRE_MEAS,
	enumSeqItemID_GLASS_AGING,
	enumSeqItemID_GLASS_ADSORPTION,
	enumSeqItemID_GLASS_SW_ALIGN,
	enumSeqItemID_GLASS_MEAS_STICK,
	enumSeqItemID_GLASS_MEASURE,
	enumSeqItemID_GLASS_RESULT,
	enumSeqItemID_GLASS_UNLOAD,
	
	enumSeqItemID_INITIALIZE = 0,
	enumSeqItemID_REQ_PACC,
	enumSeqItemID_REQ_AUTO_FOCUS,
	enumSeqItemID_REQ_ALIGNMENT,
	enumSeqItemID_REQ_EQ_STATE_CHANGE,
	enumSeqItemID_REQ_EQ_STATE,
	enumSeqItemID_REQ_EQ_INTERLOCK,
	enumSeqItemID_REQ_EQ_KMAC_IMAGE_AF,

	
	// Probe
	enumSeqItemID_PROBE_SW_ALIGN,

	// CIM
	enumSeqItemID_SEQ_CIM,

	//CCD LIVE
	enumSeqItemID_CCD_LIVE_START,
	enumSeqItemID_CCD_LIVE_STOP,


};

enum enumLogItemID
{
	enumLogItemID_DLOG = 0,
	enumLogItemID_TLOG,
	enumLogItemID_OLOG,
	enumLogItemID_MOTION,
	enumLogItemID_IO,
	enumLogItemID_SERIAL,
	enumLogItemID_CIM,
	enumLogItemID_MCC,
	enumLogItemID_PROBE,
	enumLogItemID_MEAS1,	// 화질
	enumLogItemID_MEAS2,    // 색이상
	enumLogItemID_MEAS3,    // 시야각
	enumLogItemID_MEAS4,    // PG
	enumLogItemID_MEAS5,    // 특성
	enumLogItemID_MEAS6,    // TSP / LCR
	enumLogItemID_MEAS7,	// AGIGN
	enumLogItemID_MEAS8,
	enumLogItemID_MEAS9,
	enumLogItemID_MEAS10,
	enumLogItemIDCount,
};

enum enumManualTemID
{
	enumManualTemID_MEAS = 0,
	enumManualTemID_SACC,
	enumManualTemID_IONIZER,
	enumManualTemID_AUTOCONTACT,
	enumManualTemID_AGINGTEST,
	enumManualTemID_AGINGSET,
	enumManualTemID_WADMULTI,	// LYS 2026-07-23 WAD Multi Meas
	enumManualTemID_AGINGNEW,	// LYS 2026-09-28 EFA
	enumManualTemIDCount,
};

class CWindowEventHandler
{
	HWND	m_Spec_MsgHandleWnd[enumWndIDCount];
	HWND	m_Spec_MsgHandleSeqItem[enumSeqItemIDCount];
	CRect	m_Spec_rtROI;

public:
	void SetMessageHandle(enumWndID id, HWND hWnd);
	LRESULT SendMessageToUI(enumWndID id, UINT idMessage, WPARAM wParam, LPARAM lParam);
	BOOL PostMessageToUI(enumWndID id, UINT idMessage, WPARAM wParam, LPARAM lParam);
	void SetMessageHandle(enumSeqItemID id, HWND hWnd);
	LRESULT SendMessageToUI(enumSeqItemID id, UINT idMessage, WPARAM wParam, LPARAM lParam);
	BOOL PostMessageToUI(enumSeqItemID id, UINT idMessage, WPARAM wParam, LPARAM lParam);
	CWnd* GetHandleWnd(enumWndID id);


    //----------------------------------------------------------------------------------------------------
    // Singleton
    //----------------------------------------------------------------------------------------------------
private:
    static CWindowEventHandler	*m_pInstance;
public:
    static CWindowEventHandler	*GetInstance();
    static void					DelInstance();
private:
    CWindowEventHandler(void);
    ~CWindowEventHandler(void);
};
