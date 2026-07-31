#include "pch.h"
#include "XStringHelper.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CXStingHelper::CXStingHelper(void)
{
	
}

CXStingHelper::~CXStingHelper(void)
{
}

CString CXStingHelper::STRTRANSLATOR(UINT nID, ...)
{
	CString	fmt;
	fmt.LoadString (nID);

	if (fmt.Find (_T('\n')) == 0) {
		fmt = fmt.Mid (1);
	}

	CString	string;
	va_list	argptr;
	va_start (argptr, nID);
	string.FormatV (fmt, argptr);
	va_end (argptr);
	return string;
}