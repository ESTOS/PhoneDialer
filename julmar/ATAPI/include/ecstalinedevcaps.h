#ifndef _ECSTALINEDEVCAPSGLOBAL_H
#define _ECSTALINEDEVCAPSGLOBAL_H

//First version with Dynamic Line Create
#define ECSTACAPSEXTVERSION1			1
//Second version with Async LineOpen
#define ECSTACAPSEXTVERSION2			2
//Third version with ECSTACapabilities
#define ECSTACAPSEXTVERSION3			3
//Fourth version with ECSTA Version
#define ECSTACAPSEXTVERSION4			4

/* Line DevCaps Devspecific extension (Version 1 and Version 2)*/
typedef struct _ECSTALINEDEVCAPS
{
	//Extended version of this ECSTA (ECSTACAPSEXTVERSION1)
	DWORD       dwCapsExtVersion;
	//Permanent Provider ID of this line
	DWORD       dwPermanentProviderID;
	//UUID of the provider instance e.g. {08124B5B-1D7F-49fe-80D0-BFC22B365982}
	//remains constant as long as the driver is installed.
	//used to assign lines to the respective control line
	wchar_t     szControlUUID[50];
	//If the line was added by the CtiServer via Devspecific, it carries this cookie
	DWORD       dwCreateCookie;
} ECSTALINEDEVCAPS;

//Property whether query PBX Line exists can be used
//Specifically for CtiServer whether automatic line creation is offered
#define ECSTACONTROLCAPS_QUERYLINEEXISTS			0x00000001
#define ECSTACONTROLCAPS_QUERYLINEEXISTSV3			0x00000002

//Property whether this line supports the RemoteOffice feature
#define ECSTACAPS_REMOTEOFFICE_NATIVE				0x00000004		// Native support by the PBX
#define ECSTACAPS_REMOTEOFFICE_EMULATED				0x00000010		// Emulation supported by this driver
#define ECSTACAPS_REMOTEOFFICE_NOTSUPPORTED			0x00000020		// Emulation NOT supported by this driver

//Property whether this line supports the InvalidPassword feature (Broadsoft)
#define ECSTACAPS_INVALIDPASSWORD					0x00000008

// ECSTA supports ECSTACALLINFOS which provide additional info for a call
#define ECSTACAPS_ECSTACALLINFOS					0x00000040

// ECSTA supports ActiveAddress - one address of a line is defined as default for dialing / answering (Panasonic NSX)
#define ECSTACAPS_ACTIVEADDRESS						0x00000080

// ECSTA supports Call Recording (Panasonic)
#define ECSTACAPS_CALLRECORDING						0x00000100

// ECSTA supports Advanced MessageWaiting (number of messages) in lineDevspecific (ECSTA150DEVSPECIFICNEXTLIST - ECSTADEVSPECIFICELEMENT_MESSAGEWAITING)
#define ECSTACAPS_MESSAGEWAITINGADV					0x00000200

// ECSTA supports changing the CSTA Password (CA Pro) Panasonic lineDevspecific on the Control Line
#define ECSTACONTROLCAPS_CHANGEPBXPASSWORD			0x00000400

// ECSTA supports changing Presence and AbsentMessage on the phone (Panasonic)
#define ECSTACAPS_PHONEPRESENCE						0x00000800

// ECSTA supports AgentGroups functionality (Panasonic)
#define ECSTACAPS_AGENTGROUPS						0x00001000

// ECSTA supports AgentState interface/functionality (e.g. OS4K, not for PBXs that offer AGENTGROUP support)
#define ECSTACAPS_AGENTSUPPORT						0x00002000

// ECSTA provides the CallLinkageID in CallInfo as a DevSpecific extension
#define ECSTACAPS_CALLLINKAGEID						0x00004000

// ECSTA provides a park line that can monitor the various park positions of the PBX
#define ECSTACAPS_SUPPORTS_PARK_LINE				0x00008000

// ECSTA provides a queue line that can monitor the various wait queues of the PBX
#define ECSTACAPS_SUPPORTS_QUE_LINE					0x00010000

/* Line DevCaps Devspecific extension (Version 3) */
typedef struct _ECSTALINEDEVCAPS3 : public _ECSTALINEDEVCAPS
{
	//ECSTA Control Line Capabilities - also applies to the other lines
	//ECSTACONTROLCAPS_QUERYLINEEXISTS
	//ECSTACONTROLCAPS_QUERYLINEEXISTSV2
	DWORD       dwECSTACapabilities;
} ECSTALINEDEVCAPS3;

/* Line DevCaps Devspecific extension (Version 4) */
//* Added - driver version
typedef struct _ECSTALINEDEVCAPS4 : public _ECSTALINEDEVCAPS3
{
	wchar_t     szDriverVersion[50];	// 4.1.5.12056
} ECSTALINEDEVCAPS4;

class IECSTAUIProxyInterface
{
public:
	//ecstaproxy Interface for the ecstas (only used in UI Mode (provider Add / config)
	//Get Installed Provider IDs
	//szName: Instamnce name to test
	//dwExcludeProviderID: Provider ID to exclude from test
	//returns 0 if name is not in use
	virtual long GetProviderNameInUse(const wchar_t* szName) = 0;
};

/********************************************************************/
//TSP_UCServerInit struct to define Settings and special behavior in UCServer mode
/********************************************************************/
typedef struct _ECSTAUCServerProps
{
	DWORD dwSize; //struct size for Version compatibility
	const wchar_t* szLogDir;		//Logging Dir
	const wchar_t* szRegistryBase;	//Registry Base Path
	int bCurrentUser;				//Registry use current user instead of local machine
	int iFirstPartyMode;			//First Party (ProCall One) Mode
	LCID iLCIDServerUI;				//Language of the Server Admin User Interface
	const wchar_t* szConfigDir;		//Config File Directory
	IECSTAUIProxyInterface* pUIInterface;	//Interface to ecstaproxy for checking Driver Names
} ECSTAUCServerProps;

class IUCServerForECSTALogWriterFactory;
typedef struct _ECSTAUCServerProps2 : public _ECSTAUCServerProps
{
	IUCServerForECSTALogWriterFactory* pCustomLogWriterFactory;	// Interface for creating custom log writer, used for logging over UCServer
} ECSTAUCServerProps2;

#endif
