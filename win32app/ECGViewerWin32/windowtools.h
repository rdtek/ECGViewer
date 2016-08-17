#pragma once
#include <windows.h>

typedef struct { int width; int height; } WindowSize;

void SaveWindowSize
    (HWND hwnd, WindowSize* windowSize);

BOOL WindowSizeChanged
    (HWND hwnd, WindowSize* oldSize);

int GetWindowWidth
    (HWND hWindow);

int GetWindowHeight
    (HWND hWindow);

int GetClientWidth
    (HWND hWindow);

int GetClientHeight
    (HWND hWindow);

int GetRectWidth
    (RECT rect);

int GetRectHeight
    (RECT rect);

HWND CreateButtonW
    (HWND hWindow, int btnId, LPCWSTR lpButtonText, int x, int y, int width, int height);