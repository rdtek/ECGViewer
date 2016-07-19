#include <windows.h>
#include <winnt.h>
#include <stdio.h>
#include <Commdlg.h>
#include <math.h>

#include "logging.h"
#include "ecg.h"

#define IDM_FILE_NEW 1
#define IDM_FILE_OPEN 2
#define IDM_FILE_QUIT 3

//Declare functions
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void AddMenus(HWND);
void DoOpenFile();
void LoadSignalData(char* fileName, long* signalArray, int maxNum);

OPENFILENAME openFileName;

/*  WinMain(), application entry point  */
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR szCmdLine, int iCmdShow) {
    
    static char szAppName[] = "ECGViewer";
    HWND        hwnd;
    MSG         msg;
    WNDCLASSEX  wndclass;        
    
    /*  Fill in WNDCLASSEX struct members  */
    wndclass.cbSize         = sizeof(wndclass);
    wndclass.style          = CS_HREDRAW | CS_VREDRAW;
    wndclass.lpfnWndProc    = WndProc;
    wndclass.cbClsExtra     = 0;
    wndclass.cbWndExtra     = 0;
    wndclass.hInstance      = hInstance;
    wndclass.hIcon          = LoadIcon(NULL, IDI_APPLICATION);
    wndclass.hIconSm        = LoadIcon(NULL, IDI_APPLICATION);
    wndclass.hCursor        = LoadCursor(NULL, IDC_ARROW);
    wndclass.hbrBackground  = (HBRUSH) GetStockObject(WHITE_BRUSH);
    wndclass.lpszClassName  = szAppName;
    wndclass.lpszMenuName   = NULL;

    /*  Register a new window class with Windows  */
    RegisterClassEx(&wndclass);

    /*  Create a window based on our new class  */
    hwnd = CreateWindow(szAppName, "ECG Viewer",
      WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, CW_USEDEFAULT,
      CW_USEDEFAULT, CW_USEDEFAULT,
      NULL, NULL, hInstance, NULL);
    
    /*  Show and update our window  */
    ShowWindow(hwnd, iCmdShow);
    UpdateWindow(hwnd);
    
    /*  Retrieve and process messages until we get WM_QUIT  */
    while ( GetMessage(&msg, NULL, 0, 0) ) {
        TranslateMessage(&msg);    /*  for certain keyboard messages  */
        DispatchMessage(&msg);     /*  send message to WndProc        */
    }

    /*  Exit with status specified in WM_QUIT message  */
    return msg.wParam;
}

/*  Window procedure  */
LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam) {

    PAINTSTRUCT ps;
    HDC         hdc;

    /*  Switch according to what type of message we have received  */
    switch ( iMsg ) {
    
        case WM_CREATE:
            AddMenus(hwnd);
            break;
        
        case WM_PAINT:
            hdc = BeginPaint(hwnd, &ps);
            DrawSignalPanel(hdc, hwnd);
            EndPaint(hwnd, &ps);
            return 0;
        
        case WM_COMMAND:
            switch(LOWORD(wParam)) {
                case IDM_FILE_NEW:
                    printf("File new\n");
                    break;
                case IDM_FILE_OPEN:
                    DoOpenFile();
                    RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
                    break;
                case IDM_FILE_QUIT:
                    SendMessage(hwnd, WM_CLOSE, 0, 0);
                    break;
            }
            return 0;
        
        case WM_DESTROY:
            /*  Window has been destroyed, so exit cleanly  */
            PostQuitMessage(0);
            return 0;
    }
    /*  Send any messages we don't handle to default window procedure  */
    return DefWindowProc(hwnd, iMsg, wParam, lParam);
}

void DoOpenFile(){
    char szFile[100];
     
     // open a file name
    ZeroMemory( &openFileName , sizeof( openFileName));
    openFileName.lStructSize        = sizeof ( openFileName );
    openFileName.hwndOwner          = NULL;
    openFileName.lpstrFile          = szFile;
    openFileName.lpstrFile[0]       = '\0';
    openFileName.nMaxFile           = sizeof( szFile );
    openFileName.lpstrFilter        = "All\0*.*\0Text\0*.TXT\0";
    openFileName.nFilterIndex       = 1;
    openFileName.lpstrFileTitle     = NULL;
    openFileName.nMaxFileTitle      = 0;
    openFileName.lpstrInitialDir    = NULL;
    openFileName.Flags              = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
    GetOpenFileName( &openFileName );
    
    LoadSignalData(openFileName.lpstrFile, signalBuffer, 10000);
}

int CountFileLines(char* fileName){
    int numLines = 0;
    int chr = 0;
    
    FILE * ptr_file = fopen(fileName, "r");
    if(ptr_file != NULL){
        while(!feof(ptr_file)) {
            chr = fgetc(ptr_file);
            if(chr == '\n') numLines++;
        }
        fclose( ptr_file );
    }
    
    log_int("\nNumber of lines: ", numLines);
    return numLines;
}

void LoadSignalData(char* fileName, long* signalArray, int maxNum){
    
    int numLines = CountFileLines(fileName);
    FILE * ptr_file = fopen(fileName, "r");
    
    if(ptr_file != NULL){
    
        int idxSignalArray = 0;
        char strLine [ 128 ];
        
        while ( fgets ( strLine, sizeof strLine, ptr_file ) != NULL ) {
            signalArray[idxSignalArray] = strtol(strLine, NULL, 10);
            idxSignalArray++;
            if(idxSignalArray >= maxNum) break;
        }
        if(idxSignalArray >= 1) signalLoaded = 1;
        
        fclose( ptr_file );
        log_int("\nidxSignalArray: ", idxSignalArray);
    }
}

void AddMenus(HWND hwnd) {

    HMENU hMenubar;
    HMENU hMenu;

    hMenubar = CreateMenu();
    hMenu = CreateMenu();

    AppendMenuW(hMenu, MF_STRING, IDM_FILE_NEW, L"&New");
    AppendMenuW(hMenu, MF_STRING, IDM_FILE_OPEN, L"&Open");
    AppendMenuW(hMenu, MF_SEPARATOR, 0, NULL);
    AppendMenuW(hMenu, MF_STRING, IDM_FILE_QUIT, L"&Quit");

    AppendMenuW(hMenubar, MF_POPUP, (UINT_PTR) hMenu, L"&File");
    SetMenu(hwnd, hMenubar);
}