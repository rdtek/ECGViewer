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