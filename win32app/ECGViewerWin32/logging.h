#pragma once
#include <stdio.h>
#include <wchar.h>

void log_int(const char* note, int intVal);
void log_long(const char* note, long longVal);
void log_float(const char* note, float floatVal);
void log_dbl(const char* note, double doubleVal);
void log_wstr(const wchar_t* note, const wchar_t* strVal);