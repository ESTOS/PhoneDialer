// MakeCallExtended.cpp : implementation file
//

#include "stdafx.h"
#include "Phone.h"
#include "MakeCallExtended.h"


// MakeCallExtended dialog

IMPLEMENT_DYNAMIC(MakeCallExtended, CDialog)

MakeCallExtended::MakeCallExtended(CWnd* pParent /*=NULL*/)
	: CDialog(MakeCallExtended::IDD, pParent)
{

}

MakeCallExtended::~MakeCallExtended()
{
}

void MakeCallExtended::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(MakeCallExtended, CDialog)
END_MESSAGE_MAP()


// MakeCallExtended message handlers

void MakeCallExtended::OnOK()
{
	// TODO: Add your specialized code here and/or call the base class
	GetDlgItemText(IDC_EDITCALLINGID, m_strCallingID);
	m_bBlockMyCallingID = IsDlgButtonChecked(IDC_CHECKBLOCKID) == BST_CHECKED;
    m_bSetDestinationAutoOffHook = IsDlgButtonChecked(IDC_CHECKDESTAUTOOFFHOOK) == BST_CHECKED;

	CDialog::OnOK();
}

BOOL MakeCallExtended::OnInitDialog()
{
	CDialog::OnInitDialog();
	SetDlgItemText(IDC_EDITCALLEDID, m_strCalledID);

	if (dwAddressCapsFlags & LINEADDRCAPFLAGS_SETCALLINGID)
	{
		SetDlgItemText(IDC_EDITCALLINGID, m_strCallingID);
	}
	else
	{
		GetDlgItem(IDC_EDITCALLINGID)->EnableWindow(FALSE);
	}
	

	if (dwAddressCapsFlags & LINEADDRCAPFLAGS_BLOCKIDOVERRIDE)
	{
		CheckDlgButton(IDC_CHECKBLOCKID, m_bBlockMyCallingID ? BST_CHECKED: BST_UNCHECKED);
	}
	else
	{
		GetDlgItem(IDC_CHECKBLOCKID)->EnableWindow(FALSE);
	}
	
    if (dwAddressCapsFlags & LINEADDRCAPFLAGS_DESTOFFHOOK)
    {
        CheckDlgButton(IDC_CHECKDESTAUTOOFFHOOK, m_bSetDestinationAutoOffHook ? BST_CHECKED : BST_UNCHECKED);
    }
    else
    {
        GetDlgItem(IDC_CHECKDESTAUTOOFFHOOK)->EnableWindow(FALSE);
    }

	// TODO:  Add extra initialization here

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}
