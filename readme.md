PhoneDialer original copyright (c) 1995-2004 by JulMar Technology - http://www.julmar.com
PhoneDialer 4.0 copyright (c) 2016-2024 estos GmbH / Starnberg - http://www.estos.de

PhoneDialer 3.x/4.x extends the original JulMar phone dialer with additional features:

PhoneDialer 3.0:
- x64 build destination
- HostCallID, Appspecific, AdressID and CallFeatures Information as presented by the TAPI driver
- Show CallFeatures view in detail
- Creating Conference from two existing calls
- Separated ID and name presentation for Caller, Called, Connected, Redirecting and Redirection information
- List of call forwardings instead of a single call forwarding
- Removing of all forwardings
- Several enhancements and bugfixes
- extended Make Call to set clip no screening (hold control key while pushing Make Call button) 
- Make Call Pickup... Unpark with no destination Number (hold Control key to enable the buttons with no number present)
- Forwarding

PhoneDialer 3.1:
- Set DND within forwarding dialog
- Show DND within forwarding list

PhoneDialer 3.2:
- Show LineCallFeatures2 in detail

PhoneDialer 3.3:
- Added LINEADDRCAPFLAGS_DESTOFFHOOK to the extended MakeCall Options (provided by Innovaphone)
- Added LINECALLFEATURE2_TRANSFERNORM and LINECALLFEATURE2_TRANSFERCONF support. This enables PhoneDialer to correctly support european and american way of conference creation. (Via consultationCall (european) via setupConference (american))

PhoneDialer 3.4:
- Added Command Line Startup Parameter for automatic line open on ephone.exe start (ephone.exe 110)
- Fix for SetupConference if called destination drops the call phone.exe crashed

PhoneDialer 3.5:
- Added Scrollbar to callfeatures dialog

PhoneDialer 3.6:
- SetupTransfer uses the chooses the adress id from the phone dialer dialog.

PhoneDialer 3.7:
- Forwarding supports multiple addresses. If the monitored line offers multiple addresses the forwarding dialog shows on which address the forward is present. This enables modifikation of forwardings on the different addresses.

PhoneDialer 3.9:
- fixed Crash if call is getting deallocated while phone.exe is waiting for a specific callfeature
- fixed linedeallocate immediately after newcall has been announced for ip office (initialy reporting calls with linecallstate idle)

PhoneDialer 4.0:
- Date and Time within the status toolbar (to match screenshots with log)
- Dialog Caption contains the Version of the phone.exe to see if the current version has been used for capturing screenshots

PhoneDialer 4.0.0.1:
- When opening phone.exe it will automatically find other windows and place itself left or right of an existing phone.exe and try not to overlap others.

PhoneDialer 4.0.0.4:
- Added Panasonic Active Device selection
- Added Panasonic Group Agent State Dialog

PhoneDialer 4.0.0.5:
- Fixed crash on startup while searching proper window position

PhoneDialer 4.0.0.6:
- Fixed Agent Button enabled even if address does not support agent functionality
- Fixed address selection changes on call events

PhoneDialer 4.0.0.7:
- ConsultationCallDlg only shows transfer if both calls have proper CallFeatures set (LINECALLFEATURE_COMPLETETRANSF) and LineCallFeatures2 are empty or proper set
- removed MBCS and ansi build destinations
- now build with Visual Studio 2015

PhoneDialer 4.1.0.0:
- ECSTA Agent Login/Logout/Ready/NotReady/WorkingAfterCall support in Agent dialog
- now build with Visual Studio 2017

PhoneDialer 4.1.0.1:
- Added ECSTA CallLinkageID Support (Shows CSTA provided Call Linkag ID in CallData Dialog)
- Fixed several Crash issues in context to the CallData Dialog

PhoneDialer 4.2.0.0:
- Agent Login Parameter from ECSTA 
- Fixed callinfo dwdevspecific size issue
- Replaced stdstring.h

PhoneDialer 4.2.0.1:
- Added NumRings Support for forwarding (Visualization of the configured num of rings in the forwarding list)

PhoneDialer 4.3.0.1:
- Reworked CallLinkageID Support (improved version compatibility and structure detection)

PhoneDialer 4.4.0.0:
- Agent Login now supports the login parameters (AgentID, DeviceID, GroupID, Password) as required by the specific ECSTA

PhoneDialer 4.4.0.1:
- Saving agent login parameters per line

PhoneDialer 4.4.0.2:
- Agent Logout may require a password. Added logout parameters if the driver requires them.

PhoneDialer 4.4.0.3:
- Fixed HIGH DPI issues

PhoneDialer 4.4.0.5:
- Do no longer show "line unavailable" for lines which have been removed on TAPI when restarting the ephone.exe

Sourcecode contains all modifications, and the required ATAPI lib also provided from JulMar Technology.

Sourcecode is available for download from https://github.com/ESTOS/PhoneDialer

Provided as Visual Studio 2022 project. (Visual Studio 2022 (c) Microsoft Corporation)