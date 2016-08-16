#pragma once
#define _USE_MATH_DEFINES
#include <windows.h>
#include <math.h>
#include "windowtools.h"
#include "logging.h"

typedef struct {
    LPCSTR label;
    double samples[1000000];
    int numberOfSamples;
} HeartSignal;

int maxSamples;
int padding_x;
int sampleFrequency;

void GenerateSignal(HeartSignal* ptr_signal);
void GenerateFlatLine(HeartSignal* ptr_signal, size_t length);
void GeneratePWave(HeartSignal* ptr_signal);
void GenerateQRSComplex(HeartSignal* ptr_signal);
void GenerateTWave(HeartSignal* ptr_signal);
double NormalDistributionPDF(double x, double mu, double variance);
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