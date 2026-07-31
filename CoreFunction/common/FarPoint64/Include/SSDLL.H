//--------------------------------------------------------------------
//
// File: SSDLL.H
//
// Copyright (C) 2004 FarPoint Technologies.
// All rights reserved.
//
//--------------------------------------------------------------------

#ifndef SSDLL_H
#define SSDLL_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef RC_INVOKED
#if defined(_WIN64) || defined(_IA64)
#pragma pack(8)
#else
#pragma pack(1)
#endif
#endif  /* RC_INVOKED */

//--------------------------------------------------------------------
//
//  Class name used in resource script or CreateWindow API call.
//
#if _IA64
#if defined(UNICODE) || defined(_UNICODE)
      #define SS_CLASSNAME _T("fpUSpread80")
      #define SPV_CLASSNAME _T("fpSpreadPreview80")
      #define SS_DLLNAME _T("SPRIA64D80.DLL")
    #else
      #define SS_CLASSNAME "fpSpread80"
      #define SPV_CLASSNAME "fpSpreadPreview80"
      #define SS_DLLNAME "SPRIA64DU80.DLL"
 #endif
#elif _WIN64
#if defined(UNICODE) || defined(_UNICODE)
      #define SS_CLASSNAME _T("fpUSpread80")
      #define SPV_CLASSNAME _T("fpSpreadPreview80")
      #define SS_DLLNAME _T("SPR64DU80.DLL")
    #else
      #define SS_CLASSNAME "fpSpread80"
      #define SPV_CLASSNAME "fpSpreadPreview80"
      #define SS_DLLNAME "SPR64D80.DLL"
 #endif
#else
 #if defined(UNICODE) || defined(_UNICODE)
      #define SS_CLASSNAME _T("fpUSpread80")
      #define SPV_CLASSNAME _T("fpSpreadPreview80")
      #define SS_DLLNAME _T("SPR32DU80.DLL")
    #else
      #define SS_CLASSNAME "fpSpread80"
      #define SPV_CLASSNAME "fpSpreadPreview80"
      #define SS_DLLNAME "SPR32D80.DLL"
 #endif
#endif

//--------------------------------------------------------------------
//
// Spreadsheet window styles used in resource file or calls
// to CreateWindow.
//

#define SSS_RESTRICTROWS       0x001L
#define SSS_RESTRICTCOLS       0x002L
#define SSS_PROTECT            0x004L
#define SSS_LOCKALL            0x008L
#define SSS_NORESIZE           0x010L
#define SSS_NOSELECTBLOCK      0x020L
#define SSS_NOGRIDLINES        0x040L
#define SSS_ALLOWUSERFORMULAS  0x080L
#define SSS_HIDECOLHEADERS     0x100L
#define SSS_HIDEROWHEADERS     0x200L
#define SSS_ALLOWCELLOVERFLOW  0x400L
#define SSS_SCROLLBAREXTMODE   0x800L
#define SSS_APPEARANCESTYLE_CLASSIC 0x1000L
#define SSS_APPEARANCESTYLE_VISUALSTYLES 0x2000L

//--------------------------------------------------------------------
//
// Spreadsheet messages
//

#define SSM_DBLCLK             0x0501
#define SSM_EDITMODEON         0x0502
#define SSM_EDITMODEOFF        0x0503
#define SSM_CELLLOCKED         0x0504
#define SSM_ROWRESTRICTED      0x0505
#define SSM_COLRESTRICTED      0x0506
#define SSM_COLROWRESTRICTED   0x0507
#define SSM_DATACHANGE         0x0508
#define SSM_LEAVECELL          0x0509
#define SSM_CLICK              0x050A
#define SSM_KEYDOWN            0x050B
#define SSM_LEAVEROW           0x050C
#define SSM_RBUTTON            0x050D
#define SSM_ENTERROW           0x050E
#define SSM_COLWIDTHCHANGE     0x050F
#define SSM_ROWHEIGHTCHANGE    0x0510
#define SSM_DRAWITEM           0x0511
#define SSM_VQUERYDATA         0x0512
#define SSM_DRAGDROP           0x0513
#define SSM_VCLEARDATA         0x0514
#define SSM_KEYUP              0x0515
#define SSM_KEYPRESS           0x0516
#define SSM_COMBOSELCHANGE     0x0517
#define SSM_COMBODROPDOWN      0x0518
#define SSM_COMBOCLOSEUP       0x0519
#define SSM_TEXTTIPFETCH       0x0520
#define SSM_EDITCHANGE         0x0521
#define SSM_BEFOREUSERSORT     0x0522
#define SSM_AFTERUSERSORT      0x0523
#define SSM_SHEETCHANGING      0x0524
#define SSM_SHEETCHANGED       0x0525
#define SSM_TABSCROLLING       0x0526
#define SSM_TABSCROLLED        0x0527
#define SSM_BEFOREEDITMODE     0x0528
#define SSM_BEFORESCROLLTIP    0x0529
#define SSM_BEFORECOLMOVE      0x0530
#define SSM_BEFOREROWMOVE      0x0531
#define SSM_CIRCULARFORMULA    0x0534

//--------------------------------------------------------------------
//
// Spreadsheet notification messages (part of WM_COMMAND message)
//

#define SSN_SETFOCUS           51
#define SSN_KILLFOCUS          52
#define SSN_INVALIDDATA        53
#define SSN_BLOCKSELECTED      54
#define SSN_BLOCKCHANGED       55
#define SSN_USERFORMULA        56
#define SSN_EXITNEXT           57
#define SSN_EXITPREV           58
#define SSN_BUTTONDOWN         59
#define SSN_BUTTONUP           60
#define SSN_TOPROWCHANGE       61
#define SSN_LEFTCOLCHANGE      62
#define SSN_TOPLEFTCHANGE      63
#define SSN_BLOCKDESELECTED    64

//--------------------------------------------------------------------
//
//  Spreadsheet API decoration
//

#ifdef DLLENTRY
#undef DLLENTRY
#endif

#if defined(WIN32) || defined(_WIN32)
   #if defined(SS_DLL) || defined(SS_OCX) || defined(SS_VB)
      #define DLLENTRY __declspec(dllexport) __stdcall
   #else
      #define DLLENTRY __stdcall
   #endif
#else
   #if defined(SS_DLL) || defined(SS_OCX) || defined(SS_VB)
      #define DLLENTRY __export FAR PASCAL
   #else
      #define DLLENTRY FAR PASCAL
   #endif
#endif

//--------------------------------------------------------------------
//
//  Following definitions provide generic character and text types
//  which can map to SBCS, DBCS, or Unicode.
//

#if !defined(_TCHAR_DEFINED)
   #define _TCHAR_DEFINED
   #if defined(UNICODE) || defined(_UNICODE)
      typedef wchar_t TCHAR;
   #else
      typedef char TCHAR;
   #endif
#endif

#if defined(UNICODE) || defined(_UNICODE)
   typedef wchar_t TUCHAR;
#else
   typedef unsigned char TUCHAR;
#endif

//--------------------------------------------------------------------
//
//  Data types used by the spreadsheet.
//

typedef float FAR        *LPFLOAT;
typedef short FAR        *LPSHORT;
typedef BOOL FAR         *LPBOOL;
typedef double FAR       *LPDOUBLE;
typedef long double FAR  *LPLDOUBLE;
typedef COLORREF FAR     *LPCOLORREF;
typedef unsigned short    USHORT;
typedef unsigned long     ULONG;

#if defined(WIN32) || defined(_WIN32)
   typedef char        *HPSTR;
   typedef BYTE        *HPBYTE;
   typedef short       *HPSHORT;
   typedef VOID        *HPVOID;
   typedef const VOID  *HPCVOID;     
   typedef TCHAR       *HPTSTR;
   typedef const TCHAR *HPCTSTR;
#else
   typedef TCHAR FAR        *LPTSTR;
   typedef const TCHAR FAR  *LPCTSTR;
   typedef char huge        *HPSTR;
   typedef BYTE huge        *HPBYTE;
   typedef short huge       *HPSHORT;
   typedef VOID huge        *HPVOID;
   typedef const VOID huge  *HPCVOID;
   typedef TCHAR huge       *HPTSTR;
   typedef const TCHAR huge *HPCTSTR;
#endif

//--------------------------------------------------------------------
//
//  Spreadsheet specific data types.
//

typedef short SS_FONTID;
typedef long  SS_COLORID;
typedef long  SS_COORD;
typedef SS_COORD FAR *LPSS_COORD;

#if defined(WIN32) || defined(_WIN32)
   typedef SS_COORD *HPSS_COORD;
#else
   typedef SS_COORD huge *HPSS_COORD;
#endif

//--------------------------------------------------------------------
//  Miscellaneous Spreadsheet defines
//

#define SS_SHEET_MAX 1000


//--------------------------------------------------------------------
//
//  The SS_BEFORECOLMOVE structure is used by the SSM_BEFORECOLMOVE message.
//

typedef struct ss_beforecolmove
   {
	SS_COORD Col;         // Column being moved
	SS_COORD Col2;        // Column2 of column range being moved
	SS_COORD ColDest;     // Destination Column
	BOOL     fCancel;     // Set to true to cancel column move
   } SS_BEFORECOLMOVE, FAR *LPSS_BEFORECOLMOVE;

//--------------------------------------------------------------------
//
//  The SS_BEFOREEDITMODE structure is used by the SSM_BEFOREEDITMODE message.
//

typedef struct ss_beforeeditmode
   {
	SS_COORD Col;         // Column
	SS_COORD Row;         // Row
	WORD     wUserAction; // The user action that initiated edit mode
	WORD     wCursorPos;  // The position the cursor will be in
	BOOL     fCancel;     // Set to true to cancel edit mode
   } SS_BEFOREEDITMODE, FAR *LPSS_BEFOREEDITMODE;

//--------------------------------------------------------------------
//
//  The SS_BEFOREROWMOVE structure is used by the SSM_BEFOREROWMOVE message.
//

typedef struct ss_beforerowmove
   {
	SS_COORD Row;         // Row being moved
	SS_COORD Row2;        // Row2 of row range being moved
	SS_COORD RowDest;     // Destination Row
	BOOL     fCancel;     // Set to true to cancel row move
   } SS_BEFOREROWMOVE, FAR *LPSS_BEFOREROWMOVE;

//--------------------------------------------------------------------
//
//  The SS_BEFORESCROLLTIP structure is used by the SSM_BEFORESCROLLTIP message.
//

#define SS_SCROLLTIP_TEXTMAX        1024

typedef struct ss_beforescrolltip
   {
	BOOL     fIsVertical; // Is vertical scroll bar
	SS_COORD Index;       // Column or Row
	TCHAR    szText[SS_SCROLLTIP_TEXTMAX + 1]; // Text to display
	short    nWidth;      // Width of scroll window.  -1 means auto size.
   } SS_BEFORESCROLLTIP, FAR *LPSS_BEFORESCROLLTIP;

#define SS_BEM_KEYBOARD 0
#define SS_BEM_MOUSE    1
#define SS_BEM_CODE     2

#define SS_BEM_CURSORPOS_DEFAULT   0
#define SS_BEM_CURSORPOS_SELECTALL 1
#define SS_BEM_CURSORPOS_BEGINNING 2
#define SS_BEM_CURSORPOS_END       3
#define SS_BEM_CURSORPOS_MOUSE     4
#define SS_BEM_CURSORPOS_DECIMAL   5

//--------------------------------------------------------------------
//
//  The SS_BEFOREUSERSORT structure used by the SSM_BEFOREUSERSORT message.
//

typedef struct ss_beforeusersort
   {
   SS_COORD lCol;
   long lState;
   long lDefaultAction;
   } SS_BEFOREUSERSORT, FAR *LPSS_BEFOREUSERSORT;

#define BEFOREUSERSORT_DEFAULTACTION_CANCEL     0
#define BEFOREUSERSORT_DEFAULTACTION_AUTOSORT   1
#define BEFOREUSERSORT_DEFAULTACTION_MANUALSORT 2


#define BEFOREUSERSORT_STATE_NONE		0
#define BEFOREUSERSORT_STATE_ASCENDING          1
#define BEFOREUSERSORT_STATE_DESCENDING         2

//--------------------------------------------------------------------
//
//  The SS_CELLCOORD structure contains the coordinates for a cell.
//

typedef struct ss_cellcoord
   {
   SS_COORD Col;
   SS_COORD Row;
   } SS_CELLCOORD, FAR *LPSS_CELLCOORD;

//--------------------------------------------------------------------
//
//  The SS_CELLREF structure is used in the SS_VALUE structure when
//  passing a cell reference to a custom function.
//

typedef struct ss_cellref
   {
   short nSheet;
   SS_COORD Col;
   SS_COORD Row;
   union
     {
     HWND hwndSS;
#if defined(_WIN64) || defined(_IA64)
     LONG_PTR lInfo; // internal use only
#else
     long lInfo;  // internal use only
#endif
     }u;
   } SS_CELLREF, FAR* LPSS_CELLREF;

//--------------------------------------------------------------------
//
//  The SS_COMBOCLOSEUP structure is used by the SSM_COMBOCLOSEUP message.
//

typedef struct ss_ComboCloseUp
   {
   SS_COORD Col;
   SS_COORD Row;
   short    dSelChange;
   } SS_COMBOCLOSEUP, FAR *LPSS_COMBOCLOSEUP;

//--------------------------------------------------------------------
//
//  The SS_COORDRANGE structure contains the coordinates for a
//  range of cells.
//

typedef struct ss_coordrange
   {
   SS_COORD Coord1;
   SS_COORD Coord2;
   } SS_COORDRANGE, FAR *LPSS_COORDRANGE;

//--------------------------------------------------------------------
//
//  The SS_DATE structure represents a date.
//

typedef struct tagSS_DATE
   {
   int nDay;      // Day of the Month  1..31
   int nMonth;    // Month of the Year 1..12
   int nYear;     // Year
   } SS_DATE, FAR *LPSS_DATE;

//--------------------------------------------------------------------
//
//  The SS_DATEFORMAT structure describes the format used to display
//  a date.
//

typedef struct tagSS_DATEFORMAT
   {
   BOOL   bCentury;
   TUCHAR cSeparator;
   int    nFormat;
   BOOL   bSpin;
   } SS_DATEFORMAT, FAR *LPSS_DATEFORMAT;

#define  IDF_DDMONYY    100
#define  IDF_DDMMYY     101
#define  IDF_MMDDYY     102
#define  IDF_YYMMDD     103
#define	 IDF_YYMM       104
#define  IDF_MMDD       105

//--------------------------------------------------------------------
//
//  The SS_SELBLOCK structure contains the coordinates for a block
//  of cells.
//

typedef struct ss_selblock
   {
   SS_CELLCOORD UL;  // Upper left cell coordinate
   SS_CELLCOORD LR;  // Lower right cell coordinate
   } SS_SELBLOCK, FAR *LPSS_SELBLOCK;

//--------------------------------------------------------------------
//
//  The SS_DRAGDROP structure is used in the SSM_DRAGDROP message.
//

typedef struct ss_dragdrop
   {
   SS_SELBLOCK BlockPrev;  // Previous block
   SS_SELBLOCK BlockNew;   // New block being draged to
   BOOL        fDataBeingOverwritten;  // TRUE if data is being overwritten
   BOOL        fCopy;      // TRUE if the data is being copied.
                           //   FALSE if the data is being moved.
   BOOL        fDataOnly;  // The application should return TRUE
                           //   if only the data should be copied,
                           //   otherwise all formatting attributes
                           //   will be transfered.
   BOOL        fCancel;    // The application should return TRUE
                           //   if the drag/drop operation should
                           //   be canceled.
   } SS_DRAGDROP, FAR *LPSS_DRAGDROP;

//--------------------------------------------------------------------
//
//  The SS_DRAWITEMSTRUCT structure contains information needed by
//  the application to paint a owner drawn cell.  The structure is
//  sent to the application in a SSM_DRAWITEM message.
//

typedef struct ss_drawitemstruct
   {
   WORD     CtlID;   // ID of spreadsheet control
   SS_COORD Col;     // Column of cell to be drawn
   SS_COORD Row;     // Row of cell to be drawn
   HWND     hWnd;    // Spreadsheet window handle
   HDC      hDC;     // Spreadsheet device context to be used
   RECT     Rect;    // Rectangle of cell.  All drawing must be
                     //   be done within this rectangle.
   long     lStyle;  // User defined style
   LPCTSTR  lpData;  // Cell data
   } SS_DRAWITEMSTRUCT, FAR *LPSS_DRAWITEMSTRUCT;


//--------------------------------------------------------------------
//
//  The SS_CIRCULARFORMULA structure is used in the SSM_CIRCCULARFORMULA
//  message.
//

typedef struct ss_circularformula
   {
   short Sheet;
   SS_COORD Col;
   SS_COORD Row;
   } SS_CIRCULARFORMULA, FAR *LPSS_CIRCULARFORMULA;

//--------------------------------------------------------------------
//
//  The SS_EDITMODEOFF structure is used the SSM_EDITMODEOFF message.
//

typedef struct ss_editmodeoff
   {
   SS_COORD Col;
   SS_COORD Row;
   BOOL     fChangeMade;
   } SS_EDITMODEOFF, FAR *LPSS_EDITMODEOFF;

//--------------------------------------------------------------------
//
//  The SS_ENTERROW structure is used in the SSM_ENTERROW message.
//

typedef struct ss_enterrow
   {
   SS_COORD Row;
   BOOL     fNewRow;
   } SS_ENTERROW, FAR *LPSS_ENTERROW;

//--------------------------------------------------------------------
//
//  The SS_FLOATFORMAT structure describes the format used to
//  display a floating point number.
//

typedef struct tagSS_FLOATFORMAT
   {
   TUCHAR  cCurrencySign;
   TUCHAR  cDecimalSign;
   TUCHAR  cSeparator;
   } SS_FLOATFORMAT, FAR *LPSS_FLOATFORMAT;

//--------------------------------------------------------------------
//
//  The SS_LEAVECELL structure is used in the SSM_LEAVECELL message.
//

typedef struct ss_leavecell
   {
   SS_COORD ColCurrent;
   SS_COORD RowCurrent;
   SS_COORD ColNew;
   SS_COORD RowNew;
   } SS_LEAVECELL, FAR *LPSS_LEAVECELL;

//--------------------------------------------------------------------
//
//  The SS_LEAVEROW structure is used in the SSM_LEAVEROW message.
//

typedef struct ss_leaverow
   {
   SS_COORD RowCurrent;        // Current selected row
   SS_COORD RowNew;            // New row being entered
   BOOL     fRowEdited;        // The user entered edit mode within
                               //   the current row
   BOOL     fRowChangeMade;    // The user made a change to the
                               //   current row
   BOOL     fRowNewBeyondLastDataRow; // The new row the user is clicking
                               //   into is beyond the row below
                               //   the last row containing data.  The
                               //   purpose of this flag is so the
                               //   application can limit the user to
                               //   entering data one line at a time
                               //   and not skipping rows.
   BOOL     fAllCellsHaveData; // All cells within the current row
                               //   have data
   BOOL     fAddRow;           // The current row that was just edited
                               //   should be treated as a new row.
   BOOL     fCancel;           // If the application chooses to
                               //   prevent the user from entering
                               //   the new row, this value should be
                               //   set to TRUE upon exiting the
                               //   SSM_LEAVEROW message.
   } SS_LEAVEROW, FAR *LPSS_LEAVEROW;

//--------------------------------------------------------------------
//
//  The SS_PRINTFORMAT structure contains information passed to
//  the SSPrint() function.
//

typedef struct ss_printformat
   {
   BOOL     fDrawBorder;
   BOOL     fDrawShadows;
   BOOL     fDrawColors;
   BOOL     fShowGrid;
   BOOL     fShowColHeaders;
   BOOL     fShowRowHeaders;
   BOOL     fUseDataMax;
   short    dPrintType;
   SS_COORD x1CellRange;
   SS_COORD y1CellRange;
   SS_COORD x2CellRange;
   SS_COORD y2CellRange;
   short    nPageStart;
   short    nPageEnd;
   float    fMarginLeft;
   float    fMarginTop;
   float    fMarginRight;
   float    fMarginBottom;
   HDC      hDCPrinter;
   WORD     wOrientation;
   BYTE     fSmartPrint;
   WORD     wScalingMethod;
   double   ZoomFactor;
   short    nBestFitPagesWide;
   short    nBestFitPagesTall;
   BOOL     fCenterOnPageH;
   BOOL     fCenterOnPageV;
   } SS_PRINTFORMAT, FAR *LPSS_PRINTFORMAT;

#define SS_PRINTSCALINGMETHOD_NONE       0
#define SS_PRINTSCALINGMETHOD_SMARTPRINT 1
#define SS_PRINTSCALINGMETHOD_ZOOM       2
#define SS_PRINTSCALINGMETHOD_BESTFIT    3

#define SS_PRINTTYPE_CURRENTPAGE  0x01
#define SS_PRINTTYPE_CELLRANGE    0x02
#define SS_PRINTTYPE_PAGERANGE    0x04
#define SS_PRINTTYPE_OVERTHENDOWN 0x08
#define SS_PRINTTYPE_DOWNTHENOVER 0x10

#define SS_PRINTORIENT_DEFAULT   0
#define SS_PRINTORIENT_PORTRAIT  1
#define SS_PRINTORIENT_LANDSCAPE 2

#define SS_PRINTCMD_HEADERLEN   0
#define SS_PRINTCMD_FOOTERLEN   1
#define SS_PRINTCMD_PRINTHEADER 2
#define SS_PRINTCMD_PRINTFOOTER 3

typedef struct ss_printinfo
   {
   BOOL fIsPreview;
   HDC  hDCPrinter;
   RECT RectPreview;
   RECT RectPrinter;
   HWND hWndSpread;
#if defined(_WIN64) || defined(_IA64)
   LONG_PTR lXtra;
#else
   long lXtra;
#endif
   } SS_PRINTINFO, FAR *LPSS_PRINTINFO;

#if defined(_WIN64) || defined(_IA64)
typedef short (CALLBACK* SS_PRINTFUNC)(HDC hDC, short dCommand,
                                       short nPageNum, LONG_PTR lAppData,
                                       LPSS_PRINTINFO lpPrintInfo);
#else
typedef short (CALLBACK* SS_PRINTFUNC)(HDC hDC, short dCommand,
                                       short nPageNum, LONG lAppData,
                                       LPSS_PRINTINFO lpPrintInfo);
#endif

typedef struct ss_pageformat
   {
   BOOL     fDrawBorder;
   BOOL     fDrawShadows;
   BOOL     fDrawColors;
   BOOL     fShowGrid;
   BOOL     fShowColHeaders;
   BOOL     fShowRowHeaders;
   BOOL     fUseDataMax;
   short    dPrintType;
   SS_COORD x1CellRange;
   SS_COORD y1CellRange;
   SS_COORD x2CellRange;
   SS_COORD y2CellRange;
   short    nPageStart;
   short    nPageEnd;
   float    fMarginLeft;
   float    fMarginTop;
   float    fMarginRight;
   float    fMarginBottom;
   HDC      hDCPrinter;
   WORD     wOrientation;
   BYTE     fSmartPrint;
	HGLOBAL  hHeader;
	HGLOBAL  hFooter;
	long     lFirstPageNumber;
	WORD     wScalingMethod;
	double   ZoomFactor;
	short    nBestFitPagesWide;
	short    nBestFitPagesTall;
	BOOL     fCenterOnPageH;
	BOOL     fCenterOnPageV;
   } SS_PAGEFORMAT, FAR *LPSS_PAGEFORMAT;


//--------------------------------------------------------------------
//
//  The SS_RANGEREF structure is used in the SS_VALUE structure when
//  passing a cell range reference to a custom function.
//

typedef struct ss_rangeref
   {
   short nSheet;
   SS_COORD Col1;
   SS_COORD Row1;
   SS_COORD Col2;
   SS_COORD Row2;
   union
     {
     HWND hwndSS;
#if defined(_WIN64) || defined(_IA64)
	  LONG_PTR lInfo;
#else
    long lInfo;
#endif
     }u;
   } SS_RANGEREF, FAR* LPSS_RANGEREF;

//--------------------------------------------------------------------
//
//  The SS_RBUTTON structure is used in the SSM_RBUTTON message.
//

typedef struct ss_rbutton
   {
   SS_COORD Col;          // Column that was clicked on
   SS_COORD Row;          // Row that was clicked on
   short    xPos;         // Mouse x coordinate
   short    yPos;         // Mouse y coordinate
   short    RButtonType;  // Button Type. May be one of the following:
                          //     RBUTTONTYPE_DOWN
                          //     RBUTTONTYPE_UP
                          //     RBUTTONTYPE_DBLCLK
   } SS_RBUTTON, FAR *LPSS_RBUTTON;

#define RBUTTONTYPE_DOWN   0
#define RBUTTONTYPE_UP     1
#define RBUTTONTYPE_DBLCLK 2

//--------------------------------------------------------------------
//
//  The SS_SORT structure contains information passed to the SSSort()
//  function.
//

typedef struct ss_sort
   {
   WORD     wSortBy;        // Sort by rows or columns. Use
                            // one of the following:
                            //    SS_SORT_ROW
                            //    SS_SORT_COL
   SS_COORD Key1Reference;  // Column or row of first key.  Key1
                            // must be specified.
   WORD     wKey1Order;     // The sort order of Key1.  One of the
                            // following may be used:
                            //    SS_SORT_ASCENDING
                            //    SS_SORT_DESCENDING
   SS_COORD Key2Reference;  // Column or row of second key.  Key2
                            // must be -1 if only 1 key is desired
   WORD     wKey2Order;     // The sort order of Key2.  One of the
                            // following may be used:
                            //    SS_SORT_NONE - No Key2
                            //    SS_SORT_ASCENDING
                            //    SS_SORT_DESCENDING
   SS_COORD Key3Reference;  // Column or row of second key.  Key3
                            // must be -1 if 2 keys are desired.
   WORD     wKey3Order;     // The sort order of Key3.  One of the
                            // following may be used:
                            //    SS_SORT_NONE - No Key3
                            //    SS_SORT_ASCENDING
                            //    SS_SORT_DESCENDING
   } SS_SORT, FAR *LPSS_SORT;

#define SS_SORT_ROW            0
#define SS_SORT_COL            1

#define SS_SORT_NONE           0
#define SS_SORT_ASCENDING      1
#define SS_SORT_DESCENDING     2

typedef int (FAR PASCAL *SS_SORTCOMPARE)(HPVOID, HPVOID, long);

//--------------------------------------------------------------------
//
//  The SS_SORTKEY structure contains information passed to the SSSortEx()
//  function.
//

typedef struct ss_sortKEY
   {
   SS_COORD Reference;      // Column or row of first key.
   WORD     wOrder;         // The sort order of Key.  One of the
                            // following may be used:
                            //    SS_SORT_ASCENDING
                            //    SS_SORT_DESCENDING
   } SS_SORTKEY, FAR *LPSS_SORTKEY;

//--------------------------------------------------------------------
//
//  The SS_TEXTTIPFETCH structure is used in the SSM_TEXTTIPFETCH message.
//
#define SS_TT_TEXTMAX        1024

typedef struct ss_texttipfetch
   {
   BOOL     fShow;
   WORD     wMultiLine;
   WORD     wAlignX;
   WORD     wAlignY;
   short    xOffset;
   short    yOffset;
   short    nWidth;
   SS_COORD Col;
   SS_COORD Row;
   TCHAR    szText[SS_TT_TEXTMAX + 1];
   HGLOBAL  hText;  // If this member is non-empty it is used instead of szText.
                    // The app does not need to free it.  Spread will free it.
   } SS_TEXTTIPFETCH, FAR *LPSS_TEXTTIPFETCH;

#define SS_TT_XALIGN_AUTO   0
#define SS_TT_XALIGN_LEFT   1
#define SS_TT_XALIGN_RIGHT  2
#define SS_TT_XALIGN_CENTER 3

#define SS_TT_YALIGN_AUTO   0
#define SS_TT_YALIGN_TOP    1
#define SS_TT_YALIGN_BOTTOM 2
#define SS_TT_YALIGN_CENTER 3

#define SS_TT_MULTILINE_SINGLE 0
#define SS_TT_MULTILINE_MULTI  1
#define SS_TT_MULTILINE_AUTO   2


//--------------------------------------------------------------------
//
//  The SS_TIME structure represents a time.
//

typedef struct tagSS_TIME
   {
   int nHour;    // Hour 0..23
   int nMinute;  // Minutes 0..59
   int nSecond;  // Seconds 0..59
   } SS_TIME, FAR *LPSS_TIME;

//--------------------------------------------------------------------
//
//  The SS_TIMEFORMAT structure describes the format used to display
//  a time.
//

typedef struct tagSS_TIMEFORMAT 
   {
   BOOL   b24Hour;
   BOOL   bSeconds;
   TUCHAR cSeparator;
   BOOL   bSpin;
   } SS_TIMEFORMAT, FAR *LPSS_TIMEFORMAT;

//--------------------------------------------------------------------
//
//  The SS_VALUE structure is used in the evaluation of custom
//  fucntions.
//

typedef struct ss_value
   {
   BYTE Status;
   BYTE Type;
   union
      {
      HGLOBAL     hValStr;
      long        ValLong;
      double      ValDouble;
      SS_CELLREF  ValCell;
      SS_RANGEREF ValRange;
      } Val;
   } SS_VALUE, FAR *LPSS_VALUE;

#define SS_VALUE_STATUS_OK     0
#define SS_VALUE_STATUS_ERROR  1
#define SS_VALUE_STATUS_EMPTY  2
    
#define SS_VALUE_TYPE_LONG     0
#define SS_VALUE_TYPE_DOUBLE   1
#define SS_VALUE_TYPE_STR      2
#define SS_VALUE_TYPE_CELL     3
#define SS_VALUE_TYPE_RANGE    4

typedef FARPROC SS_CALCFUNC;

//--------------------------------------------------------------------
//
//  The SS_VCLEARDATA structure is used in the SSM_VCLEARDATA message.
//

typedef struct ss_vcleardata
   {
   SS_COORD Row;               // Top row of buffer being cleared
   SS_COORD RowsBeingCleared;  // The number of rows being cleared
   } SS_VCLEARDATA, FAR *LPSS_VCLEARDATA;

//--------------------------------------------------------------------
//
//  The SS_VQUERYDATA structure is used in the SSM_VQUERYDATA message.
//

typedef struct ss_vquerydata
   {
   SS_COORD Row;         // Virtual top row to begin
                         //   assignment of data
   SS_COORD RowsNeeded;  // The number of rows requested
   SS_COORD RowsLoaded;  // The application should set the
                         //   number of rows actually loaded
                         //   before returning
   WORD     wDirection;  // Direction to retrieve data
                         //   SS_VDOWN
                         //   SS_VUP
                         //   SS_VTOP
                         //   SS_VBOTTOM
                         //   SS_VREFRESH
   BOOL     fAtTop;      // The application should set this
                         //   flag to TRUE if the top of the
                         //   data has been reached
   BOOL     fAtBottom;   // The application should set this
                         //   flag to TRUE if the bottom of the
                         //   data has been reached
   } SS_VQUERYDATA, FAR *LPSS_VQUERYDATA;

#define SS_VDOWN    1
#define SS_VUP      2
#define SS_VTOP     3
#define SS_VBOTTOM  4
#define SS_VREFRESH 5

//--------------------------------------------------------------------
//
//  The SUPERBTNCOLOR structure describes the colors used to paint
//  a button.
//

typedef struct superbtncolor
   {
   COLORREF Color;
   COLORREF ColorBorder;
   COLORREF ColorShadow;
   COLORREF ColorHighlight;
   COLORREF ColorText;
   } SUPERBTNCOLOR, FAR *LPSUPERBTNCOLOR;

//--------------------------------------------------------------------
//
//  Value types for SS_CT_VALUE structure.
//

#define SS_CT_VALUE_TYPE_EMPTY  0
#define SS_CT_VALUE_TYPE_TSTR   1
#define SS_CT_VALUE_TYPE_DOUBLE 2
#define SS_CT_VALUE_TYPE_LONG   3
#define SS_CT_VALUE_TYPE_BUFFER 4

//--------------------------------------------------------------------
//
//  The SS_CT_BUFFER structure passes buffer values in a SS_CT_VALUE.
//

typedef struct ss_ct_buffer
{
   HGLOBAL hBuffer;
   long lSize;
} SS_CT_BUFFER, FAR *LPSS_CT_BUFFER;

//--------------------------------------------------------------------
//
//  The SS_CT_VALUE structure passes values to and from a
//  custom celltype.
//

typedef struct ss_ct_value
{
  int type;                   // in/out: value type
  union
  {
    HGLOBAL hszValue;         // in/out: handle to string value (TSTR)
    double dblValue;          // in/out: double value
    long lValue;              // in/out: long value
    SS_CT_BUFFER Buffer;      // in/out: buffer value
  } u;
} SS_CT_VALUE, FAR *LPSS_CT_VALUE;

//--------------------------------------------------------------------
//
//  The SS_CT_APPEARANCE structure is passed to the custom celltype callback
//  methods to specify values to be used for editing and rendering.
//

typedef struct ss_ct_appearance
{
   COLORREF clrBackColor;         // background color for cell
   COLORREF clrForeColor;         // foreground color for cell
   COLORREF clrSelBackColor;      // selected background color for cell
   COLORREF clrSelForeColor;      // selected foreground color for cell
   COLORREF clrLockBackColor;     // lock background color for cell
   COLORREF clrLockForeColor;     // lock foreground color for cell
   HFONT    hFont;                // font for cell
   long     lStyle;               // style for cell
   BOOL		  fDrawPrimaryButton;   // TRUE to draw primary buttons
   BOOL		  fDrawSecondaryButton; // TRUE to draw secondary buttons (i.e. combo)
   BOOL     fDrawColors;          // TRUE to draw colors
} SS_CT_APPEARANCE, FAR *LPSS_CT_APPEARANCE;

//--------------------------------------------------------------------
//
//  Structures for custom celltype callback function definitions
//

//--------------------------------------------------------------------
//
//  The SS_CT_CREATEEDITORCONTROL_ARGS structure is passed to the 
//  CreateEditorControl callback function for a custom celltype.
//

typedef struct ss_ct_createeditorcontrol_args
{
   LPTSTR lpszName;   // in: name of custom celltype
   HWND   hWndSpread; // in: HWND of Spread control
   long   lStyle;     // in: style for custom celltype
   int    CtrlId;     // in: Value to use for control id
   HWND   hWndEditor; // out: HWND of editor control
} SS_CT_CREATEEDITORCONTROL_ARGS, FAR *LPSS_CT_CREATEEDITORCONTROL_ARGS;

//--------------------------------------------------------------------
//
//  The SS_CT_CREATERENDERERCONTROL_ARGS structure is passed to the 
//  CreateRendererControl callback function for a custom celltype.
//

typedef struct ss_ct_createrenderercontrol_args
{
   LPTSTR lpszName;     // in: name of custom celltype
   HWND   hWndSpread;   // in: HWND of Spread control
   long   lStyle;       // in: style for custom celltype
   int    CtrlId;       // in: Value to use for control id
   HWND   hWndRenderer; // out: HWND of renderer control
} SS_CT_CREATERENDERERCONTROL_ARGS, FAR *LPSS_CT_CREATERENDERERCONTROL_ARGS;

//--------------------------------------------------------------------
//
//  The SS_CT_INITIALIZECONTROL_ARGS structure is passed to the 
//  InitializeControl callback function for a custom celltype.
//  

typedef struct ss_ct_initializecontrol_args
{
   LPTSTR            lpszName;    // in: name of custom celltype
   HWND              hWndSpread;  // in: HWND of Spread control
   HWND              hWndCtrl;    // in: HWND of editor or renderer control
   SS_CT_APPEARANCE  Appearance;  // in: Appearance settings to apply to editor
   short             Sheet;       // in: index of sheet
   SS_CELLCOORD      Cell;        // in: coordinates for cell
} SS_CT_INITIALIZECONTROL_ARGS, FAR *LPSS_CT_INITIALIZECONTROL_ARGS;

//--------------------------------------------------------------------
//
//  The SS_CT_GETEDITORVALUE_ARGS structure is passed to the 
//  GetEditorValue callback function for a custom celltype.
//  

typedef struct ss_ct_geteditorvalue_args
{
   LPTSTR       lpszName;   // in: name of custom celltype
   HWND         hWndSpread; // in: HWND of Spread control
   HWND         hWndEditor; // in: HWND of editor control
   SS_CT_VALUE  Value;      // out: value from editor
   short        Sheet;      // in: index of sheet
   SS_CELLCOORD Cell;       // in: coordinates for cell
} SS_CT_GETEDITORVALUE_ARGS, FAR *LPSS_CT_GETEDITORVALUE_ARGS;

//--------------------------------------------------------------------
//
//  The SS_CT_SETVALUE_ARGS structure is passed to the 
//  SetValue callback function for a custom celltype.
//  

typedef struct ss_ct_setvalue_args
{
   LPTSTR       lpszName;   // in: name of custom celltype
   HWND         hWndSpread; // in: HWND of Spread control
   HWND         hWndCtrl;   // in: HWND of editor or renderer control
   SS_CT_VALUE  Value;      // in: value to set into editor or renderer
   short        Sheet;      // in: index of sheet
   SS_CELLCOORD Cell;       // in: coordinates for cell
} SS_CT_SETVALUE_ARGS, FAR *LPSS_CT_SETVALUE_ARGS;

//--------------------------------------------------------------------
//
//  The SS_CT_STARTEDITING_ARGS structure is passed to the 
//  StartEditing callback function for a custom celltype.
//  

typedef struct ss_ct_startediting_args
{
   LPTSTR       lpszName;       // in: name of custom celltype
   HWND         hWndSpread;     // in: HWND of Spread control
   HWND         hWndEditor;     // in: HWND of editor control
   short        Sheet;          // in: index of sheet
   SS_CELLCOORD Cell;           // in: coordinates for cell
   MSG          msg;            // in: MSG to forward to editor
   BOOL         fSelectAll;     // in: TRUE if editor should select all contents
   BOOL         fAutoClipboard; // in: TRUE if editor should handle clipboard keys
} SS_CT_STARTEDITING_ARGS, FAR *LPSS_CT_STARTEDITING_ARGS;

//--------------------------------------------------------------------
//
//  The SS_CT_CANCELEDITING_ARGS structure is passed to the 
//  CancelEditing callback function for a custom celltype.
//  

typedef struct ss_ct_cancelediting_args
{
   LPTSTR       lpszName;   // in: name of custom celltype
   HWND         hWndSpread; // in: HWND of Spread control
   HWND         hWndEditor; // in: HWND of editor control
   short        Sheet;      // in: index of sheet
   SS_CELLCOORD Cell;       // in: coordinates for cell
} SS_CT_CANCELEDITING_ARGS, FAR *LPSS_CT_CANCELEDITING_ARGS;

//--------------------------------------------------------------------
//
//  The SS_CT_STOPEDITING_ARGS structure is passed to the 
//  StopEditing callback function for a custom celltype.
//  

typedef struct ss_ct_stopediting_args
{
   LPTSTR       lpszName;   // in: name of custom celltype
   HWND         hWndSpread; // in: HWND of Spread control
   HWND         hWndEditor; // in: HWND of editor control
   short        Sheet;      // in: index of sheet
   SS_CELLCOORD Cell;       // in: coordinates for cell
   BOOL         Cancel;     // out: TRUE to cancel edit mode stopping
} SS_CT_STOPEDITING_ARGS, FAR *LPSS_CT_STOPEDITING_ARGS;


//--------------------------------------------------------------------
//
//  The SS_CT_ISRESERVEDKEY_ARGS structure is passed to the 
//  IsReservedKey callback function for a custom celltype.
//  The default is all keys except the escape key will invoke edit mode.
//  Return FALSE to prevent edit mode from starting with the specified key.

typedef struct ss_ct_isreservedkey_args
{
   LPTSTR       lpszName;       // in: name of custom celltype
   HWND         hWndSpread;     // in: HWND of Spread control
   HWND         hWndRenderer;   // in: HWND of renderer control
   short        Sheet;          // in: index of sheet
   SS_CELLCOORD Cell;           // in: coordinates for cell
   UINT         nKeyCode;       // in: key code of key pressed
   USHORT       nShift;         // in: shift state
} SS_CT_ISRESERVEDKEY_ARGS, FAR *LPSS_CT_ISRESERVEDKEY_ARGS;

//--------------------------------------------------------------------
//
//  The SS_CT_ISRESERVEDLOCATION_ARGS structure is passed to the 
//  IsReservedLocation callback function for a custom celltype.
//  Return TRUE to indicate that the specified location should invoke
//  edit mode in the cell when clicked, and return a reserved cursor
//  id if the cell should show a special cursor for that location.

typedef struct ss_ct_isreservedlocation_args
{
   LPTSTR           lpszName;           // in: name of custom celltype
   HWND             hWndSpread;         // in: HWND of Spread control
   HWND             hWndRenderer;       // in: HWND of renderer control
   short            Sheet;              // in: index of sheet
   SS_CELLCOORD     Cell;               // in: coordinates for cell
   POINT            p;                  // in: mouse cursor location (in client coordinates)
   RECT             rect;               // in: rectangle for cell (in client coordinates)
   SS_CT_APPEARANCE Appearance;         // in: Appearance settings for cell
   SS_CT_VALUE      Value;              // in: value in cell
   long             lReservedCursorId;  // out: reserved cursor id (0 for none)
} SS_CT_ISRESERVEDLOCATION_ARGS, FAR *LPSS_CT_ISRESERVEDLOCATION_ARGS;

//--------------------------------------------------------------------
//
//  The SS_CT_ISVALID_ARGS structure is passed to the 
//  IsValid callback function for a custom celltype.
//  Return FALSE to indicate an invalid value.
//  All values are considered valid if this function is not implemented.
//

typedef struct ss_ct_isvalid_args
{
   LPTSTR       lpszName;   // in: name of custom celltype
   HWND         hWndSpread; // in: HWND of Spread control
   HWND         hWndEditor; // in: HWND of editor control
   SS_CT_VALUE  Value;      // in: value to validate
} SS_CT_ISVALID_ARGS, FAR *LPSS_CT_ISVALID_ARGS;

//--------------------------------------------------------------------
//
//  The SS_CT_GETRESERVEDCURSOR_ARGS structure is passed to the 
//  GetReservedCursor callback function for a custom celltype.
//  

typedef struct ss_ct_getreservedcursor_args
{
   LPTSTR   lpszName;           // in: name of custom celltype
   HWND     hWndSpread;         // in: HWND of Spread control
   HWND     hWndRenderer;       // in: HWND of renderer control
   long     lReservedCursorId;  // in: reserved cursor id
   HCURSOR  hCursor;            // out: cursor to use (application must free this object)
} SS_CT_GETRESERVEDCURSOR_ARGS, FAR *LPSS_CT_GETRESERVEDCURSOR_ARGS;

//--------------------------------------------------------------------
//
//  The SS_CT_GETPREFERREDSIZE_ARGS structure is passed to the 
//  GetPreferredSize callback function for a custom celltype.
//  

typedef struct ss_ct_getpreferredsize_args
{
   LPTSTR           lpszName;       // in: name of custom celltype
   HWND             hWndSpread;     // in: HWND of Spread control
   HWND             hWndRenderer;   // in: HWND of renderer control
   HDC              hDC;            // in: device context
   short            Sheet;          // in: index of sheet
   SS_CELLCOORD     Cell;           // in: coordinates for cell
   SS_CT_APPEARANCE Appearance;     // in: Appearance settings for cell
   RECT             rect;           // in: rectangle for cell (in client coordinates)
   SS_CT_VALUE      Value;          // in: value in cell
   SIZE             sizePreferred;  // out: preferred size
} SS_CT_GETPREFERREDSIZE_ARGS, FAR *LPSS_CT_GETPREFERREDSIZE_ARGS;

//--------------------------------------------------------------------
//
//  The SS_CT_PAINTCELL_ARGS structure is passed to the 
//  PaintCell callback function for a custom celltype.
//  

typedef struct ss_ct_paintcell_args
{
   LPTSTR           lpszName;     // in: name of custom celltype
   HWND             hWndSpread;   // in: HWND of Spread control
   HWND             hWndRenderer; // in: HWND of renderer control
   HDC              hDC;          // in: device context
   RECT             rect;         // in: rectangle for cell (in client coordinates)
   SS_CT_APPEARANCE Appearance;   // in: Appearance settings for cell
   short            Sheet;        // in: index of sheet
   SS_CELLCOORD     Cell;         // in: coordinates for cell
   SS_CT_VALUE      Value;        // in: value in cell
   BOOL             fIsSelected;  // in: TRUE if cell is selected
   BOOL             fIsLocked;    // in: TRUE if cell is locked
} SS_CT_PAINTCELL_ARGS, FAR *LPSS_CT_PAINTCELL_ARGS;

//--------------------------------------------------------------------
//
//  The SS_CT_STRINGFORMAT_ARGS structure is passed to the 
//  StringFormat callback function for a custom celltype.
//  

typedef struct ss_ct_stringformat_args
{
   LPTSTR   lpszName;     // in: name of custom celltype
   HWND     hWndSpread;   // in: HWND of Spread control
   HWND     hWndRenderer; // in: HWND of renderer control
   long     lStyle;       // in: style for custom celltype
   SS_CT_VALUE Value;     // in: Value to format
   HGLOBAL  hszFormatted; // out: handle to formatted TSTR
} SS_CT_STRINGFORMAT_ARGS, FAR *LPSS_CT_STRINGFORMAT_ARGS;

//--------------------------------------------------------------------
//
//  The SS_CT_STRINGUNFORMAT_ARGS structure is passed to the 
//  StringUnformat callback function for a custom celltype.
//  

typedef struct ss_ct_stringunformat_args
{
   LPTSTR   lpszName;     // in: name of custom celltype
   HWND     hWndSpread;   // in: HWND of Spread control
   HWND     hWndRenderer; // in: HWND of renderer control
   long     lStyle;       // in: style for custom celltype
   LPTSTR   lpszFormatted;// in: handle to formatted TSTR
   SS_CT_VALUE Value;     // out: unformatted Value
} SS_CT_STRINGUNFORMAT_ARGS, FAR *LPSS_CT_STRINGUNFORMAT_ARGS;

//--------------------------------------------------------------------
//
//  Callback function definitions for the custom celltype structure.
//  All methods return FALSE on failure.

typedef BOOL (CALLBACK* SS_CT_CREATEEDITORCONTROLFUNC)(LPSS_CT_CREATEEDITORCONTROL_ARGS lpArgs);
typedef BOOL (CALLBACK* SS_CT_CREATERENDERERCONTROLFUNC)(LPSS_CT_CREATERENDERERCONTROL_ARGS lpArgs);
typedef BOOL (CALLBACK* SS_CT_INITIALIZECONTROLFUNC)(LPSS_CT_INITIALIZECONTROL_ARGS lpArgs);
typedef BOOL (CALLBACK* SS_CT_GETEDITORVALUEFUNC)(LPSS_CT_GETEDITORVALUE_ARGS lpArgs);
typedef BOOL (CALLBACK* SS_CT_SETVALUEFUNC)(LPSS_CT_SETVALUE_ARGS lpArgs);
typedef BOOL (CALLBACK* SS_CT_STARTEDITINGFUNC)(LPSS_CT_STARTEDITING_ARGS lpArgs);
typedef BOOL (CALLBACK* SS_CT_CANCELEDITINGFUNC)(LPSS_CT_CANCELEDITING_ARGS lpArgs);
typedef BOOL (CALLBACK* SS_CT_STOPEDITINGFUNC)(LPSS_CT_STOPEDITING_ARGS lpArgs);
typedef BOOL (CALLBACK* SS_CT_ISRESERVEDKEYFUNC)(LPSS_CT_ISRESERVEDKEY_ARGS lpArgs);
typedef BOOL (CALLBACK* SS_CT_ISRESERVEDLOCATIONFUNC)(LPSS_CT_ISRESERVEDLOCATION_ARGS lpArgs);
typedef BOOL (CALLBACK* SS_CT_ISVALIDFUNC)(LPSS_CT_ISVALID_ARGS lpArgs);
typedef BOOL (CALLBACK* SS_CT_GETRESERVEDCURSORFUNC)(LPSS_CT_GETRESERVEDCURSOR_ARGS lpArgs);
typedef BOOL (CALLBACK* SS_CT_GETPREFERREDSIZEFUNC)(LPSS_CT_GETPREFERREDSIZE_ARGS lpArgs);
typedef BOOL (CALLBACK* SS_CT_PAINTCELLFUNC)(LPSS_CT_PAINTCELL_ARGS lpArgs);
typedef BOOL (CALLBACK* SS_CT_STRINGFORMATFUNC)(LPSS_CT_STRINGFORMAT_ARGS lpArgs);
typedef BOOL (CALLBACK* SS_CT_STRINGUNFORMATFUNC)(LPSS_CT_STRINGUNFORMAT_ARGS lpArgs);

//--------------------------------------------------------------------
//
//  The SS_CT_PROCS structure represents the interface of callback functions
//  for custom celltypes.
//

typedef struct ss_ct_procs
{
   SS_CT_CREATEEDITORCONTROLFUNC	   pfnCreateEditorControl;
   SS_CT_CREATERENDERERCONTROLFUNC	pfnCreateRendererControl;
   SS_CT_INITIALIZECONTROLFUNC	   pfnInitializeControl;
   SS_CT_GETEDITORVALUEFUNC		   pfnGetEditorValue;
   SS_CT_SETVALUEFUNC		         pfnSetValue;
   SS_CT_STARTEDITINGFUNC			   pfnStartEditing;
   SS_CT_CANCELEDITINGFUNC			   pfnCancelEditing;
   SS_CT_STOPEDITINGFUNC			   pfnStopEditing;
   SS_CT_ISRESERVEDKEYFUNC			   pfnIsReservedKey;
   SS_CT_ISRESERVEDLOCATIONFUNC	   pfnIsReservedLocation;
   SS_CT_ISVALIDFUNC				      pfnIsValid;
   SS_CT_GETRESERVEDCURSORFUNC	   pfnGetReservedCursor;
   SS_CT_GETPREFERREDSIZEFUNC		   pfnGetPreferredSize;
   SS_CT_PAINTCELLFUNC				   pfnPaintCell;
   SS_CT_STRINGFORMATFUNC           pfnStringFormat;
   SS_CT_STRINGUNFORMATFUNC         pfnStringUnformat;
} SS_CT_PROCS, FAR *LPSS_CT_PROCS;

//--------------------------------------------------------------------
//
//  SSAddCustomCellType adds a new custom cell type.
//
BOOL DLLENTRY SSAddCustomCellType(HWND hWnd, LPCTSTR lpszName, BOOL bEditable, BOOL bCanOverflow, BOOL bCanBeOverflown, BOOL bUseRendererControl, LPSS_CT_PROCS lpProcs);
//--------------------------------------------------------------------
//
//  SSRemoveCustomCellType removes a custom cell type.
//
BOOL DLLENTRY SSRemoveCustomCellType(HWND hWnd, LPCTSTR lpszName);
//--------------------------------------------------------------------
//
//  SSEnumCustomCellType enumerates through the custom cell type names.
//
BOOL DLLENTRY SSEnumCustomCellType(HWND hWnd, LPCTSTR lpszPrevName, LPTSTR lpszName, int nLen);
//--------------------------------------------------------------------
//
//  SSEnumCustomCellTypeLen enumerates through the lengths of the
//  custom cell type names.
//
int DLLENTRY SSEnumCustomCellTypeLen(HWND hWnd, LPCTSTR lpszPrevName);
//--------------------------------------------------------------------
//
//  SSGetCustomCellType gets the specified custom cell type settings.
//
BOOL DLLENTRY SSGetCustomCellType(HWND hWnd, LPCTSTR lpszName, LPBOOL lpbEditable, LPBOOL lpbCanOverflow, LPBOOL lpbCanBeOverflown, LPBOOL lpbUseRendererControl, LPSS_CT_PROCS lpProcs);

//--------------------------------------------------------------------
//
//  The SS_CELLTYPE structure contains information for displaying
//  and editing a cell.
//

#if defined(_WIN64) || defined(_IA64)
typedef HGLOBAL TBGLOBALHANDLE, FAR *LPTBGLOBALHANDLE;
#else
typedef ULONG TBGLOBALHANDLE, FAR *LPTBGLOBALHANDLE;
#endif

typedef struct ss_celltype
   {
   BYTE  Type;
   short ControlID;
   long  Style;
   union
      {
      struct
     {
     short  ChrSet;
     short  Len;
     short  ChrCase;
     double dfWidth;
     short  nRowCnt;
     } Edit;
      struct
     {
     long  Min;
     long  Max;
     BOOL  fSpinWrap;
     long  lSpinInc;
     } Integer;
      struct
     {
     short       Left;
     short       Right;
     double      Min;
     double      Max;
     BOOL        fSetFormat;
     SS_FLOATFORMAT Format;
     } Float;
      struct
     {
     short          dItemCnt;
     TBGLOBALHANDLE hItems;
     short          dMaxRows;
     short          dMaxEditLen;
     short          dComboWidth;
     HWND           hWndDropDown;
     BYTE           AutoSearch;
     } ComboBox;
      struct
     {
     BOOL       SetFormat;
     BOOL       SetRange;
     SS_TIMEFORMAT Format;
     SS_TIME       Min;
     SS_TIME       Max;
     } Time;
      struct
     {
     BOOL       SetFormat;
     BOOL       SetRange;
     SS_DATEFORMAT Format;
     SS_DATE    Min;
     SS_DATE    Max;
     } Date;
      struct
     {
     TBGLOBALHANDLE hMask;
     } Pic;
      struct
     {
     TBGLOBALHANDLE hPictName;
     HPALETTE       hPal;
#ifdef SS_VB
     HANDLE         hPic;
#elif defined(SS_OCX)
   LPVOID         lpPict;
#else
   LPVOID res1;
#endif
     BOOL           fDeleteHandle;
     } ViewPict;
      struct
     {
     HGLOBAL  hText;
     HGLOBAL  hPictName;
     HGLOBAL  hPictDownName;
     short    nPictureType;
     short    nPictureDownType;
     BYTE     ButtonType;
     BYTE     ShadowSize;
     BYTE     fDelHandle;
     BYTE     fDelDownHandle;
     SUPERBTNCOLOR Color;
#ifdef SS_VB
     HANDLE         hPic;
     HANDLE         hPicDown;
#elif defined(SS_OCX)
   LPVOID         lpPict;
   LPVOID         lpPictDown;
#else
   LPVOID res1;
   LPVOID res2;
#endif
     } Button;
      struct
     {
     HGLOBAL        hText;
     TBGLOBALHANDLE hPictUpName;
     TBGLOBALHANDLE hPictDownName;
     TBGLOBALHANDLE hPictFocusUpName;
     TBGLOBALHANDLE hPictFocusDownName;
     TBGLOBALHANDLE hPictGrayName;
     TBGLOBALHANDLE hPictFocusGrayName;
     BYTE           bPictUpType;
     BYTE           bPictDownType;
     BYTE           bPictFocusUpType;
     BYTE           bPictFocusDownType;
     BYTE           bPictGrayType;
     BYTE           bPictFocusGrayType;
     BYTE           fDelUpHandle;
     BYTE           fDelDownHandle;
     BYTE           fDelFocusUpHandle;
     BYTE           fDelFocusDownHandle;
     BYTE           fDelGrayHandle;
     BYTE           fDelFocusGrayHandle;
#ifdef SS_VB
     HANDLE         hPicUp;
     HANDLE         hPicDown;
     HANDLE         hPicFocusUp;
     HANDLE         hPicFocusDown;
     HANDLE         hPicGray;
     HANDLE         hPicFocusGray;
#elif defined(SS_OCX)
   LPVOID         lpPictUp;
   LPVOID         lpPictDown;
   LPVOID         lpPictFocusUp;
   LPVOID         lpPictFocusDown;
   LPVOID         lpPictGray;
   LPVOID         lpPictFocusGray;
#else
   LPVOID res1;
   LPVOID res2;
   LPVOID res3;
   LPVOID res4;
   LPVOID res5;
   LPVOID res6;
#endif
     } CheckBox;
     struct
			{
			double Min;
			double Max;
			TCHAR  szDecimal[3 + 1];
			TCHAR  szSeparator[3 + 1];
			TCHAR  szCurrency[5 + 1];
			BYTE   bLeadingZero;         // 0 - 2
			short  Right;
			BYTE   bNegStyle;            // 0 - 16
			BYTE   bPosStyle;            // 0 - 5
			BYTE   fShowSeparator;       // T/F
			BYTE   fShowCurrencySymbol;  // T/F
			BYTE   fSpin;
			BYTE   fSpinWrap;
			double SpinInc;
			} Currency;
      struct
			{
			double Min;
			double Max;
			TCHAR  szDecimal[3 + 1];
			TCHAR  szSeparator[3 + 1];
			BYTE   bLeadingZero;         // 0 - 2
			short  Right;
			BYTE   bNegStyle;            // 0 - 5
			BYTE   fShowSeparator;       // T/F
			BYTE   fSpin;
			BYTE   fSpinWrap;
			double SpinInc;
			} Number;
      struct
			{
			double Min;
			double Max;
			TCHAR  szDecimal[3 + 1];
			short  Right;
			BYTE   bLeadingZero;         // 0 - 2
			BYTE   bNegStyle;            // 0 - 8
			BYTE   fSpin;
			BYTE   fSpinWrap;
			double SpinInc;
			} Percent;
      struct
			{
			double Min;
			double Max;
			TCHAR  szDecimal[3 + 1];
			short  Right;
			} Scientific;
	  struct
			{
		    HGLOBAL	hName;
          short RendererControlID;
          SS_CT_VALUE ItemData;
			} Custom;

      } Spec;
   } SS_CELLTYPE, FAR *LPSS_CELLTYPE;

#define SS_TYPE_DATE           1
#define SS_TYPE_EDIT           2
#define SS_TYPE_PIC            3
#define SS_TYPE_FLOAT          4
#define SS_TYPE_INTEGER        5
#define SS_TYPE_STATICTEXT     6
#define SS_TYPE_TIME           7
#define SS_TYPE_PICTURE        8
#define SS_TYPE_CUSTOM		     9
#define SS_TYPE_BUTTON         10
#define SS_TYPE_COMBOBOX       11
#define SS_TYPE_CHECKBOX       12
#define SS_TYPE_OWNERDRAW      13
#define SS_TYPE_CURRENCY       14
#define SS_TYPE_NUMBER         15
#define SS_TYPE_PERCENT        16
#define SS_TYPE_SCIENTIFIC     17

//--------------------------------------------------------------------
//
//  Spreadsheet function prototypes
//

BOOL DLLENTRY SSAddCustomFunction(HWND hWnd, LPCTSTR FunctionName,
                                  short ParameterCnt, FARPROC lpfnFunctionProc);
BOOL DLLENTRY SSAddCustomFunctionExt(HWND hWnd, LPCTSTR lpszFuncName,
                                     short nMinParamCnt, short nMaxParamCnt,
                                     FARPROC lpfnFuncProc, long lFlags);
BOOL DLLENTRY SSAddMultiSelBlocks(HWND hWnd, LPSS_SELBLOCK lpSelBlock);
BOOL DLLENTRY SSBuildDependencies(HWND hWnd);
BOOL DLLENTRY SSClear(HWND hWnd, SS_COORD Col, SS_COORD Row);
BOOL DLLENTRY SSClearData(HWND hWnd, SS_COORD Col, SS_COORD Row);
BOOL DLLENTRY SSClearDataRange(HWND hWnd, SS_COORD Col, SS_COORD Row,
                               SS_COORD Col2, SS_COORD Row2);
BOOL DLLENTRY SSClearRange(HWND hWnd, SS_COORD Col, SS_COORD Row,
                           SS_COORD Col2, SS_COORD Row2);
BOOL DLLENTRY SSClipboardCopy(HWND hWnd);
BOOL DLLENTRY SSClipboardCut(HWND hWnd);
BOOL DLLENTRY SSClipboardPaste(HWND hWnd);
BOOL DLLENTRY SSClipIn(HWND hWnd, SS_COORD Col, SS_COORD Row,
                       SS_COORD Col2, SS_COORD Row2,
                       LPCTSTR Buffer, long BufferLen);
HGLOBAL DLLENTRY SSClipOut(HWND hWnd, SS_COORD Col, SS_COORD Row,
                           SS_COORD Col2, SS_COORD Row2);
BOOL DLLENTRY SSClipValueIn(HWND hWnd, SS_COORD Col, SS_COORD Row,
                            SS_COORD Col2, SS_COORD Row2,
                            LPCTSTR Buffer, long BufferLen);
HGLOBAL DLLENTRY SSClipValueOut(HWND hWnd, SS_COORD Col, SS_COORD Row,
                                SS_COORD Col2, SS_COORD Row2);
SS_COORD DLLENTRY SSColLetterToNumber(LPCTSTR lpszText);
int  DLLENTRY SSColWidthToLogUnits(HWND hWnd, double dfColWidth);
LRESULT DLLENTRY SSComboBoxSendMessage(HWND hWnd, SS_COORD Col, SS_COORD Row,
                                       UINT uMsg, WPARAM wParam,
                                       LPARAM lParam);
BOOL DLLENTRY SSCopyRange(HWND hWnd, SS_COORD Col, SS_COORD Row,
                          SS_COORD Col2, SS_COORD Row2,
                          SS_COORD ColDest, SS_COORD RowDest);
BOOL DLLENTRY SSDelCol(HWND hWnd, SS_COORD Col);
BOOL DLLENTRY SSDelColRange(HWND hWnd, SS_COORD Col, SS_COORD Col2);
BOOL DLLENTRY SSDeleteSheets(HWND hWnd, short nSheetIndex, short nSheetCnt);
BOOL DLLENTRY SSDelRow(HWND hWnd, SS_COORD Row);
BOOL DLLENTRY SSDelRowRange(HWND hWnd, SS_COORD Row, SS_COORD Row2);
BOOL DLLENTRY SSDeSelectBlock(HWND hWnd);
int  DLLENTRY SSEnumCustomFunction(HWND hWnd, LPCTSTR lpszPrevFunc,
                                   LPTSTR lpszFunc, int nFuncLen);
int  DLLENTRY SSEnumCustomFunctionLen(HWND hWnd, LPCTSTR lpszPrevFunc);

BOOL DLLENTRY SSExportExcelBook(HWND hWnd, LPCTSTR lpszfileName, 
                                LPCTSTR lpszlogFileName);

BOOL DLLENTRY SSExportExcelBookEx(HWND hWnd, LPCTSTR lpszfileName, 
								  LPCTSTR lpszlogFileName, SHORT nFlags);

BOOL DLLENTRY SSExportRangeToHTML (HWND hWnd, SS_COORD col, 
								   SS_COORD row, SS_COORD col2, SS_COORD row2, 
								   LPCTSTR lpszFileName, BOOL bAppendFlag, 
								   LPCTSTR lpszLogFile);

BOOL DLLENTRY SSExportRangeToTextFile (HWND hWnd, 
									SS_COORD lCol, 
									SS_COORD lRow, 
									SS_COORD lCol2, 
									SS_COORD lRow2, 
									LPCTSTR pcszFileName, 
									LPCTSTR pcszCellDelim, 
									LPCTSTR pcszColDelim, 
									LPCTSTR pcszRowDelim,
									long lFlags, 
									LPCTSTR pcszLogFile);

BOOL DLLENTRY SSExportRangeToTextFileU (HWND hWnd, 
									SS_COORD lCol, 
									SS_COORD lRow, 
									SS_COORD lCol2, 
									SS_COORD lRow2, 
									LPCTSTR pcszFileName, 
									LPCTSTR pcszCellDelim, 
									LPCTSTR pcszColDelim, 
									LPCTSTR pcszRowDelim,
									long lFlags, 
									LPCTSTR pcszLogFile);

BOOL DLLENTRY SSExportRangeToXML (HWND hWnd,SS_COORD lCol, 
											SS_COORD lRow, 
											SS_COORD lCol2, 
											SS_COORD lRow2, 
											LPCTSTR lpszFileName, 
											LPCTSTR pszRoot, 
											LPCTSTR pszCollection,
											long lFlags,
											LPCTSTR lpszLogFile);

BOOL DLLENTRY SSExportRangeToXMLBuffer (HWND hWnd,
										SS_COORD lCol, 
										SS_COORD lRow, 
										SS_COORD lCol2, 
										SS_COORD lRow2, 
										LPCTSTR pszRoot, 
										LPCTSTR pszCollection, 
										HGLOBAL *hgBuff,
										long lFlags,
										LPCTSTR lpszLogFile);

BOOL DLLENTRY SSExportToExcel(HWND hWnd, LPCTSTR lpszFileName,
                              LPCTSTR lpszSheetName, LPCTSTR lpszLogFileName);

BOOL DLLENTRY SSExportToExcelEx(HWND hWnd, LPCTSTR lpszFileName, 
								LPCTSTR lpszSheetName, LPCTSTR lpszLogFileName, SHORT nFlags);

#define SS_TABSTRIPBUTTONPOLICY_ASNEEDED 0
#define SS_TABSTRIPBUTTONPOLICY_ALWAYS   1
#define SS_TABSTRIPBUTTONPOLICY_NEVER    2
#define SS_CELLNOTEINDICATORSHAPE_SQUARE		0
#define SS_CELLNOTEINDICATORSHAPE_TRIANGLE		1
#define SS_SCROLLBARSTYLE_INHERIT		0
#define SS_SCROLLBARSTYLE_CLASSIC		1
#define SS_SCROLLBARSTYLE_VISUALSTYLES	2
#define SS_SCROLLBARSTYLE_ENHANCED		3

#define SS_APPEARANCESTYLE_CLASSIC				0
#define SS_APPEARANCESTYLE_VISUALSTYLES			1
#define SS_APPEARANCESTYLE_ENHANCED				2
#define SS_USEVISUALSTYLES_INHERIT		0
#define SS_USEVISUALSTYLES_YES			1
#define SS_USEVISUALSTYLES_NO			2
#define SS_TABENHANCEDSHAPE_RECTANGLE			0
#define SS_TABENHANCEDSHAPE_ROUNDEDRECTANGLE	1
#define SS_TABENHANCEDSHAPE_TRAPEZOID			2
#define SS_TABENHANCEDSHAPE_ISOSCELESTRAPEZOID	3
#define SS_SHEETTAB_FORECOLOR						    RGB(0,0,0)
#define SS_SHEETTAB_ACTIVEFORECOLOR					 RGB(0,0,0)
#define SS_SHEETTAB_UPPER_STARTCOLOR				 RGB(216, 231, 250)
#define SS_SHEETTAB_UPPER_ENDCOLOR					 RGB(213, 229, 249)
#define SS_SHEETTAB_LOWER_STARTCOLOR				 RGB(182, 210, 245)
#define SS_SHEETTAB_LOWER_ENDCOLOR					 RGB(217, 231, 249)
#define SS_SHEETTAB_UPPER_HOVERSTARTCOLOR			 RGB(255,255,255)
#define SS_SHEETTAB_UPPER_HOVERENDCOLOR				 RGB(255,255,255)
#define SS_SHEETTAB_LOWER_HOVERSTARTCOLOR			 RGB(255,255,255)
#define SS_SHEETTAB_LOWER_HOVERENDCOLOR				 RGB(183, 210, 244)
#define SS_SHEETTAB_OUTERBORDERCOLOR				 RGB(146, 165, 199)
#define SS_SHEETTAB_INNERBORDERCOLOR				 RGB(255,255,255)
#define SS_SHEETTAB_SCROLLBTN_UPPER_NORMALSTARTCOLOR RGB(216, 231, 250)
#define SS_SHEETTAB_SCROLLBTN_UPPER_NORMALENDCOLOR	 RGB(213, 229, 249)
#define SS_SHEETTAB_SCROLLBTN_LOWER_NORMALSTARTCOLOR RGB(182, 210, 245)
#define SS_SHEETTAB_SCROLLBTN_LOWER_NORMALENDCOLOR	 RGB(217, 231, 249)
#define SS_SHEETTAB_SCROLLBTN_UPPER_HOVERSTARTCOLOR  RGB(234, 241, 249)
#define SS_SHEETTAB_SCROLLBTN_UPPER_HOVERENDCOLOR	 RGB(216, 228, 244)
#define SS_SHEETTAB_SCROLLBTN_LOWER_HOVERSTARTCOLOR  RGB(219, 230, 245)
#define SS_SHEETTAB_SCROLLBTN_LOWER_HOVERENDCOLOR	 RGB(232, 239, 249)
#define SS_SHEETTAB_SCROLLBTN_UPPER_PUSHEDSTARTCOLOR RGB(199, 217, 243)
#define SS_SHEETTAB_SCROLLBTN_UPPER_PUSHEDENDCOLOR	 RGB(195, 214, 242)
#define SS_SHEETTAB_SCROLLBTN_LOWER_PUSHEDSTARTCOLOR RGB(146, 182, 232)
#define SS_SHEETTAB_SCROLLBTN_LOWER_PUSHEDENDCOLOR	 RGB(190, 212, 241)
#define SS_SHEETTAB_SCROLLBTN_ARROWCOLOR			 RGB(106,90,205)
#define SS_SHEETTAB_GRAYAREA_STARTCOLOR				 RGB(127, 161, 209)
#define SS_SHEETTAB_GRAYAREA_ENDCOLOR				 RGB(161, 186, 221)
#define SS_SPLITBOX_STARTCOLOR						 RGB(127, 161, 209)
#define SS_SPLITBOX_ENDCOLOR					 	 RGB(161, 186, 221)

#define SS_SCROLLBAR_TRACKCOLOR						 RGB(115, 150, 190)
#define SS_SCROLLBAR_ARROWCOLOR						 RGB(66, 75, 99)
#define SS_SCROLLBAR_NORMAL_UPPER_STARTCOLOR		 RGB(215,230,247)
#define SS_SCROLLBAR_NORMAL_UPPER_ENDCOLOR			 RGB(217,231,249)
#define SS_SCROLLBAR_NORMAL_LOWER_STARTCOLOR		 RGB(190,212,241)
#define SS_SCROLLBAR_NORMAL_LOWER_ENDCOLOR			 RGB(195,214,242)
#define SS_SCROLLBAR_PUSHED_UPPER_STARTCOLOR		 RGB(161,186,221)
#define SS_SCROLLBAR_PUSHED_UPPER_ENDCOLOR			 RGB(170,185,205)
#define SS_SCROLLBAR_PUSHED_LOWER_STARTCOLOR		 RGB(60,110,175)
#define SS_SCROLLBAR_PUSHED_LOWER_ENDCOLOR			 RGB(85,105,145)
#define SS_SCROLLBAR_HOVER_UPPER_STARTCOLOR			 RGB(195,214,242)
#define SS_SCROLLBAR_HOVER_UPPER_ENDCOLOR			 RGB(183,210,244)
#define SS_SCROLLBAR_HOVER_LOWER_STARTCOLOR			 RGB(161,186,221)
#define SS_SCROLLBAR_HOVER_LOWER_ENDCOLOR			 RGB(170,185,205)
#define SS_SCROLLBAR_HOVER_BUTTON_BORDERCOLOR		 RGB(60, 110, 175)
#define SS_SCROLLBAR_BUTTON_BORDERCOLOR				 RGB(85, 105, 145)

#define SS_COLHEADER_SELECTEDHOVER_UPPERCOLOR		 RGB(255, 212, 141)
#define SS_COLHEADER_SELECTEDHOVER_LOWERCOLOR		 RGB(242, 147, 59)
#define SS_COLHEADER_SELECTED_UPPERCOLOR			 RGB(249, 217, 159)
#define SS_COLHEADER_SELECTED_LOWERCOLOR			 RGB(241, 193, 96)
#define SS_COLHEADER_HOVER_UPPERCOLOR				 RGB(223, 226, 228)
#define SS_COLHEADER_HOVER_LOWERCOLOR				 RGB(189, 197, 210)
#define SS_COLHEADER_UPPERCOLOR						 RGB(249, 252, 253)
#define SS_COLHEADER_LOWERCOLOR						 RGB(211, 220, 233)
#define SS_ROWHEADER_SELECTEDHOVER_UPPERCOLOR		 RGB(241, 192, 92)
#define SS_ROWHEADER_SELECTEDHOVER_LOWERCOLOR		 RGB(241, 192, 92)
#define SS_ROWHEADER_SELECTED_UPPERCOLOR			 RGB(255, 213, 141)
#define SS_ROWHEADER_SELECTED_LOWERCOLOR			 RGB(255, 213, 141)
#define SS_ROWHEADER_HOVER_UPPERCOLOR				 RGB(187, 196, 209)
#define SS_ROWHEADER_HOVER_LOWERCOLOR				 RGB(187, 196, 209)
#define SS_ROWHEADER_UPPERCOLOR						 RGB(228, 236, 247)
#define SS_ROWHEADER_LOWERCOLOR						 RGB(228, 236, 247)
#define SS_COLHEADER_SELECTEDBORDERCOLOR			 RGB(242, 149, 54)
#define SS_COLHEADER_BORDERCOLOR					 RGB(158, 182, 206)
#define SS_ROWHEADER_SELECTEDBORDERCOLOR			 RGB(242, 149, 54)
#define SS_ROWHEADER_BORDERCOLOR					 RGB(158, 182, 206)
#define SS_CORNER_BACKCOLOR							 RGB(169, 196, 233)
#define SS_CORNER_HOVER_BACKCOLOR					 RGB(140, 161, 182)
#define SS_CORNER_TRIANGLE_COLOR					 RGB(228, 236, 247)
#define SS_CORNER_TRIANGLE_HOVER_COLOR                RGB(228, 236, 247)
#define SS_CORNER_TRIANGLE_BORDERCOLOR               RGB(169, 196, 233)
#define SS_CORNER_TRIANGLE_HOVER_BORDERCOLOR         RGB(140, 161, 182)
#define SS_HIGHLIGHT_ALPHABLEND                  75

#define SS_HIGHLIGHTSTYLE_DEFAULT				0
#define SS_HIGHLIGHTSTYLE_INVERT					1
#define SS_HIGHLIGHTSTYLE_COLOR					2
#define SS_HIGHLIGHTSTYLE_ALPHABLEND			3

#define SS_HIGHLIGHTHEADERS_DEFAULT				0
#define SS_HIGHLIGHTHEADERS_OFF					1
#define SS_HIGHLIGHTHEADERS_ON					2

WORD DLLENTRY SSGetTabStripButtonPolicy(HWND hWnd);
WORD DLLENTRY SSSetTabStripButtonPolicy(HWND hWnd, WORD wTabStripButtonPolicy);
BOOL DLLENTRY SSIsVerticalScrollBarVisible(HWND hWnd);
BOOL DLLENTRY SSIsHorizontalScrollBarVisible(HWND hWnd);
WORD DLLENTRY SSSetAppearanceStyle(HWND hWnd, WORD nStyle);
WORD DLLENTRY SSGetAppearanceStyle(HWND hWnd);
WORD DLLENTRY SSSetUseVisualStyles(HWND hWnd, WORD nStyle);
WORD DLLENTRY SSGetUseVisualStyles(HWND hWnd);
BOOL DLLENTRY SSSetEnhanceStaticCells(HWND hWnd, BOOL nStyle);
BOOL DLLENTRY SSGetEnhanceStaticCells(HWND hWnd);
WORD DLLENTRY SSSetScrollBarStyle(HWND hWnd, WORD nStyle);
WORD DLLENTRY SSGetScrollBarStyle(HWND hWnd);

WORD DLLENTRY SSSetTabEnhancedShape(HWND hWnd, WORD nStyle);
WORD DLLENTRY SSGetTabEnhancedShape(HWND hWnd);
WORD DLLENTRY SSGetCellNoteIndicatorShape(HWND hWnd);
WORD DLLENTRY SSSetCellNoteIndicatorShape(HWND hWnd, WORD nStyle);
COLORREF DLLENTRY SSGetCellNoteIndicatorColor(HWND hWnd);
COLORREF DLLENTRY SSSetCellNoteIndicatorColor(HWND hWnd, COLORREF Color);

BOOL DLLENTRY SSSetEnhancedCornerColors(HWND hWnd, COLORREF clrBackColor, COLORREF clrHoverColor, COLORREF clrTriangle, COLORREF clrTriangleHover, COLORREF clrTriangleBorderColor, COLORREF clrTriangleHoverBorderColor);

BOOL DLLENTRY SSSetEnhancedColumnHeaderColors(HWND hWnd, COLORREF clrSelectedHoverUpperColor, COLORREF clrSelectedHoverLowerColor, COLORREF clrSelectedUpperColor,
											  COLORREF clrSelectedLowerColor, COLORREF clrHoverUpperColor, COLORREF clrHoverLowerColor, COLORREF clrUpperColor,
											  COLORREF clrLowerColor, COLORREF clrSelectedBorderColor, COLORREF clrBorderColor);
BOOL DLLENTRY SSSetEnhancedRowHeaderColors(HWND hWnd, COLORREF clrSelectedHoverUpperColor, COLORREF clrSelectedHoverLowerColor, COLORREF clrSelectedUpperColor,
											  COLORREF clrSelectedLowerColor, COLORREF clrHoverUpperColor, COLORREF clrHoverLowerColor, COLORREF clrUpperColor,
											  COLORREF clrLowerColor, COLORREF clrSelectedBorderColor, COLORREF clrBorderColor);

BOOL DLLENTRY SSSetEnhancedSheetTabColors(HWND hWnd, COLORREF clrNormalUpperStartColor, COLORREF clrNormalUpperEndColor,
								   COLORREF clrNormalLowerStartColor, COLORREF clrNormalLowerEndColor, COLORREF clrHoverUpperStartColor,
								   COLORREF clrHoverUpperEndColor, COLORREF clrHoverLowerStartColor, COLORREF clrHoverLowerEndColor,
								   COLORREF clrTabOuterBorderColor, COLORREF clrTabInnerBorderColor, COLORREF clrButtonUpperNormalStartColor,
								   COLORREF clrButtonUpperNormalEndColor, COLORREF clrButtonLowerNormalStartColor, COLORREF clrButtonLowerNormalEndColor,
								   COLORREF clrButtonUpperHoverStartColor, COLORREF clrButtonUpperHoverEndColor, COLORREF clrButtonLowerHoverStartColor,
								   COLORREF clrButtonLowerHoverEndColor,COLORREF clrButtonUpperPushedStartColor, COLORREF clrButtonUpperPushedEndColor,
								   COLORREF clrButtonLowerPushedStartColor, COLORREF clrButtonLowerPushedEndColor, COLORREF clrArrowColor,
								   COLORREF clrBackColorStartColor, COLORREF clrBackColorEndColor, COLORREF clrSplitBoxStartColor,
								   COLORREF clrSplitBoxEndColor, COLORREF clrForeColor, COLORREF clrActiveForeColor);

BOOL DLLENTRY SSSetEnhancedScrollBarColors(HWND hWnd, COLORREF clrTrackColor, COLORREF clrArrowColor, COLORREF clrUpperNormalStartColor, COLORREF clrUpperNormalEndColor,
										   COLORREF clrLowerNormalStartColor, COLORREF clrLowerNormalEndColor, COLORREF clrUpperPushedStartColor, COLORREF clrUpperPushedEndColor,
										   COLORREF clrLowerPushedStartColor, COLORREF clrLowerPushedEndColor, COLORREF clrUpperHoverStartColor, COLORREF clrUpperHoverEndColor,
										   COLORREF clrLowerHoverStartColor, COLORREF clrLowerHoverEndColor, COLORREF clrHoverButtonBorderColor, COLORREF clrButtonBorderColor);

WORD DLLENTRY SSSetHighlightHeaders(HWND hWnd, WORD wOption);
WORD DLLENTRY SSGetHighlightHeaders(HWND hWnd);
WORD DLLENTRY SSSetHighlightStyle(HWND hWnd, WORD wStyle);
WORD DLLENTRY SSGetHighlightStyle(HWND hWnd);
BOOL DLLENTRY SSSetHighlightAlphaBlend(HWND hWnd, COLORREF clrAlphaBlend, short nAlphaBlend);
BOOL DLLENTRY SSGetHighlightAlphaBlend(HWND hWnd, LPCOLORREF lpclrAlphaBlend, LPSHORT lpnAlphaBlend);

BOOL DLLENTRY SSExportToHTMLEx (HWND hWnd, LPCTSTR lpszFileName, 
							  BOOL bAppendFlag, LPCTSTR lpszLogFile, LPCTSTR lpszTitle, LPCTSTR lpszCssFile, LPCTSTR lpszTableClass, LPCTSTR lpszTableRowClass, LPCTSTR lpszTableDefinitionClass, LPCTSTR lpszTableHeaderClass);
BOOL DLLENTRY SSExportRangeToHTMLEx (HWND hWnd, SS_COORD col, 
								   SS_COORD row, SS_COORD col2, SS_COORD row2, 
								   LPCTSTR lpszFileName, BOOL bAppendFlag, 
								   LPCTSTR lpszLogFile, LPCTSTR lpszTitle,
								   LPCTSTR lpszCssFile, LPCTSTR lpszTableClass, LPCTSTR lpszTableRowClass, LPCTSTR lpszTableDefinitionClass, LPCTSTR lpszTableHeaderClass);
BOOL DLLENTRY SSExportToHTML (HWND hWnd, LPCTSTR lpszFileName, 
							  BOOL bAppendFlag, LPCTSTR lpszLogFile);

BOOL DLLENTRY SSExportToTextFile (HWND hWnd, 
					  			LPCTSTR pcszFileName, 
					  			LPCTSTR pcszCellDelim, 
					  			LPCTSTR pcszColDelim, 
					  			LPCTSTR pcszRowDelim,
					  			long lFlags,
								LPCTSTR pcszLogFile);

BOOL DLLENTRY SSExportToTextFileU (HWND hWnd, 
					  			LPCTSTR pcszFileName, 
					  			LPCTSTR pcszCellDelim, 
					  			LPCTSTR pcszColDelim, 
					  			LPCTSTR pcszRowDelim,
					  			long lFlags,
								  LPCTSTR pcszLogFile);

BOOL DLLENTRY SSExportToXML (HWND hWnd,	LPCTSTR lpszFileName, 
									   	LPCTSTR pszRoot, 
									   	LPCTSTR pszCollection,
										long lFlags,
									   	LPCTSTR lpszLogFile);

BOOL DLLENTRY SSExportToXMLBuffer (HWND hWnd,	
									LPCTSTR pszRoot, 
									LPCTSTR pszCollection, 
									HGLOBAL *hgBuff,
									long lFlags,
									LPCTSTR lpszLogFile);

void DLLENTRY SSFloatFormat(HWND hWnd, LPSS_CELLTYPE lpCellType,
                            double dfVal, LPTSTR lpszText, BOOL fValue);

BOOL DLLENTRY SSGetActionKey(HWND hWnd, WORD wAction, LPBOOL lpfShift, 
							 LPBOOL lpfCtrl, LPWORD lpwKey);
BOOL DLLENTRY SSSetActionKey(HWND hWnd, WORD wAction, BOOL fShift, 
							 BOOL fCtrl, WORD wKey);

BOOL DLLENTRY SSGetActiveCell(HWND hWnd, LPSS_COORD Col, LPSS_COORD Row);
BOOL DLLENTRY SSSetActiveCell(HWND hWnd, SS_COORD Col, SS_COORD Row);

WORD DLLENTRY SSGetActiveCellHighlightStyle(HWND hWnd);
void DLLENTRY SSSetActiveCellHighlightStyle(HWND hWnd, WORD wActiveCellHighlightStyle);

short DLLENTRY SSGetActiveSheet(HWND hWnd);
BOOL DLLENTRY SSSetActiveSheet(HWND hWnd, short nSheet);

BOOL DLLENTRY SSAddCellSpan(HWND hWnd, SS_COORD lCol, SS_COORD lRow, SS_COORD lNumCols, SS_COORD lNumRows);
WORD DLLENTRY SSGetCellSpan(HWND hWnd, SS_COORD lCol, SS_COORD lRow, LPSS_COORD lplColAnchor,
                            LPSS_COORD lplRowAnchor, LPSS_COORD lplNumCols, LPSS_COORD lplNumRows);

WORD DLLENTRY SSGetAppearance(HWND hWnd);
WORD DLLENTRY SSSetAppearance(HWND hWnd, WORD wAppearance);

BOOL DLLENTRY SSGetArray(HWND hWnd, LPVOID lpArray, SS_COORD ColLeft,
                         SS_COORD RowTop, SS_COORD ArrayColCnt,
                         SS_COORD ArrayRowCnt, WORD wDataType);
BOOL DLLENTRY SSSetArray(HWND hWnd, LPVOID lpArray, SS_COORD ColLeft,
                         SS_COORD RowTop, SS_COORD ArrayColCnt,
                         SS_COORD ArrayRowCnt, WORD wDataType);

void DLLENTRY SSGetAutoSizeVisible(HWND hWnd, LPSS_COORD lpVisibleCols, LPSS_COORD lpVisibleRows);
void DLLENTRY SSSetAutoSizeVisible(HWND hWnd, SS_COORD VisibleCols, SS_COORD VisibleRows);

WORD DLLENTRY SSGetBackColorStyle(HWND hWnd);
WORD DLLENTRY SSSetBackColorStyle(HWND hWnd, WORD wStyle);

BOOL DLLENTRY SSGetBiffBuffer(HWND hWnd, GLOBALHANDLE FAR *lpghBuffer, LPLONG lplBufferLen);

BOOL DLLENTRY SSGetBool(HWND hWnd, short nIndex);
BOOL DLLENTRY SSSetBool(HWND hWnd, short nIndex, BOOL bNewVal);

BOOL DLLENTRY SSGetBorder(HWND hWnd, SS_COORD Col, SS_COORD Row,
                          LPWORD lpwBorderType, LPWORD lpwBorderStyle,
                          LPCOLORREF lpColor);
BOOL DLLENTRY SSSetBorder(HWND hWnd, SS_COORD Col, SS_COORD Row,
                          WORD wBorderType, WORD wBorderStyle,
                          COLORREF Color);
BOOL DLLENTRY SSSetBorderRange(HWND hWnd, SS_COORD Col, SS_COORD Row,
                               SS_COORD Col2, SS_COORD Row2,
                               WORD wBorderType, WORD wBorderStyle,
                               COLORREF Color);

BOOL DLLENTRY SSGetBorderEx(HWND hWnd, SS_COORD Col, SS_COORD Row,
                            LPWORD lpwStyleLeft, LPCOLORREF lpclrLeft,
                            LPWORD lpwStyleTop, LPCOLORREF lpclrTop,
                            LPWORD lpwStyleRight, LPCOLORREF lpclrRight,
                            LPWORD lpwStyleBottom, LPCOLORREF lpclrBottom);

BOOL DLLENTRY SSGetBottomRightCell(HWND hWnd, LPSS_COORD lpCol,
                                   LPSS_COORD lpRow);

WORD DLLENTRY SSGetButtonDrawMode(HWND hWnd);
WORD DLLENTRY SSSetButtonDrawMode(HWND hWnd, WORD wMode);

int  DLLENTRY SSGetButtonText(HWND hWnd, SS_COORD Col, SS_COORD Row,
                              LPTSTR lpText);
BOOL DLLENTRY SSSetButtonText(HWND hWnd, SS_COORD Col, SS_COORD Row,
                              LPCTSTR lpText);

BOOL DLLENTRY SSGetBuffer(HWND hWnd, SS_COORD Col, SS_COORD Row, LPVOID lpBuffer, long *plSize);
BOOL DLLENTRY SSSetBuffer(HWND hWnd, SS_COORD Col, SS_COORD Row, LPVOID lpBuffer, long lSize);

WNDPROC DLLENTRY SSGetCallBack(HWND hWnd);
WNDPROC DLLENTRY SSSetCallBack(HWND hWnd, WNDPROC lpfnCallBack);

void DLLENTRY SSSetCalText(LPCTSTR lpszShortDays, LPCTSTR lpszLongDays,
                           LPCTSTR lpszShortMonths, LPCTSTR lpszLongMonths,
                           LPCTSTR lpszOkText, LPCTSTR lpszCancelText);
void DLLENTRY SSGetCalTextOverride(HWND hWnd, LPTSTR lpszShortDays, LPSHORT lpLenShortDays, 
                                   LPTSTR lpszLongDays, LPSHORT lpLenLongDays,
                                   LPTSTR lpszShortMonths, LPSHORT lpLenShortMonths, 
                                   LPTSTR lpszLongMonths, LPSHORT lpLenLongMonths,
                                   LPTSTR lpszOkText, LPSHORT lpLenOkText, 
                                   LPTSTR lpszCancelText, LPSHORT lpLenCancelText);
void DLLENTRY SSSetCalTextOverride(HWND hWnd, LPCTSTR lpszShortDays, LPCTSTR lpszLongDays,
                                   LPCTSTR lpszShortMonths, LPCTSTR lpszLongMonths,
                                   LPCTSTR lpszOkText, LPCTSTR lpszCancelText);

BOOL DLLENTRY SSGetCellFromPixel(HWND hWnd, LPSS_COORD lpCol,
                                 LPSS_COORD lpRow, int MouseX, int MouseY);

short DLLENTRY SSGetCellNote(HWND hWnd, SS_COORD Col, SS_COORD Row, LPTSTR Note);
BOOL DLLENTRY SSSetCellNote(HWND hWnd, SS_COORD Col, SS_COORD Row, LPTSTR Note);
BOOL DLLENTRY SSSetCellNoteRange(HWND hWnd, SS_COORD Col, SS_COORD Row, SS_COORD Col2, SS_COORD Row2,
								 LPTSTR Note);

WORD DLLENTRY SSGetCellNoteIndicator(HWND hWnd);
void DLLENTRY SSSetCellNoteIndicator(HWND hWnd, WORD wValue);

BOOL DLLENTRY SSGetCellRect(HWND hWnd, SS_COORD Col, SS_COORD Row,
                            LPRECT lpRect);

void DLLENTRY SSGetCellSendingMsg(HWND hWnd, LPSS_COORD lpCol,
                                  LPSS_COORD lpRow);

short DLLENTRY SSGetCellTag(HWND hWnd, SS_COORD Col, SS_COORD Row, LPTSTR CellTag);
BOOL DLLENTRY  SSSetCellTag(HWND hWnd, SS_COORD Col, SS_COORD Row, LPCTSTR CellTag);

BOOL DLLENTRY SSGetCellType(HWND hWnd, SS_COORD Col, SS_COORD Row,
                            LPSS_CELLTYPE lpCellType);
BOOL DLLENTRY SSSetCellType(HWND hWnd, SS_COORD Col, SS_COORD Row,
                            LPSS_CELLTYPE lpCellType);
BOOL DLLENTRY SSSetCellTypeRange(HWND hWnd, SS_COORD Col, SS_COORD Row,
                                 SS_COORD Col2, SS_COORD Row2,
                                 LPSS_CELLTYPE lpCellType);

void DLLENTRY SSGetClientRect(HWND hWnd, LPRECT lpRect);

WORD DLLENTRY SSGetClipboardOptions(HWND hWnd);
WORD DLLENTRY SSSetClipboardOptions(HWND hWnd, WORD wOptions);

SS_COORD DLLENTRY SSGetColFromID(HWND hWnd, LPCTSTR lpszColID);

WORD DLLENTRY SSGetColHeaderDisplay(HWND hWnd);
WORD DLLENTRY SSSetColHeaderDisplay(HWND hWnd, WORD wDisplay);

SS_COORD DLLENTRY SSGetColHeaderRows(HWND hWnd);
void DLLENTRY     SSSetColHeaderRows(HWND hWnd, SS_COORD lHeaderRows);

SS_COORD DLLENTRY SSGetColHeadersAutoTextIndex(HWND hWnd);
void DLLENTRY     SSSetColHeadersAutoTextIndex(HWND hWnd, SS_COORD lColHeadersAutoTextIndex);

SS_COORD DLLENTRY SSGetColHeadersUserSortIndex(HWND hWnd);
void DLLENTRY     SSSetColHeadersUserSortIndex(HWND hWnd, SS_COORD lColHeadersUserSortIndex);

short DLLENTRY SSGetColID(HWND hWnd, SS_COORD Col, LPTSTR ColID);
BOOL  DLLENTRY SSSetColID(HWND hWnd, SS_COORD Col, LPCTSTR ColID);

short DLLENTRY SSGetColMerge(HWND hWnd, SS_COORD Col);
void  DLLENTRY SSSetColMerge(HWND hWnd, SS_COORD Col, short nColMerge);

BOOL DLLENTRY SSGetColor(HWND hWnd, SS_COORD Col, SS_COORD Row,
                         LPCOLORREF lpBackground, LPCOLORREF lpForeground);
BOOL DLLENTRY SSSetColor(HWND hWnd, SS_COORD Col, SS_COORD Row,
                         COLORREF Background, COLORREF Foreground);
BOOL DLLENTRY SSSetColorRange(HWND hWnd, SS_COORD Col, SS_COORD Row,
                              SS_COORD Col2, SS_COORD Row2,
                              COLORREF Background, COLORREF Foreground);

BOOL DLLENTRY SSGetColPageBreak(HWND hWnd, SS_COORD Col);
BOOL DLLENTRY SSSetColPageBreak(HWND hWnd, SS_COORD Col, BOOL fPageBreak);

#if defined(_WIN64) || defined(_IA64)
BOOL DLLENTRY SSGetColUserData(HWND hWnd, SS_COORD Col, LONG_PTR* lplUserData);
BOOL DLLENTRY SSSetColUserData(HWND hWnd, SS_COORD Col, LONG_PTR lUserData);
#else
BOOL DLLENTRY SSGetColUserData(HWND hWnd, SS_COORD Col, LPLONG lplUserData);
BOOL DLLENTRY SSSetColUserData(HWND hWnd, SS_COORD Col, long lUserData);
#endif

short DLLENTRY SSGetColUserSortIndicator(HWND hWnd, SS_COORD Col);
void  DLLENTRY SSSetColUserSortIndicator(HWND hWnd, SS_COORD Col, short nColUserSortIndicator);

BOOL DLLENTRY SSGetColWidth(HWND hWnd, SS_COORD Col, LPDOUBLE lpdfColWidth);
BOOL DLLENTRY SSSetColWidth(HWND hWnd, SS_COORD Col, double dfWidth);

BOOL DLLENTRY SSGetColWidthInPixels(HWND hWnd, SS_COORD Col, LPINT lpdWidth);
BOOL DLLENTRY SSSetColWidthInPixels(HWND hWnd, SS_COORD Col, int dWidth);

BOOL DLLENTRY SSGetCurrSelBlockPos(HWND hWnd, LPSS_COORD CurrentCol,
                                   LPSS_COORD CurrentRow);

HCURSOR DLLENTRY SSGetCursor(HWND hWnd, WORD wType);
BOOL DLLENTRY SSSetCursor(HWND hWnd, WORD wType, HCURSOR hCursor);

HWND DLLENTRY SSGetCustomEditor(HWND hWnd, LPCTSTR lpszName, long lStyle);
HWND DLLENTRY SSGetCustomRenderer(HWND hWnd, LPCTSTR lpszName, long lStyle);
BOOL DLLENTRY SSGetCustomItemData(HWND hWnd, SS_COORD CellCol, SS_COORD CellRow, LPSS_CT_VALUE ItemData);
BOOL DLLENTRY SSSetCustomItemData(HWND hWnd, SS_COORD CellCol, SS_COORD CellRow, LPSS_CT_VALUE ItemData);

BOOL DLLENTRY SSGetCustomFunction(HWND hWnd, LPCTSTR lpszFunc,
                                  LPSHORT lpnMinParamCnt,
                                  LPSHORT lpnMaxParamCnt,
                                  LPLONG lplFlags);

int  DLLENTRY SSGetCustomName(HWND hWnd, LPCTSTR lpszName,
                              LPTSTR lpszValue, int nValueLen);
BOOL DLLENTRY SSSetCustomName(HWND hWnd, LPCTSTR lpszName, 
							  LPCTSTR lpszValue);

int  DLLENTRY SSGetCustomNameLen(HWND hWnd, LPCTSTR lpszName);

int  DLLENTRY SSGetData(HWND hWnd, SS_COORD Col, SS_COORD Row, LPTSTR Data);
BOOL DLLENTRY SSSetData(HWND hWnd, SS_COORD Col, SS_COORD Row, LPCTSTR lpData);
BOOL DLLENTRY SSSetDataRange(HWND hWnd, SS_COORD Col, SS_COORD Row,
                             SS_COORD Col2, SS_COORD Row2, LPCTSTR lpData);

BOOL DLLENTRY SSGetDataCnt(HWND hWnd, LPSS_COORD lpColCnt,
                           LPSS_COORD lpRowCnt);

int  DLLENTRY SSGetDataLen(HWND hWnd, SS_COORD Col, SS_COORD Row);

BOOL DLLENTRY SSGetDefFloatFormat(HWND hWnd, LPSS_FLOATFORMAT lpFloatFormat);
BOOL DLLENTRY SSSetDefFloatFormat(HWND hWnd, LPSS_FLOATFORMAT lpFloatFormat);

WORD DLLENTRY SSGetEditEnterAction(HWND hWnd);
WORD DLLENTRY SSSetEditEnterAction(HWND hWnd, WORD wAction);

BOOL DLLENTRY SSGetEditMode(HWND hWnd);
BOOL DLLENTRY SSSetEditMode(HWND hWnd, BOOL fEditModeOn);

BOOL DLLENTRY SSGetExcelSheetList(HWND hWnd, LPCTSTR lpszFileName, 
                                  GLOBALHANDLE FAR *lpghList, LPSHORT lpnListCount,
                                  LPCTSTR lpszLogFileName, LPSHORT lpnWorkbookHandle,
                                  BOOL fReplace);

BOOL DLLENTRY SSGetFirstValidCell(HWND hWnd, LPSS_COORD lpCol,
                                  LPSS_COORD lpRow);

BOOL DLLENTRY SSGetFloat(HWND hWnd, SS_COORD Col, SS_COORD Row,
                         LPDOUBLE lpdfValue);
BOOL DLLENTRY SSSetFloat(HWND hWnd, SS_COORD Col, SS_COORD Row,
                         double dfValue);
BOOL DLLENTRY SSSetFloatRange(HWND hWnd, SS_COORD Col, SS_COORD Row,
                              SS_COORD Col2, SS_COORD Row2, double dfValue);

HFONT DLLENTRY SSGetFont(HWND hWnd, SS_COORD Col, SS_COORD Row);
BOOL DLLENTRY SSSetFont(HWND hWnd, SS_COORD Col, SS_COORD Row,
                        HFONT hFont, BOOL fDeleteFont);
BOOL DLLENTRY SSSetFontRange(HWND hWnd, SS_COORD Col, SS_COORD Row,
                             SS_COORD Col2, SS_COORD Row2,
                             HFONT hFont, BOOL fDeleteFont);

int  DLLENTRY SSGetFormula(HWND hWnd, SS_COORD Col, SS_COORD Row,
                           LPTSTR lpFormula);
BOOL DLLENTRY SSSetFormula(HWND hWnd, SS_COORD Col, SS_COORD Row,
                           LPCTSTR Formula, BOOL BuildDependencies);
BOOL DLLENTRY SSSetFormulaRange(HWND hWnd, SS_COORD Col, SS_COORD Row,
                                SS_COORD Col2, SS_COORD Row2,
                                LPCTSTR Formula, BOOL BuildDependencies);

int  DLLENTRY SSGetFormulaLen(HWND hWnd, SS_COORD Col, SS_COORD Row);

BOOL DLLENTRY SSGetFreeze(HWND hWnd, LPSS_COORD lpColsFrozen,
                          LPSS_COORD lpRowsFrozen);
BOOL DLLENTRY SSSetFreeze(HWND hWnd, SS_COORD ColsFrozen, SS_COORD RowsFrozen);

void DLLENTRY SSGetGrayAreaColor(HWND hWnd, LPCOLORREF lpBackground,
                                 LPCOLORREF lpForeground);
void DLLENTRY SSSetGrayAreaColor(HWND hWnd, COLORREF Background,
                                 COLORREF Foreground);

COLORREF DLLENTRY SSGetGridColor(HWND hWnd);
COLORREF DLLENTRY SSSetGridColor(HWND hWnd, COLORREF Color);

WORD DLLENTRY SSGetGridType(HWND hWnd);
WORD DLLENTRY SSSetGridType(HWND hWnd, WORD wGridType);

BOOL DLLENTRY SSGetInteger(HWND hWnd, SS_COORD Col, SS_COORD Row,
                           LPLONG lplValue);
BOOL DLLENTRY SSSetInteger(HWND hWnd, SS_COORD Col, SS_COORD Row, long lValue);
BOOL DLLENTRY SSSetIntegerRange(HWND hWnd, SS_COORD Col, SS_COORD Row,
                                SS_COORD Col2, SS_COORD Row2, long lValue);

BOOL DLLENTRY SSGetIteration(HWND hWnd, LPSHORT lpnMaxIterations,
                             LPDOUBLE lpdfMaxChange);
BOOL DLLENTRY SSSetIteration(HWND hWnd, BOOL bIteration,
                             short nMaxIterations, double dfMaxChange);

BOOL DLLENTRY SSGetLastValidCell(HWND hWnd, LPSS_COORD lpCol,
                                 LPSS_COORD lpRow);

BOOL DLLENTRY SSGetLock(HWND hWnd, SS_COORD Col, SS_COORD Row);
BOOL DLLENTRY SSSetLock(HWND hWnd, SS_COORD Col, SS_COORD Row, BOOL Lock);
BOOL DLLENTRY SSSetLockRange(HWND hWnd, SS_COORD Col, SS_COORD Row,
                             SS_COORD Col2, SS_COORD Row2, BOOL Lock);

void DLLENTRY SSGetLockColor(HWND hWnd, LPCOLORREF lpBackground,
                             LPCOLORREF lpForeground);
void DLLENTRY SSSetLockColor(HWND hWnd, COLORREF Background,
                             COLORREF Foreground);

SS_COORD DLLENTRY SSGetMaxCols(HWND hWnd);
void DLLENTRY SSSetMaxCols(HWND hWnd, SS_COORD MaxCols);

SS_COORD DLLENTRY SSGetMaxRows(HWND hWnd);
void DLLENTRY SSSetMaxRows(HWND hWnd, SS_COORD MaxRows);

BOOL DLLENTRY SSGetMaxTextCellSize(HWND hWnd, SS_COORD Col, SS_COORD Row,
                                   LPDOUBLE lpdfWidth, LPDOUBLE lpdfHeight);

BOOL DLLENTRY SSGetMaxTextColWidth(HWND hWnd, SS_COORD Col,
                                   LPDOUBLE lpdfColWidth);

BOOL DLLENTRY SSGetMaxTextRowHeight(HWND hWnd, SS_COORD Row,
                                    LPDOUBLE lpdfRowHeight);

HGLOBAL DLLENTRY SSGetMultiSelBlocks(HWND hWnd, LPSHORT lpdSelBlockCnt);
BOOL DLLENTRY SSSetMultiSelBlocks(HWND hWnd, LPSS_SELBLOCK lpSelBlockList,
                                  short dSelBlockCnt);

long DLLENTRY SSGetNextPageBreakCol(HWND hWnd, long lPrevCol);
long DLLENTRY SSGetNextPageBreakRow(HWND hWnd, long lPrevRow);

BOOL DLLENTRY SSGetOddEvenRowColor(HWND hWnd, LPCOLORREF lpclrBackOdd, 
								   LPCOLORREF lpclrForeOdd, 
								   LPCOLORREF lpclrBackEven, 
								   LPCOLORREF lpclrForeEven);
BOOL DLLENTRY SSSetOddEvenRowColor(HWND hWnd, COLORREF clrBackOdd, 
								   COLORREF clrForeOdd, 
								   COLORREF clrBackEven, 
								   COLORREF clrForeEven);

WORD DLLENTRY SSGetOperationMode(HWND hWnd);
WORD DLLENTRY SSSetOperationMode(HWND hWnd, WORD wMode);

HWND DLLENTRY SSGetOwner(HWND hWnd);
HWND DLLENTRY SSSetOwner(HWND hWnd, HWND hWndOwner);

short DLLENTRY SSGetPrintAbortMsg(HWND hWnd, LPTSTR lpszText, short nLen);
BOOL DLLENTRY SSSetPrintAbortMsg(HWND hWnd, LPTSTR lpszText);

short DLLENTRY SSGetPrintJobName(HWND hWnd, LPTSTR lpszText, short nLen);
BOOL DLLENTRY SSSetPrintJobName(HWND hWnd, LPTSTR lpszText);

#if defined(_WIN64) || defined(_IA64)
void DLLENTRY SSSetPrintOptions(HWND hWnd, LPSS_PRINTFORMAT lpPrintFormat,
                                SS_PRINTFUNC lpfnPrintProc, LONG_PTR lAppData);
BOOL DLLENTRY SSGetPrintOptions(HWND hWnd, LPSS_PRINTFORMAT lpPrintFormat,
                                SS_PRINTFUNC FAR *lpfnPrintProcAddr,
                                LONG_PTR *lplAppData);
void DLLENTRY SSGetPrintOptionsEx(HWND hWnd, LPSS_PAGEFORMAT lpPageFormat,
                                  SS_PRINTFUNC *lpfnPrintProc, LONG_PTR *lplAppData);
void DLLENTRY SSSetPrintOptionsEx(HWND hWnd, LPSS_PAGEFORMAT lpPageFormat,
                                  SS_PRINTFUNC lpfnPrintProc, LONG_PTR lAppData);
#else
void DLLENTRY SSSetPrintOptions(HWND hWnd, LPSS_PRINTFORMAT lpPrintFormat,
                                SS_PRINTFUNC lpfnPrintProc, LONG lAppData);
BOOL DLLENTRY SSGetPrintOptions(HWND hWnd, LPSS_PRINTFORMAT lpPrintFormat,
                                SS_PRINTFUNC FAR *lpfnPrintProcAddr,
                                LPLONG lplAppData);
void DLLENTRY SSGetPrintOptionsEx(HWND hWnd, LPSS_PAGEFORMAT lpPageFormat,
                                  SS_PRINTFUNC *lpfnPrintProc, LPLONG lplAppData);
void DLLENTRY SSSetPrintOptionsEx(HWND hWnd, LPSS_PAGEFORMAT lpPageFormat,
                                  SS_PRINTFUNC lpfnPrintProc, LONG lAppData);
#endif

long DLLENTRY SSGetPrintPageCount(HWND hWnd);

short DLLENTRY SSGetRefStyle(HWND hWnd);
short DLLENTRY SSSetRefStyle(HWND hWnd, short nStyle);

SS_COORD DLLENTRY SSGetRowHeaderCols(HWND hWnd);
void DLLENTRY     SSSetRowHeaderCols(HWND hWnd, SS_COORD lRowHeaderCols);

WORD DLLENTRY SSGetRowHeaderDisplay(HWND hWnd);
WORD DLLENTRY SSSetRowHeaderDisplay(HWND hWnd, WORD wDisplay);

SS_COORD DLLENTRY SSGetRowHeadersAutoTextIndex(HWND hWnd);
void DLLENTRY     SSSetRowHeadersAutoTextIndex(HWND hWnd, SS_COORD lRowHeadersAutoTextIndex);

BOOL DLLENTRY SSGetRowHeight(HWND hWnd, SS_COORD Row, LPDOUBLE lpdfRowHeight);
BOOL DLLENTRY SSSetRowHeight(HWND hWnd, SS_COORD Row, double dfHeight);

BOOL DLLENTRY SSGetRowHeightInPixels(HWND hWnd, SS_COORD Row, LPINT lpdHeight);
BOOL DLLENTRY SSSetRowHeightInPixels(HWND hWnd, SS_COORD Row, int dHeight);

short DLLENTRY SSGetRowMerge(HWND hWnd, SS_COORD Row);
void  DLLENTRY SSSetRowMerge(HWND hWnd, SS_COORD Row, short nRowMerge);

BOOL DLLENTRY SSGetRowPageBreak(HWND hWnd, SS_COORD Row);
BOOL DLLENTRY SSSetRowPageBreak(HWND hWnd, SS_COORD Row, BOOL fPageBreak);

#if defined(_WIN64) || defined(_IA64)
BOOL DLLENTRY SSGetRowUserData(HWND hWnd, SS_COORD Row, LONG_PTR* lplUserData);
BOOL DLLENTRY SSSetRowUserData(HWND hWnd, SS_COORD Row, LONG_PTR lUserData);
#else
BOOL DLLENTRY SSGetRowUserData(HWND hWnd, SS_COORD Row, LPLONG lplUserData);
BOOL DLLENTRY SSSetRowUserData(HWND hWnd, SS_COORD Row, long lUserData);
#endif

void DLLENTRY SSGetScrollBarColor(HWND hWnd, LPCOLORREF lpVScrollBarColor, LPCOLORREF lpHScrollBarColor);
void DLLENTRY SSSetScrollBarColor(HWND hWnd, COLORREF VScrollBarColor, COLORREF HScrollBarColor);

void DLLENTRY SSGetScrollBarSize(HWND hWnd, LPSHORT lpdVScrollWidth, LPSHORT lpdHScrollHeight);
void DLLENTRY SSSetScrollBarSize(HWND hWnd, short dVScrollWidth, short dHScrollHeight);

BOOL DLLENTRY SSGetSel(HWND hWnd, LPINT SelStart, LPINT SelEnd);
BOOL DLLENTRY SSSetSel(HWND hWnd, int SelStart, int SelEnd);

WORD DLLENTRY SSGetSelBlockOptions(HWND hWnd);
WORD DLLENTRY SSSetSelBlockOptions(HWND hWnd, WORD wOption);

void DLLENTRY SSGetSelColor(HWND hWnd, LPCOLORREF lpclrBack, LPCOLORREF lpclrFore);
void DLLENTRY SSSetSelColor(HWND hWnd, COLORREF clrBack, COLORREF clrFore);

BOOL DLLENTRY SSGetSelectBlock(HWND hWnd, LPSS_CELLCOORD CellUL,
                               LPSS_CELLCOORD CellLR);
BOOL DLLENTRY SSSetSelectBlock(HWND hWnd, LPSS_CELLCOORD CellUL,
                               LPSS_CELLCOORD CellLR);

HGLOBAL DLLENTRY SSGetSelText(HWND hWnd);

void DLLENTRY SSGetShadowColor(HWND hWnd, LPCOLORREF lpShadowColor,
                               LPCOLORREF lpShadowText,
                               LPCOLORREF lpShadowDark,
                               LPCOLORREF lpShadowLight);
void DLLENTRY SSSetShadowColor(HWND hWnd, COLORREF ShadowColor,
                               COLORREF ShadowText, COLORREF ShadowDark,
                               COLORREF ShadowLight);

short DLLENTRY SSGetSheet(HWND hWnd);
void DLLENTRY SSSetSheet(HWND hWnd, short nSheet);

short DLLENTRY SSGetSheetCount(HWND hWnd);
BOOL DLLENTRY SSSetSheetCount(HWND hWnd, short nSheetCnt);

short DLLENTRY SSGetSheetName(HWND hWnd, short nSheet, LPTSTR lpszName, short nLen);
BOOL DLLENTRY SSSetSheetName(HWND hWnd, short nSheet, LPCTSTR lpszSheetName);

short DLLENTRY SSGetSheetSendingMsg(HWND hWnd);

BOOL DLLENTRY SSGetSheetVisible(HWND hWnd, short nSheet);
BOOL DLLENTRY SSSetSheetVisible(HWND hWnd, short nSheet, BOOL fVisible);

WORD DLLENTRY SSGetShowScrollTips(HWND hWnd);
void DLLENTRY SSSetShowScrollTips(HWND hWnd, WORD wScrollTips);

void DLLENTRY SSGetStartingNumbers(HWND hWnd, LPSS_COORD lpStartingColNumber,
                                   LPSS_COORD lpStartingRowNumber);
void DLLENTRY SSSetStartingNumbers(HWND hWnd, SS_COORD StartingColNumber,
                                   SS_COORD StartingRowNumber);

HFONT DLLENTRY SSGetTabStripFont(HWND hWnd);
BOOL DLLENTRY SSSetTabStripFont(HWND hWnd, HFONT hFont);

short DLLENTRY SSGetTabStripLeftSheet(HWND hWnd);
BOOL DLLENTRY SSSetTabStripLeftSheet(HWND hWnd, short nSheet);

WORD DLLENTRY SSGetTabStripPolicy(HWND hWnd);
WORD DLLENTRY SSSetTabStripPolicy(HWND hWnd, WORD wTabStripPolicy);

double DLLENTRY SSGetTabStripRatio(HWND hWnd);
double DLLENTRY SSSetTabStripRatio(HWND hWnd, double nTabStripRatio);

BOOL DLLENTRY SSGetTextTip(HWND hWnd, LPWORD lpwStatus, LPLONG lplDelay,
                           LPLOGFONT lpLogFont, LPCOLORREF lpclrBack,
                           LPCOLORREF lpclrFore);
BOOL DLLENTRY SSSetTextTip(HWND hWnd, WORD wStatus, LONG lDelay,
                           LPLOGFONT lpLogFont, COLORREF clrBack,
                           COLORREF clrFore);

BOOL DLLENTRY SSGetTopLeftCell(HWND hWnd, LPSS_COORD lpCol, LPSS_COORD lpRow);
void DLLENTRY SSGetTopLeftPrev(HWND hWnd, LPSS_COORD lpCol, LPSS_COORD lpRow);

short DLLENTRY SSGetTwoDigitYearMax(HWND hWnd);
BOOL DLLENTRY  SSSetTwoDigitYearMax(HWND hWnd, short nTwoDigitYearMax);

WORD DLLENTRY SSGetUnitType(HWND hWnd);
WORD DLLENTRY SSSetUnitType(HWND hWnd, WORD wUnitType);

short DLLENTRY SSGetUserColAction(HWND hWnd);
void  DLLENTRY SSSetUserColAction(HWND hWnd, short nUserColAction);

#if defined(_WIN64) || defined(_IA64)
LONG_PTR DLLENTRY SSGetUserData(HWND hWnd);
LONG_PTR DLLENTRY SSSetUserData(HWND hWnd, LONG_PTR lUserData);
#else
LONG DLLENTRY SSGetUserData(HWND hWnd);
LONG DLLENTRY SSSetUserData(HWND hWnd, LONG lUserData);
#endif

WORD DLLENTRY SSGetUserResize(HWND hWnd);
WORD DLLENTRY SSSetUserResize(HWND hWnd, WORD wUserResize);

short DLLENTRY SSGetUserResizeCol(HWND hWnd, SS_COORD Col);
short DLLENTRY SSSetUserResizeCol(HWND hWnd, SS_COORD Col, short dOption);

short DLLENTRY SSGetUserResizeRow(HWND hWnd, SS_COORD Row);
short DLLENTRY SSSetUserResizeRow(HWND hWnd, SS_COORD Row, short dOption);

int  DLLENTRY SSGetValue(HWND hWnd, SS_COORD Col, SS_COORD Row,
                         LPTSTR lpBuffer);
BOOL DLLENTRY SSSetValue(HWND hWnd, SS_COORD Col, SS_COORD Row,
                         LPCTSTR lpData);
BOOL DLLENTRY SSSetValueRange(HWND hWnd, SS_COORD Col, SS_COORD Row,
                              SS_COORD Col2, SS_COORD Row2, LPCTSTR lpData);

int  DLLENTRY SSGetValueLen(HWND hWnd, SS_COORD Col, SS_COORD Row);

BOOL DLLENTRY SSGetVisible(HWND hWnd, SS_COORD Col, SS_COORD Row,
                           short Visible);
BOOL DLLENTRY SSImportExcelBook(HWND hWnd, LPCTSTR lpszfileName, LPCTSTR lpszlogFileName);
BOOL DLLENTRY SSImportExcelSheet(HWND hWnd, short nWorkbookHandle, short nSheetIndex, 
                                 LPCTSTR lpszSheetName);
BOOL DLLENTRY SSInsCol(HWND hWnd, SS_COORD Col);
BOOL DLLENTRY SSInsColRange(HWND hWnd, SS_COORD Col, SS_COORD Col2);
BOOL DLLENTRY SSInsertSheets(HWND hWnd, short nSheetIndex, short nSheetCnt);
BOOL DLLENTRY SSInsRow(HWND hWnd, SS_COORD Row);
BOOL DLLENTRY SSInsRowRange(HWND hWnd, SS_COORD Row, SS_COORD Row2);
BOOL DLLENTRY SSIsCellInSelection(HWND hWnd, SS_COORD Col, SS_COORD Row);
BOOL DLLENTRY SSIsColHidden(HWND hWnd, SS_COORD Col);
int  DLLENTRY SSIsExcelFile(LPCTSTR lpszFileName);
BOOL DLLENTRY SSIsFetchCellNote(HWND hWnd);
BOOL DLLENTRY SSIsRowHidden(HWND hWnd, SS_COORD Row);
BOOL DLLENTRY SSLoadFromBuffer(HWND hWnd, HGLOBAL hBuffer, long lBufferLen);
BOOL DLLENTRY SSLoadFromFile(HWND hWnd, LPCTSTR lpszFileName);
HANDLE DLLENTRY SSLoadPicture(LPCTSTR lpszFileName, short nPictType);
HANDLE DLLENTRY SSLoadPictureBuffer(HGLOBAL hGlobal, long lSize, short nPictType);
HANDLE DLLENTRY SSLoadResPicture(HINSTANCE hInstance, LPCTSTR lpszResName, LPCTSTR lpszResType, short nPictType);
BOOL DLLENTRY SSLoadTabFile(HWND hWnd, LPCTSTR lpszFileName);
BOOL DLLENTRY SSLoadTextFile (HWND hWnd, LPCTSTR pcszFileName, 
							  LPCTSTR pcszCellDelim, LPCTSTR pcszColDelim, 
							  LPCTSTR pcszRowDelim, long lFlags, LPCTSTR pcszLogFile);
void DLLENTRY SSLogUnitsToColWidth(HWND hWnd, int dUnits,
                                   LPDOUBLE lpdfColWidth);
void DLLENTRY SSLogUnitsToRowHeight(HWND hWnd, SS_COORD Row, int dUnits,
                                    LPDOUBLE lpdfRowHeight);
BOOL DLLENTRY SSMoveRange(HWND hWnd, SS_COORD Col, SS_COORD Row,
                          SS_COORD Col2, SS_COORD Row2,
                          SS_COORD ColDest, SS_COORD RowDest);
BOOL DLLENTRY SSMoveSheets(HWND hWnd, short nSheetSrc, short nSheetCnt, short nSheetDest);
BOOL DLLENTRY PaintControl(HWND hWnd, HDC hDC, LPRECT lpRect, BOOL fClientOnly);
#if defined(_WIN64) || defined(_IA64)
BOOL DLLENTRY SSPrint(HWND hWnd, LPCTSTR lpszPrintJobName,
                      LPSS_PRINTFORMAT lpPrintFormat,
                      SS_PRINTFUNC lpfnPrintProc, LONG_PTR lAppData,
                      ABORTPROC lpfnAbortProc);
#else
BOOL DLLENTRY SSPrint(HWND hWnd, LPCTSTR lpszPrintJobName,
                      LPSS_PRINTFORMAT lpPrintFormat,
                      SS_PRINTFUNC lpfnPrintProc, LONG lAppData,
                      ABORTPROC lpfnAbortProc);
#endif
BOOL DLLENTRY SSOwnerPrintDraw(HWND hWnd, HDC hDC, long left, long top, long right, long bottom, short page);
BOOL DLLENTRY SSOwnerPrintPageCount(HWND hWnd, HDC hDC, long left, long top, long right, long bottom, LPSHORT pageCount);
int  DLLENTRY SSQueryCustomName(HWND hWnd, LPCTSTR lpszPrevName,
                                LPTSTR lpszName, int nNameLen);
int  DLLENTRY SSQueryCustomNameLen(HWND hWnd, LPCTSTR lpszPrevName);
BOOL DLLENTRY SSReCalc(HWND hWnd);
BOOL DLLENTRY SSReCalcCell(HWND hWnd, SS_COORD lCol, SS_COORD lRow);
void DLLENTRY SSRemoveCellSpan(HWND hWnd, SS_COORD lCol, SS_COORD lRow);
BOOL DLLENTRY SSRemoveCustomFunction(HWND hWnd, LPCTSTR lpszFuncName);
BOOL DLLENTRY SSReplaceSel(HWND hWnd, LPCTSTR lpText);
void DLLENTRY SSReset(HWND hWnd);
void DLLENTRY SSResetSheet(HWND hWnd, short nSheetIndex);
int  DLLENTRY SSRowHeightToLogUnits(HWND hWnd, SS_COORD Row,
                                    double dfRowHeight);
BOOL DLLENTRY SSSavePicture(HANDLE hPict, HPALETTE hPal, LPCTSTR lpszFileName, short nSaveType);
BOOL DLLENTRY SSSavePictureBuffer(HANDLE hPict, HPALETTE hPal, short nSaveType, HGLOBAL *phGlobal, long *plSize);
BOOL DLLENTRY SSSaveTabFile(HWND hWnd, LPCTSTR lpszFileName);
BOOL DLLENTRY SSSaveTabFileU(HWND hWnd, LPCTSTR lpszFileName);

HGLOBAL DLLENTRY SSSaveToBuffer(HWND hWnd, LPLONG lpBufferLen, BOOL fDataOnly);
BOOL DLLENTRY SSSaveToFile(HWND hWnd, LPCTSTR lpszFileName, BOOL fDataOnly);

SS_COORD DLLENTRY SSSearchCol(HWND hWnd, SS_COORD lCol, SS_COORD lRowStart, SS_COORD lRowEnd,
                     LPCTSTR lpszText, WORD wSearchFlags);
SS_COORD DLLENTRY SSSearchRow(HWND hWnd, SS_COORD lRow, SS_COORD lColStart, SS_COORD lColEnd,
                     LPCTSTR lpszText, WORD wSearchFlags);

#if defined(_WIN64) || defined(_IA64)
long DLLENTRY SSSelModeSendMessage(HWND hWnd, UINT uMsg, long lParam1,
                                   LONG_PTR lParam2, long lParam3);
#else
long DLLENTRY SSSelModeSendMessage(HWND hWnd, UINT uMsg, long lParam1,
                                   long lParam2, long lParam3);
#endif

LPSS_CELLTYPE DLLENTRY SSSetTypeButton(HWND hWnd, LPSS_CELLTYPE lpCellType,
                                       long Style, LPCTSTR Text,
                                       LPCTSTR Picture, short nPictureType,
                                       LPCTSTR PictureDown, short nPictureDownType,
                                       short ButtonType, short nShadowSize,
                                       LPSUPERBTNCOLOR lpColor);
LPSS_CELLTYPE DLLENTRY SSSetTypeCheckBox(HWND hWnd, LPSS_CELLTYPE lpCellType,
                                         long lStyle, LPCTSTR lpText,
                                         LPCTSTR lpPictUp,
                                         WORD wPictUpType,
                                         LPCTSTR lpPictDown,
                                         WORD wPictDownType,
                                         LPCTSTR lpPictFocusUp,
                                         WORD wPictFocusUpType,
                                         LPCTSTR lpPictFocusDown,
                                         WORD wPictFocusDownType,
                                         LPCTSTR lpPictGray,
                                         WORD wPictGrayType,
                                         LPCTSTR lpPictFocusGray,
                                         WORD wPictFocusGrayType);
LPSS_CELLTYPE DLLENTRY SSSetTypeComboBox(HWND hWnd, LPSS_CELLTYPE lpCellType,
                                         long Style, LPCTSTR lpItems);
LPSS_CELLTYPE DLLENTRY SSSetTypeComboBoxEx(HWND hWnd, LPSS_CELLTYPE lpCellType,
                                           long Style, LPCTSTR lpItems,
                                           short dMaxRows, short dMaxEditLen,
                                           short dComboWidth, HWND hWndDropDown, BYTE bAutoSearch);
LPSS_CELLTYPE DLLENTRY SSSetTypeCurrency(HWND hWnd, LPSS_CELLTYPE lpCellType, long lStyle,
                          BYTE Right, double Min, double Max, BYTE fShowSeparator);
LPSS_CELLTYPE DLLENTRY SSSetTypeCurrencyEx(HWND hWnd, LPSS_CELLTYPE lpCellType, long lStyle,
                          BYTE Right, double Min, double Max, BYTE fShowSeparator, BYTE fShowCurrencySymbol,
                          LPTSTR lpszDecimal, LPTSTR lpszSeparator, LPTSTR lpszCurrency,
                          BYTE bLeadingZero, BYTE bNegCurrencyStyle, BYTE bPosCurrencyStyle,
                          BYTE fSpin, BYTE fSpinWrap, double SpinInc);
LPSS_CELLTYPE DLLENTRY SSSetTypeCustom(HWND hWnd, LPSS_CELLTYPE lpCellType,
                                    LPCTSTR lpszName, long Style, LPSS_CT_VALUE ItemData);
LPSS_CELLTYPE DLLENTRY SSSetTypeDate(HWND hWnd, LPSS_CELLTYPE lpCellType,
                                     long Style, LPSS_DATEFORMAT lpFormat,
                                     LPSS_DATE lpMin, LPSS_DATE lpMax);
LPSS_CELLTYPE DLLENTRY SSSetTypeEdit(HWND hWnd, LPSS_CELLTYPE lpCellType,
                                     long Style, int Len,
                                     short ChrSet, short ChrCase);
LPSS_CELLTYPE DLLENTRY SSSetTypeFloat(HWND hWnd, LPSS_CELLTYPE lpCellType,
                                      long Style, short Left, short Right,
                                      double Min, double Max);
LPSS_CELLTYPE DLLENTRY SSSetTypeFloatExt(HWND hWnd, LPSS_CELLTYPE lpCellType,
                                         long Style, short Left, short Right,
                                         double Min, double Max,
                                         LPSS_FLOATFORMAT lpFormat);
LPSS_CELLTYPE DLLENTRY SSSetTypeInteger(HWND hWnd, LPSS_CELLTYPE lpCellType,
                                        long Min, long Max);
LPSS_CELLTYPE DLLENTRY SSSetTypeIntegerExt(HWND hWnd, LPSS_CELLTYPE lpCellType,
                                           long lStyle, long lMin, long lMax,
                                           BOOL fSpinWrap, long lSpinInc);
LPSS_CELLTYPE DLLENTRY SSSetTypeNumber(HWND hWnd, LPSS_CELLTYPE lpCellType, long lStyle,
                          BYTE Right, double Min, double Max, BYTE fShowSeparator);
LPSS_CELLTYPE DLLENTRY SSSetTypeNumberEx(HWND hWnd, LPSS_CELLTYPE lpCellType, long lStyle,
                          BYTE Right, double Min, double Max, BYTE fShowSeparator, LPTSTR lpszDecimal,
                          LPTSTR lpszSeparator, BYTE bLeadingZero, BYTE bNegStyle,
                          BYTE fSpin, BYTE fSpinWrap, double SpinInc);
LPSS_CELLTYPE DLLENTRY SSSetTypeOwnerDraw(HWND hWnd, LPSS_CELLTYPE lpCellType,
                                          long lStyle);
LPSS_CELLTYPE DLLENTRY SSSetTypePercent(HWND hWnd, LPSS_CELLTYPE lpCellType, long lStyle,
                          BYTE Right, double Min, double Max);
LPSS_CELLTYPE DLLENTRY SSSetTypePercentEx(HWND hWnd, LPSS_CELLTYPE lpCellType, long lStyle,
                          BYTE Right, double Min, double Max, LPTSTR lpszDecimal, BYTE bLeadingZero,
                          BYTE bNegStyle, BYTE fSpin, BYTE fSpinWrap, double SpinInc);
LPSS_CELLTYPE DLLENTRY SSSetTypePic(HWND hWnd, LPSS_CELLTYPE lpCellType,
                                    long Style, LPCTSTR Mask);
LPSS_CELLTYPE DLLENTRY SSSetTypePicture(HWND hWnd, LPSS_CELLTYPE lpCellType,
                                        long Style, LPCTSTR PictName);
LPSS_CELLTYPE DLLENTRY SSSetTypePictureHandle(HWND hWnd,
                                              LPSS_CELLTYPE lpCellType,
                                              long Style, HANDLE hPict,
                                              HPALETTE hPal,
                                              BOOL fDeleteHandle);
LPSS_CELLTYPE DLLENTRY SSSetTypeScientific(HWND hWnd, LPSS_CELLTYPE lpCellType, 
										   long Style, BYTE Right, double Min, 
										   double Max, LPTSTR lpszDecimal);
LPSS_CELLTYPE DLLENTRY SSSetTypeStaticText(HWND hWnd, LPSS_CELLTYPE lpCellType,
                                           long TextStyle);
LPSS_CELLTYPE DLLENTRY SSSetTypeTime(HWND hWnd, LPSS_CELLTYPE lpCellType,
                                     long Style, LPSS_TIMEFORMAT lpFormat,
                                     LPSS_TIME lpMin, LPSS_TIME lpMax);
BOOL DLLENTRY SSShowActiveCell(HWND hwnd, short Position);
BOOL DLLENTRY SSShowCell(HWND hwnd, SS_COORD Col, SS_COORD Row,
                         short Position);
BOOL DLLENTRY SSShowCol(HWND hWnd, SS_COORD Col, BOOL fShow);
BOOL DLLENTRY SSShowRow(HWND hWnd, SS_COORD Row, BOOL fShow);
BOOL DLLENTRY SSSort(HWND hWnd, SS_COORD Col, SS_COORD Row,
                     SS_COORD Col2, SS_COORD Row2, LPSS_SORT lpSort);
BOOL DLLENTRY SSSortEx(HWND hWnd, SS_COORD Col, SS_COORD Row, SS_COORD Col2,
                       SS_COORD Row2, WORD wSortBy, LPSS_SORTKEY lpSortKeys,
                       short nSortKeyCnt);
BOOL DLLENTRY SSSwapRange(HWND hWnd, SS_COORD Col, SS_COORD Row,
                          SS_COORD Col2, SS_COORD Row2,
                          SS_COORD ColDest, SS_COORD RowDest);
BOOL DLLENTRY SSValidateFormula(HWND hWnd, LPCTSTR lpszFormula);

void DLLENTRY SSVGetBufferSize(HWND hWnd, LPSS_COORD lpdBufferSize,
                               LPSS_COORD lpdOverlap);
void DLLENTRY SSVSetBufferSize(HWND hWnd, SS_COORD dBufferSize,
                               SS_COORD dOverlap);

SS_COORD DLLENTRY SSVGetMax(HWND hWnd);
SS_COORD DLLENTRY SSVSetMax(HWND hWnd, SS_COORD Max);

SS_COORD DLLENTRY SSVGetPhysBufferSize(HWND hWnd);
SS_COORD DLLENTRY SSVGetPhysBufferTop(HWND hWnd);

LONG DLLENTRY SSVGetStyle(HWND hWnd);
LONG DLLENTRY SSVSetStyle(HWND hWnd, LONG lStyle);

BOOL DLLENTRY SSVRefreshBuffer(HWND hWnd);

BOOL DLLENTRY SSVScrollGetSpecial(HWND hWnd, LPWORD lpwOptions);
BOOL DLLENTRY SSVScrollSetSpecial(HWND hWnd, BOOL fUseSpecialVScroll,
                                  WORD wOptions);

BOOL DLLENTRY SSSaveExcel2007File(HWND hWnd, LPCTSTR lpszFileName, LPCTSTR lpszPassword, short nFlags, LPCTSTR lpszLogFileName);
BOOL DLLENTRY SSOpenExcel2007File(HWND hWnd, LPCTSTR lpszFileName, LPCTSTR lpszPassword, short nSheet, short nExcelSheet, LPCTSTR lpszLogFileName);
short DLLENTRY SSIsExcel2007File(HWND hWnd, LPCTSTR lpszFileName);

BOOL DLLENTRY SSLoadBlockFromBuffer(HWND hWnd,
                                   SS_COORD lCol, SS_COORD lRow,
                                   SS_COORD lCol2, SS_COORD lRow2,
                                   LPBYTE lpBuff, long lBuffLen);

HGLOBAL DLLENTRY SSSaveBlockToBuffer(HWND hWnd, SS_COORD lCol, SS_COORD lRow,
                                          SS_COORD lCol2, SS_COORD lRow2,
                                          LPLONG lplBuffLen, BOOL  bDataOnly);

HGLOBAL DLLENTRY SSSaveBlockToBufferExt(HWND hWnd, SS_COORD lCol, SS_COORD lRow,
                                             SS_COORD lCol2, SS_COORD lRow2,
                                             LPLONG lplBuffLen, BOOL  bDataOnly, BOOL fSaveColInfo, BOOL fSaveRowInfo);

//--------------------------------------------------------------------
//
//  Constants used by spreadsheet functions.
//

// Limit on column and/or row index parameter
#define SS_MAXCOORD            2000000000L

// Column and/or row index parameter
#define SS_ALLCOLS             -1
#define SS_ALLROWS             -1
#define SS_FIRST               -1
#define SS_LAST                -2
#define SS_HEADER              -32767

// SSAddCustomFunction - parameter count
#define SS_CALC_MAXPARAMS      30
#define SS_CALC_VARPARAMS      -1

// SSAddCustomFunctionExt - custom function flags
#define SS_CUSTFUNC_WANTCELLREF  1
#define SS_CUSTFUNC_WANTRANGEREF 2

// SSComboBoxSendMessage - combo box cell messages
#define SS_CBM_ADDSTRING       CB_ADDSTRING
#define SS_CBM_DELETESTRING    CB_DELETESTRING
#define SS_CBM_GETCOUNT        CB_GETCOUNT
#define SS_CBM_GETCURSEL       CB_GETCURSEL
#define SS_CBM_GETLBTEXT       CB_GETLBTEXT
#define SS_CBM_GETLBTEXTLEN    CB_GETLBTEXTLEN
#define SS_CBM_INSERTSTRING    CB_INSERTSTRING
#define SS_CBM_RESETCONTENT    CB_RESETCONTENT
#define SS_CBM_SETCURSEL       CB_SETCURSEL
#define SS_CBM_SETLIST         (WM_USER+100)

// SSExportExcelBookEx, SSExportExcelToFileEx
#define SS_EXCELSAVEFLAG_NONE       0 
#define SS_EXCELSAVEFLAG_NOFORMULAS 1

// SSExportTextFile, SSExportRangeToTextFile, SSExportToXML and SSLoadTextFile
#define SS_EXPORTTEXT_CREATE	 	0x0001
#define SS_EXPORTTEXT_APPEND	 	0x0002
#define SS_EXPORTTEXT_UNFORMATTED	0x0004
#define SS_EXPORTTEXT_COLHEADERS  	0x0008
#define SS_EXPORTTEXT_ROWHEADERS  	0x0010
#define SS_EXPORTTEXT_CHECKBOXFALSE 0x0020

#define SS_EXPORTXML_FORMATTED  	0x0000
#define SS_EXPORTXML_UNFORMATTED  	0x0001

#define SS_LOADTEXT_NOHEADERS		0x0000
#define SS_LOADTEXT_COLHEADERS  	0x0001
#define SS_LOADTEXT_ROWHEADERS  	0x0002
#define SS_LOADTEXT_CLEARDATAONLY 0x0004

// SSGetActionKey, SSSetActionKey - keyboard actions
#define SS_KBA_CLEAR   0
#define SS_KBA_CURRENT 1
#define SS_KBA_POPUP   2
#define SS_KBA_CNT     3

// Keyboard Key Constants
#define SS_KBK_F2	113
#define SS_KBK_F3	114
#define SS_KBK_F4 115

// SSGetActiveCellHighlightStyle, SSSetActiveCellHighlightStyle - property settings
#define SS_ACTIVECELLHIGHLIGHTSTYLE_OFF    0
#define SS_ACTIVECELLHIGHLIGHTSTYLE_NORMAL 1

// SSGetAppearance, SSSetAppearance - property settings
#define SS_APPEARANCE_FLAT         0
#define SS_APPEARANCE_3D           1
#define SS_APPEARANCE_3DWITHBORDER 2
#define SS_APPEARANCE_DEFAULT      3

// SSGetArray, SSSetArray - data type values 
#define SS_ARRAYTYPE_SHORT  0
#define SS_ARRAYTYPE_LONG   1
#define SS_ARRAYTYPE_FLOAT  2
#define SS_ARRAYTYPE_DOUBLE 3
#define SS_ARRAYTYPE_INT    4

// SSGetBackColorStyle, SSSetBackColorStyle - back color styles
#define SS_BACKCOLORSTYLE_OVERGRID         0
#define SS_BACKCOLORSTYLE_UNDERGRID        1
#define SS_BACKCOLORSTYLE_OVERHORZGRIDONLY 2
#define SS_BACKCOLORSTYLE_OVERVERTGRIDONLY 3

// SSGetBool, SSSetBool - Boolean settings
#define SSB_PROTECT            0
#define SSB_RESTRICTROWS       1
#define SSB_RESTRICTCOLS       2
#define SSB_SHOWCOLHEADERS     3
#define SSB_SHOWROWHEADERS     4
#define SSB_ALLOWUSERRESIZE    5
#define SSB_ALLOWUSERSELBLOCK  6
#define SSB_AUTOCALC           7
#define SSB_REDRAW             8
#define SSB_GRIDLINES          9
#define SSB_HORZSCROLLBAR      10
#define SSB_VERTSCROLLBAR      11
#define SSB_ALLOWUSERFORMULAS  12
#define SSB_EDITMODEPERMANENT  13
#define SSB_AUTOSIZE           14
#define SSB_SCROLLBAREXTMODE   15
#define SSB_SCROLLBARSHOWMAX   16
#define SSB_SCROLLBARMAXALIGN  17
#define SSB_PROCESSTAB         18
#define SSB_ARROWSEXITEDITMODE 19
#define SSB_MOVEACTIVEONFOCUS  20
#define SSB_RETAINSELBLOCK     21
#define SSB_EDITMODEREPLACE    22
#define SSB_NOBORDER           23
#define SSB_NOBEEP             24
#define SSB_AUTOCLIPBOARD      25
#define SSB_ALLOWMULTIBLOCKS   26
#define SSB_ALLOWCELLOVERFLOW  27
#define SSB_VIRTUALMODE        28
#define SSB_ALLOWDRAGDROP      29
#define SSB_CHANGEMADE         30
#define SSB_FORMULASYNC        31
#define SSB_HSCROLLBARTRACK    32
#define SSB_VSCROLLBARTRACK    33
#define SSB_ALLOWEDITOVERFLOW  34
#define SSB_ALLOWCOLMOVE       35
#define SSB_ALLOWROWMOVE       36

// SSGetBorder, SSSetBorder, SSSetBorderRange - border types
#define SS_BORDERTYPE_NONE         0x00
#define SS_BORDERTYPE_LEFT         0x01
#define SS_BORDERTYPE_RIGHT        0x02
#define SS_BORDERTYPE_TOP          0x04
#define SS_BORDERTYPE_BOTTOM       0x08
#define SS_BORDERTYPE_OUTLINE      0x10

// SSGetBorder, SSSetBorder, SSSetBorderRange - border styles
#define SS_BORDERSTYLE_NONE              0
#define SS_BORDERSTYLE_SOLID             1
#define SS_BORDERSTYLE_DASH              2
#define SS_BORDERSTYLE_DOT               3
#define SS_BORDERSTYLE_DASHDOT           4
#define SS_BORDERSTYLE_DASHDOTDOT        5
#define SS_BORDERSTYLE_BLANK             6
#define SS_BORDERSTYLE_FINE_SOLID        11
#define SS_BORDERSTYLE_FINE_DASH         12
#define SS_BORDERSTYLE_FINE_DOT          13
#define SS_BORDERSTYLE_FINE_DASH_DOT     14
#define SS_BORDERSTYLE_FINE_DASH_DOT_DOT 15

// SSGetButtonDrawMode, SSSetButtonDrawMode - button draw modes
#define SS_BDM_ALWAYS          0x00
#define SS_BDM_CURRENTCELL     0x01
#define SS_BDM_CURRENTCOL      0x02
#define SS_BDM_CURRENTROW      0x04
#define SS_BDM_ALWAYSBUTTON    0x08
#define SS_BDM_ALWAYSCOMBO     0x10

// SSGetCellNoteIndicator, SSSetCellNoteIndicator - settings
#define SS_CELLNOTEINDICATOR_SHOWANDFIREEVENT 0
#define SS_CELLNOTEINDICATOR_SHOWANDDONOTFIREEVENT 1
#define SS_CELLNOTEINDICATOR_DONOTSHOWANDFIREEVENT 2
#define SS_CELLNOTEINDICATOR_DONOTSHOWANDDONOTFIREEVENT 3

// SSGetCellSpan - return values
#define SS_SPAN_NO     0
#define SS_SPAN_YES    1
#define SS_SPAN_ANCHOR 2

// SSGetClipboardOptions, SSSetClipboardOptions - values
#define SS_CLIP_NOHEADERS           0
#define SS_CLIP_COPYROWHEADERS      1
#define SS_CLIP_PASTEROWHEADERS     2
#define SS_CLIP_COPYCOLHEADERS      4
#define SS_CLIP_PASTECOLHEADERS     8
#define SS_CLIP_COPYPASTEALLHEADERS 15

// SSGetColHeaderDisplay, SSSetColHeaderDisplay, SSGetRowHeaderDisplay, SSSetRowHeaderDisplay
// - header options 
#define SS_HEADERDISPLAY_BLANK   0
#define SS_HEADERDISPLAY_NUMBERS 1
#define SS_HEADERDISPLAY_LETTERS 2

// SSGetColMerge, SSSetColMerge, SSGetRowMerge, SSSetRowMerge
// - merge values
#define SS_MERGE_NONE       0
#define SS_MERGE_ALWAYS     1
#define SS_MERGE_RESTRICTED 2
#define SS_MERGE_CNT        3

// SSGetColUserSortIndicator, SSSetColUserSortIndicator - property settings
#define SS_COLUSERSORTINDICATOR_NONE          0
#define SS_COLUSERSORTINDICATOR_ASCENDING     1
#define SS_COLUSERSORTINDICATOR_DESCENDING    2
#define SS_COLUSERSORTINDICATOR_DISABLED      3

// SSGetColor, SSSetColor, SSSetColorRange - colors
#ifndef RGBCOLOR_DEFAULT
#if defined(SS_OCX) || defined(SS_VB)
#define RGBCOLOR_DEFAULT     (0x80000000L | COLOR_INACTIVEBORDER)
#else
#define RGBCOLOR_DEFAULT   (COLORREF)-1L
#endif
#endif
#define RGBCOLOR_WHITE     RGB(255, 255, 255)
#define RGBCOLOR_BLUE      RGB(0, 0, 255)
#define RGBCOLOR_RED       RGB(255, 0, 0)
#define RGBCOLOR_PINK      RGB(255, 0, 255)
#define RGBCOLOR_GREEN     RGB(0, 255, 0)
#define RGBCOLOR_CYAN      RGB(0, 255, 255)
#define RGBCOLOR_YELLOW    RGB(255, 255, 0)
#define RGBCOLOR_BLACK     RGB(0, 0, 0)
#define RGBCOLOR_DARKGRAY  RGB(128, 128, 128)
#define RGBCOLOR_DARKBLUE  RGB(0, 0, 128)
#define RGBCOLOR_DARKRED   RGB(128, 0, 0)
#define RGBCOLOR_DARKPINK  RGB(255, 0, 128)
#define RGBCOLOR_DARKGREEN RGB(0, 128, 0)
#define RGBCOLOR_DARKCYAN  RGB(0, 128, 128)
#define RGBCOLOR_BROWN     RGB(128, 128, 64)
#define RGBCOLOR_PALEGRAY  RGB(192, 192, 192)

#define SPREAD_COLOR_NONE  0x8000000B
#define SPREAD_COLOR_IGNORE 0x8000FFFF

// SSGetCursor, SSSetCursor - cursor types
#define SS_CURSORTYPE_DEFAULT      0x0001
#define SS_CURSORTYPE_COLRESIZE    0x0002
#define SS_CURSORTYPE_ROWRESIZE    0x0004
#define SS_CURSORTYPE_BUTTON       0x0008
#define SS_CURSORTYPE_GRAYAREA     0x0010
#define SS_CURSORTYPE_LOCKEDCELL   0x0020
#define SS_CURSORTYPE_COLHEADER    0x0040
#define SS_CURSORTYPE_ROWHEADER    0x0080
#define SS_CURSORTYPE_DRAGDROPAREA 0x0100
#define SS_CURSORTYPE_DRAGDROP     0x0200

// SSGetCursor, SSSetCursor - cursors
#define SS_CURSOR_DEFAULT        0
#define SS_CURSOR_ARROW          ((HCURSOR)-1)
#define SS_CURSOR_DEFCOLRESIZE   ((HCURSOR)-2)
#define SS_CURSOR_DEFROWRESIZE   ((HCURSOR)-3)

// SSGetEditEnterAction, SSSetEditEnterAction - Edit ENTER key actions
#define SS_ENTERACTION_NONE    0
#define SS_ENTERACTION_UP      1
#define SS_ENTERACTION_DOWN    2
#define SS_ENTERACTION_LEFT    3
#define SS_ENTERACTION_RIGHT   4
#define SS_ENTERACTION_NEXT    5
#define SS_ENTERACTION_PREV    6
#define SS_ENTERACTION_SAME    7
#define SS_ENTERACTION_NEXTROW 8

// SSGetGridType, SSSetGridType - grid types
#define SS_GRID_HORIZONTAL     0x01
#define SS_GRID_VERTICAL       0x02
#define SS_GRID_SOLID          0x04

// SSGetOperationMode, SSSetOperationMode - operation modes 
#define SS_OPMODE_NORMAL       0
#define SS_OPMODE_READONLY     1
#define SS_OPMODE_ROWMODE      2
#define SS_OPMODE_SINGLESEL    3
#define SS_OPMODE_MULTISEL     4
#define SS_OPMODE_EXTSEL       5

// SSGetRefStyle, SSSetRefStyle - reference styles
#define SS_REFSTYLE_DEFAULT 0
#define SS_REFSTYLE_A1      1
#define SS_REFSTYLE_R1C1    2

// SSGetSelBlockOptions, SSSetSelBlockOptionsSelect - block options
#define SS_SELBLOCK_COLS           0x01
#define SS_SELBLOCK_ROWS           0x02
#define SS_SELBLOCK_BLOCKS         0x04
#define SS_SELBLOCK_ALL            0x08

// SSGetShowScrollTips, SSSetShowScrollTips - display values
#define SS_SHOWSCROLLTIPS_OFF    0
#define SS_SHOWSCROLLTIPS_VERT   1
#define SS_SHOWSCROLLTIPS_HORZ   2
#define SS_SHOWSCROLLTIPS_BOTH   3

// SSGetTabStripPolicy & SSSetTabStripPolicy - display values 
#define SS_TABSTRIPPOLICY_ASNEEDED 0
#define SS_TABSTRIPPOLICY_ALWAYS   1
#define SS_TABSTRIPPOLICY_NEVER    2

// SSGetTextTips & SSSetTextTip - display values 
#define SS_TT_STATUS_OFF               0
#define SS_TT_STATUS_FIXED             1
#define SS_TT_STATUS_FLOATING          2
#define SS_TT_STATUS_FIXEDFOCUSONLY    3
#define SS_TT_STATUS_FLOATINGFOCUSONLY 4

// SSGetUnitType, SSSetUnitType - unit types
#define SS_UNITTYPE_NORMAL     0
#define SS_UNITTYPE_DEFAULT    1
#define SS_UNITTYPE_TWIPS      2

// SSGetUserColAction, SSSetUserColAction - property settings
#define SS_USERCOLACTION_DEFAULT         0
#define SS_USERCOLACTION_SORT            1
#define SS_USERCOLACTION_SORTNOINDICATOR 2
#define SS_USERCOLACTION_MINVAL          0
#define SS_USERCOLACTION_AUTOSIZE        3
#define SS_USERCOLACTION_MAXVAL          3

// SSGetUserResize, SSSetUserResize - user resize options
#define SS_USERRESIZE_COL      0x01
#define SS_USERRESIZE_ROW      0x02

// SSGetUserResizeCol, SSSetUserResizeCol, SSGetUserResizeRow, SSSetUserResizeRow
// - user resize options
#define SS_RESIZE_DEFAULT      0
#define SS_RESIZE_ON           1
#define SS_RESIZE_OFF          2

// SSGetVisible - values 
#define SS_VISIBLE_PARTIAL     0
#define SS_VISIBLE_ALL         1

// SSSearchCol. SSSearchRow - search values
#define SS_SEARCHFLAGS_NONE             0
#define SS_SEARCHFLAGS_GREATEROREQUAL   1
#define SS_SEARCHFLAGS_PARTIALMATCH     2
#define SS_SEARCHFLAGS_VALUE            4
#define SS_SEARCHFLAGS_CASESENSITIVE    8
#define SS_SEARCHFLAGS_SORTEDASCENDING  16
#define SS_SEARCHFLAGS_SORTEDDESCENDING 32

// SSSelModeSendMessage - Select mode messages 
#define SS_SELMODE_CLEARSEL     1
#define SS_SELMODE_GETCURSEL    2
#define SS_SELMODE_GETSEL       3
#define SS_SELMODE_GETSELCOUNT  4
#define SS_SELMODE_GETSELITEMS  5
#define SS_SELMODE_SELITEMRANGE 6
#define SS_SELMODE_SETCURSEL    7
#define SS_SELMODE_SETSEL       8
#define SS_SELMODE_GETSELITEM   9

// SSSetTypeButton - button styles
#define BS_CENTER             0x00000300L

// SSSetTypeButton - button styles
#define SBS_DEFPUSHBUTTON        0x0001L
#define SBS_TEXTTOP_PICTBOT      0x0002L
#define SBS_TEXTBOT_PICTTOP      0x0004L
#define SBS_TEXTLEFT_PICTRIGHT   0x0008L
#define SBS_TEXTRIGHT_PICTLEFT   0x0010L
#define SBS_BTNFRAMEIN           0x0020L
#define SBS_BTNFRAMEOUT          0x0040L
#define SBS_PICTFRAMEIN          0x0080L
#define SBS_PICTFRAMEOUT         0x0100L
#define SBS_TEXTFRAMEIN          0x0200L
#define SBS_TEXTFRAMEOUT         0x0400L
#define SBS_NOINVERTFRAME        0x0800L
#define SBS_TEXTICON             0x1000L
#define SBS_TEXTBITMAP           0x2000L
#define SBS_TEXTTEXT             0x4000L
#define SBS_NOPOINTERFOCUS       0x8000L

#define SUPERBTN_UP              0
#define SUPERBTN_DOWN            1

// SSSetTypeButton - button types
#define SUPERBTN_TYPE_NORMAL     0
#define SUPERBTN_TYPE_2STATE     1

// SSSetTypeButton - button picture types
#define SUPERBTN_PICT_NONE       0
#define SUPERBTN_PICT_ICON       BT_ICON
#define SUPERBTN_PICT_BITMAP     BT_BITMAP
#define SUPERBTN_PICT_HANDLE     BT_HANDLE

// SSSetTypeCheckBox - check box picture types
#define BT_NONE                  0x0000
#define BT_ICON                  0x0001
#define BT_BITMAP                0x0002
#define BT_HANDLE                0x0004
#define BT_FREEHANDLE            0x0008

// SSSetTypeComboBox - combo box style
#define SS_CB_DROPDOWN         0x02

// SSSetTypeComboBoxEx - AutoSearch styles
#define SS_COMBOBOX_AUTOSEARCH_NONE              0
#define SS_COMBOBOX_AUTOSEARCH_SINGLECHAR        1
#define SS_COMBOBOX_AUTOSEARCH_MULTIPLECHAR      2
#define SS_COMBOBOX_AUTOSEARCH_SINGLECHARGREATER 3

// SSSetTypeComboBoxEx - combo box width settings
#define SS_COMBOWIDTH_CELLWIDTH    0
#define SS_COMBOWIDTH_AUTORIGHT    1
#define SS_COMBOWIDTH_AUTOLEFT    -1

// SSSetTypeCurrency - format styles
#define  SS_CURR_NEGSTYLE_INTL  0
#define  SS_CURR_NEGSTYLE_1		1
#define  SS_CURR_NEGSTYLE_2		2
#define  SS_CURR_NEGSTYLE_3		3
#define  SS_CURR_NEGSTYLE_4		4
#define  SS_CURR_NEGSTYLE_5		5
#define  SS_CURR_NEGSTYLE_6		6
#define  SS_CURR_NEGSTYLE_7		7
#define  SS_CURR_NEGSTYLE_8		8
#define  SS_CURR_NEGSTYLE_9		9
#define  SS_CURR_NEGSTYLE_10	10
#define  SS_CURR_NEGSTYLE_11	11
#define  SS_CURR_NEGSTYLE_12	12
#define  SS_CURR_NEGSTYLE_13	13
#define  SS_CURR_NEGSTYLE_14	14
#define  SS_CURR_NEGSTYLE_15	15
#define  SS_CURR_NEGSTYLE_16	16
#define  SS_CURR_POSSTYLE_INTL	0
#define  SS_CURR_POSSTYLE_1		1
#define  SS_CURR_POSSTYLE_2		2
#define  SS_CURR_POSSTYLE_3		3
#define  SS_CURR_POSSTYLE_4		4

// SSSetTypeDate - date styles
#define  DS_SPIN        0x0040L
#define  DS_DEFCENTURY  0x0004L
#define  DS_CENTURY     0x1000L
#define  DS_DDMONYY     0x2000L
#define  DS_DDMMYY      0x4000L
#define  DS_MMDDYY      0x6000L
#define  DS_YYMMDD      0x8000L

// SSSetTypeEdit - edit styles
#define ES_LEFTALIGN   0x0200L

// SSSetTypeEdit - character sets
#define SS_CHRSET_CHR            0
#define SS_CHRSET_ALPHA          1
#define SS_CHRSET_ALPHANUM       2
#define SS_CHRSET_NUM            3
// Character sets supported in the JPN version
#define SS_CHRSET_KNJONLY        4
#define SS_CHRSET_KNJONLY_IME    5
#define SS_CHRSET_ALLCHR_IME     6

// SSSetTypeEdit - character cases
#define SS_CASE_LCASE          0
#define SS_CASE_NOCASE         1
#define SS_CASE_UCASE          2

// SSSetTypeFloat - float styles
#define FS_MONEY       0x1000L
#define FS_SEPARATOR   0x2000L

// SSSetTypeInteger - integer styles
#define IS_SPIN        0x1000L
#define IS_SPINWRAP    0x2000L

// SSSetTypeNumber - format styles
#define	 SS_NUMBER_NEGSTYLE_INTL 0
#define	 SS_NUMBER_NEGSTYLE_1	1
#define	 SS_NUMBER_NEGSTYLE_2	2
#define	 SS_NUMBER_NEGSTYLE_3	3
#define	 SS_NUMBER_NEGSTYLE_4	4
#define	 SS_NUMBER_NEGSTYLE_5	5

// SSSetTypePercent - format styles
#define  SS_PERCENT_NEGSTYLE_INTL 0
#define  SS_PERCENT_NEGSTYLE_1	1
#define  SS_PERCENT_NEGSTYLE_2	2
#define  SS_PERCENT_NEGSTYLE_3	3
#define  SS_PERCENT_NEGSTYLE_4	4
#define  SS_PERCENT_NEGSTYLE_5	5
#define  SS_PERCENT_NEGSTYLE_6	6
#define  SS_PERCENT_NEGSTYLE_7	7
#define  SS_PERCENT_NEGSTYLE_8	8

// SSSetTypePicture - picture styles
#define VPS_NORMAL            0x0000L
#define VPS_CENTER            0x00220000L
#define VPS_STRETCH           0x0002L
#define VPS_MAINTAINSCALE     0x0004L
#define VPS_RESOURCE          0x0010L
#define VPS_BMP               0x0020L
#define VPS_PCX               0x0040L
#define VPS_GIF               0x0080L
#define VPS_JPEG			  0x0200L
#define VPS_HANDLE            0x0800L
#define VPS_ICON              0x1000L

// SSLoadPicture, SSLoadResPicture, SSSavePicture PictType constants
#define SS_PICTURE_TYPE_UNKNOWN   0 // unknown loads any of the below formats
#define SS_PICTURE_TYPE_BMP       1
#define SS_PICTURE_TYPE_GIF       2
#define SS_PICTURE_TYPE_JPEG      3
#define SS_PICTURE_TYPE_PNG       4
#define SS_PICTURE_TYPE_ICO       5
#define SS_PICTURE_TYPE_TIFF      6
#define SS_PICTURE_TYPE_TGA       7
#define SS_PICTURE_TYPE_PCX       8
#define SS_PICTURE_TYPE_WBMP      9
#define SS_PICTURE_TYPE_WMF       10
#define SS_PICTURE_TYPE_JP2       11
#define SS_PICTURE_TYPE_JPC       12

// SSSetTypeStatic - text styles
#define SS_TEXT_LEFT           0x001
#define SS_TEXT_RIGHT          0x002
#define SS_TEXT_CENTER         0x004
#define SS_TEXT_VCENTER        0x008
#define SS_TEXT_TOP            0x010
#define SS_TEXT_BOTTOM         0x020
#define SS_TEXT_WORDWRAP       0x040
#define SS_TEXT_SHADOW         0x080
#define SS_TEXT_PREFIX         0x100
#define SS_TEXT_SHADOWIN       0x200

// SSSetTypeTime - time styles
#define  TS_12HOUR      0x1000L
#define  TS_24HOUR      0x2000L
#define  TS_SECONDS     0x4000L
#define  TS_SPIN        0x8000L

// == Constants used for creating cell type == 

// Alignment styles for creating cell types
#define SSS_ALIGN_TOP     0x00010000L
#define SSS_ALIGN_VCENTER 0x00020000L
#define SSS_ALIGN_BOTTOM  0x00040000L
#define SSS_ALIGN_LEFT    0x00100000L
#define SSS_ALIGN_CENTER  0x00200000L
#define SSS_ALIGN_RIGHT   0x00400000L

// Ellipsis constant
#define SSS_ELLIPSES   0x00800000L

// Numeric cell type contants
#define SSS_NEGRED 0x10000000L

#define SS_LEADINGZERO_INTL	0
#define SS_LEADINGZERO_NO		1
#define SS_LEADINGZERO_YES		2

// Text Orientation
#define SSS_TEXTORIENT_VERT_LTR 0x01000000L
#define SSS_TEXTORIENT_DOWN     0x02000000L
#define SSS_TEXTORIENT_UP       0x03000000L
#define SSS_TEXTORIENT_INVERT   0x04000000L
#define SSS_TEXTORIENT_VERT_RTL 0x05000000L
#define SSS_TEXTORIENT_MASK     0x0f000000L

// SSShowActiveCell and SSShowCell - positions 
#define SS_SHOW_TOPLEFT        0
#define SS_SHOW_TOPCENTER      1
#define SS_SHOW_TOPRIGHT       2
#define SS_SHOW_CENTERLEFT     3
#define SS_SHOW_CENTER         4
#define SS_SHOW_CENTERRIGHT    5
#define SS_SHOW_BOTTOMLEFT     6
#define SS_SHOW_BOTTOMCENTER   7
#define SS_SHOW_BOTTOMRIGHT    8
#define SS_SHOW_NEAREST        9

// SSVGetStyle, SSVSetStyle - virtual mode styles
#define SSV_SCROLLBARSHOWBUFFER 0x01
#define SSV_NOROWNUMBERS        0x02

// SSVScrollGetSpecial, SSVScrollSetSpecial - virtual scroll bar options
#define SS_VSCROLL_NOHOMEEND  0x01
#define SS_VSCROLL_NOPAGEUPDN 0x02
#define SS_VSCROLL_NOUPDN     0x04

// SSSaveExcel2007File - Flag constants
#define SS_SAVEEXCEL2007FILEFLAG_NONE = 0
#define SS_SAVEEXCEL2007FILEFLAG_NOFORMULAS = 1
#define SS_SAVEEXCEL2007FILEFLAG_DATAONLY = 32

// SSIsExcel2007File constants
#define SS_ISEXCEL2007FILE_NO 0
#define SS_ISEXCEL2007FILE_YES 1
#define SS_ISEXCEL2007FILE_ENCRYPTED 2

//--------------------------------------------------------------------
// Preview Control
//--------------------------------------------------------------------

#define SPVN_PAGECHANGE             1
#define SPVN_ZOOM                   2

#define SPV_MARGINTYPE_SCALED       0
#define SPV_MARGINTYPE_ACTUAL       1

#define SPV_VIEWTYPE_WHOLEPAGE      0
#define SPV_VIEWTYPE_NORMALSIZE     1
#define SPV_VIEWTYPE_PERCENTAGE     2
#define SPV_VIEWTYPE_PAGEWIDTH      3
#define SPV_VIEWTYPE_PAGEHEIGHT     4
#define SPV_VIEWTYPE_MULTIPLEPAGES  5

#define SPV_ZOOMSTATE_INDETERMINATE 0
#define SPV_ZOOMSTATE_IN            1
#define SPV_ZOOMSTATE_OUT           2
#define SPV_ZOOMSTATE_SWITCH        3

#define SPV_SCROLLBAR_SHOW          0
#define SPV_SCROLLBAR_AUTO          1
#define SPV_SCROLLBAR_HIDE          2

#ifndef FP_API
#ifdef  WIN32
#define FP_API   __declspec(dllexport) __stdcall
#else
#define FP_API   FAR PASCAL _export
#endif
#endif

int FP_API SpvGetAllowUserZoom(HWND hWnd, BOOL FAR *lpValue);
int FP_API SpvSetAllowUserZoom(HWND hWnd, BOOL value);

int FP_API SpvGetGrayAreaColor(HWND hWnd, COLORREF FAR *lpValue);
int FP_API SpvSetGrayAreaColor(HWND hWnd, COLORREF value);

int FP_API SpvGetGrayAreaMarginH(HWND hWnd, long FAR *lpValue);
int FP_API SpvSetGrayAreaMarginH(HWND hWnd, long value);

int FP_API SpvGetGrayAreaMarginType(HWND hWnd, short FAR *lpValue);
int FP_API SpvSetGrayAreaMarginType(HWND hWnd, short value);

int FP_API SpvGetGrayAreaMarginV(HWND hWnd, long FAR *lpValue);
int FP_API SpvSetGrayAreaMarginV(HWND hWnd, long value);

int FP_API SpvGethWndSpread(HWND hPreview, HWND FAR *lphSpread);
int FP_API SpvSethWndSpread(HWND hPreview, HWND hSpread);

int FP_API SpvGetPageBorderColor(HWND hWnd, COLORREF FAR *lpValue);
int FP_API SpvSetPageBorderColor(HWND hWnd, COLORREF value);

int FP_API SpvGetPageBorderWidth(HWND hWnd, short FAR *lpValue);
int FP_API SpvSetPageBorderWidth(HWND hWnd, short value);

int FP_API SpvGetPageCurrent(HWND hWnd, long FAR *lpValue);
int FP_API SpvSetPageCurrent(HWND hWnd, long value);

int FP_API SpvGetPageGutterH(HWND hWnd, long FAR *lpValue);
int FP_API SpvSetPageGutterH(HWND hWnd, long value);

int FP_API SpvGetPageGutterV(HWND hWnd, long FAR *lpValue);
int FP_API SpvSetPageGutterV(HWND hWnd, long value);

int FP_API SpvGetPageMultiCntH(HWND hWnd, short FAR *lpValue);
int FP_API SpvSetPageMultiCntH(HWND hWnd, short value);

int FP_API SpvGetPageMultiCntV(HWND hWnd, short FAR *lpValue);
int FP_API SpvSetPageMultiCntV(HWND hWnd, short value);

int FP_API SpvGetPagePercentageActual(HWND hWnd, short FAR *lpValue);

int FP_API SpvGetPageShadowColor(HWND hWnd, COLORREF FAR *lpValue);
int FP_API SpvSetPageShadowColor(HWND hWnd, COLORREF value);

int FP_API SpvGetPageShadowWidth(HWND hWnd, short FAR *lpValue);
int FP_API SpvSetPageShadowWidth(HWND hWnd, short value);

int FP_API SpvGetPagesPerScreen(HWND hWnd, short FAR *lpValue);

int FP_API SpvGetPageViewPercentage(HWND hWnd, short FAR *lpValue);
int FP_API SpvSetPageViewPercentage(HWND hWnd, short value);

int FP_API SpvGetPageViewType(HWND hWnd, short FAR *lpValue);
int FP_API SpvSetPageViewType(HWND hWnd, short value);

int FP_API SpvGetScrollBarH(HWND hWnd, short FAR *lpValue);
int FP_API SpvSetScrollBarH(HWND hWnd, short value);

int FP_API SpvGetScrollBarV(HWND hWnd, short FAR *lpValue);
int FP_API SpvSetScrollBarV(HWND hWnd, short value);

int FP_API SpvGetScrollIncH(HWND hWnd, long FAR *lpValue);
int FP_API SpvSetScrollIncH(HWND hWnd, long value);

int FP_API SpvGetScrollIncV(HWND hWnd, long FAR *lpValue);
int FP_API SpvSetScrollIncV(HWND hWnd, long value);

int FP_API SpvGetZoomState(HWND hWnd, short FAR *lpValue);
int FP_API SpvSetZoomState(HWND hWnd, short value);

//--------------------------------------------------------------------

#ifndef RC_INVOKED
#pragma pack()
#endif  /* RC_INVOKED */

#ifdef __cplusplus
}
#endif

#endif


