#include "pch.h"
#include "AlarmCSVFile.h"

#include "..\common\XLS\XLEzAutomation.h"
#include "..\common\BinaryHelper.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CAlarmCSVFile::CAlarmCSVFile()
{
}

CAlarmCSVFile::~CAlarmCSVFile()
{
}

bool CAlarmCSVFile::CSVtoDAT(CString strFileName)
{
	m_alarmMessage.RemoveAll();

	if (LoadCSVFile(strFileName) == TRUE)
	{
		CBinaryHelper bin;

		for (UINT i = 0; i < ALARM_MAX_COUNT; ++i)
		{
			ALARM_MESSAGE data;

			if (m_alarmMessage.Lookup(i, data) == TRUE)
			{
				CString strCATEGORY(_T("ALARM_CSV"));
				CString strKEY;
				strKEY.Format(_T("MESSAGE_%04d_"), i);
				bin.Link(true, strCATEGORY, strKEY + _T("CODE"), &data.code, (UINT)0);
				bin.Link(true, strCATEGORY, strKEY + _T("TYPE"), &data.nType, (UINT)0);
				bin.Link(true, strCATEGORY, strKEY + _T("USE"), &data.use, (CString)_T("E"));
				bin.Link(true, strCATEGORY, strKEY + _T("TEXT1"), &data.text1, (CString)_T("Unknown Alarm"));
				bin.Link(true, strCATEGORY, strKEY + _T("TEXT2"), &data.text2, (CString)_T("Unknown Alarm"));
			}
		}

		CString strBinFile(strFileName);
		strBinFile.Replace(_T(".csv"), _T(".dat"));
		bin.SetFilePath(strBinFile);
		return bin.FileSave();
	}
    
	return false;
}

BOOL CAlarmCSVFile::LoadFile(CString strFileName)
{
	m_alarmMessage.RemoveAll();

	CBinaryHelper bin;
	bin.SetFilePath(strFileName);
	
	if (bin.FileLoad() == true)
	{
		for (UINT i = 0; i < ALARM_MAX_COUNT; ++i)
		{
			ALARM_MESSAGE data;

			if (m_alarmMessage.Lookup(i, data) == TRUE)
			{
				CString strCATEGORY(_T("ALARM_CSV"));
				CString strKEY;
				strKEY.Format(_T("MESSAGE_%04d_"), i);
				bin.Link(false, strCATEGORY, strKEY + _T("CODE"), &data.code, (UINT)0);
				bin.Link(false, strCATEGORY, strKEY + _T("TYPE"), &data.nType, (UINT)0);
				bin.Link(false, strCATEGORY, strKEY + _T("USE"), &data.use, (CString)_T("E"));
				bin.Link(false, strCATEGORY, strKEY + _T("TEXT1"), &data.text1, (CString)_T("Unknown Alarm"));
				bin.Link(false, strCATEGORY, strKEY + _T("TEXT2"), &data.text2, (CString)_T("Unknown Alarm"));
			}

			m_alarmMessage.SetAt(i, data);
		}

		return TRUE;
	}

	return FALSE;
}

bool CAlarmCSVFile::SaveFile(CString strFileName)
{
	CBinaryHelper bin;
	bin.SetFilePath(strFileName);

	for (UINT i = 0; i < ALARM_MAX_COUNT; ++i)
	{
		ALARM_MESSAGE data;

		if (m_alarmMessage.Lookup(i, data) == TRUE)
		{
			CString strCATEGORY(_T("ALARM_CSV"));
			CString strKEY;
			strKEY.Format(_T("MESSAGE_%04d_"), i);
			bin.Link(true, strCATEGORY, strKEY + _T("CODE"), &data.code, (UINT)0);
			bin.Link(true, strCATEGORY, strKEY + _T("TYPE"), &data.nType, (UINT)0);
			bin.Link(true, strCATEGORY, strKEY + _T("USE"), &data.use, (CString)_T("E"));
			bin.Link(true, strCATEGORY, strKEY + _T("TEXT1"), &data.text1, (CString)_T("Unknown Alarm"));
			bin.Link(true, strCATEGORY, strKEY + _T("TEXT2"), &data.text2, (CString)_T("Unknown Alarm"));
		}
	}

	return bin.FileSave();
}

BOOL CAlarmCSVFile::Lookup(UINT id, ALARM_MESSAGE &data)
{
    return m_alarmMessage.Lookup(id, data);
}







bool CAlarmCSVFile::LoadCSVFile(CString strFileName)
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

    int imax_row = 0;
    for( int row = 1; row < ALARM_MAX_COUNT; row++)
    {
        CString str = m_ListExcel.GetCellValue(1, row);

        if(str.IsEmpty())
        {
            imax_row = row - 1;
            break;
        }
    }

    for( int row = 1; row < imax_row; row++)
    {
        CString str = m_ListExcel.GetCellValue(1, row);

        if( 0 <= str.CompareNoCase(_T("Alarm")))
            continue;

        if(str.IsEmpty() == false)
        {
            ALARM_MESSAGE alarmMessage;

            // ID
            alarmMessage.code = _ttoi(m_ListExcel.GetCellValue(1, row));

            // CODE
            alarmMessage.nType = _ttoi(m_ListExcel.GetCellValue(2, row));

            // Alarm language
            alarmMessage.use = m_ListExcel.GetCellValue(3, row);

            // Alarm text1
            CString strText;
            strText = m_ListExcel.GetCellValue(4, row);
            strText.TrimLeft();
            strText.TrimRight();
            strText.Replace(_T("?"), _T(""));
            alarmMessage.text1 = strText;

            // Alarm text2
            strText = m_ListExcel.GetCellValue(5, row);
            strText.TrimLeft();
            strText.TrimRight();
            strText.Replace(_T("?"), _T(""));
            alarmMessage.text2 = strText;

            //Alarm Num
            m_alarmMessage.SetAt(alarmMessage.code, alarmMessage);
        }
    }

    if(m_ListExcel.ReleaseExcel() == FALSE)
        AfxMessageBox(_T("not released."));

    file.Close();

    return true;
}
