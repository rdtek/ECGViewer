#include <windows.h>
#include <winnt.h>
#include <stdio.h>

//Declare functions
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void DrawGridLines(HDC hdc, HWND hwnd, int interval);
void DrawSignal(HDC hdc, HWND hwnd);
void LoadSignalData(char* fileName, long* signalArray, int maxNum);

/*  WinMain(), our entry point  */
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
    HPEN	hBigGridPen;
    HPEN	hSmallGridPen;
    HPEN	hSignalPen;
    
    hSmallGridPen = CreatePen(PS_SOLID, 1, RGB(255, 192, 192));
    hBigGridPen = CreatePen(PS_SOLID, 1, RGB(240, 128, 128));
    hSignalPen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));

    /*  Switch according to what type of message we have received  */
    switch ( iMsg ) {
        case WM_PAINT:
            /*  Receive WM_PAINT every time window is updated  */
            hdc = BeginPaint(hwnd, &ps);
            SelectObject(hdc, hSmallGridPen); DrawGridLines(hdc, hwnd, 10);
            SelectObject(hdc, hBigGridPen); DrawGridLines(hdc, hwnd, 50);
            SelectObject(hdc, hSignalPen); DrawSignal(hdc, hwnd);
            EndPaint(hwnd, &ps);
            return 0;
	      
         case WM_DESTROY:
            /*  Window has been destroyed, so exit cleanly  */
            PostQuitMessage(0);
	    return 0;
    }

    /*  Send any messages we don't handle to default window procedure  */
    return DefWindowProc(hwnd, iMsg, wParam, lParam);
}

void DrawGridLines(HDC hdc, HWND hwnd, int interval){

    POINT points[2];
    RECT rect;

    if(GetWindowRect(hwnd, &rect)) {
        
        int windowWidth = rect.right - rect.left;
        int windowHeight = rect.bottom - rect.top;
		
        //Vertical lines
        for(int i = 0; i*interval < windowWidth; i++){
            points[0].x = interval * i;
            points[0].y = 0;
            points[1].x = interval * i;
            points[1].y = windowHeight;
            Polyline(hdc, points, 2);
        }
		
        //Horizontal lines
        for(int i = 0; i*interval < windowHeight; i++){
            points[0].x = 0;
            points[0].y = interval * i;
            points[1].x = windowWidth;
            points[1].y = interval * i;
            Polyline(hdc, points, 2);
        }
    }
}

void DrawSignal(HDC hdc, HWND hwnd){

    POINT points[2];
    RECT rect;

    long signalBuffer[10000];
	int maxSamples = 128;
    LoadSignalData("C:\\temp\\testdata.txt", signalBuffer, maxSamples);    
	
    if(GetWindowRect(hwnd, &rect)) {
        
		int windowWidth = rect.right - rect.left;
        int windowHeight = rect.bottom - rect.top;
		
		for(int i = 0; i < maxSamples - 1; i++){

			printf("i %ld\n", signalBuffer[i]);
		
			points[0].x = i;
			points[0].y = signalBuffer[i];
			points[1].x = i + 1;
			points[1].y = signalBuffer[i+1];
			Polyline(hdc, points, 2);
		}
    }
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
	printf("Number of lines: %d\n", numLines );
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
		
        fclose( ptr_file );
    }
}

