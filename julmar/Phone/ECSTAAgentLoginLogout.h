#pragma once


// ECSTAAgentLogin dialog

class AgentLoginParam
{
public:
	AgentLoginParam(CString* pstrValue, const wchar_t* szCaption)
	{
		m_pstrValue = pstrValue;
		m_strCaption = szCaption;
		m_nDialogEditID = 0;
	}
	CString* m_pstrValue = NULL;
	CString m_strCaption;
	int m_nDialogEditID;
};

class ECSTAAgentLoginLogout : public CDialog
{
	DECLARE_DYNAMIC(ECSTAAgentLoginLogout)

public:
	ECSTAAgentLoginLogout(const DWORD dwRequiredLoginParameters, CTapiLine* pLine, CWnd* pParent = nullptr);   // standard constructor
	virtual ~ECSTAAgentLoginLogout();

// Dialog Data
	enum { IDD = IDD_ECSTAAGENTLOGINLOGOUT };

	std::list<AgentLoginParam> m_Options;

	const wchar_t* GetAgentID() const;
	const wchar_t* GetGroupID() const;
	const wchar_t* GetDeviceID() const;
	const wchar_t* GetPassword() const;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	CString GetRegSTRING(const wchar_t* szName);
	void SetRegSTRING(const wchar_t* szName, const wchar_t* szValue);
	void DeleteRegValue(const TCHAR* szName);

	CString* m_pstrAgentID = __nullptr;
	CString* m_pstrGroupID = __nullptr;
	CString* m_pstrDeviceID = __nullptr;
	CString* m_pstrPassword = __nullptr;
	DWORD m_dwRequiredLoginParameters;
	std::wstring m_strRegistryHive;

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
};
