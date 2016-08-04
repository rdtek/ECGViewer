#pragma once
#include <windows.h>
#include <stdio.h>
#include "ecg.h"

void DoOpenFile
    (HeartSignal* heartSignal, int maxNum, OPENFILENAME openFileName);

int CountFileLines
    (const wchar_t* fileName);