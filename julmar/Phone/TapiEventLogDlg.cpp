#include "stdafx.h"
#include "TapiEventLogDlg.h"
#include "resource.h"
#include "TapiEventLog.h"

#include <afxdlgs.h>

HWND g_hLogWnd = NULL;
volatile LONG g_lOpening = 0;

// UI thread for the log dialog. The phone window stays responsive, and
// this thread keeps painting the status line while another thread is
// inside a TAPI call.
class CTapiEventLogThread : public CWinThread
{
	DECLARE_DYNCREATE(CTapiEventLogThread)
public:
	CTapiEventLogThread() {}
	virtual BOOL InitInstance();
};

IMPLEMENT_DYNCREATE(CTapiEventLogThread, CWinThread)

BOOL CTapiEventLogThread::InitInstance()
{
	CTapiEventLogDlg* pDlg = new CTapiEventLogDlg();
	if (!pDlg->Create(IDD_TAPIEVENTLOG))
	{
		delete pDlg;
		InterlockedExchange(&g_lOpening, 0);
		return FALSE;
	}
	pDlg->ShowWindow(SW_SHOW);
	return TRUE;
}

CTapiEventLogDlg::CTapiEventLogDlg()
	: CDialog(IDD_TAPIEVENTLOG)
{
}

void CTapiEventLogDlg::Open()
{
	if (g_hLogWnd != NULL && ::IsWindow(g_hLogWnd))
	{
		::ShowWindow(g_hLogWnd, SW_RESTORE);
		::SetForegroundWindow(g_hLogWnd);
		return;
	}
	if (InterlockedCompareExchange(&g_lOpening, 1, 0) != 0)
		return;
	AfxBeginThread(RUNTIME_CLASS(CTapiEventLogThread));
}

void CTapiEventLogDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TAPIEVENTTEXT, m_edit);
	DDX_Control(pDX, IDC_TAPIEVENTSTATUS, m_status);
	DDX_Control(pDX, IDC_TAPIEVENTCOPY, m_copy);
	DDX_Control(pDX, IDC_TAPIEVENTSAVE, m_save);
	DDX_Control(pDX, IDC_TAPIEVENTCLEAR, m_clear);
}

BEGIN_MESSAGE_MAP(CTapiEventLogDlg, CDialog)
	ON_WM_SIZE()
	ON_WM_TIMER()
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_TAPIEVENTCOPY, OnCopy)
	ON_BN_CLICKED(IDC_TAPIEVENTSAVE, OnSave)
	ON_BN_CLICKED(IDC_TAPIEVENTCLEAR, OnClear)
END_MESSAGE_MAP()

BOOL CTapiEventLogDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	// A multiline edit stops at 32767 characters unless told otherwise.
	// ReplaceSel then cuts the newest line in the middle of a word.
	m_edit.SetLimitText(0);
	g_hLogWnd = m_hWnd;
	CRect rect;
	GetClientRect(&rect);
	LayoutControls(rect.Width(), rect.Height());
	SetTimer(1, 200, NULL);
	return TRUE;
}

// Fills the edit with messages taken since the last tick and rewrites the
// status line from whatever TAPI call is still inside.
void CTapiEventLogDlg::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent != 1)
	{
		CDialog::OnTimer(nIDEvent);
		return;
	}

	CString strNewLines;
	CString strStatus;
	TapiEventLogTakeLines(strNewLines, strStatus);
	if (!strNewLines.IsEmpty())
	{
		int iStart = 0;
		int iEnd = 0;
		m_edit.GetSel(iStart, iEnd);
		int iLength = m_edit.GetWindowTextLength();
		bool bSelectAll = (iLength > 0 && iStart == 0 && iEnd == iLength);
		// Follow new lines only while the last line is already in view.
		// A manual scroll upward keeps its place across later appends.
		int iFirstVisible = m_edit.GetFirstVisibleLine();
		SCROLLINFO si = {};
		si.cbSize = sizeof(si);
		si.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
		bool bAtEnd = true;
		if (m_edit.GetScrollInfo(SB_VERT, &si) && si.nMax > 0 && si.nPage > 0)
			bAtEnd = si.nPos + static_cast<int>(si.nPage) >= si.nMax - 1;
		m_edit.SetSel(iLength, iLength);
		m_edit.ReplaceSel(strNewLines);
		if (bSelectAll)
			m_edit.SetSel(0, -1);
		else if (!bAtEnd)
			m_edit.LineScroll(iFirstVisible - m_edit.GetFirstVisibleLine());
	}
	m_status.SetWindowText(strStatus);
}

// The standard edit does not treat Ctrl+A as Select All. Select the whole
// log so it can be copied. A later append keeps that full selection.
BOOL CTapiEventLogDlg::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == 'A' && (GetKeyState(VK_CONTROL) & 0x8000) != 0)
	{
		if (m_edit.GetSafeHwnd() != NULL)
		{
			m_edit.SetFocus();
			m_edit.SetSel(0, -1);
			return TRUE;
		}
	}
	return CDialog::PreTranslateMessage(pMsg);
}

void CTapiEventLogDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);
	LayoutControls(cx, cy);
}

// Text on top, Copy and Save on the left of the row under it, Clear on the
// right of that row, status line along the bottom edge.
void CTapiEventLogDlg::LayoutControls(int cx, int cy)
{
	if (m_edit.GetSafeHwnd() == NULL || m_status.GetSafeHwnd() == NULL)
		return;
	if (m_copy.GetSafeHwnd() == NULL || m_save.GetSafeHwnd() == NULL || m_clear.GetSafeHwnd() == NULL)
		return;
	const int iStatus = 22;
	const int iButtons = 26;
	const int iGap = 4;
	int iText = cy - iStatus - iButtons - iGap;
	if (iText < 0)
		iText = 0;
	m_edit.MoveWindow(0, 0, cx, iText);
	int iButtonY = iText + 2;
	const int iButtonW = 72;
	const int iButtonH = 22;
	m_copy.MoveWindow(4, iButtonY, iButtonW, iButtonH);
	m_save.MoveWindow(8 + iButtonW, iButtonY, iButtonW, iButtonH);
	m_clear.MoveWindow(cx - iButtonW - 4, iButtonY, iButtonW, iButtonH);
	m_status.MoveWindow(0, cy - iStatus, cx, iStatus);
}

// Copies the whole log. The edit's Copy only copies the selection, and
// clicking this button would otherwise leave that selection empty.
void CTapiEventLogDlg::OnCopy()
{
	m_edit.SetSel(0, -1);
	m_edit.Copy();
}

// Writes the visible log as UTF-8. The save dialog runs on this window's
// thread. Cancel leaves the file untouched. A failed create is reported
// and does not change the log.
void CTapiEventLogDlg::OnSave()
{
	CString strText;
	m_edit.GetWindowText(strText);
	CFileDialog dlg(FALSE, L"txt", L"tapi-events.txt",
		OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST,
		L"Text files (*.txt)|*.txt||", this);
	if (dlg.DoModal() != IDOK)
		return;

	CFile file;
	if (!file.Open(dlg.GetPathName(), CFile::modeCreate | CFile::modeWrite))
	{
		AfxMessageBox(L"Could not save the log.", MB_ICONEXCLAMATION);
		return;
	}

	static const BYTE bom[] = {0xEF, 0xBB, 0xBF};
	file.Write(bom, sizeof(bom));
	int nChars = strText.GetLength();
	int nBytes = WideCharToMultiByte(CP_UTF8, 0, strText, nChars, NULL, 0, NULL, NULL);
	if (nBytes <= 0)
		return;

	CStringA utf8;
	LPSTR pUtf8 = utf8.GetBuffer(nBytes);
	WideCharToMultiByte(CP_UTF8, 0, strText, nChars, pUtf8, nBytes, NULL, NULL);
	utf8.ReleaseBuffer(nBytes);
	file.Write(static_cast<LPCSTR>(utf8), nBytes);
}

// Drops the stored lines and the text in the window. The message counter
// keeps counting, so the next line stays in sequence.
void CTapiEventLogDlg::OnClear()
{
	TapiEventLogClear();
	m_edit.SetWindowText(L"");
}

void CTapiEventLogDlg::OnClose()
{
	DestroyWindow();
}

void CTapiEventLogDlg::PostNcDestroy()
{
	g_hLogWnd = NULL;
	InterlockedExchange(&g_lOpening, 0);
	CDialog::PostNcDestroy();
	delete this;
	AfxPostQuitMessage(0);
}
