void MD__Unload_External_Dll(int Nummer);
void MD__Send_External_DLL_Menu_Cmd(unsigned int MessageId);
int MD__Load_External_Dll(HINSTANCE hInstance);
void MD__StartPluginsRunning(HINSTANCE hInst, HWND hWnd);
BOOL MD__ExternCommandDispatch(HWND hWnd,UINT message,UINT wParam,LONG lParam);
void MD__ChannelChange(int nProgramNumber,
					   int nVideoPID, int nAudioPID, int nTeletextPID, int nPCRPID,
					   int nPMTPID, int nECMPID, TCA_System *CA,
					   char * szChannelName);
void MD__DataToFilters(BYTE * pData, int * nSize, int * nPacketLength);
void MD__Shutdown();
void MD__IPDataToFilters(int nPID, BYTE * pData, PMPEIPPACKET pmpeippacket);


#define TSREADER_MDAPI_GET_PIDS				  0x01030000
#define TSREADER_MDAPI_SWITCH_IP_MODE		  0x01030001
#define TSREADER_MDAPI_DIALOG_MESSAGE_ENABLE  0x01030002
#define TSREADER_MDAPI_DIALOG_MESSAGE_DISABLE 0x01030003

typedef VOID (*Extern_IPData)(int nPID, BYTE * pData, PMPEIPPACKET pmpeippacket);
typedef BOOL (*Extern_Descriptor_Decode) (BOOL fWantDescription, int nNetworkPID, BYTE * szDescriptor, char * szDecodedName);
