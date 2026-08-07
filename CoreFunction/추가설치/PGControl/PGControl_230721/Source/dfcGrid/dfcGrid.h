#pragma once

#include "../dfcGrid/GridCtrl.h"
#include "../dfcGrid/GridCellCombo.h"
#include "../dfcGrid/GridCellNumeric.h"
#include "../dfcGrid/GridCellCheck.h"
#include "../dfcGrid/GridCellButton.h"
#include "../dfcGrid/GridCtrlEx.h"

#ifndef	DFC_GRID_LIB
	#ifdef	_DEBUG
		#ifdef _M_X64
			#pragma comment (lib, "../../x64/Debug/dfcGrid/dfcGridd.lib")
		#else
			#pragma comment (lib, "../../x86/Debug/dfcGrid/dfcGridd.lib")
		#endif 
	#else
		#ifdef _M_X64
			#pragma comment (lib, "../../x64/Release/dfcGrid/dfcGrid.lib")
		#else
			#pragma comment (lib, "../../x86/Release/dfcGrid/dfcGrid.lib")
		#endif 
	#endif
#endif


