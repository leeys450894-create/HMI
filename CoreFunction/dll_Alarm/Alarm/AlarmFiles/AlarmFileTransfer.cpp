#include "pch.h"
#include "AlarmFileTransfer.h"

#include "AlarmXLSFile.h"
#include "..\..\..\common\BinaryHelper.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CAlarmFileTransfer::CAlarmFileTransfer()
{
}

CAlarmFileTransfer::~CAlarmFileTransfer()
{
}



bool CAlarmFileTransfer::LoadFile(CString strFileName)
{
	m_alarmTable.RemoveAll();

	CBinaryHelper bin;
	bin.SetFilePath(strFileName);
	
	if (bin.FileLoad() == true)
	{
		for (UINT i = 0; i < ALARM_MAX_COUNT; ++i)
		{
			ALARM_DATA data;

			CString strCATEGORY(_T("ALARM_XLS"));
			CString strKEY;
			CString strValue;
			strKEY.Format(_T("ALARM_DATA_%04d"), i);
			bin.Link(false, strCATEGORY, strKEY + _T("ID"), &data.id, (UINT)i);
			bin.Link(false, strCATEGORY, strKEY + _T("CODE"), &data.code, (UINT)0);


			bin.Link(false, strCATEGORY, strKEY + _T("LEVEL"), &strValue, (CString)_T("H"));

			if (strValue.CompareNoCase(_T("H")) == 0)
				data.level = ALARM_LEVLE_HIGH;
			else if (strValue.CompareNoCase(_T("L")) == 0)
				data.level = ALARM_LEVLE_LOW;
			else  if (strValue.CompareNoCase(_T("O")) == 0)
				data.level = ALARM_LEVLE_MSG;
			else
				data.level = ALARM_LEVLE_HIGH;

			bin.Link(false, strCATEGORY, strKEY + _T("MODULE"), &strValue, (CString)_T("UNKNOWN"));	_tcscpy_s(data.module, _countof(data.module), strValue);
			bin.Link(false, strCATEGORY, strKEY + _T("CMD"), &strValue, (CString)_T("UNKNOWN"));		_tcscpy_s(data.cmd, _countof(data.cmd), strValue);
			bin.Link(false, strCATEGORY, strKEY + _T("TEXT"), &strValue, (CString)_T("UNKNOWN"));	_tcscpy_s(data.text, _countof(data.text), strValue);
			bin.Link(false, strCATEGORY, strKEY + _T("DESCRIPTION"), &strValue, (CString)_T(""));	 _tcscpy_s(data.Description, _countof(data.Description), strValue);

			/*bin.Link(false, strCATEGORY, strKEY + _T("LEVEL"), &data.level, (CString)_T("UNKNOWN"));
			bin.Link(false, strCATEGORY, strKEY + _T("MODULE"), &data.module, (CString)_T("UNKNOWN"));
			bin.Link(false, strCATEGORY, strKEY + _T("CMD"), &data.cmd, (CString)_T("UNKNOWN"));
			bin.Link(false, strCATEGORY, strKEY + _T("TEXT"), &data.text, (CString)_T("UNKNOWN"));
			bin.Link(false, strCATEGORY, strKEY + _T("DESCRIPTION"), &data.Description, (CString)_T(""));*/
			bin.Link(false, strCATEGORY, strKEY + _T("RETRY"), &data.nRetry, (UINT)0);
			bin.Link(false, strCATEGORY, strKEY + _T("TYPE"), &data.nType, (UINT)0);

			m_alarmTable.SetAt(i, data);
		}

		return true;
	}

	return false;
}

bool CAlarmFileTransfer::SaveFile(CString strFileName)
{
	CBinaryHelper bin;
	bin.SetFilePath(strFileName);

	for (UINT i = 0; i < ALARM_MAX_COUNT; ++i)
	{
		ALARM_DATA data;

		if (m_alarmTable.Lookup(i, data) == TRUE)
		{
			CString strCATEGORY(_T("ALARM_XLS"));
			CString strKEY;
			CString strValue;
			strKEY.Format(_T("ALARM_DATA_%04d"), i);
			bin.Link(true, strCATEGORY, strKEY + _T("ID"), &data.id, (UINT)i);
			bin.Link(true, strCATEGORY, strKEY + _T("CODE"), &data.code, (UINT)0);

			strValue.Format(_T("%s"), data.level);
			bin.Link(true, strCATEGORY, strKEY + _T("LEVEL"), &strValue, (CString)_T("H"));
			strValue.Format(_T("%s"), data.module);
			bin.Link(true, strCATEGORY, strKEY + _T("MODULE"), &strValue, (CString)_T("UNKNOWN"));
			strValue.Format(_T("%s"), data.cmd);
			bin.Link(true, strCATEGORY, strKEY + _T("CMD"), &strValue, (CString)_T("UNKNOWN"));
			strValue.Format(_T("%s"), data.text);
			bin.Link(true, strCATEGORY, strKEY + _T("TEXT"), &strValue, (CString)_T("UNKNOWN"));
			strValue.Format(_T("%s"), data.Description);
			bin.Link(true, strCATEGORY, strKEY + _T("DESCRIPTION"), &strValue, (CString)_T(""));

			//bin.Link(true, strCATEGORY, strKEY + _T("LEVEL"), &data.level, (CString)_T("UNKNOWN"));
			//bin.Link(true, strCATEGORY, strKEY + _T("MODULE"), &data.module, (CString)_T("UNKNOWN"));
			//bin.Link(true, strCATEGORY, strKEY + _T("CMD"), &data.cmd, (CString)_T("UNKNOWN"));
			//bin.Link(true, strCATEGORY, strKEY + _T("TEXT"), &data.text, (CString)_T("UNKNOWN"));
			//bin.Link(true, strCATEGORY, strKEY + _T("DESCRIPTION"), &data.Description, (CString)_T(""));
			bin.Link(true, strCATEGORY, strKEY + _T("RETRY"), &data.nRetry, (UINT)0);
			bin.Link(true, strCATEGORY, strKEY + _T("TYPE"), &data.nType, (UINT)0);
		}
	}

	return bin.FileSave();
}
