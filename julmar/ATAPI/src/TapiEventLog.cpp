#include "stdafx.h"
#include "TapiEventLog.h"
#include "ecstaextensions.h"

#include <deque>
#include <vector>

namespace
{
	const size_t MaxLines = 3000;

	struct Operation
	{
		DWORD threadId;
		DWORD tick;
		CString name;
		bool hasResult;
		LONG result;
	};

	CCriticalSection g_lock;
	std::deque<CString> g_lines;
	size_t g_shown = 0;
	DWORD g_processed = 0;
	// Thread that waits in lineGetMessage. Only that thread's other TAPI
	// calls are written as enter/leave lines, so the UI poll does not flood the log.
	DWORD g_eventThreadId = 0;
	std::vector<Operation> g_operations;

	// Name of one LINECALLSTATE_* bit. Empty when the bit is not one we print.
	const wchar_t* CallStateName(DWORD dwState)
	{
		if (dwState & LINECALLSTATE_IDLE)
			return L"LINECALLSTATE_IDLE";
		if (dwState & LINECALLSTATE_OFFERING)
			return L"LINECALLSTATE_OFFERING";
		if (dwState & LINECALLSTATE_ACCEPTED)
			return L"LINECALLSTATE_ACCEPTED";
		if (dwState & LINECALLSTATE_DIALTONE)
			return L"LINECALLSTATE_DIALTONE";
		if (dwState & LINECALLSTATE_DIALING)
			return L"LINECALLSTATE_DIALING";
		if (dwState & LINECALLSTATE_RINGBACK)
			return L"LINECALLSTATE_RINGBACK";
		if (dwState & LINECALLSTATE_BUSY)
			return L"LINECALLSTATE_BUSY";
		if (dwState & LINECALLSTATE_CONFERENCED)
			return L"LINECALLSTATE_CONFERENCED";
		if (dwState & LINECALLSTATE_CONNECTED)
			return L"LINECALLSTATE_CONNECTED";
		if (dwState & LINECALLSTATE_PROCEEDING)
			return L"LINECALLSTATE_PROCEEDING";
		if (dwState & LINECALLSTATE_ONHOLD)
			return L"LINECALLSTATE_ONHOLD";
		if (dwState & LINECALLSTATE_ONHOLDPENDCONF)
			return L"LINECALLSTATE_ONHOLDPENDCONF";
		if (dwState & LINECALLSTATE_ONHOLDPENDTRANSFER)
			return L"LINECALLSTATE_ONHOLDPENDTRANSFER";
		if (dwState & LINECALLSTATE_DISCONNECTED)
			return L"LINECALLSTATE_DISCONNECTED";
		if (dwState & LINECALLSTATE_SPECIALINFO)
			return L"LINECALLSTATE_SPECIALINFO";
		if (dwState & LINECALLSTATE_UNKNOWN)
			return L"LINECALLSTATE_UNKNOWN";
		return L"";
	}

	const wchar_t* MessageName(DWORD dwMessage)
	{
		switch (dwMessage)
		{
			case LINE_ADDRESSSTATE:
				return L"LINE_ADDRESSSTATE";
			case LINE_CALLINFO:
				return L"LINE_CALLINFO";
			case LINE_CALLSTATE:
				return L"LINE_CALLSTATE";
			case LINE_CLOSE:
				return L"LINE_CLOSE";
			case LINE_DEVSPECIFIC:
				return L"LINE_DEVSPECIFIC";
			case LINE_DEVSPECIFICFEATURE:
				return L"LINE_DEVSPECIFICFEATURE";
			case LINE_GATHERDIGITS:
				return L"LINE_GATHERDIGITS";
			case LINE_GENERATE:
				return L"LINE_GENERATE";
			case LINE_LINEDEVSTATE:
				return L"LINE_LINEDEVSTATE";
			case LINE_MONITORDIGITS:
				return L"LINE_MONITORDIGITS";
			case LINE_MONITORMEDIA:
				return L"LINE_MONITORMEDIA";
			case LINE_MONITORTONE:
				return L"LINE_MONITORTONE";
			case LINE_REPLY:
				return L"LINE_REPLY";
			case LINE_REQUEST:
				return L"LINE_REQUEST";
			case LINE_APPNEWCALL:
				return L"LINE_APPNEWCALL";
			case LINE_CREATE:
				return L"LINE_CREATE";
			case LINE_REMOVE:
				return L"LINE_REMOVE";
			default:
				return NULL;
		}
	}

	// Local time in the same shape tapicaps writes at the start of a trace line.
	CString TimeStamp()
	{
		SYSTEMTIME time;
		GetLocalTime(&time);
		CString strTime;
		strTime.Format(L"%02u:%02u:%02u.%03u", time.wHour, time.wMinute, time.wSecond, time.wMilliseconds);
		return strTime;
	}

	CString CallStateMode(DWORD dwState, DWORD dwMode);
	CString CallInfoNames(DWORD dwInfo);
	CString AddressStateNames(DWORD dwState);

	// One trace line for a LINEMESSAGE. Header text matches the tapicaps
	// trace item. Child lines are added later by TapiEventLogAddLineMessage.
	CString FormatMessage(const LINEMESSAGE* pMessage)
	{
		const wchar_t* szName = MessageName(pMessage->dwMessageID);
		CString strLine;
		if (pMessage->dwMessageID == LINE_CALLSTATE)
		{
			const wchar_t* szState = CallStateName((DWORD)pMessage->dwParam1);
			strLine.Format(L"%s LINE_CALLSTATE: hCall: %08lX %s %s",
				TimeStamp().GetString(),
				(unsigned long)pMessage->hDevice,
				szState != NULL && szState[0] != 0 ? szState : L"LINECALLSTATE_?",
				CallStateMode((DWORD)pMessage->dwParam1, (DWORD)pMessage->dwParam2).GetString());
			return strLine;
		}
		if (pMessage->dwMessageID == LINE_CALLINFO)
		{
			strLine.Format(L"%s LINE_CALLINFO: hCall: %08lX %s",
				TimeStamp().GetString(),
				(unsigned long)pMessage->hDevice,
				CallInfoNames((DWORD)pMessage->dwParam1).GetString());
			return strLine;
		}
		if (pMessage->dwMessageID == LINE_APPNEWCALL)
		{
			strLine.Format(L"%s LINE_APPNEWCALL: hCall: %08lX",
				TimeStamp().GetString(),
				(unsigned long)pMessage->dwParam2);
			return strLine;
		}
		if (pMessage->dwMessageID == LINE_ADDRESSSTATE)
		{
			strLine.Format(L"%s LINE_ADDRESSSTATE: %s",
				TimeStamp().GetString(),
				AddressStateNames((DWORD)pMessage->dwParam2).GetString());
			return strLine;
		}
		if (pMessage->dwMessageID == LINE_CLOSE)
		{
			strLine.Format(L"%s LINE_CLOSE", TimeStamp().GetString());
			return strLine;
		}
		if (pMessage->dwMessageID == LINE_REPLY)
		{
			strLine.Format(L"%s LINE_REPLY : %08lX",
				TimeStamp().GetString(),
				(unsigned long)pMessage->dwParam1);
			return strLine;
		}
		if (pMessage->dwMessageID == LINE_DEVSPECIFIC)
		{
			strLine.Format(L"%s LINE_DEVSPECIFIC: hDevice: %08lX Param1: %08lX Param2: %08lX Param3: %08lX",
				TimeStamp().GetString(),
				(unsigned long)pMessage->hDevice,
				(unsigned long)pMessage->dwParam1,
				(unsigned long)pMessage->dwParam2,
				(unsigned long)pMessage->dwParam3);
			return strLine;
		}
		if (szName != NULL)
		{
			strLine.Format(L"%s %s hDevice=%08lX p1=%08lX p2=%08lX p3=%08lX",
				TimeStamp().GetString(),
				szName,
				(unsigned long)pMessage->hDevice,
				(unsigned long)pMessage->dwParam1,
				(unsigned long)pMessage->dwParam2,
				(unsigned long)pMessage->dwParam3);
			return strLine;
		}
		strLine.Format(L"%s LINE_ %08lX hDevice=%08lX p1=%08lX p2=%08lX p3=%08lX",
			TimeStamp().GetString(),
			pMessage->dwMessageID,
			(unsigned long)pMessage->hDevice,
			(unsigned long)pMessage->dwParam1,
			(unsigned long)pMessage->dwParam2,
			(unsigned long)pMessage->dwParam3);
		return strLine;
	}

	// Status text for every TAPI call that has entered and not yet left.
	// Called with g_lock held. The message number is on each log line.
	CString CurrentStatus()
	{
		if (g_operations.empty())
			return L"idle";

		DWORD dwNow = GetTickCount();
		CString strStatus;
		for (size_t i = 0; i < g_operations.size(); ++i)
		{
			CString strOne;
			strOne.Format(L"inside %s for %lu ms", g_operations[i].name.GetString(), dwNow - g_operations[i].tick);
			if (!strStatus.IsEmpty())
				strStatus += L"; ";
			strStatus += strOne;
		}
		return strStatus;
	}

	// Appends one log line. Caller holds g_lock. Drops the oldest line past MaxLines
	// and keeps the "already shown" index aligned with that drop.
	void PushLine(const CString& strLine)
	{
		g_lines.push_back(strLine);
		while (g_lines.size() > MaxLines)
		{
			g_lines.pop_front();
			if (g_shown > 0)
				--g_shown;
		}
	}

	// Party number stored at a byte offset inside a variable-length TAPI struct.
	// dwSize is bytes, including the trailing NUL when TAPI supplies one.
	// Returns empty when the offset or size is zero. Does not read past dwSize.
	CString StructString(const void* pBase, DWORD dwOffset, DWORD dwSize)
	{
		if (pBase == NULL || dwOffset == 0 || dwSize < sizeof(wchar_t))
			return CString();
		const wchar_t* pText = (const wchar_t*)((const BYTE*)pBase + dwOffset);
		int iChars = (int)(dwSize / sizeof(wchar_t));
		CString strText(pText, iChars);
		int iEnd = strText.GetLength();
		while (iEnd > 0 && strText[iEnd - 1] == L'\0')
			--iEnd;
		return strText.Left(iEnd);
	}

	CString CallStateMode(DWORD dwState, DWORD dwMode)
	{
		CString strMode;
		if (dwState == LINECALLSTATE_CONNECTED)
		{
			if (dwMode & LINECONNECTEDMODE_ACTIVE)
				strMode += L"LINECONNECTEDMODE_ACTIVE ";
			if (dwMode & LINECONNECTEDMODE_INACTIVE)
				strMode += L"LINECONNECTEDMODE_INACTIVE ";
			if (dwMode & LINECONNECTEDMODE_ACTIVEHELD)
				strMode += L"LINECONNECTEDMODE_ACTIVEHELD ";
			if (dwMode & LINECONNECTEDMODE_INACTIVEHELD)
				strMode += L"LINECONNECTEDMODE_INACTIVEHELD ";
			if (dwMode & LINECONNECTEDMODE_CONFIRMED)
				strMode += L"LINECONNECTEDMODE_CONFIRMED ";
		}
		else if (dwState == LINECALLSTATE_DIALTONE)
		{
			if (dwMode & LINEDIALTONEMODE_NORMAL)
				strMode += L"LINEDIALTONEMODE_NORMAL ";
			if (dwMode & LINEDIALTONEMODE_SPECIAL)
				strMode += L"LINEDIALTONEMODE_SPECIAL ";
			if (dwMode & LINEDIALTONEMODE_INTERNAL)
				strMode += L"LINEDIALTONEMODE_INTERNAL ";
			if (dwMode & LINEDIALTONEMODE_EXTERNAL)
				strMode += L"LINEDIALTONEMODE_EXTERNAL ";
			if (dwMode & LINEDIALTONEMODE_UNKNOWN)
				strMode += L"LINEDIALTONEMODE_UNKNOWN ";
			if (dwMode & LINEDIALTONEMODE_UNAVAIL)
				strMode += L"LINEDIALTONEMODE_UNAVAIL ";
		}
		else if (dwState == LINECALLSTATE_BUSY)
		{
			if (dwMode & LINEBUSYMODE_STATION)
				strMode += L"LINEBUSYMODE_STATION ";
			if (dwMode & LINEBUSYMODE_TRUNK)
				strMode += L"LINEBUSYMODE_TRUNK ";
			if (dwMode & LINEBUSYMODE_UNKNOWN)
				strMode += L"LINEBUSYMODE_UNKNOWN ";
			if (dwMode & LINEBUSYMODE_UNAVAIL)
				strMode += L"LINEBUSYMODE_UNAVAIL ";
		}
		else if (dwState == LINECALLSTATE_DISCONNECTED)
		{
			if (dwMode & LINEDISCONNECTMODE_NORMAL)
				strMode += L"LINEDISCONNECTMODE_NORMAL ";
			if (dwMode & LINEDISCONNECTMODE_UNKNOWN)
				strMode += L"LINEDISCONNECTMODE_UNKNOWN ";
			if (dwMode & LINEDISCONNECTMODE_REJECT)
				strMode += L"LINEDISCONNECTMODE_REJECT ";
			if (dwMode & LINEDISCONNECTMODE_PICKUP)
				strMode += L"LINEDISCONNECTMODE_PICKUP ";
			if (dwMode & LINEDISCONNECTMODE_FORWARDED)
				strMode += L"LINEDISCONNECTMODE_FORWARDED ";
			if (dwMode & LINEDISCONNECTMODE_BUSY)
				strMode += L"LINEDISCONNECTMODE_BUSY ";
			if (dwMode & LINEDISCONNECTMODE_NOANSWER)
				strMode += L"LINEDISCONNECTMODE_NOANSWER ";
			if (dwMode & LINEDISCONNECTMODE_BADADDRESS)
				strMode += L"LINEDISCONNECTMODE_BADADDRESS ";
			if (dwMode & LINEDISCONNECTMODE_UNREACHABLE)
				strMode += L"LINEDISCONNECTMODE_UNREACHABLE ";
			if (dwMode & LINEDISCONNECTMODE_CONGESTION)
				strMode += L"LINEDISCONNECTMODE_CONGESTION ";
			if (dwMode & LINEDISCONNECTMODE_INCOMPATIBLE)
				strMode += L"LINEDISCONNECTMODE_INCOMPATIBLE ";
			if (dwMode & LINEDISCONNECTMODE_UNAVAIL)
				strMode += L"LINEDISCONNECTMODE_UNAVAIL ";
			if (dwMode & LINEDISCONNECTMODE_NODIALTONE)
				strMode += L"LINEDISCONNECTMODE_NODIALTONE ";
			if (dwMode & LINEDISCONNECTMODE_CANCELLED)
				strMode += L"LINEDISCONNECTMODE_CANCELLED ";
		}
		else if (dwState == LINECALLSTATE_OFFERING)
		{
			if (dwMode & LINEOFFERINGMODE_ACTIVE)
				strMode += L"LINEOFFERINGMODE_ACTIVE ";
			if (dwMode & LINEOFFERINGMODE_INACTIVE)
				strMode += L"LINEOFFERINGMODE_INACTIVE ";
		}
		else if (dwState == LINECALLSTATE_SPECIALINFO)
		{
			if (dwMode & LINESPECIALINFO_NOCIRCUIT)
				strMode += L"LINESPECIALINFO_NOCIRCUIT ";
			if (dwMode & LINESPECIALINFO_CUSTIRREG)
				strMode += L"LINESPECIALINFO_CUSTIRREG ";
			if (dwMode & LINESPECIALINFO_REORDER)
				strMode += L"LINESPECIALINFO_REORDER ";
			if (dwMode & LINESPECIALINFO_UNKNOWN)
				strMode += L"LINESPECIALINFO_UNKNOWN ";
			if (dwMode & LINESPECIALINFO_UNAVAIL)
				strMode += L"LINESPECIALINFO_UNAVAIL ";
		}
		if (strMode.IsEmpty())
			strMode.Format(L"0x%08lX", dwMode);
		return strMode;
	}

	// LINECALLFEATURE_* names, same set tapicaps prints under Features.
	CString FeatureNames(DWORD dwFeatures)
	{
		CString strNames;
		if (dwFeatures & LINECALLFEATURE_ACCEPT)
			strNames += L"LINECALLFEATURE_ACCEPT ";
		if (dwFeatures & LINECALLFEATURE_ADDTOCONF)
			strNames += L"LINECALLFEATURE_ADDTOCONF ";
		if (dwFeatures & LINECALLFEATURE_ANSWER)
			strNames += L"LINECALLFEATURE_ANSWER ";
		if (dwFeatures & LINECALLFEATURE_BLINDTRANSFER)
			strNames += L"LINECALLFEATURE_BLINDTRANSFER ";
		if (dwFeatures & LINECALLFEATURE_COMPLETECALL)
			strNames += L"LINECALLFEATURE_COMPLETECALL ";
		if (dwFeatures & LINECALLFEATURE_COMPLETETRANSF)
			strNames += L"LINECALLFEATURE_COMPLETETRANSF ";
		if (dwFeatures & LINECALLFEATURE_DIAL)
			strNames += L"LINECALLFEATURE_DIAL ";
		if (dwFeatures & LINECALLFEATURE_DROP)
			strNames += L"LINECALLFEATURE_DROP ";
		if (dwFeatures & LINECALLFEATURE_GATHERDIGITS)
			strNames += L"LINECALLFEATURE_GATHERDIGITS ";
		if (dwFeatures & LINECALLFEATURE_GENERATEDIGITS)
			strNames += L"LINECALLFEATURE_GENERATEDIGITS ";
		if (dwFeatures & LINECALLFEATURE_GENERATETONE)
			strNames += L"LINECALLFEATURE_GENERATETONE ";
		if (dwFeatures & LINECALLFEATURE_HOLD)
			strNames += L"LINECALLFEATURE_HOLD ";
		if (dwFeatures & LINECALLFEATURE_MONITORDIGITS)
			strNames += L"LINECALLFEATURE_MONITORDIGITS ";
		if (dwFeatures & LINECALLFEATURE_MONITORMEDIA)
			strNames += L"LINECALLFEATURE_MONITORMEDIA ";
		if (dwFeatures & LINECALLFEATURE_MONITORTONES)
			strNames += L"LINECALLFEATURE_MONITORTONES ";
		if (dwFeatures & LINECALLFEATURE_PARK)
			strNames += L"LINECALLFEATURE_PARK ";
		if (dwFeatures & LINECALLFEATURE_PREPAREADDCONF)
			strNames += L"LINECALLFEATURE_PREPAREADDCONF ";
		if (dwFeatures & LINECALLFEATURE_REDIRECT)
			strNames += L"LINECALLFEATURE_REDIRECT ";
		if (dwFeatures & LINECALLFEATURE_REMOVEFROMCONF)
			strNames += L"LINECALLFEATURE_REMOVEFROMCONF ";
		if (dwFeatures & LINECALLFEATURE_SECURECALL)
			strNames += L"LINECALLFEATURE_SECURECALL ";
		if (dwFeatures & LINECALLFEATURE_SENDUSERUSER)
			strNames += L"LINECALLFEATURE_SENDUSERUSER ";
		if (dwFeatures & LINECALLFEATURE_SETCALLDATA)
			strNames += L"LINECALLFEATURE_SETCALLDATA ";
		if (dwFeatures & LINECALLFEATURE_SETCALLPARAMS)
			strNames += L"LINECALLFEATURE_SETCALLPARAMS ";
		if (dwFeatures & LINECALLFEATURE_SETMEDIACONTROL)
			strNames += L"LINECALLFEATURE_SETMEDIACONTROL ";
		if (dwFeatures & LINECALLFEATURE_SETQOS)
			strNames += L"LINECALLFEATURE_SETQOS ";
		if (dwFeatures & LINECALLFEATURE_SETTERMINAL)
			strNames += L"LINECALLFEATURE_SETTERMINAL ";
		if (dwFeatures & LINECALLFEATURE_SETUPCONF)
			strNames += L"LINECALLFEATURE_SETUPCONF ";
		if (dwFeatures & LINECALLFEATURE_SETUPTRANSFER)
			strNames += L"LINECALLFEATURE_SETUPTRANSFER ";
		if (dwFeatures & LINECALLFEATURE_SWAPHOLD)
			strNames += L"LINECALLFEATURE_SWAPHOLD ";
		if (dwFeatures & LINECALLFEATURE_UNHOLD)
			strNames += L"LINECALLFEATURE_UNHOLD ";
		return strNames;
	}

	// LINECALLINFOSTATE_* names from the LINE_CALLINFO parameter.
	CString CallInfoNames(DWORD dwInfo)
	{
		CString strNames;
		if (dwInfo & LINECALLINFOSTATE_APPSPECIFIC)
			strNames += L"LINECALLINFOSTATE_APPSPECIFIC ";
		if (dwInfo & LINECALLINFOSTATE_BEARERMODE)
			strNames += L"LINECALLINFOSTATE_BEARERMODE ";
		if (dwInfo & LINECALLINFOSTATE_CALLDATA)
			strNames += L"LINECALLINFOSTATE_CALLDATA ";
		if (dwInfo & LINECALLINFOSTATE_CALLEDID)
			strNames += L"LINECALLINFOSTATE_CALLEDID ";
		if (dwInfo & LINECALLINFOSTATE_CALLERID)
			strNames += L"LINECALLINFOSTATE_CALLERID ";
		if (dwInfo & LINECALLINFOSTATE_CALLID)
			strNames += L"LINECALLINFOSTATE_CALLID ";
		if (dwInfo & LINECALLINFOSTATE_CHARGINGINFO)
			strNames += L"LINECALLINFOSTATE_CHARGINGINFO ";
		if (dwInfo & LINECALLINFOSTATE_COMPLETIONID)
			strNames += L"LINECALLINFOSTATE_COMPLETIONID ";
		if (dwInfo & LINECALLINFOSTATE_CONNECTEDID)
			strNames += L"LINECALLINFOSTATE_CONNECTEDID ";
		if (dwInfo & LINECALLINFOSTATE_DEVSPECIFIC)
			strNames += L"LINECALLINFOSTATE_DEVSPECIFIC ";
		if (dwInfo & LINECALLINFOSTATE_DIALPARAMS)
			strNames += L"LINECALLINFOSTATE_DIALPARAMS ";
		if (dwInfo & LINECALLINFOSTATE_DISPLAY)
			strNames += L"LINECALLINFOSTATE_DISPLAY ";
		if (dwInfo & LINECALLINFOSTATE_HIGHLEVELCOMP)
			strNames += L"LINECALLINFOSTATE_HIGHLEVELCOMP ";
		if (dwInfo & LINECALLINFOSTATE_LOWLEVELCOMP)
			strNames += L"LINECALLINFOSTATE_LOWLEVELCOMP ";
		if (dwInfo & LINECALLINFOSTATE_MEDIAMODE)
			strNames += L"LINECALLINFOSTATE_MEDIAMODE ";
		if (dwInfo & LINECALLINFOSTATE_MONITORMODES)
			strNames += L"LINECALLINFOSTATE_MONITORMODES ";
		if (dwInfo & LINECALLINFOSTATE_NUMMONITORS)
			strNames += L"LINECALLINFOSTATE_NUMMONITORS ";
		if (dwInfo & LINECALLINFOSTATE_NUMOWNERDECR)
			strNames += L"LINECALLINFOSTATE_NUMOWNERDECR ";
		if (dwInfo & LINECALLINFOSTATE_NUMOWNERINCR)
			strNames += L"LINECALLINFOSTATE_NUMOWNERINCR ";
		if (dwInfo & LINECALLINFOSTATE_ORIGIN)
			strNames += L"LINECALLINFOSTATE_ORIGIN ";
		if (dwInfo & LINECALLINFOSTATE_OTHER)
			strNames += L"LINECALLINFOSTATE_OTHER ";
		if (dwInfo & LINECALLINFOSTATE_QOS)
			strNames += L"LINECALLINFOSTATE_QOS ";
		if (dwInfo & LINECALLINFOSTATE_RATE)
			strNames += L"LINECALLINFOSTATE_RATE ";
		if (dwInfo & LINECALLINFOSTATE_REASON)
			strNames += L"LINECALLINFOSTATE_REASON ";
		if (dwInfo & LINECALLINFOSTATE_REDIRECTINGID)
			strNames += L"LINECALLINFOSTATE_REDIRECTINGID ";
		if (dwInfo & LINECALLINFOSTATE_REDIRECTIONID)
			strNames += L"LINECALLINFOSTATE_REDIRECTIONID ";
		if (dwInfo & LINECALLINFOSTATE_RELATEDCALLID)
			strNames += L"LINECALLINFOSTATE_RELATEDCALLID ";
		if (dwInfo & LINECALLINFOSTATE_TERMINAL)
			strNames += L"LINECALLINFOSTATE_TERMINAL ";
		if (dwInfo & LINECALLINFOSTATE_TREATMENT)
			strNames += L"LINECALLINFOSTATE_TREATMENT ";
		if (dwInfo & LINECALLINFOSTATE_TRUNK)
			strNames += L"LINECALLINFOSTATE_TRUNK ";
		if (dwInfo & LINECALLINFOSTATE_USERUSERINFO)
			strNames += L"LINECALLINFOSTATE_USERUSERINFO ";
		if (strNames.IsEmpty())
			strNames.Format(L"%08lX", dwInfo);
		return strNames;
	}

	// LINEADDRESSSTATE_* names from the LINE_ADDRESSSTATE parameter.
	CString AddressStateNames(DWORD dwState)
	{
		CString strNames;
		if (dwState & LINEADDRESSSTATE_CAPSCHANGE)
			strNames += L"LINEADDRESSSTATE_CAPSCHANGE ";
		if (dwState & LINEADDRESSSTATE_DEVSPECIFIC)
			strNames += L"LINEADDRESSSTATE_DEVSPECIFIC ";
		if (dwState & LINEADDRESSSTATE_FORWARD)
			strNames += L"LINEADDRESSSTATE_FORWARD ";
		if (dwState & LINEADDRESSSTATE_INUSEMANY)
			strNames += L"LINEADDRESSSTATE_INUSEMANY ";
		if (dwState & LINEADDRESSSTATE_INUSEONE)
			strNames += L"LINEADDRESSSTATE_INUSEONE ";
		if (dwState & LINEADDRESSSTATE_INUSEZERO)
			strNames += L"LINEADDRESSSTATE_INUSEZERO ";
		if (dwState & LINEADDRESSSTATE_NUMCALLS)
			strNames += L"LINEADDRESSSTATE_NUMCALLS ";
		if (dwState & LINEADDRESSSTATE_OTHER)
			strNames += L"LINEADDRESSSTATE_OTHER ";
		if (dwState & LINEADDRESSSTATE_TERMINALS)
			strNames += L"LINEADDRESSSTATE_TERMINALS ";
		if (strNames.IsEmpty())
			strNames.Format(L"%08lX", dwState);
		return strNames;
	}

	const wchar_t* OriginName(DWORD dwOrigin)
	{
		switch (dwOrigin)
		{
			case LINECALLORIGIN_OUTBOUND:
				return L"LINECALLORIGIN_OUTBOUND";
			case LINECALLORIGIN_INBOUND:
				return L"LINECALLORIGIN_INBOUND";
			case LINECALLORIGIN_INTERNAL:
				return L"LINECALLORIGIN_INTERNAL";
			case LINECALLORIGIN_EXTERNAL:
				return L"LINECALLORIGIN_EXTERNAL";
			case LINECALLORIGIN_CONFERENCE:
				return L"LINECALLORIGIN_CONFERENCE";
			case LINECALLORIGIN_UNAVAIL:
				return L"LINECALLORIGIN_UNAVAIL";
			case LINECALLORIGIN_UNKNOWN:
				return L"LINECALLORIGIN_UNKNOWN";
			default:
				return NULL;
		}
	}

	const wchar_t* ReasonName(DWORD dwReason)
	{
		switch (dwReason)
		{
			case LINECALLREASON_DIRECT:
				return L"Direct";
			case LINECALLREASON_FWDBUSY:
				return L"FwdBusy";
			case LINECALLREASON_FWDNOANSWER:
				return L"FwdNoAns";
			case LINECALLREASON_FWDUNCOND:
				return L"FwdUnc";
			case LINECALLREASON_TRANSFER:
				return L"Transfer";
			case LINECALLREASON_PICKUP:
				return L"Pickup";
			case LINECALLREASON_UNPARK:
				return L"Unpark";
			case LINECALLREASON_REDIRECT:
				return L"Redirect";
			case LINECALLREASON_CALLCOMPLETION:
				return L"CallComp";
			case LINECALLREASON_REMINDER:
				return L"Reminder";
			case LINECALLREASON_UNAVAIL:
				return L"Unavail";
			case LINECALLREASON_INTRUDE:
				return L"Intrude";
			case LINECALLREASON_PARKED:
				return L"Parked";
			case LINECALLREASON_CAMPEDON:
				return L"CampedOn";
			case LINECALLREASON_ROUTEREQUEST:
				return L"Route";
			case LINECALLREASON_UNKNOWN:
				return L"Unknown";
			default:
				return NULL;
		}
	}

	// lineGetCallStatus / lineGetCallInfo, growing the buffer the way tapicaps does.
	// The scope makes a stuck read show on the log status line.
	bool ReadCallStatus(HCALL hCall, std::vector<BYTE>& buffer)
	{
		DWORD dwSize = sizeof(LINECALLSTATUS) + 1024;
		for (int iTry = 0; iTry < 5; ++iTry)
		{
			buffer.assign(dwSize, 0);
			LPLINECALLSTATUS pStatus = (LPLINECALLSTATUS)buffer.data();
			pStatus->dwTotalSize = dwSize;
			CTapiEventLogScope scope(L"lineGetCallStatus");
			LONG lResult = lineGetCallStatus(hCall, pStatus);
			if (lResult != 0)
				return false;
			if (pStatus->dwNeededSize <= dwSize)
				return true;
			dwSize = pStatus->dwNeededSize;
		}
		return false;
	}

	bool ReadCallInfo(HCALL hCall, std::vector<BYTE>& buffer)
	{
		DWORD dwSize = sizeof(LINECALLINFO) + 2048;
		for (int iTry = 0; iTry < 5; ++iTry)
		{
			buffer.assign(dwSize, 0);
			LPLINECALLINFO pInfo = (LPLINECALLINFO)buffer.data();
			pInfo->dwTotalSize = dwSize;
			CTapiEventLogScope scope(L"lineGetCallInfo");
			LONG lResult = lineGetCallInfo(hCall, pInfo);
			if (lResult != 0)
				return false;
			if (pInfo->dwNeededSize <= dwSize)
				return true;
			dwSize = pInfo->dwNeededSize;
		}
		return false;
	}

	void AppendCallData(const LINECALLINFO* pInfo, std::vector<CString>& lines);
	void AppendDevSpecific(const LINECALLINFO* pInfo, std::vector<CString>& lines);

	// Child lines tapicaps hangs under a LINE_CALLSTATE or LINE_CALLINFO item.
	// Called without g_lock. The reads show on the status line while they run.
	void AppendCallDetails(HCALL hCall, std::vector<CString>& lines)
	{
		std::vector<BYTE> statusBytes;
		if (ReadCallStatus(hCall, statusBytes))
		{
			LPLINECALLSTATUS pStatus = (LPLINECALLSTATUS)statusBytes.data();
			CString strLine;
			const wchar_t* szState = CallStateName(pStatus->dwCallState);
			strLine.Format(L"    State: %s", szState != NULL && szState[0] != 0 ? szState : L"LINECALLSTATE_?");
			lines.push_back(strLine);
			if (pStatus->dwCallStateMode != 0)
			{
				strLine.Format(L"    CallState Detail: %s", CallStateMode(pStatus->dwCallState, pStatus->dwCallStateMode).GetString());
				lines.push_back(strLine);
			}
			if (pStatus->dwCallFeatures != 0)
			{
				strLine.Format(L"    Features: %s", FeatureNames(pStatus->dwCallFeatures).GetString());
				lines.push_back(strLine);
			}
		}

		std::vector<BYTE> infoBytes;
		if (!ReadCallInfo(hCall, infoBytes))
			return;

		LPLINECALLINFO pInfo = (LPLINECALLINFO)infoBytes.data();
		CString strLine;
		const wchar_t* szOrigin = OriginName(pInfo->dwOrigin);
		if (szOrigin == NULL)
			strLine.Format(L"    Origin: unknown %08lX", pInfo->dwOrigin);
		else
			strLine.Format(L"    Origin: %s", szOrigin);
		lines.push_back(strLine);
		const wchar_t* szReason = ReasonName(pInfo->dwReason);
		if (szReason == NULL)
			strLine.Format(L"    Type: unknown %08lX", pInfo->dwReason);
		else
			strLine.Format(L"    Type: %s", szReason);
		lines.push_back(strLine);

		strLine.Format(L"    CallerID: %s %s",
			StructString(pInfo, pInfo->dwCallerIDOffset, pInfo->dwCallerIDSize).GetString(),
			StructString(pInfo, pInfo->dwCallerIDNameOffset, pInfo->dwCallerIDNameSize).GetString());
		lines.push_back(strLine);
		strLine.Format(L"    CalledID: %s %s",
			StructString(pInfo, pInfo->dwCalledIDOffset, pInfo->dwCalledIDSize).GetString(),
			StructString(pInfo, pInfo->dwCalledIDNameOffset, pInfo->dwCalledIDNameSize).GetString());
		lines.push_back(strLine);
		strLine.Format(L"    ConnectedID: %s %s",
			StructString(pInfo, pInfo->dwConnectedIDOffset, pInfo->dwConnectedIDSize).GetString(),
			StructString(pInfo, pInfo->dwConnectedIDNameOffset, pInfo->dwConnectedIDNameSize).GetString());
		lines.push_back(strLine);

		CString strRedirecting = StructString(pInfo, pInfo->dwRedirectingIDOffset, pInfo->dwRedirectingIDSize);
		CString strRedirectingName = StructString(pInfo, pInfo->dwRedirectingIDNameOffset, pInfo->dwRedirectingIDNameSize);
		if (!strRedirecting.IsEmpty() || !strRedirectingName.IsEmpty())
		{
			strLine.Format(L"    RedirectingID: %s %s", strRedirecting.GetString(), strRedirectingName.GetString());
			lines.push_back(strLine);
		}
		CString strRedirection = StructString(pInfo, pInfo->dwRedirectionIDOffset, pInfo->dwRedirectionIDSize);
		CString strRedirectionName = StructString(pInfo, pInfo->dwRedirectionIDNameOffset, pInfo->dwRedirectionIDNameSize);
		if (!strRedirection.IsEmpty() || !strRedirectionName.IsEmpty())
		{
			strLine.Format(L"    RedirectionID: %s %s", strRedirection.GetString(), strRedirectionName.GetString());
			lines.push_back(strLine);
		}
		if (pInfo->dwCallID != 0)
		{
			strLine.Format(L"    Host CallID: %08lX", pInfo->dwCallID);
			lines.push_back(strLine);
		}
		if (pInfo->dwRelatedCallID != 0)
		{
			strLine.Format(L"    Related Host CallID: %08lX", pInfo->dwRelatedCallID);
			lines.push_back(strLine);
		}
		if (pInfo->dwAddressID != 0)
		{
			strLine.Format(L"    AddressID: %08lX", pInfo->dwAddressID);
			lines.push_back(strLine);
		}
		if (pInfo->dwAppSpecific != 0)
		{
			strLine.Format(L"    Appspecific: %08lX", pInfo->dwAppSpecific);
			lines.push_back(strLine);
		}
		AppendCallData(pInfo, lines);
		AppendDevSpecific(pInfo, lines);
	}

	// Hex dump of LINECALLINFO call data. Offset is dwCallDataOffset.
	// Capped so one message cannot fill the window.
	void AppendCallData(const LINECALLINFO* pInfo, std::vector<CString>& lines)
	{
		if (pInfo->dwCallDataSize == 0 || pInfo->dwCallDataOffset == 0)
			return;
		if (pInfo->dwCallDataOffset > pInfo->dwUsedSize || pInfo->dwCallDataSize > pInfo->dwUsedSize - pInfo->dwCallDataOffset)
			return;

		const BYTE* pData = ((const BYTE*)pInfo) + pInfo->dwCallDataOffset;
		DWORD dwCount = pInfo->dwCallDataSize;
		bool bTruncated = false;
		if (dwCount > 64)
		{
			dwCount = 64;
			bTruncated = true;
		}
		CString strHex;
		for (DWORD i = 0; i < dwCount; ++i)
			strHex.AppendFormat(L" %02X", pData[i]);
		if (bTruncated)
			strHex += L" ...";
		CString strLine;
		strLine.Format(L"    CallData:%s", strHex.GetString());
		lines.push_back(strLine);
	}

	// CallLinkageID and other ECSTA devspecific elements, same fields tapicaps prints.
	void AppendDevSpecific(const LINECALLINFO* pInfo, std::vector<CString>& lines)
	{
		if (pInfo->dwDevSpecificOffset == 0 || pInfo->dwDevSpecificSize < sizeof(ECSTADEVSPECIFICELEMENTLISTV2))
			return;
		if (pInfo->dwDevSpecificOffset > pInfo->dwUsedSize || pInfo->dwDevSpecificSize > pInfo->dwUsedSize - pInfo->dwDevSpecificOffset)
			return;

		const ECSTADEVSPECIFICELEMENTLISTV2* pList = (const ECSTADEVSPECIFICELEMENTLISTV2*)(((const BYTE*)pInfo) + pInfo->dwDevSpecificOffset);
		if (pList->dwMagicValue != ECSTADEVSPECIFICELEMENTLISTV2_MAGICVALUE || pList->dwVersion != ECSTADEVSPECIFICELEMENTLISTV2_VERSION)
			return;
		if (pList->dwNumElements == 0 || pList->dwNumElements > 32)
			return;

		DWORD dwListBytes = sizeof(ECSTADEVSPECIFICELEMENTLISTV2) + (pList->dwNumElements - 1) * sizeof(ECSTADEVSPECIFICELEMENT);
		if (dwListBytes > pInfo->dwDevSpecificSize)
			return;

		for (DWORD i = 0; i < pList->dwNumElements; ++i)
		{
			const ECSTADEVSPECIFICELEMENT& element = pList->elements[i];
			if (element.dwSize == 0 || element.dwOffset > pInfo->dwDevSpecificSize || element.dwSize > pInfo->dwDevSpecificSize - element.dwOffset)
				continue;
			DWORD dwOffset = pInfo->dwDevSpecificOffset + element.dwOffset;
			CString strText = StructString(pInfo, dwOffset, element.dwSize);
			CString strLine;
			if (element.dwElementID == ECSTADEVSPECIFICELEMENT_CALLLINKAGEID)
				strLine.Format(L"    CallLinkageID: %s", strText.GetString());
			else
				strLine.Format(L"    DevSpecificElement 0x%08x: %s", element.dwElementID, strText.GetString());
			lines.push_back(strLine);
		}
	}

	// lineGetAddressStatus, growing the buffer the way tapicaps does.
	bool ReadAddressStatus(HLINE hLine, DWORD dwAddressID, std::vector<BYTE>& buffer)
	{
		DWORD dwSize = sizeof(LINEADDRESSSTATUS) + 2048;
		for (int iTry = 0; iTry < 5; ++iTry)
		{
			buffer.assign(dwSize, 0);
			LPLINEADDRESSSTATUS pStatus = (LPLINEADDRESSSTATUS)buffer.data();
			pStatus->dwTotalSize = dwSize;
			CTapiEventLogScope scope(L"lineGetAddressStatus");
			LONG lResult = lineGetAddressStatus(hLine, dwAddressID, pStatus);
			if (lResult != 0)
				return false;
			if (pStatus->dwNeededSize <= dwSize)
				return true;
			dwSize = pStatus->dwNeededSize;
		}
		return false;
	}

	CString AddressFeatureNames(DWORD dwFeatures)
	{
		CString strNames;
		if (dwFeatures & LINEADDRFEATURE_FORWARD)
			strNames += L"LINEADDRFEATURE_FORWARD ";
		if (dwFeatures & LINEADDRFEATURE_MAKECALL)
			strNames += L"LINEADDRFEATURE_MAKECALL ";
		if (dwFeatures & LINEADDRFEATURE_PICKUP)
			strNames += L"LINEADDRFEATURE_PICKUP ";
		if (dwFeatures & LINEADDRFEATURE_SETUPCONF)
			strNames += L"LINEADDRFEATURE_SETUPCONF ";
		if (dwFeatures & LINEADDRFEATURE_UNPARK)
			strNames += L"LINEADDRFEATURE_UNPARK ";
		if (strNames.IsEmpty())
			strNames.Format(L"%08lX", dwFeatures);
		return strNames;
	}

	CString ForwardModeNames(DWORD dwMode)
	{
		CString strNames;
		if (dwMode & LINEFORWARDMODE_UNCOND)
			strNames += L"LINEFORWARDMODE_UNCOND ";
		if (dwMode & LINEFORWARDMODE_UNCONDEXTERNAL)
			strNames += L"LINEFORWARDMODE_UNCONDEXTERNAL ";
		if (dwMode & LINEFORWARDMODE_UNCONDINTERNAL)
			strNames += L"LINEFORWARDMODE_UNCONDINTERNAL ";
		if (dwMode & LINEFORWARDMODE_BUSY)
			strNames += L"LINEFORWARDMODE_BUSY ";
		if (dwMode & LINEFORWARDMODE_BUSYEXTERNAL)
			strNames += L"LINEFORWARDMODE_BUSYEXTERNAL ";
		if (dwMode & LINEFORWARDMODE_BUSYINTERNAL)
			strNames += L"LINEFORWARDMODE_BUSYINTERNAL ";
		if (dwMode & LINEFORWARDMODE_NOANSW)
			strNames += L"LINEFORWARDMODE_NOANSW ";
		if (dwMode & LINEFORWARDMODE_NOANSWEXTERNAL)
			strNames += L"LINEFORWARDMODE_NOANSWEXTERNAL ";
		if (dwMode & LINEFORWARDMODE_NOANSWINTERNAL)
			strNames += L"LINEFORWARDMODE_NOANSWINTERNAL ";
		if (strNames.IsEmpty())
			strNames.Format(L"0x%08lX", dwMode);
		return strNames;
	}

	// Child lines tapicaps hangs under a LINE_ADDRESSSTATE item.
	// Called without g_lock. Does not change the call.
	void AppendAddressDetails(HLINE hLine, DWORD dwAddressID, std::vector<CString>& lines)
	{
		std::vector<BYTE> statusBytes;
		if (!ReadAddressStatus(hLine, dwAddressID, statusBytes))
			return;

		LPLINEADDRESSSTATUS pStatus = (LPLINEADDRESSSTATUS)statusBytes.data();
		CString strLine;
		strLine.Format(L"    AddressID: %lu", dwAddressID);
		lines.push_back(strLine);
		strLine.Format(L"    NumInUse: %lu", pStatus->dwNumInUse);
		lines.push_back(strLine);
		strLine.Format(L"    NumActiveCalls: %lu", pStatus->dwNumActiveCalls);
		lines.push_back(strLine);
		strLine.Format(L"    NumOnHoldCalls: %lu", pStatus->dwNumOnHoldCalls);
		lines.push_back(strLine);
		strLine.Format(L"    NumOnHoldPendCalls: %lu", pStatus->dwNumOnHoldPendCalls);
		lines.push_back(strLine);
		strLine.Format(L"    AddressFeatures: %s", AddressFeatureNames(pStatus->dwAddressFeatures).GetString());
		lines.push_back(strLine);
		strLine.Format(L"    NumRingsNoAnswer: %lu", pStatus->dwNumRingsNoAnswer);
		lines.push_back(strLine);
		strLine.Format(L"    ForwardNumEntries: %lu", pStatus->dwForwardNumEntries);
		lines.push_back(strLine);

		if (pStatus->dwForwardNumEntries == 0 || pStatus->dwForwardOffset == 0)
			return;
		if (pStatus->dwForwardNumEntries > 16)
			return;
		DWORD dwBytes = pStatus->dwForwardNumEntries * sizeof(LINEFORWARD);
		if (pStatus->dwForwardOffset > pStatus->dwUsedSize || dwBytes > pStatus->dwUsedSize - pStatus->dwForwardOffset)
			return;

		for (DWORD i = 0; i < pStatus->dwForwardNumEntries; ++i)
		{
			const LINEFORWARD* pForward = (const LINEFORWARD*)(((const BYTE*)pStatus) + pStatus->dwForwardOffset + i * sizeof(LINEFORWARD));
			strLine.Format(L"    ForwardEntry: %lu", i);
			lines.push_back(strLine);
			strLine.Format(L"        ForwardMode: %s", ForwardModeNames(pForward->dwForwardMode).GetString());
			lines.push_back(strLine);
			CString strDest = StructString(pStatus, pForward->dwDestAddressOffset, pForward->dwDestAddressSize);
			if (strDest.IsEmpty())
				strDest = L"Do Not disturb";
			strLine.Format(L"        ForwardTo: %s", strDest.GetString());
			lines.push_back(strLine);
		}
	}
}

void TapiEventLogEnter(const wchar_t* szOperation)
{
	CSingleLock lock(&g_lock, TRUE);
	DWORD dwThread = GetCurrentThreadId();
	// The event thread is the one that blocks in lineGetMessage. Remember it
	// here so the UI poll is not written into the log.
	if (szOperation != NULL && wcscmp(szOperation, L"lineGetMessage") == 0)
		g_eventThreadId = dwThread;

	Operation operation;
	operation.threadId = dwThread;
	operation.tick = GetTickCount();
	operation.name = szOperation;
	operation.hasResult = false;
	operation.result = 0;
	g_operations.push_back(operation);

	if (dwThread == g_eventThreadId)
	{
		CString strLine;
		strLine.Format(L"%s -> %s", TimeStamp().GetString(), operation.name.GetString());
		PushLine(strLine);
	}
}

// Attaches the lineGetMessage result to the open wait on this thread.
// Leave prints message, timeout, or the raw code. A missing call leaves
// the wait line without that word.
void TapiEventLogSetWaitResult(LONG lResult)
{
	CSingleLock lock(&g_lock, TRUE);
	DWORD dwThread = GetCurrentThreadId();
	for (size_t i = g_operations.size(); i > 0; --i)
	{
		if (g_operations[i - 1].threadId != dwThread)
			continue;
		if (g_operations[i - 1].name != L"lineGetMessage")
			continue;
		g_operations[i - 1].hasResult = true;
		g_operations[i - 1].result = lResult;
		return;
	}
}

void TapiEventLogLeave()
{
	CSingleLock lock(&g_lock, TRUE);
	DWORD dwThread = GetCurrentThreadId();
	for (size_t i = g_operations.size(); i > 0; --i)
	{
		if (g_operations[i - 1].threadId == dwThread)
		{
			Operation operation = g_operations[i - 1];
			g_operations.erase(g_operations.begin() + (i - 1));
			if (dwThread == g_eventThreadId)
			{
				DWORD dwElapsed = GetTickCount() - operation.tick;
				CString strHow;
				if (operation.name == L"lineGetMessage" && operation.hasResult)
				{
					if (operation.result == 0)
						strHow = L" message";
					else if (operation.result == LINEERR_OPERATIONFAILED)
						strHow = L" timeout";
					else
						strHow.Format(L" 0x%08lX", (unsigned long)operation.result);
				}
				CString strLine;
				strLine.Format(L"%s <- %s%s %lu ms", TimeStamp().GetString(), operation.name.GetString(), strHow.GetString(), dwElapsed);
				PushLine(strLine);
			}
			return;
		}
	}
}

// Stores one message returned by lineGetMessage. The number is how many
// messages this process has taken from TAPI. Call and address messages then
// get the same child fields tapicaps reads. The header is stored before those
// reads, and the lock is not held across them, so a stuck lineGet* still
// leaves the numbered line and shows up on the status line. Does not
// deallocate calls on IDLE.
void TapiEventLogAddLineMessage(const LINEMESSAGE* pMessage)
{
	if (pMessage == NULL)
		return;

	{
		CSingleLock lock(&g_lock, TRUE);
		++g_processed;
		CString strHeader;
		strHeader.Format(L"%06lu %s", g_processed, FormatMessage(pMessage).GetString());
		PushLine(strHeader);
	}

	std::vector<CString> details;
	if (pMessage->dwMessageID == LINE_CALLSTATE || pMessage->dwMessageID == LINE_CALLINFO)
	{
		if (pMessage->hDevice != 0)
			AppendCallDetails((HCALL)pMessage->hDevice, details);
	}
	else if (pMessage->dwMessageID == LINE_ADDRESSSTATE)
	{
		if (pMessage->hDevice != 0)
			AppendAddressDetails((HLINE)pMessage->hDevice, (DWORD)pMessage->dwParam1, details);
	}

	if (details.empty())
		return;

	CSingleLock lock(&g_lock, TRUE);
	for (size_t i = 0; i < details.size(); ++i)
		PushLine(details[i]);
}

void TapiEventLogTakeLines(CString& strNewLines, CString& strStatus)
{
	CSingleLock lock(&g_lock, TRUE);
	strNewLines.Empty();
	while (g_shown < g_lines.size())
	{
		strNewLines += g_lines[g_shown];
		strNewLines += L"\r\n";
		++g_shown;
	}
	strStatus = CurrentStatus();
}

// Drops every stored line. The processed-message counter is left as it is,
// so the next message keeps the next number. Safe while the event thread
// is still appending; the log window clears its edit after this returns.
void TapiEventLogClear()
{
	CSingleLock lock(&g_lock, TRUE);
	g_lines.clear();
	g_shown = 0;
}

CTapiEventLogScope::CTapiEventLogScope(const wchar_t* szOperation)
{
	TapiEventLogEnter(szOperation);
}

CTapiEventLogScope::~CTapiEventLogScope()
{
	TapiEventLogLeave();
}
