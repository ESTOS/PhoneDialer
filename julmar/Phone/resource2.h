// Constants which cannot be re-ordered.
#define LINEERR_STR_ALLOCATED                       0x8001
#define LINEERR_STR_BADDEVICEID                     0x8002
#define LINEERR_STR_BEARERMODEUNAVAIL               0x8003
#define LINEERR_STR_CALLUNAVAIL                     0x8005
#define LINEERR_STR_COMPLETIONOVERRUN               0x8006
#define LINEERR_STR_CONFERENCEFULL                  0x8007
#define LINEERR_STR_DIALBILLING                     0x8008
#define LINEERR_STR_DIALDIALTONE                    0x8009
#define LINEERR_STR_DIALPROMPT                      0x800A
#define LINEERR_STR_DIALQUIET                       0x800B
#define LINEERR_STR_INCOMPATIBLEAPIVERSION          0x800C
#define LINEERR_STR_INCOMPATIBLEEXTVERSION          0x800D
#define LINEERR_STR_INIFILECORRUPT                  0x800E
#define LINEERR_STR_INUSE                           0x800F
#define LINEERR_STR_INVALADDRESS                    0x8010
#define LINEERR_STR_INVALADDRESSID                  0x8011
#define LINEERR_STR_INVALADDRESSMODE                0x8012
#define LINEERR_STR_INVALADDRESSSTATE               0x8013
#define LINEERR_STR_INVALAPPHANDLE                  0x8014
#define LINEERR_STR_INVALAPPNAME                    0x8015
#define LINEERR_STR_INVALBEARERMODE                 0x8016
#define LINEERR_STR_INVALCALLCOMPLMODE              0x8017
#define LINEERR_STR_INVALCALLHANDLE                 0x8018
#define LINEERR_STR_INVALCALLPARAMS                 0x8019
#define LINEERR_STR_INVALCALLPRIVILEGE              0x801A
#define LINEERR_STR_INVALCALLSELECT                 0x801B
#define LINEERR_STR_INVALCALLSTATE                  0x801C
#define LINEERR_STR_INVALCALLSTATELIST              0x801D
#define LINEERR_STR_INVALCARD                       0x801E
#define LINEERR_STR_INVALCOMPLETIONID               0x801F
#define LINEERR_STR_INVALCONFCALLHANDLE             0x8020
#define LINEERR_STR_INVALCONSULTCALLHANDLE          0x8021
#define LINEERR_STR_INVALCOUNTRYCODE                0x8022
#define LINEERR_STR_INVALDEVICECLASS                0x8023
#define LINEERR_STR_INVALDEVICEHANDLE               0x8024
#define LINEERR_STR_INVALDIALPARAMS                 0x8025
#define LINEERR_STR_INVALDIGITLIST                  0x8026
#define LINEERR_STR_INVALDIGITMODE                  0x8027
#define LINEERR_STR_INVALDIGITS                     0x8028
#define LINEERR_STR_INVALEXTVERSION                 0x8029
#define LINEERR_STR_INVALGROUPID                    0x802A
#define LINEERR_STR_INVALLINEHANDLE                 0x802B
#define LINEERR_STR_INVALLINESTATE                  0x802C
#define LINEERR_STR_INVALLOCATION                   0x802D
#define LINEERR_STR_INVALMEDIALIST                  0x802E
#define LINEERR_STR_INVALMEDIAMODE                  0x802F
#define LINEERR_STR_INVALMESSAGEID                  0x8030
#define LINEERR_STR_INVALPARAM                      0x8032
#define LINEERR_STR_INVALPARKID                     0x8033
#define LINEERR_STR_INVALPARKMODE                   0x8034
#define LINEERR_STR_INVALPOINTER                    0x8035
#define LINEERR_STR_INVALPRIVSELECT                 0x8036
#define LINEERR_STR_INVALRATE                       0x8037
#define LINEERR_STR_INVALREQUESTMODE                0x8038
#define LINEERR_STR_INVALTERMINALID                 0x8039
#define LINEERR_STR_INVALTERMINALMODE               0x803A
#define LINEERR_STR_INVALTIMEOUT                    0x803B
#define LINEERR_STR_INVALTONE                       0x803C
#define LINEERR_STR_INVALTONELIST                   0x803D
#define LINEERR_STR_INVALTONEMODE                   0x803E
#define LINEERR_STR_INVALTRANSFERMODE               0x803F
#define LINEERR_STR_LINEMAPPERFAILED                0x8040
#define LINEERR_STR_NOCONFERENCE                    0x8041
#define LINEERR_STR_NODEVICE                        0x8042
#define LINEERR_STR_NODRIVER                        0x8043
#define LINEERR_STR_NOMEM                           0x8044
#define LINEERR_STR_NOREQUEST                       0x8045
#define LINEERR_STR_NOTOWNER                        0x8046
#define LINEERR_STR_NOTREGISTERED                   0x8047
#define LINEERR_STR_OPERATIONFAILED                 0x8048
#define LINEERR_STR_OPERATIONUNAVAIL                0x8049
#define LINEERR_STR_RATEUNAVAIL                     0x804A
#define LINEERR_STR_RESOURCEUNAVAIL                 0x804B
#define LINEERR_STR_REQUESTOVERRUN                  0x804C
#define LINEERR_STR_STRUCTURETOOSMALL               0x804D
#define LINEERR_STR_TARGETNOTFOUND                  0x804E
#define LINEERR_STR_TARGETSELF                      0x804F
#define LINEERR_STR_UNINITIALIZED                   0x8050
#define LINEERR_STR_USERUSERINFOTOOBIG              0x8051
#define LINEERR_STR_REINIT                          0x8052
#define LINEERR_STR_ADDRESSBLOCKED                  0x8053
#define LINEERR_STR_BILLINGREJECTED                 0x8054
#define LINEERR_STR_INVALFEATURE                    0x8055
#define LINEERR_STR_NOMULTIPLEINSTANCE              0x8056
#define LINEERR_STR_INVALAGENTID                    0x8057
#define LINEERR_STR_INVALAGENTGROUP                 0x8058
#define LINEERR_STR_INVALPASSWORD                   0x8059
#define LINEERR_STR_INVALAGENTSTATE                 0x805A
#define LINEERR_STR_INVALAGENTACTIVITY              0x805B
#define LINEERR_STR_DIALVOICEDETECT                 0x805C
#define PHONEERR_STR_ALLOCATED                      0x9001
#define PHONEERR_STR_BADDEVICEID                    0x9002
#define PHONEERR_STR_INCOMPATIBLEAPIVERSION         0x9003
#define PHONEERR_STR_INCOMPATIBLEEXTVERSION         0x9004
#define PHONEERR_STR_INIFILECORRUPT                 0x9005
#define PHONEERR_STR_INUSE                          0x9006
#define PHONEERR_STR_INVALAPPHANDLE                 0x9007
#define PHONEERR_STR_INVALAPPNAME                   0x9008
#define PHONEERR_STR_INVALBUTTONLAMPID              0x9009
#define PHONEERR_STR_INVALBUTTONMODE                0x900A
#define PHONEERR_STR_INVALBUTTONSTATE               0x900B
#define PHONEERR_STR_INVALDATAID                    0x900C
#define PHONEERR_STR_INVALDEVICECLASS               0x900D
#define PHONEERR_STR_INVALEXTVERSION                0x900E
#define PHONEERR_STR_INVALHOOKSWITCHDEV             0x900F
#define PHONEERR_STR_INVALHOOKSWITCHMODE            0x9010
#define PHONEERR_STR_INVALLAMPMODE                  0x9011
#define PHONEERR_STR_INVALPARAM                     0x9012
#define PHONEERR_STR_INVALPHONEHANDLE               0x9013
#define PHONEERR_STR_INVALPHONESTATE                0x9014
#define PHONEERR_STR_INVALPOINTER                   0x9015
#define PHONEERR_STR_INVALPRIVILEGE                 0x9016
#define PHONEERR_STR_INVALRINGMODE                  0x9017
#define PHONEERR_STR_NODEVICE                       0x9018
#define PHONEERR_STR_NODRIVER                       0x9019
#define PHONEERR_STR_NOMEM                          0x901A
#define PHONEERR_STR_NOTOWNER                       0x901B
#define PHONEERR_STR_OPERATIONFAILED                0x901C
#define PHONEERR_STR_OPERATIONUNAVAIL               0x901D
#define PHONEERR_STR_RESOURCEUNAVAIL                0x901F
#define PHONEERR_STR_REQUESTOVERRUN                 0x9020
#define PHONEERR_STR_STRUCTURETOOSMALL              0x9021
#define PHONEERR_STR_UNINITIALIZED                  0x9022
#define PHONEERR_STR_REINIT                         0x9023
