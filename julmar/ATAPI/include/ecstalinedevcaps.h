#ifndef _ECSTALINEDEVCAPSGLOBAL_H
#define _ECSTALINEDEVCAPSGLOBAL_H

//Erste Version mit Dynamic Line Create
#define ECSTACAPSEXTVERSION1			1
//Zweite version mit Async LineOpen
#define ECSTACAPSEXTVERSION2			2
//Dritte version mit ECSTACapabilities
#define ECSTACAPSEXTVERSION3			3
//Vierte Version mit ECSTA Version
#define ECSTACAPSEXTVERSION4			4

/* Line DevCaps Devspecific extension (Version 1 und Version 2)*/
typedef struct _ECSTALINEDEVCAPS
{
	//Extended Version dieses ECSTA (ECSTACAPSEXTVERSION1)
	DWORD       dwCapsExtVersion;
	//Permanent Provider ID dieser Line
	DWORD       dwPermanentProviderID;
	//UUID der Provider Instanz z.B. {08124B5B-1D7F-49fe-80D0-BFC22B365982}
	//bleibt konstant solange der Treiber installiert ist.
	//Dient zur Zuordnung der Leitungen zu der jeweiligen Control Line
	wchar_t     szControlUUID[50];
	//Falls die Line vom CtiServer mit Devspcific hinzugefügt wurde, hat sie diesen Cookie
	DWORD       dwCreateCookie;
} ECSTALINEDEVCAPS;

//Property ob query PBX Line exists verwendet werden kann 
//Speziell für CtiServer ob Leitungen automatisch anlegen angeboten wird
#define ECSTACONTROLCAPS_QUERYLINEEXISTS			0x00000001
#define ECSTACONTROLCAPS_QUERYLINEEXISTSV3			0x00000002

//Property, ob diese Leitung das Feature RemoteOffice Unterstützt
#define ECSTACAPS_REMOTEOFFICE_NATIVE				0x00000004		// Native Unterstützung seitens der PBX
#define ECSTACAPS_REMOTEOFFICE_EMULATED				0x00000010		// Emulation wird mit diesem Treiber unterstützt
#define ECSTACAPS_REMOTEOFFICE_NOTSUPPORTED			0x00000020		// Emulation wird mit diesem Treiber NICHT unterstützt

//Property, ob diese Leitung das Feature InvalidPassword (Broadsoft) unterstützt
#define ECSTACAPS_INVALIDPASSWORD					0x00000008

// ECSTA unterstützt die ECSTACALLINFOS die zu einem Call weitere Infos liefern
#define ECSTACAPS_ECSTACALLINFOS					0x00000040

// ECSTA unterstützt die ActiveAddress - Eine Addresse einer Leitung wird als Default zum Wählen / Annehmen definiert (Panasonic NSX)
#define ECSTACAPS_ACTIVEADDRESS						0x00000080

// ECSTA unterstützt Call Recording (Panasonic)
#define ECSTACAPS_CALLRECORDING						0x00000100

// ECSTA unterstützt Advanced MessageWaiting (Anzahl der Nachrichten) in lineDevspecific (ECSTA150DEVSPECIFICNEXTLIST - ECSTADEVSPECIFICELEMENT_MESSAGEWAITING)
#define ECSTACAPS_MESSAGEWAITINGADV					0x00000200

// ECSTA unterstützt das Ändern des CSTA Passwords (CA Pro) Panasonic lineDevspecific auf der Control Line
#define ECSTACONTROLCAPS_CHANGEPBXPASSWORD			0x00000400

// ECSTA unterstützt das Ändern der Presence und AbsentMessage am Telefon (Panasonic)
#define ECSTACAPS_PHONEPRESENCE						0x00000800

// ECSTA unterstützt die AgentGroups Funtionalität (Panasonic)
#define ECSTACAPS_AGENTGROUPS						0x00001000

// ECSTA unterstützt die AgentState Schnittstelle/Funtionalität (z.B. OS4K, nicht für PBXen die AGENTGROUP support anbieten)
#define ECSTACAPS_AGENTSUPPORT						0x00002000

// ECSTA stellt die CallLinkageID in der CallInfo als DevSpecific Extension bereit
#define ECSTACAPS_CALLLINKAGEID						0x00004000

// ECSTA stellt eine Park Leitung bereit welche die verschiedenen Parkpositionen der PBX überwachen kann
#define ECSTACAPS_SUPPORTS_PARK_LINE				0x00008000

// ECSTA stellt eine Que Leitung bereit welche die verschiedenen Wartefelder der PBX überwachen kann
#define ECSTACAPS_SUPPORTS_QUE_LINE					0x00010000

/* Line DevCaps Devspecific extension (Version 3) */
typedef struct _ECSTALINEDEVCAPS3 : public _ECSTALINEDEVCAPS
{
	//ECSTA Control Line Capabilities - gilt auch für die anderen Leitungen
	//ECSTACONTROLCAPS_QUERYLINEEXISTS
	//ECSTACONTROLCAPS_QUERYLINEEXISTSV2
	DWORD       dwECSTACapabilities;
} ECSTALINEDEVCAPS3;

/* Line DevCaps Devspecific extension (Version 4) */
//* Hinzugekommen - Treiber Version
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

#endif