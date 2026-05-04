#include <windows.h>
#include <hasp_hl.h>

#include "resource.h"

hasp_handle_t handle = 0;
unsigned char vendor_code[] =
"yndP2uWijYQMGG+hZCdCOXmTNSo7dJouB9ZIva4AVZj5pqY/F3iThD208T7zMp/l7hVXybbl/ujO2Xf/"
"0g6RX/2gJdUtQJFUDB0Ygx7ASLNQ1i/ISh6O88HoZqKGTDAmoRuRYOe16G03QQEsDQkY+BYeOLEqj+LF"
"Qx2m05v4zeWqvd+ReoW0XBgMih5vjCb+DplooTsMoXvgTkIag2EZTU6OIYkrZZ2p3EOHp3SwNbHgqZrl"
"k1MxZ/yJWCrCB26neFQx2QB23WoaDds+vKPMwrTAAxmCPkivKURRVL/LbDnegsIjqF+himXporeZ+FQK"
"OhN7r4VZSUFKu4dA4YTz18G9E+pemP7NUkBH5wcNXH7c6AqyKQmAynfmfwJju+dkdVz/XU342966UcQJ"
"aKR67ZGRk/uZHJbzLVchwNZE58sZZjKQEncpyBgTZ20yY4etFXjJtNgUZ8tOqp6ooJONKIN4VuQWl/YL"
"Rpvh6ow3EH5zUDF6WXOA59wxaRnXqjrGknJFHrUOUXSDfHFT1BkI9k02L1+MqDywHJh7a4vd7SfdbEY7"
"Q7hDOnKcMteAQyX1DRmIKllFGsNy25x+OL0PsiGp8GNovipQr0R2lOdVXM9lQ6N6+nmu2jeJceqVc34r"
"Cc1/AdvpZhL/RD4kfRaVZlQbSz7HWNsupp5tNG2mCaDGLK2pm5JXPhx5S92XV2AfFCIi9CBDyxoASB8P"
"6pfvAXklCaXH";

void UpdateKeyStatus(HWND hDlg)
{
	hasp_status_t status;
	unsigned char data[16];

	status = hasp_read(handle, HASP_FILEID_MAIN, 0, 16, data);
	status = hasp_decrypt(handle, data, 16);
	switch(data[15])
	{
	case 0xde:
		SetDlgItemText(hDlg, IDC_KEY_STATUS, "Initialized key - ready for customers");
		break;
	default:
		SetDlgItemText(hDlg, IDC_KEY_STATUS, "Virgin key - reset please");
		break;
	case 0xdb:
		{
			hasp_time_t time;
			hasp_time_t time_now;
			int day, month, year, hour, minute, second;
			char szTemp[128];
			char * szState;

			memcpy(&time, &data[0], sizeof(time));
			hasp_hasptime_to_datetime(time, &day, &month, &year, &hour, &minute, &second);

			status = hasp_get_rtc(handle, &time_now);
			if (time_now > time)
				szState = "Expired";
			else
				szState = "Expires";
			wsprintf(szTemp, "%s %04d/%02d/%02d %02d:%02d:%02d",
					 szState, year, month, day, hour, minute, second);
			SetDlgItemText(hDlg, IDC_KEY_STATUS, szTemp);
		}
		break;
	}
}

BOOL CALLBACK MainDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		{
			hasp_status_t status;
			const hasp_feature_t feature = HASP_PROGNUM_DEFAULT_FID | HASP_PROGNUM_OPT_NO_REMOTE;

			SetDlgItemText(hDlg, IDC_KEY_STATUS, "");
			SetDlgItemText(hDlg, IDC_KEY_TIME, "");
			
			status = hasp_login(feature, vendor_code, &handle);
			if (status == HASP_STATUS_OK)
			{
				hasp_status_t memSize = 0;

				status = hasp_get_size(handle, HASP_FILEID_MAIN, &memSize);
				if (memSize)
				{
					SetTimer(hDlg, 1, 1000, NULL);
					SendMessage(hDlg, WM_TIMER, 0, 0);
					UpdateKeyStatus(hDlg);
				}
				else
				{
					SetDlgItemText(hDlg, IDC_KEY_STATUS, "Incorrect key type");
					EnableWindow(GetDlgItem(hDlg, IDC_RESET_KEY), FALSE);
				}
			}
			else
			{
				SetDlgItemText(hDlg, IDC_KEY_STATUS, "No key detected");
				EnableWindow(GetDlgItem(hDlg, IDC_RESET_KEY), FALSE);
			}
		}
		break;
	case WM_DESTROY:
		if (handle)
		{
			KillTimer(hDlg, 1);
			hasp_logout(handle);
		}
		break;
	case WM_TIMER:
		{
			hasp_time_t time;
			hasp_status_t status;
			char szTemp[128];
			
			status = hasp_get_rtc(handle, &time);
			if (status == HASP_STATUS_OK)
			{
				int day, month, year, hour, minute, second;
				hasp_hasptime_to_datetime(time, &day, &month, &year, &hour, &minute, &second);
				wsprintf(szTemp, "%04d/%02d/%02d %02d:%02d:%02d",
					     year, month, day, hour, minute, second);
				SetDlgItemText(hDlg, IDC_KEY_TIME, szTemp);
			}
			else
			{
				SetDlgItemText(hDlg, IDC_KEY_TIME, "----");
			}
		}
		break;
	case WM_CLOSE:
		EndDialog(hDlg, FALSE);
		break;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDCANCEL:
			EndDialog(hDlg, FALSE);
			break;
		case IDC_RESET_KEY:
			{
				hasp_status_t status;
				unsigned char data[16];

				memset(&data, 0, sizeof(data));
				data[15] = 0xde;
				status = hasp_encrypt(handle, data, 16);
				status = hasp_write(handle, HASP_FILEID_MAIN, 0, 16, data);
				UpdateKeyStatus(hDlg);
			}
			break;
		}
		break;
	}

	return FALSE;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int nCmdShow)
{
	DialogBox(hInstance, MAKEINTRESOURCE(IDD_MAIN), NULL, MainDlgProc);
}
