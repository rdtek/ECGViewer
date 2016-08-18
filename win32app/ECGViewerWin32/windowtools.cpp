#include "windowtools.h"

void SaveWindowSize(HWND hwnd, WindowSize* windowSize) {
    RECT windowRect;
    if (GetWindowRect(hwnd, &windowRect)) {
        windowSize->width = windowRect.right - windowRect.left;
        windowSize->height = windowRect.bottom - windowRect.top;
    }
}

BOOL WindowSizeChanged(HWND hwnd, WindowSize* oldSize) {
    RECT windowRect;
    BOOL changed = 0;
    if (GetWindowRect(hwnd, &windowRect)) {
        if ((windowRect.right - windowRect.left) != oldSize->width
            || (windowRect.bottom - windowRect.top) != oldSize->height) {
            changed = 1;
        }
    }
    return changed;
}

int GetWindowWidth(HWND hWindow) {
    RECT rect;
    return GetWindowRect(hWindow, &rect) 
        ? GetRectWidth(rect) : -1;
}

int GetWindowHeight(HWND hWindow) {
    RECT rect;
    return GetWindowRect(hWindow, &rect) 
        ? GetRectHeight(rect) : -1;
}

int GetClientWidth(HWND hWindow) {
    RECT rect;
    return GetClientRect(hWindow, &rect) 
        ? GetRectWidth(rect) : -1;
}

int GetClientHeight(HWND hWindow) {
    RECT rect;
    return GetClientRect(hWindow, &rect)
        ? GetRectHeight(rect) : -1;
}

int GetRectWidth(RECT rect) {
    return rect.right - rect.left;
}

int GetRectHeight(RECT rect) {
    return rect.bottom - rect.top;
}

HWND CreateButtonW(HWND hWindow, int btnId, LPCWSTR lpButtonText, int x, int y, int width, int height) {
    return CreateWindow(L"BUTTON", lpButtonText,
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        x, y, width, height, hWindow, (HMENU)btnId,
        GetModuleHandle(NULL), NULL);
}

HWND CreateToolTip(int toolID, HINSTANCE hInst, HWND hWindow, PWSTR pszText)
{
    if (!toolID || !hWindow || !pszText)
    {
        return FALSE;
    }
    // Get the window of the tool.
    HWND hwndTool = GetDlgItem(hWindow, toolID);

    // Create the tooltip. g_hInst is the global instance handle.
    HWND hwndTip = CreateWindowEx(NULL, TOOLTIPS_CLASS, NULL,
        WS_POPUP | TTS_ALWAYSTIP | TTS_BALLOON,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        hWindow, NULL, hInst, NULL);

    if (!hwndTool || !hwndTip)
    {
        return (HWND)NULL;
    }

    // Associate the tooltip with the tool.
    TOOLINFO toolInfo = { 0 };
    toolInfo.cbSize = sizeof(toolInfo);
    toolInfo.hwnd = hWindow;
    toolInfo.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
    toolInfo.uId = (UINT_PTR)hwndTool;
    toolInfo.lpszText = pszText;
    
    if(!SendMessage(hwndTip, TTM_ADDTOOL, 0, (LPARAM)&toolInfo))
        MessageBox(0, TEXT("Failed: TTM_ADDTOOL"), 0, 0);

    return hwndTip;
}