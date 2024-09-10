// ForwardList.cpp : implementation file
//

#include "stdafx.h"
#include "ForwardList.h"
#include "ForwardDlg.h"
#include "Phone.h"
#include "PhoneDlg.h"

// CForwardList dialog

IMPLEMENT_DYNAMIC(CForwardList, CDialog)

CForwardList::CForwardList(CWnd* pParent, CTapiLine* pLine)
	: CDialog(CForwardList::IDD, pParent)
{
	m_pLine = pLine;
	m_dwAddressCount = m_pLine->GetAddressCount();
}

CForwardList::~CForwardList()
{
}

void CForwardList::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_List);
}

BEGIN_MESSAGE_MAP(CForwardList, CDialog)
	ON_BN_CLICKED(IDC_CMDADD, &CForwardList::OnBnClickedCmdadd)
	ON_BN_CLICKED(IDC_CMDREMOVE, &CForwardList::OnBnClickedCmdremove)
	ON_BN_CLICKED(IDC_CMDREMOVEALL, &CForwardList::OnBnClickedCmdremoveall)
END_MESSAGE_MAP()

// CForwardList message handlers

BOOL CForwardList::OnInitDialog()
{
	CDialog::OnInitDialog();

	if (m_dwAddressCount > 1)
	{
		m_List.InsertColumn(0, _T("Address"), 0, 170);
		m_List.InsertColumn(1, _T("Type"), 0, 220);
		m_List.InsertColumn(2, _T("Destination"), 0, 155);
	}
	else
	{
		m_List.InsertColumn(0, _T("Type"), 0, 272);
		m_List.InsertColumn(1, _T("Destination"), 0, 272);
	}

	UpdateList();

	return TRUE; // return TRUE unless you set the focus to a control
				 // EXCEPTION: OCX Property Pages should return FALSE
}

TCHAR* CForwardList::GetForwardModeString(DWORD dwForwardMode)
{
	switch (dwForwardMode)
	{
		case LINEFORWARDMODE_UNCOND:
			return _T("LINEFORWARDMODE_UNCOND");
		case LINEFORWARDMODE_UNCONDINTERNAL:
			return _T("LINEFORWARDMODE_UNCONDINTERNAL");
		case LINEFORWARDMODE_UNCONDEXTERNAL:
			return _T("LINEFORWARDMODE_UNCONDEXTERNAL");
		case LINEFORWARDMODE_UNCONDSPECIFIC:
			return _T("LINEFORWARDMODE_UNCONDSPECIFIC");
		case LINEFORWARDMODE_BUSY:
			return _T("LINEFORWARDMODE_BUSY");
		case LINEFORWARDMODE_BUSYINTERNAL:
			return _T("LINEFORWARDMODE_BUSYINTERNAL");
		case LINEFORWARDMODE_BUSYEXTERNAL:
			return _T("LINEFORWARDMODE_BUSYEXTERNAL");
		case LINEFORWARDMODE_BUSYSPECIFIC:
			return _T("LINEFORWARDMODE_BUSYSPECIFIC");
		case LINEFORWARDMODE_NOANSW:
			return _T("LINEFORWARDMODE_NOANSW");
		case LINEFORWARDMODE_NOANSWINTERNAL:
			return _T("LINEFORWARDMODE_NOANSWINTERNAL");
		case LINEFORWARDMODE_NOANSWEXTERNAL:
			return _T("LINEFORWARDMODE_NOANSWEXTERNAL");
		case LINEFORWARDMODE_NOANSWSPECIFIC:
			return _T("LINEFORWARDMODE_NOANSWSPECIFIC");
		case LINEFORWARDMODE_BUSYNA:
			return _T("LINEFORWARDMODE_BUSYNA");
		case LINEFORWARDMODE_BUSYNAINTERNAL:
			return _T("LINEFORWARDMODE_BUSYNAINTERNAL");
		case LINEFORWARDMODE_BUSYNAEXTERNAL:
			return _T("LINEFORWARDMODE_BUSYNAEXTERNAL");
		case LINEFORWARDMODE_BUSYNASPECIFIC:
			return _T("LINEFORWARDMODE_BUSYNASPECIFIC");
		case LINEFORWARDMODE_UNKNOWN:
			return _T("LINEFORWARDMODE_UNKNOWN");
		case LINEFORWARDMODE_UNAVAIL:
			return _T("LINEFORWARDMODE_UNAVAIL");
	}

	return _T("LINEFORWARDMODE_UNKNOWN");
}

void CForwardList::UpdateList()
{
	m_List.DeleteAllItems();

	m_pLine->GetAddressCount();
	m_ForwardList.ClearList();

	for (DWORD dwCount = 0; dwCount < m_dwAddressCount; dwCount++)
	{
		CTapiAddress* pAddress = m_pLine->GetAddress(dwCount);
		if (pAddress)
		{
			LPLINEADDRESSSTATUS lpStatus = pAddress->GetAddressStatus(TRUE);
			m_ForwardList.AddAddressStatus(lpStatus, pAddress->GetDisplayText(false), dwCount);
		}
	}
	ETapiForwardEntryList::iterator it;
	for (it = m_ForwardList.m_ForwardEntryList.begin(); it != m_ForwardList.m_ForwardEntryList.end(); it++)
	{
		ETapiForwardEntry& entry = *it;

		CString strMode = GetForwardModeString(entry.dwForwardMode);

		if (entry.dwForwardMode == LINEFORWARDMODE_UNCOND && _tcslen(entry.szDestination) == 0)
			strMode += _T(" (DND)");
		else if (entry.dwNumRingsNoAnswer)
			strMode.AppendFormat(L" (%i rings)", (int)entry.dwNumRingsNoAnswer);

		if (m_dwAddressCount > 1)
		{
			int iItem = m_List.InsertItem(0, entry.szDialableAddress);
			m_List.SetItemText(iItem, 1, strMode);
			m_List.SetItemText(iItem, 2, entry.szDestination);
			m_List.SetItemData(iItem, entry.dwInternalID);
		}
		else
		{
			int iItem = m_List.InsertItem(0, strMode);
			m_List.SetItemText(iItem, 1, entry.szDestination);
			m_List.SetItemData(iItem, entry.dwInternalID);
		}
	}
}
void CForwardList::OnBnClickedCmdadd()
{
	CForwardDlg dlg(this, m_pLine);
	if (dlg.DoModal() == IDOK)
	{
		ETapiForwardEntry entry;
		entry.dwForwardMode = dlg.m_dwFwdMode;
		entry.dwAddressID = dlg.m_bAllAddresses ? 0xffffffff : dlg.m_dwAddress;
		lstrcpyn(entry.szCallerID, dlg.m_strCaller, sizeof(entry.szCallerID));
		lstrcpyn(entry.szDestination, dlg.m_strDest, sizeof(entry.szDestination));
		entry.dwInternalID = m_ForwardList.GetNextEntryID();

		ETapiForwardList newList;
		newList = m_ForwardList;
		newList.m_ForwardEntryList.push_back(entry);
		LINEFORWARDLIST* plist = newList.GetLineForwardList(entry.dwAddressID);

		CTapiCall* pCall = NULL;
		LONG lResult = GetTAPIConnection()->WaitForReply(m_pLine->Forward(entry.dwAddressID, plist, dlg.m_iNumRings, &pCall, NULL));
		if (lResult != 0)
			ShowError("lineForward", lResult);

		if (pCall != NULL)
		{
			pCall->Drop();
			pCall->Deallocate();
		}

		free(plist);
	}
}

void CForwardList::OnBnClickedCmdremove()
{
	ETapiForwardList newList;
	newList = m_ForwardList;

	DWORD dwAddressID = 0;
	bool m_bAddressFetched = false;
	POSITION pos = m_List.GetFirstSelectedItemPosition();
	while (pos)
	{
		int iItem = m_List.GetNextSelectedItem(pos);
		DWORD_PTR dwData = m_List.GetItemData(iItem);

		// find the entry and remove it.
		ETapiForwardEntryList::iterator it;
		for (it = newList.m_ForwardEntryList.begin(); it != newList.m_ForwardEntryList.end(); it++)
		{
			if (it->dwInternalID == dwData)
			{
				if (!m_bAddressFetched)
				{
					dwAddressID = it->dwAddressID;
					m_bAddressFetched = true;
				}
				else if (dwAddressID != 0xffffffff)
				{
					if (dwAddressID != it->dwAddressID)
						dwAddressID = 0xffffffff;
				}
				newList.m_ForwardEntryList.erase(it);
				break;
			}
		}
	}

	LINEFORWARDLIST* plist = newList.GetLineForwardList(dwAddressID);

	CTapiCall* pCall = NULL;
	LONG lResult = GetTAPIConnection()->WaitForReply(m_pLine->Forward(dwAddressID, plist, 0, &pCall, NULL));
	if (lResult != 0)
		ShowError("lineForward", lResult);

	if (pCall != NULL)
	{
		pCall->Drop();
		pCall->Deallocate();
	}

	free(plist);
}

void CForwardList::OnBnClickedCmdremoveall()
{
	LINEFORWARDLIST* plist = NULL;
	CTapiCall* pCall = NULL;
	LONG lResult = GetTAPIConnection()->WaitForReply(m_pLine->Forward(0xffffffff, plist, 0, &pCall, NULL));
	if (lResult != 0)
		ShowError("lineForward", lResult);
}
