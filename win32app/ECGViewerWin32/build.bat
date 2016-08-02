@ECHO OFF
set "APP_NAME=ECGViewerWin32"
set "OUTPUT_DIR=Release"

if not exist %OUTPUT_DIR% mkdir %OUTPUT_DIR%

echo Building %APP_NAME%
gcc -D"UNICODE" -D"_UNICODE" wndmain.c ecgview.c logging.c -lgdi32 -mwindows -std=gnu99 -o %OUTPUT_DIR%\%APP_NAME%.exe

echo %OUTPUT_DIR%\%APP_NAME%.exe