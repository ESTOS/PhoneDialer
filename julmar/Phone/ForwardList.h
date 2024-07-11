#pragma once
#include <list>
#include "afxcmn.h"
#include "resource.h"

class CPhoneDlg;
// CForwardList dialog

class ETapiForwardEntry
{
public:
	ETapiForwardEntry()
	{
		dwInternalID = 0;
		dwForwardMode = 0;
		dwAddressID = 0;
		dwNumRingsNoAnswer = 0;
		memset(szDestination, 0x00, sizeof(szDestination));
		memset(szCallerID, 0x00, sizeof(szCallerID));
		memset(szDialableAddress, 0x00, sizeof(szDialableAddress));
	}

	// Foridentification of this entry
	DWORD dwInternalID;

	DWORD dwForwardMode;
	// Destination. If Destination is empty then its a donotdisturb.
	TCHAR szDestination[100];
	TCHAR szCallerID[100];
	TCHAR szDialableAddress[100];
	DWORD dwAddressID;
	DWORD dwNumRingsNoAnswer;
};

typedef std::list<ETapiForwardEntry> ETapiForwardEntryList;

class ETapiForwardList
{
public:
	ETapiForwardList()
	{
		m_dwEntryCounter = 0;
	}

	LPLINEFORWARDLIST GetLineForwardList(const DWORD dwAddressID)
	{
		DWORD dwTotalSize = sizeof(LINEFORWARDLIST);
		DWORD dwNumEntries = 0;
		ETapiForwardEntryList::iterator it;
		for (it = m_ForwardEntryList.begin(); it != m_ForwardEntryList.end(); it++)
		{
			if (it->dwAddressID == dwAddressID || dwAddressID == 0xffffffff)
			{
				dwTotalSize += sizeof(LINEFORWARD);
				if (_tcslen(it->szDestination))
					dwTotalSize += (DWORD)(_tcslen(it->szDestination) + 1) * sizeof(TCHAR);

				if (_tcslen(it->szCallerID))
					dwTotalSize += (DWORD)(_tcslen(it->szCallerID) + 1) * sizeof(TCHAR);

				dwNumEntries++;
			}
		}
		LPLINEFORWARDLIST plist = (LPLINEFORWARDLIST)malloc(dwTotalSize);
		if (!plist)
			return NULL;
		memset(plist, 0x00, dwTotalSize);

		plist->dwTotalSize = dwTotalSize;
		plist->dwNumEntries = dwNumEntries;

		DWORD dwUsedSize = sizeof(LINEFORWARDLIST);
		if (dwNumEntries)
			dwUsedSize += sizeof(LINEFORWARD) * (dwNumEntries - 1);

		int iCounter = 0;
		for (it = m_ForwardEntryList.begin(); it != m_ForwardEntryList.end(); it++)
		{
			if (it->dwAddressID == dwAddressID || dwAddressID == 0xffffffff)
			{
				LINEFORWARD* plf = &plist->ForwardList[iCounter];
				plf->dwForwardMode = it->dwForwardMode;

				int iLen = (int)_tcslen(it->szCallerID);
				if (iLen)
				{
					plf->dwCallerAddressOffset = dwUsedSize;
					plf->dwCallerAddressSize = (iLen + 1) * sizeof(TCHAR);

					lstrcpyn((LPTSTR)((LPBYTE)plist + dwUsedSize), it->szCallerID, plf->dwCallerAddressSize);
					dwUsedSize += plf->dwCallerAddressSize;
				}
				iLen = (int)_tcslen(it->szDestination);
				if (iLen)
				{
					plf->dwDestAddressOffset = dwUsedSize;
					plf->dwDestAddressSize = (iLen + 1) * sizeof(TCHAR);

					lstrcpyn((LPTSTR)((LPBYTE)plist + dwUsedSize), it->szDestination, plf->dwDestAddressSize);
					dwUsedSize += plf->dwDestAddressSize;
				}
				iCounter++;
			}
		}
		return plist;
	}

	void ClearList()
	{
		m_ForwardEntryList.clear();
	}

	bool AddAddressStatus(LPLINEADDRESSSTATUS lpStatus, CString strDialableAddress, const DWORD dwAddressID)
	{
		// Alte Einträge mit dieser AdressID entfernen
		ETapiForwardEntryList::iterator iter;
		iter = m_ForwardEntryList.begin();
		while (iter != m_ForwardEntryList.end())
			if (iter->dwAddressID == dwAddressID)
				iter = m_ForwardEntryList.erase(iter);
			else
				iter++;

		if (lpStatus && lpStatus->dwForwardNumEntries && lpStatus->dwForwardOffset && lpStatus->dwForwardSize)
		{
			DWORD i = 0;
			for (i = 0; i < lpStatus->dwForwardNumEntries; i++)
			{
				LINEFORWARD* pForward = (LINEFORWARD*)((((BYTE*)lpStatus) + lpStatus->dwForwardOffset) + i * sizeof(LINEFORWARD));

				ETapiForwardEntry entry;
				entry.dwInternalID = GetNextEntryID();
				entry.dwAddressID = dwAddressID;
				entry.dwForwardMode = pForward->dwForwardMode;
				if (entry.dwForwardMode == LINEFORWARDMODE_NOANSW || entry.dwForwardMode == LINEFORWARDMODE_NOANSWINTERNAL || entry.dwForwardMode == LINEFORWARDMODE_NOANSWEXTERNAL || entry.dwForwardMode == LINEFORWARDMODE_NOANSWSPECIFIC)
					entry.dwNumRingsNoAnswer = lpStatus->dwNumRingsNoAnswer;

				if (pForward->dwDestAddressOffset && pForward->dwDestAddressSize)
				{
					memset(entry.szDestination, 0x00, sizeof(entry.szDestination));
					TCHAR* pDestAddr = (TCHAR*)(((BYTE*)lpStatus) + pForward->dwDestAddressOffset);
					memcpy(entry.szDestination, pDestAddr, min(99, pForward->dwDestAddressSize));
				}
				if (pForward->dwCallerAddressOffset && pForward->dwCallerAddressSize)
				{
					memset(entry.szCallerID, 0x00, sizeof(entry.szDestination));
					TCHAR* pDestAddr = (TCHAR*)(((BYTE*)lpStatus) + pForward->dwCallerAddressOffset);
					memcpy(entry.szCallerID, pDestAddr, min(99, pForward->dwCallerAddressSize));
				}

				lstrcpyn(entry.szDialableAddress, strDialableAddress, 99);
				m_ForwardEntryList.push_back(entry);
			}

			return true;
		}
		return false;
	}

	ETapiForwardEntryList m_ForwardEntryList;

	DWORD GetNextEntryID()
	{
		return m_dwEntryCounter++;
	}

private:
	DWORD m_dwEntryCounter;
};

class CForwardList : public CDialog
{
	DECLARE_DYNAMIC(CForwardList)

public:
	CForwardList(CWnd* pParent, CTapiLine* pLine); // standard constructor
	virtual ~CForwardList();

	// Dialog Data
	enum
	{
		IDD = IDD_FORWARDLIST
	};

	void UpdateList();

protected:
	virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support

	DECLARE_MESSAGE_MAP()
private:
	CListCtrl m_List;
	TCHAR* GetForwardModeString(DWORD dwForwardMode);
	CTapiLine* m_pLine;
	ETapiForwardList m_ForwardList;
	DWORD m_dwAddressCount;

public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedCmdadd();
	afx_msg void OnBnClickedCmdremove();
	afx_msg void OnBnClickedCmdremoveall();
};
