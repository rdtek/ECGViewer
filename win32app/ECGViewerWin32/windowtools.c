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
    RECT windowRect;
    int windowWidth = -1;
    if (GetWindowRect(hWindow, &windowRect)) {
        windowWidth = windowRect.right - windowRect.left;
    }
    return windowWidth;
}

int GetWindowHeight(HWND hWindow) {
    RECT windowRect;
    int windowHeight = -1;
    if (GetWindowRect(hWindow, &windowRect)) {
        windowHeight = windowRect.bottom - windowRect.top;
    }
    return windowHeight;
}