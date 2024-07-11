#ifndef _ECSTAEXTGLOBAL_H
#define _ECSTAEXTGLOBAL_H

// DevCaps die von anderen Applikationen verwenden werden (CCG ProCall etc. pp) in eigenen Header im global ausgelagert damit diese überall gleich sind
#include "ecstalinedevcaps.h"

//Extension Interface definitions for 
//ESTOS ECSTA150 TSP for Hicom 150 E Office

//UUIDgen generated ExtensionID
#define ECSTA150_EXTID0 0x1787f890
#define ECSTA150_EXTID1 0x2bea11d4
#define ECSTA150_EXTID2 0x9ee500e0
#define ECSTA150_EXTID3 0x292539c9

//ExtID 3 für die Control Line
#define ECSTA150_EXTID3CONTROL 	0x292539c8
#define ECSTA150_EXTID3PARK 	0x292539c7
#define ECSTA150_EXTID3QUE 		0x292539c6

//Extension Version 1.0
#define ECSTA150_EXTVERSION 0x00010000

/********************************************************************/
//lineDevSpecificFeature definitions
/********************************************************************/

/********************************************************************/
//Feature Agentstate:
//This Feature allows the ACD Agent State to be controlled.

//Returns: LINEERR_OPERATIONFAILED
//or	   NO_ERROR
/********************************************************************/

#define ECSTA150_FEATURE_AGENTSTATEREQUEST			0x00000001
typedef struct _ECSTA150AGENTSTATEREQUEST
{
	DWORD dwAgentStateRequest;
	char szAgentID[32];
} ECSTA150AGENTSTATEREQUEST;

typedef struct _ECSTA150AGENTSTATEREQUEST20
{
	DWORD dwAgentStateRequest;
	char szAgentID[32];
	char szAgentGroupID[32];
	char szAgentPassword[32];
} ECSTA150AGENTSTATEREQUEST20;

typedef struct _ECSTA150AGENTSTATEREQUEST21
{
	DWORD dwAgentStateRequest;
	char szAgentID[32];
	char szAgentGroupID[32];
	char szAgentPassword[32];
	char szDevice[32];		  //default empty
	DWORD dwAgentStateDetail; //default unused (0xffffffff)
} ECSTA150AGENTSTATEREQUEST21;

#define ECSTA150_agentRequestLogIn				0x00000000
//dwAgentStateDetail = 1: enable Agent Headset mode
#define ECSTA150_agentRequestLogOut				0x00000001
#define ECSTA150_agentRequestNotReady			0x00000002
//dwAgentStateDetail = 0: request Pause
//dwAgentStateDetail = 1 - 9: Set Not Ready Reason (coffee break...)
#define ECSTA150_agentRequestReady				0x00000003
#define ECSTA150_agentRequestBusy				0x00000004
#define ECSTA150_agentRequestWorkingAfterCall	0x00000005

#define ECSTA150_SUPPORTEDAGENTREQUESTS_LOGIN				0x000000001
#define ECSTA150_SUPPORTEDAGENTREQUESTS_LOGOUT				0x000000002
#define ECSTA150_SUPPORTEDAGENTREQUESTS_NOTREADY			0x000000004
#define ECSTA150_SUPPORTEDAGENTREQUESTS_READY				0x000000008
#define ECSTA150_SUPPORTEDAGENTREQUESTS_BUSY				0x000000010
#define ECSTA150_SUPPORTEDAGENTREQUESTS_WORKINGAFTERCALL	0x000000020
#define ECSTA150_SUPPORTEDAGENTREQUESTS_ALL					(ECSTA150_SUPPORTEDAGENTREQUESTS_LOGIN | ECSTA150_SUPPORTEDAGENTREQUESTS_LOGOUT | ECSTA150_SUPPORTEDAGENTREQUESTS_NOTREADY | ECSTA150_SUPPORTEDAGENTREQUESTS_READY | ECSTA150_SUPPORTEDAGENTREQUESTS_BUSY | ECSTA150_SUPPORTEDAGENTREQUESTS_WORKINGAFTERCALL)

// Flag which announces that currently no request is possible, definitiv flag to be able to differ from 0x00 which also could announce no option but could also point out that the driver does not deliver this information.
#define ECSTA150_SUPPORTEDAGENTREQUESTS_NONE				0x000000080

#define ECSTA150_SUPPORTEDAGENTREQUESTS_MASK				0x0000000ff

//Agent State Sample:
/*
ECSTA150AGENTSTATEREQUEST AgentState;
AgentState.dwAgentStateRequest = ECSTA150_agentRequestLogIn;
strcpy(AgentState.szAgentID, "100");

lineDevSpecificFeature(hLine, ECSTA150_FEATURE_AGENTSTATEREQUEST, &AgentState, sizeof(ECSTA150AGENTSTATEREQUEST));
*/


/********************************************************************/
//Feature Phone Display
//This Feature allows to write Text in the phone Display
//The Parameter szDevice allows to write in any phone display (not only in the display assoziated with this line)
//If no device is specified, the Text is written in the device assoziated with the given line.
//ATTENTION: do not forget to delete the text in the phone display.
//The text will stay in the phone display forever.
//In a maximum of 50 devices text may be displayed using this function
//A normal Optiset E advance may display up to 23 characters
//An empty String in szDisplayText deletes the Text from the display

//Returns: LINEERR_OPERATIONFAILED
//or	   NO_ERROR
/********************************************************************/
#define ECSTA150_FEATURE_DISPLAY		0x00000002
typedef struct _ECSTA150DISPLAY
{
	char szDevice[20];
	char szDisplayText[255];
} ECSTA150DISPLAY;

/*
ECSTA150DISPLAY Display;
strcpy(Display.szDevice, ""); //Write to our own device
strcpy(Display.szDisplayText, "Hi how are you?");
lineDevSpecificFeature(hLine, ECSTA150_FEATURE_DISPLAY, &Display, sizeof(ECSTA150DISPLAY));
*/

/********************************************************************/
//Feature Remote Office
//This Feature allows to enable or disable Remote Office Feature (for Broadsoft)
//Returns: LINEERR_OPERATIONFAILED
//or	   NO_ERROR
/********************************************************************/
#define ECSTA150_FEATURE_REMOTEOFFICE		0x00000003
typedef struct _ECSTA150REMOTEOFFICE
{
	DWORD   dwEnabled;				//Enable Remote Office (1) or disable (0)
	wchar_t szDestination[100];		//Destination Phone Number
} ECSTA150REMOTEOFFICE;

/*
ECSTA150DISPLAY Display;
strcpy(Display.szDevice, ""); //Write to our own device
strcpy(Display.szDisplayText, "Hi how are you?");
lineDevSpecificFeature(hLine, ECSTA150_FEATURE_DISPLAY, &Display, sizeof(ECSTA150DISPLAY));
*/

/********************************************************************/
//Feature Set Absent Message und Presence No
//This Feature allows to set the PresenceNo and AbsentMessage (for Panasonic)
//Returns: LINEERR_OPERATIONFAILED
//or	   NO_ERROR
/********************************************************************/
#define ECSTA150_FEATURE_ABSENTMESSAGE		0x00000004
//the struct used for this Feature is ECSTADEVSPECIFICELEMENT_ABSENTMESSAGEPARAMS

/********************************************************************/
//Feature Start Busy Lamp Field (Panasonic)
//This Feature allows to enable the BusyLamp Field Feature
//Returns: LINEERR_OPERATIONFAILED / LINEERR_OPERATIONUNAVAIL
//or	   NO_ERROR
/********************************************************************/
#define ECSTA150_FEATURE_BUSYLAMPFIELD		0x00000005
//there is no struct used 


/********************************************************************/
//lineDevSpecific definitions
/********************************************************************/
typedef struct _ECSTA150DEVSPECIFIC
{
	DWORD dwAgentState;
	char szAgentID[32];
} ECSTA150DEVSPECIFIC;

typedef struct _ECSTA150DEVSPECIFIC20 : public _ECSTA150DEVSPECIFIC
{
	char szAgentGroupID[32];
	char szAgentPassword[32];
} ECSTA150DEVSPECIFIC20;

typedef struct _ECSTA150DEVSPECIFIC20B : public _ECSTA150DEVSPECIFIC20
{
	char szLastCallCode[32];
} ECSTA150DEVSPECIFIC20B;

typedef struct _ECSTA150DEVSPECIFIC20C : public _ECSTA150DEVSPECIFIC20B
{
	/*! Details about the AgentState.
	At the Moment there is only one:
	ECSTAAGENTFLAG_CHANGEDFROMAPP indicates, that the
	Agent State was set by a pc application and not initiated at the phone. */
	DWORD dwFlags;
} ECSTA150DEVSPECIFIC20C;

typedef struct _ECSTA150DEVSPECIFIC20D : public _ECSTA150DEVSPECIFIC20C
{
	/* Details about the Agent State.
	value: 0xffffffff unused
	For Alcatel OXE: 0 Pause, 1 - 9 NotReady Details */
	DWORD dwAgentStateDetails;

	/* Device Phone number, where the agent logged in (Alcatel OXE only) */
	char szLogonDevice[32];
	/* Name of the Agent (Alcatel OXE only) */
	char szLogonName[32];

} ECSTA150DEVSPECIFIC20D;

typedef struct _ECSTA150DEVSPECIFIC20E : public _ECSTA150DEVSPECIFIC20D
{
	/* Name of the Agent Group (Panasonic) */
	wchar_t szAgentGroupName[32];
} ECSTA150DEVSPECIFIC20E;

typedef struct _ECSTA150DEVSPECIFIC20F : public _ECSTA150DEVSPECIFIC20E
{
	/* Possible agent requests the agent can call from the current state */
	/* ECSTA150_SUPPORTEDAGENTREQUESTS_xxx Bit Mask */
	DWORD dwSupportedAgentRequests;
} ECSTA150DEVSPECIFIC20F;

//ECSTA150DEVSPECIFIC Params;
//
//lineDevSpecific(hLine, 0, NULL, &Params, sizeof(ECSTA150DEVSPECIFIC));

/*!Flags for ECSTA150DEVSPECIFIC20B.
Make sure you test the flags bit wise and not check equality. There may be more than one inside.
*/
#define ECSTAAGENTFLAG_CHANGEDFROMAPP			0x00000001

/*! This bit can only be present during
dwAgentState is ECSTA150_agentStateReady
This Flag tells, that the Ready State is entered because the logon key was pressed.
If the Ready State is entered because the pause taste was pressed, this flag is not set.
*/
#define ECSTAAGENTFLAG_READYFROMLOGON			0x00000002

/*! This bit can only be present during
dwAgentState is ECSTA150_agentStateReady
This Flag tells, that the Ready State is entered because the working after call key was pressed.
If the Ready State is entered because the pause taste or the logon key was pressed, this flag is not set.
*/
#define ECSTAAGENTFLAG_READYFROMWORKINGAFTERCALL 0x00000004

/*! Set if Event.eventSpecificInfo.u.agentStateEvent.u.notReadyEvent.cause == forcedPause
*/
#define ECSTAAGENTFLAG_FORCEDPAUSE				0x00000008

/*! Set if AlcatelQueryAgentInformation.agentPreAssigned = TRUE
*/
#define ECSTAAGENTFLAG_AGENTPREASSIGNED			0x00000010

/*! This Flag is used only in lineGetDevspecific and tells to get the current AgentState (ignore the m_AgentStateEventList history)
*/
#define ECSTAAGENTFLAG_GETCURRENT				0x00000020

/*! This Flag is used to tell the driver that the application is using the index mechanism to fetch agent information)
*/
#define ECSTAAGENTFLAG_USEINDEX					0x00000040

/*! Flags ECSTAAGENTFLAG_ALWAYSGETCURRENT and ECSTAAGENTFLAG_USEINDEX are XOR. In case both specified driver will return LINEERR_INVALADDRESSMODE
*/

/*! Escape Sequences for the values */
#define ECSTAAGENTFLAG_ESCAPEFLAGS				0x000000ff

/*! Within this range is the index for the agent state. Used in events to send the index to the application, used in fetching the agent state data after an event */
/*! be sure to set ECSTAAGENTFLAG_USEDINDEX when fetching the data from the driver */
/*! in case the application is querying for an entry which is not existing in the driver the driver will return LINEERR_INVALADDRESSID */
#define ECSTAAGENTFLAG_ESCAPEINDEX				0xff000000

/********************************************************************/
//LINE_DEVSPECIFIC: dwParam1, dwParam2, dwParam3
//Event Report
/********************************************************************/
//Notifies that an agent state has changed
//Also available as ecsta/AgentState via GetDevConfig
#define ECSTA150_AGENTSTATE						0x00000001
//dwParam1: ECSTA150_AGENTSTATE
//dwParam2: one of the  ECSTA150_agentState constants
//dwParam3: not used
#define ECSTA150_agentStateNoAgent				0x00000000
#define ECSTA150_agentStateNotReady				0x00000001
#define ECSTA150_agentStateReady				0x00000002
#define ECSTA150_agentStateBusy 				0x00000003
#define ECSTA150_agentStateWorkingAfterCall		0x00000004

/********************************************************************/
//LINE_DEVSPECIFIC: dwParam1, dwParam2, dwParam3
//Event Report
/********************************************************************/
//Notifies that a callcode was received the callcode can be received with 
#define ECSTA150_CALLCODE		0x00000002
//dwParam1: ECSTA150_CALLCODE
//dwParam2: not used
//dwParam3: not used

//Sent on the Control Line as Devspecific Message
#define ECSTADEVSPEC_LINENAMECHANGED 0x00000003
//dwParam1: ECSTADEVSPEC_LINENAMECHANGED
//dwParam2: Tapi Device ID
//dwParam3: not used

//Sent on the Line as Devspecific Message
#define ECSTADEVSPEC_REMOTEOFFICECHANGED 0x00000004
//dwParam1: ECSTADEVSPEC_REMOTEOFFICECHANGED
//dwParam2: Tapi Device ID
//dwParam3: not used

//Sent on the Line as Devspecific Message
#define ECSTADEVSPEC_INVALIDPASSWORDCHANGED 0x00000008
//dwParam1: ECSTADEVSPEC_INVALIDPASSWORDCHANGED
//dwParam2: Invalid Password State ( 0 / 1 )
//dwParam3: not used

//Sent on the Line as Devspecific Message
#define ECSTADEVSPEC_ACTIVEADDRESSCHANGED 0x00000009
//dwParam1: ECSTADEVSPEC_ACTIVEADDRESSCHANGED
//dwParam2: AddressID (0 - 2) of Panasonic User Container Device that is now marked active
//dwParam3: not used

//Sent on the Line as Devspecific Message
#define ECSTADEVSPEC_MESSAGEWAITINGCOUNTER	0x0000000a
//dwParam1: ECSTADEVSPEC_MESSAGEWAITINGCOUNTER
//dwParam2: Counter (0 - n) Panasonic Message Waiting Counter
//dwParam3: not used

//Sent on the Line as Devspecific Message
#define ECSTADEVSPEC_ABSENTMESSAGE			0x0000000b
//dwParam1: ECSTADEVSPEC_ABSENTMESSAGE
//dwParam2: umPresenceNo (0 - n) Panasonic Presence No
//dwParam3: not used

//Sent on the Line as Devspecific Message
#define ECSTADEVSPEC_AGENTGROUPSCHANGED		0x0000000c
//dwParam1: ECSTADEVSPEC_AGENTGROUPSCHANGED
//dwParam2: dwAgentState (total)
//dwParam3: not used

//ECSTA Advanced Control Line Devspecific
typedef struct _ECSTACONTROLDEVSPECIFIC
{
	DWORD       dwTotalSize;			//in
	DWORD       dwNeededSize;			//in/out
	DWORD       dwUsedSize;				//in/out
	DWORD       dwControlCommand;		//in
	DWORD       dwControlResult;		//out
	DWORD       dwControlResultDetail;	//out
	DWORD       dwDataOffSet;			//in/out
	DWORD       dwDataSize;				//in/out
} ECSTACONTROLDEVSPECIFIC;

/* Get List of Line Configurations
ECSTACONTROLDEVSPECIFIC contains a ECSTACONTROLDATA_GETLINECONFIGLIST
The Offset is always related to the beginning of the ECSTACONTROLDEVSPECIFIC struct*/
#define ECSTACONTROL_GETLINECONFIGLIST	0x80000009
/* Set a new List of Line Configurations
ECSTACONTROLDEVSPECIFIC contains a ECSTACONTROLDATA_GETLINECONFIGLIST
The Offset is always related to the beginning of the ECSTACONTROLDEVSPECIFIC struct*/
#define ECSTACONTROL_SETLINECONFIGLIST	0x8000000a
/* Add additional Lines
ECSTACONTROLDEVSPECIFIC contains a ECSTACONTROLDATA_GETLINECONFIGLIST
The Offset is always related to the beginning of the ECSTACONTROLDEVSPECIFIC struct*/
#define ECSTACONTROL_ADDLINECONFIGLIST	0x8000000b
/* Remove some Lines
ECSTACONTROLDEVSPECIFIC contains a ECSTACONTROLDATA_GETLINECONFIGLIST
The Offset is always related to the beginning of the ECSTACONTROLDEVSPECIFIC struct*/
#define ECSTACONTROL_REMOVELINECONFIGLIST	0x8000000c
/* Query Line exists
ECSTACONTROLDEVSPECIFIC contains a ECSTACONTROLDATA_GETLINECONFIG
The Offset is always related to the beginning of the ECSTACONTROLDEVSPECIFIC struct*/
#define ECSTACONTROL_QUERYLINEEXISTS		0x8000000d
/* Set License Mode
ECSTACONTROLDEVSPECIFIC contains nothing
dwControlResultDetail is set to 0 (line licensing) or 1 (line open licensing) */
#define ECSTACONTROL_SETLICENSEMODE			0x8000000e
/* Set Async LineOpen Mode
ECSTACONTROLDEVSPECIFIC contains nothing
dwControlResultDetail is set to 0 (no async lineopen) or 1 (use async lineopen) */
#define ECSTACONTROL_SETASYNCLINEOPEN		0x8000000f
/* Query Supercanonic Dialing Required
ECSTACONTROLDEVSPECIFIC contains nothing
dwControlResult is set to 0 (no Supercanonic Dialing) or 1 (use Supercanonic Dialing)
Used with Hipath4000 non unique numbering plan */
#define ECSTACONTROL_GETSUPERCANONICDIALING	0x80000010

//****** ECSTACAPSEXTVERSION3 commands ***************/
/* ECSTACONTROLDEVSPECIFIC contains a ECSTACONTROLDATA_GETLINECONFIGLISTV3
The request may only contain one phone number (one ECSTACONTROLDATA_GETLINECONFIGV3)
The result may contain more tham one ECSTACONTROLDATA_GETLINECONFIGV3.
The Offset is always related to the beginning of the ECSTACONTROLDEVSPECIFIC struct*/
#define ECSTACONTROL_QUERYLINEEXISTSV3		0x80000011

//****** ECSTACAPSEXTVERSION3 commands ***************/
/* ECSTACONTROLDEVSPECIFIC contains a ECSTA150REMOTEOFFICE
The Offset is always related to the beginning of the ECSTACONTROLDEVSPECIFIC struct*/
#define ECSTA_GETREMOTEOFFICE				0x80000012

/* Set a new 1st Party Line Configuration (for 1st Party incl IP and Passwd) (only one element allowed)
ECSTACONTROLDEVSPECIFIC contains a ECSTACONTROLDATA_GETLINECONFIGLIST
The Offset is always related to the beginning of the ECSTACONTROLDEVSPECIFIC struct*/
#define ECSTACONTROL_SETFIRSTPARTYLINECONFIGLIST 0x80000013

/* 1st Party Mode - Change Password in PBX
ECSTACONTROLDEVSPECIFIC contains a ECSTACONTROLDATA_CHANGEPASSWD
The Offset is always related to the beginning of the ECSTACONTROLDEVSPECIFIC struct*/
#define ECSTACONTROL_CHANGEPBXPASSWORD			0x80000014

/* Validate PBX Connection credentials using a seperate Connection. Returns synchonous
ECSTACONTROLDEVSPECIFIC contains a ECSTACONTROLDATA_GETLINECONFIGLIST
The Offset is always related to the beginning of the ECSTACONTROLDEVSPECIFIC struct*/
#define ECSTACONTROL_VALIDATESERVERCREDENTIALS		0x80000015

/* Query the device agent capability EAGENTCAPABILITY enum
ECSTACONTROLDEVSPECIFIC contains a ECSTACONTROLDATA_GETLINECONFIG
The Offset is always related to the beginning of the ECSTACONTROLDEVSPECIFIC struct*/
#define ECSTACONTROL_QUERYDEVICEAGENTCAPABILITY		0x80000016

enum class EAGENTCAPABILITY
{
	// Gerät gibt es nicht
	DEVICE_DOES_NOT_EXIST = -1,
	// ACD nicht möglich
	DEVICE_IS_NOT_ACD_CAPABLE = 0,
	// ACD möglich
	DEVICE_IS_ACD_CAPABLE = 1,
	// Eine Agenten Leitung
	DEVICE_IS_AN_AGENT = 2,
	// Eine Supervisor Leitung
	DEVICE_IS_A_SUPERVISOR = 3,
	// Anderer Wert den die PBX liefert der nicht weiter aufgeschlüsselt werden konnte
	OTHER = 0xff
};

//Error Codes for dwControlResult und für Devspecific return error codes (z.B. lineopen)
//not enough licenses available
#define ECSTAERR_OUTOFLICENSES		0xC0000001
#define ECSTAERR_CONNECTFAILED		0xC0000002
#define ECSTAERR_INVALIDPARAM		0xC0000003

typedef struct _ECSTACONTROLDATA_CHANGEPASSWD
{
	wchar_t szOldPasswd[20];
	wchar_t szNewPasswd[20];
} ECSTACONTROLDATA_CHANGEPASSWD;

typedef struct _ECSTACONTROLDATA_GETLINECONFIG
{
	DWORD       dwLineNumberOffset;
	DWORD       dwLineNumberSize;
	DWORD       dwLineNameOffset;
	DWORD       dwLineNameSize;
	DWORD       dwCookie;			//In addLine the cookie will be transferred to the created line
} ECSTACONTROLDATA_GETLINECONFIG;

typedef struct _ECSTACONTROLDATA_GETLINECONFIGLIST
{
	DWORD       dwCount;
	ECSTACONTROLDATA_GETLINECONFIG lineConfig[1];
} ECSTACONTROLDATA_GETLINECONFIGLIST;

typedef struct _ECSTACONTROLDATA_GETLINECONFIGV3
{
	DWORD       dwLineConfigNumberOffset;		//Rufnummer, as in Configuration (123;1;44)
	DWORD       dwLineConfigNumberSize;
	DWORD       dwLineNameOffset;
	DWORD       dwLineNameSize;
	DWORD       dwLineDialingNumberOffset;		//Phone Number (Dialable) offset
	DWORD       dwLineDialingNumberSize;		//Phone Number (Dialable) size
	DWORD       dwLineDeviceType;				//Line Device Type
	DWORD       dwCookie;			//In addLine the cookie will be transferred to the created line
} ECSTACONTROLDATA_GETLINECONFIGV3;

typedef struct _ECSTACONTROLDATA_GETLINECONFIGLISTV3
{
	DWORD       dwCount;
	ECSTACONTROLDATA_GETLINECONFIGV3 lineConfig[1];
} ECSTACONTROLDATA_GETLINECONFIGLISTV3;

/* Line DevCaps Devspecific extension */
typedef struct _ESNOMLINEDEVCAPS
{
	//Extended Version dieses ECSTA (ECSTACAPSEXTVERSION1)
	DWORD       dwCapsExtVersion;
	//Permanent Provider ID dieser Line
	DWORD       dwPermanentProviderID;
	//UUID der Provider Instanz z.B. {08124B5B-1D7F-49fe-80D0-BFC22B365982}
	//bleibt konstant solange der Treiber installiert ist.
	//Dient zur Zuordnung der Leitungen zu der jeweiligen Control Line
	wchar_t     szControlUUID[50];
} ESNOMLINEDEVCAPS;

/********************************************************************/
//lineGetDevConfig: Struct used in LPVARSTRING lpDeviceConfig
//deviceClass "tapi/line"
/********************************************************************/
#define LINEDEVCONFIGCHALLENGESIZE 24

#define ECSTALINEDEVCONFIGFLAG_CHALLENGE		0x00000001
typedef struct _ECSTALINEDEVCONFIG
{
	DWORD dwSize;	//Size of Struct
	DWORD dwFlags; //Flags what to do ECSTALINEDEVCONFIGFLAG_
	unsigned char ucChallenge[LINEDEVCONFIGCHALLENGESIZE];	//Challenge used for CtiServer
} ECSTALINEDEVCONFIG;

/********************************************************************/
//DevSpecific Element
/********************************************************************/
typedef struct _ECSTADEVSPECIFICELEMENT
{
	DWORD dwElementID;	// Element ID
	DWORD dwOffset;		// Offset for Data (base Address is the beginning of the ECSTADEVSPECIFICELEMENTLIST Structure)
	DWORD dwSize;		// Size of Data
} ECSTADEVSPECIFICELEMENT;

typedef struct _ECSTADEVSPECIFICELEMENTLIST
{
	DWORD       dwTotalSize;
	DWORD       dwNeededSize;
	DWORD       dwUsedSize;
	DWORD		dwNumElements;	// Number of elements in the list
	ECSTADEVSPECIFICELEMENT elements[1];
} ECSTADEVSPECIFICELEMENTLIST;


// Neue Liste um eine Version in der Liste zu haben und einen Magic Identifier
typedef struct _ECSTADEVSPECIFICELEMENTLISTV2
{
	DWORD       dwTotalSize;
	DWORD       dwNeededSize;
	DWORD       dwUsedSize;
	DWORD		dwVersion;		// Version of the Structure 
	// Currently we use 1, may change in the future if the structur changes
	DWORD		dwMagicValue;	// Magic Value to identify our object 
	// Currently we use 0x07072018 :)
	DWORD		dwNumElements;	// Number of elements in the list
	ECSTADEVSPECIFICELEMENT elements[1];
} ECSTADEVSPECIFICELEMENTLISTV2;


// USING ECSTADEVSPECIFICELEMENTLIST
// *********************************

//Contains ECSTADEVSPECIFICELEMENT_GETSTATUSINFORMATIONELEMENT
#define ECSTADEVSPECIFICELEMENT_GETSTATUSINFORMATION		0x00000001
//String - "unknown" "deskphone" "dect" "mobile"
#define ECSTADEVSPECIFICELEMENT_ADDRESSDEVICETYPE			0x00000002
//String - Name of Device z.b. Model Name
#define ECSTADEVSPECIFICELEMENT_ADDRESSDEVICENAME			0x00000003
//String - ID of Device (z.b. MAC oder IP)
#define ECSTADEVSPECIFICELEMENT_ADDRESSDEVICEID				0x00000004
//DWORD (size = 4) Transport von weiteren Adressflags (siehe ECSTA_ADDRESSFLAG_)
#define ECSTADEVSPECIFICELEMENT_ADDRESSFLAGS				0x00000005
//DWORD (dwSize = 4) der ECSTA Call Features (In lineGetCallStatus)
#define ECSTADEVSPECIFICELEMENT_ECSTACALLFEATURES			0x00000006
//ECSTADEVSPECIFICELEMENT_CALLRECORDINGPARAMS ECSTA Custom Call Command (In lineDevspecific)
#define ECSTADEVSPECIFICELEMENT_CALLRECORDINGSTART			0x00000007
//ECSTADEVSPECIFICELEMENT_CALLRECORDINGPARAMS ECSTA Custom Call Command (In lineDevspecific)
#define ECSTADEVSPECIFICELEMENT_CALLRECORDINGSTOP			0x00000008
//ECSTADEVSPECIFICELEMENT_VALIDATESERVERCREDENTIALSELEMENT (In control line)
#define ECSTADEVSPECIFICELEMENT_VALIDATESERVERCREDENTIALS	0x00000009

// USING ECSTADEVSPECIFICELEMENTLISTV2
// ***********************************

#define ECSTADEVSPECIFICELEMENTLISTV2_MAGICVALUE			0x07072018
#define ECSTADEVSPECIFICELEMENTLISTV2_VERSION				1

//Call Link Data der CSTA Schnittstelle für einen Call
#define ECSTADEVSPECIFICELEMENT_CALLLINKAGEID				0x00000000A

//! Ist dies eine Dynamische Adresse (ECSTA MX-ONE oder NSX) - ProCall zeigt Leitungsbuttons für diese Adresse
#define ECSTA_ADDRESSFLAG_DYNAMIC_ADDRESS					0x00000001
//! Die Calls auf der Adresse 0 sind nicht zwingend auf dieser Adresse(Gerät) - ProCall zeigt immer alle Stati auf allen AddressButtons
#define ECSTA_ADDRESSFLAG_CALLADDRESSINVALID				0x00000002
//! Diese Adresse kann mit setactiveAddress an die PBX (NSX) weitergegeben werden
#define ECSTA_ADDRESSFLAG_ALLOWACTIVEADDRESS				0x00000004
//! Ist diese Adresse ist die ActiveAddress (NSX)
#define ECSTA_ADDRESSFLAG_IS_ACTIVEADDRESS					0x00000008
//! Ist diese Adresse ist eine Adresse alleine für Rufumleitungen
#define ECSTA_ADDRESSFLAG_SUPPORTS_FORWARDS_ONLY			0x00000010
//! Ist diese Adresse ist eine Gruppenrufnummer
#define ECSTA_ADDRESSFLAG_IS_ACD_GROUP_ADDRESS				0x00000020

// ALTE KONSTANTEN (11.07.2024 Jan Fellner)
#ifdef ECSTA_LEGACY_ADDRESSFLAGS
#define ECSTADEVSPECIFICELEMENT_ACTIVEADDRESSFLAGS			ECSTADEVSPECIFICELEMENT_ADDRESSFLAGS
#define ECSTA_ACTIVEADDRESSFLAG_DYNAMIC_ADDRESS				ECSTA_ADDRESSFLAG_DYNAMIC_ADDRESS
#define ECSTA_ACTIVEADDRESSFLAG_CALLADDRESSINVALID			ECSTA_ADDRESSFLAG_CALLADDRESSINVALID
#define ECSTA_ACTIVEADDRESSFLAG_ALLOWACTIVEADDRESS			ECSTA_ADDRESSFLAG_ALLOWACTIVEADDRESS
#define ECSTA_ACTIVEADDRESSFLAG_IS_ACTIVEADDRESS			ECSTA_ADDRESSFLAG_IS_ACTIVEADDRESS
#endif

/********************************************************************/
//DevSpecific Element StatusInformation
/********************************************************************/

typedef struct _ECSTADEVSPECIFICELEMENT_GETSTATUSINFORMATIONELEMENT
{
	DWORD dwECSTAPBXConnectionStatus;
	DWORD dwECSTAConnectFailedDetail;
} ECSTADEVSPECIFICELEMENT_GETSTATUSINFORMATIONELEMENT;

#define ECSTA_PBXCONNECTIONSTATUS_NOERROR					0x00000000
#define ECSTA_PBXCONNECTIONSTATUS_CONNECTFAILED				0x00000001
#define	ECSTA_PBXCONNECTIONSTATUS_LOGINFAILED				0x00000002
#define	ECSTA_PBXCONNECTIONSTATUS_SESSIONCOUNTEXCEEDED		0x00000004
#define	ECSTA_PBXCONNECTIONSTATUS_LICENSEFAILUREWITHINPBX	0x00000008
#define	ECSTA_PBXCONNECTIONSTATUS_UNKNOWN					0xffffffff

//Connection Failed Detail
#define ECSTA_PBXCONNECTIONFAILEDDETAIL_UNKNOWN				0x00000000
//Connection Failed Detail (ECSTA_PBXCONNECTIONSTATUS_LOGINFAILED Details)
#define ECSTA_PBXCONNECTIONFAILEDDETAIL_NONEXIST_EXT			101
#define ECSTA_PBXCONNECTIONFAILEDDETAIL_INVALID_PASSWORD		102
#define ECSTA_PBXCONNECTIONFAILEDDETAIL_PASSWORD_LOCKED			103
#define ECSTA_PBXCONNECTIONFAILEDDETAIL_INVALID_EXTSETTING		104
#define ECSTA_PBXCONNECTIONFAILEDDETAIL_MAX_CONNECTION_EXCEED	105
#define ECSTA_PBXCONNECTIONFAILEDDETAIL_INSUFFICIENT_LICENSE	106
#define ECSTA_PBXCONNECTIONFAILEDDETAIL_INVALID_VERSION			152
#define ECSTA_PBXCONNECTIONFAILEDDETAIL_SERVER_BUSY				153

//Type of First Pty Client - must match ECTIFIRSTPTY_CLIENT_XXX in UCServer ICtiClientSouce.h
//ECSTA_PBXCONNECTION_CLIENT_NONE means 3rd Party normal Operation (no Client Login)
#define ECSTA_PBXCONNECTION_CLIENT_NONE			0
#define ECSTA_PBXCONNECTION_CLIENT_BASIC		1
#define ECSTA_PBXCONNECTION_CLIENT_PRO			2
#define ECSTA_PBXCONNECTION_CLIENT_OPERATOR		3
#define ECSTA_PBXCONNECTION_CLIENT_SUPERVISOR	4

//Element for validate Server Credentials
typedef struct _ECSTADEVSPECIFICELEMENT_VALIDATESERVERCREDENTIALSELEMENT
{
	DWORD			dwClientType;
	wchar_t			szServer[100];
	unsigned int	uiPort;
	bool			useTLS;
	wchar_t			szUser[30];
	wchar_t			szPasswd[30];
	//Results are in here...
	DWORD dwECSTAPBXConnectionStatus;
	DWORD dwECSTAConnectFailedDetail;
} ECSTADEVSPECIFICELEMENT_VALIDATESERVERCREDENTIALSELEMENT;



//ECSTA Call Features in Devspecific Element in lineGetCallStatus
//Start Call Recording
#define ECSTA_CALLFEATURE_CALLRECORDINGSTART	0x00000001
//Stop Call Recording, nur gesetzt, wenn gerade ein CallRecording läuft
#define ECSTA_CALLFEATURE_CALLRECORDINGSTOP	0x00000002

//Element for ECSTADEVSPECIFICELEMENT_CALLRECORDINGSTART und ECSTADEVSPECIFICELEMENT_CALLRECORDINGSTOP
typedef struct _ECSTADEVSPECIFICELEMENT_CALLRECORDINGPARAMS
{
	wchar_t szVoiceMailNumber[16];
} ECSTADEVSPECIFICELEMENT_CALLRECORDINGPARAMS;

//Element for lineGetDevConfig "ecsta/MessageWaitingParams"
typedef struct _ECSTADEVSPECIFICELEMENT_MESSAGEWAITINGPARAMS
{
	DWORD dwNumMessages;
	wchar_t szVoiceMailNumber[16];
} ECSTADEVSPECIFICELEMENT_MESSAGEWAITINGPARAMS;

//Element for lineGetDevConfig "ecsta/AbsentMessage" und ECSTA150_FEATURE_ABSENTMESSAGE
typedef struct _ECSTADEVSPECIFICELEMENT_ABSENTMESSAGEPARAMS
{
	DWORD dwcaLoginState;	//Ignored in set operation
	DWORD dwumPresence;
	wchar_t szAbsentMessage[17];
} ECSTADEVSPECIFICELEMENT_ABSENTMESSAGEPARAMS;

//Element for lineGetDevConfig "ecsta/AgentFeatures"
typedef struct _ECSTADEVSPECIFICELEMENT_SUPPORTEDAGENTFEATURES
{
	DWORD dwSupportedAgentStates;
} ECSTADEVSPECIFICELEMENT_SUPPORTEDAGENTFEATURES;

// Mask which agent states the driver supports, 
#define ECSTA150_SUPPORTEDAGENTSTATES_LOGGEDOUT					0x000000001
#define ECSTA150_SUPPORTEDAGENTSTATES_LOGGEDIN_NOTREADY			0x000000002
#define ECSTA150_SUPPORTEDAGENTSTATES_LOGGEDIN_READY			0x000000004
#define ECSTA150_SUPPORTEDAGENTSTATES_LOGGEDIN_BUSY				0x000000008
#define ECSTA150_SUPPORTEDAGENTSTATES_LOGGEDIN_WORKINGAFTERCALL	0x000000010

#define ECSTA150_SUPPORTEDAGENTSTATES_ALL						(ECSTA150_SUPPORTEDAGENTSTATES_LOGGEDOUT | ECSTA150_SUPPORTEDAGENTSTATES_LOGGEDIN_NOTREADY | ECSTA150_SUPPORTEDAGENTSTATES_LOGGEDIN_READY | ECSTA150_SUPPORTEDAGENTSTATES_LOGGEDIN_BUSY | ECSTA150_SUPPORTEDAGENTSTATES_LOGGEDIN_WORKINGAFTERCALL)
#define ECSTA150_SUPPORTEDAGENTSTATES_MASK						0x0000000ff

// Für das setzen von Not Ready ist ein Reason zu übergeben (Flag in dwSupportedAgentStates)
#define ECSTA150_SUPPORTEDAGENTSTATES_LOGGEDIN_NOTREADY_REQUIRES_REASON	0x000000100

//Element for lineGetDevConfig "ecsta/AgentFeatures"
typedef struct _ECSTADEVSPECIFICELEMENT_SUPPORTEDAGENTFEATURES2 : public _ECSTADEVSPECIFICELEMENT_SUPPORTEDAGENTFEATURES
{
	// Welche Parameter werden beim Login/Logout benötigt?
	// ECSTA150_AGENTLOGIN_AGENTID Konstanten
	DWORD dwRequiredLoginLogoutParams;
} ECSTADEVSPECIFICELEMENT_SUPPORTEDAGENTFEATURES2;

// Für das Login/Logout benötigte Informationen vom Treiber, der Client frägt genau diese Informationen ab
#define ECSTA150_AGENTLOGIN_AGENTID								0x00000001
#define ECSTA150_AGENTLOGIN_GROUPID								0x00000002
#define ECSTA150_AGENTLOGIN_PASSWORD							0x00000004
#define ECSTA150_AGENTLOGIN_DEVICEID							0x00000008
#define ECSTA150_AGENTLOGIN_MASK								0x000000ff
#define ECSTA150_AGENTLOGOUT_PASSWORD							0x00000100
#define ECSTA150_AGENTLOGOUT_GROUPID							0x00000200
#define ECSTA150_AGENTLOGOUT_MASK								0x0000ff00

//ECSTA lineDevspecific Struct (Next Generation with element List)
#define ECSTA150DEVSPECIFICNEXTLISTID 0xdeadfeed
typedef struct _ECSTA150DEVSPECIFICNEXTLIST
{
	DWORD       dwID;	//bei den anderen ist das dwAgentstate, dieser Dummy Code ist immer ECSTA150DEVSPECIFICNEXTLISTID um das Struct von den alten zu unterscheiden
	DWORD       dwTotalSize;
	DWORD       dwNeededSize;
	DWORD       dwUsedSize;
	DWORD       dwNumElements;	//Number of elements
	ECSTADEVSPECIFICELEMENT elements[1];
} ECSTA150DEVSPECIFICNEXTLIST;

#endif