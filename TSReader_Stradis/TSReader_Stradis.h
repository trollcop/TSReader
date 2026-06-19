
typedef BOOL (* td_SetupStradis) (PVARIABLES pv);
typedef void (* td_ShutdownStradis) (void);
BOOL (*SetupStradis) (PVARIABLES pv);
void (*ShutdownStradis) (void);

typedef BOOL (* td_XNS_SetupSocket) (PVARIABLES pv);
typedef void (* td_XNS_ShutdownSocket) (PVARIABLES pv);
BOOL (*XNS_SetupSocket) (PVARIABLES pv);
void (*XNS_ShutdownSocket) (PVARIABLES pv);

typedef BOOL (* td_UDP_SetupSocket) (PVARIABLES pv);
typedef void (* td_UDP_ShutdownSocket) (void);
BOOL (*UDP_SetupSocket) (PVARIABLES pv);
void (*UDP_ShutdownSocket) (void);
