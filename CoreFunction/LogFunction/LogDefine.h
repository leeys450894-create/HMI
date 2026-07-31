#pragma once

#include <afxmt.h>
using namespace std;

#define WM_GUILOG (WM_USER + 9000)

enum enumLogLevel
{
	ERRO = 0,
	WARN,
	SYST,
	INSP,
	OPER,
	SEND,
	RECV,
};
