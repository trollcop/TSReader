#include <windows.h>
#include <commctrl.h>
#include <time.h>
#include <winsock.h>
#include <strsafe.h>
#include <stdint.h>
#include <shlobj.h>
#include "TSReader.h"
#include "bcdmux.h"
#include "util.h"
#include "Md/MULTIDEC/Globals.h"
#include "MDInterface.h"
#include "TSID.h"

// Stuff in ATSC_huffman.c
void ATSCHuffmanDecode(int nBitBufferIndex, int type, int bytes, char * outtext);

// In TSReader.c
int GetLogicalChannelNumber(int nProgramNumber);

extern PVARIABLES v;
extern Extern_Descriptor_Decode DescriptorDecode[5];

int nLayer1Rates[] = {-1, 32, 64, 96, 128, 160, 192, 224, 256, 288, 320, 352, 384, 416, 448};
int nLayer2Rates[] = {-1, 32, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320, 384};
int nLayer3Rates[] = {-1, 32, 40, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320};
const struct AC3frmsize AC3frmsizecod_tbl[] = {
      { 32  ,{64   ,69   ,96   } },
      { 32  ,{64   ,70   ,96   } },
      { 40  ,{80   ,87   ,120  } },
      { 40  ,{80   ,88   ,120  } },
      { 48  ,{96   ,104  ,144  } },
      { 48  ,{96   ,105  ,144  } },
      { 56  ,{112  ,121  ,168  } },
      { 56  ,{112  ,122  ,168  } },
      { 64  ,{128  ,139  ,192  } },
      { 64  ,{128  ,140  ,192  } },
      { 80  ,{160  ,174  ,240  } },
      { 80  ,{160  ,175  ,240  } },
      { 96  ,{192  ,208  ,288  } },
      { 96  ,{192  ,209  ,288  } },
      { 112 ,{224  ,243  ,336  } },
      { 112 ,{224  ,244  ,336  } },
      { 128 ,{256  ,278  ,384  } },
      { 128 ,{256  ,279  ,384  } },
      { 160 ,{320  ,348  ,480  } },
      { 160 ,{320  ,349  ,480  } },
      { 192 ,{384  ,417  ,576  } },
      { 192 ,{384  ,418  ,576  } },
      { 224 ,{448  ,487  ,672  } },
      { 224 ,{448  ,488  ,672  } },
      { 256 ,{512  ,557  ,768  } },
      { 256 ,{512  ,558  ,768  } },
      { 320 ,{640  ,696  ,960  } },
      { 320 ,{640  ,697  ,960  } },
      { 384 ,{768  ,835  ,1152 } },
      { 384 ,{768  ,836  ,1152 } },
      { 448 ,{896  ,975  ,1344 } },
      { 448 ,{896  ,976  ,1344 } },
      { 512 ,{1024 ,1114 ,1536 } },
      { 512 ,{1024 ,1115 ,1536 } },
      { 576 ,{1152 ,1253 ,1728 } },
      { 576 ,{1152 ,1254 ,1728 } },
      { 640 ,{1280 ,1393 ,1920 } },
      { 640 ,{1280 ,1394 ,1920 } }};
const double DTSSamplingRates[] =
	{
		0.0,
		8.0,
		16.0,
		32.0,
		64.0,
		128.0,
		11.025,
		22.05,
		44.1,
		88.2,
		176.4,
		12.0,
		24.0,
		48.0,
		96.0,
		192.0
	};
const double DTSBitRates[] =
	{
		0.0,				// 0
		0.0,				// 1
		0.0,				// 2
		0.0,				// 3
		0.0,				// 4
		128.0,				// 5
		192.0,
		224.0,
		256.0,
		320.0,
		384.0,
		448.0,
		512.0,
		576.0,
		640.0,
		768.0,
		960.0,
		1024.0,
		1152.0,
		1280.0,
		1344.0,
		1408.0,
		1411.2,
		1472.0,
		1536.0,
		1920.0,
		2048.0,
		3072.0,
		3840.0,
		0.0,
		0.0,
		0.0
};

double DecodeDVBLeakRate(int nInput)
{
	switch(nInput)
	{
	default:
		return -1;
	case 1:
		return 0.0009;
	case 2:
		return 0.0018;
	case 3:
		return 0.0036;
	case 4:
		return 0.0072;
	case 5:
		return 0.0108;
	case 6:
		return 0.0144;
	case 7:
		return 0.0216;
	case 8:
		return 0.0288;
	case 9:
		return 0.075;
	case 10:
		return 0.5;
	case 11:
		return 0.5625;
	case 12:
		return 0.8437;
	case 13:
		return 1.0;
	case 14:
		return 1.125;
	case 15:
		return 1.5;
	case 16:
		return 1.6875;
	case 17:
		return 2.0;
	case 18:
		return 2.25;
	case 19:
		return 2.5;
	case 20:
		return 3.0;
	case 21:
		return 3.375;
	case 22:
		return 3.5;
	case 23:
		return 4.0;
	case 24:
		return 4.5;
	case 25:
		return 5.0;
	case 26:
		return 5.5;
	case 27:
		return 6.0;
	case 28:
		return 6.5;
	case 29:
		return 6.75;
	case 30:
	case 31:
	case 32:
		return ((double)nInput - 16.0) * 0.5;
	case 33:
	case 34:
	case 35:
	case 36:
	case 37:
		return (double)nInput - 24.0;
	case 38:
		return 13.5;
	case 39:
	case 40:
	case 41:
	case 42:
	case 43:
		return (double)nInput - 25.0;
	case 44:
	case 45:
	case 46:
	case 47:
		return ((double)nInput - 34.0) * 2.0;
	case 48:
		return 27.0;
	case 49:
	case 50:
	case 51:
	case 52:
	case 53:
	case 54:
	case 55:
		return ((double)nInput - 35) * 2.0;
	case 56:
		return 44.0;
	case 57:
		return 48.0;
	case 58:
		return 54.0;
	case 59:
		return 72.0;
	case 60:
		return 108.0;
	}

	return -1;
}


void FormatDVBSCodeRate(char * szFEC, int nFECRate)
{
	switch(nFECRate)
	{
	case 1:
		lstrcpy(szFEC, "1/2");
		break;
	case 2:
		lstrcpy(szFEC, "2/3");
		break;
	case 3:
		lstrcpy(szFEC, "3/4");
		break;
	case 4:
		lstrcpy(szFEC, "5/6");
		break;
	case 5:
		lstrcpy(szFEC, "7/8");
		break;
	case 6:
		lstrcpy(szFEC, "8/9");
		break;
	case 7:
		lstrcpy(szFEC, "3/5");
		break;
	case 8:
		lstrcpy(szFEC, "4/5");
		break;
	case 9:
		lstrcpy(szFEC, "9/10");
		break;
	case 15:
		lstrcpy(szFEC, "1/1");
		break;
	default:
		wsprintf(szFEC, "Unknown (0x%02x)", nFECRate);
		break;
	}
}

void FormatDVBTCodeRate(char * szFEC, int nFECRate)
{
	switch(nFECRate)
	{
	case 0:
		lstrcpy(szFEC, "1/2");
		break;
	case 1:
		lstrcpy(szFEC, "2/3");
		break;
	case 2:
		lstrcpy(szFEC, "3/4");
		break;
	case 3:
		lstrcpy(szFEC, "5/6");
		break;
	case 4:
		lstrcpy(szFEC, "7/8");
		break;
	default:
		wsprintf(szFEC, "Unknown (0x%02x)", nFECRate);
		break;
	}
}

void FormatDVBTGuardInterval(char * szInterval, int nGuardInterval)
{
	switch(nGuardInterval)
	{
	case 0:
		lstrcpy(szInterval, "1/32");
		break;
	case 1:
		lstrcpy(szInterval, "1/16");
		break;
	case 2:
		lstrcpy(szInterval, "1/8");
		break;
	case 3:
		lstrcpy(szInterval, "1/4");
		break;
	}
}

void FormatDVBTTransmissionMode(char * szTransmissionMode, int nTransmissionMode)
{
	switch(nTransmissionMode)
	{
	case 0:
		lstrcpy(szTransmissionMode, "2k mode");
		break;
	case 1:
		lstrcpy(szTransmissionMode, "8k mode");
		break;
	case 2:
		lstrcpy(szTransmissionMode, "4k mode");
		break;
	default:
		lstrcpy(szTransmissionMode, "reserved for future use");
		break;
	}
}

static void FormatISDBTransmissionTypeInfo(char *szTransmissionType, size_t len, uint8_t transmission_type_info)
{
	const char *parameter_type[] = { "Type a", "Type b", "Type c", "Reserved" };
	const char *modulation_system[] = { "64QAM", "16QAM", "QPSK", "Reserved" };

	uint8_t parameter = (transmission_type_info >> 6) & 3;
	uint8_t modulation = (transmission_type_info >> 4) & 3;
	StringCchPrintf(szTransmissionType, len, "%s - %s", parameter_type[parameter], modulation_system[modulation]);
}

static const char *FormatISDBDataComponentId(uint16_t data_component_id)
{
	const char *isdb_data_component[] = {
		"", "", "", "", "", "", "",
		"ARIB-XML-base multimedia coding", /* 0x7 TR-B15 STD-B24 Sub-clause 9.3.2, Vol. 2; Appended specification C.1, Vol. 3*/
		"ARIB-Subtitle & teletext coding", /* TR-B14, B15 STD-B24 Sub-clause 9.6.1, Part 3, Vol. 1 */
		"ARIB-Data download", /* TR-B14, B15, B26 STD-B21 */
		"G-guide (G-Guide Gold)",
		"BML for 110E CS", /* TR-B15 Part 2 */
		"Multimedia coding for digital terrestrial broadcasting (A profile)", /* TR-B14 STD-B24 Sub-clause 9.3.2, Vol. 2; Appended specification C.1, Vol. 3 */
		"Multimedia coding for digital terrestrial broadcasting (C profile)", /* TR-B14 STD-B24 Sub-clause 9.3.2, Vol. 2; Appended specification C.1, Vol. 3 */
		"Multimedia coding for digital terrestrial broadcasting (P profile)", /* TR-B13 STD-B24 Sub-clause 9.3.2, Vol. 2; Appended specification C.1, Vol. 3 */
		"Multimedia coding for digital terrestrial broadcasting (E profile)", /* STD-B13 Sub-clause 5.3, Reference, Vol. 3 */
		"Real-time data service (Mobile profile)", /* TR-B26 STD-B24 Sub-clause 9.3.2, Vol. 2; Appended specification C.1, Vol. 3 */
		"Accumulation-type data service(Mobile profile)", /* TR-B26 STD-B24 Sub-clause 9.3.2, Vol. 2; Appended specification C.1, Vol. 3 */
		"Subtitle coding for digital terrestrial broadcasting (C profile)", /* TR-B14 STD-B24 Sub-clause 9.6.1, Part 3, Vol. 1 */
		"Multimedia coding for digital terrestrial broadcasting (P2 profile)",
		"Data carousel scheme for TYPE2 content transmission", /* TR-B27 */
		"DSM-CC section scheme for transmission of program start time information" /* 0x15 */,
		"ARIB-Descriptive language type metadata coding" /* 0x16 */,
		"Undefined" /* 0x17 */ };

	if (data_component_id >= 7 && data_component_id <= 0x16)
		return isdb_data_component[data_component_id];
	else
		return isdb_data_component[0x17];
}

static const char *FormatISDBDocumentResolution(uint8_t document_resolution)
{
	const char *isdb_document_resolution[] = {
		"Different resolution", /* 0 */
		"1920x1080 (16:9)",		/* 1 */
		"1280x720 (16:9)",		/* 2 */
		"960x540 (16:9)",		/* 3 */
		"720x480 (16:9)",		/* 4 */
		"720x480 (4:3)",		/* 5 */
		"320x240 (4:3)",		/* 6 */
		"Reserved",				/* 7+ */
		"No resolution specified"
	};

	if (document_resolution <= 7)
		return isdb_document_resolution[document_resolution];
	else if (document_resolution == 0xf)
		return isdb_document_resolution[8];
	else
		return isdb_document_resolution[7];
}

void FormatPolarity(char * szPolarity, int nPolarityIndicator, BOOL fLong)
{
	switch(nPolarityIndicator)
	{
	case 0:
		if (fLong)
			lstrcpy(szPolarity, "Horizontal");
		else
			lstrcpy(szPolarity, "H");
		break;
	case 1:
		if (fLong)
			lstrcpy(szPolarity, "Vertical");
		else
			lstrcpy(szPolarity, "V");
		break;
	case 2:
		if (fLong)
			lstrcpy(szPolarity, "LHCP");
		else
			lstrcpy(szPolarity, "L");
		break;
	case 3:
		if (fLong)
			lstrcpy(szPolarity, "RHCP");
		else
			lstrcpy(szPolarity, "R");
		break;
	}
}

void FormatDVBSModulation(char * szModulation, int nModulationIndicator)
{
	int nRollOff = nModulationIndicator >> 3;
	int nModulationSystem = nModulationIndicator >> 2;
	int nModulationType = nModulationIndicator & 3;
	
	switch(nModulationType)
	{
	case 0:
		lstrcpy(szModulation, "Auto");
		break;
	case 1:
		lstrcpy(szModulation, "QPSK");
		break;
	case 2:
		lstrcpy(szModulation, "8PSK (DVB-DSNG/Turbo/DVB-S2)");
		break;
	case 3:
		lstrcpy(szModulation, "16QAM (DVB-DNG)/QPSK (Turbo)");
		break;
	}

	switch(nModulationSystem)
	{
	case 0:
		lstrcat(szModulation, ", DVB-S");
		break;
	case 1:
		lstrcat(szModulation, ", DVB-S2 ");
		switch(nRollOff)
		{
		case 0:
			lstrcat(szModulation, "(a=.35)");
			break;
		case 1:
			lstrcat(szModulation, "(a=.25)");
			break;
		case 2:
			lstrcat(szModulation, "(a=.20)");
			break;
		case 3:
			lstrcat(szModulation, "(reserved)");
			break;
		}
		break;
	}
}

void DecodeStreamType(int nStreamType, char * szStreamTypeEnglish, int nPMTIndex, int nESIndex)
{
	switch(nStreamType)
	{
	case 0x00:
		lstrcpy(szStreamTypeEnglish, "ISO/IEC Reserved");
		break;
	case 0x01:
		lstrcpy(szStreamTypeEnglish, "MPEG-1 Video");
		break;
	case 0x02:
		lstrcpy(szStreamTypeEnglish, "MPEG-2 Video");
		break;
	case 0x03:
		lstrcpy(szStreamTypeEnglish, "MPEG-1 Audio");
		break;
	case 0x04:
		lstrcpy(szStreamTypeEnglish, "MPEG-2 Audio");
		break;
	case 0x05:
		lstrcpy(szStreamTypeEnglish, "ISO/IEC 13818-1 private_sections");
		break;
	case 0x06:
		if (nPMTIndex != -1 && nESIndex != -1)
		{
			// Let's see if this is one of them fancy streams
			if (IsAC3AudioStream(nPMTIndex, nESIndex) == TRUE)
			{
				lstrcpy(szStreamTypeEnglish, "Dolby AC3 Audio");
				break;
			}
			if (IsPCMAudioStream(nPMTIndex, nESIndex) == TRUE)
			{
				lstrcpy(szStreamTypeEnglish, "PCM Audio");
				break;
			}
			if (IsDTSAudioStream(nPMTIndex, nESIndex) == TRUE)
			{
				lstrcpy(szStreamTypeEnglish, "DTS Audio");
				break;
			}
			if (IsTeleTextOrVBIStream(nPMTIndex, nESIndex) == TRUE)
			{
				lstrcpy(szStreamTypeEnglish, "Teletext/VBI");
				break;
			}
		}
		lstrcpy(szStreamTypeEnglish, "ISO/IEC 13818-1 PES packets containing private data");
		break;
	case 0x07:
		lstrcpy(szStreamTypeEnglish, "ISO/IEC 13522 MHEG");
		break;
	case 0x08:
		lstrcpy(szStreamTypeEnglish, "ISO/IEC 13818-1 Annex A DSM CC");
		break;
		/*
0x11 ISO/IEC 14496-3 Audio with the LATM transport syntax as defined in ISO/IEC
0x1B AVC video stream as defined in ITU-T Rec. H.264 | ISO/IEC 14496-10 Video

  */
	case 0x09:
		lstrcpy(szStreamTypeEnglish, "ITU-T Rec. H.222.1");
		break;
	case 0x0A:
		lstrcpy(szStreamTypeEnglish, "ISO/IEC 13818-6 type A");
		break;
	case 0x0B:
		lstrcpy(szStreamTypeEnglish, "ISO/IEC 13818-6 type B");
		break;
	case 0x0C:
		lstrcpy(szStreamTypeEnglish, "ISO/IEC 13818-6 type C");
		break;
	case 0x0D:
		lstrcpy(szStreamTypeEnglish, "ISO/IEC 13818-6 type D");
		break;
	case 0x0E:
		lstrcpy(szStreamTypeEnglish, "ISO/IEC 13818-1 auxiliary");
		break;
	case 0x0f:
		lstrcpy(szStreamTypeEnglish, "MPEG-2 AAC Audio");
		break;
	case 0x10:
		lstrcpy(szStreamTypeEnglish, "MPEG-4 Video");
		break;
	case 0x11:
		lstrcpy(szStreamTypeEnglish, "MPEG-4 Audio");
		break;
	case 0x12:
		lstrcpy(szStreamTypeEnglish, "ISO/IEC 14496-1 SL-packetized stream");
		break;
	case 0x13:
		lstrcpy(szStreamTypeEnglish, "ISO/IEC 14496-1 SL-packetized stream");
		break;
	case 0x14:
		lstrcpy(szStreamTypeEnglish, "ISO/IEC 13818-6 Synchronized Download Protocol");
		break;
	case 0x15:
		lstrcpy(szStreamTypeEnglish, "Metadata carried in PES packets");
		break;
	case 0x16:
		lstrcpy(szStreamTypeEnglish, "Metadata carried in metadata_sections");
		break;
	case 0x17:
		lstrcpy(szStreamTypeEnglish, "Metadata carried in ISO/IEC 13818-6 Data Carousel");
		break;
	case 0x18:
		lstrcpy(szStreamTypeEnglish, "Metadata carried in ISO/IEC 13818-6 Object Carousel");
		break;
	case 0x19:
		lstrcpy(szStreamTypeEnglish, "Metadata carried in ISO/IEC 13818-6 Synchronized Download Protocol");
		break;
	case 0x1A:
		lstrcpy(szStreamTypeEnglish, "IPMP stream (defined in ISO/IEC 13818-11, MPEG-2 IPMP)");
		break;
	case 0x1B:
		lstrcpy(szStreamTypeEnglish, "H.264 Video");
		break;
	case 0x7f:
		lstrcpy(szStreamTypeEnglish, "IPMP stream");
		break;
	case 0x81:
		lstrcpy(szStreamTypeEnglish, "AC-3 Audio");
		break;
	case 0x83:
		lstrcpy(szStreamTypeEnglish, "LPCM Audio");
		break;
	case 0x85:
		lstrcpy(szStreamTypeEnglish, "DTS Audio");
		break;
	case 0x87:
		lstrcpy(szStreamTypeEnglish, "Enhanced AC-3 Audio");
		break;
	case 0xe6:
		lstrcpy(szStreamTypeEnglish, "WM9 Audio");
		break;
	case 0xea:
		lstrcpy(szStreamTypeEnglish, "VC1 Video");
		break;
	default:
		if (v->nNetworkPID != 0x0010)
		{
			if (nStreamType == 0x80)
			{
				lstrcpy(szStreamTypeEnglish, "DC-II Video");
				break;
			}
		}
		if ( (nStreamType >= 0x0F) && (nStreamType <= 0x7F) )
			lstrcpy(szStreamTypeEnglish, "ISO/IEC 13818-1 Reserved");
		else
			lstrcpy(szStreamTypeEnglish, "User Private");
		break;
	}
}

void DecodeServiceType(char * szServiceType, int nServiceType)
{
	switch(nServiceType)
	{
	case 0x00:
		lstrcpy(szServiceType, "reserved for future use");
		break;
	case 0x01:
		lstrcpy(szServiceType, "digital television service");
		break;
	case 0x02:
		lstrcpy(szServiceType, "digital radio sound service");
		break;
	case 0x03:
		lstrcpy(szServiceType, "Teletext service");
		break;
	case 0x04:
		lstrcpy(szServiceType, "NVOD reference service");
		break;
	case 0x05:
		lstrcpy(szServiceType, "NVOD time-shifted service");
		break;
	case 0x06:
		lstrcpy(szServiceType, "mosaic service");
		break;
	case 0x07:
		lstrcpy(szServiceType, "PAL coded signal");
		break;
	case 0x08:
		lstrcpy(szServiceType, "SECAM coded signal");
		break;
	case 0x09:
		lstrcpy(szServiceType, "D/D2-MAC");
		break;
	case 0x0A:
		lstrcpy(szServiceType, "FM Radio");
		break;
	case 0x0B:
		lstrcpy(szServiceType, "NTSC coded signal");
		break;
	case 0x0C:
		lstrcpy(szServiceType, "data broadcast service");
		break;
	case 0x0D:
		lstrcpy(szServiceType, "reserved for Common Interface Usage");
		break;
	case 0x0E:
		lstrcpy(szServiceType, "RCS Map (see EN 301 790 [24] )");
		break;
	case 0x0F:
		lstrcpy(szServiceType, "RCS FLS (see EN 301 790 [24] )");
		break;
	case 0x10:
		lstrcpy(szServiceType, "DVB MHP service");
		break;
	case 0x11:
		lstrcpy(szServiceType, "MPEG-2 HD digital television service");
		break;
	case 0x16:
		lstrcpy(szServiceType, "advanced codec SD digital television");
		break;
	case 0x17:
		lstrcpy(szServiceType, "advanced codec SD NVOD time-shifted");
		break;
	case 0x18:
		lstrcpy(szServiceType, "advanced codec SD NVOD reference");
		break;
	case 0x19:
		lstrcpy(szServiceType, "advanced codec HD digital television");
		break;
	case 0x1A:
		lstrcpy(szServiceType, "advanced codec HD NVOD time-shifted");
		break;
	case 0x1B:
		lstrcpy(szServiceType, "advanced codec HD NVOD reference");
		break;
	case 0xC0:
		lstrcpy(szServiceType, "data service");
		break;
	default:
		if (nServiceType >= 0x80 && nServiceType <= 0xfe)
			wsprintf(szServiceType, "user defined (0x%02x)", nServiceType);
		else
			wsprintf(szServiceType, "reserved for future use (0x%02x)", nServiceType);
		break;
	}
}

char * DecodePrivateDataSpecifierDescriptor(BYTE * pDescriptor)
{
	int descriptor_tag;
	int descriptor_length;

	set_buf(BM_USER_THREAD, pDescriptor, 0, FALSE);
	descriptor_tag = get_bits(BM_USER_THREAD, 8);
	descriptor_length = get_bits(BM_USER_THREAD, 8);
	if (descriptor_length)
	{
		DWORD private_data_specifier = get_bits(BM_USER_THREAD, 32);

		switch(private_data_specifier)
		{
		case 0x00000000:
			return "Reserved";
		case 0x00000001:
			return "SES";
		case 0x00000002:
			return "BskyB 1";
		case 0x00000003:
			return "BskyB 2";
		case 0x00000004:
			return "BskyB 3";
		case 0x00000005:
			return "ARD, ZDF, ORF";
		case 0x00000006:
			return "Nokia Multimedia Network Terminals";
		case 0x00000007:
			return "AT Entertainment Ltd.";
		case 0x00000010:
			return "La Télévision Par Satellite (TPS)";
		case 0x00000011:
			return "Echostar Communications";
		case 0x00000012:
			return "Telia AB";
		case 0x00000015:
			return "MediaKabel";
		case 0x00000020:
			return "Lyonnaise Cable 1";
		case 0x00000021:
			return "Lyonnaise Cable 2";
		case 0x00000022:
			return "Lyonnaise Cable 3";
		case 0x00000023:
			return "Lyonnaise Cable 4";
		case 0x00000025:
			return "MTV Europe";
		case 0x00000026:
			return "Pansonic";
		case 0x00000030:
			return "Telenor";
		case 0x00000031:
			return "TeleDenmark";
		case 0x000000BE:
			return "BetaTechnik";
		case 0x000000C0:
			return "Canal+";
		case 0x000000D0:
			return "Dolby Laboratories Inc.";
		case 0x000000E0:
			return "ExpressVu Inc.";
		case 0x000000F0:
			return "France Telecom, CNES and DGA (STENTOR)";
		case 0x00000100:
			return "OpenTV";
		case 0x00000150:
			return "Loewe Opta GmbH";
		case 0x00001000:
			return "La Télévision Par Satellite (TPS)";
		case 0x000022D4:
			return "Spanish Broadcasting Regulator";
		case 0x000022F1:
			return "Swedish Broadcasting Regulator";
		case 0x0000233A:
			return "Independent Television Commission";
		case 0x00006000:
			return "News Datacom";
		case 0x00006001:
			return "NDC 1";
		case 0x00006002:
			return "NDC 2";
		case 0x00006003:
			return "NDC 3";
		case 0x00006004:
			return "NDC 4";
		case 0x00006005:
			return "NDC 5";
		case 0x00006006:
			return "NDC 6";
		case 0x00362275:
			return "Irdeto";
		case 0x004E544C:
			return "NTL";
		case 0x00532D41:
			return "Scientific Atlanta";
		case 0x5347444E:
			return "StarGuide Digital Networks";
		case 0x00600000:
			return "Rhône Vision Cable";
		case 0x44414E59:
			return "News Datacom (IL) 1";
		case 0x46524549:
			return "News Datacom (IL) 1";
		case 0x53415053:
			return "Scientific Atlanta";
		case 0xFCFCFCFC:
			return "France Telecom";
		default:
			if (private_data_specifier >= 0x46545600 && private_data_specifier <= 0x46545620)
				return "Free TV";
			else if (private_data_specifier >= 0x4F545600 && private_data_specifier <= 0x4F5456FF)
				return "OpenTV";
			else if (private_data_specifier >= 0x50484900 && private_data_specifier <= 0x504849FF)
				return "Philips DVS";
		}
	}

	return "Unknown";
}


void FormatDVBCModulation(char * szModulation, int nModulation)
{
	switch(nModulation)
	{
	case 0x00:
		lstrcpy(szModulation, "not defined");
		break;
	case 0x01:
		lstrcpy(szModulation, "16 QAM");
		break;
	case 0x02:
		lstrcpy(szModulation, "32 QAM");
		break;
	case 0x03:
		lstrcpy(szModulation, "64 QAM");
		break;
	case 0x04:
		lstrcpy(szModulation, "128 QAM");
		break;
	case 0x05:
		lstrcpy(szModulation, "256 QAM");
		break;
	default:
		lstrcpy(szModulation, "reserved for future use");
		break;
	}
}

void FormatDVBCOuterFEC(char * szFECOuter, int nFECOuter)
{
	switch(nFECOuter)
	{
	case 0:
		lstrcpy(szFECOuter, "not defined");
		break;
	case 1:
		lstrcpy(szFECOuter, "no outer FEC coding");
		break;
	case 2:
		lstrcpy(szFECOuter, "Reed-Solomon (204/188)");
		break;
	default:
		lstrcpy(szFECOuter, "reserved for future use");
		break;
	}
}

void FormatDVBCInnerFEC(char * szFECInner, int nFECInner)
{
	switch(nFECInner)
	{
	case 0:
		lstrcpy(szFECInner, "not defined");
		break;
	case 1:
		lstrcpy(szFECInner, "1/2 conv. code rate");
		break;
	case 2:
		lstrcpy(szFECInner, "2/3 conv. code rate");
		break;
	case 3:
		lstrcpy(szFECInner, "3/4 conv. code rate");
		break;
	case 4:
		lstrcpy(szFECInner, "5/6 conv. code rate");
		break;
	case 5:
		lstrcpy(szFECInner, "7/8 conv. code rate");
		break;
	case 15:
		lstrcpy(szFECInner, "No conv. coding");
		break;
	default:
		lstrcpy(szFECInner, "reserved for future use");
		break;
	}
}

void FormatDVBTBandwidth(char * szBandwidth, int nBandwidth)
{
	switch(nBandwidth)
	{
	case 0:
		lstrcpy(szBandwidth, "8 MHz");
		break;
	case 1:
		lstrcpy(szBandwidth, "7 MHz");
		break;
	case 2:
		lstrcpy(szBandwidth, "6 MHz");
		break;
	case 3:
		lstrcpy(szBandwidth, "5 MHz");
		break;
	default:
		szBandwidth[0] = 0;
		break;
	}
}

void FormatDVBTConstellation(char * szConstellation, int nConstellation)
{
	switch(nConstellation)
	{
	case 0:
		lstrcpy(szConstellation, "QPSK");
		break;
	case 1:
		lstrcpy(szConstellation, "16-QAM");
		break;
	case 2:
		lstrcpy(szConstellation, "64-QAM");
		break;
	default:
		lstrcpy(szConstellation, "Reserved");
		break;
	}
}

void FormatDVBTHierarchyInformation(char * szHierarchyInformation, int nHierarchyInformation)
{
	switch(nHierarchyInformation)
	{
	case 0:
		lstrcpy(szHierarchyInformation, "non-hierarchical, native interleaver");
		break;
	case 1:
		lstrcpy(szHierarchyInformation, "a = 1, native interleaver");
		break;
	case 2:
		lstrcpy(szHierarchyInformation, "a = 2, native interleaver");
		break;
	case 3:
		lstrcpy(szHierarchyInformation, "a = 4, native interleaver");
		break;
	case 4:
		lstrcpy(szHierarchyInformation, "non-hierarchical, in-depth interleaver");
		break;
	case 5:
		lstrcpy(szHierarchyInformation, "a = 1, in-depth interleaver");
		break;
	case 6:
		lstrcpy(szHierarchyInformation, "a = 2, in-depth interleaver");
		break;
	case 7:
		lstrcpy(szHierarchyInformation, "a = 4, in-depth interleaver");
		break;
	default:
		lstrcpy(szHierarchyInformation, "Reserved");
		break;
	}
}

void DecodeDescriptorNames(char * szDescriptor, BYTE nDescriptorID)
{
	{
		int i;

		// User decoding in Pro
		for (i = 0; i < 5; i++)
		{
			if (DescriptorDecode[i] != NULL)
			{
				BOOL fRetVal = DescriptorDecode[i](TRUE, v->nNetworkPID, &nDescriptorID, szDescriptor);
				if (fRetVal)
					return;
			}
		}
	}

	switch(nDescriptorID)
	{
	case 0:
	case 1:
		lstrcpy(szDescriptor, "Reserved");
		return;
	case 2:
		lstrcpy(szDescriptor, "Video Stream");
		break;
	case 3:
		lstrcpy(szDescriptor, "Audio Stream");
		break;
	case 4:
		lstrcpy(szDescriptor, "Hierarchy");
		break;
	case 5:
		lstrcpy(szDescriptor, "Registration");
		break;
	case 6:
		lstrcpy(szDescriptor, "Data Stream Alignment");
		break;
	case 7:
		lstrcpy(szDescriptor, "Target Background Grid");
		break;
	case 8:
		lstrcpy(szDescriptor, "Video Window");
		break;
	case 9:
		lstrcpy(szDescriptor, "CA");
		break;
	case 10:
		lstrcpy(szDescriptor, "ISO639 Language");
		break;
	case 11:
		lstrcpy(szDescriptor, "System Clock");
		break;
	case 12:
		lstrcpy(szDescriptor, "Multiplex Buffer Utilization");
		break;
	case 13:
		lstrcpy(szDescriptor, "Copyright");
		break;
	case 14:
		lstrcpy(szDescriptor, "Maximum Bitrate");
		break;
	case 15:
		lstrcpy(szDescriptor, "Private Data Indicator");
		break;
	case 16:
		lstrcpy(szDescriptor, "Smoothing Buffer");
		break;
	case 17:
		lstrcpy(szDescriptor, "STD");
		break;
	case 18:
		lstrcpy(szDescriptor, "IBP");
		break;
	case 19:
	case 20:
	case 21:
	case 22:
	case 23:
	case 24:
	case 25:
	case 26:
		lstrcpy(szDescriptor, "Defined in ISO/IEC 13818-6");
		break;
	case 27:
		lstrcpy(szDescriptor, "MPEG-4 video");
		break;
	case 28:
		lstrcpy(szDescriptor, "MPEG-4 audio");
		break;
	case 29:
		lstrcpy(szDescriptor, "IOD");
		break;
	case 30:
		lstrcpy(szDescriptor, "SL");
		break;
	case 31:
		lstrcpy(szDescriptor, "FMC");
		break;
	case 32:
		lstrcpy(szDescriptor, "External ES ID");
		break;
	case 33:
		lstrcpy(szDescriptor, "MuxCode");
		break;
	case 34:
		lstrcpy(szDescriptor, "FmxBufferSize");
		break;
	case 35:
		lstrcpy(szDescriptor, "MultiplexBuffer");
		break;
	case 36:
		lstrcpy(szDescriptor, "Content labeling");
		break;
	case 37:
		lstrcpy(szDescriptor, "Metadata pointer");
		break;
	case 38:
		lstrcpy(szDescriptor, "Metadata");
		break;
	case 39:
		lstrcpy(szDescriptor, "Metadata STD");
		break;
	case 40:
		lstrcpy(szDescriptor, "AVC video descriptor");
		break;
	case 41:
		lstrcpy(szDescriptor, "IPMP");
		break;
	case 42:
		lstrcpy(szDescriptor, "AVC timing and HRD");
		break;
	case 43:
		lstrcpy(szDescriptor, "MPEG-2 AAC audio");
		break;
	case 0x40:
		lstrcpy(szDescriptor, "Network Name");
		break;
	case 0x41:
		lstrcpy(szDescriptor, "Service List");
		break;
	case 0x42:
		lstrcpy(szDescriptor, "Stuffing");
		break;
	case 0x43:
		lstrcpy(szDescriptor, "Satellite Delivery System");
		break;
	case 0x44:
		lstrcpy(szDescriptor, "Cable Delivery System");
		break;
	case 0x45:
		lstrcpy(szDescriptor, "VBI Data");
		break;
	case 0x46:
		lstrcpy(szDescriptor, "VBI Teletext");
		break;
	case 0x47:
		lstrcpy(szDescriptor, "Bouquet Name");
		break;
	case 0x48:
		lstrcpy(szDescriptor, "Service");
		break;
	case 0x49:
		lstrcpy(szDescriptor, "Country Availability");
		break;
	case 0x4a:
		lstrcpy(szDescriptor, "Linkage");
		break;
	case 0x4b:
		lstrcpy(szDescriptor, "NVOD Reference");
		break;
	case 0x4c:
		lstrcpy(szDescriptor, "Time Shifted Service");
		break;
	case 0x4d:
		lstrcpy(szDescriptor, "Short Event");
		break;
	case 0x4e:
		lstrcpy(szDescriptor, "Extended Event");
		break;
	case 0x4f:
		lstrcpy(szDescriptor, "Time Shifted Event");
		break;
	case 0x50:
		lstrcpy(szDescriptor, "Component");
		break;
	case 0x51:
		lstrcpy(szDescriptor, "Mosaic");
		break;
	case 0x52:
		lstrcpy(szDescriptor, "Stream Identifier");
		break;
	case 0x53:
		lstrcpy(szDescriptor, "CA Identifier");
		break;
	case 0x54:
		lstrcpy(szDescriptor, "Content");
		break;
	case 0x55:
		lstrcpy(szDescriptor, "Parental Rating");
		break;
	case 0x56:
		lstrcpy(szDescriptor, "Teletext");
		break;
	case 0x57:
		lstrcpy(szDescriptor, "Telephone");
		break;
	case 0x58:
		lstrcpy(szDescriptor, "Local Time Offset");
		break;
	case 0x59:
		lstrcpy(szDescriptor, "Subtitling");
		break;
	case 0x5f:
		lstrcpy(szDescriptor, "Private Data Specifier");
		break;
	case 0x60:
		lstrcpy(szDescriptor, "Service Move");
		break;
	case 0x61:
		lstrcpy(szDescriptor, "Short Smoothing Buffer");
		break;
	case 0x62:
		lstrcpy(szDescriptor, "Frequency List");
		break;
	case 0x63:
		lstrcpy(szDescriptor, "Partial Transport Stream");
		break;
	case 0x64:
		lstrcpy(szDescriptor, "Data Broadcast");
		break;
	case 0x65:
		lstrcpy(szDescriptor, "CA System");
		break;
	case 0x66:
		lstrcpy(szDescriptor, "Data Broadcast ID");
		break;
	case 0x67:
		lstrcpy(szDescriptor, "Transport Stream");
		break;
	case 0x68:
		lstrcpy(szDescriptor, "DSNG");
		break;
	case 0x69:
		lstrcpy(szDescriptor, "PDC");
		break;
	case 0x6a:
		lstrcpy(szDescriptor, "AC3 Audio");
		break;
	case 0x6b:
		lstrcpy(szDescriptor, "Ancillary Data");
		break;
	case 0x6c:
		lstrcpy(szDescriptor, "Cell List");
		break;
	case 0x6d:
		lstrcpy(szDescriptor, "Cell Frequency Link");
		break;
	case 0x6e:
		lstrcpy(szDescriptor, "Announcement Support");
		break;
	case 0x6f:
		lstrcpy(szDescriptor, "Application Signalling");
		break;
	case 0x70:
		lstrcpy(szDescriptor, "Adaptation Field Data");
		break;
	case 0x71:
		lstrcpy(szDescriptor, "Service Identifier");
		break;
	case 0x72:
		lstrcpy(szDescriptor, "Service Availability");
		break;
	case 0x73:
		lstrcpy(szDescriptor, "Default Authority");
		break;
	case 0x74:
		lstrcpy(szDescriptor, "Related Content");
		break;
	case 0x75:
		lstrcpy(szDescriptor, "TVA ID");
		break;
	case 0x76:
		lstrcpy(szDescriptor, "Content Identifier");
		break;
	case 0x77:
		lstrcpy(szDescriptor, "Time Slice FEC Identifier");
		break;
	case 0x78:
		lstrcpy(szDescriptor, "ECM Repetition Rate");
		break;
	case 0x79:
		lstrcpy(szDescriptor, "S2 Satellite Delivery System");
		break;
	case 0x7a:
		lstrcpy(szDescriptor, "Enhanced AC-3");
		break;
	case 0x7b:
		lstrcpy(szDescriptor, "DTS");
		break;
	case 0x7c:
		lstrcpy(szDescriptor, "AAC");
		break;
	case 0x80:
		if (v->nNetworkPID == 0x1ffb)
			lstrcpy(szDescriptor, "ATSC Stuffing");
		else if (v->nNetworkPID == 0x0ffe)
			lstrcpy(szDescriptor, "DCII Stuffing");
		else
			goto DecodeDescriptorNames_ForceDefault;
		break;
	case 0x81:
		if (v->nNetworkPID == 0x1ffb)
			lstrcpy(szDescriptor, "ATSC AC-3 audio");
		else if (v->nNetworkPID == 0x0ffe)
			lstrcpy(szDescriptor, "DCII AC-3 audio");
		else
			goto DecodeDescriptorNames_ForceDefault;
		break;
	case 0x82:
		if (v->nNetworkPID == 0x0ffe)
			lstrcpy(szDescriptor, "DCII Frame Rate");
		else
			goto DecodeDescriptorNames_ForceDefault;
		break;
	case 0x83:
		if (v->nNetworkPID == 0x0ffe)
			lstrcpy(szDescriptor, "DCII Extended Video");
		else
			goto DecodeDescriptorNames_ForceDefault;
		break;
	case 0x84:
		if (v->nNetworkPID == 0x0ffe)
			lstrcpy(szDescriptor, "DCII Component Name");
		else
			goto DecodeDescriptorNames_ForceDefault;
		break;
	case 0x86:
		if (v->nNetworkPID != 0x1ffb)
			goto DecodeDescriptorNames_ForceDefault;
		lstrcpy(szDescriptor, "ATSC Caption Service");
		break;
	case 0x87:
		if (v->nNetworkPID != 0x1ffb)
			goto DecodeDescriptorNames_ForceDefault;
		lstrcpy(szDescriptor, "ATSC Content Advisory");
		break;
	case 0x90:
		if (v->nNetworkPID == 0x0ffe)
			lstrcpy(szDescriptor, "DCII Frequency Specification");
		else
			goto DecodeDescriptorNames_ForceDefault;
		break;
	case 0x91:
		if (v->nNetworkPID == 0x0ffe)
			lstrcpy(szDescriptor, "DCII Modulation Parameters");
		else
			goto DecodeDescriptorNames_ForceDefault;
		break;
	case 0x92:
		if (v->nNetworkPID == 0x0ffe)
			lstrcpy(szDescriptor, "DCII Transport Stream ID");
		else
			goto DecodeDescriptorNames_ForceDefault;
		break;
	case 0xa0:
		if (v->nNetworkPID != 0x1ffb)
			goto DecodeDescriptorNames_ForceDefault;
		lstrcpy(szDescriptor, "ATSC Extended Channel Name");
		break;
	case 0xa1:
		if (v->nNetworkPID != 0x1ffb)
			goto DecodeDescriptorNames_ForceDefault;
		lstrcpy(szDescriptor, "ATSC Service Location");
		break;
	case 0xa2:
		if (v->nNetworkPID != 0x1ffb)
			goto DecodeDescriptorNames_ForceDefault;
		lstrcpy(szDescriptor, "ATSC Time-shifted Service");
		break;
	case 0xa3:
		if (v->nNetworkPID != 0x1ffb)
			goto DecodeDescriptorNames_ForceDefault;
		lstrcpy(szDescriptor, "ATSC Component Name");
		break;
	case 0xa8:
		if (v->nNetworkPID != 0x1ffb)
			goto DecodeDescriptorNames_ForceDefault;
		lstrcpy(szDescriptor, "ATSC DCC Departing Request");
		break;
	case 0xa9:
		if (v->nNetworkPID != 0x1ffb)
			goto DecodeDescriptorNames_ForceDefault;
		lstrcpy(szDescriptor, "ATSC DCC Arriving Request");
		break;
	case 0xaa:
		if (v->nNetworkPID != 0x1ffb)
			goto DecodeDescriptorNames_ForceDefault;
		lstrcpy(szDescriptor, "ATSC Redistribution Control");
		break;
	case 0xc0:
		if (v->nNetworkPID == 0x0ffe)
			lstrcpy(szDescriptor, "DCII Banner Override");
		else
			goto DecodeDescriptorNames_ForceDefault;
		break;
	case 0xc1:
		if (v->fISDB)
			lstrcpy(szDescriptor, "ISDB Digital Copy Control");
		else
			goto DecodeDescriptorNames_ForceDefault;
		break;
	case 0xc4:
		if (v->fISDB)
			lstrcpy(szDescriptor, "ISDB Audio Component");
		else
			goto DecodeDescriptorNames_ForceDefault;
		break;
	case 0xc7:
		if (v->fISDB)
			lstrcpy(szDescriptor, "ISDB Data Content");
		else
			goto DecodeDescriptorNames_ForceDefault;
		break;
	case 0xc8:
		if (v->fISDB)
			lstrcpy(szDescriptor, "ISDB Video Decode Control");
		else
			goto DecodeDescriptorNames_ForceDefault;
		break;
	case 0xcd:
		if (v->fISDB)
			lstrcpy(szDescriptor, "ISDB TS Information");
		else
			goto DecodeDescriptorNames_ForceDefault;
		break;
	case 0xce:
		if (v->fISDB)
			lstrcpy(szDescriptor, "ISDB Extended Broadcaster");
		else
			goto DecodeDescriptorNames_ForceDefault;
		break;
	case 0xcf:
		if (v->fISDB)
			lstrcpy(szDescriptor, "ISDB Logo Transmission");
		else
			goto DecodeDescriptorNames_ForceDefault;
		break;
	case 0xd6:
		if (v->fISDB)
			lstrcpy(szDescriptor, "ISDB Event Group");
		else
			goto DecodeDescriptorNames_ForceDefault;
		break;
	case 0xd7:
		if (v->fISDB)
			lstrcpy(szDescriptor, "ISDB SI Parameter");
		else
			goto DecodeDescriptorNames_ForceDefault;
		break;
	case 0xde:
		if (v->fISDB)
			lstrcpy(szDescriptor, "ISDB Content Availability");
		else
			goto DecodeDescriptorNames_ForceDefault;
		break;
	case 0xf6:
		if (v->fISDB)
			lstrcpy(szDescriptor, "ISDB Access Control");
		else
			goto DecodeDescriptorNames_ForceDefault;
		break;
	case 0xfa:
		if (v->fISDB)
			lstrcpy(szDescriptor, "ISDB Terrestrial Delivery System");
		else
			goto DecodeDescriptorNames_ForceDefault;
		break;
	case 0xfb:
		if (v->fISDB)
			lstrcpy(szDescriptor, "ISDB Partial Reception");
		else
			goto DecodeDescriptorNames_ForceDefault;
		break;
	case 0xfd:
		if (v->fISDB)
			lstrcpy(szDescriptor, "ISDB Data Component");
		else
			goto DecodeDescriptorNames_ForceDefault;
		break;
	case 0xfe:
		if (v->fISDB)
			lstrcpy(szDescriptor, "ISDB System Management");
		else
			goto DecodeDescriptorNames_ForceDefault;
		break;
	default:
DecodeDescriptorNames_ForceDefault:
		if (nDescriptorID < 64)
			wsprintf(szDescriptor, "ISO/IEC 13818-1 Reserved Descriptor: 0x%02x", nDescriptorID);
		else
			wsprintf(szDescriptor, "User Private Descriptor: 0x%02x", nDescriptorID);
		return;
	}
	lstrcat(szDescriptor, " Descriptor");
}

void FormatCASystemName(int nCASystemID, char * szCAName)
{
	if (nCASystemID == 0x1234)						lstrcat(szCAName, "Echostar/Nagravision");
	else if (nCASystemID == 0x0005)					lstrcat(szCAName, "ARIB CAS");
	else if ((nCASystemID & 0xff00) == 0x0000)		lstrcat(szCAName, "Standardized");
	else if ((nCASystemID & 0xff00) == 0x0100)		lstrcat(szCAName, "Canal Plus");
	else if ((nCASystemID & 0xff00) == 0x0200)		lstrcat(szCAName, "CCETT");
	else if ((nCASystemID & 0xff00) == 0x0300)		lstrcat(szCAName, "Deutsche Telecom");
	else if ((nCASystemID & 0xff00) == 0x0400)		lstrcat(szCAName, "Eurodec");
	else if ((nCASystemID & 0xff00) == 0x0500)		lstrcat(szCAName, "France Telecom");
	else if ((nCASystemID & 0xff00) == 0x0600)		lstrcat(szCAName, "Irdeto");
	else if ((nCASystemID & 0xff00) == 0x0700)		lstrcat(szCAName, "Jerrold/GI/Motorola");
	else if ((nCASystemID & 0xff00) == 0x0800)		lstrcat(szCAName, "Matra Communication");
	else if ((nCASystemID & 0xff00) == 0x0900)		lstrcat(szCAName, "News Datacom");
	else if ((nCASystemID & 0xff00) == 0x0a00)		lstrcat(szCAName, "Nokia");
	else if ((nCASystemID & 0xff00) == 0x0b00)		lstrcat(szCAName, "Conax");
	else if ((nCASystemID & 0xff00) == 0x0c00)		lstrcat(szCAName, "NTL (RAS)");
	else if ((nCASystemID & 0xff00) == 0x0d00)		lstrcat(szCAName, "Philips");
	else if ((nCASystemID & 0xff00) == 0x0e00)		lstrcat(szCAName, "Scientfic Atlanta");
	else if ((nCASystemID & 0xff00) == 0x0f00)		lstrcat(szCAName, "Sony");
	else if ((nCASystemID & 0xff00) == 0x1000)		lstrcat(szCAName, "Tandberg Television");
	else if ((nCASystemID & 0xff00) == 0x1100)		lstrcat(szCAName, "Thomson");
	else if ((nCASystemID & 0xff00) == 0x1200)		lstrcat(szCAName, "TV/Com");
	else if ((nCASystemID & 0xff00) == 0x1300)		lstrcat(szCAName, "HPT - Croatian Post and Telecommunications");
	else if ((nCASystemID & 0xff00) == 0x1400)		lstrcat(szCAName, "HRT - Croatian Radio and Television");
	else if ((nCASystemID & 0xff00) == 0x1500)		lstrcat(szCAName, "IBM");
	else if ((nCASystemID & 0xff00) == 0x1600)		lstrcat(szCAName, "Nera");	
	else if ((nCASystemID & 0xff00) == 0x1700)		lstrcat(szCAName, "BetaTechnik");
	else if ((nCASystemID & 0xff00) == 0x1800)		lstrcpy(szCAName, "Nagravision");
	else if ((nCASystemID & 0xff00) == 0x1900)		lstrcpy(szCAName, "Titan Information Systems");
	else if ((nCASystemID & 0xff00) == 0x2000)		lstrcpy(szCAName, "Telefónica Servicios Audiovisuales");
	else if ((nCASystemID & 0xff00) == 0x2100)		lstrcpy(szCAName, "STENTOR (France Telecom, CNES and DGA)");
	else if ((nCASystemID & 0xff00) == 0x2200)		lstrcpy(szCAName, "Scopus Network Techno");
	else if ((nCASystemID & 0xff00) == 0x2300)		lstrcpy(szCAName, "BARCO AS");
	else if ((nCASystemID & 0xff00) == 0x2400)		lstrcpy(szCAName, "StarGuide Digital Networks");
	else if ((nCASystemID & 0xff00) == 0x2500)		lstrcpy(szCAName, "Mentor Data System, Inc.");
	else if ((nCASystemID & 0xff00) == 0x2600)		lstrcat(szCAName, "EBU (BISS-E)");
	else if ((nCASystemID & 0xff00) == 0x4700)		lstrcat(szCAName, "General Instrument");
	else if ((nCASystemID & 0xff00) == 0x4800)		lstrcat(szCAName, "Telemann");
	else if ((nCASystemID & 0xff00) == 0x4900)		lstrcat(szCAName, "Digital TV Industry Alliance of China");
	else if ((nCASystemID & 0xfff0) == 0x4a00)		lstrcat(szCAName, "Tsinghua TongFang");
	else if ((nCASystemID & 0xfff0) == 0x4a10)		lstrcat(szCAName, "Easycas");
	else if ((nCASystemID & 0xfff0) == 0x4a20)		lstrcat(szCAName, "AlphaCrypt");
	else if ((nCASystemID & 0xfff0) == 0x4a30)		lstrcat(szCAName, "DVN Holdings");
	else if ((nCASystemID & 0xfff0) == 0x4a40)		lstrcat(szCAName, "Shanghai Advanced Digital Technology Co. Ltd. (ADT)");
	else if ((nCASystemID & 0xfff0) == 0x4a50)		lstrcat(szCAName, "Shenzhen Kingsky Company (China) Ltd.");
	else if ((nCASystemID & 0xfff0) == 0x4a60)		lstrcat(szCAName, "@Sky");
	else if ((nCASystemID & 0xfff0) == 0x4a70)		lstrcat(szCAName, "Dreamcrypt");
	else if ((nCASystemID & 0xfff0) == 0x4a80)		lstrcat(szCAName, "THALESCrypt");
	else if ((nCASystemID & 0xfff0) == 0x4a90)		lstrcat(szCAName, "Runcom Technologies");
	else if ((nCASystemID & 0xfff0) == 0x4aa0)		lstrcat(szCAName, "SIDSA");
	else if ((nCASystemID & 0xfff0) == 0x4ab0)		lstrcat(szCAName, "Beijing Compunicate Technology Inc.");
	else if ((nCASystemID & 0xfff0) == 0x4ac0)		lstrcat(szCAName, "Latens Systems Ltd");
	else if (nCASystemID == 0x4ad0 || nCASystemID == 0x4ad1)	lstrcat(szCAName, "XCrypt Inc.");
	else if (nCASystemID == 0x4ad2 || nCASystemID == 0x4ad3)	lstrcat(szCAName, "Beijing Digital Video Technology Co., Ltd.");
	else if (nCASystemID == 0x4ad4 || nCASystemID == 0x4ad5)	lstrcat(szCAName, "Widevine Technologies, Inc.");
	else if (nCASystemID == 0x4ad6 || nCASystemID == 0x4ad7)	lstrcat(szCAName, "SK Telecom Co., Ltd.");
	else if (nCASystemID == 0x4ad8 || nCASystemID == 0x4ad9)	lstrcat(szCAName, "Enigma Systems");
	else if (nCASystemID == 0x4ae0 || nCASystemID == 0x4ae1)	lstrcat(szCAName, "Digi Raum Electronics Co. Ltd.");
	else if (nCASystemID == 0x7f39)					lstrcat(szCAName, "PrediWave");
}

void DecodeDTSAudioDescriptor(BYTE * pDescriptorData, char * szBuffer)
{
	set_buf(BM_USER_THREAD, pDescriptorData, 0, FALSE);
	{
		int descriptor_tag = get_bits(BM_USER_THREAD, 8);
		int descriptor_length = get_bits(BM_USER_THREAD, 8);
		int sample_rate_code = get_bits(BM_USER_THREAD, 4);
		int bit_rate_code = get_bits(BM_USER_THREAD, 6);
		int nblks = get_bits(BM_USER_THREAD, 7);
		int fsize = get_bits(BM_USER_THREAD, 14);
		int surround_mode = get_bits(BM_USER_THREAD, 6);
		int lfe_flag = get_bits(BM_USER_THREAD, 1);
		int extended_surround_flag = get_bits(BM_USER_THREAD, 2);

		double dSampleRate = DTSSamplingRates[sample_rate_code];
		double dBitRate = DTSBitRates[bit_rate_code & 0x1f];

		char szSampleRate[32];
		char szBitrate[32];
		char szSurroundMode[32];
		char szLFEChannel[8];
		char szExtendedSurroundMode[32];

		if (dSampleRate == 0.0)
			wsprintf(szSampleRate, "Invalid (0x%x)", sample_rate_code);
		else
			sprintf(szSampleRate, "%.2f KHz", dSampleRate);

		if (dBitRate == 0.0)
		{
			switch(bit_rate_code & 0x1f)
			{
			case 29:
				lstrcpy(szBitrate, "open");
				break;
			case 30:
				lstrcpy(szBitrate, "variable");
				break;
			case 31:
				lstrcpy(szBitrate, "lossless");
				break;
			default:
				wsprintf(szBitrate, "Invalid (0x%x)", bit_rate_code & 0x1f);
				break;
			}
		}
		else
			sprintf(szBitrate, "%.1f Kbps", dBitRate);

		switch(surround_mode)
		{
		case 0:
			lstrcpy(szSurroundMode, "1 / mono");
			break;
		case 1:
			lstrcpy(szSurroundMode, "unknown");
			break;
		case 2:
			lstrcpy(szSurroundMode, "2 / L + R (stereo)");
			break;
		case 3:
			lstrcpy(szSurroundMode, "2 / (L+R) + (L-R) (sum-difference)");
			break;
		case 4:
			lstrcpy(szSurroundMode, "2 / LT + RT (left and right total)");
			break;
		case 5:
			lstrcpy(szSurroundMode, "3 / C + L + R");
			break;
		case 6:
			lstrcpy(szSurroundMode, "3 / L + R + S");
			break;
		case 7:
			lstrcpy(szSurroundMode, "4 / C + L + R + S");
			break;
		case 8:
			lstrcpy(szSurroundMode, "4 / L + R + SL + SR");
			break;
		case 9:
			lstrcpy(szSurroundMode, "5 / C + L + R + SL + SR");
			break;
		default:
			wsprintf(szSurroundMode, "User Defined (0x%x)", surround_mode);
			break;
		}

		if (lfe_flag)
			lstrcpy(szLFEChannel, "On");
		else
			lstrcpy(szLFEChannel, "Off");

		switch(extended_surround_flag)
		{
		case 0:
			lstrcpy(szExtendedSurroundMode, "None");
			break;
		case 1:
			lstrcpy(szExtendedSurroundMode, "Maxtrixed");
			break;
		case 2:
			lstrcpy(szExtendedSurroundMode, "Discrete");
			break;
		case 3:
			lstrcpy(szExtendedSurroundMode, "Undefined");
			break;
		}

		wsprintf(szBuffer, " Sample Rate: %s Bitrate: %s\r\n Surround Mode: %s LFE: %s\r\n Extended Surround Mode: %s",
			szSampleRate, szBitrate,
			szSurroundMode, szLFEChannel,
			szExtendedSurroundMode);
	}
}

void DecodeVBIDataServiceDescription(int data_service_id, char * szBuffer)
{
	switch(data_service_id)
	{
	default:
		wsprintf(szBuffer, "Reserved for future use (0x%02x)", data_service_id);
		break;
	case 0x01:
		lstrcpy(szBuffer, "EBU teletext");
		break;
	case 0x02:
		lstrcpy(szBuffer, "Inverted teletext");
		break;
	case 0x03:
		lstrcpy(szBuffer, "Reserved");
		break;
	case 0x04:
		lstrcpy(szBuffer, "VPS");
		break;
	case 0x05:
		lstrcpy(szBuffer, "WSS");
		break;
	case 0x06:
		lstrcpy(szBuffer, "Closed Captioning");
		break;
	case 0x07:
		lstrcpy(szBuffer, "Monochrome 4:2:2 samples");
		break;
	}
}

BOOL QuickFormatNIT(char * szBuffer, int nTransportStreamID, BOOL fLongVersion)
{
	int nNITIndex;

	for (nNITIndex = 0; nNITIndex < MAX_TS_ENTRIES; nNITIndex++)
	{
		if (v->pNITData[nNITIndex] != NULL)
		{
			if (v->pNITData[nNITIndex]->nTransportStreamID == nTransportStreamID)
			{
				char szFormatBuffer[100];

				switch(v->pNITData[nNITIndex]->nType)
				{
				case NIT_DVBT:
				case NIT_ISDBT:
					{
						if (fLongVersion)
							lstrcpy(szFormatBuffer, "NIT: %.1f MHz\r\n");
						else
							lstrcpy(szFormatBuffer, "%.1f MHz");
						sprintf(szBuffer, szFormatBuffer, (double)v->pNITData[nNITIndex]->nFrequency / 100000.0);
					}
					break;
				case NIT_DVBC:
					{
						if (fLongVersion)
							lstrcpy(szFormatBuffer, "NIT: %.1f MHz\r\n");
						else
							lstrcpy(szFormatBuffer, "%.1f MHz");
						sprintf(szBuffer, szFormatBuffer, (double)v->pNITData[nNITIndex]->nFrequency / 10000.0);
					}
					break;
				case NIT_DVBS:
					{
						char szFEC[16];
						char szEW[16];
						char szPolarity[24];
						char szModulation[64] = {0};

						FormatDVBSCodeRate(szFEC, v->pNITData[nNITIndex]->dvbs.nFEC);
						FormatPolarity(szPolarity, v->pNITData[nNITIndex]->dvbs.nPolarization, fLongVersion);
						if (v->pNITData[nNITIndex]->dvbs.fEastern == TRUE)
							lstrcpy(szEW, "E");
						else
							lstrcpy(szEW, "W");
						if (v->pNITData[nNITIndex] != NULL)
							FormatDVBSModulation(szModulation, v->pNITData[nNITIndex]->dvbs.nModulation);
						
						if (fLongVersion)
							lstrcpy(szFormatBuffer, "NIT: %3.1f%s %.3f GHz %s %5d %s %s\r\n");
						else
							lstrcpy(szFormatBuffer, "%3.1f%s %.3f %s %d %s %s");

						sprintf(szBuffer, szFormatBuffer,
								(double)v->pNITData[nNITIndex]->dvbs.nOrbitalPosition / 10.0,
								szEW,
								(double)v->pNITData[nNITIndex]->nFrequency / 100000.0,
								szPolarity,
								v->pNITData[nNITIndex]->dvbs.nSymbolRate / 10,
								szFEC,
								szModulation);
					}
					break;
				case NIT_ISDBS:
					/* TODO */
					break;
				}
				return TRUE;
			}
		}
	}

	return FALSE;
}

static void DecodeCADescriptor(char *szBuffer, BYTE *pDescriptor)
{
	set_buf(BM_USER_THREAD, pDescriptor, 0, FALSE);
	{
		int i;
		int descriptor_tag = get_bits(BM_USER_THREAD, 8);
		int descriptor_length = get_bits(BM_USER_THREAD, 8);
		char szTemp[256];

		szBuffer[0] = '\0';
		for (i = 0; i < descriptor_length / 2; i++)
		{
			char szCAName[64] = {0};

			int CA_system_id = get_bits(BM_USER_THREAD, 16);
			FormatCASystemName(CA_system_id, szCAName);
			wsprintf(szTemp, "CA Identifier: %s\r\n", szCAName);
			lstrcat(szBuffer, szTemp);
		}
	}
}

static void DecodeSIDescriptor(char *szBuffer, BYTE *pDescriptor)
{
	char szTemp[256];

	set_buf(BM_USER_THREAD, pDescriptor, 0, FALSE);
	uint8_t component_tag = get_bits(BM_USER_THREAD, 8);

	wsprintf(szTemp, " Component tag: 0x%02x (%d)\r\n", component_tag, component_tag);
	lstrcat(szBuffer, szTemp);
}

char * DecodeComponentAndStreamContent(int stream_content, int component_type)
{
	switch(stream_content)
	{
	case 0x01:
		switch(component_type)
		{
		case 0x01:
			return "video, 4:3 aspect ratio, 25 Hz";
		case 0x02:
			return "video, 16:9 aspect ratio with pan vectors, 25 Hz";
		case 0x03:
			return "video, 16:9 aspect ratio without pan vectors, 25 Hz";
		case 0x04:
			return "video, > 16:9 aspect ratio, 25 Hz";
		case 0x05:
			return "video, 4:3 aspect ratio, 30 Hz";
		case 0x06:
			return "video, 16:9 aspect ratio with pan vectors, 30 Hz";
		case 0x07:
			return "video, 16:9 aspect ratio without pan vectors, 30 Hz";
		case 0x08:
			return "video, > 16:9 aspect ratio, 30 Hz";
		case 0x09:
			return "high definition video, 4:3 aspect ratio, 25 Hz";
		case 0x0A:
			return "high definition video, 16:9 aspect ratio with pan vectors, 25 Hz";
		case 0x0B:
			return "high definition video, 16:9 aspect ratio without pan vectors, 25 Hz";
		case 0x0C:
			return "high definition video, > 16:9 aspect ratio, 25 Hz";
		case 0x0D:
			return "high definition video, 4:3 aspect ratio, 30 Hz";
		case 0x0E:
			return "high definition video, 16:9 aspect ratio with pan vectors, 30 Hz";
		case 0x0F:
			return "high definition video, 16:9 aspect ratio without pan vec., 30 Hz";
		case 0x10:
			return "high definition video, > 16:9 aspect ratio, 30 Hz";
		default:
			if (component_type >= 0xB0 && component_type <= 0xFE)
				return "user defined";
			break;
		}
		break;
	case 0x02:
		switch(component_type)
		{			
		case 0x01:
			return "audio, single mono channel";
		case 0x02:
			return "audio, dual mono channel";
		case 0x03:
			return "audio, stereo (2 channel)";
		case 0x04:
			return "audio, multi-lingual, multi-channel";
		case 0x05:
			return "audio, surround sound";
		case 0x40:
			return "audio description for the visually impaired";
		case 0x41:
			return "audio for the hard of hearing";
		default:
			if (component_type >= 0xB0 && component_type <= 0xFE)
				return "user defined";
			break;
		}
		break;
	case 0x03:
		switch(component_type)
		{
		case 0x01:
			return "EBU Teletext subtitles";
		case 0x02:
			return "associated EBU Teletext";
		case 0x03:
			return "VBI data";
		case 0x10:
			return "DVB subtitles (normal) with no monitor aspect ratio criticality";
		case 0x11:
			return "DVB subtitles (normal) for display on 4:3 aspect ratio monitor";
		case 0x12:
			return "DVB subtitles (normal) for display on 16:9 aspect ratio monitor";
		case 0x13:
			return "DVB subtitles (normal) for display on 2.21:1 aspect ratio monitor";
		case 0x20:
			return "DVBsubtitles (for the hard of hearing)with nomonitor aspect ratio criticality";
		case 0x21:
			return "DVBsubtitles (for the hard of hearing) for display on 4:3 aspect ratiomonitor";
		case 0x22:
			return "DVBsubtitles (for the hard of hearing) for display on 16:9 aspect ratiomonitor";
		case 0x23:
			return "DVBsubtitles (for the hard of hearing) for display on 2.21:1 aspect ratiomonitor";
		default:
			if (component_type >= 0xB0 && component_type <= 0xFE)
				return "user defined";
			break;
		}
		break;
	case 0x04:
		if (component_type >= 0x00 && component_type <= 0x7f)
			return "reserved for AC-3 audio modes";
		break;
	case 0x05:
		switch(component_type)
		{
		case 0x01:
			return "H.264/AVC standard definition video, 4:3 aspect ratio, 25 Hz";
		case 0x03:
			return "H.264/AVC standard definition video, 16:9 aspect ratio, 25 Hz";
		case 0x04:
			return "H.264/AVC standard definition video, > 16:9 aspect ratio, 25 Hz";
		case 0x05:
			return "H.264/AVC standard definition video, 4:3 aspect ratio, 30 Hz";
		case 0x07:
			return "H.264/AVC standard definition video, 16:9 aspect ratio, 30 Hz";
		case 0x08:
			return "H.264/AVC standard definition video, > 16:9 aspect ratio, 30 Hz";
		case 0x0B:
			return "H.264/AVC high definition video, 16:9 aspect ratio, 25 Hz";
		case 0x0C:
			return "H.264/AVC high definition video, > 16:9 aspect ratio, 25 Hz";
		case 0x0F:
			return "H.264/AVC high definition video, 16:9 aspect ratio, 30 Hz";
		case 0x10:
			return "H.264/AVC high definition video, > 16:9 aspect ratio, 30 Hz";
		default:
			if (component_type >= 0xb0 && component_type <= 0xfe)
				return "user-defined";
			return "reserved for future use";
		}
		break;
	case 0x06:
		switch(component_type)
		{
		case 0x01:
			return "HE-AAC audio, single mono channel";
		case 0x03:
			return "HE-AAC audio, stereo";
		case 0x05:
			return "HE-AAC audio, surround sound";
		case 0x40:
			return "HE-AAC audio description for the visually impaired";
		case 0x41:
			return "HE-AAC audio for the hard of hearing";
		case 0x42:
			return "HE-AAC receiver-mixed supplementary audio";
		case 0x43:
			return "HE-AAC v2 audio, stereo";
		case 0x44:
			return "HE-AAC v2 audio description for the visually impaired";
		case 0x45:
			return "HE-AAC v2 audio for the hard of hearing";
		case 0x46:
			return "HE-AAC v2 receiver-mixed supplementary audio";
		default:
			if (component_type >= 0xb0 && component_type <= 0xfe)
				return "user-defined";
			return "reserved for future use";
		}
		break;
	case 0x07:
		if (component_type <= 0x7f)
			return "reserved for DTS audio modes";
		return "reserved for future use";
	case 0x0c:
	case 0x0d:
	case 0x0e:
	case 0x0f:
		return "user defined";
	}	

	return "reserved for future use";
}

void DecodeComponentDescriptor(char * szBuffer, BYTE * pDescriptor)
{
	set_buf(BM_USER_THREAD, pDescriptor, 0, FALSE);
	{
		int i;
		char text_char[256];

		int descriptor_tag = get_bits(BM_USER_THREAD, 8);
		int descriptor_length = get_bits(BM_USER_THREAD, 8);
		
		szBuffer[0] = '\0';
		if (descriptor_length)
		{
			int reserved_future_use = get_bits(BM_USER_THREAD, 4);
			int stream_content = get_bits(BM_USER_THREAD, 4);
			int component_type = get_bits(BM_USER_THREAD, 8);
			int component_tag = get_bits(BM_USER_THREAD, 8);
			int language_code = get_bits(BM_USER_THREAD, 24);
			for (i = 0; i < descriptor_length - 6; i++)
				text_char[i] = get_bits(BM_USER_THREAD, 8);
			text_char[i] = 0;
			wsprintf(szBuffer, "Component: %s %c%c%c (%s)\r\n", text_char, 
					 language_code >> 16, language_code >> 8, language_code & 0xff,
					 DecodeComponentAndStreamContent(stream_content, component_type));
		}
	}
}

char * DecodeContentNibbles(int content_nibble_level_1, int content_nibble_level_2)
{
	switch(content_nibble_level_1)
	{
	case 0x0:
		return "undefined content";
	case 0x1:
		switch(content_nibble_level_2)
		{
		case 0x0:
			return "movie/drama (general)";
		case 0x1:
			return "detective/thriller";
		case 0x2:
			return "adventure/western/war";
		case 0x3:
			return "science fiction/fantasy/horror";
		case 0x4:
			return "comedy";
		case 0x5:
			return "soap/melodrama/folkloric";
		case 0x6:
			return "romance";
		case 0x7:
			return "serious/classical/religious/historical movie/drama";
		case 0x8:
			return "adult movie/drama";
		case 0xF:
			return "user defined";
		}
		break;
	case 0x2:
		switch(content_nibble_level_2)
		{
		case 0x0:
			return "news/current affairs (general)";
		case 0x1:
			return "news/weather report";
		case 0x2:
			return "news magazine";
		case 0x3:
			return "documentary";
		case 0x4:
			return "discussion/interview/debate";
		case 0xF:
			return "user defined";
		}
		break;
	case 0x3:
		switch(content_nibble_level_2)
		{
		case 0x0:
			return "show/game show (general)";
		case 0x1:
			return "game show/quiz/contest";
		case 0x2:
			return "variety show";
		case 0x3:
			return "talk show";
		case 0xF:
			return "user defined";
		}
		break;
	case 0x4:
		switch(content_nibble_level_2)
		{
		case 0x0:
			return "sports (general)";
		case 0x1:
			return "special events (Olympic Games,World Cup etc.)";
		case 0x2:
			return "sports magazines";
		case 0x3:
			return "football/soccer";
		case 0x4:
			return "tennis/squash";
		case 0x5:
			return "team sports (excluding football)";
		case 0x6:
			return "athletics";
		case 0x7:
			return "motor sport";
		case 0x8:
			return "water sport";
		case 0x9:
			return "winter sports";
		case 0xA:
			return "equestrian";
		case 0xB:
			return "martial sports";
		case 0xF:
			return "user defined";
		}
		break;
	case 0x5:
		switch(content_nibble_level_2)
		{
		case 0x0:
			return "children's/youth programmes (general)";
		case 0x1:
			return "pre-school children's programmes";
		case 0x2:
			return "entertainment programmes for 6 to14";
		case 0x3:
			return "entertainment programmes for 10 to 16";
		case 0x4:
			return "informational/educational/school programmes";
		case 0x5:
			return "cartoons/puppets";
		case 0xF:
			return "user defined";
		}
		break;
	case 0x6:
		switch(content_nibble_level_2)
		{
		case 0x0:
			return "music/ballet/dance (general)";
		case 0x1:
			return "rock/pop";
		case 0x2:
			return "serious music/classical music";
		case 0x3:
			return "folk/traditional music";
		case 0x4:
			return "jazz";
		case 0x5:
			return "musical/opera";
		case 0x6:
			return "ballet";
		case 0xF:
			return "user defined";
		}
		break;
	case 0x7:
		switch(content_nibble_level_2)
		{
		case 0x0:
			return "arts/culture (without music, general)";
		case 0x1:
			return "performing arts";
		case 0x2:
			return "fine arts";
		case 0x3:
			return "religion";
		case 0x4:
			return "popular culture/traditional arts";
		case 0x5:
			return "literature";
		case 0x6:
			return "film/cinema";
		case 0x7:
			return "experimental film/video";
		case 0x8:
			return "broadcasting/press";
		case 0x9:
			return "new media";
		case 0xA:
			return "arts/culture magazines";
		case 0xB:
			return "fashion";
		case 0xF:
			return "user defined";
		}
		break;
	case 0x8:
		switch(content_nibble_level_2)
		{
		case 0x0:
			return "social/political issues/economics (general)";
		case 0x1:
			return "magazines/reports/documentary";
		case 0x2:
			return "economics/social advisory";
		case 0x3:
			return "remarkable people";
		case 0xF:
			return "user defined";
		}
		break;
	case 0x9:
		switch(content_nibble_level_2)
		{
		case 0x0:
			return "education/science/factual topics (general)";
		case 0x1:
			return "nature/animals/environment";
		case 0x2:
			return "technology/natural sciences";
		case 0x3:
			return "medicine/physiology/psychology";
		case 0x4:
			return "foreign countries/expeditions";
		case 0x5:
			return "social/spiritual sciences";
		case 0x6:
			return "further education";
		case 0x7:
			return "languages";
		case 0xF:
			return "user defined";
		}
		break;
	case 0xA:
		switch(content_nibble_level_2)
		{
		case 0x0:
			return "leisure hobbies (general)";
		case 0x1:
			return "tourism/travel";
		case 0x2:
			return "handicraft";
		case 0x3:
			return "motoring";
		case 0x4:
			return "fitness & health";
		case 0x5:
			return "cooking";
		case 0x6:
			return "advertisement/shopping";
		case 0x7:
			return "gardening";
		case 0xF:
			return "user defined";
		}
		break;
	case 0xB:
		switch(content_nibble_level_2)
		{
		case 0x0:
			return "original language";
		case 0x1:
			return "black & white";
		case 0x2:
			return "unpublished";
		case 0x3:
			return "live broadcast";
		case 0xF:
			return "user defined";
		}
		break;
	case 0xF:
		return "user defined";
	}

	return "reserved for future use";
}

void DecodeContentDescriptor(char * szBuffer, BYTE * pDescriptor)
{
	set_buf(BM_USER_THREAD, pDescriptor, 0, FALSE);
	{
		int i;
		char szTemp[256];
		int descriptor_tag = get_bits(BM_USER_THREAD, 8);
		int descriptor_length = get_bits(BM_USER_THREAD, 8);

		szBuffer[0] = '\0';
		for (i = 0; i < descriptor_length / 2; i++)
		{
			int content_nibble_level_1 = get_bits(BM_USER_THREAD, 4);
			int content_nibble_level_2 = get_bits(BM_USER_THREAD, 4);
			int user_nibble_1 = get_bits(BM_USER_THREAD, 4);
			int user_nibble_2 = get_bits(BM_USER_THREAD, 4);

			wsprintf(szTemp, "Content: %s (user 0x%1x/0x%1x)\r\n",
					 DecodeContentNibbles(content_nibble_level_1, content_nibble_level_2),
					 user_nibble_1, user_nibble_2);
			lstrcat(szBuffer, szTemp);				     
		}
	}
}

void DecodeParentalRatingDescriptor(char * szBuffer, BYTE * pDescriptor)
{
	set_buf(BM_USER_THREAD, pDescriptor, 0, FALSE);
	{
		int i;
		char szTemp[256];
		int descriptor_tag = get_bits(BM_USER_THREAD, 8);
		int descriptor_length = get_bits(BM_USER_THREAD, 8);

		szBuffer[0] = '\0';
		
		for (i = 0; i < descriptor_length / 4; i++)
		{
			int country_code = get_bits(BM_USER_THREAD, 24);
			int rating = get_bits(BM_USER_THREAD, 8);
			char szRating[64];

			if (rating == 0)
				lstrcpy(szRating, "undefined");
			else if (rating >= 1 && rating <= 0x0f)
				wsprintf(szRating, "%d years old", rating + 3);
			else
				lstrcpy(szRating, "defined by broadcaster");
			wsprintf(szTemp, "Parental rating: Country %c%c%c Rating %s\r\n",
				     country_code >> 16, country_code >> 8, country_code & 0xff,
					 szRating);
			lstrcat(szBuffer, szTemp);
		}
	}
}

void DecodeMultilingualComponentDescriptor(char * szBuffer, BYTE * pDescriptor)
{
	set_buf(BM_USER_THREAD, pDescriptor, 0, FALSE);
	szBuffer[0] = '\0';
	{
		int descriptor_tag = get_bits(BM_USER_THREAD, 8);
		int descriptor_length = get_bits(BM_USER_THREAD, 8);
		if (descriptor_length)
		{
			int component_tag = get_bits(BM_USER_THREAD, 8);
			int text_description_length;

			do
			{
				int j;
				char text_char[128];
				char szTemp[256];

				int language_code = get_bits(BM_USER_THREAD, 24);
				text_description_length = get_bits(BM_USER_THREAD, 8);

				for (j = 0; j < text_description_length; j++)
					text_char[j] = get_bits(BM_USER_THREAD, 8);
				text_char[j] = '\0';

				wsprintf(szTemp, "Multilangual component: %c%c%c %s\r\n",
						 language_code >> 16, language_code >> 8, language_code & 0xff,
						 text_char);
				lstrcat(szBuffer, szTemp);
			} while ((descriptor_length -= 4 + text_description_length) > 4);
		}
	}	
}

void DecodeATSCAC3Descriptor(char * szBuffer, BYTE * pDescriptor)
{
	lstrcpy(szBuffer, "ATSC AC3 Descriptor\r\n");
}

void DecodeATSCContentAdvisoryDescriptor(char * szBuffer, BYTE * pDescriptor, BOOL fShortMode)
{
	int i, j;
	char szRatingText[128] = {0};

	if (fShortMode == FALSE)
		lstrcpy(szBuffer, "ATSC Content Advisory Descriptor:\r\n");

	set_buf(BM_USER_THREAD, pDescriptor, 0, FALSE);
	{
		int descriptor_tag = get_bits(BM_USER_THREAD, 8);
		int descriptor_length = get_bits(BM_USER_THREAD, 8);
		int reserved1 = get_bits(BM_USER_THREAD, 2);
		int rating_region_count = get_bits(BM_USER_THREAD, 6);
		for (i = 0; i < rating_region_count; i++)
		{
			int rating_region = get_bits(BM_USER_THREAD, 8);
			int rated_dimensions = get_bits(BM_USER_THREAD, 8);

			for (j = 0; j < rated_dimensions; j++)
			{
				int rating_dimension_j = get_bits(BM_USER_THREAD, 8);
				int reserved2 = get_bits(BM_USER_THREAD, 4);
				int rating_value = get_bits(BM_USER_THREAD, 4);

				if (v->prrt[rating_region_count] != NULL)
				{
					if (lstrlen(szRatingText))
						lstrcat(szRatingText, "-");
					lstrcat(szRatingText, v->prrt[rating_region_count]->rrtdimension[rating_dimension_j].rrtrating[rating_value].szAbbreviatedRating);			
				}
			}
			{
				int rating_description_length = get_bits(BM_USER_THREAD, 8);

				char rating_description_text[24];
				char szTemp[128];

				memset(rating_description_text, 0, sizeof(rating_description_text));
				GetATSCMultipleString(BM_USER_THREAD, rating_description_text, rating_description_length);
				if (fShortMode == FALSE)
				{
					if (lstrlen(szRatingText) == 0)
						lstrcpy(szRatingText, "n/a");
					wsprintf(szTemp, " Region %d Rating: %s Description: %s\r\n", rating_region, szRatingText, rating_description_text);
				}
				else
				{
					if (lstrlen(szRatingText) == 0)
						lstrcpy(szTemp, rating_description_text);
					else
						wsprintf(szTemp, "%s(%s)", szRatingText, rating_description_text);
				}
				lstrcat(szBuffer, szTemp);
			}
		}
	}
}

void DecodeATSCCaptionServceDescriptor(char * szBuffer, BYTE * pDescriptor)
{
	int i;
	char szLanguage[4];
	char szTemp[128];

	lstrcpy(szBuffer, "ATSC Caption Service Descriptor:\r\n");
	set_buf(BM_USER_THREAD, pDescriptor, 0, FALSE);
	{
		int descriptor_tag = get_bits(BM_USER_THREAD, 8);
		int descriptor_length = get_bits(BM_USER_THREAD, 8);
		int reserved1 = get_bits(BM_USER_THREAD, 3);
		int number_of_services = get_bits(BM_USER_THREAD, 5);
		for (i = 0; i < number_of_services; i++)
		{
			szLanguage[0] = get_bits(BM_USER_THREAD, 8);
			szLanguage[1] = get_bits(BM_USER_THREAD, 8);
			szLanguage[2] = get_bits(BM_USER_THREAD, 8);
			szLanguage[3] = '\0';
			{
				int cc_type = get_bits(BM_USER_THREAD, 1);
				int reserved2 = get_bits(BM_USER_THREAD, 1);
				int line21_field;
				int caption_service_number;
				char szCaptionFormat[64];

				if (cc_type == 0)
				{
					char * szField;
					int reserved3 = get_bits(BM_USER_THREAD, 5);
					line21_field = get_bits(BM_USER_THREAD, 1);

					if (line21_field)
						szField = "Odd";
					else
						szField = "Even";
					wsprintf(szCaptionFormat, "Line 21 captions from %s field", szField);
				}
				else
				{
					caption_service_number = get_bits(BM_USER_THREAD, 6);
					wsprintf(szCaptionFormat, "Captions service %d", caption_service_number);
				}
				{
					int easy_reader = get_bits(BM_USER_THREAD, 1);
					int wide_aspect_ration = get_bits(BM_USER_THREAD, 1);
					int reserved4 = get_bits(BM_USER_THREAD, 14);

					wsprintf(szTemp, " Language: %s Format: %s\r\n", szLanguage, szCaptionFormat);
					lstrcat(szBuffer, szTemp);
				}
			}
		}
	}
}

static void DecodeISDBAdditionalARIBBxmlInfo(char *szBuffer, BYTE *pInfo, uint16_t data_component_id)
{
	char szTemp[512];

	set_buf(BM_USER_THREAD, pInfo, 0, FALSE);

	uint8_t transmission_format = get_bits(BM_USER_THREAD, 2) & 3;
	uint8_t entry_point_flag = get_bits(BM_USER_THREAD, 1) & 1;

	wsprintf(szBuffer, "");
	wsprintf(szTemp, " - Additional data component\r\n");
	lstrcat(szBuffer, szTemp);

	wsprintf(szTemp, "   Transmission format: %d (%s)\r\n"
					 "   Entry point flag: %s\r\n", 
		transmission_format, transmission_format == 0 ? "Data carousel and event message transmission methods" : "Reserved",
		TrueFalseString(entry_point_flag));
	lstrcat(szBuffer, szTemp);

	if (entry_point_flag) {
		uint8_t auto_start_flag = get_bits(BM_USER_THREAD, 1) & 1;
		uint8_t document_resolution = get_bits(BM_USER_THREAD, 4) & 0xf;
		uint8_t use_xml = get_bits(BM_USER_THREAD, 1) & 1;
		uint8_t default_version_flag = get_bits(BM_USER_THREAD, 1) & 1;
		uint8_t independent_flag = get_bits(BM_USER_THREAD, 1) & 1;
		uint8_t style_for_tv_flag = get_bits(BM_USER_THREAD, 1) & 1;
		wsprintf(szTemp,
			"   Auto start flag: %s\r\n"
			"   Document resolution: %s\r\n"
			"   Use XML: %s\r\n"
			"   Default version flag: %s\r\n"
			"   Independent flag: %s\r\n"
			"   Style for TV flag: %s\r\n",

			TrueFalseString(auto_start_flag),
			FormatISDBDocumentResolution(document_resolution),
			TrueFalseString(use_xml),
			TrueFalseString(default_version_flag),
			TrueFalseString(independent_flag),
			TrueFalseString(style_for_tv_flag));
		lstrcat(szBuffer, szTemp);

		/* reserved */
		get_bits(BM_USER_THREAD, 4);
		if (default_version_flag == 0) {
			uint16_t bml_major_version = get_bits(BM_USER_THREAD, 16);
			uint16_t bml_minor_version = get_bits(BM_USER_THREAD, 16);
			wsprintf(szTemp,
				"   BML version: %d.%d\r\n",
				bml_major_version, bml_minor_version);

			if (use_xml == 1) {
				uint16_t bxml_major_version = get_bits(BM_USER_THREAD, 16);
				uint16_t bxml_minor_version = get_bits(BM_USER_THREAD, 16);
				wsprintf(szTemp,
					"   XML version: %d.%d\r\n",
					bxml_major_version, bxml_minor_version);
			}
			lstrcat(szBuffer, szTemp);
		}
	} else {
		/* reserved */
		get_bits(BM_USER_THREAD, 5);
	}

	if (transmission_format == 0) {
		/* additional_arib_carousel_info() */
		uint8_t data_event_id = get_bits(BM_USER_THREAD, 4) & 0xf;
		uint8_t event_section_flag = get_bits(BM_USER_THREAD, 1) & 1;
		/* reserved */
		get_bits(BM_USER_THREAD, 3);

		/* the rest */
		uint8_t ondemand_retrieval_flag = get_bits(BM_USER_THREAD, 1) & 1;
		uint8_t file_storable_flag = get_bits(BM_USER_THREAD, 1) & 1;
		/* reserved */
		get_bits(BM_USER_THREAD, 6);

		wsprintf(szTemp,
			"   Data event id: %d\r\n"
			"   Event section flag: %s\r\n"
			"   On-demand retrieval flag: %s\r\n"
			"   File storable flag: %s\r\n",
			data_event_id, TrueFalseString(event_section_flag),
			TrueFalseString(ondemand_retrieval_flag), TrueFalseString(file_storable_flag));
		lstrcat(szBuffer, szTemp);
	}
}

static void DecodeISDBDataComponentDescriptor(char *szBuffer, BYTE *pDescriptor)
{
	char szTemp[512];
	char *ptr = szTemp;
	uint8_t bytes[32] = { 0, };
	int i;

	set_buf(BM_USER_THREAD, pDescriptor, 0, FALSE);

	int descriptor_tag = get_bits(BM_USER_THREAD, 8);
	int descriptor_length = get_bits(BM_USER_THREAD, 8);

	uint16_t data_component_id = get_bits(BM_USER_THREAD, 16);
	descriptor_length -= 2;

	for (i = 0; i < descriptor_length; i++)
		bytes[i] = get_bits(BM_USER_THREAD, 8);

	wsprintf(szTemp, " Data component id: 0x%04x (%s)\r\n", data_component_id, FormatISDBDataComponentId(data_component_id));
	lstrcat(v->szSIFormatBuffer, szTemp);

	/* decode the bytes */
	set_buf(BM_USER_THREAD, bytes, 0, FALSE);

	if (data_component_id == 0x0007) {
		/* ARIB-XML-base multimedia coding */
		DecodeISDBAdditionalARIBBxmlInfo(szTemp, bytes, data_component_id);
		lstrcat(v->szSIFormatBuffer, szTemp);
	} else if (data_component_id == 0x0008 || data_component_id == 0x0012) {
		/* ARIB-Subtitle& teletext coding */
		const char *isdb_timing[] = { "Asynchronous", "Program synchronous", "Time synchronous", "Undefined" };
		uint8_t DMF = get_bits(BM_USER_THREAD, 4);
		get_bits(BM_USER_THREAD, 2);
		uint8_t Timing = get_bits(BM_USER_THREAD, 2);
		wsprintf(szTemp, " - Additional data component of caption and superimpose\r\n"
			"   Display mode flag: %d\r\n"
			"   Timing: %d (%s)\r\n",
			DMF,
			Timing, isdb_timing[Timing]);
		lstrcat(v->szSIFormatBuffer, szTemp);
	} else if (data_component_id == 0x000c || data_component_id == 0x000d) {
		DecodeISDBAdditionalARIBBxmlInfo(szTemp, bytes, data_component_id);
		lstrcat(v->szSIFormatBuffer, szTemp);
	} else {
		/* undecoded bytes */
		wsprintf(szTemp, " Additional data component info (%d bytes): ", descriptor_length);
		lstrcat(v->szSIFormatBuffer, szTemp);

		for (i = 0; i < descriptor_length; i++)
			ptr += wsprintf(ptr, "%02X ", bytes[i]);
		wsprintf(ptr, "\r\n");
		lstrcat(v->szSIFormatBuffer, szTemp);
	}
}

void FormatDefaultEITDescriptor(char * szBuffer, BYTE * pDescriptor, BOOL fXML)
{
	int j;
	char szTemp2[16];

	if (fXML == TRUE)
	{
		wsprintf(szBuffer, "  <DESCRIPTOR>\r\n   <TAG>0x%02x</TAG>\r\n   <LENGTH>%d</LENGTH>\r\n   <DATA>", pDescriptor[0], pDescriptor[1]);
		for (j = 0; j < pDescriptor[1]; j++)
		{
			wsprintf(szTemp2, "0x%02x ", pDescriptor[2 + j]);
			lstrcat(szBuffer, szTemp2);
		}
		StripTrailingSpaces(szBuffer);
		lstrcat(szBuffer, "</DATA>\r\n   </DESCRIPTOR>");
	}
	else
	{
		wsprintf(szBuffer, "Descriptor: 0x%02x: ", pDescriptor[0]);
		for (j = 0; j < pDescriptor[1]; j++)
		{
			wsprintf(szTemp2, "%02x ", pDescriptor[2 + j]);
			lstrcat(szBuffer, szTemp2);
		}
	}

	lstrcat(szBuffer, "\r\n");
}

void DecodeAncilliaryDataDescriptor(char * szOutput, BYTE * pDescriptorData)
{
	lstrcpy(szOutput, " ");

	if (pDescriptorData[2] & 0x01)		lstrcat(szOutput, "DVD-Video Ancillary Data, ");
	if (pDescriptorData[2] & 0x02)		lstrcat(szOutput, "Extended Ancillary Data, ");
	if (pDescriptorData[2] & 0x04)		lstrcat(szOutput, "Announcement Switching Data, ");
	if (pDescriptorData[2] & 0x08)		lstrcat(szOutput, "DAB Ancillary Data, ");
	if (pDescriptorData[2] & 0x10)		lstrcat(szOutput, "Scale Factor Error Check (ScF-CRC), ");

	if (szOutput[lstrlen(szOutput) - 2] == ',')
		szOutput[lstrlen(szOutput) - 2] = '\0';
}

void DecodeAnnouncementSupportDescriptor(char * szOutput, BYTE * pDescriptorData)
{
	lstrcpy(szOutput, " Announcement support indicator: ");

	set_buf(BM_USER_THREAD, pDescriptorData, 0, FALSE);
	{
		int descriptor_tag = get_bits(BM_USER_THREAD, 8);
		int descriptor_length = get_bits(BM_USER_THREAD, 8);
		int announcement_support_indicator = get_bits(BM_USER_THREAD, 16);	
		
		if (announcement_support_indicator & 0x0001)		lstrcat(szOutput, "Emergency alarm, ");
		if (announcement_support_indicator & 0x0002)		lstrcat(szOutput, "Road Traffic flash, ");
		if (announcement_support_indicator & 0x0004)		lstrcat(szOutput, "Public Transport flash, ");
		if (announcement_support_indicator & 0x0008)		lstrcat(szOutput, "Warning message, ");
		if (announcement_support_indicator & 0x0010)		lstrcat(szOutput, "News flash, ");
		if (announcement_support_indicator & 0x0020)		lstrcat(szOutput, "Weather flash, ");
		if (announcement_support_indicator & 0x0040)		lstrcat(szOutput, "Event announcement, ");
		if (announcement_support_indicator & 0x0080)		lstrcat(szOutput, "Personal call, ");

		if (szOutput[lstrlen(szOutput) - 2] == ',')
			szOutput[lstrlen(szOutput) - 2] = '\0';
		lstrcat(szOutput, "\r\n");

		descriptor_length -= 2;
		while (descriptor_length)
		{
			int announcement_type = get_bits(BM_USER_THREAD, 4);
			int reserved_future_user = get_bits(BM_USER_THREAD, 1);
			int reference_type = get_bits(BM_USER_THREAD, 3);

			char szAnnouncementType[32];
			char szReferenceType[64];
			char szTemp[256];

			switch(announcement_type)
			{
			case 0:
				lstrcpy(szAnnouncementType, "Emergency alarm");
				break;
			case 1:
				lstrcpy(szAnnouncementType, "Road Traffic flash");
				break;
			case 2:
				lstrcpy(szAnnouncementType, "Public Transport flash");
				break;
			case 3:
				lstrcpy(szAnnouncementType, "Warning message");
				break;
			case 4:
				lstrcpy(szAnnouncementType, "News flash");
				break;
			case 5:
				lstrcpy(szAnnouncementType, "Weather flash");
				break;
			case 6:
				lstrcpy(szAnnouncementType, "Event announcement");
				break;
			case 7:
				lstrcpy(szAnnouncementType, "Personal call");
				break;
			default:
				lstrcpy(szAnnouncementType, "reserved for future use");
				break;
			}

			switch(reference_type)
			{
			case 0:
				lstrcpy(szReferenceType, "Announcement is broadcast in the usual audio stream of the service");
				break;
			case 1:
				lstrcpy(szReferenceType, "Announcement is broadcast in a separate audio stream that is part of the service");
				break;
			case 2:
				lstrcpy(szReferenceType, "Announcement is broadcast by means of a different service within the same transport stream");
				break;
			case 3:
				lstrcpy(szReferenceType, "Announcement is broadcast by means of a different service within a different transport stream");
				break;
			default:
				lstrcpy(szReferenceType, "reserved for future use");
				break;
			}
			wsprintf(szTemp, " Announcement Type: %s Reference Type: %s\r\n", szAnnouncementType, szReferenceType);
			lstrcat(szOutput, szTemp);
			descriptor_length--;

			if (reference_type == 0x01 || reference_type == 0x02 || reference_type == 0x03)
			{
				int original_network_id = get_bits(BM_USER_THREAD, 16);
				int transport_stream_id = get_bits(BM_USER_THREAD, 16);
				int service_id = get_bits(BM_USER_THREAD, 16);
				int component_tag = get_bits(BM_USER_THREAD, 8);
				wsprintf(szTemp, " Original Network ID: %d Transport Stream ID: %d Service ID: %d Component Tag: %d\r\n");
				lstrcat(szOutput, szTemp);
				descriptor_length -= 7;
			}
		}
	}
}

char * DecodeMosaicCellCount(int nCount)
{
	switch(nCount)
	{
	case 0:
		return "one cell";
	case 1:
		return "two cells";
	case 2:
		return "three cells";
	case 3:
		return "four cells";
	case 4:
		return "five cells";
	case 5:
		return "six cells";
	case 6:
		return "seven cells";
	case 7:
		return "eight cells";
	}

	return "";
}

void DecodeFrameRate(char * szFrameRate, int nRate)
{
	switch(nRate)
	{
	case 0:
		lstrcpy(szFrameRate, "forbidden");
		break;
	case 1:
		lstrcpy(szFrameRate, "23.976¼");
		break;
	case 2:
		lstrcpy(szFrameRate, "24");
		break;
	case 3:
		lstrcpy(szFrameRate, "25");
		break;
	case 4:
		lstrcpy(szFrameRate, "29.97¼");
		break;
	case 5:
		lstrcpy(szFrameRate, "30");
		break;
	case 6:
		lstrcpy(szFrameRate, "50");
		break;
	case 7:
		lstrcpy(szFrameRate, "59.94¼");
		break;
	case 8:
		lstrcpy(szFrameRate, "60");
		break;
	default:
		lstrcpy(szFrameRate, "reserved");
		break;
	}
}

void DecodeMPEG2Descriptor(BYTE * pDescriptorData, BOOL fHTMLMode)
{
	char szTemp[32 * 1024] = {""};

	int i;

	// User decoding in Pro
	for (i = 0; i < 5; i++)
	{
		if (DescriptorDecode[i] != NULL)
		{
			BOOL fRetVal = DescriptorDecode[i](FALSE, v->nNetworkPID, pDescriptorData, szTemp);
			if (fRetVal)
			{
				lstrcat(v->szSIFormatBuffer, szTemp);
				return;
			}
		}
	}

	switch(pDescriptorData[0])
	{
	case 0x00:	// reserved
	case 0x01:	// reserved
		lstrcpy(v->szSIFormatBuffer, " Reserved Descriptor");
		break;
	case 0x02:	// Video stream
		{
			char szFrameRate[20];

			wsprintf(szTemp, " Multiple frame rate flag: %s\r\n", TrueFalseString(pDescriptorData[2] & 0x80));
			lstrcat(v->szSIFormatBuffer, szTemp);
			DecodeFrameRate(szFrameRate, (pDescriptorData[2] >> 3) & 0xf);
			wsprintf(szTemp, " Frame rate: %s\r\n", szFrameRate);
			lstrcat(v->szSIFormatBuffer, szTemp);
			wsprintf(szTemp, " MPEG-1 only flag: %s\r\n", TrueFalseString(pDescriptorData[2] & 0x4));
			lstrcat(v->szSIFormatBuffer, szTemp);
			wsprintf(szTemp, " Constrained paramter flag: %s\r\n", TrueFalseString(pDescriptorData[2] & 0x2));
			lstrcat(v->szSIFormatBuffer, szTemp);
			wsprintf(szTemp, " Still picture flag: %s\r\n", TrueFalseString(pDescriptorData[2] & 0x1));
			lstrcat(v->szSIFormatBuffer, szTemp);
		}
		break;
	case 0x03:	// Audio stream 
		{
			wsprintf(szTemp, " Free format flag: %s\r\n", TrueFalseString(pDescriptorData[2] & 0x80));
			lstrcat(v->szSIFormatBuffer, szTemp);
			wsprintf(szTemp, " Layer: %d\r\n", (pDescriptorData[2] >> 4) & 3);
			lstrcat(v->szSIFormatBuffer, szTemp);
			wsprintf(szTemp, " Variable rate audio indicator: %s\r\n", TrueFalseString(pDescriptorData[2] & 0x8));
			lstrcat(v->szSIFormatBuffer, szTemp);
		}
		break;
	case 0x04:	// hierarchy_descriptor
		set_buf(BM_USER_THREAD, pDescriptorData, 0, FALSE);
		{
			char * szHierarchyType;

			int descriptor_tag = get_bits(BM_USER_THREAD, 8);
			int descriptor_length = get_bits(BM_USER_THREAD, 8);
			int reserved1 = get_bits(BM_USER_THREAD, 4);
			int hierarchy_type = get_bits(BM_USER_THREAD, 4);
			int reserved2 = get_bits(BM_USER_THREAD, 2);
			int hierarchy_layer_index = get_bits(BM_USER_THREAD, 6);
			int reserved3 = get_bits(BM_USER_THREAD, 2);
			int hierarchy_embedded_layer_index = get_bits(BM_USER_THREAD, 6);
			int reserved4 = get_bits(BM_USER_THREAD, 2);
			int hierarchy_channel = get_bits(BM_USER_THREAD, 6);

			switch(hierarchy_type)
			{
			case 1:
				szHierarchyType = "ITU-T Rec. H.262 | ISO/IEC 13818-2 Spatial Scalability";
				break;
			case 2:
				szHierarchyType = "ITU-T Rec. H.262 | ISO/IEC 13818-2 SNR Scalability";
				break;
			case 3:
				szHierarchyType = "ITU-T Rec. H.262 | ISO/IEC 13818-2 Temporal Scalability";
				break;
			case 4:
				szHierarchyType = "ITU-T Rec. H.262 | ISO/IEC 13818-2 Data partitioning";
				break;
			case 5:
				szHierarchyType = "ISO/IEC 13818-3 Extension bitstream";
				break;
			case 6:
				szHierarchyType = "ITU-T Rec. H.222.0 | ISO/IEC 13818-1 Private Stream";
				break;
			case 15:
				szHierarchyType = "Base layer";
				break;
			default:
				szHierarchyType = "reserved";
				break;
			}
			wsprintf(szTemp, " Hierarchy Type: %s\r\n Hierarchy Layer Index: %d Embedded Layer Index: %d Channel: %d\r\n",
				     szHierarchyType, hierarchy_layer_index, hierarchy_embedded_layer_index, hierarchy_channel);
			lstrcat(v->szSIFormatBuffer, szTemp);
		}
		break;
	case 0x05:	// registration descriptor
		{
			DWORD dwFormatIdentifier = ( (pDescriptorData[2] << 24) 
									   + (pDescriptorData[3] << 16)
									   + (pDescriptorData[4] << 8)
									   + pDescriptorData[5] );
			wsprintf(szTemp, " Format identifier: 0x%08x (%c%c%c%c)\r\n",
				     dwFormatIdentifier,
					 pDescriptorData[2], pDescriptorData[3],
					 pDescriptorData[4], pDescriptorData[5]);
			lstrcat(v->szSIFormatBuffer, szTemp);
		}
		break;
	case 0x06:			// data stream alignment
		{
			char szAlignment[30];

			switch(pDescriptorData[2])
			{
			case 0:
				lstrcpy(szAlignment, "reserved");
				break;
			case 1:
				lstrcpy(szAlignment, "slice, or video access unit");
				break;
			case 2:
				lstrcpy(szAlignment, "video access unit");
				break;
			case 3:
				lstrcpy(szAlignment, "GOP, or SEQ");
				break;
			case 4:
				lstrcpy(szAlignment, "SEQ");
				break;
			default:
				lstrcpy(szAlignment, "reserved");
				break;
			}
			wsprintf(szTemp, " Alignment type: %s\r\n", szAlignment);
			lstrcat(v->szSIFormatBuffer, szTemp);
		}
		break;
	case 0x07:	// target_background_grid_descriptor
		set_buf(BM_USER_THREAD, pDescriptorData, 0, FALSE);
		{
			int descriptor_tag = get_bits(BM_USER_THREAD, 8);
			int descriptor_length = get_bits(BM_USER_THREAD, 8);
			int horizontal_size = get_bits(BM_USER_THREAD, 14);
			int vertical_size = get_bits(BM_USER_THREAD, 14);
			int aspect_ratio_information = get_bits(BM_USER_THREAD, 4);
			char * szAspectRatio;

			switch(aspect_ratio_information)
			{
			case 0:
				szAspectRatio = "forbidden";
				break;
			case 1:
				szAspectRatio = "square pixels";
				break;
			case 2:
				szAspectRatio = "4:3";
				break;
			case 3:
				szAspectRatio = "16:9";
				break;
			case 4:
				szAspectRatio = "2.21:1";
				break;
			default:
				szAspectRatio = "reserved";
				break;
			}
			wsprintf(szTemp, " Horizontal Size: %d Vertical Size: %d Aspect: %s\r\n", 
				     horizontal_size, vertical_size, szAspectRatio);
			lstrcat(v->szSIFormatBuffer, szTemp);
		}
		break;
	case 0x08:	// video_window_descriptor
		set_buf(BM_USER_THREAD, pDescriptorData, 0, FALSE);
		{
			int descriptor_tag = get_bits(BM_USER_THREAD, 8);
			int descriptor_length = get_bits(BM_USER_THREAD, 8);	
			int horizontal_offset = get_bits(BM_USER_THREAD, 14);
			int vertical_offset = get_bits(BM_USER_THREAD, 14);
			int window_priority = get_bits(BM_USER_THREAD, 4);

			wsprintf(szTemp, " Horizontal Offset: %d Vertical Offset: %d Window Priority: %d\r\n",
					 horizontal_offset, vertical_offset, window_priority);
			lstrcat(v->szSIFormatBuffer, szTemp);
		}
		break;
	case 0x09:		// CA
		set_buf(BM_USER_THREAD, pDescriptorData, 0, FALSE);
		{
			BOOL fDecodedPrivateData = FALSE;
			int descriptor_tag = get_bits(BM_USER_THREAD, 8);
			int descriptor_length = get_bits(BM_USER_THREAD, 8);
			int CA_system_ID = get_bits(BM_USER_THREAD, 16);
			int reserved = get_bits(BM_USER_THREAD, 3);
			int CA_PID = get_bits(BM_USER_THREAD, 13);
			char szCAName[50] = {0};

			FormatCASystemName(CA_system_ID, szCAName);

			wsprintf(szTemp, " CA System ID: %d (0x%04x) %s\r\n", CA_system_ID, CA_system_ID, szCAName);
			lstrcat(v->szSIFormatBuffer, szTemp);
			wsprintf(szTemp, " CA PID %d (0x%04x)\r\n", CA_PID, CA_PID);
			lstrcat(v->szSIFormatBuffer, szTemp);
			if (!v->fPlainCADescriptors)
			{
				if (descriptor_length >= 8)
				{
					int operator_id_tag = get_bits(BM_USER_THREAD, 8);
					if (operator_id_tag == 0x10)
					{
						int operator_id_length = get_bits(BM_USER_THREAD, 8);
						if (operator_id_length == 2)
						{
							int operator_id = get_bits(BM_USER_THREAD, 16);
							wsprintf(szTemp, " CA operator ID %d (0x%04x)\r\n", operator_id, operator_id);
							lstrcat(v->szSIFormatBuffer, szTemp);
							fDecodedPrivateData = TRUE;
						}
					}
				}
			}
			if (v->fPlainCADescriptors || fDecodedPrivateData == FALSE)
			{
				if (!v->fPlainCADescriptors)
				{
					// Reset bit position
					set_buf(BM_USER_THREAD, pDescriptorData, 0, FALSE);
					{
						BOOL fDecodedPrivateData = FALSE;
						int descriptor_tag = get_bits(BM_USER_THREAD, 8);
						int descriptor_length = get_bits(BM_USER_THREAD, 8);
						int CA_system_ID = get_bits(BM_USER_THREAD, 16);
						int reserved = get_bits(BM_USER_THREAD, 3);
						int CA_PID = get_bits(BM_USER_THREAD, 13);
						/// todo -- what's this?
					}
				}

				if (descriptor_length > 4)
				{
					int nPrivateDataLength = descriptor_length - 4;
					lstrcpy(szTemp, " CA private data: ");
					while (nPrivateDataLength)
					{
						char szTemp2[16];
						wsprintf(szTemp2, "%02x ", get_bits(BM_USER_THREAD, 8));
						lstrcat(szTemp, szTemp2);
						nPrivateDataLength--;
					}
					lstrcat(szTemp, "\r\n");
					lstrcat(v->szSIFormatBuffer, szTemp);
				}
			}
		}
		break;
	case 0x0a:	// ISO 639 Language
		{
			char szLanguage[4];

			szLanguage[0] = pDescriptorData[2];
			szLanguage[1] = pDescriptorData[3];
			szLanguage[2] = pDescriptorData[4];
			szLanguage[3] = 0;
			wsprintf(szTemp, " Language: %s\r\n", szLanguage);
			lstrcat(v->szSIFormatBuffer, szTemp);
			lstrcat(v->szSIFormatBuffer, " Audio type: ");
			switch(pDescriptorData[5])
			{
			case 0x00:
				lstrcat(v->szSIFormatBuffer, "undefined\r\n");
				break;
			case 0x01:
				lstrcat(v->szSIFormatBuffer, "clean effects\r\n");
				break;
			case 0x02:
				lstrcat(v->szSIFormatBuffer, "hearing impaired\r\n");
				break;
			case 0x03:
				lstrcat(v->szSIFormatBuffer, "visual impaired commentary\r\n");
				break;
			default:
				lstrcat(v->szSIFormatBuffer, "visual impaired commentary\r\n");
				break;
			}
		}
		break;
	case 0x0b:	// system_clock_descriptor
		set_buf(BM_USER_THREAD, pDescriptorData, 0, FALSE);
		{
			int descriptor_tag = get_bits(BM_USER_THREAD, 8);
			int descriptor_length = get_bits(BM_USER_THREAD, 8);
		
			int external_clock_reference_indicator = get_bits(BM_USER_THREAD, 1);
			int reserved1 = get_bits(BM_USER_THREAD, 1);
			int clock_accuracy_integer = get_bits(BM_USER_THREAD, 6);
			int clock_accuracy_exponent = get_bits(BM_USER_THREAD, 3);
			int reserved2 = get_bits(BM_USER_THREAD, 5);

			wsprintf(szTemp, " External Clock Reference: %d Clock Accuracy Integer: %d Exponent: %d\r\n",
				     external_clock_reference_indicator, clock_accuracy_integer, clock_accuracy_exponent);
			lstrcat(v->szSIFormatBuffer, szTemp);				   
		}
		break;
	case 0x0c:	// multiplex_buffer_utilization_descriptor
		set_buf(BM_USER_THREAD, pDescriptorData, 0, FALSE);
		{
			int descriptor_tag = get_bits(BM_USER_THREAD, 8);
			int descriptor_length = get_bits(BM_USER_THREAD, 8);

			int bound_valid_flag = get_bits(BM_USER_THREAD, 1);
			int LTW_offset_lower_bound = get_bits(BM_USER_THREAD, 15);
			int reserved = get_bits(BM_USER_THREAD, 1);
			int LTW_offset_upper_bound = get_bits(BM_USER_THREAD, 14);
			
			wsprintf(szTemp, " Bound Valid Flag: %d\r\n LTW Offset Lower: %d Upper: %d\r\n",
				     bound_valid_flag, LTW_offset_lower_bound, LTW_offset_upper_bound);
			lstrcat(v->szSIFormatBuffer, szTemp);
		}
		break;
	case 0x0d:	// copyright_descriptor
		goto DecodeMPEG2Descriptor_Default;
	case 0x0e:	// maximum bitrate
		{
			int nMaximumBitrate = ( (pDescriptorData[2] << 16)
								  + (pDescriptorData[3] << 8)
								  + pDescriptorData[4]) & 0x3fffff;
			nMaximumBitrate *= 50;
			wsprintf(szTemp, " Maximum bitrate: %d bytes per second\r\n", nMaximumBitrate);
			lstrcat(v->szSIFormatBuffer, szTemp);
		}
		break;
	case 0x0f:	// private data indicator descriptor
		goto DecodeMPEG2Descriptor_Default;
	case 0x10:	// smoothing buffer descriptor
		set_buf(BM_USER_THREAD, pDescriptorData, 0, FALSE);
		{
			int descriptor_tag = get_bits(BM_USER_THREAD, 8);
			int descriptor_length = get_bits(BM_USER_THREAD, 8);

			int reserved1 = get_bits(BM_USER_THREAD, 2);
			int sb_leak_rate = get_bits(BM_USER_THREAD, 22);
			int reserved2 = get_bits(BM_USER_THREAD, 2);
			int sb_size = get_bits(BM_USER_THREAD, 22);

			wsprintf(szTemp, " SB Leak Rate: %d SB Size: %d\r\n", sb_leak_rate, sb_size);
			lstrcat(v->szSIFormatBuffer, szTemp);
		}
		break;
	case 0x11:	// STD_descriptor
		set_buf(BM_USER_THREAD, pDescriptorData, 0, FALSE);
		{
			int descriptor_tag = get_bits(BM_USER_THREAD, 8);
			int descriptor_length = get_bits(BM_USER_THREAD, 8);

			int reserved = get_bits(BM_USER_THREAD, 7);
			int leak_valid_flag = get_bits(BM_USER_THREAD, 1);

			wsprintf(szTemp, " Leak Valid Flag: %d\r\n", leak_valid_flag);
			lstrcat(v->szSIFormatBuffer, szTemp);
		}
		break;
	case 0x12:	// IBP descriptor
		{
			wsprintf(szTemp, " Closed GOP flag: %s\r\n", TrueFalseString(pDescriptorData[2] & 0x80));
			lstrcat(v->szSIFormatBuffer, szTemp);
			wsprintf(szTemp, " Identical GOP flag: %s\r\n", TrueFalseString(pDescriptorData[2] & 0x40));
			lstrcat(v->szSIFormatBuffer, szTemp);
			wsprintf(szTemp, " Maximum GOP length: %d\r\n", ((pDescriptorData[2] << 8) + pDescriptorData[3]) & 0x3fff);
			lstrcat(v->szSIFormatBuffer, szTemp);
		}
		break;
	case 40:
		set_buf(BM_USER_THREAD, pDescriptorData, 0, FALSE);
		{
			int descriptor_tag = get_bits(BM_USER_THREAD, 8);
			int descriptor_length = get_bits(BM_USER_THREAD, 8);

			int profile_idc = get_bits(BM_USER_THREAD, 8);
			int constraint_set0_flag = get_bits(BM_USER_THREAD, 1);
			int constraint_set1_flag = get_bits(BM_USER_THREAD, 1);
			int constraint_set2_flag = get_bits(BM_USER_THREAD, 1);
			int AVC_compatible_flags = get_bits(BM_USER_THREAD, 5);
			int level_idc = get_bits(BM_USER_THREAD, 8);
			int AVC_still_present = get_bits(BM_USER_THREAD, 1);
			int AVC_24_hour_picture_flag = get_bits(BM_USER_THREAD, 1);

			wsprintf(szTemp, " Profile idc: %d Constraint set 0=%d 1=%d 2=%d\r\n", profile_idc,
				     constraint_set0_flag, constraint_set1_flag, constraint_set2_flag);
			lstrcat(v->szSIFormatBuffer, szTemp);
			wsprintf(szTemp, " AVC compatible: %d Level idc %d\r\n", AVC_compatible_flags, level_idc);
			lstrcat(v->szSIFormatBuffer, szTemp);
			wsprintf(szTemp, " AVC still present: %d 24 hour picture %d\r\n", AVC_still_present, AVC_24_hour_picture_flag);
			lstrcat(v->szSIFormatBuffer, szTemp);
		}
		break;
	case 42:
//		int  = get_bits(BM_USER_THREAD, );
		set_buf(BM_USER_THREAD, pDescriptorData, 0, FALSE);
		{
			int n90kHz_flag;
			int N = 1;
			int K = 300;
			int num_units_in_tick;

			int fixed_frame_rate_flag = get_bits(BM_USER_THREAD, 1);
			int temporal_poc_flag = get_bits(BM_USER_THREAD, 1);
			int picture_to_display_conversion_flag = get_bits(BM_USER_THREAD, 1);

			int descriptor_tag = get_bits(BM_USER_THREAD, 8);
			int descriptor_length = get_bits(BM_USER_THREAD, 8);

			int hrd_management_valid_flag = get_bits(BM_USER_THREAD, 1);
			int reserved = get_bits(BM_USER_THREAD, 6);
			int picture_and_timing_info_present = get_bits(BM_USER_THREAD, 1);
			if (picture_and_timing_info_present)
			{
				n90kHz_flag = get_bits(BM_USER_THREAD, 1);
				reserved = get_bits(BM_USER_THREAD, 7);
				if (n90kHz_flag == 0)
				{
					N = get_bits(BM_USER_THREAD, 32);
					K = get_bits(BM_USER_THREAD, 32);
				}
				num_units_in_tick = get_bits(BM_USER_THREAD, 32);
			}
		}
		break;
	case 0x40:	// network name descriptor -- already done for us in parser.c ParseDVBNITPacket(...)
		if (v->nNetworkPID != 0x0010)
			goto DecodeMPEG2Descriptor_Default;
		break;
	case 0x41:	// service_list_descriptor
		if (v->nNetworkPID != 0x0010)
			goto DecodeMPEG2Descriptor_Default;
		{
			int descriptor_length = pDescriptorData[1] - 2;
			BYTE * pServiceListDescriptor = &pDescriptorData[2];

			while (descriptor_length > 0)
			{
				int nServiceID = pServiceListDescriptor[0] << 8 | pServiceListDescriptor[1];
				int nServiceType = pServiceListDescriptor[2];
				char szServiceType[64];
				char szServiceID[128];

				DecodeServiceType(szServiceType, nServiceType);
				if (v->pChannelData[nServiceID] != NULL)
					wsprintf(szServiceID, "%d (%s)", nServiceID, v->pChannelData[nServiceID]->szShortName);
				else
					wsprintf(szServiceID, "%d", nServiceID);
				wsprintf(szTemp, " Service: %s %s (0x%02x)\r\n", szServiceID, szServiceType, nServiceType);
				lstrcat(v->szSIFormatBuffer, szTemp);
				pServiceListDescriptor += 3;
				descriptor_length -= 3;
			}
			break;
		}
		break;
	case 0x42:	// stuffing descriptor
		goto DecodeMPEG2Descriptor_Default;
#ifndef DECODE_ALL_DESCRIPTORS
	case 0x43:	// satellite delivery descriptor -- already decoded for us	
	case 0x44:	// cable delivery descriptor
		goto DecodeMPEG2Descriptor_Default;
#else DECODE_ALL_DESCRIPTORS
#endif DECODE_ALL_DESCRIPTORS
	case 0x45:	// VBI Data
		if (v->nNetworkPID != 0x0010)
			goto DecodeMPEG2Descriptor_Default;
		set_buf(BM_USER_THREAD, pDescriptorData, 0, FALSE);
		{
			int descriptor_tag = get_bits(BM_USER_THREAD, 8);
			int descriptor_length = get_bits(BM_USER_THREAD, 8);
			int i;

			for (i = 0; i < descriptor_length; )
			{
				int data_service_id = get_bits(BM_USER_THREAD, 8);
				int data_service_descriptor_length = get_bits(BM_USER_THREAD, 8);
				int j;
				char szDataServiceDescriptorID[64];

				i += 2;
				DecodeVBIDataServiceDescription(data_service_id, szDataServiceDescriptorID);

				if (   data_service_id == 0x01
					|| data_service_id == 0x02
					|| data_service_id == 0x04
					|| data_service_id == 0x05
					|| data_service_id == 0x06
					|| data_service_id == 0x07)
				{

					for (j = 0; j < data_service_descriptor_length; j++)
					{
						int reserved = get_bits(BM_USER_THREAD, 2);
						int field_parity = get_bits(BM_USER_THREAD, 1);
						int line_offset = get_bits(BM_USER_THREAD, 5);

						wsprintf(szTemp, " %s Line Offset: %d Field parity: %d\r\n",
							szDataServiceDescriptorID,
							line_offset,
							field_parity);
					}
				}
				else
				{
					for (j = 0; j < data_service_descriptor_length; j++)
					{
						int reserved = get_bits(BM_USER_THREAD, 8);

						wsprintf(szTemp, " %s\r\n", szDataServiceDescriptorID);
					}
				}
				lstrcat(v->szSIFormatBuffer, szTemp);
				i += data_service_descriptor_length;
			}
		}
		break;
	case 0x46:	// VBI Teletext descriptor		
	case 0x56:	// Teletext descriptor
		if (v->nNetworkPID != 0x0010)
			goto DecodeMPEG2Descriptor_Default;
		set_buf(BM_USER_THREAD, pDescriptorData, 0, FALSE);
		{
			int descriptor_tag = get_bits(BM_USER_THREAD, 8);
			int descriptor_length = get_bits(BM_USER_THREAD, 8);
			int i;

			for (i = 0; i < descriptor_length; i += 5)
			{
				int language_code = get_bits(BM_USER_THREAD, 24);
				int teletext_type = get_bits(BM_USER_THREAD, 5);
				int teletext_magazine_number = get_bits(BM_USER_THREAD, 3);
				int teletext_page_number = get_bits(BM_USER_THREAD, 8);

				char szTeletextType[64];

				switch(teletext_type)
				{
				default:
					lstrcpy(szTeletextType, "Reserved for future use");
					break;
				case 0x01:
					lstrcpy(szTeletextType, "Initial Teletext page");
					break;
				case 0x02:
					lstrcpy(szTeletextType, "Teletext subtitle page");
					break;
				case 0x03:
					lstrcpy(szTeletextType, "Additional information page");
					break;
				case 0x04:
					lstrcpy(szTeletextType, "Program schedule page");
					break;
				case 0x05:
					lstrcpy(szTeletextType, "Teletext subtitle page for hearing impaired people");
					break;
				}
				wsprintf(szTemp, " Language: %c%c%c Type: %s Magazine: 0x%02x Page: 0x%02x\r\n",
					language_code >> 16, language_code >> 8 & 0xff, language_code & 0xff,
					szTeletextType,
					teletext_magazine_number,
					teletext_page_number);
				lstrcat(v->szSIFormatBuffer, szTemp);
			}
		}
		break;
	case 0x47: // bouquet name descriptor
		if (v->nNetworkPID != 0x0010)
			goto DecodeMPEG2Descriptor_Default;
		set_buf(BM_USER_THREAD, pDescriptorData, 0, FALSE);
		{
			int descriptor_tag = get_bits(BM_USER_THREAD, 8);
			int descriptor_length = get_bits(BM_USER_THREAD, 8);
			int nOutIndex = 0;
			char szBouquetName[128] = {0};
			while (descriptor_length)
			{
				szBouquetName[nOutIndex++] = get_bits(BM_USER_THREAD, 8);
				descriptor_length--;
			}
			szBouquetName[nOutIndex] = '\0';
			wsprintf(szTemp, " Bouquet Name: %s\r\n", szBouquetName);
			lstrcat(v->szSIFormatBuffer, szTemp);
		}
		break;
	case 0x48:	// service descriptor
		if (v->nNetworkPID != 0x0010)
			goto DecodeMPEG2Descriptor_Default;
		set_buf(BM_USER_THREAD, pDescriptorData, 0, FALSE);
		{
			int i;
			int service_provider_name_length;
			int service_name_length;
			int descriptor_tag = get_bits(BM_USER_THREAD, 8);
			int descriptor_length = get_bits(BM_USER_THREAD, 8);
			int service_type = get_bits(BM_USER_THREAD, 8);
			char szProviderName[128];
			char szServiceName[128];
			char szServiceType[64];

			DecodeServiceType(szServiceType, service_type);

			service_provider_name_length = get_bits(BM_USER_THREAD, 8);
			for (i = 0; i < service_provider_name_length; i++)
				szProviderName[i] = get_bits(BM_USER_THREAD, 8);
			szProviderName[i] = '\0';

			service_name_length = get_bits(BM_USER_THREAD, 8);
			for (i = 0; i < service_name_length; i++)
				szServiceName[i] = get_bits(BM_USER_THREAD, 8);
			szServiceName[i] = '\0';

			wsprintf(szTemp, " Service Type: %s Provider: %s Service: %s\r\n",
				     szServiceType, szProviderName, szServiceName);
			lstrcat(v->szSIFormatBuffer, szTemp);
		}
		break;
	case 0x49:		// country availability descriptor
		if (v->nNetworkPID != 0x0010)
			goto DecodeMPEG2Descriptor_Default;
		set_buf(BM_USER_THREAD, pDescriptorData, 0, FALSE);
		{
			int descriptor_tag = get_bits(BM_USER_THREAD, 8);
			int descriptor_length = get_bits(BM_USER_THREAD, 8);
			if (descriptor_length)
			{
				int country_availability_flags = get_bits(BM_USER_THREAD, 1);
				int reserved = get_bits(BM_USER_THREAD, 7);
				while (descriptor_length > 3)
				{
					char szAvailability[32];
					char c1, c2, c3;

					if (country_availability_flags)
						lstrcpy(szAvailability, "Available in");
					else
						lstrcpy(szAvailability, "Not available in");

					c1 = get_bits(BM_USER_THREAD, 8); c2 = get_bits(BM_USER_THREAD, 8); c3 = get_bits(BM_USER_THREAD, 8);
					wsprintf(szTemp, "%s %c%c%c\r\n", szAvailability, c1, c2, c3);
					lstrcat(v->szSIFormatBuffer, szTemp);
					descriptor_length -= 3;
				}
			}				
		}
		break;
	case 0x4a:	// linkage
		if (v->nNetworkPID != 0x0010)
			goto DecodeMPEG2Descriptor_Default;
		{
			int nTransportStreamID = (pDescriptorData[2] << 8) + pDescriptorData[3];
			int nOriginalNetworkID = (pDescriptorData[4] << 8) + pDescriptorData[5];
			int nServiceID = (pDescriptorData[6] << 8) + pDescriptorData[7];
			int nLinkageType = pDescriptorData[8];
			char szLinkageType[128];
			char szQuickNIT[128] = {0};
		
			switch(nLinkageType)
			{
			case 0x01:
				lstrcpy(szLinkageType, "information service");
				break;
			case 0x02:
				lstrcpy(szLinkageType, "Electronic Programme Guide (EPG) service");
				break;
			case 0x03:
				lstrcpy(szLinkageType, "CA replacement service");
				break;
			case 0x04:
				lstrcpy(szLinkageType, "transport stream containing complete Network/Bouquet SI");
				break;
			case 0x05:
				lstrcpy(szLinkageType, "service replacement service");
				break;
			case 0x06:
				lstrcpy(szLinkageType, "data broadcast service");
				break;
			case 0x07:
				lstrcpy(szLinkageType, "RCS Map");
				break;
			case 0x08:
				lstrcpy(szLinkageType, "mobile hand-over");
				break;
			case 0x09:
				lstrcpy(szLinkageType, "System Software Update Service");
				break;
			case 0x0A:
				lstrcpy(szLinkageType, "TS containing SSU BAT or NIT");
				break;
			case 0x0B:
				lstrcpy(szLinkageType, "IP/MAC Notification Service");
				break;
			case 0x0C:
				lstrcpy(szLinkageType, "TS containing INT BAT or NIT");
				break;
			default:
				if (   (nLinkageType == 0)
					|| (nLinkageType == 0xFF)
					|| ( (nLinkageType >= 0x0D) && (nLinkageType <= 0x7f) ) )
					wsprintf(szLinkageType, "reserved for future use (0x%x)", nLinkageType);
				else
					wsprintf(szLinkageType, "user defined (0x%x)", nLinkageType);
			}

			QuickFormatNIT(szQuickNIT, nTransportStreamID, FALSE);
			wsprintf(szTemp, " Transport Stream ID: %d (0x%04x) %s\r\n Original Network ID: %d (0x%04x)\r\n Service ID: %d\r\n Linkage Type: %s\r\n",
					 nTransportStreamID, nTransportStreamID,
					 szQuickNIT,
					 nOriginalNetworkID, nOriginalNetworkID,
					 nServiceID,
					 szLinkageType);
			lstrcat(v->szSIFormatBuffer, szTemp);
		}
		break;
	case 0x4b:	// NVOD reference descriptor
		if (v->nNetworkPID != 0x0010)
			goto DecodeMPEG2Descriptor_Default;
		set_buf(BM_USER_THREAD, pDescriptorData, 0, FALSE);
		{
			int descriptor_tag = get_bits(BM_USER_THREAD, 8);
			int descriptor_length = get_bits(BM_USER_THREAD, 8);
			
			while (descriptor_length)
			{
				int transport_stream_id = get_bits(BM_USER_THREAD, 16);
				int original_network_id = get_bits(BM_USER_THREAD, 16);
				int service_id = get_bits(BM_USER_THREAD, 16);

				wsprintf(szTemp, " Transport Stream ID: %d Original Network ID: %d Service ID: %d\r\n",
					     transport_stream_id, original_network_id, service_id);
				lstrcat(v->szSIFormatBuffer, szTemp);
				descriptor_length -= 6;
			};
		}
		break;
	case 0x4c:	// time shifted service descriptor
		if (v->nNetworkPID != 0x0010)
			goto DecodeMPEG2Descriptor_Default;
		set_buf(BM_USER_THREAD, pDescriptorData, 0, FALSE);
		{
			int descriptor_tag = get_bits(BM_USER_THREAD, 8);
			int descriptor_length = get_bits(BM_USER_THREAD, 8);
			int reference_service_id = get_bits(BM_USER_THREAD, 16);

			wsprintf(szTemp, " Reference Service ID: %d\r\n", reference_service_id);
			lstrcat(v->szSIFormatBuffer, szTemp);
		}
		break;
	case 0x4d:	// short event descriptor
		if (v->nNetworkPID != 0x0010)
			goto DecodeMPEG2Descriptor_Default;
		set_buf(BM_USER_THREAD, pDescriptorData, 0, FALSE);
		{
			int i;
			char event_name_char[256];
			char text[256];
			int descriptor_tag = get_bits(BM_USER_THREAD, 8);
			int descriptor_length = get_bits(BM_USER_THREAD, 8);
			int language_code = get_bits(BM_USER_THREAD, 24);
			int event_name_length = get_bits(BM_USER_THREAD, 8);
			for (i=0; i < event_name_length; i++)
				event_name_char[i] = get_bits(BM_USER_THREAD, 8);
			event_name_char[i] = 0;
			{
				int ext_length = get_bits(BM_USER_THREAD, 8);
				for (i = 0; i < ext_length; i++)
					text[i] = get_bits(BM_USER_THREAD, 8);
				text[i] = 0;

				wsprintf(szTemp, " Language: %c%c%c Event: %s Text: %s\r\n",
					     language_code >> 16,
						 language_code >> 8,
						 language_code & 0xff,
						 event_name_char,
						 text);
				lstrcat(v->szSIFormatBuffer, szTemp);
			}
		}
		break;
	case 0x4e:	// extended event descriptor  -- already decoded
		goto DecodeMPEG2Descriptor_Default;
	case 0x4f:	// time shifted event descriptor
		if (v->nNetworkPID != 0x0010)
			goto DecodeMPEG2Descriptor_Default;
		set_buf(BM_USER_THREAD, pDescriptorData, 0, FALSE);
		{
			int descriptor_tag = get_bits(BM_USER_THREAD, 8);
			int descriptor_length = get_bits(BM_USER_THREAD, 8);
			int reference_service_id = get_bits(BM_USER_THREAD, 16);
			int reference_event_id = get_bits(BM_USER_THREAD, 16);
			
			wsprintf(szTemp, " Reference Service ID: %d Reference Event ID: %d\r\n", 
				     reference_service_id, reference_event_id);
			lstrcat(v->szSIFormatBuffer, szTemp);
		}
		break;
	case 0x50:	// component descriptor
		if (v->nNetworkPID != 0x0010)
			goto DecodeMPEG2Descriptor_Default;
		DecodeComponentDescriptor(szTemp, pDescriptorData);
		lstrcat(v->szSIFormatBuffer, szTemp);
		break;
	case 0x51:	// mosaic descriptor
		if (v->nNetworkPID != 0x0010)
			goto DecodeMPEG2Descriptor_Default;
		set_buf(BM_USER_THREAD, pDescriptorData, 0, FALSE);
		{
			int i, j;
			int descriptor_tag = get_bits(BM_USER_THREAD, 8);
			int descriptor_length = get_bits(BM_USER_THREAD, 8);

			int mosaic_entry_point = get_bits(BM_USER_THREAD, 1);
			int number_of_horizontal_elementary_cells = get_bits(BM_USER_THREAD, 3);
			int reserved_future_use1 = get_bits(BM_USER_THREAD, 1);
			int number_of_vertical_elementary_cells = get_bits(BM_USER_THREAD, 3);		
			wsprintf(szTemp, " Mosaic Entry Point: %s Horizontal Cells: %d Vertical Cells: %d\r\n",
				     TrueFalseString(mosaic_entry_point), number_of_horizontal_elementary_cells, number_of_vertical_elementary_cells);
			lstrcat(v->szSIFormatBuffer, szTemp);

			for (i = 0; i < number_of_vertical_elementary_cells; i++)
			{
				int logical_cell_id = get_bits(BM_USER_THREAD, 6);
				int reserved_future_use2 = get_bits(BM_USER_THREAD, 7);
				int logical_cell_presentation_info = get_bits(BM_USER_THREAD, 3);
				int elementary_cell_field_length = get_bits(BM_USER_THREAD, 8);
				int cell_linkage_info;

				wsprintf(szTemp, " Logical Cell ID: %d Presentation Info: %d\r\n",
					     logical_cell_id, logical_cell_presentation_info);
				lstrcat(v->szSIFormatBuffer, szTemp);

				for (j = 0; j < elementary_cell_field_length; j++)
				{
					int reserved_future_use3 = get_bits(BM_USER_THREAD, 2);
					int elementary_cell_id = get_bits(BM_USER_THREAD, 6);

					wsprintf(szTemp, " Elementary Cell ID: %d\r\n", 
						     elementary_cell_id);
					lstrcat(v->szSIFormatBuffer, szTemp);
				}
				cell_linkage_info = get_bits(BM_USER_THREAD, 8);
				if (cell_linkage_info == 0x01)
				{
					int bouquet_id = get_bits(BM_USER_THREAD, 16);
					wsprintf(szTemp, " Bouquet ID: %d\r\n", 
						     bouquet_id);
					lstrcat(v->szSIFormatBuffer, szTemp);
				}
				if (cell_linkage_info == 0x02)
				{
					int original_network_id = get_bits(BM_USER_THREAD, 16);
					int transport_stream_id = get_bits(BM_USER_THREAD, 16);
					int service_id = get_bits(BM_USER_THREAD, 16);
					wsprintf(szTemp, " Original Network ID: %d Transport Stream ID: %d Service ID: %d\r\n", 
						     original_network_id, transport_stream_id, service_id);
					lstrcat(v->szSIFormatBuffer, szTemp);
				}
				if (cell_linkage_info == 0x03)
				{
					int original_network_id = get_bits(BM_USER_THREAD, 16);
					int transport_stream_id = get_bits(BM_USER_THREAD, 16);
					int service_id = get_bits(BM_USER_THREAD, 16);
					wsprintf(szTemp, " Original Network ID: %d Transport Stream ID: %d Service ID: %d\r\n", 
						     original_network_id, transport_stream_id, service_id);
					lstrcat(v->szSIFormatBuffer, szTemp);
				}
				if (cell_linkage_info == 0x03)
				{
					int original_network_id = get_bits(BM_USER_THREAD, 16);
					int transport_stream_id = get_bits(BM_USER_THREAD, 16);
					int service_id = get_bits(BM_USER_THREAD, 16);
					int event_id = get_bits(BM_USER_THREAD, 16);
					wsprintf(szTemp, " Original Network ID: %d Transport Stream ID: %d Service ID: %d Event ID: %d\r\n", 
						     original_network_id, transport_stream_id, service_id, event_id);
					lstrcat(v->szSIFormatBuffer, szTemp);
				}
			}
		}
		break;
	case 0x52:	// Stream Identifier Descriptor
		if (v->nNetworkPID != 0x0010)
			goto DecodeMPEG2Descriptor_Default;
		DecodeSIDescriptor(szTemp, pDescriptorData);
		lstrcat(v->szSIFormatBuffer, szTemp);
		break;
	case 0x53:	// CA descriptor
		if (v->nNetworkPID != 0x0010)
			goto DecodeMPEG2Descriptor_Default;
		DecodeCADescriptor(szTemp, pDescriptorData);
		lstrcat(v->szSIFormatBuffer, szTemp);
		break;
	case 0x54:	// content descriptor
		if (v->nNetworkPID != 0x0010)
			goto DecodeMPEG2Descriptor_Default;
		DecodeContentDescriptor(szTemp, pDescriptorData);
		lstrcat(v->szSIFormatBuffer, szTemp);
		break;
	case 0x55:	// parental rating descriptor
		if (v->nNetworkPID != 0x0010)
			goto DecodeMPEG2Descriptor_Default;
		DecodeParentalRatingDescriptor(szTemp, pDescriptorData);
		lstrcat(v->szSIFormatBuffer, szTemp);
		break;
	case 0x57:	// telephone descriptor
		if (v->nNetworkPID != 0x0010)
			goto DecodeMPEG2Descriptor_Default;
		set_buf(BM_USER_THREAD, pDescriptorData, 0, FALSE);
		{
			int i;
			int descriptor_tag = get_bits(BM_USER_THREAD, 8);
			int descriptor_length = get_bits(BM_USER_THREAD, 8);
			int reserved_future_use1 = get_bits(BM_USER_THREAD, 2);
			int foreign_availability = get_bits(BM_USER_THREAD, 1);
			int connection_type = get_bits(BM_USER_THREAD, 5);
			int reserved_future_use2 = get_bits(BM_USER_THREAD, 1);
			int country_prefix_length = get_bits(BM_USER_THREAD, 2);
			int international_area_code_length = get_bits(BM_USER_THREAD,3 );
			int operator_code_length = get_bits(BM_USER_THREAD, 2);
			int reserved_future_use3 = get_bits(BM_USER_THREAD, 1);
			int national_area_code_length = get_bits(BM_USER_THREAD, 3);
			int core_number_length = get_bits(BM_USER_THREAD, 4);
			char szCountryPrefix[64];
			char szInternationalAreaCode[64];
			char szOperatorCode[64];
			char szNationalAreaCode[64];
			char szCoreNumber[64];

			for (i = 0; i < country_prefix_length; i++)
				szCountryPrefix[i] = get_bits(BM_USER_THREAD, 8);
			szCountryPrefix[i] = '\0';

			for (i = 0; i < international_area_code_length; i++)
				szInternationalAreaCode[i] = get_bits(BM_USER_THREAD, 8);
			szInternationalAreaCode[i] = '\0';

			for (i = 0; i < operator_code_length; i++)
				szOperatorCode[i] = get_bits(BM_USER_THREAD, 8);
			szOperatorCode[i] = '\0';

			for (i = 0; i < national_area_code_length; i++)
				szNationalAreaCode[i] = get_bits(BM_USER_THREAD, 8);
			szNationalAreaCode[i] = '\0';

			for (i = 0; i < core_number_length; i++)
				szCoreNumber[i] = get_bits(BM_USER_THREAD, 8);
			szCoreNumber[i] = '\0';

			wsprintf(szTemp, " Foreign Availability: %s Connection Type: %d\r\n Number - International: %s Operator: %s Area: %s Core Number: %s\r\n",
				     TrueFalseString(foreign_availability), connection_type,
					 szCountryPrefix, szInternationalAreaCode, szNationalAreaCode, szCoreNumber);
			lstrcat(v->szSIFormatBuffer, szTemp);
		}
		break;
	case 0x58: // Local Time Offset Descriptor
		if (v->nNetworkPID != 0x0010)
			goto DecodeMPEG2Descriptor_Default;
		set_buf(BM_USER_THREAD, pDescriptorData, 0, FALSE);
		{
			int descriptor_tag = get_bits(BM_USER_THREAD, 8);
			int descriptor_length = get_bits(BM_USER_THREAD, 8);

			while (descriptor_length)
			{
				int country_code = get_bits(BM_USER_THREAD, 24);
				int country_region = get_bits(BM_USER_THREAD, 6);
				int reserved = get_bits(BM_USER_THREAD, 1);
				int local_time_offset_polarity = get_bits(BM_USER_THREAD, 1);
				int local_time_offset = get_bits(BM_USER_THREAD, 16);
				int time_of_change_mjd = get_bits(BM_USER_THREAD, 16);
				int time_of_change_utc = get_bits(BM_USER_THREAD, 24);
				int next_time_offset = get_bits(BM_USER_THREAD, 16);
				int nHour, nMinute;
				SYSTEMTIME st;
				char szRegion[32];
				char szTimeOfChange[128];
				char szLocalTimeOffset[32] = {""};
				char szNextTimeOffset[32] = {""};
				char szDate[64] = {""};
				char szTime[64] = {""};

				if (country_region == 0)
					lstrcpy(szRegion, "Not used");
				else if (country_region > 60)
					lstrcpy(szRegion, "Reserved");
				else
					wsprintf(szRegion, "Time Zone %d", country_region);

				memset(&st, 0, sizeof(st));
				ConvertDVBDate(time_of_change_mjd, (int *)&st.wYear, (int *)&st.wMonth, (int *)&st.wDay);
				ConvertDVBTime(time_of_change_utc, (int *)&st.wHour, (int *)&st.wMinute, (int *)&st.wSecond);
				GetDateFormat(LOCALE_SYSTEM_DEFAULT, DATE_SHORTDATE, &st, NULL, szDate, sizeof(szDate));
				GetTimeFormat(LOCALE_SYSTEM_DEFAULT, 0, &st, NULL, szTime, sizeof(szTime));
				wsprintf(szTimeOfChange, "%s %s", szDate, szTime);

				ConvertDVBBCDTimeOffsets(local_time_offset, &nHour, &nMinute);
				wsprintf(szLocalTimeOffset, "%02d:%02d", nHour, nMinute);

				ConvertDVBBCDTimeOffsets(next_time_offset, &nHour, &nMinute);
				wsprintf(szNextTimeOffset, "%02d:%02d", nHour, nMinute);

				wsprintf(szTemp, " Country Code %c%c%c Region %s\r\n Local Time Offset Polarity: %d Local Time Offset %s\r\n Time of change: %s Next Time Offset: %s\r\n",
					     country_code >> 16, country_code >> 8, country_code & 0xff, szRegion,
						 local_time_offset_polarity, szLocalTimeOffset,
						 szTimeOfChange, szNextTimeOffset);
				lstrcat(v->szSIFormatBuffer, szTemp);

				descriptor_length -= 13;
			}
		}
		break;
	case 0x59:	// subtitling descriptor
		if (v->nNetworkPID != 0x0010)
			goto DecodeMPEG2Descriptor_Default;
		break;
	case 0x5a:	// terrestrial delivery descriptor -- already done
		goto DecodeMPEG2Descriptor_Default;
	case 0x5b:	// multi lingual network name descriptor
		if (v->nNetworkPID != 0x0010)
			goto DecodeMPEG2Descriptor_Default;
		set_buf(BM_USER_THREAD, pDescriptorData, 0, FALSE);
		{
			int descriptor_tag = get_bits(BM_USER_THREAD, 8);
			int descriptor_length = get_bits(BM_USER_THREAD, 8);

			while (descriptor_length)
			{
				int j;
				char szNetworkName[128];

				int lc1 = get_bits(BM_USER_THREAD, 8);
				int lc2 = get_bits(BM_USER_THREAD, 8);
				int lc3 = get_bits(BM_USER_THREAD, 8);
				int network_name_length = get_bits(BM_USER_THREAD, 8);
				for (j = 0; j < network_name_length; j++)
					szNetworkName[j] = get_bits(BM_USER_THREAD, 8);
				szNetworkName[j] = '\0';

				wsprintf(szTemp, " Language code: %c%c%c Network Name: %s\r\n",
					     lc1, lc2, lc3, szNetworkName);
				lstrcat(v->szSIFormatBuffer, szTemp);

				descriptor_length -= (4 + network_name_length);
			}
		}
		break;
	case 0x5c:	// multi lingual bouquet name descriptor
		if (v->nNetworkPID != 0x0010)
			goto DecodeMPEG2Descriptor_Default;
		set_buf(BM_USER_THREAD, pDescriptorData, 0, FALSE);
		{
			int descriptor_tag = get_bits(BM_USER_THREAD, 8);
			int descriptor_length = get_bits(BM_USER_THREAD, 8);

			while (descriptor_length)
			{
				int lc1 = get_bits(BM_USER_THREAD, 8);
				int lc2 = get_bits(BM_USER_THREAD, 8);
				int lc3 = get_bits(BM_USER_THREAD, 8);
				int subtitling_type = get_bits(BM_USER_THREAD, 8);
				int composition_page_id = get_bits(BM_USER_THREAD, 16);
				int ancillary_page_id = get_bits(BM_USER_THREAD, 16);

				wsprintf(szTemp, " Language: %c%c%c Subtitling Type: %d Composition Page: %d Ancillary Page: %d\r\n",
					     lc1, lc2, lc3, subtitling_type, composition_page_id, ancillary_page_id);
				lstrcat(v->szSIFormatBuffer, szTemp);

				descriptor_length -= 8;
			}
		}
		break;
	case 0x5d:	// multi lingual service name descriptor
		if (v->nNetworkPID != 0x0010)
			goto DecodeMPEG2Descriptor_Default;
		set_buf(BM_USER_THREAD, pDescriptorData, 0, FALSE);
		{
			int descriptor_tag = get_bits(BM_USER_THREAD, 8);
			int descriptor_length = get_bits(BM_USER_THREAD, 8);

			while (descriptor_length)
			{
				int lc1 = get_bits(BM_USER_THREAD, 8);
				int lc2 = get_bits(BM_USER_THREAD, 8);
				int lc3 = get_bits(BM_USER_THREAD, 8);
				int service_provider_name_length = get_bits(BM_USER_THREAD, 8);
				int service_name_length;
				int j;
				char szServiceProviderName[64];
				char szServiceName[64];

				for (j = 0; j < service_provider_name_length; j++)
					szServiceProviderName[j] = get_bits(BM_USER_THREAD, 8);
				szServiceProviderName[j] = '\0';

				service_name_length = get_bits(BM_USER_THREAD, 8);
				for (j = 0; j < service_name_length; j++)
					szServiceName[j] = get_bits(BM_USER_THREAD, 8);
				szServiceName[j] = '\0';

				wsprintf(szTemp, " Language: %c%c%c Service Provider: %s Service: %s\r\n",
					     lc1, lc2, lc3,
						 szServiceProviderName,
						 szServiceName);
				lstrcat(v->szSIFormatBuffer, szTemp);

				descriptor_length -= 5 + service_provider_name_length + service_name_length;
			}
		}
		break;
	case 0x5e:	// multi lingual component name descriptor
		if (v->nNetworkPID != 0x0010)
			goto DecodeMPEG2Descriptor_Default;
		DecodeMultilingualComponentDescriptor(szTemp, pDescriptorData);
		lstrcat(v->szSIFormatBuffer, szTemp);
		break;
	case 0x5f:	// private_data_specifier_descriptor
		if (v->nNetworkPID != 0x0010)
			goto DecodeMPEG2Descriptor_Default;
		{
			char * szDescriptorOwner;
			szDescriptorOwner = DecodePrivateDataSpecifierDescriptor(pDescriptorData);
			wsprintf(szTemp, " Private Data Specifier: %s\r\n", szDescriptorOwner);
			lstrcat(v->szSIFormatBuffer, szTemp);
		}
		break;
	case 0x60:	// service move descriptor
		if (v->nNetworkPID != 0x0010)
			goto DecodeMPEG2Descriptor_Default;
		set_buf(BM_USER_THREAD, pDescriptorData, 0, FALSE);
		{
			int descriptor_tag = get_bits(BM_USER_THREAD, 8);
			int descriptor_length = get_bits(BM_USER_THREAD, 8);
			int new_original_network_id = get_bits(BM_USER_THREAD, 16);
			int new_transport_stream_id = get_bits(BM_USER_THREAD, 16);
			int new_service_id = get_bits(BM_USER_THREAD, 16);

			wsprintf(szTemp, " New Original Network: %d Transport Stream: %d Service ID: %d\r\n",
				     new_original_network_id, new_transport_stream_id, new_service_id);
			lstrcat(v->szSIFormatBuffer, szTemp);
		}
		break;
	case 0x61:	// short smoothing buffer descriptor
		if (v->nNetworkPID != 0x0010)
			goto DecodeMPEG2Descriptor_Default;
		set_buf(BM_USER_THREAD, pDescriptorData, 0, FALSE);
		{
			int descriptor_tag = get_bits(BM_USER_THREAD, 8);
			int descriptor_length = get_bits(BM_USER_THREAD, 8);
			int sb_size = get_bits(BM_USER_THREAD, 2);
			int sb_leak_rate = get_bits(BM_USER_THREAD, 6);
			char szSBSize[32];

			switch(sb_size)
			{
			case 1:
				lstrcpy(szSBSize, "1536");
				break;
			default:
				lstrcpy(szSBSize, "Reserved");
				break;
			}

			sprintf(szTemp, " Buffer Size: %s Leak Rate: %.3f Mbps\r\n",
				    szSBSize, DecodeDVBLeakRate(sb_leak_rate));
			lstrcat(v->szSIFormatBuffer, szTemp);
		}
		break;
	case 0x62:		// frequency_list_descriptor
		if (v->nNetworkPID != 0x0010)
			goto DecodeMPEG2Descriptor_Default;
		set_buf(BM_USER_THREAD, pDescriptorData, 0, FALSE);
		{
			int j;
			int descriptor_tag = get_bits(BM_USER_THREAD, 8);
			int descriptor_length = get_bits(BM_USER_THREAD, 8);
			int reserved_future_use = get_bits(BM_USER_THREAD, 6);
			int coding_type = get_bits(BM_USER_THREAD, 2);
			
			for (j = 0; j < descriptor_length - 1; j += 4)
			{
				char szTemp2[128];
				DWORD centre_frequency = get_bits(BM_USER_THREAD, 32);
				float fCenterFrequency;
				switch(coding_type)
				{
				default:	// not defined
					szTemp2[0] = '\0';
					break;
				case 1:	// satellite
					fCenterFrequency = (float)ConvertBCD(centre_frequency);
					sprintf(szTemp2, " Satellite: %.3f MHz\r\n", fCenterFrequency / 1000.0f);
					break;
				case 2:	// cable
					fCenterFrequency = (float)ConvertBCD(centre_frequency);
					sprintf(szTemp2, " Cable: %.3f MHz\r\n", fCenterFrequency / 10000.0f);
					break;
				case 3:	// terrestrial
					fCenterFrequency = (float)centre_frequency;
					sprintf(szTemp2, " Terrestrial: %.3f MHz\r\n", fCenterFrequency / 100000.0f);
					break;
				}
				if (lstrlen(szTemp2))
					lstrcat(szTemp, szTemp2);
			}
			lstrcat(v->szSIFormatBuffer, szTemp);
		}
		break;
	case 0x63:	// partial transport stream descriptor
		if (v->nNetworkPID != 0x0010)
			goto DecodeMPEG2Descriptor_Default;
		set_buf(BM_USER_THREAD, pDescriptorData, 0, FALSE);
		{
			int descriptor_tag = get_bits(BM_USER_THREAD, 8);
			int descriptor_length = get_bits(BM_USER_THREAD, 8);
			int DVB_reserved_future_use1 = get_bits(BM_USER_THREAD, 2);
			int peak_rate = get_bits(BM_USER_THREAD, 22);
			int DVB_reserved_future_use2 = get_bits(BM_USER_THREAD, 2);
			int minimum_overall_smoothing_rate= get_bits(BM_USER_THREAD, 22);
			int DVB_reserved_future_use3 = get_bits(BM_USER_THREAD, 2);
			int maximum_overall_smoothing_buffer = get_bits(BM_USER_THREAD, 14);
			
			wsprintf(szTemp, " Peak rate: %d Minimum Overall Smoothing: %d Max: %d\r\n",
				     peak_rate, minimum_overall_smoothing_rate, maximum_overall_smoothing_buffer);
			lstrcat(v->szSIFormatBuffer, szTemp);
		}
		break;
	case 0x64:	// data broadcast descriptor
		if (v->nNetworkPID != 0x0010)
			goto DecodeMPEG2Descriptor_Default;
		set_buf(BM_USER_THREAD, pDescriptorData, 0, FALSE);
		{
			int lc1, lc2, lc3;
			int text_length;
			int i;
			char text_char[128];
			char szSelectorBytes[1024] = {""};

			int descriptor_tag = get_bits(BM_USER_THREAD, 8);
			int descriptor_length = get_bits(BM_USER_THREAD, 8);
			int data_broadcast_id = get_bits(BM_USER_THREAD, 16);
			int component_tag = get_bits(BM_USER_THREAD, 8);
			int selector_length = get_bits(BM_USER_THREAD, 8);
			for (i = 0; i < selector_length; i++)
			{
				char szTemp2[16];
				wsprintf(szTemp2, "%02x ", get_bits(BM_USER_THREAD, 8));
				lstrcat(szSelectorBytes, szTemp2);
			}

			lc1 = get_bits(BM_USER_THREAD, 8);
			lc2 = get_bits(BM_USER_THREAD, 8);
			lc3 = get_bits(BM_USER_THREAD, 8);
			text_length = get_bits(BM_USER_THREAD, 8);
			for (i = 0; i < text_length; i++)
				text_char[i] = get_bits(BM_USER_THREAD, 8);
			text_char[i] = '\0';

			wsprintf(szTemp, " Data Broadcast ID: %d Component tag: %d Language: %c%c%c\r\n Text: %s Selector Bytes: %s\r\n",
				     data_broadcast_id, component_tag, lc1, lc2, lc3, text_char, szSelectorBytes);
			lstrcat(v->szSIFormatBuffer, szTemp);
		}
		break;
	case 0x65:	// CA system descriptor -- defined in DAVIC
		goto DecodeMPEG2Descriptor_Default;
	case 0x66:	// data broadcast ID descriptor
		if (v->nNetworkPID != 0x0010)
			goto DecodeMPEG2Descriptor_Default;
		set_buf(BM_USER_THREAD, pDescriptorData, 0, FALSE);
		{
			int i = 0;
			char szSelectorBytes[1024] = {""};

			int descriptor_tag = get_bits(BM_USER_THREAD, 8);
			int descriptor_length = get_bits(BM_USER_THREAD, 8);
			int data_broadcast_id = get_bits(BM_USER_THREAD, 16);
			descriptor_length -= 2;
			while (descriptor_length)
			{
				char szTemp2[16];
				wsprintf(szTemp2, "%02x ", get_bits(BM_USER_THREAD, 8));
				lstrcat(szSelectorBytes, szTemp2);
				descriptor_length--;
			}

			wsprintf(szTemp, " Data Broadcast ID: %d Selector Bytes: %s\r\n",
				     data_broadcast_id, szSelectorBytes);
			lstrcat(v->szSIFormatBuffer, szTemp);
		}
		break;
	case 0x67:	// transport stream descriptor -- todo
		//if (v->nNetworkPID != 0x0010)
			goto DecodeMPEG2Descriptor_Default;
/*
descriptor_tag 8 uimsbf
descriptor_length 8 uimsbf
for (i=0;i<N;i++){
byte 8 uimsbf
*/		
		break;
	case 0x68:	// DSNG descriptor -- todo
		//if (v->nNetworkPID != 0x0010)
			goto DecodeMPEG2Descriptor_Default;
/*
descriptor_tag 8 uimsbf
descriptor_length 8 uimsbf
for (i=0;i<N;i++) {
byte 8 uimsbf
*/		
		break;
	case 0x69:	// PDC descriptor
		if (v->nNetworkPID != 0x0010)
			goto DecodeMPEG2Descriptor_Default;
		set_buf(BM_USER_THREAD, pDescriptorData, 0, FALSE);
		{
			int descriptor_tag = get_bits(BM_USER_THREAD, 8);
			int descriptor_length = get_bits(BM_USER_THREAD, 8);
			int reserved = get_bits(BM_USER_THREAD, 4);
			int day = get_bits(BM_USER_THREAD, 5);
			int month = get_bits(BM_USER_THREAD, 4);
			int hour = get_bits(BM_USER_THREAD, 5);
			int minute = get_bits(BM_USER_THREAD, 6);

			wsprintf(szTemp, " Day: %d Month: %d Hour: %d Minute: %d\r\n",
				     day, month, hour, minute);
			lstrcat(v->szSIFormatBuffer, szTemp);
		}
		break;
	case 0x6a:	// AC3 audio descriptor
		if (v->nNetworkPID != 0x0010)
			goto DecodeMPEG2Descriptor_Default;
		set_buf(BM_USER_THREAD, pDescriptorData, 0, FALSE);
		{
			int AC3_type, bsid, mainid, asvc;

			int descriptor_tag = get_bits(BM_USER_THREAD, 8);
			int descriptor_length = get_bits(BM_USER_THREAD, 8);
			int AC3_type_flag = get_bits(BM_USER_THREAD, 1);
			int bsid_flag = get_bits(BM_USER_THREAD, 1);
			int mainid_flag = get_bits(BM_USER_THREAD, 1);
			int asvc_flag = get_bits(BM_USER_THREAD, 1);
			int reserved = get_bits(BM_USER_THREAD, 4);
			if (AC3_type_flag == 1)
				AC3_type = get_bits(BM_USER_THREAD, 8);
			if (bsid_flag == 1)
				bsid = get_bits(BM_USER_THREAD, 8);
			if (mainid_flag == 1)
				mainid = get_bits(BM_USER_THREAD, 8);
			if (asvc_flag == 1)
				asvc = get_bits(BM_USER_THREAD, 8);

			wsprintf(szTemp, " Flags: AC3 Type: %s BSID: %s Main ID: %s ASVC: %s\r\n",
				     TrueFalseString(AC3_type_flag), TrueFalseString(bsid_flag),
					 TrueFalseString(mainid_flag), TrueFalseString(asvc_flag));
			lstrcat(v->szSIFormatBuffer, szTemp);

			if (AC3_type_flag == 1)
			{
				wsprintf(szTemp, " AC3 Type: %d\r\n", AC3_type);
				lstrcat(v->szSIFormatBuffer, szTemp);
			}
			if (bsid_flag == 1)
			{
				wsprintf(szTemp, " BSID: %d\r\n", bsid);
				lstrcat(v->szSIFormatBuffer, szTemp);
			}
			if (mainid_flag == 1)
			{
				wsprintf(szTemp, " Main ID: %d\r\n", mainid);
				lstrcat(v->szSIFormatBuffer, szTemp);
			}
			if (asvc_flag == 1)
			{
				wsprintf(szTemp, " ASVC: %d\r\n", asvc);
				lstrcat(v->szSIFormatBuffer, szTemp);
			}
		}
		break;
	case 0x6b:	// ancilliary data descriptor
		if (v->nNetworkPID != 0x0010)
			goto DecodeMPEG2Descriptor_Default;
		DecodeAncilliaryDataDescriptor(szTemp, pDescriptorData);
		lstrcat(v->szSIFormatBuffer, szTemp);
		break;
	case 0x6c:	// cell list descriptor
		if (v->nNetworkPID != 0x0010)
			goto DecodeMPEG2Descriptor_Default;
		set_buf(BM_USER_THREAD, pDescriptorData, 0, FALSE);
		{
			int descriptor_tag = get_bits(BM_USER_THREAD, 8);
			int descriptor_length = get_bits(BM_USER_THREAD, 8);

			while (descriptor_length)
			{
				int cell_id = get_bits(BM_USER_THREAD, 16);
				int cell_latitude = get_bits(BM_USER_THREAD, 16);
				int cell_longitude = get_bits(BM_USER_THREAD, 16);
				int cell_extend_of_latitude = get_bits(BM_USER_THREAD, 12);
				int cell_extend_of_longitude = get_bits(BM_USER_THREAD, 12);
				int subcell_info_loop_length = get_bits(BM_USER_THREAD, 8);
				descriptor_length -= 10;

				wsprintf(szTemp, " Cell ID: %d Latitude: %d Longitude: %d Extend Latitude: %d Extend Longitude: %d\r\n",
					     cell_id, cell_latitude, cell_longitude,
						 cell_extend_of_latitude, cell_extend_of_longitude);
				lstrcat(v->szSIFormatBuffer, szTemp);

				while (subcell_info_loop_length)
				{
					int cell_id_extension = get_bits(BM_USER_THREAD, 8);
					int subcell_latitude = get_bits(BM_USER_THREAD, 16);
					int subcell_longitude = get_bits(BM_USER_THREAD, 16);
					int subcell_extend_of_latitude = get_bits(BM_USER_THREAD, 12);
					int subcell_extend_of_longitude = get_bits(BM_USER_THREAD, 12);
					
					subcell_info_loop_length -= 8;
					descriptor_length -= 8;

					wsprintf(szTemp, " Subcell ID: %d Longitude: %d Extend Latitude: %d Extend Longitude: %d\r\n",
						     cell_id_extension, subcell_latitude, subcell_longitude,
							 subcell_extend_of_latitude, subcell_extend_of_longitude);
					lstrcat(v->szSIFormatBuffer, szTemp);
				}
			}
		}
		break;
	case 0x6d:	// cell frequency link descriptor
		if (v->nNetworkPID != 0x0010)
			goto DecodeMPEG2Descriptor_Default;
		set_buf(BM_USER_THREAD, pDescriptorData, 0, FALSE);
		{
			int descriptor_tag = get_bits(BM_USER_THREAD, 8);
			int descriptor_length = get_bits(BM_USER_THREAD, 8);

			szTemp[0] = '\0';
			while (descriptor_length)
			{
				int cell_id = get_bits(BM_USER_THREAD, 16);
				DWORD frequency = get_bits(BM_USER_THREAD, 32);
				int subcell_info_loop_length = get_bits(BM_USER_THREAD, 8);
				char szTemp2[128];

				sprintf(szTemp2, " Cell ID: %d Frequency: %.3f MHz\r\n", cell_id, (double)frequency / 100000.0);
				lstrcat(szTemp, szTemp2);
				while (subcell_info_loop_length)
				{
					int cell_id_extension = get_bits(BM_USER_THREAD, 8);
					DWORD transposer_frequency = get_bits(BM_USER_THREAD, 32);
					sprintf(szTemp2, "  Cell ID extension: %d Frequency: %.3f MHz\r\n", cell_id_extension, (double)transposer_frequency / 100000.0);
					lstrcat(szTemp, szTemp2);

					subcell_info_loop_length -= 5;
					descriptor_length -= 5;
				}
				descriptor_length -= 7;
			}
			lstrcat(v->szSIFormatBuffer, szTemp);
		}
		break;
	case 0x6e:	// announcement support descriptor
		if (v->nNetworkPID != 0x0010)
			goto DecodeMPEG2Descriptor_Default;
		DecodeAnnouncementSupportDescriptor(szTemp, pDescriptorData);
		lstrcat(v->szSIFormatBuffer, szTemp);
		break;
	case 0x77:	// Time Slice FEC Identifier descriptor
		if (v->nNetworkPID != 0x0010)
			goto DecodeMPEG2Descriptor_Default;
		set_buf(BM_USER_THREAD, pDescriptorData, 0, FALSE);
		{
			int descriptor_tag = get_bits(BM_USER_THREAD, 8);
			int descriptor_length = get_bits(BM_USER_THREAD, 8);
			int time_slicing = get_bits(BM_USER_THREAD, 1);
			int mpe_fec = get_bits(BM_USER_THREAD, 2);
			int reserved_for_future_use = get_bits(BM_USER_THREAD, 2);
			int frame_size = get_bits(BM_USER_THREAD, 3);
			int max_burst_duration = get_bits(BM_USER_THREAD, 8);
			int max_average_rate = get_bits(BM_USER_THREAD, 4);
			int time_slice_fec_id = get_bits(BM_USER_THREAD, 4);
			int nMaxAverageRate = -1;
			int nMaxBurstDuration;
			char * szMPE_FEC;
			char * szFrameSize;
			char szMaxAverageRate[64];
			char szIDSelectorBytes[1024] = {""};

			descriptor_length -= 3;		// taken three bytes so far
			if (descriptor_length)
				lstrcpy(szIDSelectorBytes, " ");
			while (descriptor_length)
			{
				BYTE id_selector_byte = get_bits(BM_USER_THREAD, 8);
				char szTemp2[16];
				wsprintf(szTemp2, "%02x ", id_selector_byte);
				lstrcat(szIDSelectorBytes, szTemp2);
				descriptor_length--;
			}
			if (lstrlen(szIDSelectorBytes))
				lstrcat(szIDSelectorBytes, "\r\n");

			switch(mpe_fec)
			{
			case 0:
				szMPE_FEC = "MPE-FEC not used";
				break;
			case 1:
				szMPE_FEC = "MPE-FEC Reed-Solomon(255, 191, 64)";
				break;
			default:
				szMPE_FEC = "Reserved";
				break;
			}
			switch(frame_size)
			{
			case 0:
				szFrameSize = "512kb 256 rows";
				break;
			case 1:
				szFrameSize = "1024kb 512 rows";
				break;
			case 2:
				szFrameSize = "1536kb 768 rows";
				break;
			case 3:
				szFrameSize = "2048kb 1024 rows";
				break;
			default:
				szFrameSize = "Reserved";
				break;
			}
			switch(max_average_rate)
			{
			case 0:
				nMaxAverageRate = 16;
				break;
			case 1:
				nMaxAverageRate = 32;
				break;
			case 2:
				nMaxAverageRate = 64;
				break;
			case 3:
				nMaxAverageRate = 128;
				break;
			case 4:
				nMaxAverageRate = 256;
				break;
			case 5:
				nMaxAverageRate = 512;
				break;
			case 6:
				nMaxAverageRate = 1024;
				break;
			case 7:
				nMaxAverageRate = 2048;
				break;
			}
			if (nMaxAverageRate == -1)
				lstrcpy(szMaxAverageRate, "reserved");
			else
				wsprintf(szMaxAverageRate, "%dkbps", nMaxAverageRate);
			nMaxBurstDuration = (max_burst_duration + 1) * 20;

			wsprintf(szTemp, " %s Frame size: %s\r\n Max burst duration: %d ms Max avg. rate: %s\r\n Time slice FEC id: %d\r\n%s",
				     szMPE_FEC,
					 szFrameSize,
					 nMaxBurstDuration,
					 szMaxAverageRate,
					 time_slice_fec_id,
					 szIDSelectorBytes);
			lstrcat(v->szSIFormatBuffer, szTemp);
		}
		break;
	case 0x7b:	// DTS Audio Descriptor
		DecodeDTSAudioDescriptor(pDescriptorData, szTemp);
		lstrcat(v->szSIFormatBuffer, szTemp);
		break;
	case 0x81:		// ATSC AC3 audio descriptor
		if (v->nNetworkPID == 0x1ffb)
		{
			DecodeATSCAC3Descriptor(szTemp, pDescriptorData);
			lstrcat(v->szSIFormatBuffer, szTemp);
		}
		else
			goto DecodeMPEG2Descriptor_Default;
		break;
	case 0x82:	// DCII frame rate descriptor
		if (v->nNetworkPID != 0x0ffe)
			goto DecodeMPEG2Descriptor_Default;
		set_buf(BM_USER_THREAD, pDescriptorData, 0, FALSE);
		{
			int descriptor_tag = get_bits(BM_USER_THREAD, 8);
			int descriptor_length = get_bits(BM_USER_THREAD, 8);
			int multiple_frame_rate_flag = get_bits(BM_USER_THREAD, 1);
			int frame_rate_code = get_bits(BM_USER_THREAD, 4);
			int reserved = get_bits(BM_USER_THREAD, 3);
			char szFrameRate[128];

			DecodeFrameRate(szFrameRate, frame_rate_code);
			
			wsprintf(szTemp, " Multiple frame rate flag: %s Frame Rate: %s\r\n",
				     TrueFalseString(multiple_frame_rate_flag), szFrameRate);
			lstrcat(v->szSIFormatBuffer, szTemp);
		}
		break;
	case 0x83:		// LCN
		if (v->nNetworkPID == 0x0010)
		{
			set_buf(BM_USER_THREAD, pDescriptorData, 0, FALSE);
			{
				int j;
				int descriptor_tag = get_bits(BM_USER_THREAD, 8);
				int descriptor_length = get_bits(BM_USER_THREAD, 8);
				for (j = 0; j < descriptor_length; j += 4)
				{
					int service_id = get_bits(BM_USER_THREAD, 16);
					int visible_service_flag = get_bits(BM_USER_THREAD, 1);
					int reserved = get_bits(BM_USER_THREAD, 5);
					int logical_channel_number = get_bits(BM_USER_THREAD, 10);
					char szServiceID[128];

					if (logical_channel_number)
					{
						char szTemp2[128];

						if (v->pChannelData[service_id] != NULL)
							wsprintf(szServiceID, "%d (%s)", service_id, v->pChannelData[service_id]->szShortName);
						else
							wsprintf(szServiceID, "%d", service_id);

						wsprintf(szTemp2, " Logical channel %d = MPEG service %s\r\n",
							logical_channel_number, szServiceID);
						lstrcat(szTemp, szTemp2);
					}
				}
				lstrcat(v->szSIFormatBuffer, szTemp);
			}
		}
		else if (v->nNetworkPID == 0x0ffe)
		{
			// DCII extended video descriptor
			set_buf(BM_USER_THREAD, pDescriptorData, 0, FALSE);
			{
				int descriptor_tag = get_bits(BM_USER_THREAD, 8);
				int descriptor_length = get_bits(BM_USER_THREAD, 8);
				int catalog_mode_flag = get_bits(BM_USER_THREAD, 1);
				int video_includes_setup = get_bits(BM_USER_THREAD, 1);
				int reserved = get_bits(BM_USER_THREAD, 6);

				wsprintf(szTemp, " Catalog Mode: %s Video Includes Setup: %s\r\n",
					     TrueFalseString(catalog_mode_flag), TrueFalseString(video_includes_setup));
				lstrcat(v->szSIFormatBuffer, szTemp);
			}
		}
		else
			goto DecodeMPEG2Descriptor_Default;
		break;
	case 0x84:	// DCII component name descriptor
		if (v->nNetworkPID != 0x0ffe)
			goto DecodeMPEG2Descriptor_Default;
		set_buf(BM_USER_THREAD, pDescriptorData, 0, FALSE);
		{
			int descriptor_tag = get_bits(BM_USER_THREAD, 8);
			int descriptor_length = get_bits(BM_USER_THREAD, 8);
			int reserved = get_bits(BM_USER_THREAD, 2);
			int string_count = get_bits(BM_USER_THREAD, 6);
			int i;

			for (i = 0; i < string_count; i++)
			{
				int j;
				int lc1 = get_bits(BM_USER_THREAD, 8);
				int lc2 = get_bits(BM_USER_THREAD, 8);
				int lc3 = get_bits(BM_USER_THREAD, 8);
				int string_length = get_bits(BM_USER_THREAD, 8);
				char name_string[128];
				
				for (j = 0; j < string_length; j++)
					name_string[j] = get_bits(BM_USER_THREAD, 8);
				name_string[j] = '\0';

				wsprintf(szTemp, " Langauge Code: %c%c%c Component String: %s\r\n",
					     lc1, lc2, lc3, name_string);
				lstrcat(v->szSIFormatBuffer, szTemp);
			}
		}
		break;
	case 0x86:	// ATSC caption service descriptor
		if (v->nNetworkPID == 0x1ffb)
		{
			DecodeATSCCaptionServceDescriptor(szTemp, pDescriptorData);
			lstrcat(v->szSIFormatBuffer, szTemp);
		}
		else
			goto DecodeMPEG2Descriptor_Default;
		break;
	case 0x87:	// ATSC content advisory descriptor
		if (v->nNetworkPID == 0x1ffb)
		{
			DecodeATSCContentAdvisoryDescriptor(szTemp, pDescriptorData, FALSE);
			lstrcat(v->szSIFormatBuffer, szTemp);
		}
		else
			goto DecodeMPEG2Descriptor_Default;
		break;
	case 0xa0:	// ATSC Extended Channel Name Descriptor
		if (v->nNetworkPID != 0x1ffb)
			goto DecodeMPEG2Descriptor_Default;
		{
			int number_strings = pDescriptorData[2];
			int i;
			BYTE * pDescriptor = &pDescriptorData[3];
			char szExtendedName[256] = {0};

			for (i = 0; i < number_strings; i++)
			{
				int number_segments;
				int j;

				pDescriptor += 3; // skip the language_code
				number_segments = *pDescriptor++;
				for (j = 0; j < number_segments; j++)
				{
					int compression_type = *pDescriptor++;
					int mode = *pDescriptor++;
					int number_bytes = *pDescriptor++;
					if (compression_type == 0) // uncompressed
					{
						int k;

						for (k = 0; k < number_bytes; k++)
							szExtendedName[k] = *pDescriptor++;
						szExtendedName[k] = 0;
					}
					else
					{
						lstrcpy(szExtendedName, "[Unsupported Huffman coding]");
						pDescriptor += number_bytes;
					}
				}
			}
			wsprintf(szTemp, "Extended Channel Name: %s\r\n", szExtendedName);
			lstrcat(v->szSIFormatBuffer, szTemp);
		}
		break;
	case 0xa1:	// ATSC Service Location Descriptor
		if (v->nNetworkPID != 0x1ffb)
			goto DecodeMPEG2Descriptor_Default;
		{
			int n;
			int PCR_PID = (pDescriptorData[2] << 8 | pDescriptorData[3]) & 0x1fff;
			int number_elements = pDescriptorData[4];
			BYTE * pElementPtr = &pDescriptorData[5];
			char szMask[256];
			
			lstrcat(v->szSIFormatBuffer, "Service Location Descriptor:\r\n");
			wsprintf(szMask, " PCR PID %s\r\n", v->szOutputPIDFlags);
			wsprintf(szTemp, szMask, PCR_PID);
			lstrcat(v->szSIFormatBuffer, szTemp);
			
			for (n = 0; n < number_elements; n++)
			{
				int stream_type = pElementPtr[0];
				int elementary_PID = (pElementPtr[1] << 8 | pElementPtr[2]) & 0x1fff;
				char ISO_639_language_code[4];
				char szEnglishStreamType[64] = {0};
				memcpy(ISO_639_language_code, &pElementPtr[3], 3);
				ISO_639_language_code[3] = 0;

				DecodeStreamType(stream_type, szEnglishStreamType, -1, -1);
				wsprintf(szMask, " Stream Type 0x%02x (%s) ESPID = %s Language = %s\r\n", stream_type, szEnglishStreamType, v->szOutputPIDFlags, ISO_639_language_code);
				wsprintf(szTemp, szMask, elementary_PID);
				lstrcat(v->szSIFormatBuffer, szTemp);
				
				pElementPtr += 6;
			}
		}				
		break;
	case 0xa2:	// ATSC time shifted service descriptor
		if (v->nNetworkPID != 0x1ffb)
			goto DecodeMPEG2Descriptor_Default;
		set_buf(BM_USER_THREAD, pDescriptorData, 0, FALSE);
		{
			int i;

			int descriptor_tag = get_bits(BM_USER_THREAD, 8);
			int descriptor_length = get_bits(BM_USER_THREAD, 8);
			int reserved1 = get_bits(BM_USER_THREAD, 3);
			int number_of_services = get_bits(BM_USER_THREAD, 5);
			
			for (i = 0; i < number_of_services; i++)
			{
				int reserved2 = get_bits(BM_USER_THREAD, 6);
				int time_shift = get_bits(BM_USER_THREAD, 10);
				int major_channel_number = get_bits(BM_USER_THREAD, 10);
				int minor_channel_number = get_bits(BM_USER_THREAD, 10);
				
				wsprintf(szTemp, " Time Shift: %d minutes Channel: %d.%d\r\n",
					     time_shift, major_channel_number, minor_channel_number);
				lstrcat(v->szSIFormatBuffer, szTemp);
			}
		}
		break;
	case 0xa3:	// ATSC component name descriptor
		if (v->nNetworkPID != 0x1ffb)
			goto DecodeMPEG2Descriptor_Default;
		set_buf(BM_USER_THREAD, pDescriptorData, 0, FALSE);
		{
			char component_name[128] = {0};

			int descriptor_tag = get_bits(BM_USER_THREAD, 8);
			int descriptor_length = get_bits(BM_USER_THREAD, 8);

			GetATSCMultipleString(BM_USER_THREAD, component_name, descriptor_length);
			wsprintf(szTemp, " Component Name: %s\r\n", component_name);
			lstrcat(v->szSIFormatBuffer, szTemp);
		}
		break;

	case 0xc1:	// ISDB Digital Copy Control
		if (!v->fISDB)
			goto DecodeMPEG2Descriptor_Default;
		set_buf(BM_USER_THREAD, pDescriptorData, 0, FALSE);
		{
			const char *cci_info1[] = { "Copy can be made without control condition", "Copy forbidden", "Copy can be made for only one generation", "Copy is forbidden" };
			const char *cci_info3[] = { "Copy can be made without control condition", "Not used", "Copy can be made for only one generation", "Copy is forbidden" };
			const char *digital_cci_info[] = { "Undefined", "Output with encryption to serial interface", "Undefined", "Output without encryption to serial interface" };
			const char *analog_cci_info[] = { "Can be copied without control condition", "With pseudo-sync pulse", "Pseudo-sync pulse + 2-line reversed division burst insertion", "Pseudo-sync pulse + 4-line reversed division burst insertion" };

			int descriptor_tag = get_bits(BM_USER_THREAD, 8);
			int descriptor_length = get_bits(BM_USER_THREAD, 8);
			uint8_t digital_recording_control_data = (get_bits(BM_USER_THREAD, 2) & 3);
			uint8_t maximum_bitrate_flag = get_bits(BM_USER_THREAD, 1);
			uint8_t component_control_flag = get_bits(BM_USER_THREAD, 1);
			uint8_t copy_control_type = get_bits(BM_USER_THREAD, 2);
			uint8_t aps_control_data = 0xff;
			if (copy_control_type == 1)
				aps_control_data = get_bits(BM_USER_THREAD, 2) & 3;
			else
				get_bits(BM_USER_THREAD, 2); /* reserved */

			uint8_t maximum_bitrate = 0;
			if (maximum_bitrate_flag)
				maximum_bitrate = get_bits(BM_USER_THREAD, 8);

			if (copy_control_type == 1)
				wsprintf(szTemp, " Recording control: %d (%s)\r\n", digital_recording_control_data, cci_info1[digital_recording_control_data]);
			else
				wsprintf(szTemp, " Recording control: %d (%s)\r\n", digital_recording_control_data, cci_info3[digital_recording_control_data]);
			lstrcat(v->szSIFormatBuffer, szTemp);

			wsprintf(szTemp, " Digital copy control type 0x%1x (%d) - %s\r\n", copy_control_type, copy_control_type, digital_cci_info[copy_control_type]);
			lstrcat(v->szSIFormatBuffer, szTemp);

			if (aps_control_data != 0xff) {
				wsprintf(szTemp, " Analog copy control type 0x%1x (%d) - %s\r\n", aps_control_data, aps_control_data, analog_cci_info[aps_control_data]);
				lstrcat(v->szSIFormatBuffer, szTemp);
			}

			if (maximum_bitrate_flag) {
				StringCchPrintf(szTemp, sizeof(szTemp), " Maximum bitrate: %2.2f Mbps\r\n", maximum_bitrate / 5.0f);
				lstrcat(v->szSIFormatBuffer, szTemp);
			}
			/* TODO the rest of descriptor */
		}
		break;

	case 0xc8:	// ISDB Video Decode Control
		if (!v->fISDB)
			goto DecodeMPEG2Descriptor_Default;
		set_buf(BM_USER_THREAD, pDescriptorData, 0, FALSE);
		{
			const char *isdb_video_encode_format[] = { "1080p", "1080i", "720p", "480p", "480i", "240p", "120p", "Reserved", "For extension of video encode format" };

			int descriptor_tag = get_bits(BM_USER_THREAD, 8);
			int descriptor_length = get_bits(BM_USER_THREAD, 8);
			uint8_t still_picture_flag = get_bits(BM_USER_THREAD, 1) & 1;
			uint8_t sequence_end_code_flag = get_bits(BM_USER_THREAD, 1) & 1;
			uint8_t video_encode_format = get_bits(BM_USER_THREAD, 4) & 0xf;

			wsprintf(szTemp, " Still picture: %s\r\n"
				" Sequence end code: %s\r\n"
				" Video encode format: %d (%s)\r\n",
				TrueFalseString(still_picture_flag),
				TrueFalseString(sequence_end_code_flag),
				video_encode_format, video_encode_format <= 7 ? isdb_video_encode_format[video_encode_format] : isdb_video_encode_format[8]);
			lstrcat(v->szSIFormatBuffer, szTemp);
		}
		break;

	case 0xcd:	// ISDB TS Information
		if (!v->fISDB)
			goto DecodeMPEG2Descriptor_Default;
		set_buf(BM_USER_THREAD, pDescriptorData, 0, FALSE);
		{
			int j, k;
			int descriptor_tag = get_bits(BM_USER_THREAD, 8);
			int descriptor_length = get_bits(BM_USER_THREAD, 8);

			uint8_t remote_control_key_id = get_bits(BM_USER_THREAD, 8);
			wsprintf(szTemp, " Remote control key id: 0x%02x (%d)\r\n", remote_control_key_id, remote_control_key_id);
			lstrcat(v->szSIFormatBuffer, szTemp);

			int length_of_ts_name = get_bits(BM_USER_THREAD, 6);
			int transmission_type_count = get_bits(BM_USER_THREAD, 2);

			/* get TS Name */
			char szTsName[64] = { 0, };
			for (j = 0; j < length_of_ts_name; j++)
				szTsName[j] = get_bits(BM_USER_THREAD, 8);
			szTsName[j] = '\0';
			/* TODO: ARIB decoding */
			wsprintf(szTemp, " TS name: \"%s\"\r\n", szTsName);
			lstrcat(v->szSIFormatBuffer, szTemp);

			/* list transmission type info */
			for (j = 0; j < transmission_type_count; j++) {
				uint8_t transmission_type_info = get_bits(BM_USER_THREAD, 8);
				uint8_t num_of_service = get_bits(BM_USER_THREAD, 8);
				char szTransmissionType[64] = { 0, };
				FormatISDBTransmissionTypeInfo(szTransmissionType, sizeof(szTransmissionType), transmission_type_info);
				wsprintf(szTemp, " - Transmission type info: 0x%02x (%d) - %s\r\n", transmission_type_info, transmission_type_info, szTransmissionType);
				lstrcat(v->szSIFormatBuffer, szTemp);
				for (k = 0; k < num_of_service; k++) {
					uint16_t service_id = get_bits(BM_USER_THREAD, 16);
					wsprintf(szTemp, "   Service id: %d (0x%04x)\r\n", service_id, service_id);
					lstrcat(v->szSIFormatBuffer, szTemp);
				}
			}
		}
		break;

	case 0xcf:	// ISDB Logo Transmission
		if (!v->fISDB)
			goto DecodeMPEG2Descriptor_Default;
		set_buf(BM_USER_THREAD, pDescriptorData, 0, FALSE);
		{
			int descriptor_tag = get_bits(BM_USER_THREAD, 8);
			int descriptor_length = get_bits(BM_USER_THREAD, 8);

			uint8_t logo_transmission_type = get_bits(BM_USER_THREAD, 8);
			if (logo_transmission_type == 0x01) {
				/* CDT transmission type 1 */
				get_bits(BM_USER_THREAD, 7); /* reserved */
				uint16_t logo_id = get_bits(BM_USER_THREAD, 9);
				get_bits(BM_USER_THREAD, 4); /* reserved */
				uint16_t logo_version = get_bits(BM_USER_THREAD, 12);
				uint16_t download_data_id = get_bits(BM_USER_THREAD, 16);

				wsprintf(szTemp, " Logo transmission type: %02x (CDT transmission type 1)\r\n"
					" Logo id: 0x%03x (%d)\r\n"
					" Logo version: 0x%03x (%d)\r\n"
					" Download data id: 0x%04x (%d)\r\n",

					logo_transmission_type,
					logo_id, logo_id,
					logo_version, logo_version,
					download_data_id, download_data_id);

			} else if (logo_transmission_type == 0x02) {
				/* CDT transmission type 2 */
				get_bits(BM_USER_THREAD, 7); /* reserved */
				uint16_t logo_id = get_bits(BM_USER_THREAD, 9);

				wsprintf(szTemp, " Logo transmission type: %02x (CDT transmission type 2)\r\n"
					" Logo id: 0x%03x (%d)\r\n",
					logo_transmission_type,
					logo_id, logo_id);

			} else if (logo_transmission_type == 0x03) {
				/* Simple logo type */
				int j, simple_logo_name_length = descriptor_length - 1;
				char szLogoName[64] = { 0, };

				for (j = 0; j < simple_logo_name_length; j++)
					szLogoName[j] = get_bits(BM_USER_THREAD, 8);
				szLogoName[j] = '\0';

				wsprintf(szTemp, " Logo transmission type: %02x (Simple logo type)\r\n"
					" Logo characters: \"%s\"\r\n",
					logo_transmission_type,
					szLogoName);

			} else {
				int j, reserved_length = descriptor_length - 1;
				char *psz = szTemp;
				psz += wsprintf(szTemp, " Reserved transmission type %02x\r\n"
										" Reserved bytes: ", logo_transmission_type);

				for (j = 0; j < reserved_length; j++) {
					uint8_t byte = get_bits(BM_USER_THREAD, 8);
					wsprintf(psz, "%02x ", byte);
					psz += 3;
				}
				wsprintf(psz, "\r\n");
			}

			lstrcat(v->szSIFormatBuffer, szTemp);
		}
		break;

	case 0xde:	// ISDB Content Availability
		if (!v->fISDB)
			goto DecodeMPEG2Descriptor_Default;
		set_buf(BM_USER_THREAD, pDescriptorData, 0, FALSE);
		{
			int descriptor_tag = get_bits(BM_USER_THREAD, 8);
			int descriptor_length = get_bits(BM_USER_THREAD, 8);
			get_bits(BM_USER_THREAD, 1); // reserved
			uint8_t copy_restriction_mode = get_bits(BM_USER_THREAD, 1);
			uint8_t image_constraint_token = get_bits(BM_USER_THREAD, 1);
			uint8_t retention_mode = get_bits(BM_USER_THREAD, 1);
			uint8_t retention_state = (get_bits(BM_USER_THREAD, 3) & 7);
			uint8_t encryption_mode = get_bits(BM_USER_THREAD, 1);

			wsprintf(szTemp, " Copy restriction mode: %s\r\n"
				" Image constraint token: %s\r\n"
				" Retention mode: %s\r\n"
				" Retention state: %d (%s)\r\n"
				" Encryption mode: %s\r\n",
				TrueFalseString(copy_restriction_mode),
				TrueFalseString(image_constraint_token),
				TrueFalseString(retention_mode),
				retention_state, retention_state == 7 ? "1 hour and half" : "Undefined",
				TrueFalseString(encryption_mode));
			lstrcat(v->szSIFormatBuffer, szTemp);
		}
		break;

	case 0xf6:	// ISDB Access Control
		if (!v->fISDB)
			goto DecodeMPEG2Descriptor_Default;
		set_buf(BM_USER_THREAD, pDescriptorData, 0, FALSE);
		{
			int descriptor_tag = get_bits(BM_USER_THREAD, 8);
			int descriptor_length = get_bits(BM_USER_THREAD, 8);
			uint16_t ca_system_id = get_bits(BM_USER_THREAD, 16);
			uint8_t transmission_type = get_bits(BM_USER_THREAD, 3);
			uint16_t ecm_pid = get_bits(BM_USER_THREAD, 13);

			wsprintf(szTemp, " CA System Id: 0x%04x (%s)\r\n"
							 " Transmission type: %d (%s)\r\n"
							 " ECM PID: 0x%04x (%d)\r\n", 
				ca_system_id, (ca_system_id == 0xe ? "ARIB Content Protection" : "Unknown"),
				transmission_type, (transmission_type == 7 ? "Broadcast route" : "Undefined"),
				ecm_pid, ecm_pid);
			lstrcat(v->szSIFormatBuffer, szTemp);

			/* TODO print private data byte if care */
		}
		break;

	case 0xfa:	// ISDB Terrestrial Delivery System
		if (!v->fISDB)
			goto DecodeMPEG2Descriptor_Default;
		set_buf(BM_USER_THREAD, pDescriptorData, 0, FALSE);
		{
			const char *isdb_transmission_mode[] = { "2k", "4k", "8k", "Undefined" };
			const char *isdb_guard_interval[] = { "1/32", "1/16", "1/8", "1/4" };
			int descriptor_tag = get_bits(BM_USER_THREAD, 8);
			int descriptor_length = get_bits(BM_USER_THREAD, 8);

			uint16_t area_code = get_bits(BM_USER_THREAD, 12);
			uint8_t guard_interval = get_bits(BM_USER_THREAD, 2) & 3; /* FormatDVBTGuardInterval */
			uint8_t transmission_mode = get_bits(BM_USER_THREAD, 2) & 3;

			wsprintf(szTemp, " Area code: 0x%03x (%d)\r\n"
				" Guard interval: %d (%s)\r\n"
				" Transmission mode : %d (%s)\r\n",
				area_code, area_code,
				guard_interval, isdb_guard_interval[guard_interval],
				transmission_mode, isdb_transmission_mode[transmission_mode]);
			lstrcat(v->szSIFormatBuffer, szTemp);

			int j, frequency_length = (descriptor_length - 2) / 2;

			for (j = 0; j < frequency_length; j++) {
				uint16_t frequency = get_bits(BM_USER_THREAD, 16);
				StringCchPrintf(szTemp, sizeof(szTemp), " Frequency: %3.3f MHz\r\n", frequency * (1.0f/7.0f));
				lstrcat(v->szSIFormatBuffer, szTemp);
			}
		}
		break;

	case 0xfb:	// ISDB Partial Reception
		if (!v->fISDB)
			goto DecodeMPEG2Descriptor_Default;
		set_buf(BM_USER_THREAD, pDescriptorData, 0, FALSE);
		{
			int descriptor_tag = get_bits(BM_USER_THREAD, 8);
			int descriptor_length = get_bits(BM_USER_THREAD, 8);

			int j, service_length = (descriptor_length) / 2;

			for (j = 0; j < service_length; j++) {
				uint16_t service_id = get_bits(BM_USER_THREAD, 16);
				wsprintf(szTemp, " Service id: %d (0x%04x)\r\n", service_id, service_id);
				lstrcat(v->szSIFormatBuffer, szTemp);
			}
		}
		break;

	case 0xfd:	// ISDB Data Component
		if (!v->fISDB)
			goto DecodeMPEG2Descriptor_Default;
		DecodeISDBDataComponentDescriptor(szTemp, pDescriptorData);
		lstrcat(v->szSIFormatBuffer, szTemp);
		break;

	case 0xfe:	// ISDB System Management
		if (!v->fISDB)
			goto DecodeMPEG2Descriptor_Default;
		set_buf(BM_USER_THREAD, pDescriptorData, 0, FALSE);
		{
			const char *isdb_broadcasting_flag[] = { "Broadcasting", "Non-broadcasting", "Non-broadcasting", "Undefined" };
			const char *isdb_broadcasting_identifier[] = { "Undefined", "Satellite using 27 MHz bandwidth in 12.2 to 12.75 GHz band", "Satellite using 34.5 MHz bandwidth in 11.7 to 12.2 GHz band", "Terrestrial television",
				"Satellite using 34.5 MHz bandwidth in 12.2 to 12.75 GHz band", "Terrestrial sound", "Satellites or broadcasting stations in 2630 to 2655 MHz band", "Satellite advanced narrow-band using 27 MHz bandwidth in 12.2 to 12.75 GHz band" };

			int descriptor_tag = get_bits(BM_USER_THREAD, 8);
			int descriptor_length = get_bits(BM_USER_THREAD, 8);

			uint8_t broadcasting_flag = get_bits(BM_USER_THREAD, 2) & 3;
			uint8_t broadcasting_identifier = get_bits(BM_USER_THREAD, 6) & 0x3f;
			uint8_t additional_broadcasting_identification = get_bits(BM_USER_THREAD, 8);

			wsprintf(szTemp, " Broadcasting flag: %d (%s)\r\n"
				" Broadcasting identifier: %d (%s)\r\n"
				" Additional broadcasting id: 0x%02x (%d)\r\n",

				broadcasting_flag, isdb_broadcasting_flag[broadcasting_flag],
				broadcasting_identifier, broadcasting_identifier <= 7 ? isdb_broadcasting_identifier[broadcasting_identifier] : isdb_broadcasting_identifier[0],
				additional_broadcasting_identification, additional_broadcasting_identification);

			lstrcat(v->szSIFormatBuffer, szTemp);

			int j, additional_length = descriptor_length - 2;
			for (j = 0; j < additional_length; j++) {
				uint8_t additional_identification_info = get_bits(BM_USER_THREAD, 8);
				wsprintf(szTemp, " Additional identification info: 0x%02x (%d)\r\n", additional_identification_info);
				lstrcat(v->szSIFormatBuffer, szTemp);
			}
		}
		break;


/*	case 0xaa: // ?? what the hell is this?
		if (v->nNetworkPID != 0x1ffb)
		{
			FormatDefaultEITDescriptor(szTemp, pDescriptorData, FALSE);
			lstrcat(v->szSIFormatBuffer, szTemp);
		}
		else
			goto DecodeMPEG2Descriptor_Default;
		break;*/
#ifdef SKYSTUFF
	case 0xb1:
		set_buf(BM_USER_THREAD, pDescriptorData, 0, FALSE);
		{
			int descriptor_tag = get_bits(BM_USER_THREAD, 8);
			int descriptor_length = get_bits(BM_USER_THREAD, 8);

			int type = get_bits(BM_USER_THREAD, 8);
			//if (type == 0xff)		// Sky LCN
			{
				wsprintf(szTemp, " Sky LCN 0x%02x\r\n", type);
				lstrcat(v->szSIFormatBuffer, szTemp);
				descriptor_length--;
				while (descriptor_length > 9)
				{
					int flag = get_bits(BM_USER_THREAD, 8);
					int service_id = get_bits(BM_USER_THREAD, 16);
					int service_type = get_bits(BM_USER_THREAD, 8);
					int epg_id = get_bits(BM_USER_THREAD, 16);
					int lcn = get_bits(BM_USER_THREAD, 16);
					int unknown = get_bits(BM_USER_THREAD, 8);
					char szChannelName[64] = {0};

					descriptor_length -= 9;

					if (v->pChannelData[service_id] != NULL)
						wsprintf(szChannelName, " (%s)", v->pChannelData[service_id]->szShortName);
					wsprintf(szTemp, "  lcn %d service_id %d%s\r\n", lcn, service_id, szChannelName);
					lstrcat(v->szSIFormatBuffer, szTemp);
				}
				lstrcpy(szTemp, "  ");
				while (descriptor_length)
				{
					char szTemp2[8];
					int data = get_bits(BM_USER_THREAD, 8);
					wsprintf(szTemp2, "0x%02x ", data);
					lstrcat(szTemp, szTemp2);
					descriptor_length--;
				}
				lstrcat(szTemp, "\r\n");
				lstrcat(v->szSIFormatBuffer, szTemp);

				break;
			}
		}
	// don't break so we get the default decode
#endif SKYSTUFF
DecodeMPEG2Descriptor_Default:
	default:
		{
			int i;
			int nThisDescriptorLength = pDescriptorData[1];

			szTemp[0] = '\0';

			if (nThisDescriptorLength > 0)
			{				

				int nOutputIndex = 0;
				char szFormatBuffer[(16 * 3) + 16 + 1 + 2 + 1];

				if (fHTMLMode)
					lstrcat(v->szSIFormatBuffer, "<PRE>");

				memset(szFormatBuffer, ' ', sizeof(szFormatBuffer) - 3);
				szFormatBuffer[sizeof(szFormatBuffer) - 3] = '\r';
				szFormatBuffer[sizeof(szFormatBuffer) - 2] = '\n';
				szFormatBuffer[sizeof(szFormatBuffer) - 1] = '\0';

				lstrcpy(szTemp, " ");
				for (i = 0; i < nThisDescriptorLength; i++)
				{
					char szTemp2[10];
					char c;

					wsprintf(szTemp2, "%02x", pDescriptorData[2 + i]);
					memcpy(&szFormatBuffer[nOutputIndex * 3], szTemp2, 2);
					c = pDescriptorData[2 + i];
					if (c < ' ')
						c = '.';
					szFormatBuffer[16 * 3 + 1 + nOutputIndex] = c;
					nOutputIndex++;
					if (nOutputIndex == 16)
					{
						nOutputIndex = 0;
						lstrcat(v->szSIFormatBuffer, " ");
						lstrcat(v->szSIFormatBuffer, szFormatBuffer);
						memset(szFormatBuffer, ' ', sizeof(szFormatBuffer) - 3);
						szFormatBuffer[sizeof(szFormatBuffer) - 3] = '\r';
						szFormatBuffer[sizeof(szFormatBuffer) - 2] = '\n';
						szFormatBuffer[sizeof(szFormatBuffer) - 1] = '\0';
					}
				}
				if (nOutputIndex)
				{
					lstrcat(v->szSIFormatBuffer, " ");
					lstrcat(v->szSIFormatBuffer, szFormatBuffer);
				}

				if (fHTMLMode)
					lstrcat(v->szSIFormatBuffer, "</PRE>");
			}
		}
		break;
	}
}

void FormatATSCModulationMode(char * szModulationMode, int nModulationMode)
{
	switch(nModulationMode)
	{
	case 0x00:
		lstrcpy(szModulationMode, "Reserved");
		break;
	case 0x01:
		lstrcpy(szModulationMode, "Analog");
		break;
	case 0x02:
		lstrcpy(szModulationMode, "SCTE_mode_1");
		break;
	case 0x03:
		lstrcpy(szModulationMode, "SCTE_mode_2");
		break;
	case 0x04:
		lstrcpy(szModulationMode, "ATSC (8 VSB)");
		break;
	case 0x05:
		lstrcpy(szModulationMode, "ATSC (16 VSB)");
		break;
	case 0x80:
		lstrcpy(szModulationMode, "Defined in private descriptor");
		break;
	default:
		if (nModulationMode <= 0x7f)
			lstrcpy(szModulationMode, "Reserved for future use");
		else
			lstrcpy(szModulationMode, "User Private");
		break;
	}
}

void DecodeFEC(int nFEC, char * szFEC, BOOL fDCIIMode)
{
	if (fDCIIMode == FALSE)
		FormatDVBSCodeRate(szFEC, nFEC);
	else
	{
		// DCII table

		switch(nFEC)
		{
		case 0:
			lstrcpy(szFEC, "5/11");
			break;
		case 1:
			lstrcpy(szFEC, "1/2");
			break;
		case 3:
			lstrcpy(szFEC, "3/5");
			break;
		case 5:
			lstrcpy(szFEC, "2/3");
			break;
		case 7:
			lstrcpy(szFEC, "3/4");
			break;
		case 8:
			lstrcpy(szFEC, "4/5");
			break;
		case 9:
			lstrcpy(szFEC, "5/6");
			break;
		case 11:
			lstrcpy(szFEC, "7/8");
			break;
		case 15:
			lstrcpy(szFEC, "none");
			break;
		default:
			lstrcpy(szFEC, "reserved");
			break;
		}
	}
}

char *GetTableDescription(int nTableID)
{
	switch(nTableID)
	{
	case 0x00:
		return "Program Association Table (PAT - MPEG)";
	case 0x01:
		return "Conditional Access Table (CAT - MPEG)";
	case 0x02:
		return "Program Map Table (PMT - MPEG)";
	case 0x03:
		return "Transport Stream Description (TSDT - MPEG)";
		break;
	case 0x40:
		if (v->nNetworkPID == 0x0010)
			return "Network Information Actual (NIT - DVB)";
		break;
	case 0x41:
		if (v->nNetworkPID == 0x0010)
			return "Network Information Other (NIT - DVB)";
		break;
	case 0x42:
		if (v->nNetworkPID == 0x0010)
			return "Service Definition Actual (SDT - DVB)";
		break;
	case 0x46:
		if (v->nNetworkPID == 0x0010)
			return "Service Definition Other (SDT - DVB)";
		break;
	case 0x4a:
		if (v->nNetworkPID == 0x0010)
			return "Bouquet Assocation (BAT - DVB)";
		break;
	case 0x4e:
		if (v->nNetworkPID == 0x0010)
			return "Event Present/Following Actual (EIT - DVB)";
		break;
	case 0x4f:
		if (v->nNetworkPID == 0x0010)
			return "Event Present/Following Other (EIT - DVB)";
		break;
	case 0x50:
	case 0x51:
	case 0x52:
	case 0x53:
	case 0x54:
	case 0x55:
	case 0x56:
	case 0x57:
	case 0x58:
	case 0x59:
	case 0x5a:
	case 0x5b:
	case 0x5c:
	case 0x5d:
	case 0x5e:
	case 0x5f:
		if (v->nNetworkPID == 0x0010)
			return "Event Schedule Actual (EIT - DVB)";
		break;
	case 0x60:
	case 0x61:
	case 0x62:
	case 0x63:
	case 0x64:
	case 0x65:
	case 0x66:
	case 0x67:
	case 0x68:
	case 0x69:
	case 0x6a:
	case 0x6b:
	case 0x6c:
	case 0x6d:
	case 0x6e:
	case 0x6f:
		if (v->nNetworkPID == 0x0010)
			return "Event Schedule Other (EIT - DVB)";
		break;
	case 0x70:
		if (v->nNetworkPID == 0x0010)
			return "Time Date (TDT - DVB)";
		break;
	case 0x71:
		if (v->nNetworkPID == 0x0010)
			return "Running Status (RST - DVB)";
		break;
	case 0x72:
		if (v->nNetworkPID == 0x0010)
			return "Stuffing (ST - DVB)";
		break;
	case 0x73:
		if (v->nNetworkPID == 0x0010)
			return "Time Offset (TOT - DVB)";
		break;
	case 0x7e:
		if (v->nNetworkPID == 0x0010)
			return "Discontinuity Information (DIT - DVB)";
		break;
	case 0x7f:
		if (v->nNetworkPID == 0x0010)
			return "Selection Information (SIT - DVB)";
		break;
	case 0xc0:
		if (v->nNetworkPID == 0x0ffe)
			return "Program Information (DCII)";
		break;
	case 0xc1:
		if (v->nNetworkPID == 0x0ffe)
			return "Program Name (DCII)";
		break;
	case 0xc2:
		if (v->nNetworkPID == 0x0ffe)
			return "Network Information (DCII)";
		break;
	case 0xc3:
		if (v->nNetworkPID == 0x0ffe)
			return "Network Text (DCII)";
		break;
	case 0xc4:
		if (v->nNetworkPID == 0x0ffe)
			return "Virtual Channel (DCII)";
		break;
	case 0xc5:
		if (v->nNetworkPID == 0x0ffe)
			return "System Time (DCII)";
		break;
	case 0xc6:
		if (v->nNetworkPID == 0x0ffe)
			return "Subtitle (DCII)";
		break;
	case 0xc7:
		if (v->nNetworkPID == 0x1ffb)
			return "Master Guide Table (MGT - ATSC)";
		break;
	case 0xc8:
		if (v->nNetworkPID == 0x1ffb)
			return "Terrestrial Virtual Channel Table (TVCT - ATSC)";
		break;
	case 0xc9:
		if (v->nNetworkPID == 0x1ffb)
			return "Cable Virtual Channel Table (CVCT - ATSC)";
		break;
	case 0xca:
		if (v->nNetworkPID == 0x1ffb)
			return "Rating Region Table (RRT - ATSC)";
		break;
	case 0xcb:
		if (v->nNetworkPID == 0x1ffb)
			return "Event Information Table (EIT - ATSC)";
		break;
	case 0xcc:
		if (v->nNetworkPID == 0x1ffb)
			return "Extended Text Table (ETT - ATSC)";
		break;
	case 0xcd:
		if (v->nNetworkPID == 0x1ffb)
			return "System Time Table (STT - ATSC)";
		break;
	}

	return "";
}

char * FormatNITEntry(int nTransportStreamID, BOOL fIncludeHTMLTags)
{
	int i;
	char szTemp[4096];
	char szAnchor[128] = {0};

	if (fIncludeHTMLTags)
		wsprintf(szAnchor, "<A NAME=\"nit_%d\"></A>", v->pNITData[nTransportStreamID]->nTransportStreamID);
	sprintf(v->szSIFormatBuffer,
		    "%sNetwork Name: %s\r\nNetwork ID: %d (0x%04x)\r\nTransport Stream ID: %d (0x%04x)\r\n"
			"Original Network ID: %d (0x%04x) Version: %d\r\n",
			szAnchor,
			v->pNITData[nTransportStreamID]->szNetworkName,
			v->pNITData[nTransportStreamID]->nNetworkID,
			v->pNITData[nTransportStreamID]->nNetworkID,
			v->pNITData[nTransportStreamID]->nTransportStreamID,
			v->pNITData[nTransportStreamID]->nTransportStreamID,
			v->pNITData[nTransportStreamID]->nOriginalNetworkID,
			v->pNITData[nTransportStreamID]->nOriginalNetworkID,
			v->pNITData[nTransportStreamID]->nVersionNumber);

	if (v->pNITData[nTransportStreamID]->nNetworkDescriptorsLength)
	{
		int nNetworkDescriptorsLength = v->pNITData[nTransportStreamID]->nNetworkDescriptorsLength;
		int nNetworkDescriptorsOffset = 0;
		BYTE * pDescriptors = v->pNITData[nTransportStreamID]->pNetworkDescriptors;

		while (nNetworkDescriptorsLength)
		{
			int descriptor_tag = pDescriptors[0];
			int descriptor_length = pDescriptors[1];
			char szDescriptor[128];
			char szTemp[256];

			DecodeDescriptorNames(szDescriptor, descriptor_tag);
			wsprintf(szTemp, "\r\nDescriptor: %s\r\n", szDescriptor);
			lstrcat(v->szSIFormatBuffer, szTemp);
			DecodeMPEG2Descriptor(pDescriptors, fIncludeHTMLTags);

			nNetworkDescriptorsLength -= descriptor_length + 2;
			pDescriptors += descriptor_length + 2;
		}
	}

	switch(v->pNITData[nTransportStreamID]->nType)
	{
	case NIT_DVBC:
		{
			char szModulation[32];
			char szFECOuter[32];
			char szFECInner[32];

			FormatDVBCModulation(szModulation, v->pNITData[nTransportStreamID]->dvbc.nModulation);
			FormatDVBCOuterFEC(szFECOuter, v->pNITData[nTransportStreamID]->dvbc.nFECOuter);
			FormatDVBCInnerFEC(szFECInner, v->pNITData[nTransportStreamID]->dvbc.nFECInner);

			sprintf(szTemp,
				     "DVB-C Frequency: %.3f MHz\r\nSymbol Rate: %.3f MSps\r\nModulation: %s\r\nFEC Outer: %s\r\nFEC Inner: %s\r\n",
					 (double)v->pNITData[nTransportStreamID]->nFrequency / 10000.0,
					 (double)v->pNITData[nTransportStreamID]->dvbc.nSymbolRate / 10000.0,
					 szModulation,
					 szFECOuter,
					 szFECInner);

			lstrcat(v->szSIFormatBuffer, szTemp);
		}
		break;

	case NIT_DVBS:
		{
			char szFEC[16];
			char szEW[16];
			char szPolarity[24];
			char szModulation[64] = {0};

			FormatDVBSCodeRate(szFEC, v->pNITData[nTransportStreamID]->dvbs.nFEC);
			FormatPolarity(szPolarity, v->pNITData[nTransportStreamID]->dvbs.nPolarization, TRUE);
			if (v->pNITData[nTransportStreamID]->dvbs.fEastern == TRUE)
				lstrcpy(szEW, "E");
			else
				lstrcpy(szEW, "W");
			FormatDVBSModulation(szModulation, v->pNITData[nTransportStreamID]->dvbs.nModulation);

			sprintf(szTemp,
					"DVB-S Orbital Position: %3.1f%s\r\nFrequency: %.3f GHz\r\n"
					"Modulation: %s\r\nPolarity: %s\r\nSymbol Rate: %5d MSps\r\nFEC: %s\r\n",
					(double)v->pNITData[nTransportStreamID]->dvbs.nOrbitalPosition / 10.0,
					szEW,
					(double)v->pNITData[nTransportStreamID]->nFrequency / 100000.0,
					szModulation,
					szPolarity,
					v->pNITData[nTransportStreamID]->dvbs.nSymbolRate / 10,
					szFEC);
			lstrcat(v->szSIFormatBuffer, szTemp);
		}
		break;
	case NIT_DVBT:
		{
			char szBandwidth[16];
			char szConstellation[16];
			char szHierarchyInformation[64];
			char szGuardInterval[16];
			char szFECHP[16];
			char szFECLP[16];

			FormatDVBTBandwidth(szBandwidth, v->pNITData[nTransportStreamID]->dvbt.nBandwidth);
			FormatDVBTConstellation(szConstellation, v->pNITData[nTransportStreamID]->dvbt.nConstellation);
			FormatDVBTHierarchyInformation(szHierarchyInformation, v->pNITData[nTransportStreamID]->dvbt.nHierarchyInformation);
			FormatDVBTCodeRate(szFECHP, v->pNITData[nTransportStreamID]->dvbt.nCodeRateHPStream);
			FormatDVBTCodeRate(szFECLP, v->pNITData[nTransportStreamID]->dvbt.nCodeRateLPStream);
			FormatDVBTGuardInterval(szGuardInterval, v->pNITData[nTransportStreamID]->dvbt.nGuardInterval);

			if (v->pNITData[nTransportStreamID]->dvbt.nHierarchyInformation == 0)
				sprintf(szTemp, "DVB-T Frequency %.3f MHz\r\nBandwidth: %s Constellation: %s\r\n"
								 "Hierarchy: %s Guard Interval %s\r\n"
								 "Code Rate: %s\r\n",
								 (double)v->pNITData[nTransportStreamID]->nFrequency / 100000.0,
								 szBandwidth, szConstellation,
								 szHierarchyInformation, szGuardInterval,
								 szFECHP);
			else
				sprintf(szTemp, "DVB-T Frequency %.3f MHz\r\nBandwidth: %s Constellation: %s\r\n"
								 "Hierarchy: %s Guard Interval %s\r\n"
								 "HP Code Rate: %s LP Code Rate: %s\r\n",
								 (double)v->pNITData[nTransportStreamID]->nFrequency / 100000.0,
								 szBandwidth, szConstellation,
								 szHierarchyInformation, szGuardInterval,
								 szFECHP, szFECLP);
			lstrcat(v->szSIFormatBuffer, szTemp);
		}
		break;
	}

	if (v->pNITData[nTransportStreamID]->fThisTS == TRUE)
		lstrcat(v->szSIFormatBuffer, "\r\nCurrent Network: True\r\n");

	if (fIncludeHTMLTags)
		lstrcat(v->szSIFormatBuffer, "<FONT SIZE=\"-1\">");
	for (i = 0; i < MAX_NIT_EXTRA_DESCRIPTORS; i++)
	{
		if (v->pNITData[nTransportStreamID]->pExtraDescriptors[i] == NULL)
			break;

		switch(v->pNITData[nTransportStreamID]->pExtraDescriptors[i][0])
		{
		case 0x43:		// satellite delivery
		case 0x44:		// cable delivery
		case 0x5a:		// terrestrial delivery
#ifndef DECODE_ALL_DESCRIPTORS
			break;
#endif DECODE_ALL_DESCRIPTORS
		default:
			{
				char szDescriptor[128];
				char szTemp[256];
				DecodeDescriptorNames(szDescriptor, v->pNITData[nTransportStreamID]->pExtraDescriptors[i][0]);
				wsprintf(szTemp, "\r\nDescriptor: %s\r\n", szDescriptor);
				lstrcat(v->szSIFormatBuffer, szTemp);

				DecodeMPEG2Descriptor(v->pNITData[nTransportStreamID]->pExtraDescriptors[i], fIncludeHTMLTags);
			}
			break;
		}
	}
	if (fIncludeHTMLTags)
		lstrcat(v->szSIFormatBuffer, "</FONT>");
	lstrcat(v->szSIFormatBuffer, "\r\n");

	return v->szSIFormatBuffer;
}

int __cdecl SortEITCompare(const void *elem1, const void *elem2)
{
	PEITEVENT pEIT1 = (PEITEVENT)elem1;
	PEITEVENT pEIT2 = (PEITEVENT)elem2;
	DWORD64 dwTime1, dwTime2;

	SystemTimeToFileTime(&pEIT1->stStartTime, (FILETIME *)&dwTime1);
	SystemTimeToFileTime(&pEIT2->stStartTime, (FILETIME *)&dwTime2);

	if (dwTime1 < dwTime2)
		return -1;
	if (dwTime1 > dwTime2)
		return 1;

	return 0;
}

void GetEITSource(char * szSource, PEITEVENT pEvent)
{
	if (v->nNetworkPID == 0x0010)
	{
		switch(pEvent->nSource)
		{
		case 0x4e:
			lstrcpy(szSource, "actual TS, present/following");
			break;
		case 0x4f:
			lstrcpy(szSource, "other TS, present/following");
			break;
		case 0xff:
			if (v->fSkyEPG)
				lstrcpy(szSource, "Sky 7 day EPG");
			break;
		default:
			if (pEvent->nSource >= 0x50 && pEvent->nSource <= 0x5f)
				lstrcpy(szSource, "actual TS, event schedule");
			else if (pEvent->nSource >= 0x60 && pEvent->nSource <= 0x6f)
				lstrcpy(szSource, "other TS, event schedule");
			break;
		}
	}
	else
	{
		wsprintf(szSource, "EIT%d", pEvent->nSource);
	}
}

char * FormatEITEntry(int nChannelNumber, int nEITFormat, BOOL fIncludeHTMLTags)
{
	char szShortName[128] = {0};
	char szLongName[256] = {0};
	char szSignalInfo[128] = {0};

	v->szSIFormatBuffer[0] = '\0';
	if (v->pChannelData[nChannelNumber] != NULL)
	{
		lstrcpy(szShortName, v->pChannelData[nChannelNumber]->szShortName);
		EscapeReplaceXML(szShortName);
		lstrcpy(szLongName, v->pChannelData[nChannelNumber]->szLongName);
		EscapeReplaceXML(szLongName);

		switch(nEITFormat)
		{
		case EIT_FORMAT_PLAIN:
			{
				int nLCN = GetLogicalChannelNumber(v->pChannelData[nChannelNumber]->nChannelNumber);
				char szQuickNIT[128] = {0};
				char szChannelString[48];

				memset(v->szSIFormatBuffer, 0, sizeof(v->szSIFormatBuffer));

				if (nLCN)
					wsprintf(szChannelString, "%d/%d", nLCN, v->pChannelData[nChannelNumber]->nChannelNumber);
				else
					wsprintf(szChannelString, "%d", v->pChannelData[nChannelNumber]->nChannelNumber);

				QuickFormatNIT(szQuickNIT, v->pChannelData[nChannelNumber]->nTransportStreamID, FALSE);
				wsprintf(v->szSIFormatBuffer, "Channel %s\r\nService Name: %s\r\nProvider Name: %s\r\nTransport Stream ID: %d (0x%04x) %s\r\n\r\n",
						 szChannelString,
						 szShortName,
						 szLongName,
						 v->pChannelData[nChannelNumber]->nTransportStreamID,
						 v->pChannelData[nChannelNumber]->nTransportStreamID,
						 szQuickNIT);
			}
			break;
		case EIT_FORMAT_XML:
			{
				wsprintf(v->szSIFormatBuffer, 
						 " <CHANNEL>\r\n  <SERVICE-NUMBER>%d</SERVICE-NUMBER>\r\n  <SHORT-NAME>%s</SHORT-NAME>\r\n  <LONG-NAME>%s</LONG-NAME>\r\n  <TRANSPORT-STREAM-ID>%d</TRANSPORT-STREAM-ID>\r\n",
						 v->pChannelData[nChannelNumber]->nChannelNumber,
						 szShortName,
						 szLongName,
						 v->pChannelData[nChannelNumber]->nTransportStreamID);
			}
			break;
		case EIT_FORMAT_XMLTV:
			{
				QuickFormatNIT(szSignalInfo, v->pChannelData[nChannelNumber]->nTransportStreamID, FALSE);
				wsprintf(v->szSIFormatBuffer,
						 "<channel id=\"%d-%s\">\r\n"
						 " <display-name lang=\"en\">%s</display-name>\r\n"
						 " <transport-stream-ID>%d</transport-stream-ID>\r\n"
						 " <signal-info>%s</signal-info>\r\n"
						 "</channel>\r\n",
						 nChannelNumber, szShortName,
						 szShortName,
						 v->pChannelData[nChannelNumber]->nTransportStreamID,
						 szSignalInfo);
			}
			break;
		}
	}
	else
	{
		switch(nEITFormat)
		{
		case EIT_FORMAT_PLAIN:
			{
				char szQuickNIT[128] = {0};

				wsprintf(v->szSIFormatBuffer, "Channel %d\r\n\r\n",
						 nChannelNumber);
			}
			break;
		case EIT_FORMAT_XML:
			{
				wsprintf(v->szSIFormatBuffer, 
						 " <CHANNEL>\r\n  <SERVICE-NUMBER>%d</SERVICE-NUMBER>\r\n",
						 nChannelNumber);
			}
			break;
		case EIT_FORMAT_XMLTV:
			{
				wsprintf(v->szSIFormatBuffer,
						 "<channel id=\"%d\">\r\n"
						 "</channel>\r\n",
						 nChannelNumber);
			}
			break;
		}
	}

	{
		int nEITItems;
		PEITEVENT pCurrent;
		PEITEVENT pSortList;

		// Count the EIT items so we know how many we might need to copy
		nEITItems = 0;
		EnterCriticalSection(&v->csEIT);
		pCurrent = v->pEvents[nChannelNumber];
		if (pCurrent != NULL)
		{
			do
			{
				nEITItems++;
				pCurrent = (PEITEVENT)pCurrent->dwNextEvent;
			} while (pCurrent != NULL);
		}
		if (nEITItems)
		{
			// Got some items, let's copy them into our buffer so we can sort
			int nOutputIndex = 0;

			pSortList = LocalAlloc(LPTR, nEITItems * sizeof(EITEVENT));
			if (pSortList == NULL)
				OutputDebugString("TSReader_formatter.c: LocalAlloc == NULL when copying EIT\n");
			else
			{
				pCurrent = v->pEvents[nChannelNumber];
				do
				{
					int i;

					memcpy(&pSortList[nOutputIndex], pCurrent, sizeof(EITEVENT));
					for (i = 0; i < MAX_EIT_EXTRA_DESCRIPTORS; i++)
					{
						if (pCurrent->pExtraDescriptors[i] == NULL)
							break;
						pSortList[nOutputIndex].pExtraDescriptors[i] = LocalAlloc(LPTR, pCurrent->pExtraDescriptors[i][1] + 2 + 4);
						memcpy(pSortList[nOutputIndex].pExtraDescriptors[i], pCurrent->pExtraDescriptors[i], pCurrent->pExtraDescriptors[i][1] + 2);
					}
					
					nOutputIndex++;
					pCurrent = (PEITEVENT)pCurrent->dwNextEvent;
				} while (pCurrent != NULL);
			}
		}
		LeaveCriticalSection(&v->csEIT);
		if (nEITItems)
		{
			int nEITIndex;
			int i;
			char szTemp[64 * 1024];

			qsort(pSortList, nEITItems, sizeof(EITEVENT), SortEITCompare);
			
			for (nEITIndex = 0; nEITIndex < nEITItems; nEITIndex++)
			{
				char szStartDate[128] = {0};
				char szStartTime[128] = {0};
				SYSTEMTIME stMyTime;

				switch(nEITFormat)
				{
				case EIT_FORMAT_PLAIN:
					{
						BOOL fUTCTime = FALSE;
		
						if (SystemTimeToTzSpecificLocalTime(NULL, &pSortList[nEITIndex].stStartTime, &stMyTime) == FALSE)
						{
							// Must be Windows 9x
							memcpy(&stMyTime, &pSortList[nEITIndex].stStartTime, sizeof(SYSTEMTIME));
							fUTCTime = TRUE;
						}
						GetDateFormat(LOCALE_SYSTEM_DEFAULT, DATE_SHORTDATE, &stMyTime, NULL, szStartDate, sizeof(szStartDate));
						GetTimeFormat(LOCALE_SYSTEM_DEFAULT, 0, &stMyTime, NULL, szStartTime, sizeof(szStartTime));
						if (fUTCTime == TRUE)
							lstrcat(szStartTime, " (UTC)");

						if (v->nNetworkPID == 0x0010)
						{
							char szEventDescriptionLines[4096] = {0};
							char szSource[128] = {"n/a"};
							GetEITSource(szSource, &pSortList[nEITIndex]);	
							if (pSortList[nEITIndex].szShortEventDescription != NULL)
								lstrcpy(szEventDescriptionLines, pSortList[nEITIndex].szShortEventDescription);
							if (pSortList[nEITIndex].szLongEventDescription != NULL)
							{
								if (lstrlen(szEventDescriptionLines))
								{
									lstrcat(szEventDescriptionLines, "\r\n");
									lstrcat(szEventDescriptionLines, pSortList[nEITIndex].szLongEventDescription);
								}
								else
									lstrcpy(szEventDescriptionLines, pSortList[nEITIndex].szLongEventDescription);
							}
							wsprintf(szTemp, "---------------------------------------------\r\nStarts: %s %s\r\nLength: %02d:%02d:%02d\r\nEIT Source: %s\r\nName: %s\r\n%s",
									 szStartDate,
									 szStartTime,
									 pSortList[nEITIndex].stRunTime.wHour, pSortList[nEITIndex].stRunTime.wMinute, pSortList[nEITIndex].stRunTime.wSecond,
									 szSource,
									 pSortList[nEITIndex].szEventName,
									 szEventDescriptionLines);
						}
						else
						{
							char szNA[] = {"n/a"};
							char szSource[128] = {"n/a"};
							char * pDescription = szNA;

							if (pSortList[nEITIndex].szShortEventDescription != NULL)
								pDescription = pSortList[nEITIndex].szShortEventDescription;

							GetEITSource(szSource, &pSortList[nEITIndex]);
							wsprintf(szTemp, "---------------------------------------------\r\nStarts: %s %s\r\nLength: %02d:%02d:%02d\r\nEIT Source: %s\r\nName: %s\r\nDescription: %s\r\n",
									 szStartDate,
									 szStartTime,
									 pSortList[nEITIndex].stRunTime.wHour, pSortList[nEITIndex].stRunTime.wMinute, pSortList[nEITIndex].stRunTime.wSecond,
									 szSource,
									 pSortList[nEITIndex].szEventName,
									 pDescription);
						}
					}
					break;
				case EIT_FORMAT_XML:
					{
						SYSTEMTIME stLocal;
						char szEventName[1024];
						char szEventDescription[4096];
						char szStartLocalDate[128];
						char szStartLocalTime[128];

						wsprintf(szStartDate, "%04d-%02d-%02d", pSortList[nEITIndex].stStartTime.wYear, pSortList[nEITIndex].stStartTime.wMonth, pSortList[nEITIndex].stStartTime.wDay);
						wsprintf(szStartTime, "%02d:%02d:%02d", pSortList[nEITIndex].stStartTime.wHour, pSortList[nEITIndex].stStartTime.wMinute, pSortList[nEITIndex].stStartTime.wSecond);

						if (SystemTimeToTzSpecificLocalTime(NULL, &pSortList[nEITIndex].stStartTime, &stLocal) == FALSE)
						{
							lstrcpy(szStartDate, "N/A - Windows 95");
							szStartTime[0] = 0;
						}
						else
						{
							wsprintf(szStartLocalDate, "%04d-%02d-%02d", stLocal.wYear, stLocal.wMonth, stLocal.wDay);
							wsprintf(szStartLocalTime, "%02d:%02d:%02d", stLocal.wHour, stLocal.wMinute, stLocal.wSecond);
						}

						lstrcpy(szEventName, pSortList[nEITIndex].szEventName);
						EscapeReplaceXML(szEventName);
						if (!lstrlen(szEventName))
							lstrcpy(szEventName, " ");
						szEventDescription[0] = '\0';
						if (pSortList[nEITIndex].szShortEventDescription != NULL)
							lstrcat(szEventDescription, pSortList[nEITIndex].szShortEventDescription);
						if (pSortList[nEITIndex].szLongEventDescription != NULL)
							lstrcat(szEventDescription, pSortList[nEITIndex].szLongEventDescription);
						EscapeReplaceXML(szEventDescription);
						if (!lstrlen(szEventDescription))
							lstrcpy(szEventDescription, " ");

						wsprintf(szTemp,
								 "  <EVENT>\r\n"
								 "     <START-UTC-DATE>%s</START-UTC-DATE>\r\n"
								 "     <START-UTC-TIME>%s</START-UTC-TIME>\r\n"
								 "     <START-LOCAL-DATE>%s</START-LOCAL-DATE>\r\n"
								 "     <START-LOCAL-TIME>%s</START-LOCAL-TIME>\r\n"
								 "     <DURATION>%02d:%02d:%02d</DURATION>\r\n"
								 "     <NAME>%s</NAME>\r\n"
								 "     <DESCRIPTION>%s</DESCRIPTION>\r\n"
								 "     <ID>%d</ID>\r\n"
								 "     <FREE-CA-MODE>%d</FREE-CA-MODE>\r\n"
								 "     <RUNNING-STATUS>%d</RUNNING-STATUS>\r\n",
								 szStartDate, szStartTime,
								 szStartLocalDate, szStartLocalTime,
								 pSortList[nEITIndex].stRunTime.wHour, pSortList[nEITIndex].stRunTime.wMinute, pSortList[nEITIndex].stRunTime.wSecond,
								 szEventName,
								 szEventDescription,
								 pSortList[nEITIndex].nEventID,
								 pSortList[nEITIndex].fFreeCAMode,
								 pSortList[nEITIndex].nRunningStatus);					
					}
					break;
				case EIT_FORMAT_XMLTV:
					{
						char szEventName[128];
						char szEventDescription[4096];
						char szEventStart[128] = {0};
						char szEventEnd[128] = {0};

						DWORD64 dwStartTime, dwDuration, dwEndTime;
						SYSTEMTIME stEndTime;
						SYSTEMTIME stLocal;

						SystemTimeToTzSpecificLocalTime(NULL, &pSortList[nEITIndex].stStartTime, &stLocal);

						wsprintf(szEventStart, "%04d%02d%02d%02d%02d%02d",
								 stLocal.wYear, stLocal.wMonth, stLocal.wDay,
								 stLocal.wHour, stLocal.wMinute, stLocal.wSecond);
						SystemTimeToFileTime(&stLocal, (FILETIME *)&dwStartTime);
						dwDuration = (DWORD64)10000000 * (DWORD64)pSortList[nEITIndex].stRunTime.wHour * (DWORD64)60 * (DWORD64)60
			                       + (DWORD64)10000000 * (DWORD64)pSortList[nEITIndex].stRunTime.wMinute * (DWORD64)60
								   + (DWORD64)10000000 * (DWORD64)pSortList[nEITIndex].stRunTime.wSecond;
						dwEndTime = dwStartTime + dwDuration;
						FileTimeToSystemTime((FILETIME *)&dwEndTime, &stEndTime);
						wsprintf(szEventEnd, "%04d%02d%02d%02d%02d%02d",
								 stEndTime.wYear, stEndTime.wMonth, stEndTime.wDay,
								 stEndTime.wHour, stEndTime.wMinute, stEndTime.wSecond);

						lstrcpy(szEventName, pSortList[nEITIndex].szEventName);
						EscapeReplaceXML(szEventName);
						if (!lstrlen(szEventName))
							lstrcpy(szEventName, " ");
						szEventDescription[0] = '\0';
						if (pSortList[nEITIndex].szShortEventDescription != NULL)
							lstrcat(szEventDescription, pSortList[nEITIndex].szShortEventDescription);
						if (pSortList[nEITIndex].szLongEventDescription != NULL)
							lstrcat(szEventDescription, pSortList[nEITIndex].szLongEventDescription);
						EscapeReplaceXML(szEventDescription);
						if (!lstrlen(szEventDescription))
							lstrcpy(szEventDescription, " ");

						wsprintf(szTemp,
							     "<programme start=\"%s\" stop=\"%s\" channel=\"%d-%s\">\r\n"
								 " <title>%s</title>\r\n"
								 " <desc>%s|%s</desc>\r\n"
								 "</programme>",
								 szEventStart, szEventEnd, nChannelNumber, szShortName,
								 szEventName,
								 szEventDescription,
								 szSignalInfo);
					}
					break;
				}

				if (fIncludeHTMLTags == TRUE)
					lstrcat(szTemp, "<FONT SIZE=\"-1\">");
			
				if (nEITFormat == EIT_FORMAT_PLAIN)
				{
					if (pSortList[nEITIndex].nFlags)
					{
						char szTemp2[128];

						lstrcpy(szTemp2, "\r\nDescription Source: ");
						if (pSortList[nEITIndex].nFlags & EIT_FLAG_SHORT_EVENT)
							lstrcat(szTemp2, "DVB Short Event ");
						if (pSortList[nEITIndex].nFlags & EIT_FLAG_LONG_EVENT)
							lstrcat(szTemp2, "DVB Extended Event ");
						if (pSortList[nEITIndex].nFlags & EIT_FLAG_COMPRESSED_SHORT_EVENT)
							lstrcat(szTemp2, "Dish Short Event ");
						if (pSortList[nEITIndex].nFlags & EIT_FLAG_COMPRESSED_LONG_EVENT)
							lstrcat(szTemp2, "Dish Extended Event ");
						lstrcat(szTemp2, "\r\n");
						lstrcat(szTemp, szTemp2);
					}
				}
				lstrcat(v->szSIFormatBuffer, szTemp);

				{
					for (i = 0; i < MAX_EIT_EXTRA_DESCRIPTORS; i++)
					{
						if (pSortList[nEITIndex].pExtraDescriptors[i] == NULL)
							break;
						switch(nEITFormat)
						{
						case EIT_FORMAT_XML:
							FormatDefaultEITDescriptor(szTemp, pSortList[nEITIndex].pExtraDescriptors[i], TRUE);
							lstrcat(v->szSIFormatBuffer, szTemp);
							break;
						case EIT_FORMAT_PLAIN:
							{
								char szDescriptor[256];
								char szTemp[256];

								DecodeDescriptorNames(szDescriptor, pSortList[nEITIndex].pExtraDescriptors[i][0]);
								wsprintf(szTemp, "\r\nDescriptor: %s\r\n", szDescriptor);
								lstrcat(v->szSIFormatBuffer, szTemp);
								DecodeMPEG2Descriptor(pSortList[nEITIndex].pExtraDescriptors[i], FALSE);
							}
							break;
						case EIT_FORMAT_XMLTV:
							break;
						}
					}
				}
				lstrcat(v->szSIFormatBuffer, "\r\n");
				if (fIncludeHTMLTags == TRUE)
					lstrcat(v->szSIFormatBuffer, "</FONT>");
				if (nEITFormat == EIT_FORMAT_XML)
					  lstrcat(v->szSIFormatBuffer, "  </EVENT>\r\n");
			}			

			for (nEITIndex = 0; nEITIndex < nEITItems; nEITIndex++)
			{
				int i;

				for (i = 0; i < MAX_EIT_EXTRA_DESCRIPTORS; i++)
				{
					if (pSortList[nEITIndex].pExtraDescriptors[i] == NULL)
						break;
					LocalFree(pSortList[nEITIndex].pExtraDescriptors[i]);
				}
			}
			LocalFree(pSortList);
		}
	}

	if (nEITFormat == EIT_FORMAT_XML)
		lstrcat(v->szSIFormatBuffer, " </CHANNEL>");

	return v->szSIFormatBuffer;
}

char * FormatCAT(BOOL fHTMLMode)
{
	int i;
	char szTemp[200];

	wsprintf(v->szSIFormatBuffer, "CAT Version Number: %d\r\n", v->cat.nVersionNumber);

	if (fHTMLMode)
		lstrcat(v->szSIFormatBuffer, "<FONT SIZE=\"-1\">");

	for (i = 0; i < MAX_CAT_DESCRIPTORS; i++)
	{
		char szDescriptor[128];

		if (v->cat.pDescriptor[i] == NULL)
			break;

		DecodeDescriptorNames(szDescriptor, v->cat.pDescriptor[i][0]);
		wsprintf(szTemp, "\r\nDescriptor: %s\r\n", szDescriptor);
		lstrcat(v->szSIFormatBuffer, szTemp);

		DecodeMPEG2Descriptor(v->cat.pDescriptor[i], fHTMLMode);
	}

	if (fHTMLMode)
		lstrcat(v->szSIFormatBuffer, "</FONT>");

	return v->szSIFormatBuffer;
}

char * FormatCVCT(int nCVCTIndex)
{
	v->szSIFormatBuffer[0] = '\0';
	if (nCVCTIndex != MAX_CVCT_ENTRIES + 1)
	{
		int i;
		char szTemp[128];

		wsprintf(v->szSIFormatBuffer, "Transport Stream ID %d (0x%04x) Version %d Channels %d\r\n\r\n", 
			v->cvct[nCVCTIndex].transport_stream_id, 
			v->cvct[nCVCTIndex].transport_stream_id,
			v->cvct[nCVCTIndex].version_number,
			v->cvct[nCVCTIndex].num_channels_in_section);

		for (i = 0; i < v->cvct[nCVCTIndex].num_channels_in_section; i++)
		{
			wsprintf(szTemp, "Channel: %s (%d.%d) Program %d TSID %d (0x%04x)\r\n",
				v->cvct[nCVCTIndex].CVCTEntry[i].szShortName,
				v->cvct[nCVCTIndex].CVCTEntry[i].major_channel_number,
				v->cvct[nCVCTIndex].CVCTEntry[i].minor_channel_number,
				v->cvct[nCVCTIndex].CVCTEntry[i].program_number,
				v->cvct[nCVCTIndex].CVCTEntry[i].channel_TSID,
				v->cvct[nCVCTIndex].CVCTEntry[i].channel_TSID);
			lstrcat(v->szSIFormatBuffer, szTemp);
			wsprintf(szTemp, "Carrier %d Modulation %d ETM_location %d Access Controlled %d\r\n",
				v->cvct[nCVCTIndex].CVCTEntry[i].carrier_frequency,
				v->cvct[nCVCTIndex].CVCTEntry[i].modulation_mode,
				v->cvct[nCVCTIndex].CVCTEntry[i].ETM_location,
				v->cvct[nCVCTIndex].CVCTEntry[i].access_controlled);
			lstrcat(v->szSIFormatBuffer, szTemp);
			wsprintf(szTemp, "Hidden %d Path %d OOB %d Hide Guide %d\r\n",
				v->cvct[nCVCTIndex].CVCTEntry[i].hidden,
				v->cvct[nCVCTIndex].CVCTEntry[i].path_select,
				v->cvct[nCVCTIndex].CVCTEntry[i].out_of_band,
				v->cvct[nCVCTIndex].CVCTEntry[i].hide_guide);
			lstrcat(v->szSIFormatBuffer, szTemp);
			wsprintf(szTemp, "Service Type %d Source ID %d\r\n",
				v->cvct[nCVCTIndex].CVCTEntry[i].service_type,
				v->cvct[nCVCTIndex].CVCTEntry[i].source_id);
			lstrcat(v->szSIFormatBuffer, szTemp);

			if (v->cvct[nCVCTIndex].CVCTEntry[i].descriptors_length)
			{
				BYTE * pAdditionalDescriptors = v->cvct[nCVCTIndex].CVCTEntry[i].pDescriptors;
				int nDescriptorsLength = v->cvct[nCVCTIndex].CVCTEntry[i].descriptors_length;

				while (nDescriptorsLength)
				{
					BYTE nDescriptor = pAdditionalDescriptors[0];
					BYTE nDescriptorLength = pAdditionalDescriptors[1];
					char szDescriptor[128];

					DecodeDescriptorNames(szDescriptor, nDescriptor);
					wsprintf(szTemp, " Descriptor: %s\r\n", szDescriptor);
					lstrcat(v->szSIFormatBuffer, szTemp);

					DecodeMPEG2Descriptor(&pAdditionalDescriptors[0], FALSE);
					pAdditionalDescriptors += nDescriptorLength + 2;
					nDescriptorsLength -= nDescriptorLength + 2;
					lstrcat(v->szSIFormatBuffer, "\r\n");
				}
			}
			lstrcat(v->szSIFormatBuffer, "\r\n");
		}
		if (v->cvct[nCVCTIndex].additional_descriptors_length)
		{
			BYTE * pAdditionalDescriptors = v->cvct[nCVCTIndex].pAdditionalDescriptors;
			int nDescriptorsLength = v->cvct[nCVCTIndex].additional_descriptors_length;

			lstrcat(v->szSIFormatBuffer, "Additional descriptors:\n");
			while (nDescriptorsLength)
			{
				int nDescriptor = pAdditionalDescriptors[0];
				int nDescriptorLength = pAdditionalDescriptors[1];
				char szDescriptor[128];

				DecodeDescriptorNames(szDescriptor, nDescriptor);
				wsprintf(szTemp, " Descriptor: %s", szDescriptor);
				lstrcat(v->szSIFormatBuffer, szTemp);

				DecodeMPEG2Descriptor(&pAdditionalDescriptors[2], FALSE);
				pAdditionalDescriptors += nDescriptorLength + 2;
				nDescriptorsLength -= nDescriptorLength + 2;			
			}
		}
	}

	return v->szSIFormatBuffer;
}

char * FormatBAT(int nBATIndex)
{
	v->szSIFormatBuffer[0] = '\0';
	if (nBATIndex < MAX_BAT_ENTRIES)
	{
		int nTSIndex;
		char szTemp[256];

		wsprintf(v->szSIFormatBuffer, "Bouquet ID %d (0x%04x) Version: %d\r\n", 
			     v->bat[nBATIndex].bouquet_id, v->bat[nBATIndex].bouquet_id,
				 v->bat[nBATIndex].version_number);
		if (v->bat[nBATIndex].bouquet_descriptors_length)
		{
			int nDescriptorsLength = v->bat[nBATIndex].bouquet_descriptors_length;
			BYTE * pDescriptors = v->bat[nBATIndex].bouquet_descriptors;

			while (nDescriptorsLength)
			{
				int nDescriptor = pDescriptors[0];
				int nDescriptorLength = pDescriptors[1];
				char szDescriptor[128];

				DecodeDescriptorNames(szDescriptor, nDescriptor);
				wsprintf(szTemp, "\r\nDescriptor: %s\r\n", szDescriptor);
				lstrcat(v->szSIFormatBuffer, szTemp);

				DecodeMPEG2Descriptor(&pDescriptors[0], FALSE);
				pDescriptors += nDescriptorLength + 2;
				nDescriptorsLength -= nDescriptorLength + 2;
			}
		}

		for (nTSIndex = 0; nTSIndex < MAX_BAT_TRANSPORT_ITEMS; nTSIndex++)
		{
			if (v->bat[nBATIndex].batts[nTSIndex].transport_stream_id == 0)
				break;
			wsprintf(szTemp, "Bouquet Transport Stream: %d (0x%04x) Network: %d (0x%04x)\r\n",
				     v->bat[nBATIndex].batts[nTSIndex].transport_stream_id, v->bat[nBATIndex].batts[nTSIndex].transport_stream_id,
					 v->bat[nBATIndex].batts[nTSIndex].original_network_id, v->bat[nBATIndex].batts[nTSIndex].original_network_id);
			lstrcat(v->szSIFormatBuffer, szTemp);
			if (v->bat[nBATIndex].batts[nTSIndex].transport_descriptors_length)
			{
				int nDescriptorsLength = v->bat[nBATIndex].batts[nTSIndex].transport_descriptors_length;
				BYTE * pDescriptors = v->bat[nBATIndex].batts[nTSIndex].transport_descriptors;

				while (nDescriptorsLength)
				{
					int nDescriptor = pDescriptors[0];
					int nDescriptorLength = pDescriptors[1];
					char szDescriptor[128];

					DecodeDescriptorNames(szDescriptor, nDescriptor);
					wsprintf(szTemp, "\r\nDescriptor: %s\r\n", szDescriptor);
					lstrcat(v->szSIFormatBuffer, szTemp);

					DecodeMPEG2Descriptor(&pDescriptors[0], FALSE);
					pDescriptors += nDescriptorLength + 2;
					nDescriptorsLength -= nDescriptorLength + 2;
				}
			}
		}
	}
	return v->szSIFormatBuffer;
}

char * FormatMGT(void)
{
	int i;

	v->szSIFormatBuffer[0] = '\0';

	for (i = 0; i < MAX_MGT_ENTRIES; i++)
	{
		char szTemp[128];
		char szTableName[50];

		if (v->mgt[i].nTableType == -1)
			break;
		
		if (v->mgt[i].nTableType == 0x0000)
			lstrcpy(szTableName, "Terrestrial VCT");
		else if (v->mgt[i].nTableType == 0x0001)
			lstrcpy(szTableName, "Terrestrial VCT");
		else if (v->mgt[i].nTableType == 0x0002)
			lstrcpy(szTableName, "Cable VCT");
		else if (v->mgt[i].nTableType == 0x0003)
			lstrcpy(szTableName, "Cable VCT");
		else if (v->mgt[i].nTableType == 0x0004)
			lstrcpy(szTableName, "Channel ETT");
		else if ( (v->mgt[i].nTableType >= 0x0100) && (v->mgt[i].nTableType <= 0x017f) )
			wsprintf(szTableName, "EIT-%d", v->mgt[i].nTableType - 0x100);
		else if ( (v->mgt[i].nTableType >= 0x0200) && (v->mgt[i].nTableType <= 0x027f) )
			wsprintf(szTableName, "Event ETT-%d", v->mgt[i].nTableType - 0x200);
		else if ( (v->mgt[i].nTableType >= 0x0301) && (v->mgt[i].nTableType <= 0x03ff) )
			wsprintf(szTableName, "RRT with rating_region %d", v->mgt[i].nTableType - 0x300);
		else if ( (v->mgt[i].nTableType >= 0x0400) && (v->mgt[i].nTableType <= 0x0fff) )
			lstrcpy(szTableName, "User Private");
		else
			lstrcpy(szTableName, "Reserved for future ATSC use");

		wsprintf(szTemp, "%s (%04x) on PID 0x%04x\r\n", szTableName, v->mgt[i].nTableType, v->mgt[i].nTablePID);
		lstrcat(v->szSIFormatBuffer, szTemp);
		if (v->mgt[i].pDescriptors != NULL)
		{
			int nDescriptorsLength = v->mgt[i].nDescriptorsLength;
			int nDescriptorOffset = 0;
			while (nDescriptorsLength)
			{
				int nDescriptor = v->mgt[i].pDescriptors[nDescriptorOffset];
				int nThisDescriptorLength = v->mgt[i].pDescriptors[nDescriptorOffset + 1] + 2;
				char szTemp[256];
				char szDescriptor[128];

				DecodeDescriptorNames(szDescriptor, nDescriptor);
				wsprintf(szTemp, " Descriptor: %s\r\n", szDescriptor);
				lstrcat(v->szSIFormatBuffer, szTemp);

				DecodeMPEG2Descriptor(&v->mgt[i].pDescriptors[nDescriptorOffset], FALSE);
				nDescriptorOffset += nThisDescriptorLength;
				nDescriptorsLength -= nThisDescriptorLength;
			}
		}
	}

	if (v->pMGTDescriptors != NULL)
	{
		int nDescriptorsLength = v->nMGTDescriptorsLength;
		int nDescriptorOffset = 0;
		
		lstrcat(v->szSIFormatBuffer, "\r\n");
		while (nDescriptorsLength)
		{
			int nDescriptor = v->pMGTDescriptors[nDescriptorOffset];
			int nThisDescriptorLength = v->pMGTDescriptors[nDescriptorOffset + 1] + 2;
			char szTemp[256];
			char szDescriptor[128];

			DecodeDescriptorNames(szDescriptor, nDescriptor);
			wsprintf(szTemp, " Descriptor: %s\r\n", szDescriptor);
			lstrcat(v->szSIFormatBuffer, szTemp);

			DecodeMPEG2Descriptor(&v->pMGTDescriptors[nDescriptorOffset], FALSE);
			nDescriptorOffset += nThisDescriptorLength;
			nDescriptorsLength -= nThisDescriptorLength;
		}
	}
	return v->szSIFormatBuffer;
}

char * FormatSITEntry(int nItemIndex)
{
	char szFrequencyBand[20];
	char szPolarizationType[10];
	char szHemisphere[2];

	switch(v->sit[nItemIndex].frequency_band)
	{
	case 0:
		lstrcpy(szFrequencyBand, "C Band");
		break;
	case 1:
		lstrcpy(szFrequencyBand, "Ku Band (FSS)");
		break;
	case 2:
		lstrcpy(szFrequencyBand, "Ku Band (BSS)");
		break;
	case 3:
		lstrcpy(szFrequencyBand, "Reserved");
		break;
	default:
		wsprintf(szFrequencyBand, "?%02x?", v->sit[nItemIndex].frequency_band);
		break;
	}

	if (!v->sit[nItemIndex].hemisphere)
		lstrcpy(szHemisphere, "W");
	else
		lstrcpy(szHemisphere, "E");

	if (!v->sit[nItemIndex].polarization_type)
		lstrcpy(szPolarizationType, "Linear");
	else
		lstrcpy(szPolarizationType, "Circular");

	sprintf(v->szSIFormatBuffer, "Satellite ID: %d\r\nYou are here: %s\r\nFrequency Band: %s\r\nOut of Service: %s\r\nOrbital Position: %.3fº %s\r\nPolarization Type: %s\r\nNumber of transponders: %d\r\n",
		    v->sit[nItemIndex].satellite_ID, 
			TrueFalseString(v->sit[nItemIndex].you_are_here),
			szFrequencyBand,
			TrueFalseString(v->sit[nItemIndex].out_of_service),
			(double)v->sit[nItemIndex].orbital_position / 10.0,
			szHemisphere,
			szPolarizationType,
			v->sit[nItemIndex].number_of_transponders + 1);
	
	return v->szSIFormatBuffer;
}

char * FormatRRTEntry(int nRegion)
{
	int i;
	char szTemp[512];

	v->szSIFormatBuffer[0] = '\0';
	if (v->prrt[nRegion] != NULL)
	{
		wsprintf(szTemp, "Region Name: %s\r\n", v->prrt[nRegion]->szRegionName);
		lstrcpy(v->szSIFormatBuffer, szTemp);
		
		for (i = 0; i < v->prrt[nRegion]->nDimensionsDefined; i++)
		{
			wsprintf(szTemp, "Dimension Name: %s\r\n", v->prrt[nRegion]->rrtdimension[i].szRatingDimension);
			lstrcat(v->szSIFormatBuffer, szTemp);
			{
				int j;

				for (j = 0; j < v->prrt[nRegion]->rrtdimension[i].nRatingsDefined; j++)
				{
					wsprintf(szTemp, " %s (%s)\r\n", 
							 v->prrt[nRegion]->rrtdimension[i].rrtrating[j].szAbbreviatedRating,
							 v->prrt[nRegion]->rrtdimension[i].rrtrating[j].szRating);
					lstrcat(v->szSIFormatBuffer, szTemp);
				}
			}
		}
	}

	return v->szSIFormatBuffer;
}

char * FormatCDTEntry(int nItemIndex)
{
	double dFrequencyUnit, dFirstCarrierFrequency;
	double dSpacingUnit, dFrequencySpacing;
	int i;
	char szTemp[40];

	dFirstCarrierFrequency = (double)v->cdt[nItemIndex].first_carrier_frequency;
	dFrequencySpacing = (double)v->cdt[nItemIndex].frequency_spacing;

	if (v->cdt[nItemIndex].frequency_unit)
		dFrequencyUnit = 125.0;
	else
		dFrequencyUnit = 10.0;
	if (v->cdt[nItemIndex].spacing_unit)
		dSpacingUnit = 125.0;
	else
		dSpacingUnit = 10.0;

	wsprintf(v->szSIFormatBuffer, "Carriers defined %d\r\n", v->cdt[nItemIndex].number_of_carriers);
	for (i = 0; i < v->cdt[nItemIndex].number_of_carriers; i++)
	{
		sprintf(szTemp, "Carrier %.3f MHz\r\n", (dFirstCarrierFrequency * dFrequencyUnit) / 1000.0);
		lstrcat(v->szSIFormatBuffer, szTemp);

		dFirstCarrierFrequency += (dFrequencySpacing * dSpacingUnit);
	}

	return v->szSIFormatBuffer;
}

char * FormatDVBTOT()
{
	int nDescriptorsLength = v->dvbtot.nDescriptorsLength;
	
	if (!nDescriptorsLength)
		v->szSIFormatBuffer[0] = '\0';
	else
	{
		int nDescriptorOffset = 0;

		lstrcpy(v->szSIFormatBuffer, "TOT Descriptors:\r\n");
		while (nDescriptorsLength)
		{
			int nDescriptor = v->dvbtot.pDescriptors[nDescriptorOffset];
			int nThisDescriptorLength = v->dvbtot.pDescriptors[nDescriptorOffset + 1] + 2;
			char szTemp[256];
			char szDescriptor[128];

			DecodeDescriptorNames(szDescriptor, nDescriptor);
			wsprintf(szTemp, "\r\nDescriptor: %s\r\n", szDescriptor);
			lstrcat(v->szSIFormatBuffer, szTemp);

			DecodeMPEG2Descriptor(&v->dvbtot.pDescriptors[nDescriptorOffset], FALSE);
			nDescriptorOffset += nThisDescriptorLength;
			nDescriptorsLength -= nThisDescriptorLength;
		}
	}	
	return v->szSIFormatBuffer;

}

char * FormatTDTEntry(int nItemIndex)
{
	char szPolarization[10];
	char szTemp[250];
	char szFrequency[10];
	double dFirstCarrier;
	double dMultiplier;

	if (!v->tdt[nItemIndex].polarization)
		lstrcpy(szPolarization, "H/LHCP");
	else
		lstrcpy(szPolarization, "V/RHCP");

	dFirstCarrier = (double)v->cdt[v->tdt[nItemIndex].CDT_reference].first_carrier_frequency;
	if (v->cdt[v->tdt[nItemIndex].CDT_reference].frequency_unit)
		dMultiplier = 125.0;
	else
		dMultiplier = 10.0;
	sprintf(szFrequency, "%.3f MHz", (dFirstCarrier * dMultiplier) / 1000.0);

	wsprintf(v->szSIFormatBuffer, "Satellite ID: %d\r\nPolarization: %s\r\nTransponder Number: %d\r\nFrequency from CDT: %s\r\n",
		     v->tdt[nItemIndex].satellite_ID,
			 szPolarization,
			 v->tdt[nItemIndex].transponder_number,
			 szFrequency);


	if (!v->tdt[nItemIndex].transport_type)
	{
		char szFEC[10];
		char szSplitMode[10];
		char szMMTParameters[128];
		
		DecodeFEC(v->mmt[v->tdt[nItemIndex].MMT_reference - 1].inner_coding_mode, szFEC, TRUE);

		if (v->mmt[nItemIndex].split_bitstream_mode)
			lstrcpy(szSplitMode, "Split");
		else
			lstrcpy(szSplitMode, "Combo");

		wsprintf(szMMTParameters, "%d KSps %s %s",
			     v->mmt[v->tdt[nItemIndex].MMT_reference].symbol_rate / 1000,
				 szFEC,
				 szSplitMode);
		
		wsprintf(szTemp, "MPEG-2 transport\r\nModulation: %s\r\nVCT_ID = %d\r\nRoot Transponder: %s\r\n",
			     szMMTParameters,
				 v->tdt[nItemIndex].VCT_ID,
				 TrueFalseString(v->tdt[nItemIndex].root_transponder));
	}
	else
	{
		char szWaveformStandard[20];
		char szWideBandwidthAudio[10];
		char szMatrixMode[20];

		switch(v->tdt[nItemIndex].waveform_standard)
		{
		case 0:
			lstrcpy(szWaveformStandard, "unknown");
			break;
		case 1:
			lstrcpy(szWaveformStandard, "NTSC");
			break;
		case 2:
			lstrcpy(szWaveformStandard, "PAL 625");
			break;
		case 3:
			lstrcpy(szWaveformStandard, "PAL 525");
			break;
		case 4:
			lstrcpy(szWaveformStandard, "SECAM");
			break;
		case 5:
			lstrcpy(szWaveformStandard, "D2-MAC");
			break;
		case 6:
			lstrcpy(szWaveformStandard, "B-MAC");
			break;
		case 7:
			lstrcpy(szWaveformStandard, "C-MAC");
			break;
		case 8:
			lstrcpy(szWaveformStandard, "DCI");
			break;
		case 9:
			lstrcpy(szWaveformStandard, "VideoCipher");
			break;
		case 10:
			lstrcpy(szWaveformStandard, "RCA DSS");
			break;
		case 11:
			lstrcpy(szWaveformStandard, "Orion");
			break;
		case 12:
			lstrcpy(szWaveformStandard, "Leitch");
			break;
		default:
			lstrcpy(szWaveformStandard, "reserved");
			break;
		}

		if (v->tdt[nItemIndex].wide_bandwidth_audio)
			lstrcpy(szWideBandwidthAudio, "wide");
		else
			lstrcpy(szWideBandwidthAudio, "narrow");

		switch(v->tdt[nItemIndex].matrix_mode)
		{
		case 0:
			lstrcpy(szMatrixMode, "mono");
			break;
		case 1:
			lstrcpy(szMatrixMode, "discrete stereo");
			break;
		case 2:
			lstrcpy(szMatrixMode, "matrix stereo");
			break;
		case 3:
			lstrcpy(szMatrixMode, "reserved");
			break;
		}

		sprintf(szTemp, "Non-MPEG-2 transport\r\nWide-bandwidth video: %s\r\nWaveform standard: %s\r\nAudio bandwidth: %s\r\nCompanded Audio: %s\r\nMatrix Mode: %s\r\nAudio Subcarrier 1: %.3f MHz\r\nAudio Subcarrier 2: %.3f MHz\r\n",
			     TrueFalseString(v->tdt[nItemIndex].wide_bandwidth_video),
				 szWaveformStandard,
				 szWideBandwidthAudio,
				 TrueFalseString(v->tdt[nItemIndex].companded_audio),
				 szMatrixMode,
				 5.0 + ((double)v->tdt[nItemIndex].subcarrier_1_offset / 100.0),
				 5.0 + ((double)v->tdt[nItemIndex].subcarrier_2_offset / 100.0));
	}
	lstrcat(v->szSIFormatBuffer, szTemp);



	return v->szSIFormatBuffer;
}

char * FormatMMTEntry(int nItemIndex)
{
	char szFEC[20];
	char szSplitBitstream[10];
	char szTransmissionSystem[20];
	char szModulationFormat[20];

	if (v->mmt[nItemIndex].split_bitstream_mode)
		lstrcpy(szSplitBitstream, "True");
	else
		lstrcpy(szSplitBitstream, "False");
	DecodeFEC(v->mmt[nItemIndex].inner_coding_mode, szFEC, TRUE);

	switch(v->mmt[nItemIndex].transmission_system)
	{
	case 0:
		lstrcpy(szTransmissionSystem, "unknown");
		break;
	case 1:
		lstrcpy(szTransmissionSystem, "ITU-T annex A");
		break;
	case 2:
		lstrcpy(szTransmissionSystem, "ITU-T annex B");
		break;
	case 3:
		lstrcpy(szTransmissionSystem, "ITU-R");
		break;
	case 4:
		lstrcpy(szTransmissionSystem, "ATSC");
		break;
	case 5:
		lstrcpy(szTransmissionSystem, "DigiCipher");
		break;
	case 6:
		lstrcpy(szTransmissionSystem, "ITU-T annex C");
		break;
	case 7:
		lstrcpy(szTransmissionSystem, "ITU-T annex D");
		break;
	default:
		lstrcpy(szTransmissionSystem, "reserved");
		break;
	}

	switch(v->mmt[nItemIndex].modulation_format)
	{
	case 0:
		lstrcpy(szModulationFormat, "uknown");
		break;
	case 1:
		lstrcpy(szModulationFormat, "QPSK");
		break;
	case 2:
		lstrcpy(szModulationFormat, "BPSK");
		break;
	case 3:
		lstrcpy(szModulationFormat, "OQPSK");
		break;
	case 4:
		lstrcpy(szModulationFormat, "VSB 8");
		break;
	case 5:
		lstrcpy(szModulationFormat, "VSB 16");
		break;
	case 6:
		lstrcpy(szModulationFormat, "QAM 16");
		break;
	case 7:
		lstrcpy(szModulationFormat, "QAM 32");
		break;
	case 8:
		lstrcpy(szModulationFormat, "QAM 64");
		break;
	case 9:
		lstrcpy(szModulationFormat, "QAM 80");
		break;
	case 10:
		lstrcpy(szModulationFormat, "QAM 96");
		break;
	case 11:
		lstrcpy(szModulationFormat, "QAM 112");
		break;
	case 12:
		lstrcpy(szModulationFormat, "QAM 128");
		break;
	case 13:
		lstrcpy(szModulationFormat, "QAM 160");
		break;
	case 14:
		lstrcpy(szModulationFormat, "QAM 192");
		break;
	case 15:
		lstrcpy(szModulationFormat, "QAM 224");
		break;
	case 16:
		lstrcpy(szModulationFormat, "QAM 256");
		break;
	case 17:
		lstrcpy(szModulationFormat, "QAM 320");
		break;
	case 18:
		lstrcpy(szModulationFormat, "QAM 384");
		break;
	case 19:
		lstrcpy(szModulationFormat, "QAM 448");
		break;
	case 20:
		lstrcpy(szModulationFormat, "QAM 512");
		break;
	case 21:
		lstrcpy(szModulationFormat, "QAM 640");
		break;
	case 22:
		lstrcpy(szModulationFormat, "QAM 768");
		break;
	case 23:
		lstrcpy(szModulationFormat, "QAM 896");
		break;
	case 24:
		lstrcpy(szModulationFormat, "QAM 1024");
		break;
	default:
		lstrcpy(szModulationFormat, "reserved");
		break;
	}

	wsprintf(v->szSIFormatBuffer,
		     "Transmission system: %s\r\nInner coding rate: %s\r\nSplit stream mode: %s\r\nModulation Format: %s\r\nSymbol Rate %d KSps\r\n",
		     szTransmissionSystem,
			 szFEC,
			 szSplitBitstream,
			 szModulationFormat,
			 v->mmt[nItemIndex].symbol_rate / 1000);

	return v->szSIFormatBuffer;
}

char * FormatPMTEntry(int nPMTIndex, BOOL fHTMLMode)
{
	if (v->pat.pmt[nPMTIndex].nProgramNumber == 0)
	{		
		wsprintf(v->szSIFormatBuffer, "Network PMT Entry - table on PID %d (0x%04x)",
			     v->pat.pmt[nPMTIndex].nPMTPID, v->pat.pmt[nPMTIndex].nPMTPID);
	}
	else
	{
		int nLCN = GetLogicalChannelNumber(v->pat.pmt[nPMTIndex].nProgramNumber);
		int i;
		char szTemp[200];
		char szLogicalChannelNumber[64] = {0};

		if (nLCN != 0)
			wsprintf(szLogicalChannelNumber, "%d/", nLCN);

		wsprintf(v->szSIFormatBuffer, "Program Number: %s%d\r\nPCR on PID %d (0x%04x)\r\nPMT Version: %d\r\n",
			     szLogicalChannelNumber,
			     v->pat.pmt[nPMTIndex].nProgramNumber,
				 v->pat.pmt[nPMTIndex].nPCRPID,
				 v->pat.pmt[nPMTIndex].nPCRPID,
				 v->pat.pmt[nPMTIndex].nVersionNumber);
		if (v->pChannelData[v->pat.pmt[nPMTIndex].nProgramNumber] != NULL)
		{
			wsprintf(szTemp, "Service name: %s\r\n", v->pChannelData[v->pat.pmt[nPMTIndex].nProgramNumber]->szShortName);
			lstrcat(v->szSIFormatBuffer, szTemp);
		}
		if (nLCN)
		{
			wsprintf(szTemp, "Logical channel number: %d\r\n", nLCN);
			lstrcat(v->szSIFormatBuffer, szTemp);
		}

		for (i = 0; i < MAX_ESLIST_ENTRIES; i++)
		{
			char szStreamTypeEnglish[64];

			if (v->pat.pmt[nPMTIndex].es[i].nESPID == 0)
				break;

			DecodeStreamType(v->pat.pmt[nPMTIndex].es[i].nStreamType, szStreamTypeEnglish, nPMTIndex, i);
			wsprintf(szTemp, "\r\nStream Type: 0x%02x %s\r\n Elementary Stream PID %d (0x%04x)\r\n",
				     v->pat.pmt[nPMTIndex].es[i].nStreamType,
					 szStreamTypeEnglish,
					 v->pat.pmt[nPMTIndex].es[i].nESPID,
					 v->pat.pmt[nPMTIndex].es[i].nESPID);
			lstrcat(v->szSIFormatBuffer, szTemp);
		}

		if (v->pat.pmt[nPMTIndex].nProgramInfoLength)
		{
			int nProgramInfoLength = v->pat.pmt[nPMTIndex].nProgramInfoLength;
			int nOffset = 0;
			while (nProgramInfoLength)
			{
				int nDescriptor = v->pat.pmt[nPMTIndex].pProgramInfo[nOffset];
				int nDescriptorLength = v->pat.pmt[nPMTIndex].pProgramInfo[nOffset + 1];
				char szDescriptor[128];

				DecodeDescriptorNames(szDescriptor, nDescriptor);
				wsprintf(szTemp, "\r\nDescriptor: %s\r\n", szDescriptor);
				lstrcat(v->szSIFormatBuffer, szTemp);

				DecodeMPEG2Descriptor(&v->pat.pmt[nPMTIndex].pProgramInfo[nOffset], FALSE);
				nOffset += nDescriptorLength + 2;
				nProgramInfoLength -= nDescriptorLength + 2;			
			}
		}
	}

	return v->szSIFormatBuffer;
}

void FormatAACAudioParse(PPARSEDAACAUDIO pAAC, char * szOutput)
{
	char szUnknown[] = {"Unknown"};
	char * szSBR = szUnknown;
	char * szObjectType = szUnknown;
	char * szHeaderType = szUnknown;

	switch(pAAC->sbr)
	{
	case 0:		// NO_SBR
		szSBR = "No SBR";
		break;
	case 1:		// SBR_UPSAMPLED
		szSBR = "Upsampled SBR";
		break;
	case 2:		// SBR_DOWNSAMPLED
		szSBR = "Downsampled SBR";
		break;
	case 3:		// NO_SBR_UPSAMPLED
		szSBR = "No SBR, upsampled";
		break;
	}

	switch(pAAC->object_type)
	{
	case 1:		// MAIN
		szObjectType = "Main";
		break;
	case 2:		// LC
		szObjectType = "Low Complexity";
		break;
	case 3:		// SSR
		szObjectType = "Scalable Sample Rate";
		break;
	case 4:		// LTP
		szObjectType = "Long Term Prediction";
		break;
	case 5:		// HE_AAC
		szObjectType = "High Efficiency (SBR)";
		break;
	case 17:	// ER_LC
		szObjectType = "Error Resilient Low Complexity";
		break;
	case 19:	// ER_LTP
		szObjectType = "Error Resilient Long Term Prediction";
		break;
	case 23:	// LD
		szObjectType = "Low Delay";
		break;
	}

	switch(pAAC->header_type)
	{
	case 0:			// RAW
		szHeaderType = "None";
		break;
	case 1:			// ADIF
		szHeaderType = "ADIF";
		break;
	case 2:			// ADTS
		szHeaderType = "ADTS";
		break;
	}

	wsprintf(szOutput, "AAC Audio: Channels: %d Sample Rate: %d SBR: %s\r\n"
		               "AAC Audio: Object Type: %s Header Type: %s\r\n"
					   "AAC Audio: Front: %d Side: %d Back: %d LFE: %d\r\n",
			 pAAC->channels, pAAC->samplerate, szSBR,
			 szObjectType, szHeaderType,
			 pAAC->num_front_channels, pAAC->num_side_channels, pAAC->num_back_channels, pAAC->num_lfe_channels);
}

void FormatH264VideoParse(int nPMTIndex, int nESIndex, char * szOutput)
{
	PPARSEDH264VIDEO pH264 = (PPARSEDH264VIDEO)v->pat.pmt[nPMTIndex].es[nESIndex].pParsedData;
	
	wsprintf(szOutput, "H.264 Video: Resolution %d x %d Interlaced: %d\r\n", pH264->horizontal_size_value, pH264->vertical_size_value, pH264->interlaced);
}

void FormatMPEG4VideoParse(int nPMTIndex, int nESIndex, char * szOutput)
{
	PPARSEDMPEG4VIDEO pMPEG4 = (PPARSEDMPEG4VIDEO)v->pat.pmt[nPMTIndex].es[nESIndex].pParsedData;
	
	wsprintf(szOutput, "MPEG-4 Video: Resolution %d x %d\r\n", pMPEG4->horizontal_size_value, pMPEG4->vertical_size_value);
}

void FormatVC1VideoParse(int nPMTIndex, int nESIndex, char * szOutput)
{
	PPARSEDVC1VIDEO pVC1 = (PPARSEDVC1VIDEO)v->pat.pmt[nPMTIndex].es[nESIndex].pParsedData;
	
	wsprintf(szOutput, "VC-1 Video: Resolution %d x %d Interlaced: %d\r\n", pVC1->horizontal_size_value, pVC1->vertical_size_value, pVC1->interlaced);
}

void GetMPEG2VideoAspectRation(int aspect_ratio_information, char * szAspectRatio)
{
	switch(aspect_ratio_information)
	{
	case 0:
		lstrcpy(szAspectRatio, "forbidden");
		break;
	case 1:
		lstrcpy(szAspectRatio, "1:1");
		break;
	case 2:
		lstrcpy(szAspectRatio, "4:3");
		break;
	case 3:
		lstrcpy(szAspectRatio, "16:9");
		break;
	case 4:
		lstrcpy(szAspectRatio, "2.21:1");
		break;
	default:
		lstrcpy(szAspectRatio, "reserved");
		break;
	}
}

void GetMPEG2FrameRate(int frame_rate_code, char * szFrameRate)
{
	switch(frame_rate_code)
	{
	case 0:
		lstrcpy(szFrameRate, "Forbidden");
		break;
	case 1:
		lstrcpy(szFrameRate, "23.976");
		break;
	case 2:
		lstrcpy(szFrameRate, "24");
		break;
	case 3:
		lstrcpy(szFrameRate, "25");
		break;
	case 4:
		lstrcpy(szFrameRate, "29.97");
		break;
	case 5:
		lstrcpy(szFrameRate, "30");
		break;
	case 6:
		lstrcpy(szFrameRate, "50");
		break;
	case 7:
		lstrcpy(szFrameRate, "59.94");
		break;
	case 8:
		lstrcpy(szFrameRate, "60");
		break;
	default:
		lstrcpy(szFrameRate, "Reserved");
		break;
	}
}

void GetMPEG2ChromaFormat(int chroma_format, char * szChromaFormat)
{
	switch(chroma_format)
	{
	case 0:
		lstrcpy(szChromaFormat, "reserved");
		break;
	case 1:
		lstrcpy(szChromaFormat, "4:2:0");
		break;
	case 2:
		lstrcpy(szChromaFormat, "4:2:2");
		break;
	case 3:
		lstrcpy(szChromaFormat, "4:4:4");
		break;
	}
}

void GetAFDFormat(DWORD dwAFDData, char * szAFDFormat)
{
	switch(dwAFDData)
	{
	default:
		lstrcpy(szAFDFormat, "reserved");
		break;
	case 2:
		lstrcpy(szAFDFormat, "box 16:9 (top)");
		break;
	case 3:
		lstrcpy(szAFDFormat, "box 14:9 (top)");
		break;
	case 4:
		lstrcpy(szAFDFormat, "box > 16:9 (center)");
		break;
	case 8:
		lstrcpy(szAFDFormat, "Active format is the same as the coded frame");
		break;
	case 9:
		lstrcpy(szAFDFormat, "4:3 (center)");
		break;
	case 10:
		lstrcpy(szAFDFormat, "16:9 (center)");
		break;
	case 11:
		lstrcpy(szAFDFormat, "14:9 (center)");
		break;
	case 13:
		lstrcpy(szAFDFormat, "4:3 (with shoot & protect 14:9 center)");
		break;
	case 14:
		lstrcpy(szAFDFormat, "16:9 (with shoot & protect 14:9 center)");
		break;
	case 15:
		lstrcpy(szAFDFormat, "16:9 (with shoot & protect 4:3 center)");
		break;
	}
}

void FormatMPEGVideoParse(int nPMTIndex, int nESIndex, char * szOutput)
{
	PPARSEDMPEGVIDEO pMPEG = (PPARSEDMPEGVIDEO)v->pat.pmt[nPMTIndex].es[nESIndex].pParsedData;
	int bit_rate;
	int vbv_buffer_size;
	int horizontal_size;
	int vertical_size;
	char szInterlacedOrProgressive[2];
	char szFrameRate[20] = {0};
	char szAspectRatio[20] = {0};
	char szChromaFormat[20] = {0};

	if (pMPEG->marker_bit2)
	{
		// we had an extension header
		bit_rate = (pMPEG->bit_rate_extension << 18 | pMPEG->bit_rate_value) * 400;
		vbv_buffer_size = (pMPEG->vbv_buffer_size_extension << 10 | pMPEG->vbv_buffer_size_value) * 2048;
		horizontal_size = pMPEG->horizontal_size_extension << 12 | pMPEG->horizontal_size_value;
		vertical_size = pMPEG->vertical_size_extension << 12 | pMPEG->vertical_size_value;
	}
	else
	{
		bit_rate = pMPEG->bit_rate_value * 400;
		vbv_buffer_size = pMPEG->vbv_buffer_size_value * 2048;
		horizontal_size = pMPEG->horizontal_size_value;
		vertical_size = pMPEG->vertical_size_value;
	}

	if (pMPEG->progressive_sequence == 1)
		lstrcpy(szInterlacedOrProgressive, "p");
	else
		lstrcpy(szInterlacedOrProgressive, "i");

	GetMPEG2VideoAspectRation(pMPEG->aspect_ratio_information, szAspectRatio);
	GetMPEG2FrameRate(pMPEG->frame_rate_code, szFrameRate);
	GetMPEG2ChromaFormat(pMPEG->chroma_format, szChromaFormat);
	sprintf(szOutput, "MPEG Video: Bitrate %.3f Mbps Resolution %d x %d%s\r\nMPEG Video: Framerate %s fps Aspect Ratio %s Chroma Format %s\r\n",
		     (double)bit_rate / 1000.0 / 1000.0,
			 horizontal_size,
			 vertical_size,
			 szInterlacedOrProgressive,
			 szFrameRate,
			 szAspectRatio,
			 szChromaFormat);

	// See about AFD
	if (v->pat.pmt[nPMTIndex].es[nESIndex].nTeletextServices & VBI_SERVICE_AFD)
	{
		char szAFDFormat[128] = {0};
		char szAFDString[256];

		GetAFDFormat(v->pat.pmt[nPMTIndex].es[nESIndex].dwAFDData, szAFDFormat);
		wsprintf(szAFDString, "AFD descriptor: %s\r\n", szAFDFormat);
		lstrcat(szOutput, szAFDString);
	}
}

void GetMPEGAudioMode(int mode, char * szMode)
{
	switch(mode)
	{
	case 0:
		lstrcpy(szMode, "Stereo");
		break;
	case 1:
		lstrcpy(szMode, "Joint Stereo");
		break;
	case 2:
		lstrcpy(szMode, "Dual Channel");
		break;
	case 3:
		lstrcpy(szMode, "Single Channel");
		break;
	}
}

void GetMPEGAudioSamplingFrequency(int sampling_frequency, char * szSamplingRate)
{
	switch(sampling_frequency)
	{
	case 0:
		lstrcpy(szSamplingRate, "44.1");
		break;
	case 1:
		lstrcpy(szSamplingRate, "48");
		break;
	case 2:
		lstrcpy(szSamplingRate, "32");
		break;
	default:
		lstrcpy(szSamplingRate, "reserved");
		break;
	}
}

void FormatMPEGAudioParse(PPARSEDMPEGAUDIO pMPEG, char * szOutput)
{
	int bitrate = -1;
	char szLayer[10];
	char szSamplingRate[10];
	char szMode[20];
	
	switch(pMPEG->layer)
	{
	case 3:		// layer 1
		bitrate = nLayer1Rates[pMPEG->bitrate_index];
		lstrcpy(szLayer, "I");
		break;
	case 2:		// layer 2
		bitrate = nLayer2Rates[pMPEG->bitrate_index];
		lstrcpy(szLayer, "II");
		break;
	case 1:		// layer 3
		bitrate = nLayer3Rates[pMPEG->bitrate_index];
		lstrcpy(szLayer, "III");
		break;
	default:
		lstrcpy(szLayer, "Reserved");
		break;
	}

	GetMPEGAudioMode(pMPEG->mode, szMode);
	GetMPEGAudioSamplingFrequency(pMPEG->sampling_frequency, szSamplingRate);

	wsprintf(szOutput, "MPEG1 Audio: Bitrate %d Kbps Sample Rate %s KHz\r\nMPEG1 Audio: Layer %s Mode %s\r\n",
		     bitrate,
			 szSamplingRate,
			 szLayer,
			 szMode);	
}

void GetAC3fscod(int fscod, char * szSamplingRate)
{
	switch(fscod)
	{
	case 0:
		lstrcpy(szSamplingRate, "48");
		break;
	case 1:
		lstrcpy(szSamplingRate, "44.1");
		break;
	case 2:
		lstrcpy(szSamplingRate, "32");
		break;
	default:
		lstrcpy(szSamplingRate, "reserved");
		break;
	}
}

void GetAC3bsmod(int bsmod, int acmod, char * szBitstreamMode)
{
	switch(bsmod)
	{
	case 0:
		lstrcpy(szBitstreamMode, "complete main");
		break;
	case 1:
		lstrcpy(szBitstreamMode, "music and effects");
		break;
	case 2:
		lstrcpy(szBitstreamMode, "visually impaired");
		break;
	case 3:
		lstrcpy(szBitstreamMode, "hearing impaired");
		break;
	case 4:
		lstrcpy(szBitstreamMode, "dialogue");
		break;
	case 5:
		lstrcpy(szBitstreamMode, "commentary");
		break;
	case 6:
		lstrcpy(szBitstreamMode, "emergency");
		break;
	case 7:
		switch(acmod)
		{
		case 0:
			lstrcpy(szBitstreamMode, "reserved");
			break;
		case 1:
			lstrcpy(szBitstreamMode, "voice over");
			break;
		default:
			lstrcpy(szBitstreamMode, "karaoke");
			break;
		}
		break;
	}
}

void GetAC3acmod(int acmod, char * szAudioCodingMode)
{
	switch(acmod)
	{
	case 0:
		lstrcpy(szAudioCodingMode, "1+1 Ch1, Ch2");
		break;
	case 1:
		lstrcpy(szAudioCodingMode, "1/0 C");
		break;
	case 2:
		lstrcpy(szAudioCodingMode, "2/0 L, R");
		break;
	case 3:
		lstrcpy(szAudioCodingMode, "3/0 L, C, R");
		break;
	case 4:
		lstrcpy(szAudioCodingMode, "2/1 L, R, S");
		break;
	case 5:
		lstrcpy(szAudioCodingMode, "3/1 L, C, R, S");
		break;
	case 6:
		lstrcpy(szAudioCodingMode, "2/2 L, R, SL, SR");
		break;
	case 7:
		lstrcpy(szAudioCodingMode, "3/2 5 L, C, R, SL, SR");
		break;
	}
}
void GetAC3cmixlev(int cmixlev, char * szCMixLev)
{
	switch(cmixlev)
	{
	case -1:
		szCMixLev[0] = '\0';
		break;
	case 0:
		lstrcpy(szCMixLev, "-3.0 dB ");
		break;
	case 1:
		lstrcpy(szCMixLev, "-4.5 dB ");
		break;
	case 2:
		lstrcpy(szCMixLev, "-6.0 dB ");
		break;
	case 3:
		lstrcpy(szCMixLev, "Reserved ");
		break;
	}
}

void GetAC3surmixlev(int surmixlev, char * szSurMixLev)
{
	switch(surmixlev)
	{
	case -1:
		szSurMixLev[0] = '\0';
		break;
	case 0:
		lstrcpy(szSurMixLev, "-3.0 dB ");
		break;
	case 1:
		lstrcpy(szSurMixLev, "-6.0 dB ");
		break;
	case 2:
		lstrcpy(szSurMixLev, "0 dB ");
		break;
	case 3:
		lstrcpy(szSurMixLev, "Reserved ");
		break;
	}
}

void GetAC3dsurmod(int dsurmod, char * szDSurMod)
{
	switch(dsurmod)
	{
	case -1:
		szDSurMod[0] = '\0';
		break;
	case 0:
		lstrcpy(szDSurMod, "not indicated ");
		break;
	case 1:
		lstrcpy(szDSurMod, "Not Dolby Surround ");
		break;
	case 2:
		lstrcpy(szDSurMod, "Dolby Surround ");
		break;
	case 3:
		lstrcpy(szDSurMod, "Reserved ");
		break;
	}
}

void FormatAC3Parse(PPARSEDAC3AUDIO pAC3, char * szOutput)
{
	int nDialNorm = 0;
	char szSamplingRate[16];
	char szBitstreamMode[32];
	char szAudioCodingMode[32];
	char szCMixLev[32] = {"n/a"};
	char szSurMixLev[32] = {"n/a"};
	char szDSurMod[32] = {"n/a"};
	char szLFEMod[16] = {"n/a"};
	char szDialNorm[32] = {"n/a"};
	char szOptionalLine[256] = {0};

	GetAC3fscod(pAC3->fscod, szSamplingRate);
	GetAC3bsmod(pAC3->bsmod, pAC3->acmod, szBitstreamMode);
	GetAC3acmod(pAC3->acmod, szAudioCodingMode);

	GetAC3cmixlev(pAC3->cmixlev, szCMixLev);
	GetAC3surmixlev(pAC3->surmixlev, szSurMixLev);
	GetAC3dsurmod(pAC3->dsurmod, szDSurMod);

	if (lstrlen(szCMixLev))
	{
		if (!lstrlen(szOptionalLine))
			lstrcpy(szOptionalLine, "AC3: ");
		lstrcat(szOptionalLine, "Center Mix Level ");
		lstrcat(szOptionalLine, szCMixLev);
	}
	if (lstrlen(szSurMixLev))
	{
		if (!lstrlen(szOptionalLine))
			lstrcpy(szOptionalLine, "AC3: ");
		lstrcat(szOptionalLine, "Surround Mix Level ");
		lstrcat(szOptionalLine, szSurMixLev);
	}
	if (lstrlen(szDSurMod))
	{
		if (!lstrlen(szOptionalLine))
			lstrcpy(szOptionalLine, "AC3: ");
		lstrcat(szOptionalLine, "Dolby Surround Mode ");
		lstrcat(szOptionalLine, szDSurMod);
	}
	if (lstrlen(szOptionalLine))
		lstrcat(szOptionalLine, "\r\n");

	if (pAC3->lfeon)
		lstrcpy(szLFEMod, "On");
	else
		lstrcpy(szLFEMod, "Off");
	
	nDialNorm = pAC3->dialnorm;
	if (nDialNorm == 0)
		nDialNorm = 31;
	wsprintf(szDialNorm, "-%d dB", nDialNorm);
	if (pAC3->dialnorm == 0)
		lstrcat(szDialNorm, " (actually 0)");

	wsprintf(szOutput, "AC3: Bitrate %d Kbps Sample Rate %s KHz\r\nAC3: Mode %s Coding %s\r\n%sAC3: LFE Mode %s Dialogue normalization %s\r\n",
		     AC3frmsizecod_tbl[pAC3->frmsizecod].bit_rate,
			 szSamplingRate,
			 szBitstreamMode,
			 szAudioCodingMode,
			 szOptionalLine,
			 szLFEMod,
			 szDialNorm);
}

char * FormatESEntry(int nESPID)
{
	int nPMTIndex, nESIndex;

	v->szSIFormatBuffer[0] = '\0';

	for (nPMTIndex = 0; nPMTIndex < MAX_PAT_ENTRIES; nPMTIndex++)
	{
		if (v->pat.pmt[nPMTIndex].nPMTPID == 0)
			break;
		for (nESIndex = 0; nESIndex < MAX_ESLIST_ENTRIES; nESIndex++)
		{
			if (v->pat.pmt[nPMTIndex].es[nESIndex].nESPID == 0)
				break;
			if (v->pat.pmt[nPMTIndex].es[nESIndex].nESPID == nESPID)
			{
				char szStreamTypeEnglish[64];
				DecodeStreamType(v->pat.pmt[nPMTIndex].es[nESIndex].nStreamType, szStreamTypeEnglish, nPMTIndex, nESIndex);

				wsprintf(v->szSIFormatBuffer, "Elementary Stream PID %d (0x%04x) %s\r\n",
					     v->pat.pmt[nPMTIndex].es[nESIndex].nESPID,
						 v->pat.pmt[nPMTIndex].es[nESIndex].nESPID,
						 szStreamTypeEnglish);				
				switch(v->pat.pmt[nPMTIndex].es[nESIndex].nBlacklisted)
				{
				default:
					break;
				case BLACKLIST_NO_TRAFFIC:
					lstrcat(v->szSIFormatBuffer, "**ES blacklisted - no traffic on PID**\r\n");
					break;
				case BLACKLIST_NO_DATA:
					lstrcat(v->szSIFormatBuffer, "**ES blacklisted - no appreciable data**\r\n");
					break;
				case BLACKLISTED_NO_PES_PACKETS:
					lstrcat(v->szSIFormatBuffer, "**ES blacklisted - no PES packets**\r\n");
					break;
				}

				if (v->pat.pmt[nPMTIndex].es[nESIndex].pParsedData != NULL)
				{
					char szParseDecode[256] = {0};

					switch(v->pat.pmt[nPMTIndex].es[nESIndex].nParseType)
					{
					case PARSE_ES_TYPE_MPEG2_VIDEO:
						FormatMPEGVideoParse(nPMTIndex, nESIndex, szParseDecode);
						break;
					case PARSE_ES_TYPE_MPEG_AUDIO:
						FormatMPEGAudioParse((PPARSEDMPEGAUDIO)v->pat.pmt[nPMTIndex].es[nESIndex].pParsedData, szParseDecode);
						break;
					case PARSE_ES_TYPE_AC3_AUDIO:
						FormatAC3Parse((PPARSEDAC3AUDIO)v->pat.pmt[nPMTIndex].es[nESIndex].pParsedData, szParseDecode);
						break;
					case PARSE_ES_TYPE_MPEG2_AAC_AUDIO:
					case PARSE_ES_TYPE_MPEG4_AAC_AUDIO:
						FormatAACAudioParse((PPARSEDAACAUDIO)v->pat.pmt[nPMTIndex].es[nESIndex].pParsedData, szParseDecode);
						break;
					case PARSE_ES_TYPE_H264_VIDEO:
						FormatH264VideoParse(nPMTIndex, nESIndex, szParseDecode);
						break;
					case PARSE_ES_TYPE_MPEG4_VIDEO:
						FormatMPEG4VideoParse(nPMTIndex, nESIndex, szParseDecode);
						break;
					case PARSE_ES_TYPE_VC1_VIDEO:
						FormatVC1VideoParse(nPMTIndex, nESIndex, szParseDecode);
						break;
					}
					if (lstrlen(szParseDecode))
						lstrcat(v->szSIFormatBuffer, szParseDecode);
				}
				if (v->pat.pmt[nPMTIndex].es[nESIndex].pDescriptors)
				{
					char szTemp[128];
					BYTE * pDescriptorData = v->pat.pmt[nPMTIndex].es[nESIndex].pDescriptors;
					int nDescriptorsLength = v->pat.pmt[nPMTIndex].es[nESIndex].nDescriptorsLength;
					int nCurrentIndex = 0;

					do
					{
						char szDescriptor[128];
						int nThisDescriptorLength = (BYTE)pDescriptorData[nCurrentIndex + 1];

						DecodeDescriptorNames(szDescriptor, pDescriptorData[nCurrentIndex]);
						wsprintf(szTemp, "\r\nDescriptor: %s\r\n", szDescriptor);
						lstrcat(v->szSIFormatBuffer, szTemp);

						DecodeMPEG2Descriptor(&pDescriptorData[nCurrentIndex], FALSE);

						nCurrentIndex += (BYTE)pDescriptorData[nCurrentIndex + 1]; // descriptor length
						nCurrentIndex += 2;	// descriptor tag and length
					} while (nCurrentIndex < nDescriptorsLength);
				}
				nPMTIndex = MAX_PAT_ENTRIES;
				break;
			}
		}
	}


	return v->szSIFormatBuffer;
}

void GetTSIDName(char * szTSIDName, int nTransportStreamID)
{
	int i;

	for (i = 0; tsid[i].nNTSCTSID != 0; i++)
	{
		if (tsid[i].nNTSCTSID + 1 == nTransportStreamID)
		{
			wsprintf(szTSIDName, "NTSC: %02d ATSC: %02d %s", tsid[i].nNTSCChannel, tsid[i].nATSCChannel, tsid[i].szLocale);
			return;
		}
	}
	lstrcpy(szTSIDName, "");
}

char * FormatSDTEntry(int nChannelNumber, BOOL fHTMLMode)
{
	int i;

	if (fHTMLMode)
	{
		if (v->pChannelData[nChannelNumber]->fATSC == FALSE)
		{
			char szQuickNIT[128] = {0};

			QuickFormatNIT(szQuickNIT, v->pChannelData[nChannelNumber]->nTransportStreamID, FALSE);
			if (lstrlen(szQuickNIT))
			{
				char szTemp[128];
				wsprintf(szTemp, "<A HREF=\"#nit_%d\">%s</A>", v->pChannelData[nChannelNumber]->nTransportStreamID, szQuickNIT);
				lstrcpy(szQuickNIT, szTemp);
			}
			wsprintf(v->szSIFormatBuffer, "<A NAME=\"sdt_%d\"></A>SDT Channel %d\r\nService Name: %s\r\nProvider Name: %s\r\nTransport Stream ID: %d (0x%04x) %s\r\n\r\nOriginal Network ID: %d (0x%04x)\r\n",
					 v->pChannelData[nChannelNumber]->nChannelNumber,
					 v->pChannelData[nChannelNumber]->nChannelNumber,
					 v->pChannelData[nChannelNumber]->szShortName,
					 v->pChannelData[nChannelNumber]->szLongName,
					 v->pChannelData[nChannelNumber]->nTransportStreamID,
					 v->pChannelData[nChannelNumber]->nTransportStreamID,
					 szQuickNIT,
					 v->pChannelData[nChannelNumber]->nOriginalNetworkID,
					 v->pChannelData[nChannelNumber]->nOriginalNetworkID);
		}
		else
		{
			char szModulationMode[48];
			char szTSIDName[128];

			FormatATSCModulationMode(szModulationMode, v->pChannelData[nChannelNumber]->nModulationMode);
			GetTSIDName(szTSIDName, v->pChannelData[nChannelNumber]->nTransportStreamID);
			wsprintf(v->szSIFormatBuffer, "<A NAME=\"sdt_%d\"></A>Channel %d\r\nService Name: %s\r\nTSID: %d (0x%04x) %s\r\nChannel Number: %d.%d\r\nCarrier Frequency: %d\r\nModulation Mode: %s\r\nSource ID: %d\r\n",
					 v->pChannelData[nChannelNumber]->nChannelNumber,
					 v->pChannelData[nChannelNumber]->nChannelNumber,
					 v->pChannelData[nChannelNumber]->szShortName,
					 v->pChannelData[nChannelNumber]->nTransportStreamID,
					 v->pChannelData[nChannelNumber]->nTransportStreamID,
					 szTSIDName,
					 v->pChannelData[nChannelNumber]->nMajorChannelNumber,
					 v->pChannelData[nChannelNumber]->nMinorChannelNumber,
					 v->pChannelData[nChannelNumber]->nCarrierFrequency,
					 szModulationMode,
					 v->pChannelData[nChannelNumber]->nSourceID);
		}
	}
	else
	{
		if (v->pChannelData[nChannelNumber]->fATSC == FALSE)
		{
			int nLCN = GetLogicalChannelNumber(nChannelNumber);
			char szQuickNIT[128] = {0};
			char szFromTableDescription[48];
			char szChannelString[48];

			if (nLCN)
				wsprintf(szChannelString, "%d/%d", nLCN, nChannelNumber);
			else
				wsprintf(szChannelString, "%d", nChannelNumber);
			QuickFormatNIT(szQuickNIT, v->pChannelData[nChannelNumber]->nTransportStreamID, FALSE);
			if (v->pChannelData[nChannelNumber]->nFromTable == 0x42)
				lstrcpy(szFromTableDescription, "current mux");
			else
				lstrcpy(szFromTableDescription, "another mux");
			wsprintf(v->szSIFormatBuffer, "Channel %s\r\nOn Table_ID: 0x%02x (%s)\r\nService Name: %s\r\nProvider Name: %s\r\nTransport Stream ID: %d (0x%04x) %s\r\n",
					 szChannelString,
					 v->pChannelData[nChannelNumber]->nFromTable,
					 szFromTableDescription,
					 v->pChannelData[nChannelNumber]->szShortName,
					 v->pChannelData[nChannelNumber]->szLongName,
					 v->pChannelData[nChannelNumber]->nTransportStreamID,
					 v->pChannelData[nChannelNumber]->nTransportStreamID,
					 szQuickNIT);
		}
		else
		{
			char szModulationMode[48];
			char szTSIDName[128];

			FormatATSCModulationMode(szModulationMode, v->pChannelData[nChannelNumber]->nModulationMode);
			GetTSIDName(szTSIDName, v->pChannelData[nChannelNumber]->nTransportStreamID);
			wsprintf(v->szSIFormatBuffer, "Channel %d\r\nService Name: %s\r\nTSID: %d (0x%04x) %s\r\nChannel Number: %d.%d\r\nCarrier Frequency: %d\r\nModulation Mode: %s\r\nSource ID: %d\r\n",
					 v->pChannelData[nChannelNumber]->nChannelNumber,
					 v->pChannelData[nChannelNumber]->szShortName,
					 v->pChannelData[nChannelNumber]->nTransportStreamID,
					 v->pChannelData[nChannelNumber]->nTransportStreamID,
					 szTSIDName,
					 v->pChannelData[nChannelNumber]->nMajorChannelNumber,
					 v->pChannelData[nChannelNumber]->nMinorChannelNumber,
					 v->pChannelData[nChannelNumber]->nCarrierFrequency,
					 szModulationMode,
					 v->pChannelData[nChannelNumber]->nSourceID);
			if (lstrlen(v->pChannelData[nChannelNumber]->szATSCChannelETT))
			{
				char szTemp[512];
				wsprintf(szTemp, "Channel ETT: %s\r\n", v->pChannelData[nChannelNumber]->szATSCChannelETT);
				lstrcat(v->szSIFormatBuffer, szTemp);
			}
		}
	}

	if (fHTMLMode)
		lstrcat(v->szSIFormatBuffer, "<FONT SIZE=\"-1\">");
	for (i = 0; i < MAX_SDT_EXTRA_DESCRIPTORS; i++)
	{
		if (v->pChannelData[nChannelNumber]->pExtraDescriptors[i] != NULL)
		{
			switch(v->pChannelData[nChannelNumber]->pExtraDescriptors[i][0])
			{
			case 0x48:	// name descriptor (we already decoded this)
				break;
			default:
				{
					char szDescriptor[128];
					char szTemp[256];
					DecodeDescriptorNames(szDescriptor, v->pChannelData[nChannelNumber]->pExtraDescriptors[i][0]);
					wsprintf(szTemp, "\r\nDescriptor: %s\r\n", szDescriptor);
					lstrcat(v->szSIFormatBuffer, szTemp);

					DecodeMPEG2Descriptor(v->pChannelData[nChannelNumber]->pExtraDescriptors[i], fHTMLMode);
				}
				break;
			}
		}
	}

	if (fHTMLMode)
		lstrcat(v->szSIFormatBuffer, "</FONT>");

	return v->szSIFormatBuffer;
}

char * FormatPAT(BOOL fIncludeHTMLTags, int nExportSITables)
{
	int i;
	//int nNITIndex;
	char szTemp[256] = {0};

	wsprintf(v->szSIFormatBuffer, "PAT Version Number: %d\r\nTransport Stream ID: %d (0x%04x)\r\n",
			 v->pat.nVersionNumber,
		     v->pat.nTransportStreamID,
			 v->pat.nTransportStreamID);

	if (QuickFormatNIT(szTemp, v->pat.nTransportStreamID, TRUE) == TRUE)
		lstrcat(v->szSIFormatBuffer, szTemp);

	lstrcat(v->szSIFormatBuffer, "\r\n");

	for (i = 0; i < MAX_PAT_ENTRIES; i++)
	{
		if (v->pat.pmt[i].nPMTPID == 0)
			break;
		if (v->pat.pmt[i].nProgramNumber)
		{
			if (v->pat.pmt[i].nPMTPID == -2)
			{
				if (fIncludeHTMLTags)
					wsprintf(szTemp, "Manual channel - <A HREF=\"#pmt_%d\">Program %d</A> ", v->pat.pmt[i].nProgramNumber, v->pat.pmt[i].nProgramNumber);
				else
					wsprintf(szTemp, "Manual channel - Program %d ", v->pat.pmt[i].nProgramNumber);
			}
			else
			{
				int nLCN = GetLogicalChannelNumber(v->pat.pmt[i].nProgramNumber);
				char szLogicalChannelNumber[64] = {0};
				if (nLCN != 0)
					wsprintf(szLogicalChannelNumber, "%d/", nLCN);

				if (fIncludeHTMLTags)
					wsprintf(szTemp, "PMT PID %d (0x%04x) - <A HREF=\"#pmt_%d\">Program %s%d</A> ", v->pat.pmt[i].nPMTPID, v->pat.pmt[i].nPMTPID, v->pat.pmt[i].nProgramNumber, szLogicalChannelNumber, v->pat.pmt[i].nProgramNumber);
				else
					wsprintf(szTemp, "PMT PID %d (0x%04x) - Program %s%d ", v->pat.pmt[i].nPMTPID, v->pat.pmt[i].nPMTPID, szLogicalChannelNumber, v->pat.pmt[i].nProgramNumber);
			}
		}
		else
		{
			if ( ((nExportSITables & 8) == 8) && fIncludeHTMLTags)
				wsprintf(szTemp, "PMT PID %d (0x%04x) - <A HREF=\"#nit\">Network</A> ", v->pat.pmt[i].nPMTPID, v->pat.pmt[i].nPMTPID);
			else
				wsprintf(szTemp, "PMT PID %d (0x%04x) - Network ", v->pat.pmt[i].nPMTPID, v->pat.pmt[i].nPMTPID);
		}
		if (v->pChannelData[v->pat.pmt[i].nProgramNumber] != NULL)
			lstrcat(szTemp, v->pChannelData[v->pat.pmt[i].nProgramNumber]->szShortName);
		if (fIncludeHTMLTags && v->pat.pmt[i].nProgramNumber && (nExportSITables & 0x20) && v->pEvents[v->pat.pmt[i].nProgramNumber] != NULL)
		{
			char szTemp2[128];
			wsprintf(szTemp2, " <FONT SIZE=\"-1\"><A HREF=\"#eit_%d\">EIT Link</A></FONT>", v->pat.pmt[i].nProgramNumber);
			lstrcat(szTemp, szTemp2);
		}
		lstrcat(szTemp, "\r\n");
		lstrcat(v->szSIFormatBuffer, szTemp);
	}

	return v->szSIFormatBuffer;
}
