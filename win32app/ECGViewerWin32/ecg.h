#pragma once
#include <windows.h>
#include <math.h>
#include "windowtools.h"
#include "logging.h"

typedef struct {
    LPCSTR label;
    short samples[1000000];
    int numberOfSamples;
} HeartSignal;

int maxSamples;
int padding_x;
int sampleFrequency;

void SetECGSignal(HeartSignal* signal);
void DrawGrid(HDC hdc);
void DrawGridLines(HDC hdc, int interval);
void DrawSignal(HDC hDeviceContext);
void DrawTrackStartTime(HDC hDeviceContext, int trackIndex, RECT positionRect);
int PointsPerTrack();
int TrackWidthPx();
int ScaleSignalXToPixels(int sampleIndex);
int EcgBigSquarePx();
double EcgSmallSquarePx();