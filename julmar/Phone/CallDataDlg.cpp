// CallDataDlg.cpp : implementation file
//
// This is a part of the TAPI Applications Classes C++ library.
// Original Copyright © 1995-2004 JulMar Entertainment Technology, Inc. All rights reserved.
//
// "This program is free software; you can redistribute it and/or modify it under the terms of 
// the GNU General Public License as published by the Free Software Foundation; version 2 of the License.
// This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without 
// even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General 
// Public License for more details.
//
// You should have received a copy of the GNU General Public License along with this program; if not, write 
// to the Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA. 
// Or, contact: JulMar Technology, Inc. at: info@julmar.com." 
//

#include "stdafx.h"
#include "Phone.h"
#include "PhoneDlg.h"
#include "CallDataDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

const UINT IDT_TIMER_CALLDATA = 101;

/////////////////////////////////////////////////////////////////////////////
// CCallDataDlg dialog


CCallDataDlg::CCallDataDlg(CPhoneDlg* pParent)
	: CDialog(CCallDataDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CCallDataDlg)
	m_strCallData = _T("");
	m_pPhoneDlg = pParent;
	//}}AFX_DATA_INIT
}

void CCallDataDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCallDataDlg)
	DDX_Text(pDX, IDC_CALLDATA, m_strCallData);
	DDX_Text(pDX, IDC_CALLLINKAGEDATA, m_strCallLinkageData);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CCallDataDlg, CDialog)
	//{{AFX_MSG_MAP(CCallDataDlg)
	ON_EN_CHANGE(IDC_CALLDATA, OnChangeCalldata)
	ON_BN_CLICKED(IDOK, OnSetCallData)
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCallDataDlg message handlers

void CCallDataDlg::OnChangeCalldata() 
{
	UpdateData(TRUE);
	CTapiCall* pCall = m_pPhoneDlg->GetActiveCall();
	GetDlgItem(IDOK)->EnableWindow(pCall && !m_strCallData.IsEmpty() ? TRUE : FALSE);
}

void CCallDataDlg::OnSetCallData() 
{
	UpdateData(TRUE);

	// Set the call data in the call.
	CTapiCall* pCall = m_pPhoneDlg->GetActiveCall();
	if (pCall)
	{
		LONG lResult = pCall->SetPrivilege(LINECALLPRIVILEGE_OWNER);
		if (lResult != 0)
			ShowError("lineSetCallPrivilege", lResult);
		else
		{
			lResult = GetTAPIConnection()->WaitForReply(pCall->SetCallData((LPVOID)((LPCTSTR)m_strCallData), m_strCallData.GetLength() * sizeof(wchar_t)));
			if (lResult != 0)
				ShowError("lineSetCallData", lResult);
		}
		// If we have CALLdata in our call now, retrieve it.
		LPLINECALLINFO lpCallInfo = pCall->GetCallInfo();
		if (lpCallInfo->dwCallDataSize > 0)
		{
			LPVOID lpBuff = (LPVOID)((LPBYTE)lpCallInfo + lpCallInfo->dwCallDataOffset);
			m_edtHex.SetData(lpBuff, lpCallInfo->dwCallDataSize);
		}
		else
			m_edtHex.SetData(NULL, 0);
	}
	else
	{
		m_strCallData = L"";
		m_strCallLinkageData = L"";
		m_edtHex.SetData(NULL, 0);
	}
}

BOOL CCallDataDlg::OnInitDialog()
{
    // Reset the font to all be ANSI var.
    CFont fntAnsi;
    fntAnsi.CreateStockObject (ANSI_VAR_FONT);
    CWnd* pwndChild = GetWindow (GW_CHILD);
    while (pwndChild != NULL && IsChild(pwndChild))
    {
        pwndChild->SetFont(&fntAnsi);
        pwndChild = pwndChild->GetWindow(GW_HWNDNEXT);
    }

	m_edtHex.Init(GetDlgItem(IDC_HEXDATA)->GetSafeHwnd());

	CTapiCall* pCall = m_pPhoneDlg->GetActiveCall();
	if (pCall)
	{
		// Enable/Disable the edit.
		CTapiAddress* pAddr = pCall->GetAddressInfo();
		GetDlgItem(IDC_CALLDATA)->EnableWindow((pCall->GetCallStatus()->dwCallFeatures & LINECALLFEATURE_SETCALLDATA) > 0);
		((CEdit*)GetDlgItem(IDC_CALLDATA))->LimitText(pAddr->GetAddressCaps()->dwMaxCallDataSize);
	}

	OnCallChanged();

	// Connect all the controls via DDX
	CDialog::OnInitDialog();

	return TRUE;
}

void CCallDataDlg::OnCallChanged()
{
	std::wstring strCallLinkageData;
	UpdateData(TRUE);

	bool bLineSupportsCallLinkageID = false;
	bool bIsNoECSTA = false;
	CTapiLine* pLine = m_pPhoneDlg->GetActiveLine();
	if (pLine && pLine->IsOpen())
	{
		const ECSTALINEDEVCAPS4* pECSTACaps = pLine->GetECSTALineCaps4();
		if (pECSTACaps)
		{
			if (pECSTACaps->dwECSTACapabilities & ECSTACAPS_CALLLINKAGEID)
				bLineSupportsCallLinkageID = true;
		}
		else
			bIsNoECSTA = true;
	}

	// If we have CALLdata in our call now, retrieve it.
	CTapiCall* pCall = m_pPhoneDlg->GetActiveCall();
	if (pCall)
	{
		LPLINECALLINFO lpCallInfo = pCall->GetCallInfo();
		if (lpCallInfo->dwCallDataSize > 0)
		{
			LPVOID lpBuff = (LPVOID)((LPBYTE)lpCallInfo + lpCallInfo->dwCallDataOffset);
			m_edtHex.SetData(lpBuff, lpCallInfo->dwCallDataSize);
		}
		else
		{
			m_edtHex.SetData(NULL, 0);
		}

		// TSP supports CallLinkageID, we have a DevSpecific Object and the size of the devspecific is larger or equal the ECSTADEVSPECIFICELEMENTLIST Object size
		if (bLineSupportsCallLinkageID && lpCallInfo->dwDevSpecificOffset && lpCallInfo->dwDevSpecificSize >= sizeof(ECSTADEVSPECIFICELEMENTLISTV2))
		{
			try
			{
				ECSTADEVSPECIFICELEMENTLISTV2* pList = (ECSTADEVSPECIFICELEMENTLISTV2*)(((LPBYTE)lpCallInfo) + lpCallInfo->dwDevSpecificOffset);
				// Ist der Magic Value vorhanden und eine uns bekannte Version sowie inhaltlich Elemente vorhanden?
				if (pList->dwMagicValue == ECSTADEVSPECIFICELEMENTLISTV2_MAGICVALUE && pList->dwVersion == ECSTADEVSPECIFICELEMENTLISTV2_VERSION && pList->dwNumElements)
				{
					if (lpCallInfo->dwDevSpecificSize >= sizeof(ECSTADEVSPECIFICELEMENTLISTV2) + (pList->dwNumElements - 1) * sizeof(ECSTADEVSPECIFICELEMENT))
					{
						for (unsigned int iCount = 0; iCount < pList->dwNumElements; iCount++)
						{
							const ECSTADEVSPECIFICELEMENT& element = pList->elements[iCount];
							DWORD dwLastElementPos = element.dwOffset + element.dwSize;
							if (dwLastElementPos <= lpCallInfo->dwDevSpecificSize)
							{
								DWORD dwOffset = lpCallInfo->dwDevSpecificOffset + element.dwOffset;
								if (element.dwElementID == ECSTADEVSPECIFICELEMENT_CALLLINKAGEID)
								{
									// Offset befindet sich im Bereich des DevSpefics
									strCallLinkageData = VarInfoGetStdStringW(lpCallInfo, dwOffset, element.dwSize, 100).c_str();
								}
							}
						}
					}
				}
			}
			catch (...)
			{
				strCallLinkageData = L"Exception catched";
			}
		}
	}
	else
	{
		m_edtHex.SetData(NULL, 0);
	}

	GetDlgItem(IDOK)->EnableWindow(pCall && !m_strCallData.IsEmpty() ? TRUE : FALSE);

	if (bIsNoECSTA)
		m_strCallLinkageData = L"No Call Linkage ID support (no ECSTA TAPI driver)";
	else if(!bLineSupportsCallLinkageID)
		m_strCallLinkageData = L"No Call Linkage ID support (ECSTA does not support it)";
	else
		m_strCallLinkageData = strCallLinkageData.c_str();

	UpdateData(FALSE);
}
