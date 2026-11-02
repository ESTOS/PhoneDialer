#pragma once

// Records LINE_ messages and which TAPI call the current thread is inside.
// The phone log window reads this from another thread, so a blocked
// lineGet* on the UI thread still shows up in the status line.
// Callers must include stdafx.h before this header.

void TapiEventLogEnter(const wchar_t* szOperation);
void TapiEventLogLeave();

// Result of the lineGetMessage the event thread is inside. Leave writes
// "message", "timeout", or the error code from this value.
void TapiEventLogSetWaitResult(LONG lResult);
void TapiEventLogAddLineMessage(const LINEMESSAGE* pMessage);

// Appends lines not yet returned and the current "inside ..." status.
// strNewLines is empty when nothing arrived since the last take.
void TapiEventLogTakeLines(CString& strNewLines, CString& strStatus);

// Drops every stored line. The processed-message counter is left as it is.
void TapiEventLogClear();

// Enters on construction and leaves on every exit path, including return.
class CTapiEventLogScope
{
public:
	explicit CTapiEventLogScope(const wchar_t* szOperation);
	~CTapiEventLogScope();
};
