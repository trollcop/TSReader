#pragma once

/* copy from avcodec to make charting less stupid */
typedef enum tagAVPictureType {
    AV_PICTURE_TYPE_NONE = 0, ///< Undefined
    AV_PICTURE_TYPE_I,     ///< Intra
    AV_PICTURE_TYPE_P,     ///< Predicted
    AV_PICTURE_TYPE_B,     ///< Bi-dir predicted
    AV_PICTURE_TYPE_S,     ///< S(GMC)-VOP MPEG-4
    AV_PICTURE_TYPE_SI,    ///< Switching Intra
    AV_PICTURE_TYPE_SP,    ///< Switching Predicted
    AV_PICTURE_TYPE_BI,    ///< BI type
} eAVPictureType;

LRESULT FAR PASCAL PIDPieChartWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT FAR PASCAL MuxrateChartWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT FAR PASCAL ActivePIDsChartWindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT FAR PASCAL VideoBitrateChartWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT FAR PASCAL ProgramUsageChartWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT FAR PASCAL VideoCompositionChartWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT FAR PASCAL SignalChartWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT FAR PASCAL PIDUsageVBRStackedAreaWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void AddDataToPIDUsageStackedChart(int nPIDUsageStackedAreaChartIndex);
void Charting__UpdateQuickStyle(int nChartIndex);
