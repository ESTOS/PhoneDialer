// ForwardDlg.cpp : implementation file
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
#include "ForwardDlg.h"
#include "phone.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define EPHONEEXELINEFORWARDMODE_DND 0x10000000
/////////////////////////////////////////////////////////////////////////////
// CForwardDlg dialog

CForwardDlg::CForwardDlg(CWnd* pParent, CTapiLine* pLine)
	: CDialog(CForwardDlg::IDD, pParent)
{
	ASSERT(pLine != NULL);

	//{{AFX_DATA_INIT(CForwardDlg)
	m_bAllAddresses = FALSE;
	m_bSupportsAllModes = FALSE;
	m_strCaller = _T("");
	m_strDest = _T("");
	m_iNumRings = 0;
	m_pLine = pLine;
	m_iMinRings = 0;
	m_iMaxRings = 0;
	m_dwAddress = 0;
	m_dwFwdMode = 0;
	m_dwAvailFwdModes = 0;
	//}}AFX_DATA_INIT
}

void CForwardDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CForwardDlg)
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDC_SPIN1, m_ctlSpin);
	DDX_Control(pDX, IDC_FWDMODES, m_cbFwdModes);
	DDX_Control(pDX, IDC_ADDRESSES, m_cbAddress);
	DDX_Check(pDX, IDC_ALLADDR, m_bAllAddresses);
	DDX_Text(pDX, IDC_CALLER, m_strCaller);
	DDX_Text(pDX, IDC_DEST, m_strDest);
	DDX_Text(pDX, IDC_NUMRINGS, m_iNumRings);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CForwardDlg, CDialog)
	ON_EN_CHANGE(IDC_CALLER, OnChange)
	ON_BN_CLICKED(IDC_ALLADDR, OnAllAddresses)
	ON_CBN_SELCHANGE(IDC_FWDMODES, OnSelchangeFwdmodes)
	ON_CBN_SELCHANGE(IDC_ADDRESSES, OnAddressChange)
	ON_EN_CHANGE(IDC_DEST, OnChange)
	ON_EN_CHANGE(IDC_NUMRINGS, OnChange)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CForwardDlg message handlers

BOOL CForwardDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	if (GetKeyState(VK_CONTROL) < 0)
		m_bSupportsAllModes = TRUE;

	((CEdit*)GetDlgItem(IDC_NUMRINGS))->LimitText(3);
	m_ctlSpin.SetRange(0, 999);

	// Add all the address
	for (unsigned int i = 0; i < m_pLine->GetAddressCount(); i++)
	{
		CTapiAddress* pAddr = m_pLine->GetAddress(i);
		int iPos = m_cbAddress.AddString(pAddr->GetDisplayText(false));
		m_cbAddress.SetItemData(iPos, (DWORD_PTR)pAddr);
	}

	// If no addresses, then always forward all
	if (m_cbAddress.GetCount() == 0)
	{
		m_bAllAddresses = TRUE;
		GetDlgItem(IDC_ALLADDR)->EnableWindow(FALSE);
		OnAllAddresses();
	}
	else
	{
		m_cbAddress.SetCurSel(0);
		OnAddressChange();
	}

	OnSelchangeFwdmodes();
	return TRUE; // return TRUE unless you set the focus to a control
				 // EXCEPTION: OCX Property Pages should return FALSE
}

void CForwardDlg::OnOK()
{
	UpdateData(TRUE);
	m_dwFwdMode = (DWORD)m_cbFwdModes.GetItemData(m_cbFwdModes.GetCurSel());
	if (m_dwFwdMode == EPHONEEXELINEFORWARDMODE_DND)
	{
		m_strCaller = _T("");
		m_strDest = _T("");
		m_dwFwdMode = LINEFORWARDMODE_UNCOND;
	}
	else if (!GetDlgItem(IDC_CALLER)->IsWindowEnabled())
	{
		m_strCaller = _T("");
		UpdateData(FALSE);
	}
	else
	{
		switch (m_dwFwdMode)
		{
			case LINEFORWARDMODE_UNCOND:
				m_dwFwdMode = LINEFORWARDMODE_UNCONDSPECIFIC;
				break;
			case LINEFORWARDMODE_BUSY:
				m_dwFwdMode = LINEFORWARDMODE_BUSYSPECIFIC;
				break;
			case LINEFORWARDMODE_NOANSW:
				m_dwFwdMode = LINEFORWARDMODE_NOANSWSPECIFIC;
				break;
			case LINEFORWARDMODE_BUSYNA:
				m_dwFwdMode = LINEFORWARDMODE_BUSYNASPECIFIC;
				break;
			default:
				m_strCaller = _T("");
				UpdateData(FALSE);
				break;
		}
	}

	int iCurSel = m_cbAddress.GetCurSel();
	if (iCurSel != CB_ERR)
		m_dwAddress = ((CTapiAddress*)m_cbAddress.GetItemData(iCurSel))->GetAddressID();

	CDialog::OnOK();
}

void CForwardDlg::OnChange()
{
	if (m_btnOK.GetSafeHwnd() != NULL)
	{
		UpdateData(TRUE);
		DWORD dwFwdMode = 0;
		int iCurSel = m_cbFwdModes.GetCurSel();
		if (iCurSel != CB_ERR)
			dwFwdMode = (DWORD)m_cbFwdModes.GetItemData(iCurSel);
		if (dwFwdMode == EPHONEEXELINEFORWARDMODE_DND)
			m_btnOK.EnableWindow(TRUE);
		else
			m_btnOK.EnableWindow(!m_strDest.IsEmpty() && m_cbFwdModes.GetCurSel() != CB_ERR);
	}
}

void CForwardDlg::OnAllAddresses()
{
	UpdateData(TRUE);

	if (m_bAllAddresses)
	{
		m_cbAddress.EnableWindow(FALSE);
		bool bFindEntry = true;
		{
			CTapiAddress* pAddr = (CTapiAddress*)m_cbAddress.GetItemData(m_cbAddress.GetCurSel());
			if (pAddr)
			{
				LINEADDRESSCAPS* lpCaps = pAddr->GetAddressCaps();
				if (lpCaps && (lpCaps->dwForwardModes || lpCaps->dwAddressFeatures & LINEADDRFEATURE_FORWARDDND))
					bFindEntry = false;
			}
		}
		if (bFindEntry)
		{
			for (int iCount = 0; iCount < m_cbAddress.GetCount(); iCount++)
			{
				CTapiAddress* pAddr = (CTapiAddress*)m_cbAddress.GetItemData(iCount);

				LINEADDRESSCAPS* lpCaps = pAddr->GetAddressCaps();
				if (lpCaps && (lpCaps->dwForwardModes || lpCaps->dwAddressFeatures & LINEADDRFEATURE_FORWARDDND))
				{
					m_cbAddress.SetCurSel(iCount);
					OnAddressChange();
					break;
				}
			}
		}
	}
	else
	{
		m_cbAddress.EnableWindow(TRUE);
		GotoDlgCtrl(&m_cbAddress);
		if (m_cbAddress.GetCurSel() == CB_ERR)
			m_cbAddress.SetCurSel(0);
		OnAddressChange();
	}

	OnChange();
}

void CForwardDlg::OnSelchangeFwdmodes()
{
	int iCurSel = m_cbFwdModes.GetCurSel();
	if (iCurSel == CB_ERR)
	{
		GetDlgItem(IDC_CALLER)->EnableWindow(FALSE);
		return;
	}

	DWORD dwFwdMode = (DWORD)m_cbFwdModes.GetItemData(iCurSel);
	if (dwFwdMode == EPHONEEXELINEFORWARDMODE_DND)
	{
		GetDlgItem(IDC_CALLER)->EnableWindow(FALSE);
		GetDlgItem(IDC_DEST)->EnableWindow(FALSE);
		GetDlgItem(IDC_NUMRINGS)->EnableWindow(FALSE);
	}
	else
	{
		if ((m_dwAvailFwdModes & (LINEFORWARDMODE_UNCONDSPECIFIC | LINEFORWARDMODE_BUSYSPECIFIC | LINEFORWARDMODE_NOANSWSPECIFIC | LINEFORWARDMODE_BUSYNASPECIFIC)) && dwFwdMode & (LINEFORWARDMODE_UNCOND | LINEFORWARDMODE_BUSY | LINEFORWARDMODE_NOANSW | LINEFORWARDMODE_BUSYNA))
			GetDlgItem(IDC_CALLER)->EnableWindow(TRUE);
		else
			GetDlgItem(IDC_CALLER)->EnableWindow(FALSE);
		if (m_iMinRings == 0 && m_iMaxRings == 0)
			GetDlgItem(IDC_NUMRINGS)->EnableWindow(FALSE);
		else if (dwFwdMode & (LINEFORWARDMODE_NOANSW | LINEFORWARDMODE_NOANSWINTERNAL | LINEFORWARDMODE_NOANSWEXTERNAL | LINEFORWARDMODE_NOANSWSPECIFIC))
			GetDlgItem(IDC_NUMRINGS)->EnableWindow(TRUE);
		else
			GetDlgItem(IDC_NUMRINGS)->EnableWindow(FALSE);

		GetDlgItem(IDC_DEST)->EnableWindow(TRUE);
	}

	OnChange();
}

void CForwardDlg::OnAddressChange()
{
	static struct
	{
		DWORD dwFwdMode;
		LPCTSTR pszName;
	} FwdModes[] = {
		{LINEFORWARDMODE_UNCOND, _T("Unconditional")}, {LINEFORWARDMODE_UNCONDINTERNAL, _T("Internal (Uncond)")}, {LINEFORWARDMODE_UNCONDEXTERNAL, _T("External (Uncond)")}, {LINEFORWARDMODE_BUSY, _T("Busy")}, {LINEFORWARDMODE_BUSYINTERNAL, _T("Internal (Busy)")}, {LINEFORWARDMODE_BUSYEXTERNAL, _T("External (Busy)")}, {LINEFORWARDMODE_NOANSW, _T("No Answer")}, {LINEFORWARDMODE_NOANSWINTERNAL, _T("Internal (No Answer)")}, {LINEFORWARDMODE_NOANSWEXTERNAL, _T("External (No Answer)")}, {LINEFORWARDMODE_BUSYNA, _T("Busy/No Answer")}, {LINEFORWARDMODE_BUSYNAINTERNAL, _T("Internal (Busy/NA")}, {LINEFORWARDMODE_BUSYNAEXTERNAL, _T("External (Busy/NA")}, {EPHONEEXELINEFORWARDMODE_DND, _T("Do Not Disturb (DND)")},
	};

	int iCurSel = m_cbAddress.GetCurSel();
	if (iCurSel == CB_ERR)
	{
		GetDlgItem(IDC_CALLER)->EnableWindow(FALSE);
		return;
	}

	CTapiAddress* pAddr = (CTapiAddress*)m_cbAddress.GetItemData(iCurSel);

	CString strFwdMode;
	iCurSel = m_cbFwdModes.GetCurSel();
	if (iCurSel != CB_ERR)
		m_cbFwdModes.GetLBText(iCurSel, strFwdMode);
	m_cbFwdModes.ResetContent();

	LINEADDRESSCAPS* lpCaps = pAddr->GetAddressCaps(0, 0, TRUE);
	if (lpCaps == NULL)
		return;

	m_iMinRings = lpCaps->dwMinFwdNumRings;
	m_iMaxRings = lpCaps->dwMaxFwdNumRings;
	GetDlgItem(IDC_NUMRINGS)->EnableWindow(m_iMinRings == 0 && m_iMaxRings == 0);

	m_dwAvailFwdModes = lpCaps->dwForwardModes;

	for (int i = 0; i < (sizeof(FwdModes) / sizeof(FwdModes[0])); i++)
	{
		bool bAddEntry = false;
		if (m_bSupportsAllModes)
		{
			// Support all entries by holding control on dlg open
			bAddEntry = true;
		}
		else if (FwdModes[i].dwFwdMode == EPHONEEXELINEFORWARDMODE_DND)
		{
			// DND is supported on the current address
			if (lpCaps->dwAddressFeatures & LINEADDRFEATURE_FORWARDDND)
				bAddEntry = true;
		}
		else if (lpCaps->dwForwardModes & FwdModes[i].dwFwdMode)
		{
			// The mode we are currently handling is supported by the tapi driver on the given address
			bAddEntry = true;
		}

		if (bAddEntry)
		{
			iCurSel = m_cbFwdModes.AddString(FwdModes[i].pszName);
			m_cbFwdModes.SetItemData(iCurSel, FwdModes[i].dwFwdMode);
		}
	}

	if (m_cbFwdModes.GetCount() == 0)
	{
		if (m_cbAddress.GetCount() == 0)
			m_cbFwdModes.AddString(L"Line does not support forwarding");
		else
			m_cbFwdModes.AddString(L"Address does not support forwarding");
		m_cbFwdModes.EnableWindow(FALSE);
	}
	else
		m_cbFwdModes.EnableWindow(TRUE);

	iCurSel = CB_ERR;
	if (!strFwdMode.IsEmpty())
		iCurSel = m_cbFwdModes.SelectString(-1, strFwdMode);
	if (iCurSel == CB_ERR)
		m_cbFwdModes.SetCurSel(0);

	OnChange();
}