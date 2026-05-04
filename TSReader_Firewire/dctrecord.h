#pragma once

#define MAX_DEVICES 8

typedef struct _CAPDEVICES {
    char DeviceName[MAX_PATH];
} CAPDEVICES;

int dctrecord(void);
HRESULT EnumCaptureSources(void);
