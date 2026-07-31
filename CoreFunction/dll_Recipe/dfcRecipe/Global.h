#pragma once

#ifndef	IDS_FONTNAME
#define	IDS_FONTNAME	_T("¸¼Àº °íµñ")
#endif

#define COLOR_FRAME_BRIGHT		RGB (255, 255, 255)
#define COLOR_FRAME				RGB (104, 104, 104)
#define COLOR_FRAME_DARK		RGB (64, 64, 64)
#define COLOR_YELLOW_BK			RGB (255, 235, 156)
#define COLOR_YELLOW_FONT		RGB (156, 101, 0)
#define COLOR_GREEN_BK			RGB (198, 239, 206)
#define COLOR_GREEN_FONT		RGB (0, 97, 0)
#define COLOR_RED_BK			RGB (255, 199, 206)
#define COLOR_RED_FONT			RGB (156, 0, 0)
#define COLOR_DONE				RGB (  0, 255,   0)
#define COLOR_PROC				RGB (255, 204,   0)
#define COLOR_IDLE				RGB (255, 255, 255)
#define COLOR_GOOD				RGB (0, 128, 0)
#define COLOR_NG				RGB (128, 0, 0)
#define COLOR_FAIL				RGB (255, 0, 0)

#define COLOR_BLACK				RGB(0,0,0)
#define COLOR_WHITE				RGB(255,255,255)
#define COLOR_GRAY_BRIGHT		RGB(234,234,234)
#define COLOR_GRAY				RGB(192,192,192)
#define COLOR_GRAY_DARK			RGB(128,128,128)
#define COLOR_RED				RGB(255,0,0)
#define COLOR_RED_DARK			RGB(128,0,0)
#define COLOR_GREEN				RGB(0,255,0)
#define COLOR_GREEN_DARK		RGB(0,51,0)
#define COLOR_BLUE				RGB(0,0,255)
#define COLOR_YELLOW			RGB(255,255,0)
#define COLOR_SKYBLUE_BRIGHT	RGB (152,192,235)
#define COLOR_SKYBLUE			RGB (0,130,200)
#define COLOR_SKYBLUE_DARK		RGB (49,85,153)

CString	dfcStr (LPCTSTR fmt, ...);
CString	dfcStr (int nVal);
CString	dfcStr (double fVal);
CString	dfcGetFontName ();
CRect	dfcClientRect (CWnd *pWnd, CWnd *pWndMap = 0);
CString	dfcIniFolder ();
CString	dfcIniFile ();
BOOL	dfcCreateDirectory (LPCTSTR szPath);
void	dfcArryParsing (CStringArray *strArry, CString str);

void		WriteString (CStdioFile *File, CString string);
void		WriteInt (CStdioFile *File, int nVal);
void		WriteDouble (CStdioFile *File, double fVal);
CString		ReadString (CStdioFile *File);
int			ReadInt (CStdioFile *File);
double		ReadDouble (CStdioFile *File);