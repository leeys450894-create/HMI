#pragma once

#include <vector>

struct BITFIELD
{
	unsigned char bit_31 : 1; unsigned char bit_30 : 1; unsigned char bit_29 : 1; unsigned char bit_28 : 1;

	unsigned char bit_27 : 1; unsigned char bit_26 : 1; unsigned char bit_25 : 1; unsigned char bit_24 : 1;

	unsigned char bit_23 : 1; unsigned char bit_22 : 1; unsigned char bit_21 : 1; unsigned char bit_20 : 1;

	unsigned char bit_19 : 1; unsigned char bit_18 : 1; unsigned char bit_17 : 1; unsigned char bit_16 : 1;

	unsigned char bit_15 : 1; unsigned char bit_14 : 1; unsigned char bit_13 : 1; unsigned char bit_12 : 1;

	unsigned char bit_11 : 1; unsigned char bit_10 : 1; unsigned char bit_9 : 1; unsigned char bit_8 : 1;

	unsigned char bit_7 : 1; unsigned char bit_6 : 1; unsigned char bit_5 : 1; unsigned char bit_4 : 1;

	unsigned char bit_3 : 1; unsigned char bit_2 : 1; unsigned char bit_1 : 1; unsigned char bit_0 : 1;
};

struct BYTEFIELD
{
	unsigned int m_Bottom1Byte : 16;			//실수부

	unsigned int m_Top1Byte : 16;				//정수부
};

union FLOAT_FIELD
{
	TCHAR m_char[2];

	float m_float;

	long lfloat;

	struct BITFIELD m_bit;

	struct BYTEFIELD m_16bit;
};

#define MC_PROTOCOL_SUCCESS				 0	// 0 : 성공, 그외 error
#define MC_PROTOCOL_NOT_INITIALIZE		-1

#define PLC_CMD_HEADER_LENGTH			22

#define PLC_CHAR_STX					0x02
#define PLC_CHAR_ENQ					0x05
#define PLC_CHAR_ACK					0x06
#define PLC_CHAR_NAK					0x15
#define PLC_CHAR_EOT					0x04
#define PLC_CHAR_ETX					0x03

#define PLC_CMD_RS232_SINGLE_READ					"F90000FF0004010001"
#define PLC_CMD_RS232_SINGLE_WRITE					"F90000FF0014010001"

#define PLC_CMD_ETHERNET_SINGLE_READ				"500000FF03FF000018009004010001"
#define PLC_CMD_ETHERNET_WORD_READ_D                "500000FF03FF000018009004010000"

#define PLC_CMD_ETHERNET_SINGLE_WRITE				"500000FF03FF000019009014010001"
#define PLC_CMD_ETHERNET_BYTE_WRITE					"500000FF03FF000020009014010001"
#define PLC_CMD_ETHERNET_DOUBLE_READ				"500000FF03FF0000180090040300000001"
#define PLC_CMD_ETHERNET_DOUBLE_WRITE				"500000FF03FF0000200090140200000001"
#define PLC_CMD_ETHERNET_WORD_READ					"500000FF03FF0000180090040300000100"
#define PLC_CMD_ETHERNET_WORD_WRITE					"500000FF03FF0000200090140200000100"

#define PLC_CMD_WORD_READ							"F90000FF0004010000"
#define PLC_CMD_WORD_WRITE							"F90000FF0014010000"

#define PLC_CMD_SUCCESS								"D00000FF03FF0000040000"

enum enumIO_DEVICE
{
	enumIO_DEVICE_READ_M = 0,		// M-				: address M
	enumIO_DEVICE_WRITE_M,			// Write
	enumIO_DEVICE_F,				// F-				: address F

	enumIO_DEVICE_PROBE_CARD,		// probe card		: address D
	enumIO_DEVICE_PACC,				// pacc				: address M

	enumIO_DEVICE_D,				// D-				: address D
	enumIO_DEVICE_GLASS_SENSOR,		// glass sensor		: address D
	
	enumADDRESS_DEVICE_Count,
};

//P
//PACC Interface
const UINT dfPACCADDRESS_TOTAL_COUNT	= 100;			//M3900 ~ M3999		PACC Interface
const UINT dfPACCADDRESS_HALF_COUNT		= 50;
const UINT dfPACCADDRESS_RD_START		= 3900;
const UINT dfPACCADDRESS_WT_START		= dfPACCADDRESS_RD_START + dfPACCADDRESS_HALF_COUNT;
const UINT dfPACCADDRESS_LIMIT_COUNT	= dfPACCADDRESS_RD_START + dfPACCADDRESS_TOTAL_COUNT;

//M
//IO
const UINT dfMADDRESS_TOTAL_COUNT		= 1000;			//M4000 ~ M4999		IO
const UINT dfMADDRESS_HALF_COUNT		= 500;
const UINT dfMADDRESS_RD_START			= 4000;
const UINT dfMADDRESS_WT_START			= dfMADDRESS_RD_START + dfMADDRESS_HALF_COUNT;
const UINT dfMADDRESS_LIMIT_COUNT		= dfMADDRESS_RD_START + dfMADDRESS_TOTAL_COUNT;

//F
//F Alarm
const UINT dfFADDRESS_TOTAL_COUNT		= 2000;			//F0 ~ F1999		F Alarm
const UINT dfFADDRESS_RD_START			= 0;
const UINT dfFADDRESS_LIMIT_COUNT		= dfFADDRESS_TOTAL_COUNT;

//G
//Glass Sensor Interface
const UINT dfGSADDRESS_TOTAL_COUNT		= 99;			//D1501 ~ D1599		Glass Sensor Interface
const UINT dfGSADDRESS_HALF_COUNT		= 49;
const UINT dfGSADDRESS_RD_START			= 1550;
const UINT dfGSADDRESS_WT_START			= dfGSADDRESS_RD_START - dfGSADDRESS_HALF_COUNT;
const UINT dfGSADDRESS_WT_SENSOR_VALUE	= 1510;
const UINT dfGSADDRESS_LIMIT_COUNT		= dfGSADDRESS_WT_START + dfGSADDRESS_TOTAL_COUNT;
const UINT dfGSADDRESS_SENSOR_COUNT		= 10;

//C
//Probe Card Name, Buffer and Slot
const UINT dfPROBEADDRESS_TOTAL_COUNT	= 399;			//D2100 ~ D2499		Probe Card Name, Buffer and Slot
const UINT dfPROBEADDRESS_RD_START		= 2100;
const UINT dfPROBEADDRESS_LIMIT_COUNT	= dfPROBEADDRESS_RD_START + dfPROBEADDRESS_TOTAL_COUNT;

//D
//FDC
const UINT dfDADDRESS_TOTAL_COUNT		= 500;			//D10000 ~ D11999	FDC
const UINT dfDADDRESS_HALF_COUNT		= 1500;
const UINT dfDADDRESS_RD_START			= 11500;
const UINT dfDADDRESS_WT_START			= dfDADDRESS_RD_START - dfDADDRESS_HALF_COUNT;
const UINT dfDADDRESS_LIMIT_COUNT		= dfDADDRESS_RD_START + ( dfDADDRESS_TOTAL_COUNT * 2);



struct SAMapString
{
	bool	bUsable;
	bool	bBitFlag;
	UINT	nAddress;
	CString	strMapNumber;
	CString	strDescription;

	SAMapString()
	{
		bUsable = false;
		bBitFlag = false;
		nAddress = 9999;
		strMapNumber = _T("   ");
		strDescription = _T("   ");
	}
};

struct SADDRESSMAPDATA
{
	bool bEnable;	
	std::vector<SAMapString> vecData;

	SADDRESSMAPDATA()
	{
		Clear();
	}

	void Clear()
	{
		bEnable = false;
		vecData.clear();
	}
};
