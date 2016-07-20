#ifndef HEADER_ECG
#define HEADER_ECG

#include <windows.h>
#include <math.h>
#include "logging.h"

long signalBuffer[10000];
int maxSamples;
int signalLoaded;

void DrawSignalPanel(HDC hdc, HWND hwnd);
void DrawGridLines(HDC hdc, HWND hwnd, int interval);
void DrawSignal(HDC hdc, HWND hwnd);
int PointsPerTrack();
int TrackWidthPx();
int EcgBigSquarePx();
int ScaleSignalXToPixels(int sampleIndex);

#endif
