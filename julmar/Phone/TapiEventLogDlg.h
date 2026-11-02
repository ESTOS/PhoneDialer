#pragma once

// Modeless TAPI trace. Lives on its own UI thread so the status line can
// refresh while the phone window is inside a TAPI call.

class CTapiEventLogDlg : public CDialog
{
public:
	CTapiEventLogDlg();

	// Shows the log window, creating its thread the first time.
	// Called from the phone dialog's Log button.
	static void Open();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual void PostNcDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnClose();
	afx_msg void OnCopy();
	afx_msg void OnSave();
	afx_msg void OnClear();
	DECLARE_MESSAGE_MAP()

	void LayoutControls(int cx, int cy);

	CEdit m_edit;
	CStatic m_status;
	CButton m_copy;
	CButton m_save;
	CButton m_clear;
};
