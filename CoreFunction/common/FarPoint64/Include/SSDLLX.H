//--------------------------------------------------------------------
//
// File: SSDLLX.H
//
// Copyright (C) 2004 FarPoint Technologies.
// All rights reserved.
//
//--------------------------------------------------------------------

#ifndef SSDLLX_H
#define SSDLLX_H

//--------------------------------------------------------------------

//  void OnAfterUserSort (HWND hWnd, UINT idCtl, LPSS_COORD lpCoord)
#define HANDLE_SSM_AFTERUSERSORT(hWnd, wParam, lParam, fn) \
  ((fn)((hWnd), (UINT)(wParam), (LPSS_COORD)(lParam)), 0L)
#define FORWARD_SSM_AFTERUSERSORT(hWnd, idCtl, lpCoord, fn) \
  (void)(fn)((hWnd), SSM_AFTERUSERSORT, (WPARAM)(idCtl), (LPARAM)(lpCoord))

//  void OnBeforeColMove (HWND hWnd, UINT idCtl, LPSS_BEFORECOLMOVE lpBeforeColMove)
#define HANDLE_SSM_BEFORECOLMOVE(hWnd, wParam, lParam, fn) \
  ((fn)((hWnd), (UINT)(wParam), (LPSS_BEFORECOLMOVE)(lParam)), 0L)
#define FORWARD_SSM_BEFORECOLMOVE(hWnd, idCtl, lpBeforeColMove, fn) \
  (void)(fn)((hWnd), SSM_BEFORECOLMOVE, (WPARAM)(idCtl), (LPARAM)(lpBeforeColMove))

//  void OnBeforeEditMode(HWND hWnd, UINT idCtl, LPSS_BEFOREEDITMODE lpBeforeEditMode)
#define HANDLE_SSM_BEFOREEDITMODE(hWnd, wParam, lParam, fn) \
  ((fn)((hWnd), (UINT)(wParam), (LPSS_BEFOREEDITMODE )(lParam)), 0L)
#define FORWARD_SSM_BEFOREEDITMODE(hWnd, idCtl, lpBeforeEditMode, fn) \
  (void)(fn)((hWnd), SSM_BEFOREEDITMODE, (WPARAM)(idCtl), (LPARAM)(lpBeforeEditMode))

//  void OnBeforeRowMove (HWND hWnd, UINT idCtl, LPSS_BEFOREROWMOVE lpBeforeRowMove)
#define HANDLE_SSM_BEFOREROWMOVE(hWnd, wParam, lParam, fn) \
  ((fn)((hWnd), (UINT)(wParam), (LPSS_BEFOREROWMOVE)(lParam)), 0L)
#define FORWARD_SSM_BEFOREROWMOVE(hWnd, idCtl, lpBeforeRowMove, fn) \
  (void)(fn)((hWnd), SSM_BEFOREROWMOVE, (WPARAM)(idCtl), (LPARAM)(lpBeforeRowMove))

//  void OnBeforeScrollTip(HWND hWnd, UINT idCtl, LPSS_BEFORESCROLLTIP lpBeforeScrollTip)
#define HANDLE_SSM_BEFORESCROLLTIP(hWnd, wParam, lParam, fn) \
  ((fn)((hWnd), (UINT)(wParam), (LPSS_BEFORESCROLLTIP )(lParam)), 0L)
#define FORWARD_SSM_BEFORESCROLLTIP(hWnd, idCtl, lpBeforeScrollTip, fn) \
  (void)(fn)((hWnd), SSM_BEFORESCROLLTIP, (WPARAM)(idCtl), (LPARAM)(lpBeforeScrollTip))

//  void OnBeforeUserSort (HWND hWnd, UINT idCtl, LPSS_BEFOREUSERSORT lpBeforeUserSort)
#define HANDLE_SSM_BEFOREUSERSORT(hWnd, wParam, lParam, fn) \
  ((fn)((hWnd), (UINT)(wParam), (LPSS_BEFOREUSERSORT)(lParam)), 0L)
#define FORWARD_SSM_BEFOREUSERSORT(hWnd, idCtl, lpBeforeUserSort, fn) \
  (void)(fn)((hWnd), SSM_BEFOREUSERSORT, (WPARAM)(idCtl), (LPARAM)(lpBeforeUserSort))

//  void OnCellLocked (HWND hWnd, UINT idCtl, LPSS_CELLCOORD lpCellCoord)
#define HANDLE_SSM_CELLLOCKED(hWnd, wParam, lParam, fn) \
  ((fn)((hWnd), (UINT)(wParam), (LPSS_CELLCOORD)(lParam)), 0L)
#define FORWARD_SSM_CELLLOCKED(hWnd, idCtl, lpCellCoord, fn) \
  (void)(fn)((hWnd), SSM_CELLLOCKED, (WPARAM)(idCtl), (LPARAM)(lpCellCoord))

//  BOOL OnClick (HWND hWnd, UINT idCtl, LPSS_CELLCOORD lpCellCoord)
#define HANDLE_SSM_CLICK(hWnd, wParam, lParam, fn) \
  (LRESULT)(fn)((hWnd), (UINT)(wParam), (LPSS_CELLCOORD)(lParam))
#define FORWARD_SSM_CLICK(hWnd, idCtl, fn) \
  (BOOL)(fn)((hWnd), SSM_CLICK, (WPARAM)(idCtl), (LPARAM)(lpCellCoord))

//  void OnColRestricted (HWND hWnd, UINT idCtl, SS_COORD Col)
#define HANDLE_SSM_COLRESTRICTED(hWnd, wParam, lParam, fn) \
  ((fn)((hWnd), (UINT)(wParam), (SS_COORD)(lParam)), 0L)
#define FORWARD_SSM_COLRESTRICTED(hWnd, idCtl, Col, fn) \
  (void)(fn)((hWnd), SSM_COLRESTRICTED, (WPARAM)(idCtl), (LPARAM)(Col))

//  void OnColRowRestricted (HWND hWnd, UINT idCtl, LPSS_CELLCOORD lpCellCoord)
#define HANDLE_SSM_COLROWRESTRICTED(hWnd, wParam, lParam, fn) \
  ((fn)((hWnd), (UINT)(wParam), (LPSS_CELLCOORD)(lParam)), 0L)
#define FORWARD_SSM_COLROWRESTRICTED(hWnd, idCtl, lpCellCoord, fn) \
  (void)(fn)((hWnd), SSM_COLROWRESTRICTED, (WPARAM)(idCtl), (LPARAM)(lpCellCoord))

//  void OnColWidthChange (HWND hWnd, UINT idCtl, LPSS_COORDRANGE lpCoordRange)
#define HANDLE_SSM_COLWIDTHCHANGE(hWnd, wParam, lParam, fn) \
  ((fn)((hWnd), (UINT)(wParam), (LPSS_COORDRANGE)(lParam)), 0L)
#define FORWARD_SSM_COLWIDTHCHANGE(hWnd, idCtl, lpCoordRange, fn) \
  (void)(fn)((hWnd), SSM_COLWIDTHCHANGE, (WPARAM)(idCtl), (LPARAM)(lpCoordRange))

//  void OnComboCloseUp (HWND hWnd, UINT idCtl,LPSS_COMBOCLOSEUP lpCloseUp)
#define HANDLE_SSM_COMBOCLOSEUP(hWnd, wParam, lParam, fn) \
  ((fn)((hWnd), (UINT)(wParam), (LPSS_COMBOCLOSEUP)(lParam)), 0L)
#define FORWARD_SSM_COMBOCLOSEUP(hWnd, idCtl, lpCloseUp, fn) \
  (void)(fn)((hWnd), SSM_COMBOCLOSEUP, (WPARAM)(idCtl), (LPARAM)(lpCloseUp))

//  void OnComboDropDown (HWND hWnd, UINT idCtl,LPSS_CELLCOORD lpCellCoord)
#define HANDLE_SSM_COMBODROPDOWN(hWnd, wParam, lParam, fn) \
  ((fn)((hWnd), (UINT)(wParam), (LPSS_CELLCOORD)(lParam)), 0L)
#define FORWARD_SSM_COMBODROPDOWN(hWnd, idCtl, lpCellCoord, fn) \
  (void)(fn)((hWnd), SSM_COMBODROPDOWN, (WPARAM)(idCtl), (LPARAM)(lpCellCoord))

//  void OnComboSelChange (HWND hWnd, UINT idCtl,LPSS_CELLCOORD lpCellCoord)
#define HANDLE_SSM_COMBOSELCHANGE(hWnd, wParam, lParam, fn) \
  ((fn)((hWnd), (UINT)(wParam), (LPSS_CELLCOORD)(lParam)), 0L)
#define FORWARD_SSM_COMBOSELCHANGE(hWnd, idCtl, lpCellCoord, fn) \
  (void)(fn)((hWnd), SSM_COMBOSELCHANGE, (WPARAM)(idCtl), (LPARAM)(lpCellCoord))

//  void OnDataChange (HWND hWnd, UINT idCtl, LPSS_CELLCOORD lpCellCoord)
#define HANDLE_SSM_DATACHANGE(hWnd, wParam, lParam, fn) \
    ((fn)((hWnd), (UINT)(wParam), (LPSS_CELLCOORD)(lParam)), 0L)
#define FORWARD_SSM_DATACHANGE(hWnd, idCtl, lpCellCoord, fn) \
    (void)(fn)((hWnd), SSM_DATACHANGE, (WPARAM)(idCtl), (LPARAM)(lpCellCoord))

//  BOOL OnDblClk (HWND hWnd, UINT idCtl, LPSS_CELLCOORD lpCellCoord)
#define HANDLE_SSM_DBLCLK(hWnd, wParam, lParam, fn) \
  (LRESULT)(fn)((hWnd), (UINT)(wParam), (LPSS_CELLCOORD)(lParam))
#define FORWARD_SSM_DBLCLK(hWnd, idCtl, lpCellCoord, fn) \
  (BOOL)(fn)((hWnd), SSM_DBLCLK, (WPARAM)(idCtl), (LPARAM)(lpCellCoord))

//  void OnDragDrop (HWND hWnd, UINT idCtl, LPSS_DRAGDROP)
#define HANDLE_SSM_DRAGDROP(hWnd, wParam, lParam, fn) \
  ((fn)((hWnd), (UINT)(wParam), (LPSS_DRAGDROP)(lParam)), 0L)
#define FORWARD_SSM_DRAGDROP(hWnd, idCtl, lpDragDrop, fn) \
  (void)(fn)((hWnd), SSM_DRAGDROP, (WPARAM)(idCtl), (LPARAM)(lpDragDrop))

//  void OnDrawItem (HWND hWnd, LPSS_DRAWITEMSTRUCT lpDrawItem)
#define HANDLE_SSM_DRAWITEM(hWnd, wParam, lParam, fn) \
  ((fn)((hWnd), (LPSS_DRAWITEMSTRUCT)(lParam)), 0L)
#define FORWARD_SSM_DRAWITEM(hWnd, lpDrawItem, fn) \
  (void)(fn)((hWnd), SSM_DRAWITEM, (WPARAM)0, (LPARAM)(lpDrawItem))

//  void OnEditChange (HWND hWnd, UINT idCtl, LPSS_CELLCOORD lpCellCoord)
#define HANDLE_SSM_EDITCHANGE(hWnd, wParam, lParam, fn) \
  ((fn)((hWnd), (UINT)(wParam), (LPSS_CELLCOORD)(lParam)), 0L)
#define FORWARD_SSM_EDITCHANGE(hWnd, idCtl, lpCellCoord, fn) \
  (void)(fn)((hWnd), SSM_EDITCHANGE, (WPARAM)(idCtl), (LPARAM)(lpCellCoord))

//  void OnEditModeOff (HWND hWnd, UINT idCtl, LPSS_EDITMODEOFF lpEditModeOff)
#define HANDLE_SSM_EDITMODEOFF(hWnd, wParam, lParam, fn) \
  ((fn)((hWnd), (UINT)(wParam), (LPSS_EDITMODEOFF)(lParam)), 0L)
#define FORWARD_SSM_EDITMODEOFF(hWnd, idCtl, lpEditModeOff, fn) \
  (void)(fn)((hWnd), SSM_EDITMODEOFF, (WPARAM)(idCtl), (LPARAM)(lpEditModeOff))

//  void OnEditModeOn (HWND hWnd, UINT idCtl, LPSS_CELLCOORD lpCellCoord)
#define HANDLE_SSM_EDITMODEON(hWnd, wParam, lParam, fn) \
  ((fn)((hWnd), (UINT)(wParam), (LPSS_CELLCOORD)(lParam)), 0L)
#define FORWARD_SSM_EDITMODEON(hWnd, idCtl, lpCellCoord, fn) \
  (void)(fn)((hWnd), SSM_EDITMODEON, (WPARAM)(idCtl), (LPARAM)(lpCellCoord))

//  void OnEnterRow (HWND hWnd, UINT idCtl, LPSS_ENTERROW lpEnterRow)
#define HANDLE_SSM_ENTERROW(hWnd, wParam, lParam, fn) \
  ((fn)((hWnd), (UINT)(wParam), (LPSS_ENTERROW)(lParam)), 0L)
#define FORWARD_SSM_ENTERROW(hWnd, idCtl, lpEnterRow, fn) \
  (void)(fn)((hWnd), SSM_ENTERROW, (WPARAM)(idCtl), (LPARAM)(lpEnterRow))

//  BOOL OnKeyDown (HWND hWnd, UINT idCtl, UINT VirtualKey, UINT Flags)
#define HANDLE_SSM_KEYDOWN(hWnd, wParam, lParam, fn) \
  (fn)((hWnd), (UINT)(wParam), (UINT)LOWORD(lParam), (UINT)HIWORD(lParam))
#define FORWARD_SSM_KEYDOWN(hWnd, idCtl, VirtualKey, Flags, fn) \
  (BOOL)(fn)((hWnd), SSM_KEYDOWN, (WPARAM)(idCtl), MAKELPARAM((VirtualKeys),(Flags)))

//  void OnKeyPress (HWND hWnd, UINT idCtl, UINT* lpKey)
#define HANDLE_SSM_KEYPRESS(hWnd, wParam, lParam, fn) \
  ((fn)((hWnd), (UINT)(wParam), (UINT*)(lParam)), 0)
#define FORWARD_SSM_KEYPRESS(hWnd, idCtl, lpKey, fn) \
  (void)(fn)((hWnd), SM_KEYPRESS, (WPARAM)(idCtl), (LPARAM)(lpKey))

//  void OnKeyUp (HWND hWnd, UINT idCtl, UNIT VirtualKey, UINT Flags);
#define HANDLE_SSM_KEYUP(hWnd, wParam, lParam, fn) \
  ((fn)((hWnd), (UINT)(wParam), (UINT)LOWORD(lParam), (UINT)HIWORD(lParam)), 0)
#define FORWARD_SSM_KEYUP(hWnd, idCtl, VirtualKey, Flags, fn) \
  (void)(fn)((hWnd), SSM_KEYUP, (WPARAM)(idCtl), MAKELPARAM((VirtualKeys),(Flags)))

//  BOOL OnLeaveCell (HWND hWnd, UINT idCtl, LPSS_LEAVECELL lpLeaveCell)
#define HANDLE_SSM_LEAVECELL(hWnd, wParam, lParam, fn) \
  (LRESULT)(fn)((hWnd), (UINT)(wParam), (LPSS_LEAVECELL)(lParam))
#define FORWARD_SSM_LEAVECELL(hWnd, idCtl, lpLeaveCell, fn) \
  (BOOL)(fn)((hWnd), SSM_LEAVECELL, (WPARAM)(idCtl), (LPARAM)(lpLeaveCell))

//  void OnLeaveRow (HWND hWnd, UINT idCtl, LPSS_LEAVEROW lpLeaveRow)
#define HANDLE_SSM_LEAVEROW(hWnd, wParam, lParam, fn) \
  ((fn)((hWnd), (UINT)(wParam), (LPSS_LEAVEROW)(lParam)), 0L)
#define FORWARD_SSM_LEAVEROW(hWnd, idCtl, lpLeaveRow, fn) \
  (void)(fn)((hWnd), SSM_LEAVEROW, (WPARAM)(idCtl), (LPARAM)(lpLEaveRow))

//  void OnRButton (HWND hWnd, UINT idCtl, LPSS_RBUTTON lpRButton)
#define HANDLE_SSM_RBUTTON(hWnd, wParam, lParam, fn) \
  ((fn)((hWnd), (UINT)(wParam), (LPSS_RBUTTON)(lParam)), 0L)
#define FORWARD_SSM_RBUTTON(hWnd, idCtl, lpRButton, fn) \
  (void)(fn)((hWnd), SSM_RBUTTON, (WPARAM)(idCtl), (LPARAM)(lpRButton))

//  void OnRowHeightChange (HWND hWnd, UINT idCtl, LPSS_COORDRANGE lpCoordRange)
#define HANDLE_SSM_ROWHEIGHTCHANGE(hWnd, wParam, lParam, fn) \
  ((fn)((hWnd), (UINT)(wParam), (LPSS_COORDRANGE)(lParam)), 0L)
#define FORWARD_SSM_ROWHEIGHTCHANGE(hWnd, idCtl, lpCoordRange, fn) \
  (void)(fn)((hWnd), SSM_ROWHEIGHTCHANGE, (WPARAM)(idCtl), (LPARAM)(lpCoordRange))

//  void OnRowRestricted (HWND hWnd, UINT idCtl, SS_COORD Row)
#define HANDLE_SSM_ROWRESTRICTED(hWnd, wParam, lParam, fn) \
  ((fn)((hWnd), (UINT)(wParam), (SS_COORD)(lParam)), 0L)
#define FORWARD_SSM_ROWRESTRICTED(hWnd, idCtl, Row, fn) \
  (void)(fn)((hWnd), SSM_ROWRESTRICTED, (WPARAM)(idCtl), (LPARAM)(Row))

//  void OnSheetChanged (HWND hWnd, UINT idCtl, short nOldSheet, short nNewSheet)
#define HANDLE_SSM_SHEETCHANGED(hWnd, wParam, lParam, fn) \
  ((fn)((hWnd), (UINT)(wParam), (short)LOWORD(lParam), (short)HIWORD(lParam)), 0L)
#define FORWARD_SSM_SHEETCHANGED(hWnd, idCtl, nOldSheet, nNewSheet, fn) \
  (void)(fn)((hWnd), SSM_SHEETCHANGED, (WPARAM)(idCtl), MAKEWPARAM((nOldSheet), (nNewSheet)))

//  BOOL OnSheetChanging (HWND hWnd, UINT idCtl, short nOldSheet, short nNewSheet)
#define HANDLE_SSM_SHEETCHANGING(hWnd, wParam, lParam, fn) \
  (fn)((hWnd), (UINT)(wParam), (short)LOWORD(lParam), (short)HIWORD(lParam))
#define FORWARD_SSM_SHEETCHANGING(hWnd, idCtl, nOldSheet, nNewSheet, fn) \
  (BOOL)(fn)((hWnd), SSM_SHEETCHANGING, (WPARAM)(idCtl), MAKEWPARAM((nOldSheet), (nNewSheet)))

//  void OnTabScrolled (HWND hWnd, UINT idCtl, nOldLeftSheet, short nNewLeftSheet)
#define HANDLE_SSM_TABSCROLLED(hWnd, wParam, lParam, fn) \
  ((fn)((hWnd), (UINT)(wParam), (short)LOWORD(lParam), (short)HIWORD(lParam)), 0L)
#define FORWARD_SSM_TABSCROLLED(hWnd, idCtl, nOldLeftSheet, nNewLeftSheet, fn) \
  (void)(fn)((hWnd), SSM_TABSCROLLED, (WPARAM)(idCtl), MAKEWPARAM((nOldLeftSheet), (nNewLeftSheet)))

//  void OnTabScrolling (HWND hWnd, UINT idCtl, short nOldLeftSheet, short nNewLeftSheet)
#define HANDLE_SSM_TABSCROLLING(hWnd, wParam, lParam, fn) \
  ((fn)((hWnd), (UINT)(wParam), (short)LOWORD(lParam), (short)HIWORD(lParam)), 0L)
#define FORWARD_SSM_TABSCROLLING(hWnd, idCtl, nOldLeftSheet, nNewLeftSheet, fn) \
  (void)(fn)((hWnd), SSM_TABSCROLLING, (WPARAM)(idCtl), MAKEWPARAM((nOldLeftSheet), (nNewLeftSheet)))

//  void OnTextTipFetch (HWND hWnd, UINT idCtl, LPSS_TEXTTIPFETCH lpTextTipFetch)
#define HANDLE_SSM_TEXTTIPFETCH(hWnd, wParam, lParam, fn) \
  ((fn)((hWnd), (UINT)(wParam), (LPSS_TEXTTIPFETCH)(lParam)), 0L)
#define FORWARD_SSM_TEXTTIPFETCH(hWnd, idCtl, lpTextTipFetch, fn) \
  (void)(fn)((hWnd), SSM_TEXTTIPFETCH, (WPARAM)(idCtl), (LPARAM)(lpTextTipFetch))
 
//  BOOL OnVClearData (HWND hWnd, UNIT idCtl, LPSS_VCLEARDATA lpVClearData)
#define HANDLE_SSM_VCLEARDATA(hWnd, wParam, lParam, fn) \
  (fn)((hWnd), (UINT)(wParam), (LPSS_VCLEARDATA)(lParam))
#define FORWARD_SSM_VCLEARDATA(hWnd, idCtl, lpVQueryData, fn) \
  (BOOL)(fn)((hWnd), SSM_VCLEARDATA, (WPARAM)(idCtl), (LPARAM)(lpVClearData))
  
//  void OnVQueryData (HWND hWnd, UINT idCtl, LPSS_VQUERYDATA lpVQueryData)
#define HANDLE_SSM_VQUERYDATA(hWnd, wParam, lParam, fn) \
  ((fn)((hWnd), (UINT)(wParam), (LPSS_VQUERYDATA)(lParam)), 0L)
#define FORWARD_SSM_VQUERYDATA(hWnd, idCtl, lpVQueryData, fn) \
  (void)(fn)((hWnd), SSM_VQUERYDATA, (WPARAM)(idCtl), (LPARAM)(lpVQueryData))

//-------- Alternate porting layer macros --------------------------
#define GET_SSM_AFTERUSERSORT_IDCTL(wParam, lParam) (UINT)(wParam)
#define GET_SSM_AFTERUSERSORT_LPSS_COORD(wParam, lParam) (LPSS_COORD)(lParam)
#define GET_SSM_AFTERUSERSORT_MPS(idCtl, lpCoord) \
        (WPARAM)(idCtl), (LPARAM)(lpCoord)

#define GET_SSM_BEFORECOLMOVE_IDCTL(wParam, lParam) (UINT)(wParam)
#define GET_SSM_BEFORECOLMOVE_LPSS_BEFORECOLMOVE(wParam, lParam) (LPSS_BEFORECOLMOVE)(lParam)
#define GET_SSM_BEFORECOLMOVE_MPS(idCtl, lpBeforeColMove) \
        (WPARAM)(idCtl), (LPARAM)(lpBeforeColMove)

#define GET_SSM_BEFOREEDITMODE_IDCTL(wParam, lParam) (UINT)(wParam)
#define GET_SSM_BEFOREEDITMODE_LPSS_BEFOREEDITMODE(wParam, lParam) (LPSS_BEFOREEDITMODE)(lParam)
#define GET_SSM_BEFOREEDITMODE_MPS(idCtl, lpBeforeEditMode) \
        (WPARAM)(idCtl), (LPARAM)(lpBeforeEditMode)

#define GET_SSM_BEFOREROWMOVE_IDCTL(wParam, lParam) (UINT)(wParam)
#define GET_SSM_BEFOREROWMOVE_LPSS_BEFOREROWMOVE(wParam, lParam) (LPSS_BEFOREROWMOVE)(lParam)
#define GET_SSM_BEFOREROWMOVE_MPS(idCtl, lpBeforeRowMove) \
        (WPARAM)(idCtl), (LPARAM)(lpBeforeRowMove)

#define GET_SSM_BEFORESCROLLTIP_IDCTL(wParam, lParam) (UINT)(wParam)
#define GET_SSM_BEFORESCROLLTIP_LPSS_BEFORESCROLLTIP(wParam, lParam) (LPSS_BEFORESCROLLTIP)(lParam)
#define GET_SSM_BEFORESCROLLTIP_MPS(idCtl, lpBeforeScrollTip) \
        (WPARAM)(idCtl), (LPARAM)(lpBeforeScrollTip)

#define GET_SSM_BEFOREUSERSORT_IDCTL(wParam, lParam) (UINT)(wParam)
#define GET_SSM_BEFOREUSERSORT_LPSS_BEFOREUSERSORT(wParam, lParam) (LPSS_BEFOREUSERSORT)(lParam)
#define GET_SSM_BEFOREUSERSORT_MPS(idCtl, lpBeforeUserSort) \
        (WPARAM)(idCtl), (LPARAM)(lpBeforeUserSort)

#define GET_SSM_CELLLOCKED_IDCTL(wParam, lParam) (UINT)(wParam)
#define GET_SSM_CELLLOCKED_LPCELLCOORD(wParam, lParam) (LPSS_CELLCOORD)(lParam)
#define GET_SSM_CELLLOCKED_MPS(idCtl, lpCellCoord) \
        (WPARAM)(idCtl), (LPARAM)(lpCellCoord)

#define GET_SSM_CLICK_IDCTL(wParam, lParam) (UINT)(wParam)
#define GET_SSM_CLICK_LPCELLCOORD(wParam, lParam) (LPSS_CELLCOORD)(lParam)
#define GET_SSM_CLICK_MPS(idCtl, lpCellCoord) \
        (WPARAM)(idCtl), (LPARAM)(lpCellCoord)

#define GET_SSM_COLRESTRICTED_IDCTL(wParam, lParam) (UINT)(wParam)
#define GET_SSM_COLRESTRICTED_COL(wParam, lParam) (SS_COORD)(lParam)
#define GET_SSM_COLRESTRICTED_MPS(idCtl, Col) \
        (WPARAM)(idCtl), (LPARAM)(Col)

#define GET_SSM_COLROWRESTRICTED_IDCTL(wParam, lParam) (UINT)(wParam)
#define GET_SSM_COLROWRESTRICTED_LPCELLCOORD(wParam, lParam) (LPSS_CELLCOORD)(lParam)
#define GET_SSM_COLROWRESTRICTED_MPS(idCtl, lpCellCoord) \
        (WPARAM)(idCtl), (LPARAM)(lpCellCoord)

#define GET_SSM_COLWIDTHCHANGE_IDCTL(wParam, lParam) (UINT)(wParam)
#define GET_SSM_COLWIDTHCHANGE_LPCOORDRANGE(wParam, lParam) (LPSS_COORDRANGE)(lParam)
#define GET_SSM_COLWIDTHCHANGE_MPS(idCtl, lpCoordRange) \
        (WPARAM)(idCtl), (LPARAM)(lpCoordRange)

#define GET_SSM_COMBOCLOSEUP_IDCTL(wParam, lParam) (UINT)(wParam)
#define GET_SSM_COMBOCLOSEUP_LPCOMBOCLOSEUP(wParam, lParam) (LPSS_COMBOCLOSEUP)(lParam)
#define GET_SSM_COMBOCLOSEUP_MPS(idCtl, lpCloseUp) \
        (WPARAM)(idCtl), (LPARAM)(lpCloseUp)

#define GET_SSM_COMBODROPDOWN_IDCTL(wParam, lParam) (UINT)(wParam)
#define GET_SSM_COMBODROPDOWN_LPCELLCOORD(wParam, lParam) (LPSS_CELLCOORD)(lParam)
#define GET_SSM_COMBODROPDOWN_MPS(idCtl, lpCellCoord) \
        (WPARAM)(idCtl), (LPARAM)(lpCellCoord)

#define GET_SSM_COMBOSELCHANGE_IDCTL(wParam, lParam) (UINT)(wParam)
#define GET_SSM_COMBOSELCHANGE_LPCELLCOORD(wParam, lParam) (LPSS_CELLCOORD)(lParam)
#define GET_SSM_COMBOSELCHANGE_MPS(idCtl, lpCellCoord) \
        (WPARAM)(idCtl), (LPARAM)(lpCellCoord)

#define GET_SSM_DATACHANGE_IDCTL(wParam, lParam) (UINT)(wParam)
#define GET_SSM_DATACHANGE_LPCELLCOORD(wParam, lParam) (LPSS_CELLCOORD)(lParam)
#define GET_SSM_DATACHANGE_MPS(idCtl, lpCellCoord) \
        (WPARAM)(idCtl), (LPARAM)(lpCellCoord)

#define GET_SSM_DBLCLK_IDCTL(wParam, lParam) (UINT)(wParam)
#define GET_SSM_DBLCLK_LPCELLCOORD(wParam, lParam) (LPSS_CELLCOORD)(lParam)
#define GET_SSM_DBLCLK_MPS(idCtl, lpCellCoord) \
        (WPARAM)(idCtl), (LPARAM)(lpCellCoord)

#define GET_SSM_DRAGDROP_IDCTL(wParam, lParam) (UINT)(wParam)
#define GET_SSM_DRAGDROP_LPDRAGDROP(wParam, lParam) (LPSS_DRAGDROP)(lParam)
#define GET_SSM_DRAGDROP_MPS(idCtl, lpDragDrop) \
        (WPARAM)(idCtl), (LPARAM)(lpDragDrop)

#define GET_SSM_DRAWITEM_LPDRAWITEM(wParam, lParam) (LPSS_DRAWITEMSTRUCT)(lParam)
#define GET_SSM_DRAWITEM_MPS(lpDrawItem) \
        (WPARAM)(0), (LPARAM)(lpDrawItem)

#define GET_SSM_EDITCHANGE_IDCTL(wParam, lParam) (UINT)(wParam)
#define GET_SSM_EDITCHANGE_LPCELLCOORD(wParam, lParam) (LPSS_CELLCOORD)(lParam)
#define GET_SSM_EDITCHANGE_MPS(idCtl, lpCellCoord) \
        (WPARAM)(idCtl), (LPARAM)(lpCellCoord)

#define GET_SSM_EDITMODEOFF_IDCTL(wParam, lParam) (UINT)(wParam)
#define GET_SSM_EDITMODEOFF_LPEDITMODEOFF(wParam, lParam) (LPSS_EDITMODEOFF)(lParam)
#define GET_SSM_EDITMODEOFF_MPS(idCtl, lpEditModeOff) \
        (WPARAM)(idCtl), (LPARAM)(lpEditModeOff)

#define GET_SSM_EDITMODEON_IDCTL(wParam, lParam) (UINT)(wParam)
#define GET_SSM_EDITMODEON_LPCELLCOORD(wParam, lParam) (LPSS_CELLCOORD)(lParam)
#define GET_SSM_EDITMODEON_MPS(idCtl, lpCellCoord) \
        (WPARAM)(idCtl), (LPARAM)(lpCellCoord)

#define GET_SSM_ENTERROW_IDCTL(wParam, lParam) (UINT)(wParam)
#define GET_SSM_ENTERROW_LPENTERROW(wParam, lParam) (LPSS_ENTERROW)(lParam)
#define GET_SSM_ENTERROW_MPS(idCtl, lpEnterRow) \
        (WPARAM)(idCtl), (LPARAM)(lpEnterRow)

#define GET_SSM_KEYDOWN_IDCTL(wParam, lParam) (UINT)(wParam)
#define GET_SSM_KEYDOWN_VIRTUALKEY(wParam, lParam) (UINT)LOWORD(lParam)
#define GET_SSM_KEYDOWN_FLAGS(wParam, lParam) (UINT)HIWORD(lParam)
#define GET_SSM_KEYDOWN_MPS(idCtl, VirtualKey, Flags) \
        (WPARAM)(idCtl), MAKELPARAM(VirtualKey, Flags)

#define GET_SSM_LEAVECELL_IDCTL(wParam, lParam) (UINT)(wParam)
#define GET_SSM_LEAVECELL_LPLEAVECELL(wParam, lParam) (LPSS_LEAVECELL)(lParam)
#define GET_SSM_LEAVECELL_MPS(idCtl, lpLeaveCell) \
        (WPARAM)(idCtl), (LPARAM)(lpLeaveCell)

#define GET_SSM_LEAVEROW_IDCTL(wParam, lParam) (UINT)(wParam)
#define GET_SSM_LEAVEROW_LPLEAVEROW(wParam, lParam) (LPSS_LEAVEROW)(lParam)
#define GET_SSM_LEAVEROW_MPS(idCtl, lpLeaveRow) \
        (WPARAM)(idCtl), (LPARAM)(lpLeaveRow)

#define GET_SSM_RBUTTON_IDCTL(wParam, lParam) (UNIT)(wParam)
#define GET_SSM_RBUTTON_LPRBUTTON(wParam, lParam) (LPSS_RBUTTON)(lParam)
#define GET_SSM_RBUTTON_MPS(idCtl, lpRButton) \
        (WPARAM)(idCtl), (LPARAM)(lpRButton)

#define GET_SSM_ROWHEIGHTCHANGE_IDCTL(wParam, lParam) (UINT)(idCtl)
#define GET_SSM_ROWHEIGHTCHANGE_LPCOORDRANGE(wParam, lParam) (LPSS_COORDRANGE)(lParam)
#define GET_SSM_ROWHEIGHTCHANGE_MPS(idCtl, lpCoordRange) \
        (WPARAM)(idCtl), (LPARAM)(lpCoordRange)

#define GET_SSM_ROWRESTRICTED_IDCTL(wParam, lParam) (UINT)(wParam)
#define GET_SSM_ROWRESTRICTED_ROW(wParam, lParam) (SS_COORD)(lParam)
#define GET_SSM_ROWRESTRICTED_MPS(idCtl, Row) \
        (WPARAM)(idCtl), (LPARAM)(Row)

#define GET_SSM_SHEETCHANGED_IDCTL(wParam, lParam) (UINT)(wParam)
#define GET_SSM_SHEETCHANGED_OLDSHEET(wParam, lParam) (short)LOWORD(lParam)
#define GET_SSM_SHEETCHANGED_NEWSHEET(wParam, lParam) (short)HIWORD(lParam)
#define GET_SSM_SHEETCHANGED_MPS(idCtl, nOldSheet, nNewSheet) \
        (WPARAM)(idCtl), MAKEWPARAM((nOldSheet),(nNewSheet))

#define GET_SSM_SHEETCHANGING_IDCTL(wParam, lParam) (UINT)(wParam)
#define GET_SSM_SHEETCHANGING_OLDSHEET(wParam, lParam) (short)LOWORD(lParam)
#define GET_SSM_SHEETCHANGING_NEWSHEET(wParam, lParam) (short)HIWORD(lParam)
#define GET_SSM_SHEETCHANGING_MPS(idCtl, nOldSheet, nNewSheet) \
        (WPARAM)(idCtl), MAKEWPARAM((nOldSheet),(nNewSheet))

#define GET_SSM_TABSCROLLED_IDCTL(wParam, lParam) (UINT)(wParam)
#define GET_SSM_TABSCROLLED_OLDLEFTSHEET(wParam, lParam) (short)LOWORD(lParam)
#define GET_SSM_TABSCROLLED_NEWLEFTSHEET(wParam, lParam) (short)HIWORD(lParam)
#define GET_SSM_TABSCROLLED_MPS(idCtl, nOldLeftSheet, nNewLeftSheet) \
        (WPARAM)(idCtl), MAKEWPARAM((nOldLeftSheet),(nNewLeftSheet))

#define GET_SSM_TABSCROLLING_IDCTL(wParam, lParam) (UINT)(wParam)
#define GET_SSM_TABSCROLLING_OLDLEFTSHEET(wParam, lParam) (short)LOWORD(lParam)
#define GET_SSM_TABSCROLLING_NEWLEFTSHEET(wParam, lParam) (short)HIWORD(lParam)
#define GET_SSM_TABSCROLLING_MPS(idCtl, nOldLeftSheet, nNewLeftSheet) \
        (WPARAM)(idCtl), MAKEWPARAM((nOldLeftSheet),(nNewLeftSheet))

#define GET_SSM_TEXTTIPFETCH_IDCTL(wParam, lParam) (UINT)(idCtl)
#define GET_SSM_TEXTTIPFETCH_LPTEXTTIPFETCH(wParam, lParam) (LPSS_TEXTTIPFETCH)(lParam)
#define GET_SSM_TEXTTIPFETCH_MPS(idCtl, lpTextTipFetch) \
        (WPARAM)(idCtl), (LPARAM)(lpTextTipFetch)

#define GET_SSM_VCLEARDATA_IDCTL(wParam, lParam) (UINT)(wParam)
#define GET_SSM_VCLEARDATA_LPVCLEARDATA(wParam, lParam) (LPSS_VCLEARDATA)(lParam)
#define GET_SSM_VCLEARDATA_MPS(idCtl, lpVClearData) \
        (WPARAM)(idCtl), (LPARAM)(lpVClearData)

#define GET_SSM_VQUERYDATA_IDCTL(wParam, lParam) (UINT)(wParam)
#define GET_SSM_VQUERYDATA_LPVQUERYDATA(wParam, lParam) (LPSS_VQUERYDATA)(lParam)
#define GET_SSM_VQUERYDATA_MPS(idCtl, lpVQueryData) \
        (WPARAM)(idCtl), (LPARAM)(lpVQueryData)

//--------------------------------------------------------------------

#endif
