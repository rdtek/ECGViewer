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

HWND CreateButtonW(HWND hWindow, int btnId, LPCSTR lpButtonText, int x, int y, int width, int height) {
    return CreateWindowW(L"BUTTON", lpButtonText,
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        x, y, width, height, hWindow, (HMENU)btnId,
        GetModuleHandle(NULL), NULL);
}