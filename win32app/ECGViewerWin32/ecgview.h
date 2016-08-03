#pragma once
#include <windows.h>
#include <math.h>
#include "logging.h"

long signalBuffer[100000];
int maxSamples;
int padding_x;
int sampleFrequency;

void DrawGrid(HDC hdc, HWND hwnd);
void DrawGridLines(HDC hdc, HWND hwnd, int interval);
void DrawSignal(HDC hdc, HWND hwnd);
void DrawTrackStartTime(HDC hDeviceContext, HWND hWindow, int trackIndex, RECT positionRect);

int PointsPerTrack(HDC hdc, HWND hwnd);
int TrackWidthPx(HWND hwnd);
int EcgBigSquarePx();
double EcgSmallSquarePx();
int ScaleSignalXToPixels(int sampleIndex);