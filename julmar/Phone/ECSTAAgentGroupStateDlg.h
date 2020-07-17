#pragma once

/////////////////////////////////////////////////////////////////////////////
// CECSTAAgentGroupStateDlg dialog

class CMyLine;

class EAgentStateEvent
{
public:
	CMyLine* m_pLine = NULL;
	DWORD m_dwParam1 = 0;
	DWORD m_dwParam2 = 0;
	DWORD m_dwParam3 = 0;
};

class CECSTAAgentGroupStateDlg : public CDialog
{
// Construction
public:
	CECSTAAgentGroupStateDlg(CWnd* pParent, CTapiLine* pLine, CTapiAddress* pAddr, const DWORD dwECSTACapabilites);

	enum { IDD = IDD_ECSTAAGENTSTATE };

protected:
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	afx_msg void OnListItemchanged(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBnClickedCmdagentlogin();
	afx_msg void OnBnClickedCmdagentready();
	afx_msg void OnBnClickedCmdagentlogoff();
	afx_msg void OnBnClickedCmdagentnotready();
	afx_msg void OnBnClickedCmdagentWorkingAfterCall();
	afx_msg LRESULT i_OnECSTAAgentEvent(WPARAM, LPARAM);

	const wchar_t* GetAgentStateText(DWORD dwAgentState);
	void ShowGroupDetails(const int iItem);
	void ShowListState(const int iItem);
	void UpdateAgentGroups();
	void UpdateAgentState();
	void PrepareRequest(ECSTA150AGENTSTATEREQUEST21& state);
	void HandleRequest(ECSTA150AGENTSTATEREQUEST21& state);

	void ShowState(ECSTA150DEVSPECIFIC20F* pState);

	std::list<ECSTA150DEVSPECIFIC20F> m_groups;
	
	int m_nListIndex = 0;
	int m_nLastAgentStateIndex = -1;
	ECSTA150DEVSPECIFIC20F m_VisibleState;

	CTapiLine* m_pLine;
	CTapiAddress* m_pAddr;
	CListCtrl m_lbGroups;
	DWORD m_dwECSTACapabilites;
	bool m_bGroupMode;
	DWORD m_dwSupportedAgentStates;
	DWORD m_dwRequiredLoginLogoutParams;

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnEnChangeTxtId();
};

