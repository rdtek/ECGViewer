#pragma once
#include <windows.h>
#include <stdio.h>
#include "ecg.h"

void DoOpenFile
(HeartSignal* heartSignal, int maxNum, LPWSTR fileNameOut);

int CountFileLines
    (const wchar_t* fileName);