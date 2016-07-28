#ifndef HEADER_ECG
#define HEADER_ECG

#include <windows.h>
#include <math.h>
#include "logging.h"

long signalBuffer[10000];
int maxSamples;
int PADDING_X;
int sampleFrequency;

void DrawGrid(HDC hdc, HWND hwnd);
void DrawGridLines(HDC hdc, HWND hwnd, int interval);
void DrawSignal(HDC hdc, HWND hwnd);
int PointsPerTrack(HDC hdc, HWND hwnd);
int TrackWidthPx(HDC hdc, HWND hwnd);
int EcgBigSquarePx();
int ScaleSignalXToPixels(int sampleIndex);

#endif
