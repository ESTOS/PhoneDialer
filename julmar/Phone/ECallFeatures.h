#pragma once

// ECallFeatures dialog

#include "resource.h"

class ECallFeatures : public CDialog
{
	DECLARE_DYNAMIC(ECallFeatures)

public:
	ECallFeatures(CWnd* pParent = NULL); // standard constructor
	virtual ~ECallFeatures();

	void SetCallFeatures(const CString& strCallFeatures);
	// Dialog Data
	enum
	{
		IDD = IDD_CALLFEATURES
	};

protected:
	virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};
