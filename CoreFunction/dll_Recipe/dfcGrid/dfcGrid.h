#pragma once

//#ifndef	GRID_EXPORTS
//    #ifdef	_DEBUG
//		#ifdef _M_X64
//			#pragma comment(lib, "../../Debug_x64/dfcGrid/dfcGridd.lib")
//		#else
//			#pragma comment(lib, "../../Debug/dfcGrid/dfcGridd.lib")
//		#endif
//	#else
//		#ifdef _M_X64
//			#pragma comment(lib, "../../Release_x64/dfcGrid/dfcGrid.lib")
//		#else
//			#pragma comment(lib, "../../Release/dfcGrid/dfcGrid.lib")
//		#endif 
//	#endif
//#endif

#ifndef	GRID_EXPORTS
    #ifdef	_DEBUG
		#ifdef _M_X64
			#pragma comment(lib, "../Library/dfcGrid64d.lib")
		#else
			#pragma comment(lib, "../Library/dfcGridd.lib")
		#endif
	#else
		#ifdef _M_X64
			#pragma comment(lib, "../Library/dfcGrid64.lib")
		#else
			#pragma comment(lib, "../Library/dfcGrid.lib")
		#endif 
	#endif
#endif

#include "GridCtrl.h"
#include "GridCellCombo.h"
#include "GridCellNumeric.h"
#include "GridCellCheck.h"
#include "GridCellButton.h"

