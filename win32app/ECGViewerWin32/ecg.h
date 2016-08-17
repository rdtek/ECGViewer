#pragma once
#define _USE_MATH_DEFINES
#include <windows.h>
#include <math.h>
#include "appconstants.h"
#include "windowtools.h"
#include "logging.h"

typedef struct {
    LPCSTR label;
    double samples[1000000];
    int numberOfSamples;
} HeartSignal;

//int padding_x;
//int sampleFrequency;

void InitECG(HWND hWindow);
int IncrementPagination();
int DecrementPagination();

void GenerateSignal(HeartSignal* ptr_signal);
void GenerateLine(HeartSignal* ptr_signal, size_t length, double gradient);
double CalculateGausianPDF(double x, double mu, double variance);
void GenerateGausianCurve(HeartSignal* ptr_signal, double mu, double variance, double scaleFactor);

void SetECGSignal(HeartSignal* signal);
void DrawGrid(HDC hdc);
void DrawGridLines(HDC hdc, int interval);
void DrawSignal(HDC hDeviceContext);
void DrawTrackStartTime(HDC hDeviceContext, int trackIndex, RECT positionRect);

int MaxTracksPerPage();
int PointsPerTrack();
int TrackWidthPx();
int TrackHeightPx();
int ScaleSignalXToPixels(int sampleIndex);
int EcgBigSquarePx();
double EcgSmallSquarePx();
int TotalPages();
int PaddingBottom();