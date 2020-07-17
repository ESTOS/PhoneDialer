// AppspecificDlg.cpp : implementation file
//

#include "stdafx.h"
#include "phone.h"
#include "AppspecificDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAppspecificDlg dialog


CAppspecificDlg::CAppspecificDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CAppspecificDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAppspecificDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_dwData = 0;
}


void CAppspecificDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAppspecificDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAppspecificDlg, CDialog)
	//{{AFX_MSG_MAP(CAppspecificDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAppspecificDlg message handlers

BOOL CAppspecificDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	CString strData;
	strData.Format(_T("%08lX"), m_dwData);

	SetDlgItemText(IDC_TXTDATA, strData);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CAppspecificDlg::OnOK() 
{
	CString strData;
	GetDlgItemText(IDC_TXTDATA, strData);

	if (strData.GetLength() > 8)
	{
		MessageBox(_T("Please enter a valid hexadecimal Number"));
		return;
	}
	const TCHAR* szTemp = (LPCTSTR)strData;
	while (*szTemp)
	{
		if (!((*szTemp >= '0' && *szTemp <= '9') || (*szTemp >= 'a' && *szTemp <= 'f') || (*szTemp >= 'A' && *szTemp <= 'F')))
		{
			MessageBox(_T("Please enter a valid hexadecimal Number"));	
			return;
		}
		szTemp++;
	}
	
	try
	{
		_stscanf(strData, _T("%08lX"), &m_dwData);
	}
	catch(...)
	{
		//MessageBox(_T("Please enter a valid Number"));
		return;
	}
	
	
	CDialog::OnOK();
}
