// ECSTAAgentLogin.cpp : implementation file
//

#include "stdafx.h"
#include "Phone.h"
#include "ECSTAAgentLoginLogout.h"
#include "afxdialogex.h"


// ECSTAAgentLogin dialog

IMPLEMENT_DYNAMIC(ECSTAAgentLoginLogout, CDialog)

CStdString MakeAlphaNumeric(const TCHAR* szData, const bool bAllowSpaces /* = true */, const TCHAR* szAdditionalAllowedCharacters /* = _T("") */)
{
	// Return TRUE, if szData contains ONLY a-z A-Z and 0-9
	// other characters lead to FALSE (e.g. spaces, äöü !"? etc.)
	CStdString strReturn;

	if (szData != NULL)
	{
		CStdString strData = szData;
		CStdString strAdditionalCharacters = szAdditionalAllowedCharacters;
		CStdString::iterator iter = strData.begin();
		CStdString::iterator iterAdditionalCharacters;
		while (iter != strData.end())
		{
			TCHAR tc = (*iter);
			if (tc >= _T('0') && tc <= _T('9') ||
				tc >= _T('a') && tc <= _T('z') ||
				tc >= _T('A') && tc <= _T('Z') ||
				tc == _T(' ') && bAllowSpaces)
			{
				strReturn += tc;
			}
			else
			{
				// Spezielles Zeichen - ist dieses im String für die zusätzliche erlaubten Zeichen ?
				if (!strAdditionalCharacters.IsEmpty())
				{
					// String mit zusätzlichen Zeichen nach dem Zeichen durchsuchen
					iterAdditionalCharacters = strAdditionalCharacters.begin();
					while (iterAdditionalCharacters != strAdditionalCharacters.end())
					{
						if (*iterAdditionalCharacters == tc)
						{
							strReturn += tc;
							break;
						}
						iterAdditionalCharacters++;
					}
				}
			}
			iter++;
		}

	}
	return strReturn;
}



ECSTAAgentLoginLogout::ECSTAAgentLoginLogout(const DWORD dwRequiredLoginParameters, CTapiLine* pLine, CWnd* pParent /*=nullptr*/)
	: CDialog(ECSTAAgentLoginLogout::IDD, pParent)
{
	m_dwRequiredLoginParameters = dwRequiredLoginParameters;

	CStdString strLineName = MakeAlphaNumeric(pLine->GetLineName(), false, L"*+-()[].:,;");
	m_strRegistryHive = L"Software\\ESTOS\\EPhone\\";
	m_strRegistryHive += strLineName;
}

ECSTAAgentLoginLogout::~ECSTAAgentLoginLogout()
{
	if (m_pstrAgentID)
	{
		delete m_pstrAgentID;
		m_pstrAgentID = nullptr;
	}
	if (m_pstrGroupID)
	{
		delete m_pstrGroupID;
		m_pstrGroupID = nullptr;
	}
	if (m_pstrDeviceID)
	{
		delete m_pstrDeviceID;
		m_pstrDeviceID = nullptr;
	}
	if (m_pstrPassword)
	{
		delete m_pstrPassword;
		m_pstrPassword = nullptr;
	}

}

void ECSTAAgentLoginLogout::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BOOL ECSTAAgentLoginLogout::OnInitDialog()
{
	__super::OnInitDialog();

	if (m_dwRequiredLoginParameters & ECSTA150_AGENTLOGIN_AGENTID)
	{
		m_pstrAgentID = new CString;
		if(m_pstrAgentID)
		{
			*m_pstrAgentID = GetRegSTRING(L"AgentID");
			m_Options.push_back(AgentLoginParam(m_pstrAgentID, L"Agent ID:"));
		}
	}
	if (m_dwRequiredLoginParameters & ECSTA150_AGENTLOGIN_GROUPID)
	{
		m_pstrGroupID = new CString;
		if (m_pstrGroupID)
		{
			*m_pstrGroupID = GetRegSTRING(L"GroupID");
			m_Options.push_back(AgentLoginParam(m_pstrGroupID, L"Group ID:"));
		}
	}
	if (m_dwRequiredLoginParameters & ECSTA150_AGENTLOGIN_DEVICEID)
	{
		m_pstrDeviceID = new CString;
		if (m_pstrDeviceID)
		{
			*m_pstrDeviceID = GetRegSTRING(L"DeviceID");
			m_Options.push_back(AgentLoginParam(m_pstrDeviceID, L"Device ID:"));
		}
	}
	if (m_dwRequiredLoginParameters & ECSTA150_AGENTLOGIN_PASSWORD || m_dwRequiredLoginParameters & ECSTA150_AGENTLOGOUT_PASSWORD)
	{
		m_pstrPassword = new CString;
		if (m_pstrPassword)
		{
			*m_pstrPassword = GetRegSTRING(L"Password");
			m_Options.push_back(AgentLoginParam(m_pstrPassword, L"Password:"));
		}
	}

	if(m_dwRequiredLoginParameters & ECSTA150_AGENTLOGOUT_MASK)
	{
		SetWindowText(L"Agent logout");
		SetDlgItemText(IDC_STATIC_HEADER, L"Agent logout parameter");
		SetDlgItemText(IDOK, L"Logout");
	}

	auto iter = m_Options.begin();
	for(int iPos = 0; iPos < 4; iPos++)
	{
		int iStatic = 0;
		int iEdit = 0;
		switch (iPos)
		{
		case 0:
			iStatic = IDC_OPTION_1;
			iEdit = IDC_TXT_OPTION_1;
			break;
		case 1:
			iStatic = IDC_OPTION_2;
			iEdit = IDC_TXT_OPTION_2;
			break;
		case 2:
			iStatic = IDC_OPTION_3;
			iEdit = IDC_TXT_OPTION_3;
			break;
		case 3:
			iStatic = IDC_OPTION_4;
			iEdit = IDC_TXT_OPTION_4;
			break;
		default:
			ASSERT(FALSE);
			break;
		}

		if (iter != m_Options.end())
		{
			iter->m_nDialogEditID = iEdit;
			SetDlgItemText(iEdit, *iter->m_pstrValue);
			SetDlgItemText(iStatic, iter->m_strCaption);
			iter++;
		}
		else
		{
			GetDlgItem(iStatic)->ShowWindow(SW_HIDE);
			GetDlgItem(iEdit)->ShowWindow(SW_HIDE);
		}
	}

	return TRUE;
}

CString ECSTAAgentLoginLogout::GetRegSTRING(const TCHAR* szName)
{
	HKEY hAtspKey;
	DWORD dwDataType;
	TCHAR szSearchString[1024] = { 0 };
	DWORD dwTmpLength = sizeof(szSearchString);

	//Holt ein STRING aus der Registry
	if (RegOpenKeyEx(HKEY_CURRENT_USER, m_strRegistryHive.c_str(), 0, KEY_READ, &hAtspKey) == ERROR_SUCCESS)
	{
		RegQueryValueEx(hAtspKey, szName, NULL, &dwDataType, (UCHAR*)szSearchString, &dwTmpLength);
		RegCloseKey(hAtspKey);

		if (dwDataType != REG_SZ && dwDataType != REG_EXPAND_SZ)
			*szSearchString = _T('\0');

		if (dwDataType == REG_EXPAND_SZ)
		{
			TCHAR szTmp[MAX_PATH] = { 0 };
			if (ExpandEnvironmentStrings(szSearchString, szTmp, MAX_PATH))
				lstrcpyn(szSearchString, szTmp, _countof(szSearchString));
		}
	}

	return CString(szSearchString);
}

void ECSTAAgentLoginLogout::DeleteRegValue(const TCHAR* szName)
{
	HKEY hKey;
	if (RegOpenKeyEx(HKEY_CURRENT_USER, m_strRegistryHive.c_str(), 0, KEY_SET_VALUE, &hKey) == ERROR_SUCCESS) {
		if (RegDeleteValue(hKey, szName) == ERROR_SUCCESS)
			RegCloseKey(hKey);
	}
}

void ECSTAAgentLoginLogout::SetRegSTRING(const wchar_t* szName, const wchar_t* szValue)
{
	if (szValue == 0 || _tcslen(szValue) == 0)
	{
		DeleteRegValue(szName);
	}
	else
	{
		HKEY hAtspKey;
		DWORD   dwDisposition;
		//Setzt einen STRING in die Registry
		if (RegCreateKeyEx(HKEY_CURRENT_USER, m_strRegistryHive.c_str(), 0, _T(""),
			REG_OPTION_NON_VOLATILE, KEY_WRITE,
			(LPSECURITY_ATTRIBUTES)NULL, &hAtspKey,
			&dwDisposition) == ERROR_SUCCESS)
		{
			RegSetValueEx(hAtspKey, szName, 0, REG_SZ, (UCHAR*)szValue, (int)((_tcslen(szValue) + 1) * sizeof(TCHAR)));
			RegCloseKey(hAtspKey);
		}
	}
}


BEGIN_MESSAGE_MAP(ECSTAAgentLoginLogout, CDialog)
	ON_BN_CLICKED(IDOK, &ECSTAAgentLoginLogout::OnBnClickedOk)
END_MESSAGE_MAP()


// ECSTAAgentLogin message handlers


void ECSTAAgentLoginLogout::OnBnClickedOk()
{
	for (auto& option : m_Options)
		GetDlgItemText(option.m_nDialogEditID, *option.m_pstrValue);

	if (m_pstrAgentID)
		SetRegSTRING(L"AgentID", *m_pstrAgentID);
	if (m_pstrGroupID)
		SetRegSTRING(L"GroupID", *m_pstrGroupID);
	if (m_pstrDeviceID)
		SetRegSTRING(L"DeviceID", *m_pstrDeviceID);
	if (m_pstrPassword)
		SetRegSTRING(L"Password", *m_pstrPassword);

	CDialog::OnOK();
}

const wchar_t* ECSTAAgentLoginLogout::GetAgentID() const
{
	if(m_pstrAgentID)
		return (const wchar_t*)*m_pstrAgentID;
	else
		return L"";
}
const wchar_t* ECSTAAgentLoginLogout::GetGroupID() const
{
	if (m_pstrGroupID)
		return (const wchar_t*)*m_pstrGroupID;
	else
		return L"";
}
const wchar_t* ECSTAAgentLoginLogout::GetDeviceID() const
{
	if (m_pstrDeviceID)
		return (const wchar_t*)*m_pstrDeviceID;
	else
		return L"";
}
const wchar_t* ECSTAAgentLoginLogout::GetPassword() const
{
	if (m_pstrPassword)
		return (const wchar_t*)*m_pstrPassword;
	else
		return L"";
}
