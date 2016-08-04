#pragma once
#include <windows.h>
#include <math.h>
#include "logging.h"

typedef struct {
    LPCSTR label;
    short samples[1000000];
    int numberOfSamples;
} HeartSignal;

int maxSamples;
int padding_x;
int sampleFrequency;

void DrawGrid
    (HDC hdc, HWND hwnd);

void DrawGridLines
    (HDC hdc, HWND hwnd, int interval);

void DrawSignal
    (HDC hDeviceContext, HWND hWindow, HeartSignal* heartSignal);

void DrawTrackStartTime
    (HDC hDeviceContext, HWND hWindow, int trackIndex, RECT positionRect);

int PointsPerTrack
    (HDC hdc, HWND hwnd);

int TrackWidthPx
    (HWND hwnd);

int ScaleSignalXToPixels
    (int sampleIndex);

int EcgBigSquarePx();

double EcgSmallSquarePx();