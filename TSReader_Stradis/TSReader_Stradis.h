
typedef BOOL (* td_SetupStradis) (PVARIABLES pv);
typedef void (* td_ShutdownStradis) ();
BOOL (*SetupStradis) (PVARIABLES pv);
void (*ShutdownStradis) ();

typedef BOOL (* td_XNS_SetupSocket) (PVARIABLES pv);
typedef void (* td_XNS_ShutdownSocket) ();
BOOL (*XNS_SetupSocket) (PVARIABLES pv);
void (*XNS_ShutdownSocket) ();

typedef BOOL (* td_UDP_SetupSocket) (PVARIABLES pv);
typedef void (* td_UDP_ShutdownSocket) ();
BOOL (*UDP_SetupSocket) (PVARIABLES pv);
void (*UDP_ShutdownSocket) ();

