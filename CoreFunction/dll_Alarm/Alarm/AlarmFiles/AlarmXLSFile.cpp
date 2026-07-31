#include "pch.h"
#include "AlarmXLSFile.h"

#include "..\common\XLS\XLEzAutomation.h"
#include "..\common\BinaryHelper.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CAlarmXLSFile::CAlarmXLSFile()
{
}


CAlarmXLSFile::~CAlarmXLSFile()
{
}

bool CAlarmXLSFile::XLStoDAT(CString strFileName)
{
	m_alarmTable.RemoveAll();

	if (LoadXLSFile(strFileName) == TRUE)
	{
		CBinaryHelper bin;

		CString strValuel;
		for (UINT i = 0; i < ALARM_MAX_COUNT; ++i)
		{
			ALARM_DATA data;

			if (m_alarmTable.Lookup(i, data) == TRUE)
			{
				CString strCATEGORY(_T("ALARM_XLS"));
				CString strKEY;
				CString strValue;
				strKEY.Format(_T("DATA_%04d_"), i);
				bin.Link(true, strCATEGORY, strKEY + _T("ID"), &data.id, (UINT)i);
				bin.Link(true, strCATEGORY, strKEY + _T("CODE"), &data.code, (UINT)0);

				if (data.level == ALARM_LEVLE_HIGH)
					strValuel.Format(_T("H"));
				else if (data.level == ALARM_LEVLE_LOW)
					strValuel.Format(_T("L"));
				else  if (data.level == ALARM_LEVLE_MSG)
					strValuel.Format(_T("O"));
				else
					strValuel.Format(_T("H"));

				strValue.Format(_T("%s"), strValuel);
				bin.Link(true, strCATEGORY, strKEY + _T("LEVEL"), &strValue, (CString)_T("H"));	
				strValue.Format(_T("%s"), data.module);
				bin.Link(true, strCATEGORY, strKEY + _T("MODULE"), &strValue, (CString)_T("UNKNOWN"));	
				strValue.Format(_T("%s"), data.cmd);
				bin.Link(true, strCATEGORY, strKEY + _T("CMD"), &strValue, (CString)_T("UNKNOWN"));		
				strValue.Format(_T("%s"), data.text);
				bin.Link(true, strCATEGORY, strKEY + _T("TEXT"), &strValue, (CString)_T("UNKNOWN"));	
				strValue.Format(_T("%s"), data.Description);
				bin.Link(true, strCATEGORY, strKEY + _T("DESCRIPTION"), &strValue, (CString)_T(""));
				bin.Link(true, strCATEGORY, strKEY + _T("RETRY"), &data.nRetry, (UINT)0);
				bin.Link(true, strCATEGORY, strKEY + _T("TYPE"), &data.nType, (UINT)0);
			}
		}

		CString strBinFile(strFileName);
		strBinFile.Replace(_T(".xls"), _T(".dat"));
		bin.SetFilePath(strBinFile);
		return bin.FileSave();
	}
    
	return false;
}

BOOL CAlarmXLSFile::LoadFile(CString strFileName)
{
	m_alarmTable.RemoveAll();

	CBinaryHelper bin;
	bin.SetFilePath(strFileName);
	
	if (bin.FileLoad() == true)
	{
		for (UINT i = 0; i < ALARM_MAX_COUNT; ++i)
		{
			ALARM_DATA data;
			UINT nUse = 0;
			CString strCATEGORY(_T("ALARM_XLS"));
			CString strKEY;
			CString strValue;
			strKEY.Format(_T("DATA_%04d_"), i);

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

			/*bin.Link(false, strCATEGORY, strKEY + _T("MODULE"), &data.module, (CString)_T("UNKNOWN"));
			bin.Link(false, strCATEGORY, strKEY + _T("CMD"), &data.cmd, (CString)_T("UNKNOWN"));
			bin.Link(false, strCATEGORY, strKEY + _T("TEXT"), &data.text, (CString)_T("UNKNOWN"));
			bin.Link(false, strCATEGORY, strKEY + _T("DESCRIPTION"), &data.Description, (CString)_T(""));*/
			bin.Link(false, strCATEGORY, strKEY + _T("RETRY"), &data.nRetry, (UINT)0);
			bin.Link(false, strCATEGORY, strKEY + _T("TYPE"), &data.nType, (UINT)0);

			m_alarmTable.SetAt(i, data);
		}

		return TRUE;
	}

	return FALSE;
}

bool CAlarmXLSFile::SaveFile(CString strFileName)
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
			strKEY.Format(_T("DATA_%04d_"), i);
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

			/*bin.Link(true, strCATEGORY, strKEY + _T("LEVEL"), &data.level, (CString)_T("UNKNOWN"));
			bin.Link(true, strCATEGORY, strKEY + _T("MODULE"), &data.module, (CString)_T("UNKNOWN"));
			bin.Link(true, strCATEGORY, strKEY + _T("CMD"), &data.cmd, (CString)_T("UNKNOWN"));
			bin.Link(true, strCATEGORY, strKEY + _T("TEXT"), &data.text, (CString)_T("UNKNOWN"));
			bin.Link(true, strCATEGORY, strKEY + _T("DESCRIPTION"), &data.Description, (CString)_T(""));*/
			bin.Link(true, strCATEGORY, strKEY + _T("RETRY"), &data.nRetry, (UINT)0);
			bin.Link(true, strCATEGORY, strKEY + _T("TYPE"), &data.nType, (UINT)0);
		}
	}

	return bin.FileSave();
}

BOOL CAlarmXLSFile::Lookup(UINT id, ALARM_DATA &data)
{
    return m_alarmTable.Lookup(id, data);
}





bool CAlarmXLSFile::LoadXLSFile(CString strFileName)
{
	CFile file;
    if(file.Open(strFileName, CFile::shareDenyNone) == FALSE)
        return false;

    CArchive ar(&file, CArchive::load);

    CXLEzAutomation m_ListExcel(FALSE);
    if(m_ListExcel.OpenExcelFile(strFileName) == FALSE)
    {
        file.Close();
        return false;
    }
	
	m_alarmTable.RemoveAll();

    int imax_row = 0;
    for( int row = 1; row < ALARM_MAX_COUNT; row++)
    {
        CString str = m_ListExcel.GetCellValue(1, row);

        if(str == _T("[FILE END]"))
        {
            imax_row = row;
            break;
        }
    }

	BOOL bUse = false;
    for( int row = 1; row < imax_row; row++)
    {
        CString str = m_ListExcel.GetCellValue(1, row);

        if( 0 <= str.CompareNoCase(_T("Alarm")))
            continue;

        if(str.IsEmpty() == false)
        {
			bUse = _ttoi(m_ListExcel.GetCellValue(13, row));

			if (bUse > 0)
			{
				ALARM_DATA alarmData;
				// ID
				alarmData.id = _ttoi(m_ListExcel.GetCellValue(1, row));

				// CODE
				alarmData.code = _ttoi(m_ListExcel.GetCellValue(2, row));

				// LEVEL
				CString strText;
				strText = m_ListExcel.GetCellValue(3, row).GetAt(0);
				if (strText.CompareNoCase(_T("H")) == 0)
					alarmData.level = ALARM_LEVLE_HIGH;
				else if (strText.CompareNoCase(_T("L")) == 0)
					alarmData.level = ALARM_LEVLE_LOW;
				else  if (strText.CompareNoCase(_T("O")) == 0)
					alarmData.level = ALARM_LEVLE_MSG;
				else
					alarmData.level = ALARM_LEVLE_HIGH;

				//  alarmData.level = m_ListExcel.GetCellValue(3, row).GetAt(0);

				  // module

				strText = m_ListExcel.GetCellValue(4, row);
				strText.TrimLeft();
				strText.TrimRight();
				strText.Replace(_T("?"), _T(""));
				//   alarmData.module = strText;
				_tcscpy_s(alarmData.module, _countof(alarmData.module), strText);

				//Alarm Command
				strText = m_ListExcel.GetCellValue(5, row);
				strText.TrimLeft();
				strText.TrimRight();
				strText.Replace(_T("?"), _T(""));
				//     alarmData.cmd = strText;
				_tcscpy_s(alarmData.cmd, _countof(alarmData.cmd), strText);

				//Alarm Text
				strText = m_ListExcel.GetCellValue(6, row);
				strText.TrimLeft();
				strText.TrimRight();
				strText.Replace(_T("?"), _T(""));
				//   alarmData.text = strText;
				_tcscpy_s(alarmData.text, _countof(alarmData.text), strText);

				//Alarm Description
				strText = m_ListExcel.GetCellValue(7, row);
				strText.TrimLeft();
				strText.TrimRight();
				strText.Replace(_T("?"), _T(""));
				//   alarmData.Description = strText;
				_tcscpy_s(alarmData.Description, _countof(alarmData.Description), strText);

				alarmData.nRetry = _ttoi(m_ListExcel.GetCellValue(8, row));

				// Alarm Type
				alarmData.nType = _ttoi(m_ListExcel.GetCellValue(9, row));


				// Alarm language
				CString strUse = m_ListExcel.GetCellValue(10, row);

				if (strUse.CompareNoCase(_T("E")) == 0)
				{
					//Alarm Text
					strText = m_ListExcel.GetCellValue(11, row);
					strText.TrimLeft();
					strText.TrimRight();
					strText.Replace(_T("?"), _T(""));
					//   alarmData.text = strText;
					_tcscpy_s(alarmData.text, _countof(alarmData.text), strText);

					//Alarm Description
					strText = m_ListExcel.GetCellValue(12, row);
					strText.TrimLeft();
					strText.TrimRight();
					strText.Replace(_T("?"), _T(""));
					//    alarmData.Description = strText;
					_tcscpy_s(alarmData.Description, _countof(alarmData.Description), strText);
				}

				//Alarm Num
				m_alarmTable.SetAt(alarmData.id, alarmData);
			}
        }
    }

    if(m_ListExcel.ReleaseExcel() == FALSE)
        AfxMessageBox(_T("not released."));

    file.Close();

    return true;
}