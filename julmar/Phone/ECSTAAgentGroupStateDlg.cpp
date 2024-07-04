#include "stdafx.h"
#include "phone.h"
#include "ECSTAAgentLoginLogout.h"
#include "ECSTAAgentGroupStateDlg.h"
#include "PhoneDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CECSTAAgentGroupStateDlg dialog

CECSTAAgentGroupStateDlg::CECSTAAgentGroupStateDlg(CWnd* pParent, CTapiLine* pLine, CTapiAddress* pAddr, const DWORD dwECSTACapabilites)
	: CDialog(CECSTAAgentGroupStateDlg::IDD, pParent)
{
	m_dwECSTACapabilites = dwECSTACapabilites;
	m_pLine = pLine;
	m_pAddr = pAddr;
	m_bGroupMode = m_dwECSTACapabilites & ECSTACAPS_AGENTGROUPS ? true : false;
	m_dwSupportedAgentStates = 0;
}

void CECSTAAgentGroupStateDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_GROUPLIST, m_lbGroups);
}


BEGIN_MESSAGE_MAP(CECSTAAgentGroupStateDlg, CDialog)
	ON_MESSAGE(UM_ECSTAAGENTEVENT, i_OnECSTAAgentEvent)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_GROUPLIST, OnListItemchanged)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_CMDAGENTLOGIN, &CECSTAAgentGroupStateDlg::OnBnClickedCmdagentlogin)
	ON_BN_CLICKED(IDC_CMDAGENTREADY, &CECSTAAgentGroupStateDlg::OnBnClickedCmdagentready)
	ON_BN_CLICKED(IDC_CMDAGENTLOGOFF, &CECSTAAgentGroupStateDlg::OnBnClickedCmdagentlogoff)
	ON_BN_CLICKED(IDC_CMDAGENTNOTREADY, &CECSTAAgentGroupStateDlg::OnBnClickedCmdagentnotready)
	ON_BN_CLICKED(IDC_CMDAGENTWORKINGAFTERCALL, &CECSTAAgentGroupStateDlg::OnBnClickedCmdagentWorkingAfterCall)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CECSTAAgentGroupStateDlg message handlers

BOOL CECSTAAgentGroupStateDlg::OnInitDialog()
{
	// Change our title.
	CString strTitle, strBuff;
	GetWindowText(strTitle);
	strBuff.Format(_T("%s - %s"), (const wchar_t*)strTitle, (const wchar_t*)m_pLine->GetLineName());
	SetWindowText(strBuff);

	// Connect all the controls via DDX
	__super::OnInitDialog();

	m_lbGroups.SetExtendedStyle(LVS_EX_FULLROWSELECT);
	CRect rect;
	m_lbGroups.GetClientRect(&rect);
	int nColWidth = rect.Width();

	ETSPVarStruct<VARSTRING> agentFeaturesStruct;
	if (m_pLine->GetDevConfigStruct(L"ecsta/AgentFeatures", &agentFeaturesStruct) == NO_ERROR)
	{
		if (agentFeaturesStruct.pData->dwStringFormat == STRINGFORMAT_BINARY && agentFeaturesStruct.pData->dwStringSize >= sizeof(ECSTADEVSPECIFICELEMENT_SUPPORTEDAGENTFEATURES))
		{
			ECSTADEVSPECIFICELEMENT_SUPPORTEDAGENTFEATURES* params = (ECSTADEVSPECIFICELEMENT_SUPPORTEDAGENTFEATURES*)((BYTE*)agentFeaturesStruct.pData + agentFeaturesStruct.pData->dwStringOffset);
			m_dwSupportedAgentStates = params->dwSupportedAgentStates;
			if (agentFeaturesStruct.pData->dwStringSize >= sizeof(ECSTADEVSPECIFICELEMENT_SUPPORTEDAGENTFEATURES2))
			{
				ECSTADEVSPECIFICELEMENT_SUPPORTEDAGENTFEATURES2* params2 = (ECSTADEVSPECIFICELEMENT_SUPPORTEDAGENTFEATURES2*)((BYTE*)agentFeaturesStruct.pData + agentFeaturesStruct.pData->dwStringOffset);
				m_dwRequiredLoginLogoutParams = params2->dwRequiredLoginLogoutParams;
				if (m_bGroupMode) {
					// The login logout ids are set implicitly through the selected entry in the group list
					m_dwRequiredLoginLogoutParams &= ~ECSTA150_AGENTLOGIN_GROUPID;
					m_dwRequiredLoginLogoutParams &= ~ECSTA150_AGENTLOGOUT_GROUPID;
				}
			}
		}
		else
		{
			m_dwSupportedAgentStates = 0xffffffff;
		}
	}
	else
	{
		m_dwSupportedAgentStates = 0xffffffff;
	}

	if (m_bGroupMode)
	{
		//get AgentGroups
		ETSPVarStruct<VARSTRING> agentgroupStruct;
		if (m_pLine->GetDevConfigStruct(L"ecsta/AgentGroups", &agentgroupStruct) == NO_ERROR)
		{
			if (agentgroupStruct.pData->dwStringFormat == STRINGFORMAT_BINARY && agentgroupStruct.pData->dwStringSize >= sizeof(ECSTA150DEVSPECIFIC20F))
			{
				int iNumGroups = agentgroupStruct.pData->dwStringSize / sizeof(ECSTA150DEVSPECIFIC20F);
				ECSTA150DEVSPECIFIC20F* pElement = (ECSTA150DEVSPECIFIC20F*)((BYTE*)agentgroupStruct.pData + agentgroupStruct.pData->dwStringOffset);
				for (int i = 0; i < iNumGroups; i++)
				{
					m_groups.push_back(*pElement);
					pElement++;
				}
			}
		}

		nColWidth /= 3;
		m_lbGroups.InsertColumn(0, _T("Number"), LVCFMT_LEFT, nColWidth);
		m_lbGroups.InsertColumn(1, _T("Name"), LVCFMT_LEFT, nColWidth);
		m_lbGroups.InsertColumn(2, _T("State"), LVCFMT_LEFT, nColWidth);

		int iIndex = 0;
		for (auto it = m_groups.begin(); it != m_groups.end(); it++)
		{
			m_lbGroups.InsertItem(iIndex, EStringToUnicode(it->szAgentGroupID).c_str(), 0);
			m_lbGroups.SetItemText(iIndex, 1, it->szAgentGroupName);
			ShowListState(iIndex);
			iIndex++;
		}
		m_lbGroups.SetSelectionMark(0);
		ShowGroupDetails(0);
	}
	else
	{
		SetDlgItemText(IDC_AGENT_GROUP, L"Agent-ID:");
		SetDlgItemText(IDC_STATIC_HEADER, L"Agent State History:");
		SetWindowText(L"ECSTA Agent Status");

		nColWidth /= 3;
		m_lbGroups.InsertColumn(0, _T("Agent-ID"), LVCFMT_LEFT, nColWidth);
		m_lbGroups.InsertColumn(1, _T("Time"), LVCFMT_LEFT, nColWidth);
		m_lbGroups.InsertColumn(2, _T("Agent-State"), LVCFMT_LEFT, nColWidth);

		m_nLastAgentStateIndex = -1;
		UpdateAgentState();
	}

	if (!(GetKeyState(VK_CONTROL) < 0))
	{
		GetDlgItem(IDC_CMDAGENTLOGIN)->ShowWindow(m_dwSupportedAgentStates & ECSTA150_SUPPORTEDAGENTSTATES_LOGGEDOUT ? SW_SHOW : SW_HIDE);
		GetDlgItem(IDC_CMDAGENTLOGOFF)->ShowWindow(m_dwSupportedAgentStates & ECSTA150_SUPPORTEDAGENTSTATES_LOGGEDOUT ? SW_SHOW : SW_HIDE);
		GetDlgItem(IDC_CMDAGENTNOTREADY)->ShowWindow(m_dwSupportedAgentStates & ECSTA150_SUPPORTEDAGENTSTATES_LOGGEDIN_NOTREADY ? SW_SHOW : SW_HIDE);
		GetDlgItem(IDC_CMDAGENTREADY)->ShowWindow(m_dwSupportedAgentStates & ECSTA150_SUPPORTEDAGENTSTATES_LOGGEDIN_READY ? SW_SHOW : SW_HIDE);
		GetDlgItem(IDC_CMDAGENTWORKINGAFTERCALL)->ShowWindow(m_dwSupportedAgentStates & ECSTA150_SUPPORTEDAGENTSTATES_LOGGEDIN_WORKINGAFTERCALL ? SW_SHOW : SW_HIDE);
	}

	GetDlgItem(IDC_STATIC_NOT_READY)->ShowWindow(m_dwSupportedAgentStates & ECSTA150_SUPPORTEDAGENTSTATES_LOGGEDIN_NOTREADY_REQUIRES_REASON ? SW_SHOW : SW_HIDE);
	GetDlgItem(IDC_TXTNOTREADYDETAIL)->ShowWindow(m_dwSupportedAgentStates & ECSTA150_SUPPORTEDAGENTSTATES_LOGGEDIN_NOTREADY_REQUIRES_REASON ? SW_SHOW : SW_HIDE);

	return TRUE;
}

void CECSTAAgentGroupStateDlg::UpdateAgentGroups()
{
	//get AgentGroups
	int iItem = m_lbGroups.GetSelectionMark();

	ETSPVarStruct<VARSTRING> agentgroupStruct;
	if (m_pLine->GetDevConfigStruct(L"ecsta/AgentGroups", &agentgroupStruct) == NO_ERROR)
	{
		if (agentgroupStruct.pData->dwStringFormat == STRINGFORMAT_BINARY && agentgroupStruct.pData->dwStringSize >= sizeof(ECSTA150DEVSPECIFIC20F))
		{
			m_groups.clear();

			int iNumGroups = agentgroupStruct.pData->dwStringSize / sizeof(ECSTA150DEVSPECIFIC20F);
			ECSTA150DEVSPECIFIC20F* pElement = (ECSTA150DEVSPECIFIC20F*)((BYTE*)agentgroupStruct.pData + agentgroupStruct.pData->dwStringOffset);
			for (int i = 0; i < iNumGroups; i++)
			{
				m_groups.push_back(*pElement);
				pElement++;
			}
		}
	}

	for (unsigned int i = 0; i < m_groups.size(); i++)
	{
		ShowListState(i);
	}
	if (iItem != -1)
		ShowGroupDetails(iItem);
}

void CECSTAAgentGroupStateDlg::UpdateAgentState()
{
	ECSTA150DEVSPECIFIC20F agentStruct;
	memset(&agentStruct, 0x00, sizeof(ECSTA150DEVSPECIFIC20F));
	if (m_nLastAgentStateIndex >= 0 && m_nLastAgentStateIndex <= 15)
	{
		agentStruct.dwFlags = m_nLastAgentStateIndex;
		agentStruct.dwFlags <<= 24;
		agentStruct.dwFlags |= ECSTAAGENTFLAG_USEINDEX;
	}
	if (agentStruct.dwFlags == 0)
		agentStruct.dwFlags = ECSTAAGENTFLAG_GETCURRENT;

	ETSPVarStruct<VARSTRING> agentData;
	DWORD dwSize = sizeof(VARSTRING) + sizeof(ECSTA150DEVSPECIFIC20F);
	agentData.ReAlloc(dwSize);
	agentData.pData->dwUsedSize = dwSize;
	agentData.pData->dwStringFormat = STRINGFORMAT_BINARY;
	agentData.pData->dwStringSize = sizeof(ECSTA150DEVSPECIFIC20F);
	agentData.pData->dwStringOffset = sizeof(VARSTRING);
	ECSTA150DEVSPECIFIC20F* pDevConfig = (ECSTA150DEVSPECIFIC20F*)(((LPBYTE)agentData.pData) + agentData.pData->dwStringOffset);
	memcpy(pDevConfig, &agentStruct, sizeof(ECSTA150DEVSPECIFIC20F));

	if (m_pLine->GetDevConfigStruct(L"ecsta/AgentState", &agentData) == NO_ERROR)
	{
		if (agentData.pData->dwStringFormat == STRINGFORMAT_BINARY && agentData.pData->dwStringSize >= sizeof(ECSTA150DEVSPECIFIC20F))
		{
			ECSTA150DEVSPECIFIC20F* pElement = (ECSTA150DEVSPECIFIC20F*)((BYTE*)agentData.pData + agentData.pData->dwStringOffset);
			memcpy(&m_VisibleState, pElement, sizeof(ECSTA150DEVSPECIFIC20F));
			ShowState(&m_VisibleState);
		}
	}

	if (strlen(m_VisibleState.szAgentID))
	{
		m_lbGroups.InsertItem(m_nListIndex, EStringToUnicode(m_VisibleState.szAgentID).c_str(), 0);
		SYSTEMTIME time;
		GetLocalTime(&time);
		CString strTime;
		strTime.Format(L"%02i:%02i:%02i:%03i", time.wHour, time.wMinute, time.wSecond, time.wMilliseconds);
		m_lbGroups.SetItemText(m_nListIndex, 1, strTime);
		m_lbGroups.SetItemText(m_nListIndex, 2, GetAgentStateText(m_VisibleState.dwAgentState));

		m_lbGroups.EnsureVisible(m_nListIndex, FALSE);
		m_nListIndex++;
	}
}

LRESULT CECSTAAgentGroupStateDlg::i_OnECSTAAgentEvent(WPARAM wParam, LPARAM lParam)
{
	UNUSED_ALWAYS(lParam);

	EAgentStateEvent* pEvent = (EAgentStateEvent*)wParam;

	if (pEvent->m_dwParam1 == ECSTADEVSPEC_AGENTGROUPSCHANGED)
		UpdateAgentGroups();
	else if (pEvent->m_dwParam1 == ECSTA150_AGENTSTATE)
	{
		// Agent State = pEvent->m_dwParam2 
		DWORD dwFlags = pEvent->m_dwParam3 & ECSTAAGENTFLAG_ESCAPEFLAGS;
		if (dwFlags & ECSTAAGENTFLAG_USEINDEX)
		{
			m_nLastAgentStateIndex = pEvent->m_dwParam3 & ECSTAAGENTFLAG_ESCAPEINDEX;
			m_nLastAgentStateIndex >>= 24;
		}
		else
		{
			m_nLastAgentStateIndex = -1;
		}
		UpdateAgentState();;
	}
	else
		ASSERT(FALSE);

	delete pEvent;

	return 0;

}// CECSTAAgentGroupStateDlg::i_OnAgentChange

void CECSTAAgentGroupStateDlg::OnListItemchanged(NMHDR* pNMHDR, LRESULT* pResult)
{
	if (m_bGroupMode)
	{
		NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
		if ((pNMListView->uChanged & LVIF_STATE)
			&& (pNMListView->uNewState & LVIS_SELECTED))
		{
			ShowGroupDetails(pNMListView->iItem);
		}
	}
	*pResult = 0;
}

const wchar_t* CECSTAAgentGroupStateDlg::GetAgentStateText(DWORD dwAgentState)
{
	switch (dwAgentState)
	{
	case ECSTA150_agentStateNoAgent:
		return L"Logged Off";
		break;
	case ECSTA150_agentStateNotReady:
		return L"Not Ready";
		break;
	case ECSTA150_agentStateReady:
		return L"Ready";
		break;
	case ECSTA150_agentStateBusy:
		return L"Busy";
		break;
	case ECSTA150_agentStateWorkingAfterCall:
		return L"Working after call";
		break;
	default:
		return L"unknown";
		break;
	}
}

void CECSTAAgentGroupStateDlg::ShowListState(const int iItem)
{
	int iIndex = 0;
	for (auto it = m_groups.begin(); it != m_groups.end(); it++)
	{
		if (iIndex == iItem)
		{
			m_lbGroups.SetItemText(iItem, 2, GetAgentStateText(it->dwAgentState));
		}
		iIndex++;
	}
}

void CECSTAAgentGroupStateDlg::ShowGroupDetails(const int iItem)
{
	if (m_bGroupMode)
	{
		int iIndex = 0;
		ECSTA150DEVSPECIFIC20F* pState = __nullptr;
		for (auto it = m_groups.begin(); it != m_groups.end(); it++)
		{
			if (iIndex == iItem)
			{
				pState = &(*it);
				break;
			}
			iIndex++;
		}
		ShowState(pState);
	}
}

void CECSTAAgentGroupStateDlg::ShowState(ECSTA150DEVSPECIFIC20F* pState)
{
	if (pState)
	{
		SetDlgItemText(IDC_TXTAGENTSTATE, GetAgentStateText(pState->dwAgentState));
		if (m_bGroupMode)
			SetDlgItemText(IDC_TXT_ID, EStringToUnicode(pState->szAgentGroupID).c_str());
		else {
			if (pState->dwAgentState == ECSTA150_agentStateNoAgent)
				SetDlgItemText(IDC_TXT_ID, L"");
			else
				SetDlgItemText(IDC_TXT_ID, EStringToUnicode(pState->szAgentID).c_str());
		}
		GetDlgItem(IDC_CMDAGENTLOGIN)->EnableWindow(pState->dwSupportedAgentRequests & ECSTA150_SUPPORTEDAGENTREQUESTS_LOGIN ? TRUE : FALSE);
		GetDlgItem(IDC_CMDAGENTLOGOFF)->EnableWindow(pState->dwSupportedAgentRequests & ECSTA150_SUPPORTEDAGENTREQUESTS_LOGOUT ? TRUE : FALSE);
		GetDlgItem(IDC_CMDAGENTNOTREADY)->EnableWindow(pState->dwSupportedAgentRequests & ECSTA150_SUPPORTEDAGENTREQUESTS_NOTREADY ? TRUE : FALSE);
		GetDlgItem(IDC_CMDAGENTREADY)->EnableWindow(pState->dwSupportedAgentRequests & ECSTA150_SUPPORTEDAGENTREQUESTS_READY ? TRUE : FALSE);
		GetDlgItem(IDC_CMDAGENTWORKINGAFTERCALL)->EnableWindow(pState->dwSupportedAgentRequests & ECSTA150_SUPPORTEDAGENTREQUESTS_WORKINGAFTERCALL ? TRUE : FALSE);
		((CEdit*)GetDlgItem(IDC_TXTNOTREADYDETAIL))->SetReadOnly(pState->dwSupportedAgentRequests & ECSTA150_SUPPORTEDAGENTREQUESTS_NOTREADY ? FALSE : TRUE);
	}
	else
	{
		GetDlgItem(IDC_CMDAGENTLOGIN)->EnableWindow(FALSE);
		GetDlgItem(IDC_CMDAGENTLOGOFF)->EnableWindow(FALSE);
		GetDlgItem(IDC_CMDAGENTNOTREADY)->EnableWindow(FALSE);
		GetDlgItem(IDC_CMDAGENTREADY)->EnableWindow(FALSE);
		GetDlgItem(IDC_CMDAGENTWORKINGAFTERCALL)->EnableWindow(FALSE);
		((CEdit*)GetDlgItem(IDC_TXTNOTREADYDETAIL))->SetReadOnly(TRUE);
	}
}

void CECSTAAgentGroupStateDlg::PrepareRequest(ECSTA150AGENTSTATEREQUEST21& request)
{
	memset(&request, 0x00, sizeof(request));

	CString strID;
	GetDlgItemText(IDC_TXT_ID, strID);
	CStdStringA strIDA = EStringToAnsi(strID);

	if (m_pAddr)
	{
		CStdStringA strDevice = EStringToAnsi(m_pAddr->GetDialableAddress());
		lstrcpynA(request.szDevice, strDevice.c_str(), _countof(request.szDevice));
	}

	if (m_bGroupMode)
		lstrcpynA(request.szAgentGroupID, strIDA.c_str(), _countof(request.szAgentGroupID));
	else
		lstrcpynA(request.szAgentID, strIDA.c_str(), _countof(request.szAgentID));
}

void CECSTAAgentGroupStateDlg::OnBnClickedCmdagentlogin()
{
	ECSTA150AGENTSTATEREQUEST21 state;
	PrepareRequest(state);
	state.dwAgentStateRequest = ECSTA150_agentRequestLogIn;

	bool bHandleRequest = true;
	if (m_dwRequiredLoginLogoutParams & ECSTA150_AGENTLOGIN_MASK)
	{
		ECSTAAgentLoginLogout login(m_dwRequiredLoginLogoutParams & ECSTA150_AGENTLOGIN_MASK, m_pLine, this);
		INT_PTR iResult = login.DoModal();
		if (iResult == IDOK)
		{
			lstrcpynA(state.szAgentID, EStringToAnsi(login.GetAgentID()).c_str(), _countof(state.szAgentID));
			lstrcpynA(state.szAgentGroupID, EStringToAnsi(login.GetGroupID()).c_str(), _countof(state.szAgentGroupID));
			lstrcpynA(state.szDevice, EStringToAnsi(login.GetDeviceID()).c_str(), _countof(state.szDevice));
			lstrcpynA(state.szAgentPassword, EStringToAnsi(login.GetPassword()).c_str(), _countof(state.szAgentPassword));
		}
		else if (iResult == IDCANCEL)
		{
			bHandleRequest = false;
		}
	}

	if (bHandleRequest)
		HandleRequest(state);
}

void CECSTAAgentGroupStateDlg::OnBnClickedCmdagentready()
{
	ECSTA150AGENTSTATEREQUEST21 state;
	PrepareRequest(state);
	state.dwAgentStateRequest = ECSTA150_agentRequestReady;
	HandleRequest(state);
}

void CECSTAAgentGroupStateDlg::OnBnClickedCmdagentlogoff()
{
	ECSTA150AGENTSTATEREQUEST21 state;
	PrepareRequest(state);
	state.dwAgentStateRequest = ECSTA150_agentRequestLogOut;

	bool bHandleRequest = true;
	if (m_dwRequiredLoginLogoutParams & ECSTA150_AGENTLOGOUT_MASK)
	{
		ECSTAAgentLoginLogout login(m_dwRequiredLoginLogoutParams & ECSTA150_AGENTLOGOUT_MASK, m_pLine, this);
		INT_PTR iResult = login.DoModal();
		if (iResult == IDOK)
		{
			lstrcpynA(state.szAgentID, EStringToAnsi(login.GetAgentID()).c_str(), _countof(state.szAgentID));
			lstrcpynA(state.szAgentGroupID, EStringToAnsi(login.GetGroupID()).c_str(), _countof(state.szAgentGroupID));
			lstrcpynA(state.szDevice, EStringToAnsi(login.GetDeviceID()).c_str(), _countof(state.szDevice));
			lstrcpynA(state.szAgentPassword, EStringToAnsi(login.GetPassword()).c_str(), _countof(state.szAgentPassword));
		}
		else if (iResult == IDCANCEL)
		{
			bHandleRequest = false;
		}
	}

	if (bHandleRequest)
		HandleRequest(state);
}

void CECSTAAgentGroupStateDlg::OnBnClickedCmdagentnotready()
{
	ECSTA150AGENTSTATEREQUEST21 state;
	PrepareRequest(state);
	state.dwAgentStateRequest = ECSTA150_agentRequestNotReady;
	if (GetDlgItem(IDC_TXTNOTREADYDETAIL)->IsWindowVisible())
		state.dwAgentStateDetail = GetDlgItemInt(IDC_TXTNOTREADYDETAIL);
	HandleRequest(state);
}

void CECSTAAgentGroupStateDlg::OnBnClickedCmdagentWorkingAfterCall()
{
	ECSTA150AGENTSTATEREQUEST21 state;
	PrepareRequest(state);
	state.dwAgentStateRequest = ECSTA150_agentRequestWorkingAfterCall;
	HandleRequest(state);
}

void CECSTAAgentGroupStateDlg::HandleRequest(ECSTA150AGENTSTATEREQUEST21& state)
{
	LONG lResult = m_pLine->SetECSTAAgentState(&state);
	lResult = GetTAPIConnection()->WaitForReply(lResult);
	if (lResult != NO_ERROR)
		ShowError("lineDevSpecificFeature", lResult);
}
