APP_NAME="ECGViewerWin32"
OUTPUT_DIR="Release"
mkdir -p $OUTPUT_DIR
echo Building $APP_NAME
gcc -D"UNICODE" -D"_UNICODE" wndmain.c ecgview.c logging.c -lgdi32 -mwindows -std=gnu99 -o $OUTPUT_DIR/$APP_NAME.exe
echo $OUTPUT_DIR/$APP_NAME.exe