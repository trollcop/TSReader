#pragma once

void ParseATSCEITPacket(BYTE * pSectionPointer, int nPacketLength, int nEITNumber);
void ParseATSCETTPacket(BYTE * pSectionPointer, int nPacketLength);
void ParseDVBEITPacket(BYTE * pSectionPointer, int nPacketLength);
int ParseDVBBAT(BYTE * pSectionPointer, int nSectionLength);
void ParseDVBSDTPacket(BYTE * pSectionPointer, int nPacketLength);
BOOL ParseDCIIPMTTypeThing(BYTE * pSectionPointer, int nPacketLength, int nCurrentProgramNumber);
BOOL ParseDCIIProgramNameMessage(BYTE * pSectionPointer, int nPacketLength, int nCurrentProgramNumber);
BOOL ParsePMTPacket(BYTE * pSectionPointer, int nPacketLength, int nCurrentProgramNumber, int nPMTListenIndex);
void ParseIPPacket(BYTE * pSectionPointer, int nPacketLength, int nPID, int nBufferNumber);
BOOL ParsePATPacket(BYTE * pSectionPointer, int nPacketLength);
BOOL ParsePSIPPacket(BYTE * pSectionPointer, int nPacketLength);
BOOL ParseCATPacket(BYTE * pSectionPointer, int nPacketLength);
void ParseDCIINetworkPacket(BYTE * pSectionPointer, int nPacketLength);
void ParseDVBRSTPacket(BYTE * pSectionPointer, int nPacketLength);
void ParseDVBTDTPacket(BYTE * pSectionPointer, int nPacketLength);
void ParseDVBNITPacket(BYTE * pSectionPointer, int nPacketLength);
void ParseDCIIECMPacket(BYTE * pSection, int nLength);
void ParseDVBINTPacket(BYTE * pSection, int nLength);
typedef void (* td_QuickParseUserData) (BYTE * pData, int user_data_len, int nESParsePMTIndex, int nESParseESIndex, int nES);
void QuickParseUserData(BYTE * pData, int user_data_len, int nESParsePMTIndex, int nESParseESIndex, int nES);
BOOL ParseISDBBITPacket(BYTE * pSectionPointer, int nPacketLength);
void ParseATSCCETT(BYTE * pSectionPointer, int nPacketLength);
