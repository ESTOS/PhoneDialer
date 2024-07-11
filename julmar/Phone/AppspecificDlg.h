#if !defined(AFX_APPSPECIFICDLG_H__9D3422CF_7204_40D1_A320_FE4DFFEE692A__INCLUDED_)
#define AFX_APPSPECIFICDLG_H__9D3422CF_7204_40D1_A320_FE4DFFEE692A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AppspecificDlg.h : header file
//

#include "resource.h"

/////////////////////////////////////////////////////////////////////////////
// CAppspecificDlg dialog

class CAppspecificDlg : public CDialog
{
	// Construction
public:
	CAppspecificDlg(CWnd* pParent = NULL); // standard constructor

	DWORD m_dwData;
	// Dialog Data
	//{{AFX_DATA(CAppspecificDlg)
	enum
	{
		IDD = IDD_APPSPECIFIC
	};
	// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAppspecificDlg)
protected:
	virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support
													 //}}AFX_VIRTUAL

	// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CAppspecificDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_APPSPECIFICDLG_H__9D3422CF_7204_40D1_A320_FE4DFFEE692A__INCLUDED_)
