#pragma once

LRESULT FAR PASCAL PIDPieChartWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT FAR PASCAL MuxrateChartWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT FAR PASCAL ActivePIDsChartWindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT FAR PASCAL VideoBitrateChartWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT FAR PASCAL ProgramUsageChartWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT FAR PASCAL VideoCompositionChartWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void InputMPEG2VideoCompositionESData(BYTE * pPESPacket, int nPESLength, int nChartIndex);
void InputH264VideoCompositionESData(BYTE * pPESPacket, int nPESLength, int nChartIndex);
BOOL GetVideoCompositionPID(HWND hWnd);
LRESULT FAR PASCAL SignalChartWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT FAR PASCAL PIDUsageVBRStackedAreaWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void AddDataToPIDUsageStackedChart(int nPIDUsageStackedAreaChartIndex);
