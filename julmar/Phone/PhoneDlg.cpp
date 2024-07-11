// PhoneDlg.cpp : implementation file
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
#include "PhoneDlg.h"
#include "..\ATAPI\include\ecstaext.h"
#include "AgentStateDlg.h"
#include "AppspecificDlg.h"
#include "CallDataDlg.h"
#include "CompleteCall.h"
#include "CompleteTransferDlg.h"
#include "ConfListDlg.h"
#include "DialDlg.h"
#include "ECallFeatures.h"
#include "ECSTAAgentGroupStateDlg.h"
#include "ForwardDlg.h"
#include "ForwardList.h"
#include "ISDNInfo.h"
#include "MakeCallExtended.h"
#include "Phone.h"
#include "PhoneCapsDlg.h"
#include "PredDialDlg.h"
#include "QOSInfo.h"
#include "SetupConfDlg.h"
#include "TransferDlg.h"
#include "UUIDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Globals

CMainApp theApp;
DWORD g_dwLastError = 0;
const UINT IDT_TIMER = 100;
const UINT IDT_TIMER_REPOSITION = 101;

/////////////////////////////////////////////////////////////////////////////
// CMyLine
//
// Override of the CTapiLine for notifications
//
class CMyLine : public CTapiLine
{
	DECLARE_DYNCREATE(CMyLine)
protected:
	virtual void OnDynamicCreate()
	{
		CTapiLine::OnDynamicCreate();
		::PostMessage(theApp.m_hWndMain, UM_DYNAMICCREATE, (WPARAM)m_dwDeviceID, NULL);
	}
	virtual void OnDynamicRemove()
	{
		CTapiLine::OnDynamicRemove();
		::PostMessage(theApp.m_hWndMain, UM_DYNAMICREMOVE, (WPARAM)m_dwDeviceID, NULL);
	}

	virtual void OnAddressStateChange(DWORD dwAddressID, DWORD dwState)
	{
		CTapiLine::OnAddressStateChange(dwAddressID, dwState);
		::SendMessage(theApp.m_hWndMain, UM_ADDRESSCHANGE, (WPARAM)GetAddress(dwAddressID), (LPARAM)dwState);
	}

	virtual void OnAgentStateChange(DWORD dwAddressID, DWORD dwFields, DWORD dwState)
	{
		CTapiLine::OnAgentStateChange(dwAddressID, dwFields, dwState);
		::SendMessage(theApp.m_hWndMain, UM_AGENTCHANGE, (WPARAM)GetAddress(dwAddressID), dwFields);
	}

	virtual void OnDeviceStateChange(DWORD dwDeviceState, DWORD dwStateDetail1, DWORD dwStateDetail2)
	{
		CTapiLine::OnDeviceStateChange(dwDeviceState, dwStateDetail1, dwStateDetail2);
		::SendMessage(theApp.m_hWndMain, UM_LINECHANGE, (WPARAM)this, NULL);
	}
	virtual void OnDevSpecific(DWORD dwHandle, DWORD dwParam1, DWORD dwParam2, DWORD dwParam3)
	{
		CTapiLine::OnDevSpecific(dwHandle, dwParam1, dwParam2, dwParam3);
		if (dwParam1 == ECSTADEVSPEC_MESSAGEWAITINGCOUNTER)
		{
			::SendMessage(theApp.m_hWndMain, UM_MSGWAIT, (WPARAM)this, dwParam2);
		}
		else if (dwParam1 == ECSTADEVSPEC_ABSENTMESSAGE)
		{
			::SendMessage(theApp.m_hWndMain, UM_ABSENTMESSAGE, (WPARAM)this, dwParam2);
		}
		else if (dwParam1 == ECSTADEVSPEC_ACTIVEADDRESSCHANGED)
		{
			::SendMessage(theApp.m_hWndMain, UM_ACTIVEADDRESS, (WPARAM)this, dwParam2);
		}
		else if (dwParam1 == ECSTADEVSPEC_AGENTGROUPSCHANGED || dwParam1 == ECSTA150_AGENTSTATE)
		{
			EAgentStateEvent event;
			event.m_pLine = this;
			event.m_dwParam1 = dwParam1;
			event.m_dwParam2 = dwParam2;
			event.m_dwParam3 = dwParam3;
			::SendMessage(theApp.m_hWndMain, UM_ECSTAAGENTEVENT, (WPARAM)&event, NULL);
		}
	}

	virtual void OnNewCall(CTapiCall* pCall)
	{
		CTapiLine::OnNewCall(pCall);
		::SendMessage(theApp.m_hWndMain, UM_NEWCALL, (WPARAM)pCall, NULL);
	}
};

/////////////////////////////////////////////////////////////////////////////
// CMyCall
//
// Override of the CTapiCall for notifications
//
class CMyCall : public CTapiCall
{
	DECLARE_DYNCREATE(CMyCall)
public:
	virtual void OnInfoChange(DWORD dwInfoState)
	{
		CTapiCall::OnInfoChange(dwInfoState);

		CWnd* pwnd = theApp.m_pMainWnd;
		if (pwnd != NULL && IsWindow(pwnd->GetSafeHwnd()))
		{
			if (dwInfoState & LINECALLINFOSTATE_USERUSERINFO)
				pwnd->PostMessage(UM_FLASHWINDOW, IDC_USERUSERINFO);
			if (dwInfoState & (LINECALLINFOSTATE_HIGHLEVELCOMP | LINECALLINFOSTATE_LOWLEVELCOMP | LINECALLINFOSTATE_CHARGINGINFO))
				pwnd->PostMessage(UM_FLASHWINDOW, IDC_ISDNINFO);
			if (dwInfoState & LINECALLINFOSTATE_QOS)
				pwnd->PostMessage(UM_FLASHWINDOW, IDC_QOS);
			if (dwInfoState & LINECALLINFOSTATE_CALLDATA)
				pwnd->PostMessage(UM_FLASHWINDOW, IDC_CALLDATA);
			((CPhoneDlg*)pwnd)->SendMessage(UM_CALLCHANGE, 0, (LPARAM)(CTapiCall*)this);
		}
	}

	virtual void OnStateChange(DWORD dwState, DWORD dwStateDetail, DWORD dwPrivilage)
	{
		CTapiCall::OnStateChange(dwState, dwStateDetail, dwPrivilage);
		::SendMessage(theApp.m_hWndMain, UM_CALLCHANGE, 0, (LPARAM)(CTapiCall*)this);
	}

	virtual void OnMediaModeChange(DWORD dwMediaMode)
	{
		CTapiCall::OnMediaModeChange(dwMediaMode);
		::SendMessage(theApp.m_hWndMain, UM_CALLCHANGE, 0, (LPARAM)(CTapiCall*)this);
	}
};

IMPLEMENT_DYNCREATE(CMyLine, CTapiLine)
IMPLEMENT_DYNCREATE(CMyCall, CTapiCall)
IMPLEMENT_DYNCREATE(CMyPhone, CTapiPhone)

/////////////////////////////////////////////////////////////////////////////
// CMainApp::InitInstance
//
// Initialize the main application
//
BOOL CMainApp::InitInstance()
{
#if _MSC_VER < 1300
	// Enable 3d controls
	Enable3dControlsStatic();
#endif

	m_hWndMain = NULL;
	// Initialize a connection with TAPI and determine if there
	// are any TAPI complient devices installed.
	if (GetTAPIConnection()->Init(_T("JPhoneDialer"), RUNTIME_CLASS(CMyLine), NULL, RUNTIME_CLASS(CMyCall), RUNTIME_CLASS(CMyPhone)) != 0 || GetTAPIConnection()->GetLineDeviceCount() == 0)
	{
		AfxMessageBox(_T("There are no TAPI devices installed!"));
		return FALSE;
	}

	GetFileVersionString(_T("FileVersion"), m_strVersion);

	// Create our dialog.
	CPhoneDlg dlg;
	dlg.m_strCommandLine = m_lpCmdLine;
	m_pMainWnd = &dlg;
	dlg.DoModal();

	GetTAPIConnection()->Shutdown();

	return FALSE;

} // CMainApp::InitInstance

BOOL CMainApp::GetFileVersionString(const TCHAR* szItemFromVersionToQuery, CString& strStringFromVersionInfo)
{
	BOOL bRetCode = FALSE;

	TCHAR* szTempPath = NULL;
	szTempPath = new TCHAR[_MAX_FNAME];
	if (GetModuleFileName(theApp.m_hInstance, szTempPath, _MAX_FNAME))
	{
		DWORD dwVerHnd;
		DWORD dwVerInfoSize = GetFileVersionInfoSize(szTempPath, &dwVerHnd);

		if (dwVerInfoSize)
		{
			LPTSTR lpstrVffInfo;
			HANDLE hMem;
			hMem = GlobalAlloc(GMEM_MOVEABLE, dwVerInfoSize);
			lpstrVffInfo = (LPTSTR)GlobalLock(hMem);

			if (GetFileVersionInfo(szTempPath, dwVerHnd, dwVerInfoSize, lpstrVffInfo))
			{
				TCHAR szGetName[256];
				LPTSTR lpVersion;
				UINT uVersionLen;

				// Get language id
				_tcscpy(szGetName, _T("\\VarFileInfo\\Translation"));
				if (VerQueryValue((LPVOID)lpstrVffInfo, (LPTSTR)szGetName, (void**)&lpVersion, (UINT*)&uVersionLen))
				{
					_tcscpy(szGetName, _T("\\StringFileInfo\\"));
					USHORT* pLan = (USHORT*)lpVersion;
					_stprintf_s(szGetName + _tcslen(szGetName), _countof(szGetName) - _tcslen(szGetName), _T("%.4X%.4X\\"), pLan[0], pLan[1]);

					// Get requested item
					_tcscat(szGetName, szItemFromVersionToQuery);
					if (VerQueryValue((LPVOID)lpstrVffInfo, (LPTSTR)szGetName, (void**)&lpVersion, (UINT*)&uVersionLen))
					{
						strStringFromVersionInfo = (TCHAR*)lpVersion;
						bRetCode = TRUE;
					}
				}
			}

			GlobalUnlock(hMem);
			GlobalFree(hMem);
		}
	}

	if (szTempPath)
		delete szTempPath;

	return bRetCode;
}

BOOL CAboutDlg::OnInitDialog()
{
	CString strHeader;
	GetDlgItemText(IDC_STATIC_ABOUT, strHeader);
	strHeader += _T(" ");
	strHeader += theApp.m_strVersion;
	SetDlgItemText(IDC_STATIC_ABOUT, strHeader);
	return __super::OnInitDialog();
}

/////////////////////////////////////////////////////////////////////////////
// Message maps (required by MFC)
//
BEGIN_MESSAGE_MAP(CPhoneDlg, CDialog)
	//{{AFX_MSG_MAP(CPhoneDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_CBN_SELCHANGE(IDC_LINES, OnChangeLine)
	ON_BN_CLICKED(IDC_STARTSESSION, OnStartSession)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SESSIONSPIN, OnDeltaposSession)
	ON_CBN_SELCHANGE(IDC_ADDRESS, OnAddressChange)
	ON_NOTIFY(UDN_DELTAPOS, IDC_CALLSPIN, OnChangeCall)
	ON_MESSAGE(UM_CALLCHANGE, i_OnCallChange)
	ON_MESSAGE(UM_NEWCALL, i_OnNewCall)
	ON_MESSAGE(UM_ADDRESSCHANGE, i_OnAddressChange)
	ON_MESSAGE(UM_LINECHANGE, i_OnLineChange)
	ON_MESSAGE(UM_MSGWAIT, i_OnLineMsgWaitChange)
	ON_MESSAGE(UM_ABSENTMESSAGE, i_OnLineAbsentMessageChange)
	ON_MESSAGE(UM_ACTIVEADDRESS, i_OnLineActiveAddressChange)

	ON_MESSAGE(UM_AGENTCHANGE, i_OnAgentChange)
	ON_MESSAGE(UM_ECSTAAGENTEVENT, i_OnECSTAAgentEvent)

	ON_MESSAGE(UM_FLASHWINDOW, OnFlashWindow)
	ON_MESSAGE(UM_DYNAMICCREATE, i_OnDynamicCreate)
	ON_MESSAGE(UM_DYNAMICREMOVE, i_OnDynamicRemove)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_DROP, OnDropCall)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_ACCEPT, OnAcceptCall)
	ON_BN_CLICKED(IDC_ANSWER, OnAnswerCall)
	ON_BN_CLICKED(IDC_MAKECALL, OnMakeCall)
	ON_BN_CLICKED(IDC_HOLD, OnHoldCall)
	ON_BN_CLICKED(IDC_SWAPHOLD, OnSwapHoldCall)
	ON_BN_CLICKED(IDC_CALLDATA, OnCallData)
	ON_BN_CLICKED(IDC_USERUSERINFO, OnUserUserInfo)
	ON_BN_CLICKED(IDC_DIAL, OnDial)
	ON_BN_CLICKED(IDC_REDIRECT, OnRedirectCall)
	ON_BN_CLICKED(IDC_UNPARK, OnUnparkCall)
	ON_BN_CLICKED(IDC_PICKUP, OnPickupCall)
	ON_BN_CLICKED(IDC_CONNECTED, OnChangeLineStatus)
	ON_BN_CLICKED(IDC_AGENTINFO, OnAgentInfo)
	ON_BN_CLICKED(IDC_ACTIVE, OnActiveAddress)
	ON_BN_CLICKED(IDC_CONFLIST, OnConferenceList)
	ON_BN_CLICKED(IDC_TRANSFER, OnTransfer)
	ON_BN_CLICKED(IDC_PARK, OnParkCall)
	ON_BN_CLICKED(IDC_COMPLETE, OnCompleteTransfer)
	ON_BN_CLICKED(IDC_COMPLETECALL, OnCompleteCall)
	ON_BN_CLICKED(IDC_SETUPCONF, OnSetupConference)
	ON_BN_CLICKED(IDC_ADDTOCONF, OnAddToConference)
	ON_BN_CLICKED(IDC_REMOVEFROMCONF, OnRemoveFromConference)
	ON_BN_CLICKED(IDC_PHONEINFO, OnPhoneInfo)
	ON_BN_CLICKED(IDC_CONFIG, OnConfig)
	ON_BN_CLICKED(IDC_CMDAPPSPECIFIC, OnAppspecific)
	ON_BN_CLICKED(IDC_CMDCALLRECORD, OnCallRecording)
	ON_WM_CLOSE()
	ON_EN_CHANGE(IDC_PHONENUMBER, OnAddressChange)
	ON_BN_CLICKED(IDC_INSERVICE, OnChangeLineStatus)
	ON_BN_CLICKED(IDC_MESSAGEWAITING, OnChangeLineStatus)
	ON_BN_CLICKED(IDC_LOCKED, OnChangeLineStatus)
	ON_BN_CLICKED(IDC_ISDNINFO, OnISDN)
	ON_BN_CLICKED(IDC_QOS, OnQos)
	ON_BN_CLICKED(IDC_FORWARD, OnForward)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_FEATURES, OnBnClickedFeatures)
	ON_BN_CLICKED(IDC_COMPLETECONFERENCE, &CPhoneDlg::OnBnClickedCompleteAsConference)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// LookupError
//
// Return a string representing a TAPI error
//
LPCTSTR LookupError(LONG lResult)
{
	static TCHAR szBuff[1024];

	if (lResult == 0xffffffff)
		_tcscpy(szBuff, _T("Request timeout"));
	else if (lResult > 0x80000000)
	{
		if (lResult >= 0x80000000 && lResult < 0x90000000)
		{
			lResult &= ~0x80000000;
			lResult |= 0x8000;
		}
		else
		{
			lResult &= ~0x90000000;
			lResult |= 0x9000;
		}
		if (LoadString(AfxGetResourceHandle(), (UINT)lResult, szBuff, 1024) == 0)
			_tcscpy(szBuff, _T("Unknown Error"));
	}
	else
	{
		_tcscpy(szBuff, _T("Unknown Error"));
	}

	return szBuff;

} // LookupError

/////////////////////////////////////////////////////////////////////////////
// ErrorMsg
//
// Show an error message on the status line.
//
void ErrorMsg(LPCSTR pszBuff, ...)
{
	va_list args;
	static char szBuff[1024];

	va_start(args, pszBuff);
	vsprintf(szBuff, pszBuff, args);
	va_end(args);

	CPhoneDlg* pDlg = (CPhoneDlg*)theApp.m_pMainWnd;
	if (pDlg->GetSafeHwnd() && ::IsWindow(pDlg->GetSafeHwnd()))
	{
		::SetDlgItemTextA(pDlg->m_hWnd, IDC_STATUS, szBuff);
		g_dwLastError = GetTickCount();
	}

} // ErrorMsg

void ErrorMsg(LPCWSTR pszBuff, ...)
{
	va_list args;
	static WCHAR szBuff[1024];

	va_start(args, pszBuff);
	vswprintf(szBuff, pszBuff, args);
	va_end(args);

	CPhoneDlg* pDlg = (CPhoneDlg*)theApp.m_pMainWnd;
	if (pDlg->GetSafeHwnd() && ::IsWindow(pDlg->GetSafeHwnd()))
	{
		::SetDlgItemTextW(pDlg->m_hWnd, IDC_STATUS, szBuff);
		g_dwLastError = GetTickCount();
	}

} // ErrorMsg

/////////////////////////////////////////////////////////////////////////////
// ClearErrors
//
// Clear any errors on the console
//
void ClearErrors()
{
	CPhoneDlg* pDlg = (CPhoneDlg*)theApp.m_pMainWnd;
	if (pDlg->GetSafeHwnd() && ::IsWindow(pDlg->GetSafeHwnd()))
		pDlg->SetDlgItemText(IDC_STATUS, _T(""));

} // ClearErrors

/////////////////////////////////////////////////////////////////////////////
// CPhoneDlg::CPhoneDlg
//
// Constructor for the phone dialog

CPhoneDlg::CPhoneDlg(CWnd* pParent)
	: CDialog(CPhoneDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CPhoneDlg)
	m_pForwardList = NULL;
	m_strOrigin = _T("");
	m_strCallerID = _T("");
	m_strCallerIDName = _T("");
	m_strCallID = _T("");
	m_strAppspecific = _T("");
	m_strAddressID = _T("");
	m_strRelatedCallID = _T("");
	m_strConnectedID = _T("");
	m_strConnectedIDName = _T("");
	m_strCallNumber = _T("0 of 0");
	m_strCallState = _T("");
	m_strReason = _T("");
	m_strMaxCalls = _T("");
	m_strMediaMode = _T("");
	m_fMsgWaiting = FALSE;
	m_strNumber = _T("");
	m_strRedirectedNumber = _T("");
	m_strRedirectedName = _T("");
	m_strRedirectingNumber = _T("");
	m_strRedirectingName = _T("");
	m_strSessionNum = _T("0 Open");
	m_strLastStateChangeTime = _T("00:00:00");
	SYSTEMTIME localTime;
	GetLocalTime(&localTime);
	CTime time(localTime);
	m_strCurrentTime = time.Format("%d.%m.%Y - %H:%M:%S");
	m_strTrunkID = _T("");
	m_fLocked = FALSE;
	m_iSession = 0;
	m_iCall = 0;
	m_strCalledID = _T("");
	m_strCalledIDName = _T("");
	m_fConnected = FALSE;
	m_fInService = FALSE;
	m_fPasswordFailed = FALSE;
	m_pDlgCallFeatures = NULL;
	m_pDlgCallData = NULL;
	m_pDlgECSTAAgentState = NULL;
	//}}AFX_DATA_INIT
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_bDelayDeleteCalls = false;

	m_uiTimerRefresh = 0;
	m_uiTimerReposition = 0;

} // CPhoneDlg::CPhoneDlg

/////////////////////////////////////////////////////////////////////////////
// CPhoneDlg::DoDataExchange
//
// Dialog data exchange for the phone dialog
//
void CPhoneDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPhoneDlg)
	DDX_Control(pDX, IDC_QOS, m_btnQOS);
	DDX_Control(pDX, IDC_ISDNINFO, m_btnISDN);
	DDX_Control(pDX, IDC_FEATURES, m_btnFeatures);
	DDX_Control(pDX, IDC_CONFIG, m_btnConfig);
	DDX_Control(pDX, IDC_PHONENUMBER, m_edtNumber);
	DDX_Control(pDX, IDC_CONFLIST, m_btnConfList);
	DDX_Control(pDX, IDC_PARK, m_btnPark);
	DDX_Control(pDX, IDC_COMPLETECALL, m_btnCompleteCall);
	DDX_Control(pDX, IDC_COMPLETECONFERENCE, m_btnCompleteAsConference);
	DDX_Control(pDX, IDC_SWAPHOLD, m_btnSwapHold);
	DDX_Control(pDX, IDC_ACCEPT, m_btnAccept);
	DDX_Control(pDX, IDC_ADDRESS, m_cbAddress);
	DDX_Control(pDX, IDC_USERUSERINFO, m_btnUserUserInfo);
	DDX_Control(pDX, IDC_UNPARK, m_btnUnpark);
	DDX_Control(pDX, IDC_TRANSFER, m_btnTransfer);
	DDX_Control(pDX, IDC_STARTSESSION, m_btnStartSession);
	DDX_Control(pDX, IDC_SETUPCONF, m_btnSetupConf);
	DDX_Control(pDX, IDC_SESSIONSPIN, m_ctlSession);
	DDX_Control(pDX, IDC_REMOVEFROMCONF, m_btnRemoveFromConf);
	DDX_Control(pDX, IDC_REDIRECT, m_btnRedirect);
	DDX_Control(pDX, IDC_PICKUP, m_btnPickup);
	DDX_Control(pDX, IDC_PHONEINFO, m_btnPhoneInfo);
	DDX_Control(pDX, IDC_MAKECALL, m_btnMakeCall);
	DDX_Control(pDX, IDC_FORWARD, m_btnForward);
	DDX_Control(pDX, IDC_LINES, m_cbLines);
	DDX_Control(pDX, IDC_HOLD, m_btnHold);
	DDX_Control(pDX, IDC_CMDAPPSPECIFIC, m_btnAppspecific);
	DDX_Control(pDX, IDC_CMDCALLRECORD, m_btnCallRecording);
	DDX_Control(pDX, IDC_DROP, m_btnDrop);
	DDX_Control(pDX, IDC_DIAL, m_btnDial);
	DDX_Control(pDX, IDC_COMPLETE, m_btnComplete);
	DDX_Control(pDX, IDC_CALLSPIN, m_ctlCalls);
	DDX_Control(pDX, IDC_CALLDATA, m_btnCallData);
	DDX_Control(pDX, IDC_ANSWER, m_btnAnswer);
	DDX_Control(pDX, IDC_AGENTINFO, m_btnAgentInfo);
	DDX_Control(pDX, IDC_ACTIVE, m_btnActiveAddress);

	DDX_Control(pDX, IDC_ADDTOCONF, m_btnAddToConf);
	DDX_Text(pDX, IDC_CALLDIRECTION, m_strOrigin);
	DDX_Text(pDX, IDC_CALLID, m_strCallID);
	DDX_Text(pDX, IDC_APPSPECIFIC, m_strAppspecific);
	DDX_Text(pDX, IDC_ADDRESSID, m_strAddressID);
	DDX_Text(pDX, IDC_CALLNUM, m_strCallNumber);
	DDX_Text(pDX, IDC_CALLSTATE, m_strCallState);
	DDX_Text(pDX, IDC_CALLFEATURES, m_strCallFeatures);
	DDX_Text(pDX, IDC_CALLTYPE, m_strReason);
	DDX_Text(pDX, IDC_MAXCALLS, m_strMaxCalls);
	DDX_Text(pDX, IDC_MEDIAMODE, m_strMediaMode);
	DDX_Check(pDX, IDC_MESSAGEWAITING, m_fMsgWaiting);
	DDX_Text(pDX, IDC_PHONENUMBER, m_strNumber);
	DDX_Text(pDX, IDC_SESSION, m_strSessionNum);
	DDX_Text(pDX, IDC_STATECHGTIME, m_strLastStateChangeTime);
	DDX_Text(pDX, IDC_TRUNK, m_strTrunkID);
	DDX_Text(pDX, IDC_STATICTIME, m_strCurrentTime);
	DDX_Check(pDX, IDC_LOCKED, m_fLocked);

	DDX_Text(pDX, IDC_CALLERNUM, m_strCallerID);
	DDX_Text(pDX, IDC_CALLEDNUM, m_strCalledID);
	DDX_Text(pDX, IDC_CONNECTEDNUM, m_strConnectedID);
	DDX_Text(pDX, IDC_REDIRECTEDNUM, m_strRedirectedNumber);
	DDX_Text(pDX, IDC_REDIRECTINGNUM, m_strRedirectingNumber);
	DDX_Text(pDX, IDC_CALLERNAME, m_strCallerIDName);
	DDX_Text(pDX, IDC_CALLEDNAME, m_strCalledIDName);
	DDX_Text(pDX, IDC_CONNECTEDNAME, m_strConnectedIDName);
	DDX_Text(pDX, IDC_REDIRECTEDNAME, m_strRedirectedName);
	DDX_Text(pDX, IDC_REDIRECTINGNAME, m_strRedirectingName);

	DDX_Check(pDX, IDC_CONNECTED, m_fConnected);
	DDX_Check(pDX, IDC_INSERVICE, m_fInService);
	DDX_Check(pDX, IDC_PASSWORDFAILED, m_fPasswordFailed);

	DDX_Text(pDX, IDC_RELCALLID, m_strRelatedCallID);
	//}}AFX_DATA_MAP

} // CPhoneDlg::DoDataExchange

/////////////////////////////////////////////////////////////////////////////
// CPhoneDlg::OnInitDialog
//
// Process the initial dialog
//

class EWindowListHelper : public std::list<HWND>
{
public:
	EWindowListHelper()
	{
		m_hMyHWND = 0;
	}
	CString m_strMyCaption;
	HWND m_hMyHWND;
};

BOOL CALLBACK EnumWindowsProc(HWND hWnd, LPARAM lParam)
{
	if (!hWnd)
		return TRUE; // Not a window
	if (!::IsWindowVisible(hWnd))
		return TRUE; // Not visible

	TCHAR String[255] = {0};
	if (!SendMessage(hWnd, WM_GETTEXT, _countof(String) - 1, (LPARAM)String))
		return TRUE; // No window title

	EWindowListHelper* pWndList = (EWindowListHelper*)lParam;
	if (pWndList->m_hMyHWND != hWnd)
	{
		if (pWndList->m_strMyCaption == String)
			pWndList->push_back(hWnd);
	}

	return TRUE;
}

BOOL CPhoneDlg::OnInitDialog()
{
	theApp.m_hWndMain = m_hWnd;
	CString strCaption;

#ifdef _WIN64
	GetWindowText(strCaption);
	strCaption += " X64";
	SetWindowText(strCaption);
#endif
	if (!theApp.m_strVersion.IsEmpty())
	{
		GetWindowText(strCaption);
		strCaption += " - ";
		strCaption += theApp.m_strVersion;
		SetWindowText(strCaption);
	}

	// Reset the font to all be ANSI var.
	CFont fntAnsi;
	fntAnsi.CreateStockObject(ANSI_VAR_FONT);

	LOGFONT lf;
	fntAnsi.GetObject(sizeof(LOGFONT), &lf);
	if (lf.lfHeight != 0)
		lf.lfHeight--;
	lf.lfWeight = FW_BOLD;
	m_fntBold.CreateFontIndirect(&lf);

	CWnd* pwndChild = GetWindow(GW_CHILD);
	while (pwndChild != NULL && IsChild(pwndChild))
	{
		pwndChild->SetFont(&fntAnsi);
		pwndChild = pwndChild->GetWindow(GW_HWNDNEXT);
	}

	// Connect all the controls via DDX
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.
	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);
	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);	 // Set big icon
	SetIcon(m_hIcon, FALSE); // Set small icon

	m_ctlSession.SetRange(0, 0);

	// Load our line devices with all the detected TAPI lines.
	FillLineList();

	// Set our interval timer
	m_uiTimerRefresh = SetTimer(IDT_TIMER, 1000, NULL);

	RepositionWindow();

	return TRUE;

} // CPhoneDlg::OnInitDialog

void CPhoneDlg::FillLineList()
{
	// Remember out selected line.
	int iOldPos = m_cbLines.GetCurSel();
	m_cbLines.ResetContent();
	bool bCommandLineFoundLine = false;

	// Load our line devices with all the detected TAPI lines.
	for (DWORD dwLine = 0; dwLine < GetTAPIConnection()->GetLineDeviceCount(); dwLine++)
	{
		CTapiLine* pLine = GetTAPIConnection()->GetLineFromDeviceID(dwLine);
		if (pLine != NULL)
		{
			if (pLine->IsValid())
			{
				int iPos = m_cbLines.AddString(pLine->GetLineName());
				if (!m_strCommandLine.IsEmpty() && pLine->GetLineName().Find(m_strCommandLine) != -1)
				{
					bCommandLineFoundLine = true;
					m_cbLines.SetCurSel(iPos);
				}
				ASSERT(iPos != CB_ERR);
				m_cbLines.SetItemDataPtr(iPos, pLine);
			}
			else
			{
				RemoveSession(pLine);
			}
		}
	}

	if (bCommandLineFoundLine)
	{
		m_btnStartSession.EnableWindow(TRUE);
		OnChangeLine();
		::SendMessage(m_hWnd, WM_COMMAND, MAKELONG(IDC_STARTSESSION, BN_CLICKED), (LPARAM)m_btnStartSession.m_hWnd);
	}
	else
	{
		// Select the first line
		if (m_cbLines.GetCount() > 0)
		{
			if (iOldPos != -1 && iOldPos < m_cbLines.GetCount())
				m_cbLines.SetCurSel(iOldPos);
			else
				m_cbLines.SetCurSel(0);
			m_btnStartSession.EnableWindow(TRUE);
			OnChangeLine();
		}
	}
	m_strCommandLine = _T("");
}
/////////////////////////////////////////////////////////////////////////////
// CPhoneDlg::OnSysCommand
//
// Process the WM_SYSCOMMAND message
//
void CPhoneDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
		CDialog::OnSysCommand(nID, lParam);

} // CPhoneDlg::OnSysCommand

/////////////////////////////////////////////////////////////////////////////
// CPhoneDlg::OnPaint
//
// Paint the minimized window
//
void CPhoneDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM)dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
		CDialog::OnPaint();

} // CPhoneDlg::OnPaint

/////////////////////////////////////////////////////////////////////////////
// CPhoneDlg::OnQueryDragIcon
//
// Return the icon to drag
//
HCURSOR CPhoneDlg::OnQueryDragIcon()
{
	return (HCURSOR)m_hIcon;

} // CPhoneDlg::OnQueryDragIcon

/////////////////////////////////////////////////////////////////////////////
// CPhoneDlg::OnChangeLine
//
// This is called when the user selects a new line in our combo box.
//
void CPhoneDlg::OnChangeLine()
{
	// Remove any previous line errors
	ClearErrors();

	CTapiLine* pLine = GetActiveLine();
	if (pLine != NULL)
	{
		UpdateSession(pLine);

		if (!pLine->IsValid())
		{
			RemoveSession(pLine);
			AfxMessageBox(_T("The line was removed by TAPI."));
			m_cbLines.DeleteString(m_cbLines.GetCurSel());
			m_cbLines.SetCurSel(0);
			OnChangeLine();
			return;
		}

		bool bUpdateAdresses = true;

		if (m_cbAddress.GetCount() == (int)pLine->GetAddressCount())
		{
			bUpdateAdresses = false;

			std::list<CTapiAddress*> before, after;
			for (int iCount = 0; iCount < m_cbAddress.GetCount(); iCount++)
				before.push_back((CTapiAddress*)m_cbAddress.GetItemDataPtr(iCount));

			for (DWORD dwAddress = 0; dwAddress < pLine->GetAddressCount(); dwAddress++)
				after.push_back(pLine->GetAddress(dwAddress));
			if (before != after)
				bUpdateAdresses = true;
		}

		if (bUpdateAdresses)
		{
			m_cbAddress.GetCount();
			// Load the address information
			m_cbAddress.ResetContent();
			for (DWORD dwAddress = 0; dwAddress < pLine->GetAddressCount(); dwAddress++)
			{
				CTapiAddress* pAddr = pLine->GetAddress(dwAddress);
				CString strName = pAddr->GetDialableAddress();
				if (strName.IsEmpty())
					strName.Format(_T("Address %ld"), dwAddress);
				else
					strName.Format(_T("%s - ID %ld"), (const wchar_t*)pAddr->GetDialableAddress(), dwAddress);

				ECSTAAddressCaps addressCaps;
				if (pAddr->GetECSTAAddressCaps(addressCaps) == NO_ERROR)
					strName += addressCaps.getDisplayText();

				int iPos = m_cbAddress.AddString(strName);
				ASSERT(iPos != CB_ERR);
				m_cbAddress.SetItemDataPtr(iPos, pAddr);
			}

			// Select the first address by default.
			m_cbAddress.SetCurSel(0);
			OnAddressChange();
		}

		// If the line is open, load all our info
		if (pLine->IsOpen())
		{
			m_arrCalls.RemoveAll();
			OnLoadLineInfo(pLine);
		}
		else
			NoLineSelected();
	}

} // CPhoneDlg::OnChangeLine

/////////////////////////////////////////////////////////////////////////////
// CPhoneDlg::OnLoadLineInfo
//
// This loads all the line information for a newly picked line
//
void CPhoneDlg::OnLoadLineInfo(CTapiLine* pLine)
{
	m_btnStartSession.SetWindowText(_T("End Session"));
	m_btnConfig.EnableWindow(TRUE);
	m_edtNumber.EnableWindow(TRUE);
	m_btnPhoneInfo.EnableWindow(pLine->GetRelatedPhoneID() != 0xffffffff);

	// Set our checkflags
	LPLINEDEVSTATUS lpStatus = pLine->GetLineStatus();
	if (lpStatus == NULL)
		return;

	// Set our flags
	m_fMsgWaiting = ((lpStatus->dwDevStatusFlags & LINEDEVSTATUSFLAGS_MSGWAIT) != 0);
	m_fLocked = ((lpStatus->dwDevStatusFlags & LINEDEVSTATUSFLAGS_LOCKED) != 0);
	m_fConnected = ((lpStatus->dwDevStatusFlags & LINEDEVSTATUSFLAGS_CONNECTED) != 0);
	m_fInService = ((lpStatus->dwDevStatusFlags & LINEDEVSTATUSFLAGS_INSERVICE) != 0);
	m_fPasswordFailed = pLine->GetECSTAInvalidPasswordState() ? TRUE : FALSE;

	GetECSTALineSpecific();

	// Enable/Disable the "settable" items.
	LPLINEDEVCAPS lpCaps = pLine->GetLineCaps();
	if (lpCaps)
	{
		GetDlgItem(IDC_CONNECTED)->EnableWindow((lpCaps->dwSettableDevStatus & LINEDEVSTATUSFLAGS_CONNECTED) != 0);
		GetDlgItem(IDC_INSERVICE)->EnableWindow((lpCaps->dwSettableDevStatus & LINEDEVSTATUSFLAGS_INSERVICE) != 0);
		GetDlgItem(IDC_LOCKED)->EnableWindow((lpCaps->dwSettableDevStatus & LINEDEVSTATUSFLAGS_LOCKED) != 0);
		GetDlgItem(IDC_MESSAGEWAITING)->EnableWindow((lpCaps->dwSettableDevStatus & LINEDEVSTATUSFLAGS_MSGWAIT) != 0);
		GetDlgItem(IDC_PASSWORDFAILED)->EnableWindow(FALSE);
	}

	// Grab all the call information on this line.
	CObList callList;
	pLine->GetNewCalls(callList);

	// Now add any other calls.
	if (m_arrCalls.GetSize() != pLine->GetCallCount())
	{
		m_arrCalls.RemoveAll();
		for (int iCall = 0; iCall < pLine->GetCallCount(); iCall++)
			OnNewCall(pLine->GetCall(iCall));
	}
	else
	{
		int iCall = 0;
		for (iCall = 0; iCall < pLine->GetCallCount(); iCall++)
		{
			CTapiCall* pCall = pLine->GetCall(iCall);
			if (pCall == GetActiveCall())
			{
				MoveToCall(iCall);
				break;
			}
		}

		if (iCall == pLine->GetCallCount())
			MoveToCall(0);
	}
	UpdateData(FALSE);

} // CPhoneDlg::OnLoadLineInfo

/////////////////////////////////////////////////////////////////////////////
// CPhoneDlg::MoveToCall
//
// Change to a given call
//
void CPhoneDlg::MoveToCall(int iCall)
{
	m_strOrigin = _T("");
	m_strCalledID = _T("");
	m_strCalledIDName = _T("");
	m_strCallerID = _T("");
	m_strCallerIDName = _T("");
	m_strCallID = _T("");
	m_strAppspecific = _T("");
	m_strAddressID = _T("");
	m_strRelatedCallID = _T("");
	m_strConnectedID = _T("");
	m_strConnectedIDName = _T("");
	m_strCallNumber = _T("0 of 0");
	m_strCallState = _T("");
	m_strCallFeatures = _T("");
	m_strCallFeaturesList = _T("");
	m_strReason = _T("");
	m_strMediaMode = _T("");
	m_strRedirectedNumber = _T("");
	m_strRedirectedName = _T("");
	m_strRedirectingNumber = _T("");
	m_strRedirectingName = _T("");
	m_strLastStateChangeTime = _T("00:00:00");
	m_strTrunkID = _T("");
	m_strCallNumber = _T("0 of 0");

	EnableCallButtons(0, 0, 0, 0);
	m_btnUserUserInfo.EnableWindow(FALSE);
	m_btnCallData.EnableWindow(FALSE);
	m_btnQOS.EnableWindow(FALSE);
	m_btnISDN.EnableWindow(FALSE);

	if (iCall < 0 || iCall >= m_arrCalls.GetSize())
	{
		if (GetActiveCall() == NULL)
		{
			UpdateData(FALSE);
			return;
		}
		else
			MoveToCall(m_iCall);
	}

	m_iCall = iCall;
	m_strCallNumber.Format(_T("%d of %d"), m_iCall + 1, m_arrCalls.GetSize());

	CTapiCall* pCall = (CTapiCall*)m_arrCalls[m_iCall];

	m_strCallState = pCall->GetCallStateString();

	LPLINECALLINFO lpCallInfo = pCall->GetCallInfo();
	LPLINECALLSTATUS lpCallStatus = pCall->GetCallStatus();

	if (lpCallStatus)
		m_strCallFeatures.Format(_T("0x%lX"), lpCallStatus->dwCallFeatures);
	m_btnFeatures.EnableWindow(TRUE);
	m_strCallFeaturesList = pCall->GetCallFeaturesString();
	if (m_pDlgCallFeatures)
		m_pDlgCallFeatures->SetCallFeatures(m_strCallFeaturesList);
	if (m_pDlgCallData)
		m_pDlgCallData->OnCallChanged();

	if (lpCallInfo == NULL || lpCallStatus == NULL)
		return;

	m_strCallID.Format(_T("0x%lX"), lpCallInfo->dwCallID);
	m_strRelatedCallID.Format(_T("0x%lX"), lpCallInfo->dwRelatedCallID);
	m_strTrunkID.Format(_T("0x%lX"), lpCallInfo->dwTrunk);
	m_strAppspecific.Format(_T("0x%lX"), lpCallInfo->dwAppSpecific);
	m_strAddressID.Format(_T("0x%lX"), lpCallInfo->dwAddressID);

	// Determine the call origin.
	m_strOrigin.Empty();
	if (lpCallInfo->dwOrigin & LINECALLORIGIN_CONFERENCE)
		m_strOrigin = _T("Conference");
	else if (lpCallInfo->dwOrigin & LINECALLORIGIN_OUTBOUND)
		m_strOrigin = _T("Outbound");
	else if (lpCallInfo->dwOrigin & (LINECALLORIGIN_INTERNAL | LINECALLORIGIN_EXTERNAL | LINECALLORIGIN_INBOUND))
		m_strOrigin = _T("Inbound");
	else if (lpCallInfo->dwOrigin == LINECALLORIGIN_UNAVAIL)
		m_strOrigin = _T("Unavail");

	if (m_strOrigin.IsEmpty() == FALSE)
	{
		if (lpCallInfo->dwOrigin & LINECALLORIGIN_INTERNAL)
			m_strOrigin += _T("/Int");
		else if (lpCallInfo->dwOrigin & LINECALLORIGIN_EXTERNAL)
			m_strOrigin += _T("/Ext");
	}
	else
		m_strOrigin = _T("Unknown");

	// Enable our conference list button
	m_btnConfList.EnableWindow((pCall->GetCallState() == LINECALLSTATE_CONFERENCED) || (lpCallInfo->dwOrigin & LINECALLORIGIN_CONFERENCE) != 0);

	switch (lpCallInfo->dwReason)
	{
		case LINECALLREASON_DIRECT:
			m_strReason = _T("Direct");
			break;
		case LINECALLREASON_FWDBUSY:
			m_strReason = _T("FwdBusy");
			break;
		case LINECALLREASON_FWDNOANSWER:
			m_strReason = _T("FwdNoAns");
			break;
		case LINECALLREASON_FWDUNCOND:
			m_strReason = _T("FwdUnc");
			break;
		case LINECALLREASON_PICKUP:
			m_strReason = _T("Pickup");
			break;
		case LINECALLREASON_UNPARK:
			m_strReason = _T("Unpark");
			break;
		case LINECALLREASON_REDIRECT:
			m_strReason = _T("Redirect");
			break;
		case LINECALLREASON_CALLCOMPLETION:
			m_strReason = _T("CallComp");
			break;
		case LINECALLREASON_TRANSFER:
			m_strReason = _T("Transfer");
			break;
		case LINECALLREASON_REMINDER:
			m_strReason = _T("Reminder");
			break;
		case LINECALLREASON_UNAVAIL:
			m_strReason = _T("Unavail");
			break;
		case LINECALLREASON_INTRUDE:
			m_strReason = _T("Intrude");
			break;
		case LINECALLREASON_PARKED:
			m_strReason = _T("Parked");
			break;
		case LINECALLREASON_CAMPEDON:
			m_strReason = _T("CampedOn");
			break;
		case LINECALLREASON_ROUTEREQUEST:
			m_strReason = _T("Route");
			break;
		case LINECALLREASON_UNKNOWN:
		default:
			m_strReason = _T("Unknown");
			break;
	}

	DWORD dwMediaMode = lpCallInfo->dwMediaMode & ~LINEMEDIAMODE_UNKNOWN;
	switch (dwMediaMode)
	{
		case LINEMEDIAMODE_INTERACTIVEVOICE:
			m_strMediaMode = _T("Voice");
			break;
		case LINEMEDIAMODE_AUTOMATEDVOICE:
			m_strMediaMode = _T("Automated");
			break;
		case LINEMEDIAMODE_DATAMODEM:
			m_strMediaMode = _T("Modem");
			break;
		case LINEMEDIAMODE_G3FAX:
			m_strMediaMode = _T("G3FAX");
			break;
		case LINEMEDIAMODE_TDD:
			m_strMediaMode = _T("TDD");
			break;
		case LINEMEDIAMODE_G4FAX:
			m_strMediaMode = _T("G4FAX");
			break;
		case LINEMEDIAMODE_DIGITALDATA:
			m_strMediaMode = _T("DigData");
			break;
		case LINEMEDIAMODE_TELETEX:
			m_strMediaMode = _T("Teletex");
			break;
		case LINEMEDIAMODE_VIDEOTEX:
			m_strMediaMode = _T("VidTex");
			break;
		case LINEMEDIAMODE_TELEX:
			m_strMediaMode = _T("Telex");
			break;
		case LINEMEDIAMODE_MIXED:
			m_strMediaMode = _T("Mixed");
			break;
		case LINEMEDIAMODE_ADSI:
			m_strMediaMode = _T("ASDI");
			break;
		case LINEMEDIAMODE_VOICEVIEW:
			m_strMediaMode = _T("VoiceView");
			break;
		case LINEMEDIAMODE_UNKNOWN:
		default:
			m_strMediaMode = _T("Unknown");
			break;
	}

	SYSTEMTIME localTime;
	CTimeSpan tmDiff;

	if (SystemTimeToTzSpecificLocalTime(NULL, &(lpCallStatus->tStateEntryTime), &localTime))
		tmDiff = CTime::GetCurrentTime() - CTime(localTime);
	else
	{
		GetSystemTime(&localTime);
		tmDiff = CTime(localTime) - CTime(lpCallStatus->tStateEntryTime);
	}
	m_strLastStateChangeTime = tmDiff.Format(_T("%H:%M:%S"));

	// Now fill in CALLER information.
	m_strCallerIDName = pCall->GetCallerIDName();
	m_strCallerID = pCall->GetCallerIDNumber();

	m_strCalledIDName = pCall->GetCalledIDName();
	m_strCalledID = pCall->GetCalledIDNumber();

	m_strConnectedIDName = pCall->GetConnectedIDName();
	m_strConnectedID = pCall->GetConnectedIDNumber();

	m_strRedirectingName = pCall->GetRedirectingIDName();
	m_strRedirectingNumber = pCall->GetRedirectingIDNumber();

	m_strRedirectedName = pCall->GetRedirectedFromIDName();
	m_strRedirectedNumber = pCall->GetRedirectedFromIDNumber();

	if (!m_strCallerIDName.IsEmpty())
		m_strCallerIDName = _T("Name: ") + m_strCallerIDName;

	if (!m_strCalledIDName.IsEmpty())
		m_strCalledIDName = _T("Name: ") + m_strCalledIDName;

	if (!m_strConnectedIDName.IsEmpty())
		m_strConnectedIDName = _T("Name: ") + m_strConnectedIDName;

	if (!m_strRedirectedName.IsEmpty())
		m_strRedirectedName = _T("Name: ") + m_strRedirectedName;

	if (!m_strRedirectingName.IsEmpty())
		m_strRedirectingName = _T("Name: ") + m_strRedirectingName;

	// Check the transfer mode capabilities of the owner Address
	DWORD dwTransferModes = 0;
	CTapiAddress* pAddr = pCall->GetAddressInfo();
	if (pAddr)
	{
		LPLINEADDRESSCAPS lpCaps = pAddr->GetAddressCaps();
		if (lpCaps)
			dwTransferModes = lpCaps->dwTransferModes;
	}

	DWORD dwECSTACallFeatures = pCall->GetECSTACallFeatures();

	// Enable the appropriate call buttons.
	EnableCallButtons(lpCallStatus->dwCallFeatures, lpCallStatus->dwCallFeatures2, dwTransferModes, dwECSTACallFeatures);

	m_btnUserUserInfo.EnableWindow(lpCallInfo->dwUserUserInfoSize > 0 || (lpCallStatus->dwCallFeatures & (LINECALLFEATURE_SENDUSERUSER | LINECALLFEATURE_RELEASEUSERUSERINFO)));
	m_btnCallData.EnableWindow(lpCallInfo->dwCallDataSize > 0 || (lpCallStatus->dwCallFeatures & LINECALLFEATURE_SETCALLDATA));
	m_btnQOS.EnableWindow(lpCallInfo->dwReceivingFlowspecSize > 0 || lpCallInfo->dwSendingFlowspecSize > 0 || (lpCallStatus->dwCallFeatures & LINECALLFEATURE_SETQOS));
	m_btnISDN.EnableWindow(lpCallInfo->dwHighLevelCompSize > 0 || lpCallInfo->dwLowLevelCompSize > 0 || lpCallInfo->dwChargingInfoSize > 0);

	UpdateData(FALSE);

} // CPhoneDlg::MoveToCall

/////////////////////////////////////////////////////////////////////////////
// CPhoneDlg::NoLineSelected
//
// Turn off all the controls
//
void CPhoneDlg::NoLineSelected()
{
	m_btnStartSession.SetWindowText(_T("Start Session"));
	m_btnConfig.EnableWindow(FALSE);
	m_btnPhoneInfo.EnableWindow(FALSE);
	m_edtNumber.EnableWindow(FALSE);

	EnableLineButtons(0);

	NoCallSelected();

	m_strNumber = _T("");
	m_fLocked = FALSE;
	m_fMsgWaiting = FALSE;
	m_fConnected = FALSE;
	m_fInService = FALSE;
	m_fPasswordFailed = FALSE;
	GetDlgItem(IDC_MESSAGEWAITING)->SetWindowText(_T("MWI"));

	GetDlgItem(IDC_CONNECTED)->EnableWindow(FALSE);
	GetDlgItem(IDC_INSERVICE)->EnableWindow(FALSE);
	GetDlgItem(IDC_LOCKED)->EnableWindow(FALSE);
	GetDlgItem(IDC_MESSAGEWAITING)->EnableWindow(FALSE);
	GetDlgItem(IDC_PASSWORDFAILED)->EnableWindow(FALSE);

	// Turn off the call buttons.
	m_arrCalls.RemoveAll();
	m_iCall = 0;
	m_ctlCalls.SetRange(0, 0);
	UpdateData(FALSE);

} // CPhoneDlg::NoLineSelected

/////////////////////////////////////////////////////////////////////////////
// CPhoneDlg::NoCallSelected
//
// Turn off all the controls
//
void CPhoneDlg::NoCallSelected()
{
	m_strOrigin = _T("");
	m_strCalledID = _T("");
	m_strCallerID = _T("");
	m_strConnectedID = _T("");
	m_strCalledIDName = _T("");
	m_strCallerIDName = _T("");
	m_strConnectedIDName = _T("");
	m_strCallID = _T("");
	m_strAppspecific = _T("");
	m_strAddressID = _T("");
	m_strRelatedCallID = _T("");
	m_strCallNumber = _T("0 of 0");
	m_strCallState = _T("");
	m_strCallFeatures = _T("");
	m_strCallFeaturesList = _T("");
	m_strReason = _T("");
	m_strMediaMode = _T("");
	m_strRedirectedNumber = _T("");
	m_strRedirectingNumber = _T("");
	m_strRedirectedName = _T("");
	m_strRedirectingName = _T("");
	m_strLastStateChangeTime = _T("00:00:00");
	m_strTrunkID = _T("");
	UpdateData(FALSE);

	if (m_edtNumber.IsWindowEnabled())
		GotoDlgCtrl(&m_edtNumber);
	else
		GotoDlgCtrl(&m_cbLines);

	// Turn off all the buttons.
	EnableCallButtons(0, 0, 0, 0);

	m_btnUserUserInfo.EnableWindow(FALSE);
	m_btnCallData.EnableWindow(FALSE);
	m_btnQOS.EnableWindow(FALSE);
	m_btnISDN.EnableWindow(FALSE);
	m_btnFeatures.EnableWindow(FALSE);

	if (m_pDlgCallFeatures)
		m_pDlgCallFeatures->SetCallFeatures(_T(""));
	if (m_pDlgCallData)
		m_pDlgCallData->OnCallChanged();
} // CPhoneDlg::NoCallSelected

/////////////////////////////////////////////////////////////////////////////
// CPhoneDlg::EnableCallButtons
//
// Enable/disable the various call buttons based on whether we support them.
//
void CPhoneDlg::EnableCallButtons(DWORD dwFeatures, DWORD dwFeatures2, DWORD dwTransfermodes, DWORD dwECSTACallFeatures)
{
	m_btnTransfer.EnableWindow((dwFeatures & (LINECALLFEATURE_BLINDTRANSFER | LINECALLFEATURE_SETUPTRANSFER)) > 0);
	m_btnSetupConf.EnableWindow((dwFeatures & LINECALLFEATURE_SETUPCONF) > 0);
	m_btnRemoveFromConf.EnableWindow((dwFeatures & LINECALLFEATURE_REMOVEFROMCONF) > 0);
	m_btnRedirect.EnableWindow((dwFeatures & LINECALLFEATURE_REDIRECT) > 0);
	m_btnHold.EnableWindow((dwFeatures & (LINECALLFEATURE_HOLD | LINECALLFEATURE_UNHOLD)) > 0);
	m_btnDrop.EnableWindow((dwFeatures & LINECALLFEATURE_DROP) > 0);

	m_btnComplete.EnableWindow((dwFeatures & LINECALLFEATURE_COMPLETETRANSF) > 0 && (dwTransfermodes & LINETRANSFERMODE_TRANSFER) && (dwFeatures2 & LINECALLFEATURE2_TRANSFERNORM));
	m_btnCompleteAsConference.EnableWindow((dwFeatures & LINECALLFEATURE_COMPLETETRANSF) > 0 && (dwTransfermodes & LINETRANSFERMODE_CONFERENCE) && (dwFeatures2 & LINECALLFEATURE2_TRANSFERCONF));

	m_btnAnswer.EnableWindow((dwFeatures & LINECALLFEATURE_ANSWER) > 0);
	m_btnAddToConf.EnableWindow((dwFeatures & (LINECALLFEATURE_ADDTOCONF | LINECALLFEATURE_PREPAREADDCONF)) > 0);
	m_btnAccept.EnableWindow((dwFeatures & LINECALLFEATURE_ACCEPT) > 0);
	m_btnSwapHold.EnableWindow((dwFeatures & LINECALLFEATURE_SWAPHOLD) > 0);
	m_btnCompleteCall.EnableWindow((dwFeatures & LINECALLFEATURE_COMPLETECALL) > 0);
	m_btnPark.EnableWindow((dwFeatures & LINECALLFEATURE_PARK) > 0);
	m_btnDial.EnableWindow((dwFeatures & (LINECALLFEATURE_DIAL | LINECALLFEATURE_GENERATEDIGITS)) > 0);
	m_btnAppspecific.EnableWindow(dwFeatures ? TRUE : FALSE);

	m_btnCallRecording.EnableWindow(dwECSTACallFeatures & (ECSTA_CALLFEATURE_CALLRECORDINGSTART | ECSTA_CALLFEATURE_CALLRECORDINGSTOP));
	if (dwECSTACallFeatures & ECSTA_CALLFEATURE_CALLRECORDINGSTART)
		m_btnCallRecording.SetWindowText(_T("Start Rec."));
	else if (dwECSTACallFeatures & ECSTA_CALLFEATURE_CALLRECORDINGSTOP)
		m_btnCallRecording.SetWindowText(_T("Stop Rec."));

	if (dwFeatures & LINECALLFEATURE_UNHOLD)
		m_btnHold.SetWindowText(_T("Unho&ld"));
	else
		m_btnHold.SetWindowText(_T("Ho&ld"));

} // CPhoneDlg::EnableCallButtons

/////////////////////////////////////////////////////////////////////////////
// CPhoneDlg::EnableLineButtons
//
// Enable/disable the various line buttons based on whether we support them.
//
void CPhoneDlg::EnableLineButtons(DWORD dwCaps)
{
	UpdateData(TRUE);
	UpdateButtons(dwCaps);

	CTapiLine* pLine = GetActiveLine();
	if (pLine != NULL)
	{
		LPLINEDEVSTATUS lpStatus = pLine->GetLineStatus();
		if (lpStatus != NULL)
			m_btnForward.EnableWindow((lpStatus->dwLineFeatures & (LINEFEATURE_FORWARD | LINEFEATURE_FORWARDDND | LINEFEATURE_FORWARDFWD)));
		else
			m_btnForward.EnableWindow(FALSE);
	}
	else
		m_btnForward.EnableWindow(FALSE);

} // CPhoneDlg::EnableLineButtons

void CPhoneDlg::UpdateButtons(DWORD dwAddressCaps)
{
	CTapiAddress* pAddr = GetActiveAddress();
	CTapiLine* pLine = GetActiveLine();

	static DWORD m_dwAddressCaps = 0;
	if (dwAddressCaps != m_dwAddressCaps && dwAddressCaps)
		m_dwAddressCaps = dwAddressCaps;

	if (pAddr == NULL || pLine == NULL)
	{
		m_btnMakeCall.EnableWindow(FALSE);
		m_btnPickup.EnableWindow(FALSE);
		m_btnUnpark.EnableWindow(FALSE);
	}
	else if (m_strNumber.IsEmpty() && !(GetKeyState(VK_CONTROL) < 0))
	{
		m_btnMakeCall.EnableWindow(FALSE);
		m_btnPickup.EnableWindow(FALSE);
		m_btnUnpark.EnableWindow(FALSE);
	}
	else
	{
		m_btnMakeCall.EnableWindow((m_dwAddressCaps & LINEADDRFEATURE_MAKECALL) > 0);
		m_btnPickup.EnableWindow((m_dwAddressCaps & LINEADDRFEATURE_PICKUP) > 0);
		m_btnUnpark.EnableWindow((m_dwAddressCaps & LINEADDRFEATURE_UNPARK) > 0);
	}
}

/////////////////////////////////////////////////////////////////////////////
// CPhoneDlg::OnStartSession
//
// Open a new session
//
void CPhoneDlg::OnStartSession()
{
	// Priority of media modes
	static struct
	{
		DWORD dwMediaMode;
		LPCTSTR pszName;
	} g_MediaModes[] = {{LINEMEDIAMODE_INTERACTIVEVOICE, _T("Voice")}, {LINEMEDIAMODE_DATAMODEM, _T("DataModem")}, {LINEMEDIAMODE_AUTOMATEDVOICE, _T("AutomatedVoice")}, {LINEMEDIAMODE_DIGITALDATA, _T("DigitalData")}, {LINEMEDIAMODE_G3FAX, _T("G3 FAX")}, {LINEMEDIAMODE_G4FAX, _T("G4 FAX")}, {LINEMEDIAMODE_TDD, _T("TDD")}, {LINEMEDIAMODE_TELETEX, _T("TeleTex")}, {LINEMEDIAMODE_VIDEOTEX, _T("VideoTex")}, {LINEMEDIAMODE_TELEX, _T("Telex")}, {LINEMEDIAMODE_MIXED, _T("Mixed")}, {LINEMEDIAMODE_ADSI, _T("ADSI")}, {LINEMEDIAMODE_VOICEVIEW, _T("VoiceView")}, {0, NULL}};

	ClearErrors();
	NoLineSelected();

	CTapiLine* pLine = GetActiveLine();
	if (pLine == NULL)
		return;

	if (pLine->IsOpen())
	{
		RemoveSession(pLine);
		pLine->Close();
	}
	else
	{
		// Open for ALL media modes first.
		DWORD dwMediaMode = 0;
		const LPLINEDEVCAPS lpCaps = pLine->GetLineCaps();
		if (lpCaps)
			dwMediaMode = (lpCaps->dwMediaModes & ~LINEMEDIAMODE_UNKNOWN);

		// Open the line
		LONG lResult = pLine->Open(LINECALLPRIVILEGE_OWNER | LINECALLPRIVILEGE_MONITOR, dwMediaMode);

		// UNIMODEM only allows ONE media mode to be chosen.. pick the best one available.
		if (lResult == LINEERR_INVALMEDIAMODE)
		{
			// Pick only ONE media mode
			for (int i = 0; g_MediaModes[i].dwMediaMode != 0; i++)
			{
				if (dwMediaMode & g_MediaModes[i].dwMediaMode)
				{
					lResult = pLine->Open(LINECALLPRIVILEGE_OWNER | LINECALLPRIVILEGE_MONITOR, g_MediaModes[i].dwMediaMode);
					if (lResult == 0)
					{
						ErrorMsg(_T("Forced to open line with media mode %s"), g_MediaModes[i].pszName);
						break;
					}
				}
			}
		}

		// Show an error
		if (lResult != 0)
			ShowError("lineOpen", lResult);
		else
		{
			// Get the states we get notified on
			DWORD dwAddrSt = (LINEADDRESSSTATE_CAPSCHANGE + LINEADDRESSSTATE_CAPSCHANGE - 1);
			CTapiAddress* pAddr = GetActiveAddress();
			if (pAddr)
			{
				LPLINEADDRESSCAPS lpACaps = pAddr->GetAddressCaps();
				if (lpACaps)
					dwAddrSt &= lpACaps->dwAddressStates;
			}
			DWORD dwStates = (LINEDEVSTATE_REMOVED + LINEDEVSTATE_REMOVED - 1);
			if (lpCaps)
				dwStates &= lpCaps->dwLineStates;
			dwStates |= LINEDEVSTATE_CAPSCHANGE;
			lResult = pLine->SetStatusMessages(dwStates, dwAddrSt);
			if (lResult != 0)
				ShowError("lineSetStatusMessages", lResult);

			pLine->GetLineCaps(0, 0, TRUE);
			pLine->GatherAddressInformation();
			AddSession(pLine);
			OnLoadLineInfo(pLine);
		}
	}
	OnChangeLine();
	OnAddressChange();
	GetECSTALineSpecific();

} // OnStartSession

/////////////////////////////////////////////////////////////////////////////
// CPhoneDlg::OnNewCall
//
// Called to add a call to our list of "monitored" calls.
//
void CPhoneDlg::OnNewCall(CTapiCall* pCall)
{
	CTapiLine* pLine = GetActiveLine();
	if (pLine == NULL || pCall == NULL || pCall->GetLineOwner() != pLine)
		return;

	// Make sure it isn't there.
	for (int i = 0; i < m_arrCalls.GetSize(); i++)
		if (pCall == (CTapiCall*)m_arrCalls[i])
			return;

	if (pLine->GetLineName().Find(_T("IP Office Phone")) != 0)
	{
		// Die IP Office meldet den Call nach dem MakeCall initial mit LineCallstate Idle
		// If the call is IDLE, deallocate it.
		if (pCall->GetCallState() == LINECALLSTATE_IDLE)
		{
			pCall->Deallocate();
			return;
		}
	}

	// Add it to our list.
	int iPos = (int)m_arrCalls.Add(pCall);

	// Change our counter.
	m_ctlCalls.SetRange(0, static_cast<short>(m_arrCalls.GetSize()));
	m_ctlCalls.Invalidate(TRUE);

	// Show this new call.
	MoveToCall(iPos);

} // CPhoneDlg::OnNewCall

/////////////////////////////////////////////////////////////////////////////
// CPhoneDlg::RemoveCall
//
// This removes the specified call from our list when it goes IDLE.
//
void CPhoneDlg::RemoveCall(CTapiCall* pCall)
{
	// Make sure it is there
	for (int i = 0; i < m_arrCalls.GetSize(); i++)
	{
		if (pCall == (CTapiCall*)m_arrCalls[i])
		{
			m_arrCalls.RemoveAt(i);
			if (m_iCall == i)
			{
				if (--m_iCall < 0)
					m_iCall = 0;
			}
		}
	}

	// Change our counter.
	m_ctlCalls.SetRange(0, static_cast<short>(m_arrCalls.GetSize()));
	m_ctlCalls.Invalidate(TRUE);

	if (m_arrCalls.GetSize() == 0)
		NoCallSelected();
	else
		MoveToCall(0);

	// Remove the call
	if (m_bDelayDeleteCalls)
	{
		pCall->CancelWaitForFeature();
		m_arrCallsPendingDelete.Add(pCall);
	}
	else
		pCall->Deallocate();

} // CPhoneDlg::RemoveCall

void CPhoneDlg::InitPendingDeleteCalls()
{
	m_bDelayDeleteCalls = true;
}

void CPhoneDlg::FinishPendingDeleteCalls()
{
	m_bDelayDeleteCalls = false;
	for (int i = 0; i < m_arrCallsPendingDelete.GetSize(); i++)
		((CTapiCall*)m_arrCallsPendingDelete[i])->Deallocate();
	m_arrCallsPendingDelete.RemoveAll();
}

/////////////////////////////////////////////////////////////////////////////
// CPhoneDlg::OnDeltaposSession
//
// This changes the session using the updown control.
//
void CPhoneDlg::OnDeltaposSession(NMHDR* pNMHDR, LRESULT* pResult)
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	CTapiLine* pLine = (CTapiLine*)m_arrSessions[m_iSession];
	if (pNMUpDown->iDelta > 0)
	{
		if (m_iSession < m_arrSessions.GetSize() - 1)
			pLine = (CTapiLine*)m_arrSessions[++m_iSession];
	}
	else
	{
		if (m_iSession > 0)
			pLine = (CTapiLine*)m_arrSessions[--m_iSession];
	}

	for (int i = 0; i < m_cbLines.GetCount(); i++)
	{
		if (pLine == m_cbLines.GetItemDataPtr(i))
		{
			ClearErrors();
			m_cbLines.SetCurSel(i);
			OnChangeLine();
			UpdateSession(pLine);
		}
	}
	*pResult = 0;

} // CPhoneDlg::OnDeltaposSession

/////////////////////////////////////////////////////////////////////////////
// CPhoneDlg::UpdateSession
//
// This updates our current session number when the user changes it
// using the updown control
//
void CPhoneDlg::UpdateSession(CTapiLine* pLine)
{
	m_strSessionNum.Format(_T("%d Open"), m_arrSessions.GetSize());
	m_iSession = 0;

	if (pLine != NULL)
	{
		for (int i = 0; i < m_arrSessions.GetSize(); i++)
		{
			if (pLine == (CTapiLine*)m_arrSessions[i])
			{
				m_iSession = i;
				m_strSessionNum.Format(_T("%d of %d"), i + 1, m_arrSessions.GetSize());
				break;
			}
		}
	}

	m_ctlSession.SetPos(m_iSession);
	UpdateData(FALSE);

} // CPhoneDlg::UpdateSession

/////////////////////////////////////////////////////////////////////////////
// CPhoneDlg::AddSession
//
// This adds a new session (open line) to our list.
//
void CPhoneDlg::AddSession(CTapiLine* pLine)
{
	m_iSession = (int)m_arrSessions.Add(pLine);
	m_ctlSession.SetRange(0, static_cast<short>(m_arrSessions.GetSize()));
	m_ctlSession.Invalidate(TRUE);
	UpdateSession(pLine);

} // CPhoneDlg::AddSession

/////////////////////////////////////////////////////////////////////////////
// CPhoneDlg::RemoveSession
//
// This removes a newly closed line from our session list
//
void CPhoneDlg::RemoveSession(CTapiLine* pLine)
{
	for (int i = 0; i < m_arrSessions.GetSize(); i++)
	{
		if (pLine == (CTapiLine*)m_arrSessions[i])
		{
			m_arrSessions.RemoveAt(i);
			break;
		}
	}
	m_ctlSession.SetRange(0, static_cast<short>(m_arrSessions.GetSize()));
	m_ctlSession.Invalidate(TRUE);
	UpdateSession(NULL);

} // CPhoneDlg::RemoveSession

/////////////////////////////////////////////////////////////////////////////
// CPhoneDlg::OnTimer
//
// Periodic interval timer.
//
void CPhoneDlg::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == m_uiTimerRefresh)
	{
		CTapiCall* pCall = GetActiveCall();
		if (pCall != NULL)
		{
			LPLINECALLSTATUS lpStatus = pCall->GetCallStatus();
			if (lpStatus != NULL)
			{
				SYSTEMTIME localTime;
				CTimeSpan tmDiff;
				if (SystemTimeToTzSpecificLocalTime(NULL, &lpStatus->tStateEntryTime, &localTime))
					tmDiff = CTime::GetCurrentTime() - CTime(localTime);
				else
				{
					GetSystemTime(&localTime);
					tmDiff = CTime(localTime) - CTime(lpStatus->tStateEntryTime);
				}
				m_strLastStateChangeTime = tmDiff.Format("%H:%M:%S");
			}
			else
				m_strLastStateChangeTime = _T("00:00:00");
		}

		SYSTEMTIME localTime;
		GetLocalTime(&localTime);
		CTime time(localTime);
		m_strCurrentTime = time.Format("%d.%m.%Y - %H:%M:%S");
		UpdateData(FALSE);

		// Clear the error line periodically
		if ((g_dwLastError + 10000) < GetTickCount())
			ClearErrors();
	}
	else if (nIDEvent == m_uiTimerReposition)
	{
		KillTimer(m_uiTimerReposition);
		m_uiTimerReposition = 0;
		RepositionWindow();
	}
} // CPhoneDlg::OnTimer

/////////////////////////////////////////////////////////////////////////////
// CPhoneDlg::OnAddressChange
//
// This is called when the user selects a new address in our address
// combo box.
//
void CPhoneDlg::OnAddressChange()
{
	CTapiLine* pLine = GetActiveLine();

	// If no address is supported, kill all the line-call buttons
	CTapiAddress* pAddr = GetActiveAddress();
	if (pAddr == NULL || pLine == NULL)
	{
		EnableLineButtons(0);
		m_btnAgentInfo.EnableWindow(FALSE);
		m_btnActiveAddress.EnableWindow(FALSE);
		return;
	}

	ECSTAAddressCaps addressCaps;
	if (pLine->IsOpen() && pAddr->GetECSTAAddressCaps(addressCaps) == NO_ERROR)
		if (addressCaps.m_dwAddressFlags & ECSTA_ADDRESSFLAG_ALLOWACTIVEADDRESS)
			m_btnActiveAddress.EnableWindow(TRUE);
		else
			m_btnActiveAddress.EnableWindow(FALSE);
	else
		m_btnActiveAddress.EnableWindow(FALSE);

	// Otherwise use the current address features to enable the buttons.
	LPLINEADDRESSSTATUS lpStatus = pAddr->GetAddressStatus(TRUE);
	LPLINEADDRESSCAPS lpCaps = pAddr->GetAddressCaps();
	EnableLineButtons((lpStatus != NULL) ? lpStatus->dwAddressFeatures : 0);

	// Enable/Disable the agent button
	BOOL bAgentSupported = (lpStatus != NULL) ? pAddr->SupportsAgents() : FALSE;
	const ECSTALINEDEVCAPS4* pECSTACaps = GetActiveLine()->GetECSTALineCaps4();
	if (pECSTACaps && GetActiveLine()->IsOpen())
	{
		if (pECSTACaps->dwECSTACapabilities & ECSTACAPS_AGENTGROUPS)
		{
			ETSPVarStruct<VARSTRING> agentgroupStruct;
			if (pLine->GetDevConfigStruct(L"ecsta/AgentGroups", &agentgroupStruct) == NO_ERROR)
				bAgentSupported = TRUE;
		}
		else if (pECSTACaps->dwECSTACapabilities & ECSTACAPS_AGENTSUPPORT)
		{
			ETSPVarStruct<VARSTRING> agentState;
			if (pLine->GetDevConfigStruct(L"ecsta/AgentState", &agentState) == NO_ERROR)
				bAgentSupported = TRUE;
		}
	}
	m_btnAgentInfo.EnableWindow(bAgentSupported);

	// Set the "max" calls.  This is a combination of all Active/OnHold calls.
	int iMaxCalls = (int)(lpCaps->dwMaxNumActiveCalls + lpCaps->dwMaxNumOnHoldCalls + lpCaps->dwMaxNumOnHoldPendingCalls);
	m_strMaxCalls.Format(_T("%d"), iMaxCalls);
	UpdateData(FALSE);

	if (m_pForwardList)
		m_pForwardList->UpdateList();

} // CPhoneDlg::OnAddressChange

/////////////////////////////////////////////////////////////////////////////
// CPhoneDlg::OnChangeCall
//
// This is called when the user uses the updown control to walk through
// our call list.
//
void CPhoneDlg::OnChangeCall(NMHDR* pNMHDR, LRESULT* pResult)
{
	ClearErrors();

	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	if (pNMUpDown->iDelta > 0)
	{
		if (m_iCall < m_arrCalls.GetSize() - 1)
			MoveToCall(++m_iCall);
	}
	else
	{
		if (m_iCall > 0)
			MoveToCall(--m_iCall);
	}
	*pResult = 0;

} // CPhoneDlg::OnChangeCall

/////////////////////////////////////////////////////////////////////////////
// CPhoneDlg::i_OnAgentChange
//
// An agent associated with some address we have open has changed.
//
LRESULT CPhoneDlg::i_OnAgentChange(WPARAM wParam, LPARAM lParam)
{
	CTapiAddress* pAddr = (CTapiAddress*)wParam;
	CAgentStateDlg* pDlg = (CAgentStateDlg*)m_mapAgentDlg[pAddr];
	if (pDlg != NULL)
		pDlg->PostMessage(UM_AGENTCHANGE, wParam, lParam);
	OnAddressChange();
	return 0L;

} // CPhoneDlg::i_OnAgentChange

LRESULT CPhoneDlg::i_OnECSTAAgentEvent(WPARAM wParam, LPARAM lParam)
{
	if (m_pDlgECSTAAgentState)
	{
		EAgentStateEvent* pEvent = (EAgentStateEvent*)wParam;

		if (pEvent->m_pLine == GetActiveLine())
		{
			EAgentStateEvent* pPostEvent = new EAgentStateEvent;
			*pPostEvent = *pEvent;
			m_pDlgECSTAAgentState->PostMessage(UM_ECSTAAGENTEVENT, (WPARAM)pPostEvent, lParam);
		}
	}

	return 0L;

} // CPhoneDlg::i_OnECSTAAgentGroupChange

/////////////////////////////////////////////////////////////////////////////
// CPhoneDlg::i_OnCallChange
//
// Called when TAPI notifies us that a call has changed
//
LRESULT CPhoneDlg::i_OnCallChange(WPARAM /*wParam*/, LPARAM lParam)
{
	CTapiCall* pCall = (CTapiCall*)lParam;
	// Die IP Office meldet den Call nach dem MakeCall initial mit LineCallstate Idle
	// If the call is IDLE, deallocate it.
	if (pCall->GetCallState() == LINECALLSTATE_IDLE)
	{
		CTapiLine* pLine = pCall->GetLineOwner();
		if (pLine)
		{
			if (pLine->GetLineName().Find(_T("IP Office Phone")) == 0)
			{
				if (pCall->GetCallWasNonIdle())
				{
					RemoveCall(pCall);
					return 0;
				}
			}
			else
			{
				RemoveCall(pCall);
				return 0;
			}
		}
		else
		{
			RemoveCall(pCall);
			return 0;
		}
	}

	if (pCall == GetActiveCall())
		MoveToCall(m_iCall);

	return 0;

} // CPhoneDlg::i_OnCallChange

/////////////////////////////////////////////////////////////////////////////
// CPhoneDlg::i_OnAddressChange
//
// Called when TAPI notifies us that the address changed
//
LRESULT CPhoneDlg::i_OnAddressChange(WPARAM wParam, LPARAM lParam)
{
	DWORD dwState = (DWORD)lParam;
	CTapiAddress* pAddr = (CTapiAddress*)wParam;
	if (GetActiveAddress() == pAddr)
	{
		OnAddressChange();
		if (dwState == LINEADDRESSSTATE_CAPSCHANGE)
			OnChangeLine();
	}
	return 0;

} // CPhoneDlg::i_OnAddressChange

/////////////////////////////////////////////////////////////////////////////
// CPhoneDlg::i_OnDynamicRemove
//
// Called when TAPI notifies us that the line was removed
//
LRESULT CPhoneDlg::i_OnDynamicRemove(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	FillLineList();
	return 0;

} // CPhoneDlg::i_OnDynamicRemove

/////////////////////////////////////////////////////////////////////////////
// CPhoneDlg::i_OnDynamicCreate
//
// Called when TAPI notifies us that the line was created
//
LRESULT CPhoneDlg::i_OnDynamicCreate(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	FillLineList();
	return 0;

} // CPhoneDlg::i_OnDynamicCreate

void CPhoneDlg::UpdateAddressNames()
{
	int iItem = m_cbAddress.GetCurSel();
	// update address names (for Panasonic activeAddress)
	for (int i = 0; i < m_cbAddress.GetCount(); i++)
	{
		CTapiAddress* pAddr = (CTapiAddress*)m_cbAddress.GetItemDataPtr(i);
		CString strName = pAddr->GetDialableAddress();
		if (strName.IsEmpty())
			strName.Format(_T("Address %ld"), pAddr->GetAddressID());
		else
			strName.Format(_T("%s - ID %ld"), (const wchar_t*)pAddr->GetDialableAddress(), pAddr->GetAddressID());

		ECSTAAddressCaps addressCaps;
		if (pAddr->GetECSTAAddressCaps(addressCaps) == NO_ERROR)
			strName += addressCaps.getDisplayText();
		m_cbAddress.DeleteString(i);
		m_cbAddress.InsertString(i, strName);
		m_cbAddress.SetItemDataPtr(i, pAddr);
	}
	m_cbAddress.SetCurSel(iItem);
}

LRESULT CPhoneDlg::i_OnLineActiveAddressChange(WPARAM wParam, LPARAM lParam)
{
	UpdateAddressNames();
	wParam;
	lParam;
	return 0;
}

LRESULT CPhoneDlg::i_OnLineAbsentMessageChange(WPARAM wParam, LPARAM lParam)
{
	lParam;

	CTapiLine* pLine = (CTapiLine*)wParam;
	if (GetActiveLine() == pLine)
	{
		// update absentmessage
		ETSPVarStruct<VARSTRING> absentStruct;
		if (pLine->GetDevConfigStruct(L"ecsta/AbsentMessage", &absentStruct) == NO_ERROR)
		{
			if (absentStruct.pData->dwStringFormat == STRINGFORMAT_BINARY && absentStruct.pData->dwStringSize == sizeof(ECSTADEVSPECIFICELEMENT_ABSENTMESSAGEPARAMS))
			{
				// ECSTADEVSPECIFICELEMENT_ABSENTMESSAGEPARAMS* params = (ECSTADEVSPECIFICELEMENT_ABSENTMESSAGEPARAMS*)((BYTE*)absentStruct.pData + absentStruct.pData->dwStringOffset);
				// int i = 0;
			}
		}
	}
	return 0;
}

LRESULT CPhoneDlg::i_OnLineMsgWaitChange(WPARAM wParam, LPARAM lParam)
{
	CTapiLine* pLine = (CTapiLine*)wParam;
	if (GetActiveLine() == pLine)
	{
		// update msgWait Counter...
		CString strText;
		strText.Format(_T("MWI (%d)"), lParam);
		GetDlgItem(IDC_MESSAGEWAITING)->SetWindowText(strText);
		m_fMsgWaiting = lParam ? TRUE : FALSE;
		UpdateData(FALSE);
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////
// CPhoneDlg::i_OnLineChange
//
// Called when TAPI notifies us that the line changed
//
LRESULT CPhoneDlg::i_OnLineChange(WPARAM wParam, LPARAM /*lParam*/)
{
	CTapiLine* pLine = (CTapiLine*)wParam;
	pLine->GatherAddressInformation();

	if (GetActiveLine() == pLine)
	{
		LPLINEDEVSTATUS lpStatus = pLine->GetLineStatus();
		if (lpStatus)
		{
			if ((lpStatus->dwDevStatusFlags & (LINEDEVSTATUSFLAGS_INSERVICE | LINEDEVSTATUSFLAGS_CONNECTED)) == 0)
				ErrorMsg("The line is out-of-service and disconnected.");
			else if ((lpStatus->dwDevStatusFlags & LINEDEVSTATUSFLAGS_CONNECTED) == 0)
				ErrorMsg("The line is disconnected.");
			else if ((lpStatus->dwDevStatusFlags & LINEDEVSTATUSFLAGS_INSERVICE) == 0)
				ErrorMsg("The line is out-of-service.");
		}
		OnLoadLineInfo(pLine);
		OnChangeLine();
	}
	return 0;

} // CPhoneDlg::i_OnLineChange

/////////////////////////////////////////////////////////////////////////////
// CPhoneDlg::i_OnNewCall
//
// Called when TAPI notifies us that a new call appearance was created
//
LRESULT CPhoneDlg::i_OnNewCall(WPARAM wParam, LPARAM /*lParam*/)
{
	CTapiCall* pCall = (CTapiCall*)wParam;
	OnNewCall(pCall);
	return 0;

} // CPhoneDlg::i_OnNewCall

/////////////////////////////////////////////////////////////////////////////
// CPhoneDlg::GetActiveCall
//
// Returns the currently active call (if any)
//
CTapiCall* CPhoneDlg::GetActiveCall()
{
	if (m_iCall >= 0 && m_iCall < m_arrCalls.GetSize())
		return (CTapiCall*)m_arrCalls[m_iCall];
	m_iCall = -1;
	return NULL;

} // CPhoneDlg::GetActiveCall

/////////////////////////////////////////////////////////////////////////////
// CPhoneDlg::GetActiveLine
//
// Returns the currently active line
//
CTapiLine* CPhoneDlg::GetActiveLine()
{
	// Get the new line which is active.
	int iCurSel = m_cbLines.GetCurSel();
	if (iCurSel == CB_ERR)
		return NULL;
	return (CTapiLine*)m_cbLines.GetItemDataPtr(iCurSel);

} // CPhoneDlg::GetActiveLine

/////////////////////////////////////////////////////////////////////////////
// CPhoneDlg::GetActiveAddress
//
// Returns the currently active address
//
CTapiAddress* CPhoneDlg::GetActiveAddress()
{
	int iCurSel = m_cbAddress.GetCurSel();
	if (iCurSel == CB_ERR)
		return NULL;
	return (CTapiAddress*)m_cbAddress.GetItemDataPtr(iCurSel);

} // CPhoneDlg::GetActiveLine

/////////////////////////////////////////////////////////////////////////////
// CPhoneDlg::OnDropCall
//
// Drops the currently displayed call.
//
void CPhoneDlg::OnDropCall()
{
	ClearErrors();

	CTapiCall* pCall = GetActiveCall();
	if (pCall == NULL)
		return;

	// Make sure we are owner.
	LONG lResult = pCall->SetPrivilege(LINECALLPRIVILEGE_OWNER);
	if (lResult != 0)
		ShowError("lineSetCallPrivilege", lResult);
	else
	{
		lResult = GetTAPIConnection()->WaitForReply(pCall->Drop());
		if (lResult != 0)
			ShowError("lineDrop", lResult);
	}

} // CPhoneDlg::OnDropCall

/////////////////////////////////////////////////////////////////////////////
// CPhoneDlg::OnAcceptCall
//
// Accepts the currently displayed call.
//
void CPhoneDlg::OnAcceptCall()
{
	ClearErrors();

	CTapiCall* pCall = GetActiveCall();
	if (pCall == NULL)
		return;

	// Make sure we are owner.
	LONG lResult = pCall->SetPrivilege(LINECALLPRIVILEGE_OWNER);
	if (lResult != 0)
		ShowError("lineSetCallPrivilege", lResult);
	else
	{
		lResult = GetTAPIConnection()->WaitForReply(pCall->Accept());
		if (lResult != 0)
			ShowError("lineAccept", lResult);
	}

} // CPhoneDlg::OnAcceptCall

/////////////////////////////////////////////////////////////////////////////
// CPhoneDlg::OnAnswerCall
//
// Answers the currently displayed call.
//
void CPhoneDlg::OnAnswerCall()
{
	ClearErrors();

	CTapiCall* pCall = GetActiveCall();
	if (pCall == NULL)
		return;

	// Make sure we are owner.
	LONG lResult = pCall->SetPrivilege(LINECALLPRIVILEGE_OWNER);
	if (lResult != 0)
		ShowError("lineSetCallPrivilege", lResult);
	else
	{
		lResult = GetTAPIConnection()->WaitForReply(pCall->Answer());
		if (lResult != 0)
			ShowError("lineAnswer", lResult);
	}

} // CPhoneDlg::OnAnswerCall

/////////////////////////////////////////////////////////////////////////////
// CPhoneDlg::OnMakeCall
//
// Places a new call
//
void CPhoneDlg::OnMakeCall()
{
	CTapiLine* pLine = GetActiveLine();
	if (pLine == NULL)
		return;

	UpdateData(TRUE);
	ClearErrors();

	// If this is a predictive dialer then prompt the user for
	// predictive dialer information.
	LPLINECALLPARAMS lpCallParams = NULL;
	DWORD dwCallParamsUsed = sizeof(LINECALLPARAMS);

	CTapiAddress* pAddr = GetActiveAddress();
	if (!pAddr)
		pAddr = pLine->GetAddress((DWORD)0);

	DWORD dwAddressCapsFlags = 0;
	if (pAddr != NULL)
	{
		LPLINEADDRESSCAPS lpCaps = pAddr->GetAddressCaps();
		if (lpCaps)
			dwAddressCapsFlags = lpCaps->dwAddrCapFlags;

		if ((dwAddressCapsFlags & LINEADDRCAPFLAGS_PREDICTIVEDIALER) != 0)
		{
			CPredDialDlg dlg(this);
			dlg.m_dwCallStates = lpCaps->dwPredictiveAutoTransferStates;
			dlg.m_nTimeout = lpCaps->dwMaxNoAnswerTimeout;
			if (dlg.DoModal() == IDOK)
			{
				lpCallParams = (LPLINECALLPARAMS) new BYTE[sizeof(LINECALLPARAMS) + 500];
				memset(lpCallParams, 0, sizeof(LINECALLPARAMS) + 500);
				lpCallParams->dwTotalSize = sizeof(LINECALLPARAMS) + 500;
				lpCallParams->dwBearerMode = LINEBEARERMODE_VOICE;
				lpCallParams->dwMediaMode = LINEMEDIAMODE_INTERACTIVEVOICE;
				lpCallParams->dwCallParamFlags = 0;
				lpCallParams->dwAddressID = 0;
				lpCallParams->dwAddressMode = LINEADDRESSMODE_ADDRESSID;
				lpCallParams->dwPredictiveAutoTransferStates = dlg.m_dwCallStates;
				lpCallParams->dwTargetAddressOffset = dwCallParamsUsed;
				lpCallParams->dwTargetAddressSize = (dlg.m_strTarget.GetLength() + 1) * sizeof(TCHAR);
				lstrcpy((LPTSTR)((LPBYTE)lpCallParams + lpCallParams->dwTargetAddressOffset), dlg.m_strTarget);
				lpCallParams->dwNoAnswerTimeout = dlg.m_nTimeout;

				dwCallParamsUsed += lpCallParams->dwTargetAddressSize;
			}
		}
		if (pAddr->GetAddressID() != 0)
		{
			if (!lpCallParams)
			{
				lpCallParams = (LPLINECALLPARAMS) new BYTE[sizeof(LINECALLPARAMS) + 500];
				memset(lpCallParams, 0, sizeof(LINECALLPARAMS) + 500);
				lpCallParams->dwTotalSize = sizeof(LINECALLPARAMS) + 500;
			}
			lpCallParams->dwBearerMode = LINEBEARERMODE_VOICE;
			lpCallParams->dwMediaMode = LINEMEDIAMODE_INTERACTIVEVOICE;
			lpCallParams->dwAddressID = pAddr->GetAddressID();
			lpCallParams->dwAddressMode = LINEADDRESSMODE_ADDRESSID;
			lpCallParams->dwPredictiveAutoTransferStates = 0;
		}
	}

	if (GetKeyState(VK_CONTROL) < 0)
	{
		MakeCallExtended makecallextendedDlg;

		makecallextendedDlg.m_strCalledID = m_strNumber;
		makecallextendedDlg.dwAddressCapsFlags = dwAddressCapsFlags;

		if (makecallextendedDlg.DoModal() == IDCANCEL)
			return;

		if (!lpCallParams)
		{
			lpCallParams = (LPLINECALLPARAMS) new BYTE[sizeof(LINECALLPARAMS) + 500];
			memset(lpCallParams, 0, sizeof(LINECALLPARAMS) + 500);
			lpCallParams->dwTotalSize = sizeof(LINECALLPARAMS) + 500;
		}

		lpCallParams->dwCallingPartyIDOffset = dwCallParamsUsed;
		lpCallParams->dwCallingPartyIDSize = (makecallextendedDlg.m_strCallingID.GetLength() + 1) * sizeof(TCHAR);
		lstrcpy((LPTSTR)((LPBYTE)lpCallParams + lpCallParams->dwCallingPartyIDOffset), makecallextendedDlg.m_strCallingID);
		dwCallParamsUsed += lpCallParams->dwCallingPartyIDSize;

		if (makecallextendedDlg.m_bBlockMyCallingID)
			lpCallParams->dwCallParamFlags |= LINECALLPARAMFLAGS_BLOCKID;

		if (makecallextendedDlg.m_bSetDestinationAutoOffHook)
			lpCallParams->dwCallParamFlags |= LINECALLPARAMFLAGS_DESTOFFHOOK;
	}

	// Test of calldata
	/*
	CString strCallData = "hallo";
	lpCallParams = (LPLINECALLPARAMS) new BYTE[sizeof(LINECALLPARAMS) + strCallData.GetLength()+1];
	memset(lpCallParams, 0, sizeof(LINECALLPARAMS));
	lpCallParams->dwTotalSize = sizeof(LINECALLPARAMS) + strCallData.GetLength()+1;
	lpCallParams->dwBearerMode = LINEBEARERMODE_VOICE;
	lpCallParams->dwMediaMode = LINEMEDIAMODE_INTERACTIVEVOICE;
	lpCallParams->dwCallParamFlags = 0;
	lpCallParams->dwAddressID = 0;
	lpCallParams->dwAddressMode = LINEADDRESSMODE_ADDRESSID;
	lpCallParams->dwPredictiveAutoTransferStates = 0;
	lpCallParams->dwCallDataOffset = sizeof(LINECALLPARAMS);
	lpCallParams->dwCallDataSize = (strCallData.GetLength() + 1) * sizeof(TCHAR);
	lstrcpy((LPTSTR)((LPBYTE)lpCallParams+lpCallParams->dwCallDataOffset), strCallData);
	lpCallParams->dwNoAnswerTimeout = 0;
	*/

	CTapiCall* pCall = NULL;
	LONG lResult = pLine->MakeCall(&pCall, m_strNumber, 0, lpCallParams);
	if (lResult != 0)
		ShowError("lineMakeCall", lResult);

	delete[] lpCallParams;

} // CPhoneDlg::OnMakeCall

/////////////////////////////////////////////////////////////////////////////
// CPhoneDlg::OnUnparkCall
//
// Unpark a call from an extension
//
void CPhoneDlg::OnUnparkCall()
{
	CTapiAddress* pAddress = GetActiveAddress();
	if (pAddress == NULL)
		return;

	UpdateData(TRUE);
	ClearErrors();

	CTapiCall* pCall = NULL;
	LONG lResult = GetTAPIConnection()->WaitForReply(pAddress->Unpark(&pCall, m_strNumber));
	if (lResult != 0)
		ShowError("lineUnpark", lResult);

} // CPhoneDlg::OnUnparkCall

/////////////////////////////////////////////////////////////////////////////
// CPhoneDlg::OnPickupCall
//
// Pickup a call from an extension
//
void CPhoneDlg::OnPickupCall()
{
	CTapiAddress* pAddress = GetActiveAddress();
	if (pAddress == NULL)
		return;

	UpdateData(TRUE);
	ClearErrors();

	CTapiCall* pCall = NULL;
	LONG lResult = GetTAPIConnection()->WaitForReply(pAddress->Pickup(&pCall, m_strNumber, NULL));
	if (lResult != 0)
		ShowError("linePickup", lResult);

} // CPhoneDlg::OnPickupCall

/////////////////////////////////////////////////////////////////////////////
// CPhoneDlg::OnHoldCall
//
// Hold/Unhold the current call
//
void CPhoneDlg::OnHoldCall()
{
	ClearErrors();

	CTapiCall* pCall = GetActiveCall();
	if (pCall == NULL)
		return;

	// Make sure we are owner.
	LONG lResult = pCall->SetPrivilege(LINECALLPRIVILEGE_OWNER);
	if (lResult != 0)
		ShowError("lineSetCallPrivilege", lResult);
	else if (pCall->GetCallStatus()->dwCallFeatures & LINECALLFEATURE_HOLD)
	{
		lResult = GetTAPIConnection()->WaitForReply(pCall->Hold());
		if (lResult != 0)
			ShowError("lineHold", lResult);
	}
	else
	{
		lResult = GetTAPIConnection()->WaitForReply(pCall->Unhold());
		if (lResult != 0)
			ShowError("lineUnhold", lResult);
	}

} // CPhoneDlg::OnHoldCall

/////////////////////////////////////////////////////////////////////////////
// CPhoneDlg::OnSwapHoldCall
//
// SwapHold the current call
//
void CPhoneDlg::OnSwapHoldCall()
{
	ClearErrors();

	// Get the active call
	CTapiCall* pCall = GetActiveCall();
	CTapiLine* pLine = GetActiveLine();
	if (pCall == NULL || pLine == NULL)
		return;

	// Find the other call.  If this call is NOT onHOLD, then
	// look for any onHold call with the SWAPHOLD bit set.
	CTapiCall* pCall_Cons = NULL;
	if ((pCall->GetCallState() & (LINECALLSTATE_ONHOLD | LINECALLSTATE_ONHOLDPENDTRANSFER | LINECALLSTATE_ONHOLDPENDCONF)))
	{
		pCall_Cons = pLine->FindCall((LINECALLSTATE_CONNECTED | LINECALLSTATE_RINGBACK | LINECALLSTATE_PROCEEDING | LINECALLSTATE_DIALING | LINECALLSTATE_DIALTONE), LINECALLFEATURE_SWAPHOLD);
	}
	else
	{
		pCall_Cons = pCall;
		pCall = pLine->FindCall((LINECALLSTATE_ONHOLD | LINECALLSTATE_ONHOLDPENDTRANSFER | LINECALLSTATE_ONHOLDPENDCONF), LINECALLFEATURE_SWAPHOLD);
	}

	if (pCall == NULL || pCall_Cons == NULL)
	{
		ErrorMsg("Unable to locate two calls with lineSwapHold ability.");
		return;
	}

	// Make sure we are owner of both calls.
	LONG lResult = pCall->SetPrivilege(LINECALLPRIVILEGE_OWNER);
	if (lResult != 0)
		ShowError("lineSetCallPrivilege", lResult);
	else
	{
		lResult = pCall_Cons->SetPrivilege(LINECALLPRIVILEGE_OWNER);
		if (lResult != 0)
			ShowError("lineSetCallPrivilege", lResult);
		else
		{
			lResult = GetTAPIConnection()->WaitForReply(pLine->SwapHold(pCall_Cons, pCall));
			if (lResult != 0)
				ShowError("lineSwapHold", lResult);
		}
	}

} // CPhoneDlg::OnSwapHoldCall

/////////////////////////////////////////////////////////////////////////////
// CPhoneDlg::OnCallData
//
// Set/Get the call data
//
void CPhoneDlg::OnCallData()
{
	ClearErrors();

	CTapiCall* pCall = GetActiveCall();
	if (!m_pDlgCallData)
	{
		if (pCall)
		{
			m_pDlgCallData = new CCallDataDlg(this);
			m_pDlgCallData->Create(IDD_CALLDATA, this);
			m_pDlgCallData->ShowWindow(SW_SHOW);
		}
	}
	else
	{
		if (m_pDlgCallData->IsWindowVisible())
			m_pDlgCallData->ShowWindow(SW_HIDE);
		else if (pCall)
			m_pDlgCallData->ShowWindow(SW_SHOW);
	}

	LPLINECALLINFO lpCallInfo = pCall->GetCallInfo();
	LPLINECALLSTATUS lpCallStatus = pCall->GetCallStatus();
	m_btnCallData.EnableWindow(lpCallInfo->dwCallDataSize > 0 || (lpCallStatus->dwCallFeatures & LINECALLFEATURE_SETCALLDATA));

} // CPhoneDlg::OnCallData

/////////////////////////////////////////////////////////////////////////////
// CPhoneDlg::OnUserUserInfo
//
// Set/Get the UUI.
//
void CPhoneDlg::OnUserUserInfo()
{
	ClearErrors();

	CTapiCall* pCall = GetActiveCall();
	if (pCall != NULL)
	{
		CUUIDlg dlg(this, pCall);
		dlg.DoModal();
	}

	LPLINECALLINFO lpCallInfo = pCall->GetCallInfo();
	LPLINECALLSTATUS lpCallStatus = pCall->GetCallStatus();
	m_btnUserUserInfo.EnableWindow(lpCallInfo->dwUserUserInfoSize > 0 || (lpCallStatus->dwCallFeatures & (LINECALLFEATURE_SENDUSERUSER | LINECALLFEATURE_RELEASEUSERUSERINFO)));

} // CPhoneDlg::OnUserUserInfo

/////////////////////////////////////////////////////////////////////////////
// CPhoneDlg::OnISDN
//
// Display ISDN information (Charging, Low/Hi level compat).
//
void CPhoneDlg::OnISDN()
{
	ClearErrors();

	CTapiCall* pCall = GetActiveCall();
	if (pCall != NULL)
	{
		CISDNInfo dlg(this, pCall);
		dlg.DoModal();
	}

	LPLINECALLINFO lpCallInfo = pCall->GetCallInfo();
	m_btnISDN.EnableWindow(lpCallInfo->dwHighLevelCompSize > 0 || lpCallInfo->dwLowLevelCompSize > 0 || lpCallInfo->dwChargingInfoSize > 0);

} // CPhoneDlg::OnISDN

/////////////////////////////////////////////////////////////////////////////
// CPhoneDlg::OnQos
//
// Display/Set Quality of Service information
//
void CPhoneDlg::OnQos()
{
	ClearErrors();

	CTapiCall* pCall = GetActiveCall();
	if (pCall != NULL)
	{
		CQosInfo dlg(this, pCall);
		dlg.DoModal();
	}

	LPLINECALLINFO lpCallInfo = pCall->GetCallInfo();
	LPLINECALLSTATUS lpCallStatus = pCall->GetCallStatus();
	m_btnQOS.EnableWindow(lpCallInfo->dwReceivingFlowspecSize > 0 || lpCallInfo->dwSendingFlowspecSize > 0 || (lpCallStatus->dwCallFeatures & LINECALLFEATURE_SETQOS));

} // CPhoneDlg::OnQos

/////////////////////////////////////////////////////////////////////////////
// CPhoneDlg::OnDial
//
// Dial a number
//
void CPhoneDlg::OnDial()
{
	ClearErrors();

	CTapiLine* pLine = GetActiveLine();
	CTapiCall* pCall = GetActiveCall();
	if (pCall == NULL || pLine == NULL)
		return;

	CDialDlg dlg(this, pLine, LINECALLFEATURE_DIAL);
	if (dlg.DoModal() == IDOK)
	{
		LONG lResult = pCall->SetPrivilege(LINECALLPRIVILEGE_OWNER);
		if (lResult != 0)
			ShowError("lineSetCallPrivilege", lResult);
		else
		{
			if ((pCall->GetCallStatus()->dwCallFeatures & LINECALLFEATURE_DIAL) != 0)
				lResult = GetTAPIConnection()->WaitForReply(pCall->Dial(dlg.m_strNumber));
			else
				lResult = GetTAPIConnection()->WaitForReply(pCall->GenerateDigits(LINEDIGITMODE_DTMF, dlg.m_strNumber));

			if (lResult != 0)
			{
				const LINECALLSTATUS* lpStatus = pCall->GetCallStatus();
				if (lpStatus && (lpStatus->dwCallFeatures & LINECALLFEATURE_DIAL) != 0)
					ShowError("lineDial", lResult);
				else
					ShowError("lineGenerateDigits", lResult);
			}
		}
	}

} // CPhoneDlg::OnDial

/////////////////////////////////////////////////////////////////////////////
// CPhoneDlg::OnRedirectCall
//
// Redirect the call to another number
//
void CPhoneDlg::OnRedirectCall()
{
	ClearErrors();

	CTapiLine* pLine = GetActiveLine();
	CTapiCall* pCall = GetActiveCall();
	if (pCall == NULL || pLine == NULL)
		return;

	CDialDlg dlg(this, pLine, LINECALLFEATURE_REDIRECT);
	if (dlg.DoModal() == IDOK)
	{
		LONG lResult = pCall->SetPrivilege(LINECALLPRIVILEGE_OWNER);
		if (lResult != 0)
			ShowError("lineSetCallPrivilege", lResult);
		else
		{
			lResult = GetTAPIConnection()->WaitForReply(pCall->Redirect(dlg.m_strNumber));
			if (lResult != 0)
				ShowError("lineRedirect", lResult);
		}
	}

} // CPhoneDlg::OnRedirectCall

/////////////////////////////////////////////////////////////////////////////
// CPhoneDlg::OnChangeLineStatus
//
// Update the LINEDEVSTATE flags in the provider from our config.
//
void CPhoneDlg::OnChangeLineStatus()
{
	CTapiLine* pLine = GetActiveLine();
	if (pLine == NULL)
		return;

	// If the line supports lineSetDevStatus..
	LPLINEDEVCAPS lpCaps = pLine->GetLineCaps();
	if (lpCaps == NULL)
		return;
	if ((lpCaps->dwLineFeatures & LINEFEATURE_SETDEVSTATUS) == 0)
		return;

	// Store the CURRENT flags.
	BOOL fConnected = m_fConnected, fMsgWaiting = m_fMsgWaiting;
	BOOL fInService = m_fInService, fLocked = m_fLocked;

	UpdateData(TRUE);
	BOOL fSet = FALSE;
	DWORD dwDevState = 0;

	if (m_fConnected != fConnected)
	{
		dwDevState |= LINEDEVSTATUSFLAGS_CONNECTED;
		if (m_fConnected)
			fSet = TRUE;
	}

	else if (m_fInService != fInService)
	{
		dwDevState |= LINEDEVSTATUSFLAGS_INSERVICE;
		if (m_fInService)
			fSet = TRUE;
	}

	else if (m_fLocked != fLocked)
	{
		dwDevState |= LINEDEVSTATUSFLAGS_LOCKED;
		if (m_fLocked)
			fSet = TRUE;
	}

	else if (m_fMsgWaiting != fMsgWaiting)
	{
		dwDevState |= LINEDEVSTATUSFLAGS_MSGWAIT;
		if (m_fMsgWaiting)
			fSet = TRUE;
	}

	// Set the new status.
	LONG lResult = GetTAPIConnection()->WaitForReply(pLine->SetDeviceStatus(dwDevState, fSet));
	if (lResult != 0)
	{
		ShowError("lineSetLineDevStatus", lResult);
		m_fConnected = fConnected;
		m_fMsgWaiting = fMsgWaiting;
		m_fInService = fInService;
		m_fLocked = m_fLocked;
		UpdateData(FALSE);
	}

} // CPhoneDlg::OnChangeLineStatus

void CPhoneDlg::OnActiveAddress()
{
	CTapiLine* pLine = GetActiveLine();
	if (pLine == NULL)
		return;

	CTapiAddress* pAddr = GetActiveAddress();
	if (pAddr == NULL)
		return;

	DWORD dwAddress = pAddr->GetAddressID();
	pLine->SetDevConfig(&dwAddress, sizeof(dwAddress), L"ecsta/SetActiveAddress");
} // CPhoneDlg::OnAgentInfo

/////////////////////////////////////////////////////////////////////////////
// CPhoneDlg::OnAgentInfo
//
// Open a new modaless agent information dialog
//
void CPhoneDlg::OnAgentInfo()
{
	CTapiLine* pLine = GetActiveLine();
	if (pLine == NULL)
		return;

	const ECSTALINEDEVCAPS4* pECSTACaps = GetActiveLine()->GetECSTALineCaps4();
	if (pECSTACaps)
	{
		if (pECSTACaps->dwECSTACapabilities & (ECSTACAPS_AGENTGROUPS | ECSTACAPS_AGENTSUPPORT))
		{
			CECSTAAgentGroupStateDlg dlg(this, pLine, GetActiveAddress(), pECSTACaps->dwECSTACapabilities);
			m_pDlgECSTAAgentState = &dlg;
			dlg.DoModal();
			m_pDlgECSTAAgentState = NULL;
			return;
		}
	}

	CTapiAddress* pAddr = GetActiveAddress();
	if (pAddr == NULL)
		return;

	CAgentStateDlg* pDlg = (CAgentStateDlg*)m_mapAgentDlg[pAddr];
	if (pDlg == NULL || !IsWindow(pDlg->GetSafeHwnd()))
	{
		delete pDlg;
		pDlg = new CAgentStateDlg(this, pAddr);
		m_mapAgentDlg[pAddr] = pDlg;
	}

	pDlg->ShowWindow(SW_SHOW);
	pDlg->SetActiveWindow();

} // CPhoneDlg::OnAgentInfo

/////////////////////////////////////////////////////////////////////////////
// CPhoneDlg::OnDestroy
//
// This is called when our window is being destroyed - close all the
// child agent windows.
//
void CPhoneDlg::OnDestroy()
{
	for (POSITION pos = m_mapAgentDlg.GetStartPosition(); pos != NULL;)
	{
		void *pAddr, *pDlg;
		m_mapAgentDlg.GetNextAssoc(pos, pAddr, pDlg);
		delete ((CAgentStateDlg*)pDlg);
	}
	if (m_pDlgCallFeatures)
	{
		m_pDlgCallFeatures->DestroyWindow();
		delete m_pDlgCallFeatures;
		m_pDlgCallFeatures = 0;
	}
	if (m_pDlgCallData)
	{
		m_pDlgCallData->DestroyWindow();
		delete m_pDlgCallData;
		m_pDlgCallData = 0;
	}

} // CPhoneDlg::OnDestroy

/////////////////////////////////////////////////////////////////////////////
// CPhoneDlg::OnAgentClose
//
// Notification of an agent dialog closing.
//
void CPhoneDlg::OnAgentClose(CTapiAddress* pAddr)
{
	CAgentStateDlg* pDlg = (CAgentStateDlg*)m_mapAgentDlg[pAddr];
	ASSERT(pDlg != NULL);
	m_mapAgentDlg[pAddr] = NULL;

} // CPhoneDlg::OnAgentClose

/////////////////////////////////////////////////////////////////////////////
// CPhoneDlg::OnConferenceList
//
// List the members of a conference
//
void CPhoneDlg::OnConferenceList()
{
	ClearErrors();
	CTapiCall* pCall = GetActiveCall();
	if (pCall == NULL)
		return;

	CConfListDlg dlg(this, pCall);
	dlg.DoModal();

} // CPhoneDlg::OnConferenceList

/////////////////////////////////////////////////////////////////////////////
// CPhoneDlg::OnTransfer
//
// Initiate a consultation or blind transfer of a call.
//
void CPhoneDlg::OnTransfer()
{
	ClearErrors();
	CTapiCall* pCall = GetActiveCall();
	if (pCall == NULL)
		return;

	LONG lResult = pCall->SetPrivilege(LINECALLPRIVILEGE_OWNER);
	if (lResult != 0)
	{
		ShowError("lineSetCallPrivilege", lResult);
		return;
	}

	CTransferDlg dlgT(this, pCall);
	if (dlgT.DoModal() == IDOK)
	{
		// Consultation transfer..
		if (dlgT.m_fConsultation == TRUE)
		{
			LPLINECALLPARAMS lpCallParams = NULL;
			CTapiLine* pLine = GetActiveLine();
			CTapiAddress* pAddr = NULL;
			if (pLine)
			{
				pAddr = GetActiveAddress();
				if (!pAddr)
					pAddr = pLine->GetAddress((DWORD)0);
				if (pAddr)
				{
					if (pAddr->GetAddressID() != 0)
					{
						lpCallParams = (LPLINECALLPARAMS) new BYTE[sizeof(LINECALLPARAMS) + 500];
						memset(lpCallParams, 0, sizeof(LINECALLPARAMS) + 500);
						lpCallParams->dwTotalSize = sizeof(LINECALLPARAMS) + 500;
						lpCallParams->dwBearerMode = LINEBEARERMODE_VOICE;
						lpCallParams->dwMediaMode = LINEMEDIAMODE_INTERACTIVEVOICE;
						lpCallParams->dwAddressID = pAddr->GetAddressID();
						lpCallParams->dwAddressMode = LINEADDRESSMODE_ADDRESSID;
						lpCallParams->dwPredictiveAutoTransferStates = 0;
					}
				}
			}

			InitPendingDeleteCalls();

			// Setup the transfer..
			CTapiCall* pCCall = NULL;
			LONG lResult2 = GetTAPIConnection()->WaitForReply(pCall->SetupTransfer(&pCCall, lpCallParams));
			if (lResult2 != 0)
			{
				ShowError("lineSetupTransfer", lResult2);
				return;
			}

			// Wait for the call to have the DIAL feature..
			if (!pCCall->WaitForFeature(LINECALLFEATURE_DIAL, 5000))
			{
				ShowError("Dial feature unavailable", -1L);
				pCCall->Drop();
			}
			else
			{
				// Dial the number on the consultation call..
				lResult = pCCall->Dial(dlgT.m_strNumber);
				if (IsTapiError(lResult))
				{
					ShowError("lineDial", lResult);
					pCCall->Drop();
				}
				else
				{
					// Prevent calls to be destroyed during that dialog

					// Complete the transfer..
					CCompleteTransferDlg dlgC(this, pCall, pCCall);
					INT_PTR iResult = dlgC.DoModal();
					if (iResult == IDOK)
					{
						DWORD dwTransferType = (dlgC.m_fConference == TRUE) ? LINETRANSFERMODE_CONFERENCE : LINETRANSFERMODE_TRANSFER;
						CTapiCall* pConfOwner;
						lResult = GetTAPIConnection()->WaitForReply(pCall->CompleteTransfer(pCCall, &pConfOwner, dwTransferType));
						if (lResult != 0)
							ShowError("lineCompleteTransfer", lResult);
					}
					// Canceled transfer, drop consultation call
					else if (iResult == IDCANCEL)
					{
						pCCall->Drop();
						if (pCall->GetCallStatus()->dwCallFeatures & LINECALLFEATURE_UNHOLD)
							pCall->Unhold();
					}
					else if (iResult == IDC_CMDCLOSE)
					{
						// do nothing
					}
				}
			}
			// Deallocate calls destroyed during th dialog
			FinishPendingDeleteCalls();
		}

		// Blind transfer to number indicated.
		else
		{
			LONG lResult2 = GetTAPIConnection()->WaitForReply(pCall->BlindTransfer(dlgT.m_strNumber, 0));
			if (lResult2 != 0)
				ShowError("lineBlindTransfer", lResult2);
		}
	}

} // CPhoneDlg::OnTransfer

/////////////////////////////////////////////////////////////////////////////
// CPhoneDlg::OnParkCall
//
// Park a call to an extension
//
void CPhoneDlg::OnParkCall()
{
	ClearErrors();
	CTapiCall* pCall = GetActiveCall();
	if (pCall == NULL)
		return;

	// Ask the user for the extension
	CDialDlg dlg(this, pCall->GetLineOwner(), LINECALLFEATURE_PARK);
	if (dlg.DoModal() == IDOK)
	{
		LONG lResult = pCall->SetPrivilege(LINECALLPRIVILEGE_OWNER);
		if (lResult != 0)
			ShowError("lineSetCallPrivilege", lResult);
		else
		{
			DWORD dwParkMode = (dlg.m_strNumber.IsEmpty()) ? LINEPARKMODE_NONDIRECTED : LINEPARKMODE_DIRECTED;
			TCHAR szBuff[255];
			memset(szBuff, 0, 255);

			lResult = GetTAPIConnection()->WaitForReply(pCall->Park(dwParkMode, dlg.m_strNumber, szBuff, 255));

			if (lResult != 0)
				ShowError("linePark", lResult);
			else if (szBuff[0] != '\0')
				ErrorMsg(_T("Call Parked at %s"), szBuff);
		}
	}

} // CPhoneDlg::OnParkCall

/////////////////////////////////////////////////////////////////////////////
// CPhoneDlg::OnCompleteTransfer
//
// Complete a transfer
//
void CPhoneDlg::OnCompleteTransfer()
{
	ClearErrors();
	CTapiLine* pLine = GetActiveLine();
	CTapiCall* pCall = GetActiveCall();
	if (pCall == NULL || pLine == NULL)
		return;

	// Find the other call.  If this call is NOT onHOLD, then
	// look for any onHold call with the COMPLETETRANS bit set.
	CTapiCall* pCall_Cons = NULL;
	if ((pCall->GetCallState() & (LINECALLSTATE_ONHOLD | LINECALLSTATE_ONHOLDPENDTRANSFER | LINECALLSTATE_ONHOLDPENDCONF)))
	{
		pCall_Cons = pLine->FindCall((LINECALLSTATE_CONNECTED | LINECALLSTATE_RINGBACK | LINECALLSTATE_PROCEEDING | LINECALLSTATE_DIALING | LINECALLSTATE_DIALTONE), LINECALLFEATURE_COMPLETETRANSF);
	}
	else
	{
		pCall_Cons = pCall;
		pCall = pLine->FindCall((LINECALLSTATE_ONHOLD | LINECALLSTATE_ONHOLDPENDTRANSFER | LINECALLSTATE_ONHOLDPENDCONF), LINECALLFEATURE_COMPLETETRANSF);
	}

	if (pCall == NULL || pCall_Cons == NULL)
	{
		ErrorMsg("Unable to locate two calls with lineCompleteTransfer ability");
		return;
	}

	// Set the privileges on both calls
	LONG lResult = pCall->SetPrivilege(LINECALLPRIVILEGE_OWNER);
	if (lResult != 0)
	{
		ShowError("lineSetCallPrivilege", lResult);
		return;
	}

	lResult = pCall_Cons->SetPrivilege(LINECALLPRIVILEGE_OWNER);
	if (lResult != 0)
	{
		ShowError("lineSetCallPrivilege", lResult);
		return;
	}

	// Complete the transfer
	CTapiCall* pConfOwner;
	lResult = GetTAPIConnection()->WaitForReply(pCall->CompleteTransfer(pCall_Cons, &pConfOwner, LINETRANSFERMODE_TRANSFER));
	if (lResult != 0)
		ShowError("lineCompleteTransfer", lResult);

} // CPhoneDlg::OnCompleteTransfer

void CPhoneDlg::GetECSTALineSpecific()
{
	if (GetActiveLine()->IsTSP_ECSTA())
	{
		CTapiLine* pLine = GetActiveLine();
		// update absentmessage
		ETSPVarStruct<VARSTRING> absentStruct;
		if (pLine->GetDevConfigStruct(L"ecsta/AbsentMessage", &absentStruct) == NO_ERROR)
		{
			if (absentStruct.pData->dwStringFormat == STRINGFORMAT_BINARY && absentStruct.pData->dwStringSize == sizeof(ECSTADEVSPECIFICELEMENT_ABSENTMESSAGEPARAMS))
			{
				// ECSTADEVSPECIFICELEMENT_ABSENTMESSAGEPARAMS* params = (ECSTADEVSPECIFICELEMENT_ABSENTMESSAGEPARAMS*)((BYTE*)absentStruct.pData + absentStruct.pData->dwStringOffset);
				// int i = 0;
			}
		}

		// update msgwait
		ETSPVarStruct<VARSTRING> msgwaitStruct;
		if (pLine->GetDevConfigStruct(L"ecsta/MessageWaitingParams", &msgwaitStruct) == NO_ERROR)
		{
			if (msgwaitStruct.pData->dwStringFormat == STRINGFORMAT_BINARY && msgwaitStruct.pData->dwStringSize == sizeof(ECSTADEVSPECIFICELEMENT_MESSAGEWAITINGPARAMS))
			{
				ECSTADEVSPECIFICELEMENT_MESSAGEWAITINGPARAMS* params = (ECSTADEVSPECIFICELEMENT_MESSAGEWAITINGPARAMS*)((BYTE*)msgwaitStruct.pData + msgwaitStruct.pData->dwStringOffset);
				CString strText;
				strText.Format(_T("MWI (%d)"), params->dwNumMessages);
				GetDlgItem(IDC_MESSAGEWAITING)->SetWindowText(strText);
				m_fMsgWaiting = params->dwNumMessages ? TRUE : FALSE;
				UpdateData(FALSE);
			}
		}
	}
}
/////////////////////////////////////////////////////////////////////////////
// CPhoneDlg::OnCompleteCall
//
// Complete a call delivery
//
void CPhoneDlg::OnCompleteCall()
{
	ClearErrors();
	CTapiCall* pCall = GetActiveCall();
	if (pCall == NULL)
		return;

	// Ask the user for the extension
	CCompleteCall dlg(this, pCall);
	if (dlg.DoModal() == IDOK)
	{
		LONG lResult = pCall->SetPrivilege(LINECALLPRIVILEGE_OWNER);
		if (lResult != 0)
			ShowError("lineSetCallPrivilege", lResult);
		else
		{
			DWORD dwCompletionID;
			lResult = GetTAPIConnection()->WaitForReply(pCall->CompleteCall(&dwCompletionID, dlg.m_dwMode, dlg.m_dwMessageID));
			if (lResult != 0)
				ShowError("lineCompleteCall", lResult);
			else
				ErrorMsg("Call Completion ID 0x%lx", dwCompletionID);
		}
	}

} // CPhoneDlg::OnCompleteCall

void CPhoneDlg::OnCallRecording()
{
	ClearErrors();

	CTapiCall* pCall = GetActiveCall();
	if (pCall == NULL)
		return;

	// Make sure we are owner.
	LONG lResult = pCall->SetPrivilege(LINECALLPRIVILEGE_OWNER);
	if (lResult != 0)
		ShowError("lineSetCallPrivilege", lResult);
	else if (pCall->GetECSTACallFeatures() & ECSTA_CALLFEATURE_CALLRECORDINGSTOP)
	{
		// Create Devspecific Command
		DWORD dwSize = sizeof(ECSTA150DEVSPECIFICNEXTLIST) + sizeof(ECSTADEVSPECIFICELEMENT_CALLRECORDINGPARAMS);
		ECSTA150DEVSPECIFICNEXTLIST* pList = (ECSTA150DEVSPECIFICNEXTLIST*)malloc(dwSize);
		memset(pList, 0x00, dwSize);
		pList->dwID = ECSTA150DEVSPECIFICNEXTLISTID;
		pList->dwTotalSize = dwSize;
		pList->dwUsedSize = sizeof(ECSTA150DEVSPECIFICNEXTLIST);
		pList->dwNumElements = 1;
		pList->elements[0].dwElementID = ECSTADEVSPECIFICELEMENT_CALLRECORDINGSTOP;
		pList->elements[0].dwOffset = pList->dwUsedSize;
		pList->elements[0].dwSize = sizeof(ECSTADEVSPECIFICELEMENT_CALLRECORDINGPARAMS);
		ECSTADEVSPECIFICELEMENT_CALLRECORDINGPARAMS params;
		lstrcpyn(params.szVoiceMailNumber, _T("500"), _countof(params.szVoiceMailNumber));

		memcpy((BYTE*)pList + pList->elements[0].dwOffset, &params, sizeof(params));
		pList->dwUsedSize += pList->elements[0].dwSize;

		pList->dwNeededSize = pList->dwUsedSize;

		lResult = GetTAPIConnection()->WaitForReply(pCall->DevSpecific((void*)pList, pList->dwTotalSize));
		if (lResult != 0)
			ShowError("lineDevSpecific CallRecording", lResult);

		free(pList);
	}
	else
	{
		// Create Devspecific Command
		DWORD dwSize = sizeof(ECSTA150DEVSPECIFICNEXTLIST) + sizeof(ECSTADEVSPECIFICELEMENT_CALLRECORDINGPARAMS);
		ECSTA150DEVSPECIFICNEXTLIST* pList = (ECSTA150DEVSPECIFICNEXTLIST*)malloc(dwSize);
		memset(pList, 0x00, dwSize);
		pList->dwID = ECSTA150DEVSPECIFICNEXTLISTID;
		pList->dwTotalSize = dwSize;
		pList->dwUsedSize = sizeof(ECSTA150DEVSPECIFICNEXTLIST);
		pList->dwNumElements = 1;
		pList->elements[0].dwElementID = ECSTADEVSPECIFICELEMENT_CALLRECORDINGSTART;
		pList->elements[0].dwOffset = pList->dwUsedSize;
		pList->elements[0].dwSize = sizeof(ECSTADEVSPECIFICELEMENT_CALLRECORDINGPARAMS);
		ECSTADEVSPECIFICELEMENT_CALLRECORDINGPARAMS params;
		lstrcpyn(params.szVoiceMailNumber, _T("500"), _countof(params.szVoiceMailNumber));

		memcpy((BYTE*)pList + pList->elements[0].dwOffset, &params, sizeof(params));
		pList->dwUsedSize += pList->elements[0].dwSize;

		pList->dwNeededSize = pList->dwUsedSize;

		lResult = GetTAPIConnection()->WaitForReply(pCall->DevSpecific((void*)pList, pList->dwTotalSize));
		if (lResult != 0)
			ShowError("lineDevSpecific CallRecording", lResult);

		free(pList);
	}
}

void CPhoneDlg::OnAppspecific()
{
	ClearErrors();

	CTapiCall* pCall = GetActiveCall();
	if (pCall == NULL)
		return;

	CAppspecificDlg Dlg;

	LPLINECALLINFO pCallInfo = pCall->GetCallInfo();
	if (pCallInfo)
		Dlg.m_dwData = pCallInfo->dwAppSpecific;

	if (Dlg.DoModal() != IDOK)
		return;

	// Make sure we are owner.
	LONG lResult = pCall->SetPrivilege(LINECALLPRIVILEGE_OWNER);
	if (lResult != 0)
		ShowError("lineSetCallPrivilege", lResult);
	else
	{
		lResult = pCall->SetAppSpecificData(Dlg.m_dwData);
		if (lResult != 0)
			ShowError("lineDrop", lResult);
	}
}

/////////////////////////////////////////////////////////////////////////////
// CPhoneDlg::OnSetupConference
//
// Setup a new conference using an existing call
//
void CPhoneDlg::OnSetupConference()
{
	ClearErrors();
	CTapiLine* pLine = GetActiveLine();
	CTapiCall* pCall = GetActiveCall();
	if (pLine == NULL || pCall == NULL)
		return;

	// See if we have calls which can be placed into a conference via
	// transfer.  This requires at least one call on hold and one
	// connected call.
	BOOL fMergeAllowed = FALSE;
	LPLINECALLSTATUS lpStatus = pCall->GetCallStatus();
	if ((lpStatus->dwCallFeatures & LINECALLFEATURE_COMPLETETRANSF && pCall->GetAddressInfo()->GetAddressCaps()->dwTransferModes & LINETRANSFERMODE_CONFERENCE) && lpStatus->dwCallFeatures2 & LINECALLFEATURE2_TRANSFERCONF)
		fMergeAllowed = TRUE;

	CSetupConfDlg dlg(this, pCall, fMergeAllowed);
	if (dlg.DoModal() == IDOK)
	{
		if (dlg.m_fMerge == FALSE)
		{
			LONG lResult = pCall->SetPrivilege(LINECALLPRIVILEGE_OWNER);
			if (lResult != 0)
				ShowError("lineSetCallPrivilege", lResult);
			else
			{
				CTapiCall* pConf;
				CTapiCall* pCons;
				lResult = GetTAPIConnection()->WaitForReply(pCall->SetupConference(&pConf, &pCons, 3, NULL));
				if (lResult != 0)
					ShowError("lineSetupConference", lResult);

				// Complete the conference event.
				else
				{
					InitPendingDeleteCalls();

					// Wait for the call to have the DIAL feature..
					if (!pCons->WaitForFeature(LINECALLFEATURE_DIAL, 5000))
					{
						ShowError("Dial feature unavailable", -1L);
						pCons->Drop();
						pConf->Drop();
					}
					else
					{
						// Dial the number on the consultation call.
						lResult = pCons->Dial(dlg.m_strNumber);
						if (IsTapiError(lResult))
						{
							ShowError("lineDial", lResult);
							pCons->Drop();
							pConf->Drop();
						}
						else
						{
							// Let the user decide when to complete the conference.
							CCompleteTransferDlg dlgC(this, pCall, pCons, TRUE);
							if (dlgC.DoModal() == IDOK)
							{
								lResult = GetTAPIConnection()->WaitForReply(pConf->AddToConference(pCons));
								if (lResult != 0)
									ShowError("lineAddToConference", lResult);
							}
							// Canceled conference, drop consultation call
							else
							{
								pCons->Drop();
								pCall->RemoveFromConference();
								pConf->Drop();
							}
						}
					}
					FinishPendingDeleteCalls();
				}
			}
		}
		// Merge conference call
		else
		{
			// Find the other call(s).  If this call is NOT onHOLD, then
			// look for any onHold call with the COMPLETETRANS bit set.
			CTapiCall* pCall_Cons = NULL;
			if ((pCall->GetCallState() & (LINECALLSTATE_ONHOLD | LINECALLSTATE_ONHOLDPENDTRANSFER | LINECALLSTATE_ONHOLDPENDCONF)))
			{
				pCall_Cons = pLine->FindCall((LINECALLSTATE_CONNECTED | LINECALLSTATE_RINGBACK | LINECALLSTATE_PROCEEDING | LINECALLSTATE_DIALING | LINECALLSTATE_DIALTONE), LINECALLFEATURE_COMPLETETRANSF);
			}
			else
			{
				pCall_Cons = pCall;
				pCall = pLine->FindCall((LINECALLSTATE_ONHOLD | LINECALLSTATE_ONHOLDPENDTRANSFER | LINECALLSTATE_ONHOLDPENDCONF), LINECALLFEATURE_COMPLETETRANSF);
			}

			if (pCall == NULL || pCall_Cons == NULL)
			{
				ErrorMsg("Unable to locate two calls with lineCompleteTransfer ability");
				return;
			}

			// Set the privileges on both calls
			LONG lResult = pCall->SetPrivilege(LINECALLPRIVILEGE_OWNER);
			if (lResult != 0)
			{
				ShowError("lineSetCallPrivilege", lResult);
				return;
			}

			lResult = pCall_Cons->SetPrivilege(LINECALLPRIVILEGE_OWNER);
			if (lResult != 0)
			{
				ShowError("lineSetCallPrivilege", lResult);
				return;
			}

			// Complete the transfer
			CTapiCall* pConfOwner;
			lResult = GetTAPIConnection()->WaitForReply(pCall->CompleteTransfer(pCall_Cons, &pConfOwner, LINETRANSFERMODE_CONFERENCE));
			if (lResult != 0)
				ShowError("lineCompleteTransfer", lResult);

			// Otherwise see if we have additional calls that are onHOLD right now
			// and add them to this new conference.
			else
			{
				CTapiCall* pOldCall = pCall;
				int iCount = 0;
				do
				{
					pCall = pLine->FindCall((LINECALLSTATE_ONHOLD | LINECALLSTATE_ONHOLDPENDTRANSFER | LINECALLSTATE_ONHOLDPENDCONF), LINECALLFEATURE_COMPLETETRANSF);

					if (pCall == pOldCall)
					{
						if (++iCount > 5)
							break;
						Sleep(1000);
					}

					else if (pCall != NULL)
					{
						pOldCall = pCall;
						lResult = GetTAPIConnection()->WaitForReply(pConfOwner->AddToConference(pCall));
						if (lResult != 0)
							ShowError("lineAddToConference", lResult);
					}

				} while (pCall != NULL);
			}
		}
	}

} // CPhoneDlg::OnSetupConference

/////////////////////////////////////////////////////////////////////////////
// CPhoneDlg::OnAddToConference
//
// Add a new party to a conference.
//
void CPhoneDlg::OnAddToConference()
{
	ClearErrors();
	CTapiLine* pLine = GetActiveLine();
	CTapiCall* pCall = GetActiveCall();
	if (pCall == NULL || pLine == NULL)
		return;

	// If this is a conference call that is not connected, create a new party for
	// the call.
	if (pCall->GetCallInfo()->dwOrigin == LINECALLORIGIN_CONFERENCE && pCall->GetCallState() == LINECALLSTATE_CONNECTED && (pCall->GetCallStatus()->dwCallFeatures & LINECALLFEATURE_PREPAREADDCONF) != 0)
	{
		LONG lResult = pCall->SetPrivilege(LINECALLPRIVILEGE_OWNER);
		if (lResult != 0)
			ShowError("lineSetCallPrivilege", lResult);
		else
		{
			CTapiCall* pConsCall;
			lResult = GetTAPIConnection()->WaitForReply(pCall->PrepareAddToConference(&pConsCall, NULL));
			if (lResult != 0)
				ShowError("linePrepareAddToConference", lResult);
		}
		return;
	}

	// Find the other call.  If this call is NOT onHOLD, then
	// look for any onHold call with the COMPLETETRANS bit set.
	CTapiCall* pCall_Cons = NULL;
	if (pCall->GetCallInfo()->dwOrigin == LINECALLORIGIN_CONFERENCE)
	{
		pCall_Cons = pLine->FindCall((LINECALLSTATE_CONNECTED | LINECALLSTATE_RINGBACK | LINECALLSTATE_PROCEEDING | LINECALLSTATE_DIALING | LINECALLSTATE_DIALTONE), LINECALLFEATURE_ADDTOCONF);
	}
	else
	{
		pCall_Cons = pCall;
		pCall = pLine->FindCall((LINECALLSTATE_ONHOLD | LINECALLSTATE_ONHOLDPENDTRANSFER | LINECALLSTATE_ONHOLDPENDCONF), LINECALLFEATURE_ADDTOCONF);
		if (pCall == NULL)
			pCall = pLine->FindCall(LINECALLSTATE_ONHOLDPENDCONF, 0);
	}

	if (pCall == NULL || pCall_Cons == NULL)
	{
		ErrorMsg("Unable to locate two calls with lineAddToConference ability");
		return;
	}

	// Set the privileges on both calls
	LONG lResult = pCall->SetPrivilege(LINECALLPRIVILEGE_OWNER);
	if (lResult != 0)
	{
		ShowError("lineSetCallPrivilege", lResult);
		return;
	}

	lResult = pCall_Cons->SetPrivilege(LINECALLPRIVILEGE_OWNER);
	if (lResult != 0)
	{
		ShowError("lineSetCallPrivilege", lResult);
		return;
	}

	// Complete the transfer
	lResult = GetTAPIConnection()->WaitForReply(pCall->AddToConference(pCall_Cons));
	if (lResult != 0)
		ShowError("lineAddToConference", lResult);

} // CPhoneDlg::OnAddToConference

/////////////////////////////////////////////////////////////////////////////
// CPhoneDlg::OnForward
//
// This is called to forward the phone by the user.
//
void CPhoneDlg::OnForward()
{
	CTapiLine* pLine = GetActiveLine();
	if (pLine == NULL)
		return;

	CTapiAddress* pAddress = GetActiveAddress();
	if (pAddress == NULL)
		return;

	UpdateData(TRUE);
	ClearErrors();

	CForwardList dlg(this, pLine);
	m_pForwardList = &dlg;
	dlg.DoModal();
	m_pForwardList = NULL;

} // CPhoneDlg::OnForward

/////////////////////////////////////////////////////////////////////////////
// CPhoneDlg::OnRemoveFromConference
//
// Remove a party from the conference
//
void CPhoneDlg::OnRemoveFromConference()
{
	ClearErrors();
	CTapiCall* pCall = GetActiveCall();
	if (pCall == NULL)
		return;

	// Set the privileges on both calls
	LONG lResult = pCall->SetPrivilege(LINECALLPRIVILEGE_OWNER);
	if (lResult != 0)
		ShowError("lineSetCallPrivilege", lResult);
	else
	{
		lResult = GetTAPIConnection()->WaitForReply(pCall->RemoveFromConference());
		if (lResult != 0)
			ShowError("lineRemoveFromConference", lResult);
	}

} // CPhoneDlg::OnRemoveFromConference

/////////////////////////////////////////////////////////////////////////////
// CPhoneDlg::OnPhoneInfo
//
// Display the phone information
//
void CPhoneDlg::OnPhoneInfo()
{
	ClearErrors();
	CTapiLine* pLine = GetActiveLine();
	if (pLine == NULL)
		return;

	DWORD dwPhoneID = pLine->GetRelatedPhoneID();
	if (dwPhoneID == 0xffffffff)
	{
		ErrorMsg("There is no phone related to this line.");
		return;
	}

	CMyPhone* pPhone = (CMyPhone*)GetTAPIConnection()->GetPhoneFromDeviceID(dwPhoneID);
	if (pPhone == NULL)
	{
		ErrorMsg("Invalid phoneID (%ld) from tapi/phone", dwPhoneID);
		return;
	}

	// If we have a dialog then just show it.
	if (pPhone->m_pPhone != NULL)
	{
		pPhone->m_pPhone->SetWindowPos(&CWnd::wndTop, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		return;
	}

	// Open our phone device.
	LONG lResult = pPhone->Open(PHONEPRIVILEGE_OWNER);
	if (lResult != 0)
	{
		ShowError("phoneOpen", lResult);
		return;
	}

	lResult = pPhone->SetStatusMessages(0xffffff, 0, 0);
	if (lResult != 0)
		ShowError("phoneSetStatusMessages", lResult);
	pPhone->m_pPhone = new CPhoneCapsDlg(this, pPhone);

} // CPhoneDlg::OnPhoneInfo

/////////////////////////////////////////////////////////////////////////////
// CPhoneDlg::OnConfig
//
// Display the line configuration for the open line
//
void CPhoneDlg::OnConfig()
{
	ClearErrors();
	CTapiLine* pLine = GetActiveLine();
	if (pLine != NULL)
		pLine->Config(this, NULL);

} // CPhoneDlg::OnConfig

/////////////////////////////////////////////////////////////////////////////
// CPhoneDlg::OnClose
//
// Signals the window to close
//
void CPhoneDlg::OnClose()
{
	// Look to see if we have pending requests we are still waiting for.
	// If so, prompt the user and let them decide whether to wait or
	// to destroy them now.
	int iCount = GetTAPIConnection()->GetPendingRequestCount();
	if (iCount > 0)
	{
		TCHAR chBuff[256];
		wsprintf(chBuff, _T("There are %d pending TAPI requests which have not completed.\r\nDo you want to wait for these to complete?"), iCount);
		if (AfxMessageBox(chBuff, MB_YESNO) == IDNO)
			GetTAPIConnection()->StopWaitingForAllRequests();
	}

	EndDialog(IDCANCEL);

} // CPhoneDlg::OnClose

/////////////////////////////////////////////////////////////////////////////
// CPhoneDlg::OnFlashWindow
//
// Change a button font to BOLD to show that it is changed.
//
LRESULT CPhoneDlg::OnFlashWindow(WPARAM wParam, LPARAM /*lParam*/)
{
	static DWORD dwTickCount = 0;
	static DWORD dwLastWindow = 0;

	if (dwLastWindow == wParam && dwTickCount + 1000 >= GetTickCount())
		return 0;

	CFont fntAnsi;
	fntAnsi.CreateStockObject(ANSI_VAR_FONT);

	CWnd* pwnd = GetDlgItem((int)wParam);
	if (pwnd != NULL)
	{
		for (int i = 0; i < 5; i++)
		{
			pwnd->SetFont(&m_fntBold);
			pwnd->UpdateWindow();
			Sleep(50);
			pwnd->SetFont(&fntAnsi);
			pwnd->UpdateWindow();
			Sleep(50);
		}
	}

	dwLastWindow = (DWORD)wParam;
	dwTickCount = GetTickCount();

	return 0;

} // CPhoneDlg::OnFlashWindow

void CPhoneDlg::OnBnClickedFeatures()
{
	if (!m_pDlgCallFeatures)
	{
		m_pDlgCallFeatures = new ECallFeatures();
		m_pDlgCallFeatures->Create(IDD_CALLFEATURES, this);
		m_pDlgCallFeatures->ShowWindow(SW_SHOW);
	}
	else
	{
		if (m_pDlgCallFeatures->IsWindowVisible())
			m_pDlgCallFeatures->ShowWindow(SW_HIDE);
		else
			m_pDlgCallFeatures->ShowWindow(SW_SHOW);
	}
	if (m_pDlgCallFeatures)
		m_pDlgCallFeatures->SetCallFeatures(m_strCallFeaturesList);
}

/////////////////////////////////////////////////////////////////////////////
// CPhoneDlg::OnBnClickedCompleteAsConference
//
// Complete a transfer
//
void CPhoneDlg::OnBnClickedCompleteAsConference()
{
	ClearErrors();
	CTapiLine* pLine = GetActiveLine();
	CTapiCall* pCall = GetActiveCall();
	if (pCall == NULL || pLine == NULL)
		return;

	// Find the other call.  If this call is NOT onHOLD, then
	// look for any onHold call with the COMPLETETRANS bit set.
	CTapiCall* pCall_Cons = NULL;
	if ((pCall->GetCallState() & (LINECALLSTATE_ONHOLD | LINECALLSTATE_ONHOLDPENDTRANSFER | LINECALLSTATE_ONHOLDPENDCONF)))
	{
		pCall_Cons = pLine->FindCall(LINECALLSTATE_CONNECTED, LINECALLFEATURE_COMPLETETRANSF);
	}
	else if ((pCall->GetCallState() & LINECALLSTATE_CONNECTED))
	{
		pCall_Cons = pCall;
		pCall = pLine->FindCall((LINECALLSTATE_ONHOLD | LINECALLSTATE_ONHOLDPENDTRANSFER | LINECALLSTATE_ONHOLDPENDCONF), LINECALLFEATURE_COMPLETETRANSF);
	}

	if (pCall == NULL || pCall_Cons == NULL)
	{
		ErrorMsg("Unable to locate two calls with lineCompleteTransfer ability");
		return;
	}

	// Set the privileges on both calls
	LONG lResult = pCall->SetPrivilege(LINECALLPRIVILEGE_OWNER);
	if (lResult != 0)
	{
		ShowError("lineSetCallPrivilege", lResult);
		return;
	}

	lResult = pCall_Cons->SetPrivilege(LINECALLPRIVILEGE_OWNER);
	if (lResult != 0)
	{
		ShowError("lineSetCallPrivilege", lResult);
		return;
	}

	// Complete the transfer
	CTapiCall* pConfOwner;
	lResult = GetTAPIConnection()->WaitForReply(pCall->CompleteTransfer(pCall_Cons, &pConfOwner, LINETRANSFERMODE_CONFERENCE));
	if (lResult != 0)
		ShowError("lineCompleteTransfer", lResult);

} // CPhoneDlg::OnBnClickedCompleteAsConference

BOOL CPhoneDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: Add your specialized code here and/or call the base class
	if (pMsg->message == WM_KEYDOWN || pMsg->message == WM_KEYUP)
	{
		static bool bControlPressed = false;
		if (GetKeyState(VK_CONTROL) < 0 != bControlPressed)
		{
			bControlPressed = GetKeyState(VK_CONTROL) < 0;
			UpdateButtons(0);
		}
	}
	return CDialog::PreTranslateMessage(pMsg);
}

void CPhoneDlg::RepositionWindow()
{
	EWindowListHelper listWnd;
	GetWindowText(listWnd.m_strMyCaption);
	listWnd.m_hMyHWND = GetSafeHwnd();

	EnumWindows(EnumWindowsProc, (LPARAM)&listWnd);

	CRect rcWindow;
	GetWindowRect(&rcWindow);
	rcWindow.MoveToX((GetSystemMetrics(SM_CXSCREEN) - (rcWindow.Width())) / 2);
	rcWindow.MoveToY((GetSystemMetrics(SM_CYSCREEN) - (rcWindow.Height())) / 2);
	ClientToScreen(rcWindow);

	CRect rcTest = rcWindow;

#ifndef SM_XVIRTUALSCREEN
#define SM_XVIRTUALSCREEN  76
#define SM_YVIRTUALSCREEN  77
#define SM_CXVIRTUALSCREEN 78
#define SM_CYVIRTUALSCREEN 79
#endif

	CRect rcScreenTotal;
	rcScreenTotal.left = GetSystemMetrics(SM_XVIRTUALSCREEN);
	rcScreenTotal.top = GetSystemMetrics(SM_YVIRTUALSCREEN);
	rcScreenTotal.right = rcScreenTotal.left + GetSystemMetrics(SM_CXVIRTUALSCREEN);
	rcScreenTotal.bottom = rcScreenTotal.top + GetSystemMetrics(SM_CYVIRTUALSCREEN);

	int iBorder = GetSystemMetrics(SM_CXFRAME) + GetSystemMetrics(SM_CXBORDER);

	if (listWnd.size())
	{
		EWindowListHelper::iterator iter;
		iter = listWnd.begin();
		while (iter != listWnd.end())
		{
			if (::IsIconic(*iter))
			{
				iter++;
				continue;
			}

			CRect rcWnd;
			::GetWindowRect(*iter, &rcWnd);

			// links davon
			rcTest.MoveToY(rcWnd.top);
			rcTest.MoveToX(rcWnd.left - rcTest.Width() - iBorder);

			if (rcTest.left < rcScreenTotal.left)
				rcTest.MoveToX(rcScreenTotal.left);

			bool bFailed = false;
			EWindowListHelper::iterator iterIntern;
			iterIntern = listWnd.begin();
			while (iterIntern != listWnd.end())
			{
				CRect rcWnd2;
				::GetWindowRect(*iterIntern, &rcWnd2);
				if (CRect().IntersectRect(rcWnd2, rcTest))
				{
					bFailed = true;
					break;
				}
				iterIntern++;
			}

			if (bFailed)
			{
				// rechts davon
				rcTest.MoveToX(rcWnd.right + iBorder);
				if (rcTest.right > rcScreenTotal.right)
					rcTest.MoveToX(rcScreenTotal.right - rcWnd.Width());

				bFailed = false;
				EWindowListHelper::iterator iterIntern2;
				iterIntern2 = listWnd.begin();
				while (iterIntern2 != listWnd.end())
				{
					CRect rcWnd2;
					::GetWindowRect(*iterIntern2, &rcWnd2);
					if (CRect().IntersectRect(rcWnd2, rcTest))
					{
						bFailed = true;
						break;
					}
					iterIntern2++;
				}
			}
			if (!bFailed)
			{
				EWindowListHelper::iterator iterIntern3;
				iterIntern3 = listWnd.begin();
				while (iterIntern3 != listWnd.end())
				{
					CRect rcWnd3;
					::GetWindowRect(*iterIntern3, &rcWnd3);

					if (CRect().IntersectRect(rcWnd3, rcTest))
					{
						bFailed = true;
						;
					}
					iterIntern3++;
				}
				if (!bFailed)
					break;
			}
			iter++;
		}
		SetWindowPos(NULL, rcTest.left, rcTest.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
	}
}
