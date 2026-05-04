#ifdef PRO

#include <windows.h>
#include <commctrl.h>
#include <winsock.h>

#include "TSReader.h"
#include "bcdmux.h"
#include "util.h"

extern PVARIABLES v;

void Base64Encode(char * szEncoded, char * szPlain)
{
	int i,hiteof= FALSE;
	BYTE dtable[256];

	for(i= 0;i<9;i++)
	{
		dtable[i]= 'A'+i;
		dtable[i+9]= 'J'+i;
		dtable[26+i]= 'a'+i;
		dtable[26+i+9]= 'j'+i;
	}
	for(i= 0;i<8;i++)
	{
		dtable[i+18]= 'S'+i;
		dtable[26+i+18]= 's'+i;
	}
	for(i= 0;i<10;i++)
	{
		dtable[52+i]= '0'+i;
	}
	dtable[62]= '+';
	dtable[63]= '/';

	while(!hiteof)
	{
		byte igroup[3],ogroup[4];
		int c,n;

		igroup[0]= igroup[1]= igroup[2]= 0;
		for(n= 0;n<3;n++)
		{
			c = *szPlain++;
			if(c == 0)
			{
				hiteof= TRUE;
				break;
			}
			igroup[n]= (byte)c;
		}
		if(n> 0)
		{
			ogroup[0]= dtable[igroup[0]>>2];
			ogroup[1]= dtable[((igroup[0]&3)<<4)|(igroup[1]>>4)];
			ogroup[2]= dtable[((igroup[1]&0xF)<<2)|(igroup[2]>>6)];
			ogroup[3]= dtable[igroup[2]&0x3F];

			if(n<3)
			{
				ogroup[3]= '=';
				if(n<2)
				{
					ogroup[2]= '=';
				}
			}
			for(i= 0;i<4;i++)
			{
				*szEncoded++ = ogroup[i];
			}
		}
	}
}

void Base64Decode(char * szPlain, char * szEncoded)
{
	int i;
	BYTE dtable[256];

	for(i= 0;i<255;i++)
		dtable[i]= 0x80;
	for(i= 'A';i<='I';i++)
		dtable[i]= 0+(i-'A');
	for(i= 'J';i<='R';i++)
		dtable[i]= 9+(i-'J');
	for(i= 'S';i<='Z';i++)
		dtable[i]= 18+(i-'S');
	for(i= 'a';i<='i';i++)
		dtable[i]= 26+(i-'a');
	for(i= 'j';i<='r';i++)
		dtable[i]= 35+(i-'j');
	for(i= 's';i<='z';i++)
		dtable[i]= 44+(i-'s');
	for(i= '0';i<='9';i++)
		dtable[i]= 52+(i-'0');
	dtable['+']= 62;
	dtable['/']= 63;
	dtable['=']= 0;

	while(TRUE)
	{
		int j;
		BYTE a[4],b[4],o[3];

		for(i= 0;i < 4;i++)
		{
			int c = *szEncoded++;

			if(c == 0)
				return;
			if(dtable[c]&0x80)
			{
				i--;
				continue;
			}
			a[i]= (byte)c;
			b[i]= (byte)dtable[c];
		}
		o[0]= (b[0]<<2)|(b[1]>>4);
		o[1]= (b[1]<<4)|(b[2]>>2);
		o[2]= (b[2]<<6)|b[3];
		i = a[2] == '='?1:(a[3] == '=' ? 2:3);
		for (j = 0; j < i; j++)
			*szPlain++ = o[j];
		if(i<3)
			return;
	}
}

BOOL ReadSMTPSocket(SOCKET mysocket, char * szBuffer, int nSize)
{
	char c;
	int nCounter = 0;
	int nrecvRetVal;
	char szTemp[2];

	szTemp[1] = '\0';

	memset(szBuffer, 0, nSize);
	do
	{
		nrecvRetVal = recv(mysocket, &c, 1, 0);
		if ( (nrecvRetVal == 0) || (nrecvRetVal == SOCKET_ERROR) )
			return FALSE;
		szTemp[0] = c;
		if (c == '\n')
			return TRUE;
		if (c != '\r')
		{
			*szBuffer++ = c;
			nCounter++;
			if (nCounter > nSize)
				return TRUE;
		}
	} while (TRUE);
	return FALSE;			// should never get here
}

DWORD WINAPI EmailThread(LPVOID pPtr)
{
	int nStatus;
	PEVENTEMAILITEM pEventEmailData = (PEVENTEMAILITEM)pPtr;
	SOCKADDR_IN sin;
	SOCKET mysocket;
	char szBuffer[1024];
	char szTemp[128];
	char szTemp2[128];

	mysocket = socket(AF_INET, SOCK_STREAM, 0);
	if (mysocket == INVALID_SOCKET)
		goto EmailThreadWindup;

	if (FillAddr(&sin, pEventEmailData->szSMTPServer, 25) == FALSE)
	{
		OutputDebugString("Email: FillAddr() failed\n");
		goto EmailThreadWindup;
	}

	nStatus = connect(mysocket, (PSOCKADDR)&sin, sizeof(sin));
	if (nStatus == SOCKET_ERROR)
	{
		OutputDebugString("Email: Unable to connect to SMTP server\n");
		goto EmailThreadWindup;
	}

	do	// easy way out - break!
	{
		// Read the signon message from the SMTP server
		if (ReadSMTPSocket(mysocket, szBuffer, sizeof(szBuffer)) == FALSE)
		{
			OutputDebugString("Email: Socket error expecting 220 after connect\n");
			break;
		}
		szBuffer[3] = '\0';
		if (lstrcmp(szBuffer, "220") != 0)
		{
			OutputDebugString("Email: no 220 after connect\n");
			break;
		}

		if (v->fSMTPNeedsAuthentication)
		{
			BOOL fError = FALSE;
			char szHostname[MAX_PATH];
			
			// Send the EHLO and discard the responding 250 messages
			gethostname(szHostname, sizeof(szHostname));
			wsprintf(szTemp, "EHLO %s\r\n", szHostname);
			send(mysocket, szTemp, lstrlen(szTemp), 0);
			do
			{
				if (ReadSMTPSocket(mysocket, szBuffer, sizeof(szBuffer)) == FALSE)
				{
					fError = TRUE;
					break;
				}
				if (szBuffer[3] != '-')
					break;
			} while (TRUE);
			if (fError)
			{
				OutputDebugString("Email: Socket error reading data after EHLO\n");
				break;
			}

			wsprintf(szTemp, "AUTH LOGIN\r\n");
			send(mysocket, szTemp, lstrlen(szTemp), 0);
			
			do
			{
				if (ReadSMTPSocket(mysocket, szBuffer, sizeof(szBuffer)) == FALSE)
				{
					fError = TRUE;
					break;
				}
				if (memcmp(szBuffer, "235", 3) == 0)
					break;		// we authenticated
				else if (memcmp(szBuffer, "334", 3) == 0)
				{
					memset(szTemp, 0, sizeof(szTemp));
					Base64Decode(szTemp, &szBuffer[4]);
					strlwr(szTemp);
					if (strstr(szTemp, "username") != NULL)
					{
						memset(szBuffer, 0, sizeof(szBuffer));
						Base64Encode(szBuffer, v->szSMTPUsername);
						wsprintf(szTemp, "%s\r\n", szBuffer);
						send(mysocket, szTemp, lstrlen(szTemp), 0);
					}
					else if (strstr(szTemp, "password") != NULL)
					{
						memset(szBuffer, 0, sizeof(szBuffer));
						Base64Encode(szBuffer, v->szSMTPPassword);
						wsprintf(szTemp, "%s\r\n", szBuffer);
						send(mysocket, szTemp, lstrlen(szTemp), 0);
					}
					else
					{
						fError = TRUE;
						break;
					}
				}
				else
				{
					fError = TRUE;
					break;
				}
			} while (TRUE);
		}

		// Send "MAIL FROM:"
		wsprintf(szTemp, "MAIL FROM:<%s>\r\n", pEventEmailData->szEmailFrom);
		send(mysocket, szTemp, lstrlen(szTemp), 0);
		if (ReadSMTPSocket(mysocket, szBuffer, sizeof(szBuffer)) == FALSE)
			break;
		szBuffer[3] = '\0';
		if (lstrcmp(szBuffer, "250") != 0)
			break;

		// Send "RCPT TO:" (errors and warnings address)
		wsprintf(szTemp, "RCPT TO:<%s>\r\n", pEventEmailData->szEmailAddress);
		send(mysocket, szTemp, lstrlen(szTemp), 0);
		if (ReadSMTPSocket(mysocket, szBuffer, sizeof(szBuffer)) == FALSE)
			break;
		szBuffer[3] = '\0';
		if (lstrcmp(szBuffer, "250") != 0)
			break;

		// Send "DATA"
		send(mysocket, "DATA\r\n", 6, 0);
		if (ReadSMTPSocket(mysocket, szBuffer, sizeof(szBuffer)) == FALSE)
			break;
		szBuffer[3] = '\0';
		if (lstrcmp(szBuffer, "354") != 0)
			break;
		
		// Send the email header
		wsprintf(szTemp, "From: %s\r\n", pEventEmailData->szEmailFrom);
		send(mysocket, szTemp, lstrlen(szTemp), 0);
		wsprintf(szTemp, "To: %s\r\n", pEventEmailData->szEmailAddress);
		send(mysocket, szTemp, lstrlen(szTemp), 0);

		if (pEventEmailData->el.nEventType != EVENT_ICON_BODY_ONLY)
		{
			SYSTEMTIME stLocal;

			switch(pEventEmailData->el.nEventType)
			{
			case EVENT_ICON_INFORMATION:
				lstrcpy(szTemp2, "Info");
				break;
			case EVENT_ICON_WARNING:
				lstrcpy(szTemp2, "Warning");
				break;
			case EVENT_ICON_STOP:
				lstrcpy(szTemp2, "Error");
				break;
			}
			wsprintf(szTemp, "Subject: %s\r\n", szTemp2);
			send(mysocket, szTemp, lstrlen(szTemp), 0);
			if (pEventEmailData->el.nEventType != EVENT_ICON_INFORMATION)
			{
				lstrcpy(szTemp, "Importance: High\r\n");
				send(mysocket, szTemp, lstrlen(szTemp), 0);
			}
			send(mysocket, "\r\n", 2, 0);
			
			// Send the message
			SystemTimeToTzSpecificLocalTime(NULL, &pEventEmailData->el.stEvent, &stLocal);		
			wsprintf(szTemp, "%04d/%02d/%02d %02d:%02d:%02d\r\n", 
					 stLocal.wYear, stLocal.wMonth, stLocal.wDay,
					 stLocal.wHour, stLocal.wMinute, stLocal.wSecond);
			send(mysocket, szTemp, lstrlen(szTemp), 0);
			send(mysocket, pEventEmailData->el.szShort, lstrlen(pEventEmailData->el.szShort), 0);
			send(mysocket, "\r\n", 2, 0);
			send(mysocket, pEventEmailData->el.szLong, lstrlen(pEventEmailData->el.szLong), 0);
			send(mysocket, "\r\n", 2, 0);
		}
		else
		{
			send(mysocket, pEventEmailData->szMessageBody, lstrlen(pEventEmailData->szMessageBody), 0);
			LocalFree(pEventEmailData->szMessageBody);
		}

		// Terminate the send
		send(mysocket, "\r\n.\r\n", 5, 0);
		if (ReadSMTPSocket(mysocket, szBuffer, sizeof(szBuffer)) == FALSE)
			break;
		szBuffer[3] = '\0';
		if (lstrcmp(szBuffer, "250") != 0)
			break;
	} while (FALSE);

	closesocket(mysocket);
EmailThreadWindup:
	LocalFree(pEventEmailData);
	return 0;
}

#endif PRO
