#pragma once
#include <tchar.h>
#include <wchar.h>
#include <stdbool.h>
#include <windows.h>
#include "windowtools.h"
#include "logging.h"
#include "ecg.h"
#include "signalfileio.h"

#define MYCLASSNAME              L"MainWndClass"
#define MYWINDOWNAME             L"ECG Viewer (Win32)"
#define IDM_FILE_NEW             1
#define IDM_FILE_OPEN            2
#define IDM_FILE_QUIT            3
#define IDM_TOOLS_REFRESH        4
#define IDM_TOOLS_GENERATESIGNAL 5
#define IDC_PAGELEFT_BUTTON     101
#define IDC_PAGERIGHT_BUTTON    102

int windowWidth;
int windowHeight;
int wantDrawSignal = 0;
int signalLoaded = 0;

OPENFILENAME    openFileName;
HWND            hBtnPageLeft;
HWND            hBtnPageRight;

void AddMenus(HWND hwnd);
void DoRedraw(HWND hwnd);
VOID CALLBACK PaintTimerProc(HWND hwnd, UINT uMessage, UINT_PTR uEventId, DWORD dwTime);