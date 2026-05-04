#include <windows.h>
#include <commctrl.h>
#include <limits.h>
#include "TSReader.h"
#include "util.h"
#include "bcdmux.h"

#include "resource.h"

extern PVARIABLES v;
extern char gszDecimalString[];
extern char gszHexString[];

BOOL CALLBACK AudioThumbnailSettingsDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		{
			char szPMTValue[128], szTableValue[128];

			CheckDlgButton(hDlg, IDC_AUDIO_THUMBNAILS_ENABLED, v->fAudioThumbnails);
			if (v->fDecimalPIDs)
			{
				SetDlgItemText(hDlg, IDC_AUDIO_THUMBNAIL_BASE, gszDecimalString);
				SetDlgItemText(hDlg, IDC_AUDIO_THUMBNAIL_BASE2, gszDecimalString);
				wsprintf(szPMTValue, "%d", v->nSongTitleParserPMT);
				wsprintf(szTableValue, "%d", v->nSongTitleParserTable);
			}
			else
			{
				SetDlgItemText(hDlg, IDC_AUDIO_THUMBNAIL_BASE, gszHexString);
				SetDlgItemText(hDlg, IDC_AUDIO_THUMBNAIL_BASE2, gszHexString);
				wsprintf(szPMTValue, "%02x", v->nSongTitleParserPMT);
				wsprintf(szTableValue, "%02x", v->nSongTitleParserTable);
			}
			SetDlgItemText(hDlg, IDC_AUDIO_TITLE_PMT_TYPE, szPMTValue);
			SetDlgItemText(hDlg, IDC_AUDIO_TITLE_MPEG_TABLE_NUMBER, szTableValue);
			CheckDlgButton(hDlg, IDC_TITLE_PARSER_ENABLED, v->fSongTitleParserEnabled);
			EnableWindow(GetDlgItem(hDlg, IDC_AUDIO_TITLE_PMT_TYPE), v->fSongTitleParserEnabled);
			EnableWindow(GetDlgItem(hDlg, IDC_AUDIO_TITLE_MPEG_TABLE_NUMBER), v->fSongTitleParserEnabled);
			SetFocus(GetDlgItem(hDlg, IDOK));
		}
		break;
	case WM_CLOSE:
		EndDialog(hDlg, FALSE);
		break;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDOK:
			{
				char * szMask;
				char szPMTValue[128], szTableValue[128];

				if (v->fDecimalPIDs)
					szMask = "%d";
				else
					szMask = "%x";
				GetDlgItemText(hDlg, IDC_AUDIO_TITLE_PMT_TYPE, szPMTValue, sizeof(szPMTValue));
				GetDlgItemText(hDlg, IDC_AUDIO_TITLE_MPEG_TABLE_NUMBER, szTableValue, sizeof(szTableValue));
				sscanf(szPMTValue, szMask, &v->nSongTitleParserPMT);
				sscanf(szTableValue, szMask, &v->nSongTitleParserTable);
				v->fAudioThumbnails = IsDlgButtonChecked(hDlg, IDC_AUDIO_THUMBNAILS_ENABLED);

				EndDialog(hDlg, TRUE);
			}
			break;
		case IDCANCEL:
			EndDialog(hDlg, FALSE);
			break;
		case IDC_TITLE_PARSER_ENABLED:
			v->fSongTitleParserEnabled = IsDlgButtonChecked(hDlg, IDC_TITLE_PARSER_ENABLED);
			EnableWindow(GetDlgItem(hDlg, IDC_AUDIO_TITLE_PMT_TYPE), v->fSongTitleParserEnabled);
			EnableWindow(GetDlgItem(hDlg, IDC_AUDIO_TITLE_MPEG_TABLE_NUMBER), v->fSongTitleParserEnabled);
			break;
		}
		break;
	}

	return FALSE;
}

void GetTitleString(char * szItem, int nBufferIndex, int nLength)
{
	while (nLength)
	{
		*szItem++ = get_bits(nBufferIndex, 8);
		nLength--;
	};
	*szItem = '\0';
}

void MakeAudioTitleThumbnail(int nES, char * szTitle, char * szArtist, char * szCD, char * szLabel)
{
	int nDestWidth = 240;
	int nDestHeight = 52;	
	int nESParsePMTIndex = v->nESParsePMTIndex[nES];
	int nESParseESIndex = v->nESParseESIndex[nES];
	DWORD dwChannelTextColor = RGB(0x00, 0xff, 0x00);
	BYTE *pThumbnail;

	switch(v->nThumbnailSize)
	{
	case 1:
		nDestWidth *= 2; nDestWidth /= 3;
		break;
	case 2:
		nDestWidth *= 2;
		break;
	}	
	pThumbnail = LocalAlloc(LPTR, nDestWidth * nDestHeight * 3);
	_ISDrawTextOnRGB2(pThumbnail,
					  nDestWidth,
					  nDestHeight,
					  szTitle,
					  &v->logfontChannelFont,
					  15,
					  1,
					  dwChannelTextColor);
	_ISDrawTextOnRGB2(pThumbnail,
					  nDestWidth,
					  nDestHeight,
					  szArtist,
					  &v->logfontChannelFont,
					  15,
					  10,
					  dwChannelTextColor);
	_ISDrawTextOnRGB2(pThumbnail,
					  nDestWidth,
					  nDestHeight,
					  szCD,
					  &v->logfontChannelFont,
					  15,
					  20,
					  dwChannelTextColor);
	_ISDrawTextOnRGB2(pThumbnail,
					  nDestWidth,
					  nDestHeight,
					  szLabel,
					  &v->logfontChannelFont,
					  15,
					  30,
					  dwChannelTextColor);


	EnterCriticalSection(&v->csThumbnails);
	if (v->pat.pmt[nESParsePMTIndex].es[nESParseESIndex].pRGBVideoFrame != NULL)
		LocalFree(v->pat.pmt[nESParsePMTIndex].es[nESParseESIndex].pRGBVideoFrame);
	else
	{
		v->nThumbnailImageCount++;
	}
	v->pat.pmt[nESParsePMTIndex].es[nESParseESIndex].pRGBVideoFrame = pThumbnail;
	v->pat.pmt[nESParsePMTIndex].es[nESParseESIndex].nVideoWidth = nDestWidth;
	v->pat.pmt[nESParsePMTIndex].es[nESParseESIndex].nVideoHeight = nDestHeight;
	LeaveCriticalSection(&v->csThumbnails);
	PostMessage(v->hDlgSIParser, WM_USER + 3, 0, 1);
}

BOOL DecodeAudioTitleData(BYTE * pSection, int nPacketLength, int nES)
{
	int pointer;
	int table_id;

	set_buf(BM_MPEG2_THREAD + nES, (BYTE *)pSection, 0, FALSE);
	pointer = get_bits(BM_MPEG2_THREAD + nES, 8);		// remember - raw section!

	pSection++;					// skip the pointer
	pSection += pointer;		// advance to the first byte of this section
	set_buf(BM_MPEG2_THREAD + nES, (BYTE *)pSection, 0, FALSE);
	table_id = get_bits(BM_MPEG2_THREAD + nES, 8);
	//if (table_id == v->nSongTitleParserTable)
	{
		switch(table_id)
		{
		case 0x9a:			// Music Choice on Comcast
			{
				int nSectionLength = ((pSection[1] << 8) + pSection[2]) & 0xfff;
				if ( (nSectionLength <= 0) || (nSectionLength > 65536) )
					return FALSE;
				if (SourceHelper_CRC_Check(pSection, nSectionLength + 3) != TRUE)
					return FALSE;
				{
					int section_syntax_indicator = get_bits(BM_MPEG2_THREAD + nES, 1);
					int reserved_for_future_use = get_bits(BM_MPEG2_THREAD + nES, 1);
					int reserved1 = get_bits(BM_MPEG2_THREAD + nES, 2);
					int section_length = get_bits(BM_MPEG2_THREAD + nES, 12);
					int unknown1 = get_bits(BM_MPEG2_THREAD + nES, 8);
					int language_code = get_bits(BM_MPEG2_THREAD + nES, 24);
					int unknown2 = get_bits(BM_MPEG2_THREAD + nES, 24);
					int unknown3 = get_bits(BM_MPEG2_THREAD + nES, 24);
					int unknown4 = get_bits(BM_MPEG2_THREAD + nES, 24);
					int unknown5 = get_bits(BM_MPEG2_THREAD + nES, 8);
					int remaining_length = get_bits(BM_MPEG2_THREAD + nES, 16);
					int title_length;
					int artist_length;
					int cd_length;
					int label_length;
					//int ad_length;
					char szTitle[64] = {""};
					char szArtist[64] = {""};
					char szCD[64] = {""};
					char szLabel[64] = {""};
					//char szAd[64] = {""};

					title_length = get_bits(BM_MPEG2_THREAD + nES, 16);
					GetTitleString(szTitle, BM_MPEG2_THREAD + nES, title_length);

					if (pSection[get_byte_pos(BM_MPEG2_THREAD + nES)] == 0x80)
						unknown1 = get_bits(BM_MPEG2_THREAD + nES, 8);
					artist_length = get_bits(BM_MPEG2_THREAD + nES, 16);
					GetTitleString(szArtist, BM_MPEG2_THREAD + nES, artist_length);
					
					if (pSection[get_byte_pos(BM_MPEG2_THREAD + nES)] == 0x80)
						unknown1 = get_bits(BM_MPEG2_THREAD + nES, 8);
					cd_length = get_bits(BM_MPEG2_THREAD + nES, 16);
					GetTitleString(szCD, BM_MPEG2_THREAD + nES, cd_length);

					if (pSection[get_byte_pos(BM_MPEG2_THREAD + nES)] == 0x80)
						unknown1 = get_bits(BM_MPEG2_THREAD + nES, 8);
					label_length = get_bits(BM_MPEG2_THREAD + nES, 16);
					GetTitleString(szLabel, BM_MPEG2_THREAD + nES, label_length);
					
/*					if (pSection[get_byte_pos(BM_MPEG2_THREAD + nES)] == 0x80)
						unknown1 = get_bits(BM_MPEG2_THREAD + nES, 8);
					ad_length = get_bits(BM_MPEG2_THREAD + nES, 16);
					GetTitleString(szAd, BM_MPEG2_THREAD + nES, ad_length);
					ad_length = ad_length;*/
					MakeAudioTitleThumbnail(nES, szTitle, szArtist, szCD, szLabel);
				}
			}
			return TRUE;
		}
	}


	return FALSE;
}
