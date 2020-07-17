// ECallFeatures.cpp : implementation file
//

#include "stdafx.h"
#include "Phone.h"
#include "ECallFeatures.h"


// ECallFeatures dialog

IMPLEMENT_DYNAMIC(ECallFeatures, CDialog)

ECallFeatures::ECallFeatures(CWnd* pParent /*=NULL*/)
	: CDialog(ECallFeatures::IDD, pParent)
{

}

ECallFeatures::~ECallFeatures()
{
}

void ECallFeatures::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(ECallFeatures, CDialog)
END_MESSAGE_MAP()

void ECallFeatures::SetCallFeatures(const CString& strCallFeatures)
{
	SetDlgItemText(IDC_CALLFEATURES, strCallFeatures);
}

// ECallFeatures message handlers

