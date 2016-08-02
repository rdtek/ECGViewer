del app.exe
gcc -D"UNICODE" -D"_UNICODE" wndmain.c ecgview.c logging.c -lgdi32 -mwindows -std=gnu99 -o app.exe
app.exe
pause