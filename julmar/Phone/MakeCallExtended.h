#pragma once

#include "resource.h"

// MakeCallExtended dialog

class MakeCallExtended : public CDialog
{
	DECLARE_DYNAMIC(MakeCallExtended)

public:
	MakeCallExtended(CWnd* pParent = NULL); // standard constructor
	virtual ~MakeCallExtended();

	// Dialog Data
	enum
	{
		IDD = IDD_MAKECALLEXTENDED
	};

	CString m_strCallingID;
	CString m_strCalledID;
	bool m_bBlockMyCallingID;
	bool m_bSetDestinationAutoOffHook;

	// Address Capabilities
	DWORD dwAddressCapsFlags;

protected:
	virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support

	DECLARE_MESSAGE_MAP()
	virtual void OnOK();

public:
	virtual BOOL OnInitDialog();
};
