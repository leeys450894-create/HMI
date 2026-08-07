#pragma once

#define COMM_QUEUE_SIZE 2048

#include "../dfcComm/preCommDefine.h"

#include "../dfcComm/SerialComm.h"
#include "../dfcComm/SyncSerialComm.h"
#include "../dfcComm/ServerSocket.h"
#include "../dfcComm/ClientSocket.h"

#include "../dfcComm/PGComm.h"
#include "../dfcComm/ElpPGComm.h"
#include "../dfcComm/QualityComm.h"

#include "../dfcComm/PlcAddressMDefine.h"
#include "../dfcComm/PlcAddressFDefine.h"
#include "../dfcComm/PlcCommMgr.h"

#include "../dfcComm/SPOLightComm.h"

#include "../dfcComm/CIMComm.h"

#include "../dfcComm/QualityPGComm.h"
#include "../dfcComm/SendElpPGComm.h"
#include "../dfcComm/ColorPGServerComm.h"
#include "../dfcComm/MainPGClientComm.h"

#ifndef	DFC_COMM_LIB
	#ifdef	_DEBUG
		#ifdef _M_X64
			#pragma comment(lib, "../../x64/Debug/dfcComm/dfcCommd.lib")
		#else
			#pragma comment(lib, "../../x86/Debug/dfcComm/dfcCommd.lib")
		#endif 
	#else
		#ifdef _M_X64
			#pragma comment(lib, "../../x64/Release/dfcComm/dfcComm.lib")
		#else
			#pragma comment(lib, "../../x86/Release/dfcComm/dfcComm.lib")
		#endif 
	#endif
#endif
