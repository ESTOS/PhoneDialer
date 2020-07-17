// DialDlg.h : header file
//
// This is a part of the TAPI Applications Classes C++ library.
// Original Copyright � 1995-2004 JulMar Entertainment Technology, Inc. All rights reserved.
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

#if !defined(AFX_DIALDLG_H__A776CEC6_C449_11D1_BBA4_006097D5EC19__INCLUDED_)
#define AFX_DIALDLG_H__A776CEC6_C449_11D1_BBA4_006097D5EC19__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

/////////////////////////////////////////////////////////////////////////////
// CDialDlg dialog

class CDialDlg : public CDialog
{
// Construction
public:
	CDialDlg(CWnd* pParent, CTapiLine* pLine, DWORD dwFeature);

// Dialog Data
	//{{AFX_DATA(CDialDlg)
	enum { IDD = IDD_DIAL };
	CEdit	m_edtNumber;
	CButton	m_btnOK;
	CButton	m_btnTranslate;
	CString	m_strNumber;
	CTapiLine* m_pLine;
	DWORD m_dwFeature;
	CString	m_strInstr;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDialDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDialDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnChangeNumber();
	afx_msg void OnTranslate();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DIALDLG_H__A776CEC6_C449_11D1_BBA4_006097D5EC19__INCLUDED_)
